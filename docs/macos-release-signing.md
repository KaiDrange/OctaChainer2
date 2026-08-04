# macOS Release Signing And Notarization

## One-time setup

1. Install your `Developer ID Application` certificate in the login keychain.
2. Create a notarization profile in the local keychain:

```bash
xcrun notarytool store-credentials "octa-notary" \
  --apple-id "YOUR_APPLE_ID" \
  --team-id "YOUR_TEAM_ID" \
  --password "YOUR_APP_SPECIFIC_PASSWORD"
```

## Terminal setup

Use this path if you build from a shell.

1. Export the signing values in the terminal session you will use for the build:

```bash
export OCTA_MACOS_CODESIGN_IDENTITY='Developer ID Application: Your Name (TEAMID)'
export OCTA_MACOS_NOTARY_PROFILE='octa-notary'
```

2. Build the release targets from that same terminal session.

## CLion setup

Use this path if you build from CLion.

1. In CLion, create a local copy of the shared `macos-release` CMake profile.

   - Go to `Settings | Build, Execution, Deployment | CMake`.
   - Select the imported `macos-release` profile.
   - Click `Copy` or `Edit a copy`.
   - Give the copy a local name such as `macOS Release-signing`.
   - Set a different build directory for the copy, for example `build/macos/macos-release-signing`.

   CLion imports preset-based profiles in read-only mode, so a copied local profile is the normal way to keep machine-specific settings out of `CMakePresets.json`. CLion also stores environment variables on the profile itself, and those variables apply to CMake generation and build.

2. In the copied profile, open the `Environment` field and add:

   - `OCTA_MACOS_CODESIGN_IDENTITY=Developer ID Application: Your Name (TEAMID)`
   - `OCTA_MACOS_NOTARY_PROFILE=octa-notary`

   You do not need to set these in a shell if you build only from CLion.

Only the identity and profile name are needed by the build. The credential itself stays in the keychain.

## Where to find the Apple values

- `OCTA_MACOS_CODESIGN_IDENTITY`: Use the exact identity string reported by `security find-identity -p codesigning -v`.
- `OCTA_MACOS_NOTARY_PROFILE`: Use the profile name you chose in `xcrun notarytool store-credentials`, for example `octa-notary`.
- `YOUR_TEAM_ID`: Apple lists the Team ID in your Apple Developer membership details.
- `YOUR_APPLE_ID`: Use the Apple Account email you use for the Developer Program and App Store Connect.
- `YOUR_APP_SPECIFIC_PASSWORD`: Create this at `account.apple.com` under `Sign-In and Security > App-Specific Passwords`. Apple requires two-factor authentication for app-specific passwords.

## Release build

If you build from a terminal, use the shared preset:

```bash
cmake --preset macos-release
cmake --build --preset macos-release-standalone-app
cmake --build --preset macos-release-vst-plugin
```

If you build from CLion, use your copied local profile named something like `macOS Release-signing`.

Those build steps copy the signed bundles into `MacRelease/` and then:

1. signs the copied `.app` and `.vst3` bundles
2. leaves notarization for a separate step

To notarize after the build, run the explicit notarize targets:

```bash
cmake --build --preset macos-release-standalone-app --target OctaChainer2_NotarizeApp
cmake --build --preset macos-release-vst-plugin --target OctaChainer2_NotarizePlugin
```

Those targets create a zip archive with `ditto`, submit it with `xcrun notarytool`, print the submission ID, and write a small record file in the build directory.
In CLion, build the app or plugin target first, then run the matching notarize target from the same build profile.
The record file is named like `OctaChainer2_NotarizeApp_Release_notary.txt` in the active build directory.

After Apple finishes processing the submission, use `notarytool info` or `notarytool log` to check the status:

```bash
xcrun notarytool info <submission-id> --keychain-profile "octa-notary"
xcrun notarytool log <submission-id> --keychain-profile "octa-notary"
```

Once the status is `Accepted`, run the staple target:

```bash
cmake --build --preset macos-release-standalone-app --target OctaChainer2_StapleApp
cmake --build --preset macos-release-vst-plugin --target OctaChainer2_StaplePlugin
```

If `OCTA_MACOS_NOTARY_PROFILE` is unset, the notarize targets fail immediately.

## Verification

After a release build, you can check the result manually:

```bash
codesign --verify --strict --verbose=2 "MacRelease/OctaChainer2.app"
codesign --verify --strict --verbose=2 "MacRelease/OctaChainer2.vst3"
xcrun stapler validate "MacRelease/OctaChainer2.app"
xcrun stapler validate "MacRelease/OctaChainer2.vst3"
```

## Notes

- Keep the `Developer ID Application` identity and the notarization profile local to your machine.
- If you change certificates or Apple Developer team membership, recreate the local notarization profile.
- If notarization fails, inspect the `notarytool` output first. It usually names the exact file or entitlement that needs correction.
