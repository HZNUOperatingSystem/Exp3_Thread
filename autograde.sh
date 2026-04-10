#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
  echo "usage: $0 INPUT_IMAGE REFERENCE_IMAGE [TIMEOUT_SECONDS]" >&2
  exit 1
fi

repo_root=$(cd -- "$(dirname -- "$0")" && pwd)
input_image=$1
reference_image=$2
timeout_seconds=${3:-30}
target=image_filter

if [[ ! -f "$input_image" ]]; then
  echo "error: input image not found: $input_image" >&2
  exit 1
fi

if [[ ! -f "$reference_image" ]]; then
  echo "error: reference image not found: $reference_image" >&2
  exit 1
fi

input_image=$(cd -- "$(dirname -- "$input_image")" && pwd)/$(basename -- "$input_image")
reference_image=$(cd -- "$(dirname -- "$reference_image")" && pwd)/$(basename -- "$reference_image")

cd "$repo_root"

make clean >/dev/null 2>&1 || true
make

workdir=$(mktemp -d /tmp/autograde-run.XXXXXX)
compare_src=$(mktemp /tmp/autograde-compare.XXXXXX.cpp)
compare_bin=$(mktemp /tmp/autograde-compare.XXXXXX)

cleanup() {
  rm -rf "$workdir"
  rm -f "$compare_src" "$compare_bin"
}

trap cleanup EXIT

cp "$input_image" "$workdir/input.png"

(cd "$workdir" && timeout "${timeout_seconds}s" "$repo_root/$target")

if [[ ! -f "$workdir/output.png" ]]; then
  echo "error: program did not create output.png" >&2
  exit 1
fi

cat >"$compare_src" <<'EOF'
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <cstddef>
#include <cstring>
#include <iostream>

int main(int argc, char** argv) {
  if (argc != 3) {
    return 2;
  }

  int lhs_width = 0;
  int lhs_height = 0;
  int lhs_channels = 0;
  int rhs_width = 0;
  int rhs_height = 0;
  int rhs_channels = 0;

  unsigned char* lhs = stbi_load(argv[1], &lhs_width, &lhs_height, &lhs_channels, 0);
  if (lhs == nullptr) {
    std::cerr << "failed to load " << argv[1] << '\n';
    return 1;
  }

  unsigned char* rhs = stbi_load(argv[2], &rhs_width, &rhs_height, &rhs_channels, 0);
  if (rhs == nullptr) {
    std::cerr << "failed to load " << argv[2] << '\n';
    stbi_image_free(lhs);
    return 1;
  }

  if (lhs_width != rhs_width || lhs_height != rhs_height || lhs_channels != rhs_channels) {
    std::cerr << "image shape mismatch" << '\n';
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  std::size_t byte_count = static_cast<std::size_t>(lhs_width) * static_cast<std::size_t>(lhs_height) * static_cast<std::size_t>(lhs_channels);
  if (std::memcmp(lhs, rhs, byte_count) != 0) {
    std::cerr << "pixel mismatch" << '\n';
    stbi_image_free(lhs);
    stbi_image_free(rhs);
    return 1;
  }

  stbi_image_free(lhs);
  stbi_image_free(rhs);
  return 0;
}
EOF

g++ -std=c++11 -O2 -I"$repo_root" "$compare_src" -o "$compare_bin"
"$compare_bin" "$workdir/output.png" "$reference_image"
echo "pass"
