![d3 (1)](https://github.com/DescentDevelopers/Descent3/assets/47716344/82ba0911-ee32-4565-84ee-b432c215ab95)

This is the Descent 3 open source engine, licensed under [GPL-3.0](https://github.com/DescentDevelopers/Descent3?tab=GPL-3.0-1-ov-file). It includes the '1.5' patch written by Kevin Bentley and Jeff Slutter several years ago and brought to a stable condition by the Descent community.

In order to use this, you must provide your own game files. See the [USAGE.md](USAGE.md) file for details about installation.

To build the game, follow build instructions in the [BUILD.md](BUILD.md) file.

## Testing
The project uses Google Test for unit testing. See [BUILD.md](BUILD.md#testing) for detailed build and test instructions.

### Render tests and call graph viewer

To build and run the headless render tests and view the HTML report:

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DBUILD_TESTING=ON
cmake --build .
cd ..

python3 tests/render/run_render_tests.py \
  --build-dir build \
  --output-dir build/tests/render_output
```

The main report is written to `build/tests/render_output/render_report.html`.

To enable the interactive call graph viewer (requires Node.js and npm):

```bash
cd tests/callgraph-viewer
npm install        # first time only
cd ../..

python3 tests/render/run_render_tests.py \
  --build-dir build \
  --output-dir build/tests/render_output \
  --serve
```

This will run the render tests, generate Callgrind-based call graphs, start the Next.js
viewer on `http://localhost:3000`, and add per-test links from the HTML report into the
viewer.

Build or runtime issues should be reported on our [GitHub tracker](https://github.com/DescentDevelopers/Descent3/issues).

## Contributing
Anyone can contribute! We have an active Discord presence at [Descent Developer Network](https://discord.gg/GNy5CUQ). Patches should be submitted on GitHub.