project('viterm', 'cpp',
  version: '0.1.0',
  meson_version: '>= 0.59.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
  ]
)

gtkmm_dep = dependency('gtkmm-4.0', version: '>= 4.0')
vte_dep = dependency('vte-2.91-gtk4', version: '>= 0.68')
glibmm_dep = dependency('glibmm-2.68', version: '>= 2.68')

sources = files(
  'src/main.cpp',
  'src/application.cpp',
  'src/window.cpp',
  'src/terminal_widget.cpp',
  'src/preferences_dialog.cpp',
  'src/config.cpp',
  'src/shortcuts_dialog.cpp',
)

executable('viterm',
  sources,
  dependencies: [gtkmm_dep, vte_dep, glibmm_dep],
  install: true,
)
