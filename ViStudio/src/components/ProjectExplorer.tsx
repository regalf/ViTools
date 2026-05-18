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
  onFileClick: (path: string) => void
  onProjectLoad: (projectPath: string) => void
}

const getIconForFile = (name: string): string => {
  const ext = name.split('.').pop()?.toLowerCase() || ''
  const icons: Record<string, string> = {
    ts: '🔷', tsx: '⚛️', js: '📜', jsx: '⚛️',
    py: '🐍', html: '🌐', css: '🎨', json: '📋',
    md: '📝', java: '☕', cpp: '⚙️', c: '⚙️',
    go: '🐹', rs: '🦀', rb: '💎', php: '🐘',
    vue: '💚', svelte: '🔥', sql: '🗃️', xml: '📄',
    yml: '⚙️', yaml: '⚙️', sh: '🖥️'
  }
  return icons[ext] || '📄'
}

const FileTreeItem: React.FC<{
  node: FileNode
  depth: number
  onFileClick: (path: string) => void
  onToggle: (path: string) => void
}> = ({ node, depth, onFileClick, onToggle }) => {
  const [isHovered, setIsHovered] = useState(false)

  const handleClick = () => {
    if (node.isDirectory) {
      onToggle(node.path)
    } else {
      onFileClick(node.path)
    }
  }

  return React.createElement('div', null, [
    React.createElement('div', {
      key: 'row',
      onClick: handleClick,
      onMouseEnter: () => setIsHovered(true),
      onMouseLeave: () => setIsHovered(false),
      style: {
        display: 'flex',
        alignItems: 'center',
        padding: '4px 8px',
        paddingLeft: `${depth * 16 + 8}px`,
        cursor: 'pointer',
        background: isHovered ? '#2a2d2e' : 'transparent',
        fontSize: '13px',
        color: '#cccccc',
        userSelect: 'none'
      }
    }, [
      node.isDirectory && React.createElement('span', {
        key: 'arrow',
        style: {
          marginRight: '4px',
          fontSize: '10px',
          transform: node.expanded ? 'rotate(90deg)' : 'rotate(0deg)',
          transition: 'transform 0.1s'
        }
      }, '▶'),
      !node.isDirectory && React.createElement('span', { key: 'spacer', style: { width: '14px', marginRight: '4px' } }),
      React.createElement('span', { key: 'icon', style: { marginRight: '6px', fontSize: '14px' } },
        node.isDirectory ? (node.expanded ? '📂' : '📁') : getIconForFile(node.name)
      ),
      React.createElement('span', {
        key: 'name',
        style: {
          whiteSpace: 'nowrap',
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          color: node.isDirectory ? '#e8e8e8' : '#cccccc',
          fontWeight: node.name.startsWith('.') ? 'normal' : 'normal'
        }
      }, node.name)
    ]),
    node.isDirectory && node.expanded && node.children &&
      React.createElement('div', { key: 'children' },
        node.children.map(child =>
          React.createElement(FileTreeItem, {
            key: child.path,
            node: child,
            depth: depth + 1,
            onFileClick,
            onToggle
          })
        )
      )
  ])
}

const ProjectExplorer: React.FC<ProjectExplorerProps> = ({ folderPath, onFileClick, onProjectLoad }) => {
  const [fileTree, setFileTree] = useState<FileNode[]>([])
  const [loading, setLoading] = useState(false)
  const [expandedPaths, setExpandedPaths] = useState<Set<string>>(new Set())
  const [projectConfig, setProjectConfig] = useState<any>(null)
  const [rootExpanded, setRootExpanded] = useState(true)

  const loadDirectory = useCallback(async (dirPath: string): Promise<FileNode[]> => {
    if (!window.electronAPI) {
      console.log('electronAPI not available')
      return []
    }
    console.log('Loading directory:', dirPath)
    const result = await window.electronAPI.fs.readDir(dirPath)
    console.log('readDir result:', result)
    if (!result.success || !result.items) {
      console.log('No items or not success')
      return []
    }

    const items = result.items
      .filter(item => !item.name.startsWith('.') && item.name !== 'node_modules')
      .sort((a, b) => {
        if (a.isDirectory && !b.isDirectory) return -1
        if (!a.isDirectory && b.isDirectory) return 1
        return a.name.localeCompare(b.name)
      })

    console.log('Filtered items:', items.length)
    return items.map(item => ({
      name: item.name,
      path: item.path,
      isDirectory: item.isDirectory,
      expanded: false
    }))
  }, [])

  const toggleDirectory = useCallback(async (dirPath: string) => {
    setExpandedPaths(prev => {
      const next = new Set(prev)
      if (next.has(dirPath)) {
        next.delete(dirPath)
      } else {
        next.add(dirPath)
      }
      return next
    })

    setFileTree(prev => {
      const updateTree = (nodes: FileNode[]): FileNode[] => {
        return nodes.map(node => {
          if (node.path === dirPath) {
            const isExpanding = !expandedPaths.has(dirPath)
            return {
              ...node,
              expanded: isExpanding,
              children: isExpanding && !node.children ? [] : node.children
            }
          }
          if (node.children) {
            return { ...node, children: updateTree(node.children) }
          }
          return node
        })
      }
      return updateTree(prev)
    })

    if (!expandedPaths.has(dirPath)) {
      const children = await loadDirectory(dirPath)
      setFileTree(prev => {
        const updateTree = (nodes: FileNode[]): FileNode[] => {
          return nodes.map(node => {
            if (node.path === dirPath) {
              return { ...node, children }
            }
            if (node.children) {
              return { ...node, children: updateTree(node.children) }
            }
            return node
          })
        }
        return updateTree(prev)
      })
    }
  }, [expandedPaths, loadDirectory])

  useEffect(() => {
    if (!folderPath) {
      setFileTree([])
      setProjectConfig(null)
      return
    }

    console.log('ProjectExplorer: folderPath changed to', folderPath)
    setLoading(true)
    setRootExpanded(true)
    loadDirectory(folderPath).then(async tree => {
      console.log('Tree loaded:', tree.length, 'items')
      setFileTree(tree)
      setExpandedPaths(new Set([folderPath]))

      const projPath = `${folderPath}/.vistproj`
      if (window.electronAPI) {
        const exists = await window.electronAPI.fs.exists(projPath)
        if (exists) {
          const result = await window.electronAPI.fs.readFile(projPath)
          if (result.success && result.content) {
            try {
              const config = JSON.parse(result.content)
              setProjectConfig(config)
              onProjectLoad(projPath)
            } catch (e) {
              console.error('Failed to parse .vistproj:', e)
            }
          }
        }
      }
      setLoading(false)
    })
  }, [folderPath, loadDirectory, onProjectLoad])

  if (!folderPath) {
    return React.createElement('div', {
      style: {
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        height: '100%',
        padding: '20px',
        textAlign: 'center'
      }
    }, [
      React.createElement('p', {
        key: 'text',
        style: { color: '#858585', fontSize: '13px', marginBottom: '15px' }
      }, 'No project opened'),
      React.createElement('p', {
        key: 'hint',
        style: { color: '#555', fontSize: '12px' }
      }, 'Open a folder to explore files')
    ])
  }

  if (loading) {
    return React.createElement('div', {
      style: { padding: '10px', color: '#858585', fontSize: '13px' }
    }, 'Loading...')
  }

  const folderName = folderPath.split('/').pop() || 'Project'

  console.log('Rendering explorer with', fileTree.length, 'items')

  if (fileTree.length === 0) {
    return React.createElement('div', {
      style: {
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        height: '100%',
        padding: '20px',
        textAlign: 'center'
      }
    }, [
      React.createElement('p', {
        key: 'text',
        style: { color: '#858585', fontSize: '13px', marginBottom: '15px' }
      }, 'Folder is empty'),
      React.createElement('p', {
        key: 'hint',
        style: { color: '#555', fontSize: '12px' }
      }, folderPath)
    ])
  }

  return React.createElement('div', {
    style: { display: 'flex', flexDirection: 'column', height: '100%' }
  }, [
    projectConfig && React.createElement('div', {
      key: 'project-info',
      style: {
        padding: '8px 12px',
        background: '#2d2d30',
        borderBottom: '1px solid #3c3c3c',
        fontSize: '12px',
        color: '#858585'
      }
    }, `📦 ${projectConfig.name || folderName} v${projectConfig.version || '1.0.0'}`),
    React.createElement('div', {
      key: 'tree',
      style: {
        flex: 1,
        overflowY: 'auto',
        padding: '4px 0'
      },
      className: 'explorer-tree'
    }, [
      React.createElement('div', {
        key: 'root-folder',
        onClick: () => setRootExpanded(!rootExpanded),
        style: {
          display: 'flex',
          alignItems: 'center',
          padding: '4px 8px',
          cursor: 'pointer',
          fontSize: '13px',
          color: '#e8e8e8',
          fontWeight: 600,
          userSelect: 'none'
        }
      }, [
        React.createElement('span', {
          key: 'arrow',
          style: {
            marginRight: '4px',
            fontSize: '10px',
            transform: rootExpanded ? 'rotate(90deg)' : 'rotate(0deg)',
            transition: 'transform 0.1s'
          }
        }, '▶'),
        React.createElement('span', { key: 'icon', style: { marginRight: '6px', fontSize: '14px' } }, rootExpanded ? '📂' : '📁'),
        React.createElement('span', { key: 'name' }, folderName)
      ]),
      ...(rootExpanded ? fileTree.map(node =>
        React.createElement(FileTreeItem, {
          key: node.path,
          node,
          depth: 1,
          onFileClick,
          onToggle: toggleDirectory
        })
      ) : [])
    ])
  ])
}

export default ProjectExplorer
