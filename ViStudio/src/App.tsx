import React, { useState, useEffect, useCallback } from 'react'
import MenuBar from './components/MenuBar'
import Sidebar from './components/Sidebar'
import EditorPanel from './components/EditorPanel'
import StatusBar from './components/StatusBar'
import './styles/global.css'

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
  const [currentFile, setCurrentFile] = useState<string | null>(null)
  const [fileContent, setFileContent] = useState('')
  const [language, setLanguage] = useState('plaintext')
  const [cursorPosition, setCursorPosition] = useState({ line: 1, column: 1 })
  const [isNewFile, setIsNewFile] = useState(false)
  const [projectConfig, setProjectConfig] = useState<any>(null)

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

  const handleOpenFile = useCallback(async () => {
    try {
      if (window.electronAPI) {
        const path = await window.electronAPI.dialog.openFile()
        if (path) {
          const result = await window.electronAPI.fs.readFile(path)
          if (result.success && result.content !== undefined) {
            setCurrentFile(path)
            setFileContent(result.content)
            setLanguage(getLanguageFromExtension(path))
            setIsNewFile(false)
          }
        }
      }
    } catch (e) {
      console.error('Failed to open file:', e)
    }
  }, [])

  const handleFileClick = useCallback(async (path: string) => {
    try {
      if (window.electronAPI) {
        const result = await window.electronAPI.fs.readFile(path)
        if (result.success && result.content !== undefined) {
          setCurrentFile(path)
          setFileContent(result.content)
          setLanguage(getLanguageFromExtension(path))
          setIsNewFile(false)
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
    setCurrentFile(null)
    setFileContent('')
    setLanguage('plaintext')
    setIsNewFile(true)
  }, [])

  const handleSave = useCallback(async () => {
    try {
      if (!window.electronAPI) return
      if (!currentFile) {
        const defaultPath = folderPath ? `${folderPath}/untitled` : undefined
        const path = await window.electronAPI.dialog.saveFile(defaultPath)
        if (path) {
          await window.electronAPI.fs.writeFile(path, fileContent)
          setCurrentFile(path)
          setLanguage(getLanguageFromExtension(path))
          setIsNewFile(false)
        }
      } else {
        await window.electronAPI.fs.writeFile(currentFile, fileContent)
      }
    } catch (e) {
      console.error('Failed to save:', e)
    }
  }, [currentFile, fileContent, folderPath])

  const handleSaveAs = useCallback(async () => {
    try {
      if (window.electronAPI) {
        const defaultPath = folderPath ? `${folderPath}/` : undefined
        const path = await window.electronAPI.dialog.saveFile(defaultPath)
        if (path) {
          await window.electronAPI.fs.writeFile(path, fileContent)
          setCurrentFile(path)
          setLanguage(getLanguageFromExtension(path))
          setIsNewFile(false)
        }
      }
    } catch (e) {
      console.error('Failed to save as:', e)
    }
  }, [fileContent, folderPath])

  const handleMenuAction = useCallback(async (action: string) => {
    console.log('Menu action:', action)
    switch (action) {
      case 'menu:new-file':
        await handleNewFile()
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
      case 'menu:about':
        alert('ViStudio IDE v0.1.0\nA modern, extensible code editor')
        break
    }
  }, [handleNewFile, handleCreateProject, handleOpenFile, handleOpenFolder, handleSave, handleSaveAs])

  const handleEditorChange = useCallback((value: string) => {
    setFileContent(value)
  }, [])

  useEffect(() => {
    if (window.electronAPI) {
      const cleanup = window.electronAPI.onMenuAction((action) => {
        handleMenuAction(action)
      })
      return cleanup
    }
  }, [handleMenuAction])

  const displayName = isNewFile ? 'Untitled' : (currentFile ? currentFile.split('/').pop() || 'Untitled' : null)

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
        onOpenFolder: handleOpenFolder,
        onFileClick: handleFileClick,
        onProjectLoad: handleProjectLoad
      }),
      React.createElement(EditorPanel, {
        key: 'editor',
        filePath: currentFile,
        fileName: displayName,
        content: fileContent,
        language: language,
        onChange: handleEditorChange,
        showWelcome: !isNewFile && !currentFile
      })
    ]),
    React.createElement(StatusBar, {
      key: 'statusbar',
      filePath: currentFile || (isNewFile ? 'Untitled' : null),
      language: language,
      cursorPosition: cursorPosition
    })
  ])
}

export default App
