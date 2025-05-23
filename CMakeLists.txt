cmake_minimum_required(VERSION 3.10.0)
project(KEngine VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# === GLFW Config ===
set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/libs/glfw-3.4")
include_directories(${GLFW_ROOT}/include)
link_directories(${GLFW_ROOT}/static)

# === Assimp Config ===
set(ASSIMP_ROOT "${CMAKE_SOURCE_DIR}/libs/assimp-5.4.3")
include_directories(${ASSIMP_ROOT}/include)
link_directories(${ASSIMP_ROOT}/static)

# Include headers
include_directories(include)

# Enable testing
include(CTest)
enable_testing()

# Define library files
add_library(KEngine SHARED
    src/KEngine.cpp
    
    src/engine/RenderTable.cpp

    src/modules/Camera.cpp
    src/modules/Shader.cpp
    src/modules/Mesh.cpp
    src/modules/Model.cpp

    src/utils/3DUtils.cpp
    src/utils/stb_image.c
    src/utils/glad.c
)

if (UNIX AND NOT APPLE) # Linux
    target_link_libraries(${PROJECT_NAME} glfw3 OpenGL::OpenGL assimp Xrandr Xi dl)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} glfw3dll opengl32 assimp)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} glfw3 "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Sample test application
add_executable(sample_executable src/sample/sample.cpp)
target_link_libraries(sample_executable KEngine)

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

copy_resource(sample_executable "${PROJECT_SOURCE_DIR}/libs/glfw-3.4/bin/glfw3.dll")
copy_resource(sample_executable "${PROJECT_SOURCE_DIR}/libs/assimp-5.4.3/bin/libassimp-5.dll")
copy_resource(sample_executable "${PROJECT_SOURCE_DIR}/shaders")
copy_resource(sample_executable "${PROJECT_SOURCE_DIR}/models")
