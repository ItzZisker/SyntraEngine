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
set(BULLET_LIBS
    "${BULLET3_ROOT}/static/libBullet2FileLoader.a"
    "${BULLET3_ROOT}/static/libBulletFileLoader.a"
    "${BULLET3_ROOT}/static/libBullet3OpenCL_clew.a"
    "${BULLET3_ROOT}/static/libBullet3Common.a"
    "${BULLET3_ROOT}/static/libBullet3Geometry.a"
    "${BULLET3_ROOT}/static/libBulletCollision.a"
    "${BULLET3_ROOT}/static/libBullet3Collision.a"
    "${BULLET3_ROOT}/static/libBulletInverseDynamicsUtils.a"
    "${BULLET3_ROOT}/static/libBulletInverseDynamics.a"
    "${BULLET3_ROOT}/static/libBulletDynamics.a"
    "${BULLET3_ROOT}/static/libBullet3Dynamics.a"
    "${BULLET3_ROOT}/static/libBulletSoftBody.a"
    "${BULLET3_ROOT}/static/libLinearMath.a"
)

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
    target_link_libraries(${PROJECT_NAME}
        SDL3
        opengl32
        assimp
        -Wl,--start-group
        "${BULLET_LIBS}"
        -Wl,--end-group
    )
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} SDL3 "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

add_executable(sample_executable src/sample/sample.cpp)
target_link_libraries(sample_executable Syngine)
