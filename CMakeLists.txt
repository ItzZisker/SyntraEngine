cmake_minimum_required(VERSION 3.11.0)
project(Syngine VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_ASSIMP "Enable Assimp support" ON)
option(USE_BULLET "Enable Bullet3 Physics support" ON)

set(BULLET_INSTALL "${CMAKE_SOURCE_DIR}/libs/bullet3-3.25/build/install")
set(ASSIMP_INSTALL "${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build/install")

if (USE_ASSIMP AND NOT EXISTS ${ASSIMP_INSTALL})
    message(STATUS "Verifying Assimp build...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -B build
            -DCMAKE_INSTALL_PREFIX=./build/install
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
        COMMAND make install
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build
    )
    execute_process(
        COMMAND ninja install
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build
    )
endif()

if (USE_BULLET AND NOT EXISTS ${BULLET_INSTALL})
    message(STATUS "Verifying Bullet3 build...")

    set(BULLET_EXTRA_ARGS "")
    if(UNIX AND NOT APPLE) # Linux
        set(BULLET_EXTRA_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE=ON)
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -B build
            -DCMAKE_INSTALL_PREFIX=./build/install
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
            -DLIB_DESTINATION=lib
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
            ${BULLET_EXTRA_ARGS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/bullet3-3.25
    )
    execute_process(
        COMMAND make install
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/bullet3-3.25/build
    )
    execute_process(
        COMMAND ninja install
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build
    )
endif()

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/src)

# Initialize common libraries (OpenGL & SDL3)
set(common_libs "")

if (UNIX AND NOT APPLE)  # Linux
    list(APPEND common_libs OpenGL::OpenGL Xrandr Xi dl)
elseif (WIN32)  # Windows
    list(APPEND common_libs opengl32)
elseif (APPLE)
    list(APPEND common_libs "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# === SDL Config ===
set(SDL_ROOT "${CMAKE_SOURCE_DIR}/libs/sdl-3.2.18")
if (WIN32)
    include_directories(${SDL_ROOT}/windows/include)
    list(APPEND common_libs ${SDL_ROOT}/windows/bin/SDL3.dll)
    link_directories(${SDL_ROOT}/windows/lib)
elseif(UNIX AND NOT APPLE)
    include_directories(${SDL_ROOT}/linux/include)
    list(APPEND common_libs ${SDL_ROOT}/linux/bin/libSDL3.so.0.2.18)
endif()

# == FreeType Config ==
# set(FREETYPE_ROOT "${CMAKE_SOURCE_DIR}/libs/freetype-2.14.1")
# if (WIN32)
#     include_directories(${FREETYPE_ROOT}/win32/include/freetype2)
#     list(APPEND common_libs ${FREETYPE_ROOT}/win32/lib/libfreetype.a)
# elseif(UNIX AND NOT APPLE)
#     include_directories(${FREETYPE_ROOT}/linux/include/freetype2)
#     list(APPEND common_libs ${FREETYPE_ROOT}/linux/lib/libfreetype.a)
# endif()

# === GLAD Config ===
set(GLAD_ROOT "${CMAKE_SOURCE_DIR}/libs/glad-3.3")
include_directories(${GLAD_ROOT}/include)

# === STB Image ===
set(STB_ROOT "${CMAKE_SOURCE_DIR}/libs/stb-image")
include_directories(${STB_ROOT})

# === Assimp Config ===
if (USE_ASSIMP)
    if (WIN32)
        list(APPEND common_libs "${ASSIMP_INSTALL}/lib/libassimp.dll")
    elseif(UNIX AND NOT APPLE)
        list(APPEND common_libs "${ASSIMP_INSTALL}/lib/libassimp.so")
    endif()
    add_compile_definitions(USE_ASSIMP)
    include_directories(${ASSIMP_INSTALL}/include)
    link_directories(${ASSIMP_INSTALL}/lib)
endif()

# === Bullet3 Config ===
if (USE_BULLET)
    add_compile_definitions(USE_BULLET)
    include_directories(${BULLET_INSTALL}/include/bullet)
    set(BULLET_LIBS
        "${BULLET_INSTALL}/lib/libBullet2FileLoader.a"
        "${BULLET_INSTALL}/lib/libBullet3OpenCL_clew.a"
        "${BULLET_INSTALL}/lib/libBullet3Common.a"
        "${BULLET_INSTALL}/lib/libBullet3Geometry.a"
        "${BULLET_INSTALL}/lib/libBulletCollision.a"
        "${BULLET_INSTALL}/lib/libBullet3Collision.a"
        "${BULLET_INSTALL}/lib/libBulletInverseDynamics.a"
        "${BULLET_INSTALL}/lib/libBulletDynamics.a"
        "${BULLET_INSTALL}/lib/libBullet3Dynamics.a"
        "${BULLET_INSTALL}/lib/libBulletSoftBody.a"
        "${BULLET_INSTALL}/lib/libLinearMath.a"
    )
    if (WIN32)
        list(APPEND common_libs -Wl,--start-group "${BULLET_LIBS}" -Wl,--end-group)
    else()
        list(APPEND common_libs -Wl,--whole-archive ${BULLET_LIBS} -Wl,--no-whole-archive)
    endif()
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
add_library(${PROJECT_NAME} SHARED
    src/Syngine/Syngine.cpp

    src/Syngine/engine/RenderTable.cpp
    src/Syngine/engine/Concurrency.cpp
    src/Syngine/engine/TaskQueue.cpp

    src/Syngine/serialization/DataSerializer.cpp
    src/Syngine/serialization/DataTemplates.cpp

    #src/Syngine/modules/TextUtil.cpp
    src/Syngine/modules/BatchRenderer.cpp
    src/Syngine/modules/Material.cpp
    src/Syngine/modules/Texture.cpp
    src/Syngine/modules/Presets.cpp
    src/Syngine/modules/GLObjects.cpp
    src/Syngine/modules/ShadowMapper.cpp
    src/Syngine/modules/Screenbuffer.cpp
    src/Syngine/modules/Framebuffer.cpp
    src/Syngine/modules/Scene.cpp
    src/Syngine/modules/Camera.cpp
    src/Syngine/modules/Shader.cpp
    src/Syngine/modules/Mesh.cpp
    src/Syngine/modules/MeshInstance.cpp
    src/Syngine/modules/Model.cpp
    src/Syngine/modules/ModelInstance.cpp
    src/Syngine/modules/Skybox.cpp

    src/Syngine/world/World.cpp
    src/Syngine/world/Coordination.cpp
    src/Syngine/world/entity/BT_EntityConvexHull.cpp
    src/Syngine/world/entity/BT_EntityTriangleMesh.cpp

    src/Syngine/utils/FastMath.cpp
    src/Syngine/utils/GameUtils.cpp
    src/Syngine/utils/stb_image.c
    src/Syngine/utils/glad.c
)

target_link_libraries(${PROJECT_NAME} ${common_libs})

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "Syng"
        VERSION ${PROJECT_VERSION}
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "Syng"
        VERSION ${PROJECT_VERSION}
    )
endif()

add_executable(sample_executable
    src/Syngine/sample/SampleCallbacks.cpp
    src/Syngine/sample/SampleGame.cpp
    ${IMGUI_SRC}
)
target_link_libraries(sample_executable ${PROJECT_NAME})
