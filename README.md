# mdcat

A `cat`-like tool that renders a Markdown file to a readable, formatted
terminal screen instead of dumping raw source. Written in C++17 with Boost,
built with CMake, and dependency-managed with vcpkg so it builds the same way
on Linux and Windows.

It is intentionally simple, not a full CommonMark implementation — it covers
the markdown people actually write day to day.

## Features

- `#` through `######` headers, printed bold and color-coded by level, with
  an underline rule under `#` and `##` so they stand out
- **bold**, *italic*, and `inline code` spans
- Fenced code blocks (```` ``` ````), rendered in a boxed, dimmed panel
- Pretty-printed tables, including column alignment (`:---`, `:---:`, `---:`)
- Bulleted and numbered lists (including nested/indented items)
- Blockquotes (`>`)
- Horizontal rules (`---`, `***`, `___`)
- Links `[text](url)`
- Emoji shortcodes, e.g. `:rocket:` → 🚀, `:tada:` → 🎉 (literal emoji already
  in the file pass through untouched)

## Usage

```sh
mdcat README.md
```

`mdcat` takes exactly one argument: the path to a `.md` or `.markdown` file.
Output is written to stdout with ANSI formatting; redirecting to a file or
pipe still works, you'll just see the raw escape codes.

```sh
mdcat --help
```

`examples/tables.md` exercises table rendering specifically (alignment,
uneven widths, inline formatting inside cells, empty cells, pipe-less GFM
tables) — useful as a quick smoke test after changes to the renderer:

```sh
mdcat examples/tables.md
```

## Building

### Prerequisites

- CMake 3.16+
- A C++17 compiler (GCC, Clang, or MSVC)
- [vcpkg](https://github.com/microsoft/vcpkg)
- Ninja (optional but recommended)

### 1. Get vcpkg

If you don't already have it:

```sh
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh      # Linux/macOS
.\vcpkg\bootstrap-vcpkg.bat     # Windows
```

Clone the **full** history — don't use `--depth 1`. `vcpkg.json` pins a
`builtin-baseline`, and vcpkg reads the port versions for that baseline out of
the clone's own git history, so the baseline commit has to actually be present.
Note also that the standalone vcpkg bundled with Visual Studio
(`...\VC\vcpkg`) ships no ports, and fails at configure time with *"this vcpkg
instance requires a manifest with a specified baseline"*; use a real clone
instead.

### 2. Configure and build

The project ships a `vcpkg.json` manifest, so dependencies (Boost's
`program_options`, `regex`, and `algorithm`) are fetched and built
automatically the first time you configure — no manual `vcpkg install` step
needed.

**Linux / macOS:**

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/mdcat README.md
```

**Windows (Developer PowerShell / cmd, with Visual Studio installed):**

```powershell
cmake -S . -B build ^
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
.\build\Release\mdcat.exe README.md
```

The resulting binary is a normal native executable — `build/mdcat` on
Linux/macOS, `build\Release\mdcat.exe` on Windows — with no runtime
dependency on vcpkg itself.

### 3. Windows: a self-contained binary (optional)

The default Windows build links Boost dynamically, so `mdcat.exe` needs
`boost_program_options-*.dll` beside it and the Visual C++ redistributable
installed. For a single file you can drop anywhere on your `PATH`, build
against the static triplet with the static CRT:

```powershell
cmake -S . -B build-static -G Ninja ^
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded ^
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-static
```

`build-static\mdcat.exe` then imports only `KERNEL32.dll` and `USER32.dll` —
Boost and the C++ runtime are linked in, so it needs no accompanying DLLs and
no redistributable. Verify with `dumpbin /dependents build-static\mdcat.exe`.

Note that the static triplet builds its own copy of Boost, so the first
configure is slow (~10 minutes); later builds reuse the cache.

### 4. Linux: a self-contained binary (optional)

vcpkg's default `x64-linux` triplet already links Boost statically, so the
only floating dependency in a normal Linux build is the C++ runtime
(`libstdc++.so` / `libgcc_s.so`) — visible with `ldd build/mdcat`. Statically
linking those too gives you a single binary that depends on nothing but
`libc`/`libm` and the dynamic linker, so it can be copied to another Linux
machine without worrying about a mismatched runtime version:

```sh
cmake -S . -B build-static -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"
cmake --build build-static
```

Verify with `ldd build-static/mdcat` — it should list only `linux-vdso.so.1`,
`libc.so.6`, and `ld-linux-x86-64.so.2`.

A `Makefile` wraps this for convenience:

```sh
make static                          # build-static/mdcat, statically linked
make install                         # also copies it to ~/.local/bin
```

`VCPKG_ROOT` defaults to `~/vcpkg`; override it if vcpkg lives elsewhere:

```sh
make install VCPKG_ROOT=/opt/vcpkg
```

`make install` copies `build-static/mdcat` to `~/.local/bin/mdcat`. Make sure
`~/.local/bin` is on your `PATH` (most distros add it automatically for login
shells if the directory exists; otherwise add
`export PATH="$HOME/.local/bin:$PATH"` to your shell profile).

### Installing

```sh
cmake --install build --prefix /usr/local
```

On Windows, `install(TARGETS)` copies `mdcat.exe` alone — the Boost DLL is
*not* installed alongside it, so a dynamically linked build installed this way
will fail to start. Either use the self-contained build above, or copy the DLL
next to the installed binary yourself.

On Linux, `make install` (see above) is a simpler alternative to
`cmake --install` for a single-user setup: it builds the static binary and
drops it straight into `~/.local/bin`, no `sudo` or `--prefix` needed.

## Project layout

```
CMakeLists.txt        # build configuration
Makefile               # convenience wrapper: make static / make install (Linux)
vcpkg.json             # vcpkg manifest (Boost dependencies)
src/main.cpp           # CLI entry point, argument parsing, file validation
src/markdown_renderer.* # the markdown -> ANSI-terminal renderer
src/terminal.*          # cross-platform ANSI/UTF-8 console setup, width detection
src/emoji_map.hpp       # :shortcode: -> emoji glyph table
examples/tables.md      # table-rendering smoke test / reference
```
