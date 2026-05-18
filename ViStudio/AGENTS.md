# ViStudio IDE - AI Agent Development Guide

## 📋 Overview

ViStudio è un IDE moderno costruito con Electron + React + TypeScript, ispirato a VS Code. Questo documento serve come guida completa per AI agent che devono modificare, estendere o debuggare il codebase.

---

## 🏗️ Architecture

### Stack Tecnologico
- **Runtime**: Electron 42 (system-installed su Arch Linux)
- **Frontend**: React 18 con TypeScript
- **Build Tool**: Vite 5
- **Editor Engine**: Monaco Editor 0.45 (lo stesso di VS Code)
- **Plugin System**: vite-plugin-electron

### Architettura a Livelli

```
┌─────────────────────────────────────────────┐
│           Electron Main Process             │
│  - Window management                        │
│  - IPC handlers (fs, dialog, project)       │
│  - Menu system                              │
└──────────────────┬──────────────────────────┘
                   │ IPC (contextBridge)
┌──────────────────┴──────────────────────────┐
│           Preload Script                    │
│  - electronAPI exposed to renderer          │
│  - Secure bridge main↔renderer              │
└──────────────────┬──────────────────────────┘
                   │
┌──────────────────┴──────────────────────────┐
│           React Renderer Process            │
│  - App.tsx (state management)               │
│  - Components (UI)                          │
│  - Monaco Editor (code editing)             │
└─────────────────────────────────────────────┘
```

---

## 📁 Project Structure

```
ViStudio/
├── electron/
│   ├── main.ts          # Processo principale Electron
│   │   ├── GPU fixes (commandLine switches)
│   │   ├── createWindow() - BrowserWindow config
│   │   ├── createMenu() - Menu nativo (nascosto)
│   │   └── IPC handlers:
│   │       - fs:readFile, fs:writeFile, fs:readDir, fs:stat, fs:exists
│   │       - dialog:openFile, dialog:openFolder, dialog:saveFile
│   │       └── project:create
│   └── preload.ts       # Bridge sicuro renderer↔main
│       └── electronAPI esposto: fs, dialog, project, onMenuAction
│
├── src/
│   ├── main.tsx         # Entry point React
│   ├── App.tsx          # Componente principale + state management
│   │   ├── States: sidebarOpen, folderPath, currentFile, fileContent, language, cursorPosition, isNewFile, projectConfig
│   │   ├── Handlers: handleOpenFolder, handleCreateProject, handleOpenFile, handleFileClick, handleNewFile, handleSave, handleSaveAs, handleMenuAction
│   │   └── Layout: MenuBar → [Sidebar + EditorPanel] → StatusBar
│   │
│   ├── components/
│   │   ├── MenuBar.tsx       # Barra menu personalizzata (File, Edit, View, Help)
│   │   │   ├── menuTemplate: definizione menu items
│   │   │   ├── Dropdown menus con hover/click
│   │   │   └── Actions: new-file, new-project, open-file, open-folder, save, save-as, toggle-sidebar, about
│   │   │
│   │   ├── Sidebar.tsx       # Sidebar laterale
│   │   │   ├── Header "EXPLORER"
│   │   │   ├── Pulsante "Open Folder"
│   │   │   └── ProjectExplorer component
│   │   │
│   │   ├── ProjectExplorer.tsx  # Albero file/cartelle
│   │   │   ├── FileNode interface: name, path, isDirectory, children, expanded
│   │   │   ├── FileTreeItem: componente ricorsivo per nodi
│   │   │   ├── loadDirectory(): carica file da fs.readDir
│   │   │   ├── toggleDirectory(): espande/collassa cartelle
│   │   │   ├── rootExpanded: stato cartella radice
│   │   │   ├── Rilevamento automatico .vistproj
│   │   │   └── Icone per tipo file (getIconForFile)
│   │   │
│   │   ├── EditorPanel.tsx   # Wrapper Monaco Editor
│   │   │   ├── Props: filePath, fileName, content, language, onChange, showWelcome
│   │   │   ├── Welcome screen quando nessun file aperto
│   │   │   └── Monaco Editor options (minimap, fontSize, theme, etc.)
│   │   │
│   │   └── StatusBar.tsx     # Barra di stato in basso
│   │       ├── Info: ViStudio logo, cursor position, encoding, line ending, language
│   │       └── Stile: background #007acc (blu)
│   │
│   ├── styles/
│   │   └── global.css        # Stili globali + scrollbar scura
│   │
│   └── types/
│       └── index.ts          # TypeScript interfaces
│           ├── FileSystemItem, OpenedFile, ProjectConfig, ProjectSettings
│           └── ElectronAPI interface
│
├── schemas/
│   └── vistproj.template.json  # Template formato progetto
│
├── public/
│   └── vite.svg              # Favicon
│
├── index.html                # HTML entry point
├── package.json              # Dependencies e scripts
├── tsconfig.json             # TypeScript config
├── vite.config.ts            # Vite + Electron config
└── run.sh                    # Script di avvio sviluppo
```

---

## 🔑 Key Components Deep Dive

### 1. App.tsx - State Management

**Stati principali:**
```typescript
sidebarOpen: boolean          // Sidebar visibile/nascosta
folderPath: string | null     // Percorso cartella aperta
currentFile: string | null    // File attualmente aperto
fileContent: string           // Contenuto del file
language: string              // Linguaggio per syntax highlighting
cursorPosition: {line, column}// Posizione cursore
isNewFile: boolean            // File nuovo non salvato
projectConfig: any            // Configurazione .vistproj
```

**Flusso dati:**
```
User Action → Handler → setState → Re-render → UI Update
```

### 2. ProjectExplorer.tsx - File Tree

**Algoritmo di caricamento:**
1. `folderPath` cambia → useEffect trigger
2. `loadDirectory(folderPath)` → chiama `electronAPI.fs.readDir`
3. Filtra: no file nascosti (.), no node_modules
4. Ordina: cartelle prima, poi alfabetico
5. Setta `fileTree` state
6. Controlla `.vistproj` → parse JSON → `projectConfig`

**Componente ricorsivo FileTreeItem:**
- Renderizza ogni nodo (file o cartella)
- Cartelle: cliccabili per espandere/collassare
- File: cliccabili per aprire (onFileClick)
- Depth-based indentation (depth * 16px)

### 3. MenuBar.tsx - Custom Menu

**Struttura menu:**
```typescript
menuTemplate = [
  { label: 'File', items: [New File, New Project, Open File, Open Folder, Save, Save As, Exit] },
  { label: 'Edit', items: [Undo, Redo, Cut, Copy, Paste, Find, Replace] },
  { label: 'View', items: [Toggle Sidebar, Toggle Terminal, Command Palette, Zoom...] },
  { label: 'Help', items: [About] }
]
```

**Interazioni:**
- Click su label → apre dropdown
- Hover su menu attivo → cambia menu
- Click outside → chiude dropdown
- Click su item → `onAction(action)` → App.tsx handler

### 4. EditorPanel.tsx - Monaco Editor

**Configurazione Monaco:**
```typescript
options: {
  minimap: { enabled: true },
  fontSize: 14,
  fontFamily: "'Fira Code', 'Cascadia Code', 'Consolas', monospace",
  fontLigatures: true,
  automaticLayout: true,
  bracketPairColorization: { enabled: true },
  guides: { bracketPairs: true, indentation: true },
  tabSize: 2,
  // ... altre opzioni
}
```

**Welcome Screen:**
- Mostrato quando `showWelcome = true` (nessun file aperto e non isNewFile)
- Titolo "ViStudio", sottotitolo, shortcut hints

---

## 🔌 IPC Communication

### Main Process → Renderer

**Menu Actions:**
```typescript
mainWindow.webContents.send('menu:new-file')
mainWindow.webContents.send('menu:open-file')
// ... altre actions
```

### Renderer → Main (via electronAPI)

**File System:**
```typescript
window.electronAPI.fs.readFile(path)     // → {success, content, error}
window.electronAPI.fs.writeFile(path, content)  // → {success, error}
window.electronAPI.fs.readDir(path)      // → {success, items[], error}
window.electronAPI.fs.stat(path)         // → {success, stats, error}
window.electronAPI.fs.exists(path)       // → boolean
```

**Dialogs:**
```typescript
window.electronAPI.dialog.openFile()     // → string | null
window.electronAPI.dialog.openFolder()   // → string | null
window.electronAPI.dialog.saveFile(defaultPath?)  // → string | null
```

**Project:**
```typescript
window.electronAPI.project.create()      // → string | null (project path)
```

**Menu Listener:**
```typescript
const cleanup = window.electronAPI.onMenuAction((action) => {
  // handle action
})
return cleanup  // cleanup on unmount
```

---

## 📄 .vistproj Format

```json
{
  "name": "MyProject",
  "version": "1.0.0",
  "description": "Project description",
  "language": "typescript",
  "compiler": {
    "command": "tsc",
    "args": ["--outDir", "./dist", "--sourceMap"]
  },
  "entryPoint": "src/main.ts",
  "extensions": [],
  "settings": {
    "tabSize": 2,
    "formatOnSave": true,
    "theme": "vs-dark"
  },
  "exclude": ["node_modules", "dist", ".git"]
}
```

**Creazione progetto (project:create IPC):**
1. Dialog per scegliere cartella padre
2. Input box per nome progetto
3. Crea cartella: `parentDir/projectName`
4. Crea sottocartella: `src/`
5. Scrive `.vistproj` con template
6. Scrive `src/main.ts` vuoto
7. Restituisce percorso progetto

---

## 🎨 Theming & Styling

### Color Palette
```css
--bg-primary: #1e1e1e      /* Sfondo principale */
--bg-secondary: #252526    /* Sidebar */
--bg-tertiary: #2d2d30     /* Elementi UI */
--bg-hover: #2a2d2e        /* Hover state */
--text-primary: #cccccc    /* Testo principale */
--text-secondary: #858585  /* Testo secondario */
--text-active: #ffffff     /* Testo attivo */
--border-color: #3c3c3c    /* Bordi */
--accent-color: #007acc    /* Blu ViStudio (status bar, bottoni) */
```

### Scrollbar Styling
```css
::-webkit-scrollbar { width: 10px; }
::-webkit-scrollbar-track { background: #1e1e1e; }
::-webkit-scrollbar-thumb { background: #424242; border-radius: 5px; }
::-webkit-scrollbar-thumb:hover { background: #4f4f4f; }
```

---

## 🚀 Development Workflow

### Avvio Sviluppo
```bash
./run.sh
# Oppure:
npm run electron:dev
```

**Cosa fa run.sh:**
1. Kill processi esistenti (node, vite, electron)
2. Avvia Vite dev server su porta 5173
3. Attende che Vite sia ready
4. Avvia Electron con `VITE_DEV_SERVER_URL=http://localhost:5173`
5. Electron carica URL dev server

### Hot Reload
- **React components**: Vite HMR aggiorna automaticamente
- **Electron main/preload**: richiede restart (vite-plugin-electron rebuild)

### Debug
- **DevTools**: aperte automaticamente in dev mode
- **Console logs**: `[RENDERER]` prefix per logs dal renderer
- **Main process logs**: visibili nel terminale

---

## 🛠️ Common Tasks

### Aggiungere un nuovo menu item
1. Aggiungi in `menuTemplate` in `MenuBar.tsx`
2. Aggiungi case in `handleMenuAction` in `App.tsx`
3. Implementa handler function

### Aggiungere un nuovo IPC handler
1. Aggiungi `ipcMain.handle()` in `electron/main.ts`
2. Esponi in `electron/preload.ts` via `contextBridge`
3. Aggiungi tipo in `src/types/index.ts`
4. Usa in React component via `window.electronAPI`

### Aggiungere un nuovo componente
1. Crea file in `src/components/`
2. Esporta come default
3. Importa in `App.tsx` o componente padre
4. Aggiungi nel JSX tree

### Modificare lo stile
1. Stili inline: nel componente React
2. Stili globali: `src/styles/global.css`
3. CSS modules: crea `.css` accanto al componente

---

## ⚠️ Known Issues & Workarounds

### Electron Binary
- Electron è installato a livello di sistema (`/usr/bin/electron`)
- `node_modules/electron/dist/` contiene symlink/copie dei binari
- Se Electron non si avvia: reinstalla con `pkexec pacman -S electron`

### GPU Issues
- Hardware acceleration disabilitata per evitare crash GPU
- Command line switches in `main.ts`:
  ```typescript
  app.commandLine.appendSwitch('no-sandbox')
  app.commandLine.appendSwitch('disable-gpu-compositing')
  app.disableHardwareAcceleration()
  ```

### Port Conflicts
- Vite usa porta 5173
- Se occupata, Vite usa porta successiva (5174, etc.)
- `run.sh` kill processi esistenti prima di avviare

---

## 📝 Code Conventions

### React Components
- Functional components con TypeScript
- `React.createElement()` invece di JSX (per compatibilità)
- Props interfaces definite
- useCallback/useMemo per performance

### Naming
- Components: PascalCase (`ProjectExplorer`)
- Files: PascalCase per components, camelCase per altri
- Handlers: `handle` prefix (`handleOpenFolder`)
- States: camelCase (`folderPath`, `fileContent`)

### Error Handling
- Try/catch intorno a chiamate async
- Console.error per errori
- Graceful degradation (fallback UI)

---

## 🔮 Future Extensions (Planned)

### Fase 3: Tab Multipli
- Array di file aperti
- Tab bar component
- Switch tra tab
- Stato "modificato" (dot sul tab)

### Fase 4: Extension API
- Extension host sandboxato
- API per: syntax highlighting, autocompletamento, compilatori
- Formato estensione: `.vix` package

### Fase 5: Terminale Integrato
- xterm.js per terminale
- Panel in basso
- Comandi build/run

### Fase 6: Command Palette
- Ctrl+Shift+P
- Fuzzy search comandi
- Quick open file

### Fase 7: Search/Replace Globale
- Search panel
- Regex support
- Replace in files

---

## 📞 Quick Reference

| Task | File | Function/Component |
|------|------|-------------------|
| Apri cartella | App.tsx | handleOpenFolder |
| Crea progetto | electron/main.ts | ipcMain.handle('project:create') |
| Leggi file | electron/main.ts | ipcMain.handle('fs:readFile') |
| Menu actions | MenuBar.tsx | menuTemplate |
| File tree | ProjectExplorer.tsx | loadDirectory, FileTreeItem |
| Editor | EditorPanel.tsx | Monaco Editor wrapper |
| Status bar | StatusBar.tsx | Info display |
| Sidebar | Sidebar.tsx | Layout container |
| Types | src/types/index.ts | TypeScript interfaces |
| Styles | src/styles/global.css | Global CSS |
| Config | vite.config.ts | Vite + Electron setup |
| Launch | run.sh | Development script |

---

## 🧪 Testing Checklist

Quando modifichi il codice, verifica:
- [ ] App si avvia senza errori (`./run.sh`)
- [ ] Menu bar funziona (click, dropdown)
- [ ] Open Folder carica file nell'explorer
- [ ] New Project crea cartella con .vistproj
- [ ] Click su file apre nell'editor
- [ ] Monaco Editor mostra syntax highlighting
- [ ] Save/Save As funzionano
- [ ] Scrollbar è scura
- [ ] Cartella radice è collassabile
- [ ] DevTools non mostrano errori

---

*Ultimo aggiornamento: 2026-05-18*
*Versione: 0.2.0 (Fase 2 completata)*
