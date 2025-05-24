project(
  'cplayground',
  'c',
  meson_version : '>= 1.3.0',
  version : '0.1',
  default_options : [
    'warning_level=3', 
    'default_library=static',
    ],
)

dependencies = [
  # dependency('sdl2'),
  # dependency('sdl2main'),
  dependency('libcjson'), ## cjson
]

executable(
  'cplayground',
  'src/main.c',
  # win_subsystem: 'windows',
  dependencies : dependencies,
  install : true,
)

# mkdir subprojects
# meson wrap install sdl2

# rm -r build 
# meson setup build
# meson compile -C build
# build\testproject.exe
