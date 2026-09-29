#!/bin/bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

echo "=== status BEFORE clean ==="
git status

# URLs only; does not init anything new
git submodule sync --recursive

# Drop local modifications at top level and in every ALREADY-initialized
# submodule (nested included). Uninitialized submodules like wideband/ChibiOS are
# skipped automatically by `foreach`.
git reset --hard
git submodule foreach --recursive 'git reset --hard'

# Wipe untracked + ignored + dirs + nested untracked git repos.
# Double -f is required to remove untracked nested git repositories.
git clean -ffxd
git submodule foreach --recursive 'git clean -ffxd'

# Re-materialize ONLY the submodules that were initialized before.
# `git submodule update` (no --init, no --recursive) updates exactly the
# set already recorded in .git/config, so ChibiOS stays untouched.
git submodule update
git submodule foreach --recursive 'git submodule update'

echo "=== status AFTER clean ==="
git status
git submodule status   # non-recursive on purpose
