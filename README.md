# OctaChainer2

JUCE audio project configured with CMake for CLion.

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
