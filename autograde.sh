#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
  echo "usage: $0 DATASET_DIR REFERENCE_OUTPUT_DIR [TIMEOUT_SECONDS]" >&2
  exit 1
fi

repo_root=$(cd -- "$(dirname -- "$0")" && pwd)
dataset_root=$1
reference_root=$2
timeout_seconds=${3:-60}
target=pool_batch

if [[ ! -d "$dataset_root/input" || ! -d "$dataset_root/gt" || ! -f "$dataset_root/list.txt" ]]; then
  echo "error: dataset dir must contain input/, gt/, and list.txt" >&2
  exit 1
fi

if [[ ! -d "$reference_root" ]]; then
  echo "error: reference output dir not found: $reference_root" >&2
  exit 1
fi

make -C "$repo_root" clean >/dev/null 2>&1 || true
make -C "$repo_root"

workdir=$(mktemp -d /tmp/ch3-autograde.XXXXXX)
compare_src=$(mktemp /tmp/ch3-compare.XXXXXX.c)
compare_bin=$(mktemp /tmp/ch3-compare.XXXXXX)

cleanup() {
  rm -rf "$workdir"
  rm -f "$compare_src" "$compare_bin"
}

trap cleanup EXIT

mkdir -p "$workdir/image"
cp -r "$dataset_root/input" "$workdir/image/input"
cp -r "$dataset_root/gt" "$workdir/image/gt"
cp "$dataset_root/list.txt" "$workdir/image/list.txt"

(cd "$workdir" && timeout "${timeout_seconds}s" "$repo_root/$target")

if [[ ! -f "$workdir/metrics.csv" ]]; then
  echo "error: metrics.csv was not created" >&2
  exit 1
fi

expected_count=$(grep -v '^[[:space:]]*$' "$workdir/image/list.txt" | grep -vc '^#')
actual_count=$(($(wc -l < "$workdir/metrics.csv") - 1))
if [[ "$actual_count" -ne "$expected_count" ]]; then
  echo "error: metrics.csv row count mismatch" >&2
  exit 1
fi

awk -F, '
NR == 1 { next }
NF != 6 { exit 1 }
{
  for (i = 2; i <= 5; ++i) {
    if ($i !~ /^-?[0-9]+(\.[0-9]+)?$/) {
      exit 1
    }
  }
  if ($6 !~ /^-?[0-9]+$/) {
    exit 1
  }
}
' "$workdir/metrics.csv" || {
  echo "error: metrics.csv format is invalid" >&2
  exit 1
}

cat >"$compare_src" <<'EOF'
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stddef.h>
#include <string.h>

int main(int argc, char** argv) {
  int lhs_width = 0;
  int lhs_height = 0;
  int lhs_channels = 0;
  int rhs_width = 0;
  int rhs_height = 0;
  int rhs_channels = 0;
  unsigned char* lhs;
  unsigned char* rhs;
  size_t byte_count;

  if (argc != 3) {
    return 2;
  }

  lhs = stbi_load(argv[1], &lhs_width, &lhs_height, &lhs_channels, 0);
  rhs = stbi_load(argv[2], &rhs_width, &rhs_height, &rhs_channels, 0);
  if (lhs == NULL || rhs == NULL) {
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  if (lhs_width != rhs_width || lhs_height != rhs_height || lhs_channels != rhs_channels) {
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  byte_count = (size_t)lhs_width * (size_t)lhs_height * (size_t)lhs_channels;
  if (memcmp(lhs, rhs, byte_count) != 0) {
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  stbi_image_free(lhs);
  stbi_image_free(rhs);
  return 0;
}
EOF

gcc -std=c11 -O2 -I"$repo_root" "$compare_src" -lm -o "$compare_bin"

while IFS= read -r name; do
  [[ -z "$name" || "${name:0:1}" == "#" ]] && continue
  "$compare_bin" "$workdir/output/$name" "$reference_root/$name"
done < "$workdir/image/list.txt"

echo "pass"
