---
name: run-clementine
description: Build Clementine from source and launch it to see a change working in the real app. Use when asked to build, run, launch, or try out a change in Clementine. Has platform-specific instructions; currently covers Windows only.
---

# Build and run Clementine

Clementine is a CMake project that builds on Windows, Linux and macOS. The
toolchain, dependencies and launch steps differ per platform, so find the
current platform below and follow that file exactly.

| Platform | Instructions |
| -------- | ------------ |
| Windows (MSYS2 MinGW64 + Ninja) | [windows.md](windows.md) |
| Linux   | Not written yet |
| macOS   | Not written yet |

If the platform has no instructions yet, work out the steps from
`CMakeLists.txt` and the existing build directory's `CMakeCache.txt`, then add
a `<platform>.md` file here and a row to the table above.

## Rules for every platform

- Build into a `bin/` directory at the root of the checkout you are working
  in. In a git worktree that means the worktree's own `bin/`. Never build into
  the main checkout's `bin/` from a worktree.
- Clementine allows only one instance. Launching a second copy just brings
  the running one to the front, and that one may be a different build. Check
  for a running `clementine` process before launching. If one is running,
  ask the user to quit it. Don't kill it yourself, because it may be playing
  music.
- After launching, confirm the new process is running from the build you just
  made (check its executable path).
