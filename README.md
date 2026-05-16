# ViTools

> **Vibe Coding Challenge**: Can we build a real development suite using only free AI models? This project is the answer.

## About

ViTools is an experiment in **vibe coding** — a challenge to see what's possible when you pair free-tier generative AI services with tools like [OpenCode](https://opencode.ai). No paid APIs, no premium models. Just curiosity, experimentation, and fun.

The goal? Build a complete Linux desktop development suite from scratch, one tool at a time, powered entirely by AI-assisted coding.

## Roadmap

### Current
- **[ViTerm](ViTerm/)** — A modern terminal emulator with tabs, drag & drop, zoom, and full customization

### Coming Soon
- **ViEdit** — A lightweight text editor for quick edits and note-taking
- **ViStudio** — A full-featured IDE with plugin support, code navigation, and integrated tools

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

#### Quick Start
```bash
# Build
meson setup builddir
meson compile -C builddir

# Run
./builddir/viterm
```

#### Dependencies
- `gtkmm-4.0` >= 4.0
- `vte-2.91-gtk4` >= 0.68
- `glibmm-2.68` >= 2.68
- `meson` >= 0.59.0

#### Keyboard Shortcuts
| Shortcut | Action |
|----------|--------|
| `Alt + T` | New Tab |
| `Alt + W` | Close Tab |
| `Alt + Left/Right` | Switch Tab |
| `Ctrl + Q` | Quit |
| `Ctrl + Scroll` | Zoom In/Out |

## Project Structure
```
ViTools/
└── ViTerm/              # Terminal emulator
    ├── src/             # C++ source files
    ├── data/            # GSettings schema
    ├── meson.build      # Build configuration
    └── DEVELOPER_GUIDE.md
```

## License
See individual tool directories for licensing information.
