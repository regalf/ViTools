export interface EditorTab {
  id: string
  path: string | null
  name: string
  content: string
  language: string
  isModified: boolean
  isNew: boolean
}

export interface FileSystemItem {
  name: string
  path: string
  isDirectory: boolean
}

export interface OpenedFile {
  path: string
  name: string
  content: string
  isModified: boolean
  language: string
}

export interface ProjectConfig {
  name: string
  version: string
  description?: string
  language?: string
  compiler?: {
    command: string
    args: string[]
  }
  entryPoint?: string
  extensions?: string[]
  settings?: ProjectSettings
  exclude?: string[]
}

export interface ProjectSettings {
  tabSize?: number
  formatOnSave?: boolean
  theme?: string
}

export interface ElectronAPI {
  fs: {
    readFile: (path: string) => Promise<{ success: boolean; content?: string; error?: string }>
    writeFile: (path: string, content: string) => Promise<{ success: boolean; error?: string }>
    readDir: (path: string) => Promise<{ success: boolean; items?: FileSystemItem[]; error?: string }>
    stat: (path: string) => Promise<{ success: boolean; stats?: any; error?: string }>
    exists: (path: string) => Promise<boolean>
    move: (sourcePath: string, destPath: string) => Promise<{ success: boolean; error?: string }>
  }
  dialog: {
    openFile: () => Promise<string | null>
    openFolder: () => Promise<string | null>
    saveFile: (defaultPath?: string) => Promise<string | null>
  }
  project: {
    create: () => Promise<string | null>
  }
  folder: {
    create: (folderName: string, parentPath?: string) => Promise<string | null>
  }
  file: {
    create: (fileName: string, parentPath: string) => Promise<string | null>
  }
  terminal: {
    start: (cwd: string) => Promise<{ success: boolean; error?: string }>
    write: (data: string) => Promise<{ success: boolean }>
    resize: () => Promise<{ success: boolean }>
    onData: (callback: (data: string) => void) => void
    onExit: (callback: (exitCode: number) => void) => void
  }
  log: (message: string) => Promise<{ success: boolean }>
  onMenuAction: (callback: (action: string) => void) => void
}

declare global {
  interface Window {
    electronAPI: ElectronAPI
  }
}
