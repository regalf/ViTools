# ViEdit - AI Agent Development Guide

## Project Overview

**ViEdit** is a lightweight text editor, part of the **ViTools** suite. It's built with **GTK4 + gtkmm-4.0 + GtkSourceView-5 + C++20** using **Meson** as the build system. It follows the same architecture as **ViTerm** (the terminal emulator in the same workspace).

### Workspace Structure
```
/home/regaldragoon200/Progetti/ViTools/
├── ViTerm/          # Terminal emulator (existing, reference architecture)
└── ViEdit/          # Text editor (this project)
    ├── meson.build
    ├── ci-check.sh
    ├── data/
    │   └── com.vitools.viedit.gschema.xml
    └── src/
        ├── main.cpp
        ├── application.cpp/.hpp
        ├── window.cpp/.hpp
        ├── editor_tab.cpp/.hpp
        ├── config.cpp/.hpp
        ├── shortcuts_dialog.cpp/.hpp
        └── preferences_dialog.cpp/.hpp
```

## Build System

### Dependencies
- `gtkmm-4.0 >= 4.0`
- `gtksourceview-5 >= 1.0`
- `glibmm-2.68 >= 2.68`
- `meson >= 0.59.0`

### Build Commands
```bash
cd /home/regaldragoon200/Progetti/ViTools/ViEdit
meson setup builddir          # First time only
meson compile -C builddir     # Build
./builddir/viedit             # Run
```

### CI Check Script
```bash
./ci-check.sh                 # Validates deps, builds, checks binary
```

### GSettings Schema
Must be installed for settings to work:
```bash
cp data/com.vitools.viedit.gschema.xml ~/.local/share/glib-2.0/schemas/
glib-compile-schemas ~/.local/share/glib-2.0/schemas/
```

## Architecture

### Component Flow
```
main.cpp → Application → Window → Notebook (tabs) → EditorTab
                                    ↓
                              HeaderBar + Menu Popover
                              StatusBar
                              Find/Replace Revealer
```

### File Responsibilities

| File | Purpose |
|------|---------|
| `main.cpp` | Entry point, creates Application |
| `application.cpp/.hpp` | Gtk::Application subclass, handles startup/activate |
| `window.cpp/.hpp` | Main window, ALL UI logic, actions, shortcuts, dialogs |
| `editor_tab.cpp/.hpp` | Individual editor tab with GtkSourceView (C API) |
| `config.cpp/.hpp` | Singleton for GSettings + draft file management |
| `shortcuts_dialog.cpp/.hpp` | Keyboard shortcuts reference dialog |
| `preferences_dialog.cpp/.hpp` | Settings dialog with checkboxes/spinbuttons |

## Critical Implementation Details

### GtkSourceView Uses C API (Not C++ Bindings)
**IMPORTANT**: `gtksourceviewmm-5` does NOT exist in Arch repos. Only `gtksourceviewmm` (GTK3 version) is available. All GtkSourceView usage is through **C headers** (`<gtksourceview/gtksource.h>`) with raw pointers:

```cpp
#include <gtksourceview/gtksource.h>

GtkSourceBuffer* m_buffer = nullptr;   // C pointer, NOT Glib::RefPtr
GtkSourceView* m_view = nullptr;        // C pointer, NOT Glib::RefPtr

// Creation
m_buffer = gtk_source_buffer_new(nullptr);
m_view = GTK_SOURCE_VIEW(gtk_source_view_new_with_buffer(m_buffer));

// Adding to scrolled window (GTK4 way)
gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(m_scrolled_window->gobj()), GTK_WIDGET(m_view));
```

### GtkSourceView Theme Colors
Uses GtkSourceView's built-in **style schemes** (not custom CSS):
```cpp
GtkSourceStyleSchemeManager* manager = gtk_source_style_scheme_manager_get_default();
GtkSourceStyleScheme* scheme = gtk_source_style_scheme_manager_get_scheme(manager, "Adwaita-dark");
gtk_source_buffer_set_style_scheme(m_buffer, scheme);
```
Available schemes: `Adwaita`, `Adwaita-dark`, `oblivion`, `solarized-dark`, `solarized-light`, `classic`

### CSS Provider for Tab Close Button
Uses C API for CSS (Gtk::CssProvider doesn't exist in gtkmm4):
```cpp
auto* provider = gtk_css_provider_new();
gtk_css_provider_load_from_string(provider, "button.tab-close { border-radius: 9px; ... }");
gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(btn->gobj())),
    GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
g_object_unref(provider);
```

### GSettings String Array (recent-files)
`get_strv`/`set_strv` don't exist in glibmm 2.68. Use GVariant:
```cpp
// Read
Glib::VariantBase base;
m_settings->get_value("recent-files", base);
auto variant = Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<Glib::ustring>>>(base);
auto vec = variant.get();

// Write
auto variant = Glib::Variant<std::vector<Glib::ustring>>::create(glist);
m_settings->set_value("recent-files", variant);
```

### User Config Directory
`Glib::get_user_config_dir()` doesn't exist. Use GLib C API:
```cpp
#include <glib.h>
const char* config_dir = g_get_user_config_dir();
```

### Gio::ApplicationFlags
In gtkmm4, it's `Gio::Application::Flags::HANDLES_OPEN` (not `Gio::ApplicationFlags`).

### Gtk::ApplicationWindow Constructor
Takes `Glib::RefPtr<Gtk::Application>`, not a raw reference:
```cpp
auto app_ptr = Glib::wrap(app.gobj(), true);
set_application(app_ptr);
```

### Action Signals
`signal_activate()` passes `const Glib::VariantBase&`:
```cpp
action->signal_activate().connect([callback](const Glib::VariantBase&) { callback(); });
```

### Event Controller Signals
Use `connect_notify` for void-returning lambdas, or match exact signature:
```cpp
key_controller->signal_key_pressed().connect_notify([this](guint keyval, guint keycode, Gdk::ModifierType state) {
    on_key_pressed(keyval, keycode, state);
});
```

### Gtk::EventControllerScroll
No `create(Flags)` overload. Create empty, then set flags:
```cpp
auto scroll_controller = Gtk::EventControllerScroll::create();
scroll_controller->set_flags(Gtk::EventControllerScroll::Flags::VERTICAL | Gtk::EventControllerScroll::Flags::DISCRETE);
```

### Dialogs in GTK4
- `Gtk::MessageDialog::run()` is gone. Use `GtkAlertDialog` (C API) with async callbacks.
- `Gtk::FileChooserDialog` is gone. Use `gtk_file_chooser_native_new()` (C API).
- `Gtk::EntryDialog` doesn't exist. Create custom Gtk::Window with Gtk::Entry.

### GtkAlertDialog Pattern
```cpp
auto* alert = gtk_alert_dialog_new("Title");
gtk_alert_dialog_set_detail(alert, "Detail text");
const char* buttons[] = {"_Save", "_Don't Save", "_Cancel", NULL};
gtk_alert_dialog_set_buttons(alert, buttons);

gtk_alert_dialog_choose(alert, GTK_WINDOW(gobj()), NULL, +[](GObject* source, GAsyncResult* res, gpointer data) {
    auto* win = static_cast<Window*>(data);
    int response = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), res, NULL);
    // response 0 = first button, 1 = second, 2 = third
}, this);
g_object_unref(alert);
```

### GtkNativeDialog (File Chooser) Pattern
```cpp
auto* dialog = gtk_file_chooser_native_new("Title", GTK_WINDOW(gobj()), GTK_FILE_CHOOSER_ACTION_SAVE, "_Save", "_Cancel");
gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
g_signal_connect(dialog, "response", G_CALLBACK(+[](GtkNativeDialog* dlg, int response, gpointer data) {
    if (response == GTK_RESPONSE_ACCEPT) {
        auto* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
        // use g_file_get_path(file)
        g_object_unref(file);
    }
    gtk_native_dialog_destroy(dlg);
}), this);
```

### Gdk::ModifierType
It's an enum class in gtkmm4. Need explicit cast for bitwise operations:
```cpp
auto state_int = static_cast<unsigned int>(state);
auto meta_mask = static_cast<unsigned int>(Gdk::ModifierType::META_MASK);
if (state_int & meta_mask) { ... }
```

### StatusBar
`Gtk::Statusbar` in GTK4 is minimal. Use a `Gtk::Box` instead:
```cpp
m_statusbar = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
m_statusbar->append(*label1);
m_statusbar->append(*label2);
```

### Gtk::Separator
Requires `#include <gtkmm/separator.h>` explicitly.

### Gtk::Adjustment
Requires `#include <gtkmm/adjustment.h>` explicitly.

### Gtk::Popover
No constructor taking a widget. Create empty, then `set_parent()`:
```cpp
m_popover = Gtk::make_managed<Gtk::Popover>();
m_popover->set_parent(*m_menu_button);
```

### insert_action_group
Takes `Glib::RefPtr<Gio::ActionGroup>`:
```cpp
insert_action_group("win", Glib::RefPtr<Gio::ActionGroup>(m_action_group));
```

### Gio::SimpleActionGroup
Uses `add_action()`, not `add()`:
```cpp
m_action_group->add_action(action);
```

## Features Implemented

| Feature | Implementation |
|---------|---------------|
| **Tabs** | Gtk::Notebook with custom tab labels (name + × button) |
| **Draft mode** | Auto-save to `~/.config/vitools/viedit/drafts/<hash>.draft` with `#DRAFT:<path>` header |
| **Shortcuts** | Gio::SimpleAction with accelerators via `set_accel_for_action()` |
| **Theme toggle** | `gtk_settings_get_default()` + `g_object_set()` for dark theme |
| **Exit warning** | GtkAlertDialog async callback for unsaved changes |
| **Syntax highlighting** | GtkSourceView style schemes, detected by file extension |
| **Highlight toggle** | Via Preferences dialog, shown in status bar |
| **Find/Replace** | Revealer bar with search entry, replace entry, buttons |
| **Go to Line** | Custom dialog window with Gtk::Entry |
| **Word Wrap** | Toggle via Preferences, shown in status bar |
| **Zoom font** | Ctrl+Scroll via EventControllerScroll |
| **Status bar** | Gtk::Box with labels: modified, language, position, wrap, highlight, path |
| **File recent** | GSettings string array, restored on startup |
| **Auto-indent** | GtkSourceView auto-indent enabled |
| **Shortcuts dialog** | Grid with shortcut keys and descriptions |
| **Preferences dialog** | Checkboxes and spinbuttons for all settings |
| **About dialog** | Custom window with version info and repo link |
| **Close on last tab** | App auto-closes when last tab is closed |
| **Tab close button** | Small circular "×" button with custom CSS |

## Draft System

### File Format
```
#DRAFT:/absolute/path/to/file.ext
<file content here>
```

### Lifecycle
1. **On text change**: Content saved to draft file (if modified)
2. **On file save**: Draft file deleted
3. **On tab close (× button)**: Draft file deleted, file removed from recent list
4. **On app close**: Draft files preserved for next launch
5. **On app startup**: All draft files scanned and restored as tabs

### Key Functions
- `Config::list_all_drafts()` - Scans draft directory, parses headers to get original paths
- `Config::save_draft(path, content)` - Writes `#DRAFT:<path>\n<content>`
- `Config::load_draft(path)` - Reads content after the header line
- `Config::delete_draft(path)` - Removes the draft file
- `Config::has_draft(path)` - Checks if draft exists

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New file |
| `Ctrl+O` | Open file |
| `Ctrl+S` | Save |
| `Ctrl+Shift+S` | Save As |
| `Ctrl+W` | Close tab |
| `Ctrl+Q` | Quit |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Ctrl+F` | Find |
| `Ctrl+H` | Replace |
| `Ctrl+G` | Go to Line |
| `Ctrl+Scroll` | Zoom font |
| `Alt+Left/Right` | Switch tab |
| `Ctrl+,` | Preferences |
| `Ctrl+?` | Shortcuts dialog |

## Settings (GSettings)

Schema: `com.vitools.viedit` at `/com/vitools/viedit/`

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `window-width` | int | 1024 | Window width |
| `window-height` | int | 768 | Window height |
| `window-maximized` | bool | false | Maximized state |
| `dark-theme` | bool | true | Dark theme enabled |
| `font-size` | int | 12 | Editor font size |
| `font-family` | string | 'Monospace' | Editor font family |
| `show-line-numbers` | bool | true | Show line numbers |
| `word-wrap` | bool | false | Word wrap enabled |
| `highlight-syntax` | bool | true | Syntax highlighting |
| `auto-indent` | bool | true | Auto indent |
| `recent-files` | string array | [] | Recently opened files |
| `max-recent-files` | int | 10 | Max recent files count |
| `tab-size` | int | 4 | Tab width in spaces |
| `use-spaces-for-tabs` | bool | true | Spaces instead of tabs |

## Common Pitfalls for AI Agents

1. **Don't use `Gtk::CssProvider::create()`** - It doesn't exist. Use `gtk_css_provider_new()` (C API).
2. **Don't use `Gtk::MessageDialog::run()`** - Use `GtkAlertDialog` with async callbacks.
3. **Don't use `Gtk::FileChooserDialog`** - Use `gtk_file_chooser_native_new()`.
4. **Don't use `Gtk::Statusbar::push()`** - Use a `Gtk::Box` with labels.
5. **Don't use `Glib::get_user_config_dir()`** - Use `g_get_user_config_dir()`.
6. **Don't use `Gio::ApplicationFlags`** - Use `Gio::Application::Flags`.
7. **Don't use `m_settings->get_strv()`** - Use `get_value()` with `Glib::Variant`.
8. **Don't use `signal_activate().connect(callback)`** - The callback must accept `const Glib::VariantBase&`.
9. **Don't use `set_relief()` on Gtk::Button** - Doesn't exist in gtkmm4.
10. **Don't use `Gtk::Adjustment::create()`** without `#include <gtkmm/adjustment.h>`.

## Terminal Output (for CI)

The app prints check markers to stdout:
```
[ViEdit] Starting...
[  OK  ] UI initialized
[  OK  ] Actions registered
[  OK  ] Shortcuts configured
[  OK  ] Event controllers ready
[  OK  ] Settings loaded
[  OK  ] Theme applied
[  OK  ] New file created
[  OK  ] Session restored
[ViEdit] Ready
```

On exit:
```
[ViEdit] Exiting...
```

## Repo URL
https://github.com/regalf/ViTools
