#!/usr/bin/env bash

LAB_CHAPTERS=(ch1 ch2 ch3 ch4)
LAB_IMAGE_CHAPTERS=(ch1 ch2 ch4)
LAB_CHAPTER_BINARY=lab

lab_is_chapter() {
  local chapter=$1
  local item

  for item in "${LAB_CHAPTERS[@]}"; do
    if [[ "$item" == "$chapter" ]]; then
      return 0
    fi
  done

  return 1
}

lab_is_image_chapter() {
  local chapter=$1
  local item

  for item in "${LAB_IMAGE_CHAPTERS[@]}"; do
    if [[ "$item" == "$chapter" ]]; then
      return 0
    fi
  done

  return 1
}

lab_binary_name() {
  if ! lab_is_chapter "${1:-}"; then
    return 1
  fi

  printf '%s\n' "$LAB_CHAPTER_BINARY"
}

lab_target_path() {
  local chapter=$1
  local build_dir=${2:-build}

  if ! lab_is_chapter "$chapter"; then
    return 1
  fi

  printf '%s/%s/%s\n' "$build_dir" "$chapter" "$LAB_CHAPTER_BINARY"
}

lab_timeout_seconds() {
  case "${1:-}" in
    ch3) printf '%s\n' "20" ;;
    ch1 | ch2 | ch4) printf '%s\n' "60" ;;
    *) return 1 ;;
  esac
}

lab_output_dir() {
  local chapter=$1

  if ! lab_is_image_chapter "$chapter"; then
    return 1
  fi

  printf '%s\n' "output/$chapter"
}

lab_metrics_path() {
  local chapter=$1

  if ! lab_is_image_chapter "$chapter"; then
    return 1
  fi

  printf '%s\n' "output/$chapter/metrics.csv"
}

lab_entry_source() {
  local chapter=${1:-}

  if ! lab_is_chapter "$chapter"; then
    return 1
  fi

  printf '%s\n' "src/$chapter/main.c"
}

lab_requires_tools() {
  if lab_is_image_chapter "$1"; then
    printf '%s\n' "1"
  else
    printf '%s\n' "0"
  fi
}

usage() {
  cat >&2 <<'EOF'
usage: lab_meta.sh <command> [chapter] [build_dir]

commands:
  list-chapters
  list-image-chapters
  binary-name <chapter>
  target-path <chapter> [build_dir]
  timeout-seconds <chapter>
  output-dir <chapter>
  metrics-path <chapter>
  entry-source <chapter>
  requires-tools <chapter>
EOF
}

main() {
  local command=${1:-}

  case "$command" in
    list-chapters)
      printf '%s\n' "${LAB_CHAPTERS[*]}"
      ;;
    list-image-chapters)
      printf '%s\n' "${LAB_IMAGE_CHAPTERS[*]}"
      ;;
    binary-name)
      lab_binary_name "${2:-}"
      ;;
    target-path)
      lab_target_path "${2:-}" "${3:-build}"
      ;;
    timeout-seconds)
      lab_timeout_seconds "${2:-}"
      ;;
    output-dir)
      lab_output_dir "${2:-}"
      ;;
    metrics-path)
      lab_metrics_path "${2:-}"
      ;;
    entry-source)
      lab_entry_source "${2:-}"
      ;;
    requires-tools)
      lab_requires_tools "${2:-}"
      ;;
    *)
      usage
      return 1
      ;;
  esac
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  set -euo pipefail
  main "$@"
fi
