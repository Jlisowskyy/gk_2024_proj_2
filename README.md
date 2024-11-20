# Simplistic render engine

A C++ application for visualizing and manipulating cubic Bézier surfaces with real-time lighting calculations and triangle mesh rendering.

## Features

- Load and render cubic Bézier surfaces from text files
- Interactive rotation around X and Z axes
- Triangle mesh visualization with adjustable density
- Real-time lighting with:
  - Diffuse lighting (Lambert model)
  - Specular highlights
  - Animated light source moving in a spiral pattern
- Texture mapping support
- Normal mapping with real-time normal vector modification
- Parallel processing using OpenMP

![cpu_gk](https://github.com/user-attachments/assets/94aac5b7-4460-4e50-8474-023e7520c137)

## Technologies

- C++
- Qt 6
- OpenMP
- CMake

## Building

```bash
mkdir build
cd build
cmake ..  -DCMAKE_BUILD_TYPE=RELEASE
cmake --build .
```

## Input File Format

The application expects a text file containing 16 control points (4x4 grid) for the Bézier surface. Each line should contain three real numbers representing X, Y, and Z coordinates:

```
X00 Y00 Z00
X01 Y01 Z01
...
X33 Y33 Z33
```

## License

MIT
