import { contextBridge, ipcRenderer } from 'electron'

try {
  contextBridge.exposeInMainWorld('electronAPI', {
    fs: {
      readFile: (filePath: string) => ipcRenderer.invoke('fs:readFile', filePath),
      writeFile: (filePath: string, content: string) => ipcRenderer.invoke('fs:writeFile', filePath, content),
      readDir: (dirPath: string) => ipcRenderer.invoke('fs:readDir', dirPath),
      stat: (filePath: string) => ipcRenderer.invoke('fs:stat', filePath),
      exists: (filePath: string) => ipcRenderer.invoke('fs:exists', filePath)
    },
  dialog: {
    openFile: () => ipcRenderer.invoke('dialog:openFile'),
    openFolder: () => ipcRenderer.invoke('dialog:openFolder'),
    saveFile: (defaultPath?: string) => ipcRenderer.invoke('dialog:saveFile', defaultPath)
  },
  project: {
    create: () => ipcRenderer.invoke('project:create')
  },
    onMenuAction: (callback: (action: string) => void) => {
      const actions = ['menu:new-file', 'menu:open-file', 'menu:open-folder', 'menu:save', 'menu:save-as', 'menu:find', 'menu:replace', 'menu:toggle-sidebar', 'menu:toggle-terminal', 'menu:command-palette']
      const listeners: Record<string, any> = {}
      actions.forEach(action => {
        listeners[action] = () => callback(action)
        ipcRenderer.on(action, listeners[action])
      })
      return () => {
        actions.forEach(action => {
          if (listeners[action]) {
            ipcRenderer.removeListener(action, listeners[action])
          }
        })
      }
    }
  })
} catch (error) {
  console.error('Failed to expose electronAPI:', error)
}
