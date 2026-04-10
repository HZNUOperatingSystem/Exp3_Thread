#!/usr/bin/env bash
set -euo pipefail

timeout_seconds=${1:-20}
repo_root=$(cd -- "$(dirname -- "$0")" && pwd)

make -C "$repo_root" clean >/dev/null 2>&1 || true
make -C "$repo_root"

timeout "${timeout_seconds}s" "$repo_root/thread_pool_test"
echo "pass"
