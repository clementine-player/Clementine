# Screenshots

CI takes screenshots of Clementine on Linux, Windows and macOS, in light and
dark, and posts them on each pull request next to master's, as the Android and
iOS remotes do for their apps.

- `showcase-library.tsv` and `covers/` are the music the screenshots show,
  shared with the remotes' `clementine-it/`: public-domain works with
  public-domain covers (see `covers/README.md`).
- `generate-music.sh <dir>` turns them into tagged Ogg Vorbis tones, with
  GStreamer.
- `clementine --screenshots <dir> --screenshot-theme <light|dark> <music dir>`
  adds the music to the library and the playlist, pauses the first song a
  third of the way in, saves the main window on each tab and every settings
  page (the dark ones starting `dark_`), and exits. Each theme is a run of
  its own, started in it, as people see it. On Windows and macOS the screenshots include the
  window's title bar and frame; on Linux, where Wayland doesn't allow it and
  offscreen has none, they're what Qt paints inside the window. It changes
  the profile it runs in, so it's for CI runners and throwaway profiles; see
  `src/ui/screenshottaker.h`.
- `take-screenshots.sh <clementine> <music dir> <dir>` runs that for both
  themes, each with a throwaway profile, on Linux.
- `post_screenshots.sh` uploads them and writes the pull request's comment.
  See the Screenshots job in `.github/workflows/all.yml` for the R2 bucket it
  needs.

To take them locally on Linux:

```sh
tools/screenshots/generate-music.sh /tmp/music
QT_QPA_PLATFORM=offscreen tools/screenshots/take-screenshots.sh \
  build/clementine /tmp/music /tmp/screenshots
```

`clementine-tagreader` has to be built next to `clementine`.
