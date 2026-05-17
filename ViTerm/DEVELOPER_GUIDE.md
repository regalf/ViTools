# ViTerm - Developer Documentation & AI Agent Guide

## Project Overview
**ViTerm** is a modern terminal emulator built with **GTK4** and **C++20**. It features tabbed interfaces, customizable fonts/colors, transparency, drag-and-drop file paths, and persistent configuration.

### Tech Stack
- **Language**: C++20
- **GUI Framework**: GTK4 (via `gtkmm-4.0`)
- **Terminal Backend**: VTE (`vte-2.91-gtk4`)
- **Build System**: Meson + Ninja
- **Config**: GKeyFile (INI format)

## Directory Structure
```text
ViTerm/
├── meson.build              # Build configuration
├── src/
│   ├── main.cpp             # Entry point
│   ├── application.cpp/hpp  # Application lifecycle & actions
│   ├── window.cpp/hpp       # Main window, UI layout, shortcuts
│   ├── terminal_widget.cpp/hpp # VTE wrapper, zoom, drag-drop, tab titles
│   ├── config.cpp/hpp       # Settings persistence (~/.config/vitools/viterm.cfg)
│   ├── preferences_dialog.cpp/hpp # Font/Color/Opacity settings
│   └── shortcuts_dialog.cpp/hpp   # Keyboard shortcuts info
├── data/
│   └── com.vitools.viterm.gschema.xml # GSettings schema (unused, config is file-based)
└── builddir/                # Build output
```

## Core Architecture

### 1. Application (`application.cpp`)
- Inherits `Gtk::Application`.
- Handles `on_activate` to spawn `Window`.
- Registers global actions (e.g., Quit).

### 2. Window (`window.cpp`)
- **Layout**: `Gtk::HeaderBar` (top) + `Gtk::Notebook` (center).
- **Tabs**: `Gtk::Notebook` pages contain `TerminalWidget`. Tab labels are `Gtk::Box` with a `Gtk::Label` and a close button.
- **Shortcuts**: Uses `Gtk::EventControllerKey` with `GTK_PHASE_CAPTURE` to intercept keys **before** the terminal consumes them.
  - `Alt + T`: New Tab
  - `Alt + W`: Close Tab
  - `Alt + Left/Right`: Switch Tab
  - `Ctrl + Q`: Quit
- **Menu**: Popover with Theme toggle, Preferences, Build Info, and Shortcuts dialog.

### 3. TerminalWidget (`terminal_widget.cpp`)
- Inherits `Gtk::ScrolledWindow`.
- Wraps `VteTerminal`.
- **Features**:
  - **Spawn**: Runs user's `$SHELL` via `vte_terminal_spawn_async`.
  - **Zoom**: `Ctrl + Scroll` adjusts font size (range 6-72).
  - **Drag & Drop**: `Gtk::DropTarget` captures file URIs and feeds the path to the terminal.
  - **Tab Titles**: Listens to `window-title-changed`.
    - Truncates to 15 chars by default.
    - **Scrolling**: On hover, text scrolls back-and-forth using `g_timeout_add`.
  - **Font/Color**: Methods to update VTE settings dynamically.

### 4. Config (`config.cpp`)
- Singleton pattern (`Config::instance()`).
- Reads/Writes to `~/.config/vitools/viterm.cfg`.
- Stores: `dark_theme`, `opacity`, `font_family`, `font_size`, `bg_color`.

## Key Implementation Details for AI Agents

### GTK4 Specifics
- **Managed Widgets**: Use `Gtk::make_managed<T>()` instead of `new T()` to avoid manual memory management for widgets added to containers.
- **Signals**: Use `sigc::mem_fun` for member functions or lambdas.
- **Shortcuts**: `Gtk::ShortcutController` was problematic with VTE focus. The working solution is `Gtk::EventControllerKey` with `GTK_PHASE_CAPTURE` on the Window.
- **Transparency**: `set_opacity()` on the Window works best. CSS `background-color: rgba()` had issues with the Niri Window Manager (borders became transparent).

### VTE Integration
- **Window Title**: `vte_terminal_get_window_title()` is deprecated in newer VTE versions but still functional. It's used to update tab labels.
- **Feeding Input**: Use `vte_terminal_feed_child()` for drag-and-drop paths. Always escape paths using `g_shell_quote()`.

### Build System (Meson)
```meson
project('viterm', 'cpp',
  version: '0.1.0',
  default_options: ['cpp_std=c++20']
)

gtkmm_dep = dependency('gtkmm-4.0')
vte_dep = dependency('vte-2.91-gtk4')
glibmm_dep = dependency('glibmm-2.68')

executable('viterm', sources,
  dependencies: [gtkmm_dep, vte_dep, glibmm_dep],
  install: true
)
```

## Common Tasks & Patterns

### Adding a New Shortcut
1. Define the key combination in `window.cpp` `setup_shortcuts()`.
2. Ensure `GTK_PHASE_CAPTURE` is used if the terminal might steal focus.
3. Update `shortcuts_dialog.cpp` to reflect the change.

### Adding a New Setting
1. Add member variable to `Config` class.
2. Update `Config::load()` and `Config::save()`.
3. Add UI element in `preferences_dialog.cpp`.
4. Apply setting in `window.cpp` or `terminal_widget.cpp`.

### Debugging
- Logs are written to `builddir/viterm.log` when run via `./viterm > viterm.log 2>&1`.
- Use `std::cerr` for console output.

## Known Issues & Workarounds
1. **Transparency on Niri WM**: Using CSS for transparency makes window borders transparent too. Workaround: Use `set_opacity()` on the `Gtk::Window`.
2. **Tab Title Scrolling**: Uses a GLib timeout (`g_timeout_add`). Ensure `stop_title_scroll()` is called when the widget is destroyed or hover ends to avoid dangling pointers.

## Future Development Ideas
- **Search**: Implement `vte_terminal_search_set_regex` for text search.
- **Profiles**: Support multiple color/font profiles.
- **Split Panes**: Use `Gtk::Paned` to split terminal views.
- **GPU Acceleration**: VTE supports OpenGL acceleration; ensure it's enabled if available.
