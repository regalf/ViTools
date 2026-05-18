import React, { useState, useEffect, useCallback } from 'react'
import MenuBar from './components/MenuBar'
import Sidebar from './components/Sidebar'
import EditorPanel from './components/EditorPanel'
import StatusBar from './components/StatusBar'
import TabBar from './components/TabBar'
import TerminalPanel from './components/TerminalPanel'
import './styles/global.css'
import { EditorTab } from './types'

const getLanguageFromExtension = (fileName: string): string => {
  const ext = fileName.split('.').pop()?.toLowerCase() || ''
  const languageMap: Record<string, string> = {
    js: 'javascript', jsx: 'javascript', ts: 'typescript', tsx: 'typescript',
    py: 'python', html: 'html', css: 'css', json: 'json', md: 'markdown',
    java: 'java', cpp: 'cpp', c: 'c', cs: 'csharp', go: 'go', rs: 'rust',
    rb: 'ruby', php: 'php', sh: 'shell', xml: 'xml', yaml: 'yaml', yml: 'yaml',
    sql: 'sql', vue: 'vue', svelte: 'svelte'
  }
  return languageMap[ext] || 'plaintext'
}

const App: React.FC = () => {
  const [sidebarOpen, setSidebarOpen] = useState(true)
  const [folderPath, setFolderPath] = useState<string | null>(null)
  const [tabs, setTabs] = useState<EditorTab[]>([])
  const [activeTabId, setActiveTabId] = useState<string | null>(null)
  const [_cursorPosition, _setCursorPosition] = useState({ line: 1, column: 1 })
  const [_projectConfig, setProjectConfig] = useState<any>(null)
  const [refreshPath, setRefreshPath] = useState<string | null>(null)
  const [showFolderInput, setShowFolderInput] = useState(false)
  const [folderInputValue, setFolderInputValue] = useState('')
  const [folderInputTarget, setFolderInputTarget] = useState('')
  const [showFileInput, setShowFileInput] = useState(false)
  const [fileInputValue, setFileInputValue] = useState('')
  const [fileInputTarget, setFileInputTarget] = useState('')
  const [terminalVisible, setTerminalVisible] = useState(false)

  const activeTab = tabs.find(t => t.id === activeTabId) || null

  const openFileInTab = useCallback(async (path: string | null, content: string, isNew: boolean = false) => {
    const name = path ? path.split('/').pop() || 'Untitled' : 'Untitled'
    const language = path ? getLanguageFromExtension(path) : 'plaintext'
    const id = path || `new-${Date.now()}`
    
    setTabs(prev => {
      const existing = prev.find(t => t.path === path && path !== null)
      if (existing) {
        setActiveTabId(existing.id)
        return prev
      }
      return [...prev, { id, path, name, content, language, isModified: false, isNew }]
    })
    setActiveTabId(id)
  }, [])

  const closeTab = useCallback((tabId: string) => {
    setTabs(prev => {
      const newTabs = prev.filter(t => t.id !== tabId)
      if (activeTabId === tabId) {
        setActiveTabId(newTabs.length > 0 ? newTabs[newTabs.length - 1].id : null)
      }
      return newTabs
    })
  }, [activeTabId])

  const switchTab = useCallback((tabId: string) => {
    setActiveTabId(tabId)
  }, [])

  const handleTabReorder = useCallback((newTabs: EditorTab[]) => {
    setTabs(newTabs)
  }, [])

  const updateTabContent = useCallback((tabId: string, content: string) => {
    setTabs(prev => prev.map(t => t.id === tabId ? { ...t, content, isModified: true } : t))
  }, [])

  const saveTab = useCallback(async (tabId: string) => {
    const tab = tabs.find(t => t.id === tabId)
    if (!tab || !tab.path) return
    
    try {
      if (window.electronAPI) {
        await window.electronAPI.fs.writeFile(tab.path, tab.content)
        setTabs(prev => prev.map(t => t.id === tabId ? { ...t, isModified: false, isNew: false } : t))
      }
    } catch (e) {
      console.error('Failed to save file:', e)
    }
  }, [tabs])

  const handleOpenFolder = useCallback(async () => {
    try {
      if (window.electronAPI) {
        const path = await window.electronAPI.dialog.openFolder()
        if (path) setFolderPath(path)
      }
    } catch (e) {
      console.error('Failed to open folder:', e)
    }
  }, [])

  const handleCreateProject = useCallback(async () => {
    try {
      if (window.electronAPI) {
        const projectPath = await window.electronAPI.project.create()
        if (projectPath) {
          setFolderPath(projectPath)
          setProjectConfig({ name: projectPath.split('/').pop() })
        }
      }
    } catch (e) {
      console.error('Failed to create project:', e)
    }
  }, [])

  const handleCreateFolder = useCallback(async () => {
    setShowFolderInput(true)
    setFolderInputValue('NewFolder')
    setFolderInputTarget(folderPath || '')
  }, [folderPath])

  const handleNewFileInExplorer = useCallback((parentPath: string) => {
    setShowFileInput(true)
    setFileInputValue('untitled.txt')
    setFileInputTarget(parentPath)
  }, [])

  const handleNewFolderInExplorer = useCallback((parentPath: string) => {
    setShowFolderInput(true)
    setFolderInputValue('NewFolder')
    setFolderInputTarget(parentPath)
  }, [])

  const handleRefreshExplorer = useCallback(() => {
    setRefreshPath(folderPath)
  }, [folderPath])

  const handleToggleTerminal = useCallback(() => {
    setTerminalVisible(prev => !prev)
  }, [])

  const handleFolderInputConfirm = useCallback(async () => {
    if (!folderInputValue) {
      setShowFolderInput(false)
      return
    }
    try {
      if (window.electronAPI) {
        const newFolderPath = await window.electronAPI.folder.create(folderInputValue, folderInputTarget || undefined)
        if (newFolderPath) {
          setRefreshPath(folderInputTarget || folderPath)
        }
      }
    } catch (e) {
      console.error('Failed to create folder:', e)
    }
    setShowFolderInput(false)
  }, [folderInputValue, folderInputTarget, folderPath])

  const handleFileInputConfirm = useCallback(async () => {
    if (!fileInputValue) {
      setShowFileInput(false)
      return
    }
    try {
      if (window.electronAPI) {
        const newFilePath = await window.electronAPI.file.create(fileInputValue, fileInputTarget)
        if (newFilePath) {
          setRefreshPath(fileInputTarget)
        }
      }
    } catch (e) {
      console.error('Failed to create file:', e)
    }
    setShowFileInput(false)
  }, [fileInputValue, fileInputTarget])

  const handleOpenFile = useCallback(async () => {
    try {
      if (window.electronAPI) {
        const path = await window.electronAPI.dialog.openFile()
        if (path) {
          const result = await window.electronAPI.fs.readFile(path)
          if (result.success && result.content !== undefined) {
            openFileInTab(path, result.content)
          }
        }
      }
    } catch (e) {
      console.error('Failed to open file:', e)
    }
  }, [openFileInTab])

  const handleFileClick = useCallback(async (path: string) => {
    try {
      if (window.electronAPI) {
        const result = await window.electronAPI.fs.readFile(path)
        if (result.success && result.content !== undefined) {
          openFileInTab(path, result.content)
        }
      }
    } catch (e) {
      console.error('Failed to open file:', e)
    }
  }, [])

  const handleProjectLoad = useCallback((projectPath: string) => {
    console.log('Project loaded:', projectPath)
  }, [])

  const handleNewFile = useCallback(async () => {
    openFileInTab(null, '', true)
  }, [openFileInTab])

  const handleSave = useCallback(async () => {
    if (!activeTabId) return
    const tab = tabs.find(t => t.id === activeTabId)
    if (!tab) return

    if (tab.isNew || !tab.path) {
      try {
        if (window.electronAPI) {
          const defaultPath = folderPath ? `${folderPath}/${tab.name}` : undefined
          const path = await window.electronAPI.dialog.saveFile(defaultPath)
          if (path) {
            await window.electronAPI.fs.writeFile(path, tab.content)
            setTabs(prev => prev.map(t => t.id === activeTabId ? { ...t, path, name: path.split('/').pop() || 'Untitled', isModified: false, isNew: false } : t))
          }
        }
      } catch (e) {
        console.error('Failed to save:', e)
      }
    } else {
      saveTab(activeTabId)
    }
  }, [activeTabId, tabs, folderPath, saveTab])

  const handleSaveAs = useCallback(async () => {
    if (!activeTabId) return
    const tab = tabs.find(t => t.id === activeTabId)
    if (!tab) return

    try {
      if (window.electronAPI) {
        const defaultPath = folderPath ? `${folderPath}/` : undefined
        const path = await window.electronAPI.dialog.saveFile(defaultPath)
        if (path) {
          await window.electronAPI.fs.writeFile(path, tab.content)
          setTabs(prev => prev.map(t => t.id === activeTabId ? { ...t, path, name: path.split('/').pop() || 'Untitled', isModified: false, isNew: false } : t))
        }
      }
    } catch (e) {
      console.error('Failed to save as:', e)
    }
  }, [activeTabId, tabs, folderPath])

  const handleMenuAction = useCallback(async (action: string) => {
    console.log('Menu action:', action)
    switch (action) {
      case 'menu:new-file':
        await handleNewFile()
        break
      case 'menu:new-folder':
        await handleCreateFolder()
        break
      case 'menu:new-project':
        await handleCreateProject()
        break
      case 'menu:open-file':
        await handleOpenFile()
        break
      case 'menu:open-folder':
        await handleOpenFolder()
        break
      case 'menu:save':
        await handleSave()
        break
      case 'menu:save-as':
        await handleSaveAs()
        break
      case 'menu:toggle-sidebar':
        setSidebarOpen(prev => !prev)
        break
      case 'menu:toggle-terminal':
        handleToggleTerminal()
        break
      case 'menu:about':
        alert('ViStudio IDE v0.4.0\nMulti-tab editing is here!')
        break
    }
  }, [handleNewFile, handleCreateFolder, handleCreateProject, handleOpenFile, handleOpenFolder, handleSave, handleSaveAs])

  const handleEditorChange = useCallback((value: string) => {
    if (activeTabId) {
      updateTabContent(activeTabId, value)
    }
  }, [activeTabId, updateTabContent])

  const displayName = activeTab?.name || (tabs.length === 0 ? 'ViStudio' : 'Untitled')

  return React.createElement('div', {
    style: {
      position: 'absolute',
      top: 0, left: 0, right: 0, bottom: 0,
      display: 'flex',
      flexDirection: 'column',
      background: '#1e1e1e',
      color: '#cccccc',
      fontFamily: 'sans-serif'
    }
  }, [
    React.createElement(MenuBar, {
      key: 'menubar',
      onAction: handleMenuAction
    }),
    React.createElement('div', {
      key: 'main',
      style: { display: 'flex', flex: 1, overflow: 'hidden' }
    }, [
      React.createElement(Sidebar, {
        key: 'sidebar',
        isOpen: sidebarOpen,
        folderPath: folderPath,
        refreshPath: refreshPath,
        onOpenFolder: handleOpenFolder,
        onFileClick: handleFileClick,
        onProjectLoad: handleProjectLoad,
        onNewFile: handleNewFileInExplorer,
        onNewFolder: handleNewFolderInExplorer,
        onRefreshPathConsumed: () => setRefreshPath(null),
        onRefresh: handleRefreshExplorer
      }),
      React.createElement('div', {
        key: 'editor-area',
        style: { flex: 1, display: 'flex', flexDirection: 'column', overflow: 'hidden' }
      }, [
      React.createElement(TabBar, {
        key: 'tabbar',
        tabs: tabs,
        activeTabId: activeTabId,
        onSwitch: switchTab,
        onClose: closeTab,
        onReorder: handleTabReorder
      }),
        React.createElement(EditorPanel, {
          key: 'editor',
          filePath: activeTab?.path || null,
          fileName: activeTab?.name || '',
          content: activeTab?.content || '',
          language: activeTab?.language || 'plaintext',
          onChange: handleEditorChange,
          showWelcome: tabs.length === 0
        }),
        terminalVisible && React.createElement('div', {
          key: 'terminal-container',
          style: {
            height: '200px',
            borderTop: '1px solid #3c3c3c',
            display: 'flex',
            flexDirection: 'column'
          }
        }, [
          React.createElement('div', {
            key: 'terminal-header',
            style: {
              padding: '4px 10px',
              background: '#252526',
              color: '#cccccc',
              fontSize: '12px',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              borderBottom: '1px solid #3c3c3c'
            }
          }, [
            React.createElement('span', { key: 'title' }, 'TERMINAL'),
            React.createElement('span', {
              key: 'close',
              onClick: () => setTerminalVisible(false),
              style: { cursor: 'pointer', fontSize: '16px' }
            }, '×')
          ]),
          React.createElement('div', {
            key: 'terminal-body',
            style: { flex: 1, overflow: 'hidden' }
          }, React.createElement(TerminalPanel, {
            folderPath: folderPath,
            isVisible: terminalVisible
          }))
        ])
      ])
    ]),
    React.createElement(StatusBar, {
      key: 'statusbar',
      filePath: activeTab?.path || null,
      language: activeTab?.language || 'plaintext',
      cursorPosition: _cursorPosition
    }),
    showFolderInput && React.createElement('div', {
      key: 'folder-modal',
      style: {
        position: 'fixed',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        background: 'rgba(0,0,0,0.5)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 1000
      }
    }, [
      React.createElement('div', {
        key: 'modal-content',
        style: {
          background: '#252526',
          padding: '20px',
          borderRadius: '8px',
          minWidth: '300px',
          border: '1px solid #3c3c3c'
        }
      }, [
        React.createElement('h3', {
          key: 'title',
          style: { margin: '0 0 15px 0', color: '#cccccc', fontSize: '16px' }
        }, 'New Folder'),
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: folderInputValue,
          onChange: (e: any) => setFolderInputValue(e.target.value),
          onKeyDown: (e: any) => e.key === 'Enter' && handleFolderInputConfirm(),
          autoFocus: true,
          style: {
            width: '100%',
            padding: '8px',
            background: '#3c3c3c',
            border: '1px solid #555',
            color: '#cccccc',
            borderRadius: '4px',
            marginBottom: '15px',
            boxSizing: 'border-box'
          }
        }),
        React.createElement('div', {
          key: 'buttons',
          style: { display: 'flex', justifyContent: 'flex-end', gap: '10px' }
        }, [
          React.createElement('button', {
            key: 'cancel',
            onClick: () => setShowFolderInput(false),
            style: {
              padding: '6px 16px',
              background: '#3c3c3c',
              border: 'none',
              color: '#cccccc',
              borderRadius: '4px',
              cursor: 'pointer'
            }
          }, 'Cancel'),
          React.createElement('button', {
            key: 'create',
            onClick: handleFolderInputConfirm,
            style: {
              padding: '6px 16px',
              background: '#007acc',
              border: 'none',
              color: '#fff',
              borderRadius: '4px',
              cursor: 'pointer'
            }
          }, 'Create')
        ])
      ])
    ]),
    showFileInput && React.createElement('div', {
      key: 'file-modal',
      style: {
        position: 'fixed',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        background: 'rgba(0,0,0,0.5)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 1000
      }
    }, [
      React.createElement('div', {
        key: 'modal-content',
        style: {
          background: '#252526',
          padding: '20px',
          borderRadius: '8px',
          minWidth: '300px',
          border: '1px solid #3c3c3c'
        }
      }, [
        React.createElement('h3', {
          key: 'title',
          style: { margin: '0 0 15px 0', color: '#cccccc', fontSize: '16px' }
        }, 'New File'),
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: fileInputValue,
          onChange: (e: any) => setFileInputValue(e.target.value),
          onKeyDown: (e: any) => e.key === 'Enter' && handleFileInputConfirm(),
          autoFocus: true,
          style: {
            width: '100%',
            padding: '8px',
            background: '#3c3c3c',
            border: '1px solid #555',
            color: '#cccccc',
            borderRadius: '4px',
            marginBottom: '15px',
            boxSizing: 'border-box'
          }
        }),
        React.createElement('div', {
          key: 'buttons',
          style: { display: 'flex', justifyContent: 'flex-end', gap: '10px' }
        }, [
          React.createElement('button', {
            key: 'cancel',
            onClick: () => setShowFileInput(false),
            style: {
              padding: '6px 16px',
              background: '#3c3c3c',
              border: 'none',
              color: '#cccccc',
              borderRadius: '4px',
              cursor: 'pointer'
            }
          }, 'Cancel'),
          React.createElement('button', {
            key: 'create',
            onClick: handleFileInputConfirm,
            style: {
              padding: '6px 16px',
              background: '#007acc',
              border: 'none',
              color: '#fff',
              borderRadius: '4px',
              cursor: 'pointer'
            }
          }, 'Create')
        ])
      ])
    ])
  ])
}

export default App
