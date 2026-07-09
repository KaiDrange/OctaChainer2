# OctaChainer 2

JUCE audio project configured with CMake for CLion. This project is a reimplementation of the old OctaChainer app. The original used the QT framework and was made in a hurry. This version
is a rewrite with JUCE and CMake where the intent is to do things "properly". Partly for educational purposes, but also to have a more robust and maintainable codebase for future development.

The goal for the initial release is Windows and macOS standalone applications with close to feature parity with the original.

OctaChainer 2 will be a standalone application, but can (at least initially) also be built as a VST3 plugin to keep the door open for future DAW integrations. Perhaps some drag and drop
functionality from DAWs can be added to the plugin version, that won't be possible in the standalone app? If not, then the plugin version may be dropped in the future.

## Build setup

Use the `Debug` or `Release` CMake profile.

Then choose one of the build presets:

- `Build app`
- `Build plugin`

`Build app` builds `OctaChainer2_App`.
`Build plugin` builds `AudioPluginHost`, which depends on `OctaChainer2_VST3`, so the plugin is built first if needed.

To launch the host in CLion, create a `CMake Application` run/debug configuration for `AudioPluginHost` and use `Build plugin` as the before-launch build step.

If JUCE fetch fails in your environment, set `OCTA_JUCE_SOURCE_DIR` to a local JUCE checkout and reconfigure. The checkout must contain `extras/AudioPluginHost`.

## Direct CMake commands

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug-standalone-app
```

```powershell
cmake --preset macos-debug
cmake --build --preset macos-debug-vst-plugin
```

## License

This project is licensed under the GNU General Public License v3.0.
See `LICENSE` for the full text.
