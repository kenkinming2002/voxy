project('voxy', ['c', 'cpp'], default_options : ['cpp_std=c++20'])

external_lib = static_library('external', [
    'external/src/glad.c',
    'external/src/stb_image.cpp',
    'external/src/tiny_obj_loader.cpp',
  ],
  include_directories : 'external/include',
)

external_dep = declare_dependency(
  link_with : external_lib,
  include_directories : 'external/include',
)

glfw3_dep = dependency('glfw3')
freetype2_dep = dependency('freetype2')
glm_dep = dependency('glm')
yaml_cpp_dep = dependency('yaml-cpp')
fmt_dep = dependency('fmt')
spdlog_dep = dependency('spdlog')
openmp_dep = dependency('openmp')

voxy_exe = executable('voxy', [
    'src/debug_renderer.cpp',
    'src/graphics/camera.cpp',
    'src/graphics/font.cpp',
    'src/graphics/mesh.cpp',
    'src/graphics/shader_program.cpp',
    'src/graphics/texture.cpp',
    'src/graphics/texture_array.cpp',
    'src/graphics/ui_renderer.cpp',
    'src/graphics/window.cpp',
    'src/graphics/wireframe_renderer.cpp',
    'src/light_manager.cpp',
    'src/main.cpp',
    'src/physics.cpp',
    'src/player_control.cpp',
    'src/player_ui.cpp',
    'src/ray_cast.cpp',
    'src/resource_pack.cpp',
    'src/thread_pool.cpp',
    'src/timer.cpp',
    'src/world.cpp',
    'src/world_generator.cpp',
    'src/world_renderer.cpp',
  ],
  include_directories : 'include',
  dependencies : [external_dep, glfw3_dep, freetype2_dep, glm_dep, yaml_cpp_dep, fmt_dep, spdlog_dep, openmp_dep]
)
