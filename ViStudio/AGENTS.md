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
- **Terminal Engine**: xterm.js + `child_process` (script PTY)
- **Plugin System**: vite-plugin-electron

### Architettura a Livelli

```
┌─────────────────────────────────────────────┐
│           Electron Main Process             │
│  - Window management                        │
│  - IPC handlers (fs, dialog, project, term) │
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
│  - xterm.js (terminal)                      │
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
│   │       - fs:readFile, fs:writeFile, fs:readDir, fs:stat, fs:exists, fs:move
│   │       - dialog:openFile, dialog:openFolder, dialog:saveFile
│   │       - project:create
│   │       - folder:create, file:create
│   │       └── terminal:start, terminal:write, terminal:resize
│   └── preload.ts       # Bridge sicuro renderer↔main
│       └── electronAPI esposto: fs, dialog, project, folder, file, terminal, onMenuAction
│
├── src/
│   ├── main.tsx         # Entry point React
│   ├── App.tsx          # Componente principale + state management
│   │   ├── States: sidebarOpen, folderPath, tabs[], activeTabId, terminalVisible, ...
│   │   ├── Handlers: handleOpenFolder, handleFileClick, handleSave, ...
│   │   └── Layout: MenuBar → [Sidebar + (TabBar + EditorPanel + TerminalPanel)] → StatusBar
│   │
│   ├── components/
│   │   ├── MenuBar.tsx       # Barra menu personalizzata
│   │   ├── Sidebar.tsx       # Sidebar laterale
│   │   ├── ProjectExplorer.tsx  # Albero file/cartelle con drag & drop
│   │   ├── EditorPanel.tsx   # Wrapper Monaco Editor
│   │   ├── TabBar.tsx        # Barra tab multipli (reorder, close, modified)
│   │   ├── TerminalPanel.tsx # Wrapper xterm.js (bash shell)
│   │   └── StatusBar.tsx     # Barra di stato in basso
│   │
│   ├── styles/
│   │   └── global.css        # Stili globali + scrollbar scura
│   │
│   └── types/
│       └── index.ts          # TypeScript interfaces
│           ├── EditorTab, FileSystemItem, ProjectConfig
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
├── vite.config.ts            # Vite + Electron config (external: ['electron', 'node-pty'])
└── run.sh                    # Script di avvio sviluppo
```

---

## 🔑 Key Components Deep Dive

### 1. App.tsx - State Management

**Stati principali:**
```typescript
sidebarOpen: boolean          // Sidebar visibile/nascosta
folderPath: string | null     // Percorso cartella aperta
tabs: EditorTab[]             // Array di tab aperti
activeTabId: string | null    // ID del tab attivo
terminalVisible: boolean      // Terminale visibile/nascosto
refreshPath: string | null    // Trigger per refresh explorer
// ... modali states
```

**Flusso dati:**
```
User Action → Handler → setState → Re-render → UI Update
```

### 2. ProjectExplorer.tsx - File Tree con Drag & Drop

**Algoritmo di caricamento:**
1. `folderPath` cambia → useEffect trigger
2. `loadDirectory(folderPath)` → chiama `electronAPI.fs.readDir`
3. Filtra: no file nascosti (.), no node_modules
4. Ordina: cartelle prima, poi alfabetico
5. Setta `fileTree` state
6. Controlla `.vistproj` → parse JSON → `projectConfig`

**Drag & Drop Implementation:**
- `onDragStart`: setta `dataTransfer.setData('text/plain', node.path)` e `draggedNode`
- `onDragOver`: `e.preventDefault()`, `e.dataTransfer.dropEffect = 'move'`
- `onDrop`: legge `e.dataTransfer.getData('text/plain')`, chiama `handleDrop`
- `handleDrop`: chiama `electronAPI.fs.move(sourcePath, destPath)`, poi aggiorna albero selettivamente
- Feedback visivo: bordo tratteggiato blu su cartelle target, opacità ridotta su elemento trascinato

### 3. TabBar.tsx - Multi-Tab Management

**Funzionalità:**
- Visualizza lista tab aperti
- Click per switchare tab attivo
- Drag & Drop per riordinare tab
- "×" per chiudere tab (appare su hover)
- Pallino bianco per file modificati non salvati

### 4. TerminalPanel.tsx - Integrated Terminal

**Tecnologia:**
- **xterm.js**: Rendering terminale (stesso di VS Code)
- **FitAddon**: Auto-resize
- **WebLinksAddon**: Link cliccabili
- **Backend**: `child_process` + `script` (pseudo-terminale) per evitare moduli nativi problematici
- **Shell**: Forzata a `/bin/bash` per compatibilità

**Flusso:**
1. Componente montato → `initTerminal()`
2. Crea istanza `Terminal` → `term.open(ref)`
3. Chiama `electronAPI.terminal.start(cwd)`
4. Listener `onData` riceve output dal main process → `term.write(data)`
5. Input utente → `term.onData` → `electronAPI.terminal.write(data)`

### 5. EditorPanel.tsx - Monaco Editor

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
}
```

---

## 🔌 IPC Communication

### Main Process → Renderer

**Menu Actions:**
```typescript
mainWindow.webContents.send('menu:new-file')
// ...
```

**Terminal Events:**
```typescript
mainWindow.webContents.send('terminal:data', data)
mainWindow.webContents.send('terminal:exit', exitCode)
```

### Renderer → Main (via electronAPI)

**File System:**
```typescript
window.electronAPI.fs.readFile(path)
window.electronAPI.fs.writeFile(path, content)
window.electronAPI.fs.readDir(path)
window.electronAPI.fs.stat(path)
window.electronAPI.fs.exists(path)
window.electronAPI.fs.move(source, dest)
```

**Terminal:**
```typescript
window.electronAPI.terminal.start(cwd)      // Avvia shell
window.electronAPI.terminal.write(data)     // Invia input
window.electronAPI.terminal.resize(cols, rows) // Resize
window.electronAPI.terminal.onData(callback) // Ricevi output
window.electronAPI.terminal.onExit(callback) // Gestione uscita
```

**Dialogs:**
```typescript
window.electronAPI.dialog.openFile()
window.electronAPI.dialog.openFolder()
window.electronAPI.dialog.saveFile(defaultPath?)
```

**Project/File/Folder:**
```typescript
window.electronAPI.project.create()
window.electronAPI.folder.create(name, parentPath?)
window.electronAPI.file.create(name, parentPath)
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
- **DevTools**: aperte automaticamente in dev mode (`mainWindow.webContents.openDevTools()`)
- **Console logs**: `[RENDERER]` prefix per logs dal renderer
- **Main process logs**: visibili nel terminale e in `/tmp/vistudio.log`

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

### Drag & Drop
- Il drag & drop usa `dataTransfer.setData` per passare il percorso
- `setTimeout(() => setDraggedNode(node), 0)` previene re-render prematuri che cancellano il drag
- `WebkitUserDrag: 'element'` abilita il drag nativo su WebKit/Electron
- `pointerEvents: 'none'` su icone/frecce previene interferenze con il drag
- `onRefresh()` deve essere chiamato dopo `fs.move` per aggiornare l'albero

### Terminal & Native Modules
- `node-pty` (modulo nativo) fallisce la compilazione con Vite/Electron system-installed
- **Workaround**: Usato `child_process` + comando `script` (Linux) per creare pseudo-terminale
- Shell forzata a `/bin/bash` per evitare problemi di input con `fish`
- `process` non è disponibile nel renderer (contextIsolation=true), usare fallback nel main process

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

### ✅ Fase 3: Tab Multipli (COMPLETATO)
- Array di file aperti
- Tab bar component
- Switch tra tab
- Stato "modificato" (dot sul tab)
- Drag & Drop per riordinare

### Fase 4: Extension API
- Extension host sandboxato
- API per: syntax highlighting, autocompletamento, compilatori
- Formato estensione: `.vix` package

### ✅ Fase 5: Terminale Integrato (COMPLETATO)
- xterm.js per terminale
- Panel in basso
- Comandi build/run
- Pseudo-terminale con `script`

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
| Sposta file | electron/main.ts | ipcMain.handle('fs:move') |
| Crea cartella | electron/main.ts | ipcMain.handle('folder:create') |
| Crea file | electron/main.ts | ipcMain.handle('file:create') |
| Avvia terminale | electron/main.ts | ipcMain.handle('terminal:start') |
| Scrivi terminale | electron/main.ts | ipcMain.handle('terminal:write') |
| Menu actions | MenuBar.tsx | menuTemplate |
| File tree | ProjectExplorer.tsx | loadDirectory, FileTreeItem, handleDrop |
| Tab bar | TabBar.tsx | TabBar component |
| Editor | EditorPanel.tsx | Monaco Editor wrapper |
| Terminal | TerminalPanel.tsx | xterm.js wrapper |
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
- [ ] Drag & drop sposta file/cartelle correttamente
- [ ] Explorer si aggiorna dopo drag & drop
- [ ] Pulsanti 📄+ e 📁+ creano file/cartelle
- [ ] Modali input funzionano correttamente
- [ ] **Tab Multipli**: Apertura, switch, chiusura, riordino funzionano
- [ ] **Stato Modificato**: Pallino bianco appare su modifiche non salvate
- [ ] **Terminale**: Si apre, mostra prompt, accetta input, esegue comandi
- [ ] **Terminale**: Usa bash correttamente (no fish issues)
- [ ] DevTools non mostrano errori

---

*Ultimo aggiornamento: 2026-05-18*
*Versione: 0.5.0 (Tab Multipli, Terminale Integrato, Drag & Drop migliorato)*