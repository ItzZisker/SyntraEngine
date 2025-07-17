cmake_minimum_required(VERSION 3.10.0)
project(Syngine VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/src)

# === SDL Config ===
set(SDL_ROOT "${CMAKE_SOURCE_DIR}/libs/sdl-3.2.16")
include_directories(${SDL_ROOT}/include)
link_directories(${SDL_ROOT}/static)

# === GLAD Config ===
set(GLAD_ROOT "${CMAKE_SOURCE_DIR}/libs/glad-3.3")
include_directories(${GLAD_ROOT}/include)

# === STB Image ===
set(STB_ROOT "${CMAKE_SOURCE_DIR}/libs/stb-image")
include_directories(${STB_ROOT})

# === Assimp Config ===
set(ASSIMP_ROOT "${CMAKE_SOURCE_DIR}/libs/assimp-5.4.3")
include_directories(${ASSIMP_ROOT}/include)
link_directories(${ASSIMP_ROOT}/static)

# === Bullet3 Config ===
set(BULLET3_ROOT "${CMAKE_SOURCE_DIR}/libs/bullet3-3.25")
include_directories(${BULLET3_ROOT}/include)
link_directories(${BULLET3_ROOT}/static)

# === GLM Config ===
set(GLM_ROOT "${CMAKE_SOURCE_DIR}/libs/glm")
include_directories(${GLM_ROOT})

# === ImGui Config ===
set(IMGUI_ROOT "${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b")
set(IMGUI_SRC
    ${IMGUI_ROOT}/imgui.cpp
    ${IMGUI_ROOT}/imgui_draw.cpp
    ${IMGUI_ROOT}/imgui_widgets.cpp
    ${IMGUI_ROOT}/imgui_tables.cpp
    ${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_ROOT}/backends/imgui_impl_sdl3.cpp
)
include_directories(${IMGUI_ROOT})
include_directories(${IMGUI_ROOT}/backends)

# Enable testing
include(CTest)
enable_testing()

# Define library files
add_library(Syngine SHARED
    src/Syngine.cpp
    
    src/engine/RenderTable.cpp

    src/modules/ShadowMapper.cpp
    src/modules/Screenbuffer.cpp
    src/modules/CubemapFramebuffer.cpp
    src/modules/Framebuffer.cpp
    src/modules/Scene.cpp
    src/modules/Camera.cpp
    src/modules/Shader.cpp
    src/modules/Mesh.cpp
    src/modules/MeshInstance.cpp
    src/modules/Model.cpp
    src/modules/ModelInstance.cpp
    src/modules/Skybox.cpp

    src/world/World.cpp
    src/world/WorldObject.cpp
    src/world/entity/BT_EntityConvexHull.cpp
    src/world/entity/BT_EntityConvexHullCompound.cpp
    src/world/entity/BT_EntityTriangleMesh.cpp
    src/world/entity/BT_EntityTriangleMeshCompound.cpp

    src/utils/FastMath.cpp
    src/utils/GameUtils.cpp
    src/utils/stb_image.c
    src/utils/glad.c

    ${IMGUI_SRC}
)

if (UNIX AND NOT APPLE) # Linux
    target_link_libraries(${PROJECT_NAME} SDL3 glfw3 OpenGL::OpenGL assimp Xrandr Xi dl)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} SDL3.dll opengl32 assimp
        libBullet2FileLoader
        libBullet3Collision
        libBullet3Common
        libBullet3Dynamics
        libBullet3Geometry
        libBullet3OpenCL_clew
        libBulletCollision
        libBulletDynamics
        libBulletInverseDynamics
        libBulletSoftBody
        libLinearMath
    )
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} SDL3 "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

add_executable(sample_executable src/sample/sample.cpp)
target_link_libraries(sample_executable Syngine)

function(copy_resource target src_path)
  if(IS_DIRECTORY "${src_path}")
    get_filename_component(dir_name "${src_path}" NAME)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${src_path}"
        "$<TARGET_FILE_DIR:${target}>/${dir_name}"
      COMMENT "Copying directory ${dir_name} to output directory"
    )
  elseif(EXISTS "${src_path}")
    get_filename_component(file_name "${src_path}" NAME)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${src_path}"
        "$<TARGET_FILE_DIR:${target}>/${file_name}"
      COMMENT "Copying file ${file_name} to output directory"
    )
  else()
    message(WARNING "Path '${src_path}' does not exist.")
  endif()
endfunction()

function(copy_resource_files target src_dir)
  file(GLOB files_to_copy "${src_dir}/*")

  foreach(file_path IN LISTS files_to_copy)
    get_filename_component(file_name "${file_path}" NAME)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${file_path}"
        "$<TARGET_FILE_DIR:${target}>/${file_name}"
      COMMENT "Copying ${file_name} to output directory"
    )
  endforeach()
endfunction()

copy_resource_files(sample_executable "${PROJECT_SOURCE_DIR}/libs/sdl-3.2.16/bin")
copy_resource_files(sample_executable "${PROJECT_SOURCE_DIR}/libs/assimp-5.4.3/bin")
copy_resource_files(sample_executable "${PROJECT_SOURCE_DIR}/libs/bullet3-3.25/bin")
copy_resource(sample_executable "${PROJECT_SOURCE_DIR}/shaders")
copy_resource(sample_executable "${PROJECT_SOURCE_DIR}/models")
