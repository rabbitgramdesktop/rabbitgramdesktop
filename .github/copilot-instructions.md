<!-- Copied/derived from .cursor rules and repo docs -->
# Copilot / Agent Instructions for this repository

Purpose: provide concise, actionable guidance so an AI coding agent becomes productive quickly.

1) Big picture
- **Project type:** C++ Qt application (Telegram Desktop) with CMake build system and many generated sources.
- **Major components:**
  - `Telegram/SourceFiles/` — main C++ app source.
  - `Telegram/SourceFiles/mtproto/scheme/` — TL schemas; codegen produces `MTP...` types.
  - `Telegram/SourceFiles/ui/` and `.style` files — UI styling source; style codegen emits `st::` objects and `styles/style_*.h` headers.
  - `Telegram/Resources/langs/lang.strings` — base localization; `tr::` accessors are generated.
  - `external/` and `Telegram/ThirdParty/` — vendored dependencies and libraries.

2) Where to look first (key files/dirs)
- `Telegram/SourceFiles/ui/colors.palette` — central color palette.
- `Telegram/SourceFiles/ui/*/*.style` — UI style definitions (see `.cursor/rules/styling.mdc`).
- `Telegram/Resources/langs/lang.strings` — master English strings (see `.cursor/rules/localization.mdc`).
- `Telegram/SourceFiles/mtproto/scheme/api.tl` and `mtproto.tl` — API definitions and generated `MTP` types (see `.cursor/rules/api_usage.mdc`).
- `.cursor/rules/*.mdc` — repository-specific guidance (RPL, styling, localization, API usage). Read these before making changes.

3) Coding & style conventions (concrete, discoverable rules)
- Prefer `auto` (or `const auto&`) for local variables; explicit types are uncommon in code.
- Reactive patterns use `rpl::producer` and `rpl::lifetime`:
  - Always pass a lifetime to `rpl::start_...` or store returned lifetime.
  - When starting a chain that consumes the producer, use `std::move(producer)`; duplicate if reuse is needed.
- Localization: use `tr::lng_*` accessors. Pass `tr::now` for immediate values, omit for reactive producers. For `{count}` use `tr::to_count()` when converting producers.
- API calls: construct requests via `api().request(MTPnamespace_Method(...)).done(...).fail(...).handleFloodErrors().send()` and handle multi-constructor responses via `result.match(...)` or `result.type()` + `c_*()` getters.
- Generated APIs/namespaces to reference: `st::` (styles), `tr::` (strings), `MTP*` types and `MTP` request constructors.

4) Build / test / debug (concrete commands)
- This repo uses CMake. Top-level `CMakeLists.txt` configures the build.
- Common Windows developer flow (observed in `Telegram/build/build.bat`):
  - Run `Telegram\configure.bat` (sets up environment and codegen inputs).
  - From the build dir: `cmake --build . --config Release --target Telegram`.
  - For automated packaging the repo uses `Telegram\build\build.bat` which runs the above pipeline and packaging steps.
- See `docs/building-win.md`, `docs/building-linux.md` for more platform-specific steps.

5) Code generation and generated files
- Several subsystems are generated from source artifacts:
  - `.style` files → C++ `style` structs and `st::` objects placed under `Telegram/SourceFiles/styles/style_*.h`.
  - `lang.strings` → `tr::` accessors and pluralization helpers.
  - `*.tl` schemas → `MTP...` C++ types.
- Don't hand-edit generated outputs; instead adjust the source (`.style`, `lang.strings`, `.tl`) and re-run the generator via `configure.bat` or the repository build steps.

6) Integration points & external dependencies
- Vendored libs live in `external/` and `Telegram/ThirdParty/` — check these before changing build rules.
- Packaging scripts and platform-specific helpers are in `Telegram/build/` and `Telegram/` root (e.g., `configure.bat`, `build.bat`).

7) Helpful examples (search targets)
- Styling example: look for `ui/widgets/widgets.style` and include via `#include "styles/style_widgets.h"` in C++.
- Localization example: `tr::lng_files_selected(tr::now, lt_count, count)` (immediate) or reactive variant without `tr::now`.
- API example: `api().request(MTPmessages_SendMessage(...)).done(...).fail(...).handleFloodErrors().send();`

8) Agent behavior & constraints
- Check `.cursor/rules/*.mdc` before changing UI, localization, or API code — they encode accepted patterns.
- Prefer minimal, focused edits; update generators' sources instead of generated files.
- If a change touches many generated files, mention the generator and include the `configure` / build steps required to produce outputs.

If anything in these instructions is unclear or you want more depth on a specific subsystem (styles, RPL, API, or build pipelines), tell me which area to expand and I will iterate.
