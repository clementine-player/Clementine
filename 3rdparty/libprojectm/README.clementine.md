# projectM in Clementine

The bundled copy of [projectM](https://github.com/projectM-visualizer/projectm),
built when the system has no projectM 4.2 or later (see `USE_SYSTEM_PROJECTM`
in the top-level `CMakeLists.txt`). Clementine needs 4.2 for
`projectm_opengl_render_frame_fbo()`, and 4.2 isn't released yet, so this is
upstream's master at commit
[1e7ef7803b69024d1e0656705670adda2ffac817](https://github.com/projectM-visualizer/projectm/tree/1e7ef7803b69024d1e0656705670adda2ffac817)
(2026-09-03), with its `vendor/projectm-eval` submodule at
[22fb0cfd8f2dfbcd2b68f2443e7f44e19b32c09a](https://github.com/projectM-visualizer/projectm-eval/tree/22fb0cfd8f2dfbcd2b68f2443e7f44e19b32c09a).
Move to the 4.2.0 tag when it's released.

Left out, because Clementine doesn't build or ship them: upstream's `docs`,
`tests`, `background.png`, the vcpkg manifests and its top-level dot files.

Changed from upstream:

- `CMakeLists.txt` starts with `PROJECT(projectM)` and asks for CMake 3.16,
  Clementine's minimum, instead of 3.21.
- `src/api/CMakeLists.txt` defines `PROJECTM_STATIC_DEFINE` for users of the
  static library.
