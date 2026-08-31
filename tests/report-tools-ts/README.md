# Report Tools TypeScript

TypeScript/React implementation of the render test report generation tools.

## Overview

This is a complete TypeScript/React rewrite of the Python-based report generation tools,
featuring:

- **Tree building algorithms** ported from Python to TypeScript
- **React components** for report rendering
- **Tailwind CSS v4** with dark theme
- **Vite SSG** for static HTML generation
- **Vitest** for unit testing

## Architecture

```
src/
├── algorithms/          # Core tree/DAG building logic
│   ├── treeBuilder.ts
│   └── dagBuilder.ts
├── components/        # React components
│   ├── ReportPage.tsx
│   └── TestResultCard.tsx
├── styles/             # Tailwind CSS theme
│   └── theme.css
├── types/              # TypeScript interfaces
│   └── index.ts
└── utils/              # Helper utilities
    ├── renderToHtml.ts
    └── fileHelpers.ts
```

## Installation

```bash
cd tests/report-tools-ts
npm install
```

## Usage

### Generate Main Report

```bash
npm run generate -- --mode report --results ./results.json --output-dir ./out
```

### Generate Per-Trace HTMLs

```bash
npm run generate -- --mode trace --traces trace1.json trace2.json --output-dir ./out
```

### Generate Both

```bash
npm run generate -- --mode all \
  --results ./results.json \
  --traces trace1.json trace2.json \
  --output-dir ./out
```

## Development

### Run Tests

```bash
npm test              # Run tests in watch mode
npm run test:run      # Run tests once
```

### Build

```bash
npm run build
```

### Lint

```bash
npm run lint
```

### Type Check

```bash
npm run typecheck
```

## Tailwind CSS v4 Theme

The project uses Tailwind CSS v4 with a CSS-first configuration:

- **Dark theme only** - No light mode toggle
- **CSS variables** - All colors defined in `:root`
- **Purple accent** (`#8b5cf6`) for primary actions
- **Blue accent** (`#58a6ff`) for links

### Color Palette

```css
--color-bg-primary: #0f1117;      /* Main background */
--color-bg-secondary: #161b22;    /* Card background */
--color-bg-tertiary: #21262d;     /* Nested elements */
--color-accent: #8b5cf6;            /* Purple primary */
--color-accent-blue: #58a6ff;     /* Blue secondary */
```

## Testing

Unit tests cover:

- Tree building from trace events
- std:: node pruning
- Duration calculations
- DAG construction
- Link aggregation

Run tests:

```bash
npm test
```

## Migration from Python

This replaces the following Python files:

- `tests/report_tools/html_report.py` → `src/components/` + `src/utils/renderToHtml.ts`
- `tests/report_tools/generate_html.py` → `src/algorithms/` + scripts
- `tests/report_tools/callgraph.py` → `src/algorithms/dagBuilder.ts`

## CLI Arguments

| Flag | Description | Example |
|------|-------------|---------|
| `--mode` | Generation mode: report, trace, all | `report` |
| `--output-dir` | Output directory for HTML files | `./out` |
| `--results` | Path to results JSON (for report mode) | `./results.json` |
| `--traces` | List of trace JSON files (for trace mode) | `trace1.json trace2.json` |

## Integration

To integrate with the TypeScript runner, replace Python CLI calls with:

```typescript
await runCommand('npm', ['run', 'generate', '--',
  '--mode', 'report',
  '--output-dir', outputDir,
  '--results', resultsJsonPath
], {cwd: reportToolsDir});
```

## License

Same as parent project.
