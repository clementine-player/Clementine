# Build and run Clementine on Windows

Toolchain: MSYS2 MinGW64 (GCC, Qt 6, GStreamer, all from `C:\msys64\mingw64`),
CMake, and Ninja.

Commands below are for the Bash tool (Git Bash) unless marked PowerShell.

## 1. Put the MSYS2 toolchain first on PATH

```bash
export PATH=/c/msys64/mingw64/bin:$PATH
```

Do this in every shell that configures, builds or runs. The system PATH also
contains `C:\Qt\Tools\mingw1310_64\bin`, a different GCC and Qt that do not
match the MSYS2 libraries. Using it causes build or runtime DLL failures.

## 2. Configure (once per checkout or worktree)

Skip this if `bin/CMakeCache.txt` already exists.

```bash
cmake -S . -B bin -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/cc.exe \
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/c++.exe \
  -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/ninja.exe
```

Configuring takes about 15 seconds. You can't copy another checkout's `bin/`
to skip the full first build: `CMakeCache.txt` and `build.ninja` store the full
path to the source folder.

## 3. Link the GStreamer plugins (once per build directory)

`USE_BUNDLE` is on by default on Windows. With it on, `clementine.exe` loads
GStreamer plugins only from `gstreamer-plugins\` next to the exe. Without that
folder the app starts, but nothing plays. Make it a junction to the MSYS2
plugin folder (PowerShell, no admin needed):

```powershell
New-Item -ItemType Junction -Path bin\gstreamer-plugins -Target C:\msys64\mingw64\lib\gstreamer-1.0
```

## 4. Build

```bash
export PATH=/c/msys64/mingw64/bin:$PATH
ninja -C bin -j$(nproc)
```

A full build is about 1,465 steps and takes several minutes, so run it with
`run_in_background` and send the output to a log file. Incremental builds take
seconds. The result is `bin/clementine.exe`.

To check a single `.cpp` compiles without a full build, take its command from
`ninja -C bin -t commands | grep <file>.cpp`. Replace `-MD -MT … -o <obj>` with
`-fsyntax-only` and run the rest. Use the MSYS2 `c++.exe` and pass the
arguments from a file to avoid backslash/quoting problems in Git Bash.

## 5. Launch

First check nothing else is running (see the single-instance rule in
[SKILL.md](SKILL.md)):

```powershell
Get-Process clementine -ErrorAction SilentlyContinue | Select-Object Id, Path
```

Then launch with the MSYS2 DLLs on PATH (PowerShell):

```powershell
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
Start-Process "<checkout>\bin\clementine.exe"
Start-Sleep 5
Get-Process clementine | Select-Object Id, Path, MainWindowTitle
```

The `Path` must be the `bin\clementine.exe` you just built.
