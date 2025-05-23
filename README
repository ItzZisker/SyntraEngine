# K-Engine

Very Basic, lightweight OpenGL 3D Game-Engine. Look sample.cpp.

TODO:
- ~~Cross-Platform~~
- ~~Lightweight,~~ Optimized
- Simple & Easy to Use in Code
- Normal Mapping (as well as other chapters https://www.learnopengl.com)
- Audio Support (OpenAL)
- GLTF Support
- Bones & Animation Support
- Bullet Support
- PhysX Support

## Manual Build

Those libraries are required statically built onto libs folder before build (for each platform .so .dll .a):

- GLAD (Included as glad.c)
- GLFW-3: Version v3.4
- OpenGL (Atleast v3.3) (Installed with Graphics drivers, eg. MESA) including glad.zip
- Assimp v5.4.3 (Open Asset Import Library)

### Mac OS

1. Install `clang ninja cmake` by brew:
```
brew install gcc clang make cmake
```
2. ```cd KEngine/ && cmake .```

### Linux

1. Install `clang ninja cmake` (Installation commands varies by your distribution)
2. ```cd KEngine/ && cmake .```

### Windows

Well Well, Windows Users

**NOTE:**
This project does not support Visual Studio (MSVC). Due to recurring compatibility issues with Unix-like platforms and personal experience finding Visual Studio annoying for cross-platform development, I’ve chosen MinGW as base toolchain. The goal is to ensure the engine remains fully cross-platform.

1. Install MSYS2: https://www.msys2.org/
2. Ensure all of `clang ninja cmake` packages are installed:
```
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-clang cmake ninja

cmake --version
clang --version
gcc --version
```
If you get errors or command not found, it means you've failed the installation (part 2) or your environment variables are not set.
Set MSYS binaries to your Windows Path (By default MSYS installed at C:\msys64. replace it with your installation as exception):
```
C:\msys64\clang64\bin
C:\msys64\mingw64\bin
C:\msys64\mingw32\bin
C:\msys64\ucrt64\bin
C:\msys64\usr\bin
```

3. then just:

```
cd KEngine\
cmake .
```