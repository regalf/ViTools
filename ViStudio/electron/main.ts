import { app, BrowserWindow, ipcMain, dialog, Menu } from 'electron'
import { join } from 'path'
import { readFileSync, writeFileSync, statSync, readdirSync, existsSync, mkdirSync, renameSync } from 'fs'
import { spawn } from 'child_process'

// GPU fixes - must be set before app ready
app.commandLine.appendSwitch('no-sandbox')
app.commandLine.appendSwitch('disable-gpu-compositing')
app.commandLine.appendSwitch('enable-unsafe-webgpu')
app.commandLine.appendSwitch('disable-dev-shm-usage')
app.commandLine.appendSwitch('ozone-platform-hint', 'auto')
app.disableHardwareAcceleration()

let mainWindow: BrowserWindow | null = null

function createMenu() {
  const template: Electron.MenuItemConstructorOptions[] = [
    {
      label: 'File',
      submenu: [
        { label: 'New File', accelerator: 'CmdOrCtrl+N', click: () => mainWindow?.webContents.send('menu:new-file') },
        { label: 'Open File', accelerator: 'CmdOrCtrl+O', click: () => mainWindow?.webContents.send('menu:open-file') },
        { label: 'Open Folder', accelerator: 'CmdOrCtrl+Shift+O', click: () => mainWindow?.webContents.send('menu:open-folder') },
        { type: 'separator' },
        { label: 'Save', accelerator: 'CmdOrCtrl+S', click: () => mainWindow?.webContents.send('menu:save') },
        { label: 'Save As', accelerator: 'CmdOrCtrl+Shift+S', click: () => mainWindow?.webContents.send('menu:save-as') },
        { type: 'separator' },
        { role: 'quit' }
      ]
    },
    {
      label: 'Edit',
      submenu: [
        { role: 'undo' },
        { role: 'redo' },
        { type: 'separator' },
        { role: 'cut' },
        { role: 'copy' },
        { role: 'paste' },
        { type: 'separator' },
        { label: 'Find', accelerator: 'CmdOrCtrl+F', click: () => mainWindow?.webContents.send('menu:find') },
        { label: 'Replace', accelerator: 'CmdOrCtrl+H', click: () => mainWindow?.webContents.send('menu:replace') }
      ]
    },
    {
      label: 'View',
      submenu: [
        { label: 'Toggle Sidebar', accelerator: 'CmdOrCtrl+B', click: () => mainWindow?.webContents.send('menu:toggle-sidebar') },
        { label: 'Toggle Terminal', accelerator: 'Ctrl+`', click: () => mainWindow?.webContents.send('menu:toggle-terminal') },
        { label: 'Command Palette', accelerator: 'CmdOrCtrl+Shift+P', click: () => mainWindow?.webContents.send('menu:command-palette') },
        { type: 'separator' },
        { role: 'reload' },
        { role: 'toggleDevTools' },
        { type: 'separator' },
        { role: 'zoomIn' },
        { role: 'zoomOut' },
        { role: 'resetZoom' }
      ]
    },
    {
      label: 'Help',
      submenu: [
        { label: 'About ViStudio', click: () => dialog.showMessageBox({ title: 'ViStudio', message: 'ViStudio IDE v0.1.0', detail: 'A modern, extensible code editor' }) }
      ]
    }
  ]

  const menu = Menu.buildFromTemplate(template)
  Menu.setApplicationMenu(menu)
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 800,
    minHeight: 600,
    title: 'ViStudio',
    frame: true,
    autoHideMenuBar: true,
    backgroundColor: '#1e1e1e',
    show: true,
    webPreferences: {
      preload: join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
      webSecurity: false,
      sandbox: false
    }
  })

  mainWindow.setMenuBarVisibility(false)

  mainWindow.webContents.on('did-fail-load', (_event, errorCode, errorDescription) => {
    console.error('Failed to load:', errorCode, errorDescription)
  })

  mainWindow.webContents.on('did-finish-load', () => {
    console.log('Page loaded successfully')
  })

  if (process.env.VITE_DEV_SERVER_URL) {
    console.log('Loading dev server:', process.env.VITE_DEV_SERVER_URL)
    mainWindow.loadURL(process.env.VITE_DEV_SERVER_URL)
    mainWindow.webContents.openDevTools()
  } else {
    console.log('Loading production build')
    mainWindow.loadFile(join(__dirname, '../dist/index.html'))
  }

  mainWindow.on('closed', () => {
    mainWindow = null
  })
}

// IPC Handlers
ipcMain.handle('fs:readFile', async (_, filePath: string) => {
  try {
    const content = readFileSync(filePath, 'utf-8')
    return { success: true, content }
  } catch (error: any) {
    return { success: false, error: error.message }
  }
})

ipcMain.handle('fs:writeFile', async (_, filePath: string, content: string) => {
  try {
    writeFileSync(filePath, content, 'utf-8')
    return { success: true }
  } catch (error: any) {
    return { success: false, error: error.message }
  }
})

ipcMain.handle('fs:readDir', async (_, dirPath: string) => {
  try {
    const items = readdirSync(dirPath, { withFileTypes: true })
    const result = items.map(item => ({
      name: item.name,
      isDirectory: item.isDirectory(),
      path: join(dirPath, item.name)
    }))
    return { success: true, items: result }
  } catch (error: any) {
    return { success: false, error: error.message }
  }
})

ipcMain.handle('fs:stat', async (_, filePath: string) => {
  try {
    const stats = statSync(filePath)
    return { success: true, stats: { isDirectory: stats.isDirectory(), size: stats.size, modified: stats.mtime } }
  } catch (error: any) {
    return { success: false, error: error.message }
  }
})

ipcMain.handle('fs:exists', async (_, filePath: string) => {
  return existsSync(filePath)
})

ipcMain.handle('fs:move', async (_, sourcePath: string, destPath: string) => {
  try {
    if (!existsSync(sourcePath)) {
      return { success: false, error: 'Source file not found' }
    }
    if (existsSync(destPath)) {
      return { success: false, error: 'Destination already exists' }
    }
    renameSync(sourcePath, destPath)
    return { success: true }
  } catch (error: any) {
    return { success: false, error: error.message }
  }
})

ipcMain.handle('dialog:openFile', async () => {
  const result = await dialog.showOpenDialog(mainWindow!, {
    properties: ['openFile'],
    filters: [{ name: 'All Files', extensions: ['*'] }]
  })
  return result.canceled ? null : result.filePaths[0]
})

ipcMain.handle('dialog:openFolder', async () => {
  const result = await dialog.showOpenDialog(mainWindow!, {
    properties: ['openDirectory']
  })
  return result.canceled ? null : result.filePaths[0]
})

ipcMain.handle('dialog:saveFile', async (_, defaultPath?: string) => {
  const result = await dialog.showSaveDialog(mainWindow!, {
    defaultPath,
    filters: [{ name: 'All Files', extensions: ['*'] }]
  })
  return result.canceled ? null : result.filePath
})

ipcMain.handle('project:create', async () => {
  try {
    const result = await dialog.showOpenDialog(mainWindow!, {
      properties: ['openDirectory', 'createDirectory'],
      title: 'Choose where to create the project'
    })
    if (result.canceled || !result.filePaths[0]) return null

    const parentDir = result.filePaths[0]
    const { response } = await dialog.showMessageBox(mainWindow!, {
      type: 'question',
      buttons: ['Create', 'Cancel'],
      defaultId: 0,
      cancelId: 1,
      title: 'Project Name',
      message: 'Enter the name for your new project',
      detail: 'Default: MyProject',
      noLink: true
    })
    
    const projectName = 'MyProject'
    if (response !== 0) return null

    const projectDir = join(parentDir, projectName)
    if (existsSync(projectDir)) {
      await dialog.showMessageBox(mainWindow!, {
        type: 'error',
        title: 'Error',
        message: `Folder "${projectName}" already exists`
      })
      return null
    }

    mkdirSync(projectDir, { recursive: true })
    mkdirSync(join(projectDir, 'src'), { recursive: true })

    const vistprojContent = JSON.stringify({
      name: projectName,
      version: '1.0.0',
      description: `${projectName} - Created with ViStudio`,
      language: 'typescript',
      compiler: {
        command: 'tsc',
        args: ['--outDir', './dist', '--sourceMap']
      },
      entryPoint: 'src/main.ts',
      extensions: [],
      settings: {
        tabSize: 2,
        formatOnSave: true,
        theme: 'vs-dark'
      },
      exclude: ['node_modules', 'dist', '.git']
    }, null, 2)

    writeFileSync(join(projectDir, '.vistproj'), vistprojContent, 'utf-8')
    writeFileSync(join(projectDir, 'src', 'main.ts'), '// Main entry point\n', 'utf-8')

    return projectDir
  } catch (error: any) {
    console.error('Failed to create project:', error)
    return null
  }
})

ipcMain.handle('folder:create', async (_, folderName: string, parentPath?: string) => {
  try {
    if (!folderName) return null

    const targetPath = parentPath || (await dialog.showOpenDialog(mainWindow!, {
      properties: ['openDirectory'],
      title: 'Select parent folder'
    })).filePaths[0]

    if (!targetPath) return null

    const newFolderPath = join(targetPath, folderName)
    if (existsSync(newFolderPath)) {
      await dialog.showMessageBox(mainWindow!, {
        type: 'error',
        title: 'Error',
        message: `Folder "${folderName}" already exists`
      })
      return null
    }

    mkdirSync(newFolderPath, { recursive: true })
    return newFolderPath
  } catch (error: any) {
    console.error('Failed to create folder:', error)
    return null
  }
})

ipcMain.handle('file:create', async (_, fileName: string, parentPath: string) => {
  try {
    if (!fileName || !parentPath) return null
    const newFilePath = join(parentPath, fileName)
    if (existsSync(newFilePath)) {
      await dialog.showMessageBox(mainWindow!, {
        type: 'error',
        title: 'Error',
        message: `File "${fileName}" already exists`
      })
      return null
    }
    writeFileSync(newFilePath, '', 'utf-8')
    return newFilePath
  } catch (error: any) {
    console.error('Failed to create file:', error)
    return null
  }
})

// Terminal IPC Handlers
let terminalProcess: ReturnType<typeof spawn> | null = null

ipcMain.handle('terminal:start', async (_, cwd: string) => {
  try {
    console.log('[TERMINAL] ========== STARTING TERMINAL ==========')
    console.log('[TERMINAL] CWD:', cwd)
    console.log('[TERMINAL] Shell:', process.env.SHELL)
    
    if (terminalProcess) {
      console.log('[TERMINAL] Killing existing process')
      terminalProcess.kill()
    }

    const shell = process.platform === 'win32' ? 'powershell.exe' : '/bin/bash'
    console.log('[TERMINAL] Using shell:', shell)
    
    // Use 'script' to create a pseudo-terminal on Linux/Mac
    const args = process.platform === 'win32' 
      ? [] 
      : ['-q', '-c', shell, '/dev/null']
    
    const command = process.platform === 'win32' ? shell : 'script'
    console.log('[TERMINAL] Command:', command, 'Args:', args)

    terminalProcess = spawn(command, args, {
      cwd: cwd || process.env.HOME,
      env: { ...process.env, TERM: 'xterm-256color' },
      stdio: ['pipe', 'pipe', 'pipe']
    })

    console.log('[TERMINAL] Process spawned with PID:', terminalProcess.pid)

    terminalProcess.stdout?.on('data', (data) => {
      const text = data.toString()
      console.log('[TERMINAL STDOUT]', JSON.stringify(text))
      mainWindow?.webContents.send('terminal:data', text)
    })

    terminalProcess.stderr?.on('data', (data) => {
      const text = data.toString()
      console.log('[TERMINAL STDERR]', JSON.stringify(text))
      mainWindow?.webContents.send('terminal:data', text)
    })

    terminalProcess.on('exit', (code) => {
      console.log('[TERMINAL] Process exited with code:', code)
      mainWindow?.webContents.send('terminal:exit', code)
      terminalProcess = null
    })

    terminalProcess.on('error', (err) => {
      console.error('[TERMINAL ERROR]', err)
      mainWindow?.webContents.send('terminal:data', `\r\nError: ${err.message}\r\n`)
    })

    // Send initial welcome message to test output
    setTimeout(() => {
      if (terminalProcess) {
        console.log('[TERMINAL] Sending initial prompt test')
        mainWindow?.webContents.send('terminal:data', 'ViStudio Terminal Ready\r\n$ ')
      }
    }, 500)

    return { success: true }
  } catch (error: any) {
    console.error('[TERMINAL] Failed to start:', error)
    return { success: false, error: error.message }
  }
})

ipcMain.handle('terminal:write', async (_, data: string) => {
  console.log('[TERMINAL WRITE]', JSON.stringify(data))
  if (terminalProcess && terminalProcess.stdin) {
    terminalProcess.stdin.write(data)
    console.log('[TERMINAL] Data written to stdin')
  } else {
    console.log('[TERMINAL] No process or stdin available')
  }
  return { success: true }
})

ipcMain.handle('terminal:resize', async () => {
  return { success: true }
})

// Logger IPC for debugging renderer issues
ipcMain.handle('log', async (_, message: string) => {
  console.log('[RENDERER LOG]', message)
  return { success: true }
})

app.whenReady().then(() => {
  createMenu()
  createWindow()

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow()
    }
  })
})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})
