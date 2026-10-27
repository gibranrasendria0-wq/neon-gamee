# NEON SURVIVOR

A C++17 top-down survival shooter built with raylib. The project is designed for desktop development and browser deployment through WebAssembly.

## Features

- Keyboard movement and mouse/space shooting
- Enemy waves with difficulty selection
- Health, score, survival timer, pause and game-over screens
- Neon grid, hit particles, simple power-up effect (rapid fire)
- No paid assets required

## Requirements

- CMake
- A C++17 compiler
- raylib
- For browser builds: Emscripten SDK (emsdk)

## Run on desktop

Install raylib using the official instructions for your operating system:
https://github.com/raysan5/raylib#installation

Then, from the project root:

```bash
cmake -S . -B build
cmake --build build
```

Run the generated executable:
- Windows: `build/Debug/neon-survivor.exe` or `build/neon-survivor.exe` (depends on generator)
- Linux/macOS: `./build/neon-survivor`

If CMake cannot find raylib, install raylib and ensure its CMake package is available to CMake.

## Build for the web

You need Emscripten and a raylib build compiled for the web. The most reliable way is to build raylib from source with Emscripten.

1. Clone raylib beside this project or into a temporary folder:

```bash
git clone https://github.com/raysan5/raylib.git
```

2. Activate Emscripten in your terminal (`emsdk_env.bat` on Windows, `source ./emsdk_env.sh` on macOS/Linux).
3. Build raylib for Web from its source folder:

```bash
cd raylib/src
emmake make PLATFORM=PLATFORM_WEB
```

4. Return to this project and configure CMake with the raylib source/build paths as appropriate for your setup. A common approach is to use raylib's CMake package with the Emscripten toolchain:

```bash
emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release -Draylib_DIR=/path/to/raylib/lib/cmake/raylib
cmake --build build-web
```

The web build outputs an HTML launcher plus `.js` and `.wasm` files under `build-web/dist` when the raylib CMake package is discoverable. Keep those generated files together.

> Emscripten/raylib setup differs a little between operating systems and raylib versions. If CMake can't find a web-compatible raylib package, use raylib's official Web platform instructions and pass the resulting raylib library/include paths to CMake.

## Deploy

After a successful web build, deploy the contents of `build-web/dist` as a static website. You can use Vercel CLI or any static host that serves `.html`, `.js`, and `.wasm` files with correct MIME types.

For Vercel CLI:

```bash
npm install -g vercel
cd build-web/dist
vercel
```

Choose the default options for a static project. Test the deployed URL in a desktop browser first. The desktop keyboard controls are the primary controls in this starter version; mobile touch controls can be added as a next feature.

## Controls

- `W A S D` or arrow keys: move
- Mouse click or Space: shoot toward the mouse pointer
- `P` or `Esc`: pause/resume
- Game over: `Enter` or `R` to play again, `M` for main menu

## Notes

This is a starter project. It uses geometric shapes for its visuals, so you can focus on gameplay before adding sprite sheets, sound, save data, mobile controls, and more advanced enemy types.
