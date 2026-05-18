import React, { useState, useEffect, useCallback } from 'react'

export interface FileNode {
  name: string
  path: string
  isDirectory: boolean
  children?: FileNode[]
  expanded?: boolean
}

interface ProjectExplorerProps {
  folderPath: string | null
  refreshPath?: string | null
  onFileClick: (path: string) => void
  onProjectLoad: (projectPath: string) => void
  onNewFile: (parentPath: string) => void
  onNewFolder: (parentPath: string) => void
  onRefreshPathConsumed: () => void
  onRefresh: () => void
}

const ProjectExplorer: React.FC<ProjectExplorerProps> = ({ folderPath, refreshPath, onFileClick, onProjectLoad, onNewFile, onNewFolder, onRefreshPathConsumed, onRefresh }) => {
  const [fileTree, setFileTree] = useState<FileNode[]>([])
  const [loading, setLoading] = useState(false)
  const [expandedPaths, setExpandedPaths] = useState<Set<string>>(new Set())
  const [projectConfig, setProjectConfig] = useState<any>(null)
  const [rootExpanded, setRootExpanded] = useState(true)
  const [draggedNode, setDraggedNode] = useState<FileNode | null>(null)
  const [dragOverPath, setDragOverPath] = useState<string | null>(null)

  const loadDirectory = useCallback(async (dirPath: string): Promise<FileNode[]> => {
    if (!window.electronAPI) return []
    const result = await window.electronAPI.fs.readDir(dirPath)
    if (!result.success || !result.items) return []
    return result.items
      .filter(item => !item.name.startsWith('.') && item.name !== 'node_modules')
      .sort((a, b) => {
        if (a.isDirectory && !b.isDirectory) return -1
        if (!a.isDirectory && b.isDirectory) return 1
        return a.name.localeCompare(b.name)
      })
      .map(item => ({ name: item.name, path: item.path, isDirectory: item.isDirectory, expanded: false }))
  }, [])

  const toggleDirectory = useCallback(async (dirPath: string) => {
    setExpandedPaths(prev => {
      const next = new Set(prev)
      if (next.has(dirPath)) next.delete(dirPath)
      else next.add(dirPath)
      return next
    })
    if (!expandedPaths.has(dirPath)) {
      const children = await loadDirectory(dirPath)
      setFileTree(prev => {
        const updateTree = (nodes: FileNode[]): FileNode[] => nodes.map(node => {
          if (node.path === dirPath) return { ...node, children }
          if (node.children) return { ...node, children: updateTree(node.children) }
          return node
        })
        return updateTree(prev)
      })
    }
  }, [expandedPaths, loadDirectory])

  const handleDrop = useCallback(async (sourcePath: string, destPath: string) => {
    if (!window.electronAPI || !sourcePath) return
    
    if (destPath.startsWith(sourcePath)) {
      console.warn('Cannot drop into itself or subdirectory')
      return
    }

    const result = await window.electronAPI.fs.move(sourcePath, destPath)
    if (result.success) {
      const destParent = destPath.substring(0, destPath.lastIndexOf('/'))
      const sourceParent = sourcePath.substring(0, sourcePath.lastIndexOf('/'))
      
      const pathsToUpdate = [sourceParent]
      if (sourceParent !== destParent) pathsToUpdate.push(destParent)
      
      const updates = await Promise.all(
        pathsToUpdate.map(async (p) => ({ path: p, fresh: await loadDirectory(p) }))
      )
      
      setFileTree(prev => {
        let current = prev
        for (const { path, fresh } of updates) {
          const mergeChildren = (freshNodes: FileNode[], existing?: FileNode[]) => {
            return freshNodes.map(freshNode => {
              const existingNode = existing?.find(e => e.path === freshNode.path)
              if (existingNode && existingNode.expanded) {
                return { ...freshNode, expanded: true, children: existingNode.children }
              }
              return { ...freshNode, expanded: existingNode?.expanded ?? false }
            })
          }

          const updateTree = (nodes: FileNode[]): FileNode[] => nodes.map(node => {
            if (node.path === path) {
              return { ...node, children: mergeChildren(fresh, node.children) }
            }
            if (node.children) {
              return { ...node, children: updateTree(node.children) }
            }
            return node
          })
          current = updateTree(current)
        }
        return current
      })
    } else {
      console.error('Failed to move file:', result.error)
    }
  }, [loadDirectory])

  useEffect(() => {
    if (!folderPath) { setFileTree([]); setProjectConfig(null); return }
    setLoading(true)
    setRootExpanded(true)
    loadDirectory(folderPath).then(async tree => {
      setFileTree(tree)
      setExpandedPaths(new Set([folderPath]))
      const projPath = `${folderPath}/.vistproj`
      if (window.electronAPI) {
        const exists = await window.electronAPI.fs.exists(projPath)
        if (exists) {
          const result = await window.electronAPI.fs.readFile(projPath)
          if (result.success && result.content) {
            try { setProjectConfig(JSON.parse(result.content)); onProjectLoad(projPath) } catch (e) { console.error(e) }
          }
        }
      }
      setLoading(false)
    })
  }, [folderPath, loadDirectory, onProjectLoad])

  useEffect(() => {
    if (refreshPath) {
      setLoading(true)
      loadDirectory(refreshPath).then(freshChildren => {
        setFileTree(prev => {
          const mergeChildren = (fresh: FileNode[], existing?: FileNode[]) => {
            return fresh.map(freshNode => {
              const existingNode = existing?.find(e => e.path === freshNode.path)
              if (existingNode && existingNode.expanded) {
                return { ...freshNode, expanded: true, children: existingNode.children }
              }
              return { ...freshNode, expanded: existingNode?.expanded ?? false }
            })
          }

          if (refreshPath === folderPath) {
            return mergeChildren(freshChildren, prev)
          }

          const updateTree = (nodes: FileNode[]): FileNode[] => nodes.map(node => {
            if (node.path === refreshPath) {
              return { ...node, children: mergeChildren(freshChildren, node.children) }
            }
            if (node.children) {
              return { ...node, children: updateTree(node.children) }
            }
            return node
          })
          return updateTree(prev)
        })
        setLoading(false)
        onRefreshPathConsumed()
      })
    }
  }, [refreshPath])

  const FileTreeItem: React.FC<{ node: FileNode; depth: number }> = ({ node, depth }) => {
    const isExpanded = expandedPaths.has(node.path)
    const isDragOver = dragOverPath === node.path
    const isDragging = draggedNode?.path === node.path
    const paddingLeft = depth * 16

    if (node.isDirectory) {
      return React.createElement('div', null,
        React.createElement('div', {
          key: node.path,
          style: { display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '2px 8px', paddingLeft: `${paddingLeft + 8}px`, background: isDragOver ? '#2a2d2e' : 'transparent', border: isDragOver ? '1px dashed #007acc' : '1px solid transparent', borderRadius: '3px', userSelect: 'none' },
          onDragEnter: (e: any) => { e.preventDefault(); e.stopPropagation(); setDragOverPath(node.path) },
          onDragLeave: (e: any) => { e.preventDefault(); e.stopPropagation(); if (dragOverPath === node.path) setDragOverPath(null) },
          onDragOver: (e: any) => { e.preventDefault(); e.stopPropagation(); e.dataTransfer.dropEffect = 'move' },
          onDrop: (e: any) => {
            e.preventDefault();
            e.stopPropagation();
            setDragOverPath(null);
            const sourcePath = e.dataTransfer.getData('text/plain');
            if (sourcePath && sourcePath !== node.path) {
              handleDrop(sourcePath, `${node.path}/${sourcePath.split('/').pop()}`);
            }
          }
        },
          React.createElement('div', {
            key: 'info',
            onClick: () => toggleDirectory(node.path),
            draggable: true,
            onDragStart: (e: any) => {
              e.stopPropagation();
              e.dataTransfer.effectAllowed = 'move';
              e.dataTransfer.setData('text/plain', node.path);
              setTimeout(() => setDraggedNode(node), 0);
            },
            onDragEnd: () => { setDraggedNode(null); setDragOverPath(null); },
            style: { display: 'flex', alignItems: 'center', cursor: 'grab', fontSize: '13px', color: isDragging ? '#555' : '#cccccc', opacity: isDragging ? 0.5 : 1, WebkitUserDrag: 'element' }
          },
            React.createElement('span', { key: 'arrow', style: { marginRight: '4px', fontSize: '10px', transform: isExpanded ? 'rotate(90deg)' : 'rotate(0deg)', transition: 'transform 0.1s', userSelect: 'none', pointerEvents: 'none' } }, '▶'),
            React.createElement('span', { key: 'icon', style: { marginRight: '6px', fontSize: '14px', userSelect: 'none', pointerEvents: 'none' } }, isExpanded ? '📂' : '📁'),
            React.createElement('span', { key: 'name', style: { userSelect: 'none', pointerEvents: 'none' } }, node.name)
          ),
          React.createElement('div', { key: 'actions', style: { display: 'flex', gap: '2px', opacity: 0.6 } },
            React.createElement('button', { key: 'new-file', onClick: (e: any) => { e.stopPropagation(); onNewFile(node.path) }, style: { background: 'none', border: 'none', color: '#858585', cursor: 'pointer', padding: '2px 4px', fontSize: '12px', lineHeight: 1 }, title: 'New File' }, '📄+'),
            React.createElement('button', { key: 'new-folder', onClick: (e: any) => { e.stopPropagation(); onNewFolder(node.path) }, style: { background: 'none', border: 'none', color: '#858585', cursor: 'pointer', padding: '2px 4px', fontSize: '12px', lineHeight: 1 }, title: 'New Folder' }, '📁+')
          )
        ),
        isExpanded && node.children ? node.children.map(child => React.createElement(FileTreeItem, { key: child.path, node: child, depth: depth + 1 })) : null
      )
    }

    return React.createElement('div', {
      onClick: () => onFileClick(node.path),
      draggable: true,
      onDragStart: (e: any) => {
        e.stopPropagation();
        e.dataTransfer.effectAllowed = 'move';
        e.dataTransfer.setData('text/plain', node.path);
        setTimeout(() => setDraggedNode(node), 0);
      },
      onDragEnd: () => { setDraggedNode(null); setDragOverPath(null); },
      style: { display: 'flex', alignItems: 'center', padding: '2px 8px', paddingLeft: `${paddingLeft + 8}px`, cursor: 'grab', fontSize: '13px', color: isDragging ? '#555' : '#cccccc', opacity: isDragging ? 0.5 : 1, userSelect: 'none', WebkitUserDrag: 'element' }
    },
      React.createElement('span', { key: 'icon', style: { marginRight: '6px', fontSize: '14px', userSelect: 'none', pointerEvents: 'none' } }, '📄'),
      React.createElement('span', { key: 'name', style: { userSelect: 'none', pointerEvents: 'none' } }, node.name)
    )
  }

  if (!folderPath) {
    return React.createElement('div', { style: { display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', height: '100%', padding: '20px', textAlign: 'center' } },
      React.createElement('p', { key: 'text', style: { color: '#858585', fontSize: '13px', marginBottom: '15px' } }, 'No project opened'),
      React.createElement('p', { key: 'hint', style: { color: '#555', fontSize: '12px' } }, 'Open a folder to explore files')
    )
  }

  if (loading) {
    return React.createElement('div', { style: { padding: '10px', color: '#858585', fontSize: '13px' } }, 'Loading...')
  }

  const folderName = folderPath.split('/').pop() || 'Project'

  return React.createElement('div', { style: { display: 'flex', flexDirection: 'column', height: '100%' } },
    projectConfig && React.createElement('div', { key: 'project-info', style: { padding: '8px 12px', background: '#2d2d30', borderBottom: '1px solid #3c3c3c', fontSize: '12px', color: '#858585' } }, `📦 ${projectConfig.name || folderName} v${projectConfig.version || '1.0.0'}`),
    React.createElement('div', { key: 'tree', style: { flex: 1, overflowY: 'auto', padding: '4px 0' }, className: 'explorer-tree' },
      React.createElement('div', {
        key: 'root-folder',
        style: { display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '4px 8px', userSelect: 'none', background: dragOverPath === folderPath ? '#2a2d2e' : 'transparent', border: dragOverPath === folderPath ? '1px dashed #007acc' : '1px solid transparent', borderRadius: '3px' },
        onDragEnter: (e: any) => { e.preventDefault(); e.stopPropagation(); setDragOverPath(folderPath || '') },
        onDragLeave: (e: any) => { e.preventDefault(); e.stopPropagation(); if (dragOverPath === folderPath) setDragOverPath(null) },
        onDragOver: (e: any) => { e.preventDefault(); e.stopPropagation(); e.dataTransfer.dropEffect = 'move' },
        onDrop: (e: any) => {
          e.preventDefault();
          e.stopPropagation();
          setDragOverPath(null);
          const sourcePath = e.dataTransfer.getData('text/plain');
          if (sourcePath && folderPath && sourcePath !== folderPath) {
            handleDrop(sourcePath, `${folderPath}/${sourcePath.split('/').pop()}`);
          }
        }
      },
        React.createElement('div', { key: 'folder-info', onClick: () => setRootExpanded(!rootExpanded), style: { display: 'flex', alignItems: 'center', cursor: 'pointer', fontSize: '13px', color: '#e8e8e8', fontWeight: 600 } },
          React.createElement('span', { key: 'arrow', style: { marginRight: '4px', fontSize: '10px', transform: rootExpanded ? 'rotate(90deg)' : 'rotate(0deg)', transition: 'transform 0.1s' } }, '▶'),
          React.createElement('span', { key: 'icon', style: { marginRight: '6px', fontSize: '14px' } }, rootExpanded ? '📂' : '📁'),
          React.createElement('span', { key: 'name' }, folderName)
        ),
        React.createElement('div', { key: 'actions', style: { display: 'flex', gap: '4px' } },
          React.createElement('button', { key: 'new-file', onClick: (e: any) => { e.stopPropagation(); onNewFile(folderPath || '') }, style: { background: 'none', border: 'none', color: '#858585', cursor: 'pointer', padding: '2px 4px', fontSize: '14px', lineHeight: 1 }, title: 'New File' }, '📄+'),
          React.createElement('button', { key: 'new-folder', onClick: (e: any) => { e.stopPropagation(); onNewFolder(folderPath || '') }, style: { background: 'none', border: 'none', color: '#858585', cursor: 'pointer', padding: '2px 4px', fontSize: '14px', lineHeight: 1 }, title: 'New Folder' }, '📁+'),
          React.createElement('button', { key: 'refresh', onClick: (e: any) => { e.stopPropagation(); onRefresh() }, style: { background: 'none', border: 'none', color: '#858585', cursor: 'pointer', padding: '2px 4px', fontSize: '14px', lineHeight: 1 }, title: 'Refresh' }, '🔄')
        )
      ),
      rootExpanded ? fileTree.map(node => React.createElement(FileTreeItem, { key: node.path, node, depth: 1 })) : null
    )
  )
}

export default ProjectExplorer
