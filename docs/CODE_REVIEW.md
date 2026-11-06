# Source review and readability changes

Review date: 2026-11-06. Base commit: `20b8f5c`.
Working branch: `refactor/code-readability`.

## What this review establishes

The review found readability problems, including compressed control flow, stale
names and comments, repeated build lists, and unused parameters. These findings
do not establish who wrote the code. There is no defensible "percent AI" result
for this repository.

An [ICSE 2025 study of source-code detectors](https://arxiv.org/abs/2411.04299)
found poor generalization in the detectors it evaluated. A separate
[ICSE-SEET 2024 study](https://arxiv.org/abs/2401.03676) also found that the five
detectors it tested struggled to distinguish generated code from human code.
Neither study validates an authorship classifier for this C++/OpenGL project.

This was a local source audit and code cleanup, supported by independent reviews,
compiler diagnostics, and runtime tests. No trained authorship classifier was run.
No source files were submitted to an external detection service.

## Coverage and methods

All 41 tracked project source/build files were reviewed: 7,175 lines at the base
commit. Vendored dependencies under `external/` and generated images/audio were
excluded from authorship analysis and kept unchanged.

| Area | Files | Coverage |
| --- | ---: | --- |
| Runtime | 30 | `main.cpp`, `AudioControls.cpp`, and the `.cpp`/`.h` pairs for Audio, Collectibles, Cutscene, Deployment, Enemy, Environment, ExampleScene, Game, Player, PlayerCombat, Renderer, TextRenderer, UI, and WaveManager |
| Tests | 6 | AudioControlsRenderTests, AudioTests, GameFlowTests, PlayerAbilitiesTests, RenderSmokeTests, RepositoryTests |
| Artwork tools | 3 | ExportKitchenAssets, FlatTextArtwork, FlatUIArtwork |
| Build/setup | 2 | `CMakeLists.txt`, `install_tools.py` |

The review combined:

- A tracked-file inventory, comment/prose searches, line-length and statement
  density checks, and an exact repeated-span scan.
- Manual review of control flow, private names, API comments, error handling,
  rendering order, and code shared with the offline artwork tools.
- Separate reviews of gameplay, rendering, and audio, followed by cross-reviews
  of the changes for accidental behavior changes.
- GCC builds with `-Wall -Wextra -Wpedantic`, the existing test suite, comparisons
  with the original commit, and render/export output comparisons.

The baseline prose search found no obvious chatbot phrases. It found 184 lines
containing multiple semicolons outside `for` headers and 89 overlapping repeated
eight-line spans across files. These are search candidates, not authorship
scores. For example, color/shape pairs and glyph grids are useful notation in
the artwork tools; repeated fixtures keep tests independently readable.

## Changes

31 source/build files changed; 10 needed no edits.

| Finding | Change |
| --- | --- |
| Asset storage still used intro-only names after expanding to the whole game | Renamed private texture storage/loading helpers and kept asset failure cleanup in one place |
| Audio controls and lifecycle code packed several actions onto one line | Expanded those sequences and named focus, drag state, buses, sound indexes, and the four reserved UI voices |
| Sprite selection and cutscene phases were difficult to scan | Expanded frame/pose selection and the five cutscene thresholds without changing formulas or timing |
| Comments repeated function names or described an earlier purchase flow | Removed redundant comments and corrected the API explanations; kept lab/teammate references and useful invariants |
| Every test target repeated the game source list | Defined `GAME_SOURCES` once, preserving each target's source set and link settings |
| Image-backed rendering retained unused geometry parameter names | Omitted 35 unused names in definitions; public signatures and header parameter names remain available |
| Exporter/test setup used dense shader and resource operations | Expanded setup/cleanup, named shader uniforms, and made the exporter's GLSL readable |

The source now has 78 such lines under the same scan rule. Remaining
artwork notation was retained deliberately. Names were chosen to explain roles;
no random quirks or artificial inconsistencies were introduced. Test conditions,
messages, numeric tolerances, game tuning, asset names, and public interfaces
were preserved.

## Validation

| Check | Result |
| --- | --- |
| Debug build, game/exporter/all test targets | Passed with GCC 15.2 |
| Release game build | Passed |
| Compiler warning review | Removed all 35 project unused-parameter warnings; the full build also reports initialization warnings in vendored `stb_image_write.h` |
| Existing CTest suite | 8 of 13 passed; the same 5 failures reproduce on the isolated original commit |
| Test-condition comparison | All 245 `check` calls retain their conditions and messages, allowing for renamed shader variables |
| Render output comparison | All 68 captures produced before the smoke-test failure match the original commit byte for byte |
| Exporter execution/output | Passed; all 123 PNGs match the original exporter byte for byte |
| Installer syntax | Python AST parsing passed; the installer was not executed |
| Whitespace validation | `git diff --check` passed |

The original commit was copied into a separate source tree and built in Debug
with the same compiler and asset files. Both versions fail these tests:

- `RenderSmoke`: the pause button hover image comparison.
- `Repository-movement`: encounter blocking, lane-end clamping, HP, and removal
  assertions, repeated at three frame rates.
- `Repository-stress`: the fixture records zero successful purchases.
- `PlayerAbilities`: the fixture expects selecting a card to only arm a purchase.
- `GameFlow`: the fixture's expected wave spawn count differs from the game.

The assertion failure sequences match between builds. Those disagreements need
their own gameplay/test review; this cleanup did not change the expectations to
make them pass. Rendering comparisons stop where the existing smoke test stops,
so they do not establish visual equivalence for every later game state.

The exporter already differs from 85 checked-in PNGs at the original commit.
Its output remains identical across this change; source images were not replaced.

Local evidence is under `build-readability/`: `source-audit-before.json`,
`source-audit-after.json`, `assertion-comparison.json`, `build.log`,
`final-build.log`, `release-build.log`, `test-results.log`,
`baseline-comparison.json`, and `exporter-comparison.json`.
The original build and test logs are under `build-readability/baseline-build/`.
These generated files are ignored by Git.

The checked build used all five test options and `BUILD_KITCHEN_ASSET_EXPORTER`
set to `ON`, with `CMAKE_CXX_FLAGS=-Wall -Wextra -Wpedantic`. To rerun the
configured checks:

```powershell
cmake --build build-readability
ctest --test-dir build-readability --output-on-failure
cmake --build build-fullscreen --target OpenGLStarter
.\build-readability\KitchenAssetExporter.exe .\build-readability\exported-assets
```
