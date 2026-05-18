# ViTools

> **Vibe Coding Challenge**: Can we build a real development suite using only free AI models? This project is the answer.

## About

ViTools is an experiment in **vibe coding** — a challenge to see what's possible when you pair free-tier generative AI services with tools like [OpenCode](https://opencode.ai). No paid APIs, no premium models. Just curiosity, experimentation, and fun.

The goal? Build a complete Linux desktop development suite from scratch, one tool at a time, powered entirely by AI-assisted coding.

## Roadmap

### Current
- **[ViTerm](ViTerm/)** — A modern terminal emulator with tabs, drag & drop, zoom, and full customization
- **[ViEdit](ViEdit/)** — A lightweight text editor with syntax highlighting, find/replace, draft recovery, and tabbed interface
- **[ViStudio](ViStudio/)** ⚠️ — An Electron-based IDE with Monaco Editor, project management, and file explorer *(experimental — unstable, under active development)*

### Planned
- File manager integration
- Debugger frontend
- Git GUI client
- And more...

---

A suite of modern Linux desktop utilities built with **GTK4** and **C++20**.

## Tools

### [ViTerm](ViTerm/)
A modern terminal emulator featuring:
- **Tabbed interface** with customizable titles
- **Drag & drop** file paths directly into the terminal
- **Zoom** with `Ctrl + Scroll`
- **Customizable** fonts, colors, and transparency
- **Dark/Light theme** toggle
- **Persistent configuration** saved to `~/.config/vitools/viterm.cfg`

#### ViTerm Quick Start
```bash
cd ViTerm
meson setup builddir
meson compile -C builddir
./builddir/viterm
```

#### ViTerm Keyboard Shortcuts
| Shortcut | Action |
|----------|--------|
| `Alt + T` | New Tab |
| `Alt + W` | Close Tab |
| `Alt + Left/Right` | Switch Tab |
| `Ctrl + Q` | Quit |
| `Ctrl + Scroll` | Zoom In/Out |

### [ViEdit](ViEdit/)
A lightweight text editor featuring:
- **Tabbed interface** with custom close buttons
- **Syntax highlighting** via GtkSourceView (C++, Python, JSON, and more)
- **Find & Replace** with revealed search bar
- **Draft recovery** — auto-saves unsaved work and restores on startup
- **Go to Line**, word wrap, zoom, auto-indent
- **Recent files** list and dark/light theme toggle
- **Persistent settings** via GSettings

#### ViEdit Quick Start
```bash
cd ViEdit
meson setup builddir
meson compile -C builddir
./builddir/viedit
```

#### ViEdit Keyboard Shortcuts
| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New file |
| `Ctrl+O` | Open file |
| `Ctrl+S` | Save |
| `Ctrl+Shift+S` | Save As |
| `Ctrl+W` | Close tab |
| `Ctrl+Q` | Quit |
| `Ctrl+Z/Y` | Undo/Redo |
| `Ctrl+F` | Find |
| `Ctrl+H` | Replace |
| `Ctrl+G` | Go to Line |
| `Ctrl+Scroll` | Zoom font |
| `Alt+Left/Right` | Switch tab |

### [ViStudio](ViStudio/) ⚠️ *Experimental*
> **Warning**: ViStudio is under active development and may be unstable. Expect bugs, missing features, and potential crashes.

An IDE built with **Electron + React + TypeScript**, powered by **Monaco Editor** (the same engine behind VS Code).

Features:
- **Monaco Editor** with syntax highlighting, bracket pairing, and minimap
- **Multi-tab interface** — open multiple files with modified state indicators
- **Integrated terminal** (xterm.js) for build/run commands
- **File explorer** with expandable directory tree
- **Project system** using `.vistproj` configuration files
- **Create & open projects** with auto-generated structure
- **Custom menu bar** (File, Edit, View, Help)
- **Status bar** with cursor position, language, and encoding info
- **Dark theme** inspired by VS Code

#### ViStudio Quick Start
```bash
cd ViStudio
./run.sh
# Or: npm run electron:dev
```

#### Requirements
- Node.js + npm
- Electron (system-installed on Arch: `pacman -S electron`)

#### Planned Features
- Plugin/extension API (`.vix` packages)
- Command palette (Ctrl+Shift+P)
- Global search & replace with regex support
- Git integration

## Project Structure
```
ViTools/
├── ViTerm/              # Terminal emulator (C++20, GTK4)
│   ├── src/             # C++ source files
│   ├── data/            # GSettings schema
│   ├── meson.build      # Build configuration
│   └── AGENTS.md        # AI agent development guide
├── ViEdit/              # Text editor (C++20, GTK4, GtkSourceView)
│   ├── src/             # C++ source files
│   ├── data/            # GSettings schema
│   ├── meson.build      # Build configuration
│   └── AGENTS.md        # AI agent development guide
└── ViStudio/            # IDE (Electron, React, TypeScript, Monaco Editor) ⚠️
    ├── electron/        # Main & preload scripts
    ├── src/             # React components
    ├── schemas/         # Project templates
    ├── package.json     # Node dependencies
    ├── vite.config.ts   # Vite + Electron config
    ├── run.sh           # Dev launch script
    └── AGENTS.md        # AI agent development guide
```

## License
See individual tool directories for licensing information.
