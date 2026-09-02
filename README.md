![d3 (1)](https://github.com/DescentDevelopers/Descent3/assets/47716344/82ba0911-ee32-4565-84ee-b432c215ab95)

This is the Descent 3 open source engine, licensed under [GPL-3.0](https://github.com/DescentDevelopers/Descent3?tab=GPL-3.0-1-ov-file). It includes the '1.5' patch written by Kevin Bentley and Jeff Slutter several years ago and brought to a stable condition by the Descent community.

In order to use this, you must provide your own game files. See the [USAGE.md](USAGE.md) file for details about installation.

To build the game, follow build instructions in the [BUILD.md](BUILD.md) file.

## Testing

The project uses Google Test for unit and integration testing. See [BUILD.md](BUILD.md#testing) for detailed build and test instructions.

### Run all tests

From the repository root (after [building](BUILD.md) with `BUILD_TESTING=ON`):

```bash
yarn install   # first time: installs tsx and runner deps
yarn tests
```

This runs **unit tests** (physics, cfile, manage, constants, etc.) then **render tests** (headless OpenGL), and generates a single consolidated HTML report:

- **Main report:** `build/tests/tests_report.html` — combined report with Unit/Render tabs

Open the report in your browser to view:
- **Unit tab:** Table of all unit tests with status and duration
- **Render tab:** Test cards with PNG screenshots, MD5 comparison, and inline trace visualization

Use `yarn tests -- --no-build` to skip configure/build and only run tests; use `--verbose` for detailed output.

### Test commands

```bash
yarn tests         # Run all tests (unit + render)
yarn unit-tests    # Run only unit tests  
yarn render-tests  # Run only render tests
```

### Render tests with call graph viewer

To run render tests and start the interactive call graph viewer:

```bash
yarn render-tests --serve
```

This starts the Next.js viewer at `http://localhost:3000` with per-test trace visualizations.

Build or runtime issues should be reported on our [GitHub tracker](https://github.com/DescentDevelopers/Descent3/issues).

## Quality of Life Improvements

We track potential QoL improvements from other Descent 3 forks,
particularly [PiccuEngine](https://github.com/InsanityBringer/PiccuEngine)
by InsanityBringer. See [piccuengine-qol.md](piccuengine-qol.md) for a
full list of improvements we can port, with credit to the original author.

Key areas under consideration:
- Windowed mode improvements (centered window, borderless fullscreen)
- Adjustable FOV with widescreen expansion
- Widescreen cockpit support
- MSAA and gamma correction via shaders
- Mouse polling rate and button improvements
- Sound channel prioritization fixes
- Multiplayer weapon selection logic

## Contributing
Anyone can contribute! We have an active Discord presence at [Descent Developer Network](https://discord.gg/GNy5CUQ). Patches should be submitted on GitHub.