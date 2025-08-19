cmake_minimum_required(VERSION 3.10.0)
project(Syngine VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_ASSIMP "Enable Assimp support" ON)
option(USE_BULLET "Enable Bullet3 Physics support" ON)

if(USE_ASSIMP)
    add_compile_definitions(USE_ASSIMP)
endif()
if(USE_BULLET)
    add_compile_definitions(USE_BULLET)
endif()

if (USE_ASSIMP AND NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build/lib")
    message(STATUS "Building Assimp...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -B build
            -G Ninja
            -DCMAKE_C_COMPILER=clang
            -DCMAKE_CXX_COMPILER=clang++
            -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=ON
            -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=ON
            -DASSIMP_BUILD_ASSIMP_TOOLS=ON
            -DASSIMP_BUILD_ASSIMP_VIEW=OFF
            -DASSIMP_BUILD_DOCS=OFF
            -DASSIMP_BUILD_DRACO=OFF
            -DASSIMP_BUILD_FRAMEWORK=OFF
            -DASSIMP_BUILD_M3D_EXPORTER=ON
            -DASSIMP_BUILD_M3D_IMPORTER=ON
            -DASSIMP_BUILD_NONFREE_C4D_IMPORTER=OFF
            -DASSIMP_BUILD_SAMPLES=OFF
            -DASSIMP_BUILD_TESTS=OFF
            -DASSIMP_BUILD_USD_IMPORTER=OFF
            -DASSIMP_BUILD_USD_VERBOSE_LOGS=OFF
            -DASSIMP_BUILD_USE_CCACHE=OFF
            -DASSIMP_BUILD_VRML_IMPORTER=OFF
            -DASSIMP_BUILD_ZLIB=ON
            -DASSIMP_COVERALLS=OFF
            -DASSIMP_DOUBLE_PRECISION=OFF
            -DASSIMP_HUNTER_ENABLED=OFF
            -DASSIMP_IGNORE_GIT_HASH=OFF
            -DASSIMP_INCLUDE_INSTALL_DIR=include
            -DASSIMP_INJECT_DEBUG_POSTFIX=ON
            -DASSIMP_RAPIDJSON_NO_MEMBER_ITERATOR=ON
            -DASSIMP_UBSAN=OFF
            -DASSIMP_WARNINGS_AS_ERRORS=OFF
            -DASSIMP_INSTALL=ON
            -DBUILD_SHARED_LIBS=ON
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2
    )
    execute_process(
        COMMAND ninja
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build
    )
endif()

if (USE_BULLET AND NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/bullet3-3.25/build/lib")
    message(STATUS "Building Bullet...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -B build
            -G Ninja
            -DCMAKE_C_COMPILER=clang
            -DCMAKE_CXX_COMPILER=clang++
            -DBUILD_BULLET2_DEMOS=OFF
            -DBUILD_BULLET3=ON
            -DBUILD_BULLET_ROBOTICS_EXTRA=OFF
            -DBUILD_BULLET_ROBOTICS_GUI_EXTRA=OFF
            -DBUILD_CLSOCKET=OFF
            -DBUILD_CONVEX_DECOMPOSITION_EXTRA=OFF
            -DBUILD_CPU_DEMOS=OFF
            -DBUILD_ENET=OFF
            -DBUILD_EXTRAS=OFF
            -DBUILD_GIMPACTUTILS_EXTRA=OFF
            -DBUILD_HACD_EXTRA=OFF
            -DBUILD_INVERSE_DYNAMIC_EXTRA=OFF
            -DBUILD_OBJ2SDF_EXTRA=OFF
            -DBUILD_OPENGL3_DEMOS=OFF
            -DBUILD_PYBULLET=OFF
            -DBUILD_SERIALIZE_EXTRA=OFF
            -DBUILD_SHARED_LIBS=OFF
            -DBUILD_UNIT_TESTS=OFF
            -DBULLET2_MULTITHREADING=OFF
            -DCLSOCKET_DEP_ONLY=OFF
            -DCLSOCKET_SHARED=OFF
            -DENABLE_VHACD=ON
            -DINSTALL_CMAKE_FILES=ON
            -DINSTALL_LIBS=ON
            -DINTERNAL_ADD_POSTFIX_EXECUTABLE_NAMES=OFF
            -DINTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES=OFF
            -DINTERNAL_CREATE_MSVC_RELATIVE_PATH_PROJECTFILES=OFF
            -DINTERNAL_UPDATE_SERIALIZATION_STRUCTURES=OFF
            -DUSE_DOUBLE_PRECISION=OFF
            -DUSE_GLUT=OFF
            -DUSE_GRAPHICAL_BENCHMARK=OFF
            -DUSE_MSVC_INCREMENTAL_LINKING=OFF
            -DUSE_MSVC_RELEASE_RUNTIME_ALWAYS=OFF
            -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
            -DUSE_OPENVR=OFF
            -DUSE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD=ON
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/bullet3-3.25
    )
    execute_process(
        COMMAND ninja
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/bullet3-3.25/build
    )
endif()

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/src)

# === SDL Config ===
set(SDL_ROOT "${CMAKE_SOURCE_DIR}/libs/sdl-3.2.18")
include_directories(${SDL_ROOT}/include)
link_directories(${SDL_ROOT}/lib)

# === GLAD Config ===
set(GLAD_ROOT "${CMAKE_SOURCE_DIR}/libs/glad-3.3")
include_directories(${GLAD_ROOT}/include)

# === STB Image ===
set(STB_ROOT "${CMAKE_SOURCE_DIR}/libs/stb-image")
include_directories(${STB_ROOT})

# === Assimp Config ===
if (USE_ASSIMP)
    set(ASSIMP_ROOT "${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2")
    include_directories(${ASSIMP_ROOT}/include)
    include_directories(${ASSIMP_ROOT}/build/include)
    link_directories(${ASSIMP_ROOT}/build/lib)
endif()

# === Bullet3 Config ===
if (USE_BULLET)
    set(BULLET3_ROOT "${CMAKE_SOURCE_DIR}/libs/bullet3-3.25")
    include_directories(${BULLET3_ROOT}/src)
    set(BULLET_LIBS
        "${BULLET3_ROOT}/build/lib/libBullet2FileLoader.a"
        "${BULLET3_ROOT}/build/lib/libBullet3OpenCL_clew.a"
        "${BULLET3_ROOT}/build/lib/libBullet3Common.a"
        "${BULLET3_ROOT}/build/lib/libBullet3Geometry.a"
        "${BULLET3_ROOT}/build/lib/libBulletCollision.a"
        "${BULLET3_ROOT}/build/lib/libBullet3Collision.a"
        "${BULLET3_ROOT}/build/lib/libBulletInverseDynamics.a"
        "${BULLET3_ROOT}/build/lib/libBulletDynamics.a"
        "${BULLET3_ROOT}/build/lib/libBullet3Dynamics.a"
        "${BULLET3_ROOT}/build/lib/libBulletSoftBody.a"
        "${BULLET3_ROOT}/build/lib/libLinearMath.a"
    )
endif()

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
    src/engine/Concurrency.cpp
    src/engine/TaskQueue.cpp

    src/serialization/DataSerializer.cpp
    src/serialization/DataTemplates.cpp

    src/modules/Texture.cpp
    src/modules/Presets.cpp
    src/modules/GLObjects.cpp
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
    src/world/entity/BT_EntityTriangleMesh.cpp

    src/utils/FastMath.cpp
    src/utils/GameUtils.cpp
    src/utils/stb_image.c
    src/utils/glad.c
)

# Initialize common libraries
set(common_libs SDL3)

if (UNIX AND NOT APPLE)  # Linux
    list(APPEND common_libs glfw3 OpenGL::OpenGL Xrandr Xi dl)
elseif (WIN32)  # Windows
    list(APPEND common_libs opengl32)
elseif (APPLE)
    list(APPEND common_libs "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

if (USE_ASSIMP)
    list(APPEND common_libs assimp)
endif()

if (USE_BULLET)
    if (WIN32)
        list(APPEND common_libs -Wl,--start-group "${BULLET_LIBS}" -Wl,--end-group)
    else()
        list(APPEND common_libs ${BULLET_LIBS})
    endif()
endif()

target_link_libraries(${PROJECT_NAME} ${common_libs})

set_target_properties(Syngine PROPERTIES
    PREFIX ""
    OUTPUT_NAME "Syng"
)

add_executable(sample_executable
    src/sample/SampleCallbacks.cpp
    src/sample/SampleGame.cpp
    ${IMGUI_SRC}
)
target_link_libraries(sample_executable Syngine)
