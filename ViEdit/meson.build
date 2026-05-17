project('viedit', 'cpp',
  version: '0.1.0',
  meson_version: '>= 0.59.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
  ]
)

gtkmm_dep = dependency('gtkmm-4.0', version: '>= 4.0')
gtksourceview_dep = dependency('gtksourceview-5', version: '>= 1.0')
glibmm_dep = dependency('glibmm-2.68', version: '>= 2.68')

sources = files(
  'src/main.cpp',
  'src/application.cpp',
  'src/window.cpp',
  'src/editor_tab.cpp',
  'src/config.cpp',
  'src/shortcuts_dialog.cpp',
  'src/preferences_dialog.cpp',
)

executable('viedit',
  sources,
  dependencies: [gtkmm_dep, gtksourceview_dep, glibmm_dep],
  install: true,
)
