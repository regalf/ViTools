import { app, BrowserWindow, ipcMain, dialog, Menu } from 'electron'
import { join } from 'path'
import { readFileSync, writeFileSync, statSync, readdirSync, existsSync, mkdirSync } from 'fs'

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

  mainWindow.webContents.on('did-fail-load', (event, errorCode, errorDescription) => {
    console.error('Failed to load:', errorCode, errorDescription)
  })

  mainWindow.webContents.on('did-finish-load', () => {
    console.log('Page loaded successfully')
  })

  if (process.env.VITE_DEV_SERVER_URL) {
    console.log('Loading dev server:', process.env.VITE_DEV_SERVER_URL)
    mainWindow.loadURL(process.env.VITE_DEV_SERVER_URL)
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
    const { response, filePath } = await dialog.showInputBox(mainWindow!, {
      title: 'Project Name',
      prompt: 'Enter the name for your new project',
      defaultText: 'MyProject'
    })
    if (response !== 0 || !filePath) return null

    const projectName = filePath.trim()
    if (!projectName) return null

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
