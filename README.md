# ViTools

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
