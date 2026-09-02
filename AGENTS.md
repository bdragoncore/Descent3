# AGENTS.md — Bug-Fix Workflow for bdragoncore/Descent3

This document defines the rules for analyzing, fixing, and merging upstream bug reports
into our fork (bdragoncore/Descent3). We do **not** open pull requests against the
upstream repo (DescentDevelopers/Descent3). All work is merged directly into our `main`.

---

## Repository Layout

| Repo | URL | Role |
|------|-----|------|
| **upstream** | `DescentDevelopers/Descent3` | Original project; source of bug tickets |
| **origin** | `bdragoncore/Descent3` | Our fork; `main` = our production branch |

Branches:
- `main` — our production line; all fixes land here
- `bugfix/<issue-slug>` — short-lived branches, one per fix
- `feat/render-testing-callgraphs` — experimental callgraph work (frozen)

---

## Workflow: Bug Fix Cycle

### 1. Discover Open Bug Reports

Check the upstream issue tracker for open bugs:

```
gh issue list --repo DescentDevelopers/Descent3 --label bug --state open
```

Filter for issues that are:
- Labeled `bug` or `Something isn't working`
- Reproducible (have clear steps or a description we can act on)
- Not feature requests, questions, or discussion items

### 2. Analyze the Bug

For each candidate issue:

1. **Read the full issue** — understand the reported behavior, expected behavior, and any environment details.
2. **Check if it's already fixed upstream** — search merged PRs and recent commits for related changes.
3. **Check if it applies to our fork** — verify the affected code exists in our `main` branch.
4. **Reproduce if possible** — build the project and attempt to trigger the bug.
5. **Decide**: Is this a genuine bug we can fix? If yes, proceed. If not, note the reason and skip.

### 3. Create a Bug-Fix Branch

```bash
git fetch upstream main
git checkout main
git rebase upstream/main          # ensure we're current (see §4)
git checkout -b bugfix/<issue-slug>
```

Branch naming convention:
- `bugfix/` prefix always
- `<issue-slug>` = issue number + short description, e.g. `bugfix/685-menu-resolution-mismatch`

### 4. Stay Current with Upstream

**Before** starting any new bug fix, always rebase on upstream:

```bash
git fetch upstream main
git checkout main
git rebase upstream/main
```

If the rebase produces conflicts:
1. Resolve each conflict file by file
2. Run `git rebase --continue`
3. Build and run all tests to verify nothing broke

### 5. Write a Test

Every fix must include a test that:
- Fails before the fix (demonstrates the bug)
- Passes after the fix (confirms the resolution)

```bash
# After writing the test, verify it fails on the unfixed code:
git stash   # or checkout the pre-fix state
ctest --test-dir build -R <test_name>   # should FAIL
git stash pop
# Now verify it passes:
cmake --build build --target <test_target>
ctest --test-dir build -R <test_name>   # should PASS
```

Place tests in the appropriate directory:
- C++ unit tests → `tests/cpp_src/`
- Render tests → `tests/render/`

### 6. Fix the Bug and Document Inline

Apply the minimal fix to the source code. Every fix **must** include an inline comment
explaining what was wrong and why the fix works:

```cpp
// BUGFIX #685: Menu/game resolution mismatch
// The original code used the window size instead of the selected
// resolution when computing the aspect ratio, causing menus to render
// at the wrong scale on high-DPI displays.
```

Comment format: `// BUGFIX #<issue>: <one-line summary of what was wrong>`

### 7. Build and Run All Tests

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

All tests must pass before merging. No exceptions.

### 8. Merge into Main

```bash
git checkout main
git merge --ff-only bugfix/<issue-slug>
git push origin main
```

If `--ff-only` fails (diverged history), do not force-merge. Investigate and fix.

After merging, delete the bugfix branch:

```bash
git branch -d bugfix/<issue-slug>
```

---

## Commit Message Convention

Format:
```
fix(#<issue>): <short description>

<optional body: what was wrong, what was changed, any trade-offs>

Bug: https://github.com/DescentDevelopers/Descent3/issues/<issue>
```

Example:
```
fix(#685): correct menu resolution scaling to use selected resolution

The menu renderer was using the actual window pixel dimensions instead of
the user-selected resolution from the settings, causing mismatched aspect
ratios on high-DPI monitors.

Bug: https://github.com/DescentDevelopers/Descent3/issues/685
```

---

## Rules Summary

| Rule | Detail |
|------|--------|
| **No upstream PRs** | All work is merged into our fork's `main` directly |
| **Rebase first** | Always rebase on `upstream/main` before starting a fix |
| **Test required** | Every fix must have a test that fails before and passes after |
| **Inline documentation** | Every fix must have a `// BUGFIX #N:` comment at the change site |
| **Build + tests pass** | No merge without a clean build and full test suite passing |
| **One branch per fix** | `bugfix/<issue-slug>` — short-lived, deleted after merge |
| **FF-only merge** | Linear history; no merge commits on `main` |

---

## Open Bug Tickets (DescentDevelopers/Descent3)

Tracked upstream — refresh with:
```bash
gh issue list --repo DescentDevelopers/Descent3 --label bug --state open
```

Current open bugs (as of 2026-09-02):

| # | Title | Notes |
|---|-------|-------|
| 692 | Mercenary M4 airlock door back face half-split lit | Rendering/lighting |
| 685 | Menu/game screen size smaller than selected resolution | Resolution/scaling |
| 676 | Mouse focus not captured at runtime | Input/SDL |
| 595 | Error 20 creating crash dump | Error handling |
| 586 | Shield orb/energy pickup loses emissive sound quality | Audio |
| 560 | Outdoor framerates drop to half/third of indoor | GPU perf/rendering |
| 549 | Can shoot through ground at high framerates | Physics/timing |
| 509 | Debug Windows build missing ucrtbased.dll | Build/packaging |
| 496 | Xbox controller triggers [-1,1] instead of [0,1] | Input/controller |
| 487 | Two independent sound devices for MVE and game | Audio architecture |
| 279 | x86 savegames not compatible with x64 | Serialization |
