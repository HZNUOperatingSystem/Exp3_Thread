#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "$0")" && pwd)
source "$repo_root/tools/lab_meta.sh"

dataset_dir="$repo_root/images"
reference_dir="$repo_root/reference/expected"
metrics_reference_dir="$repo_root/reference/metrics"
compare_png_bin="$repo_root/build/tools/compare_png"
compare_metrics_bin="$repo_root/build/tools/compare_metrics"
metrics_checker="$repo_root/tools/check_metrics.awk"

usage() {
  echo "usage: $0 [all|${LAB_CHAPTERS[*]}]" >&2
  exit 1
}

chapter_fail() {
  local chapter=$1
  local reason=$2

  echo "$chapter: fail"
  echo "  reason: $reason" >&2
  return 1
}

list_count() {
  grep -v '^[[:space:]]*$' "$dataset_dir/list.txt" | grep -vc '^#'
}

run_with_timeout() {
  local timeout_seconds=$1
  shift

  if command -v timeout >/dev/null 2>&1; then
    timeout "${timeout_seconds}s" "$@"
    return $?
  fi

  if command -v gtimeout >/dev/null 2>&1; then
    gtimeout "${timeout_seconds}s" "$@"
    return $?
  fi

  "$@" &
  local command_pid=$!
  local watchdog_pid=
  local status=0

  (
    sleep "$timeout_seconds"
    kill -TERM "$command_pid" 2>/dev/null || exit 0
    sleep 1
    kill -KILL "$command_pid" 2>/dev/null || true
  ) &
  watchdog_pid=$!

  if wait "$command_pid"; then
    status=0
  else
    status=$?
  fi

  kill -TERM "$watchdog_pid" 2>/dev/null || true
  wait "$watchdog_pid" 2>/dev/null || true

  case "$status" in
    143 | 137)
      return 124
      ;;
    *)
      return "$status"
      ;;
  esac
}

chapter_binary_path() {
  printf '%s/%s\n' "$repo_root" "$(lab_target_path "$1")"
}

build_chapter() {
  local chapter=$1
  local targets=("$chapter")

  if [[ "$(lab_requires_tools "$chapter")" == "1" ]]; then
    targets+=("tools")
  fi

  make -C "$repo_root" "${targets[@]}" >/dev/null
}

ensure_reference_assets() {
  local chapter=$1

  if [[ ! -d "$reference_dir" ]]; then
    echo "missing reference image directory: $reference_dir" >&2
    return 1
  fi

  if [[ ! -d "$metrics_reference_dir" ]]; then
    echo "missing reference metrics directory: $metrics_reference_dir" >&2
    return 1
  fi

  if [[ ! -f "$metrics_reference_dir/${chapter}_metrics.csv" ]]; then
    echo "missing reference metrics file: $metrics_reference_dir/${chapter}_metrics.csv" >&2
    return 1
  fi

  if [[ ! -x "$compare_png_bin" || ! -x "$compare_metrics_bin" ]]; then
    echo "comparison tools were not built under $repo_root/build/tools" >&2
    return 1
  fi
}

check_metrics_file() {
  local metrics_path=$1
  local expected_count=$2

  if [[ ! -f "$metrics_path" ]]; then
    echo "missing metrics file: $metrics_path" >&2
    return 1
  fi

  awk -v expected_count="$expected_count" -f "$metrics_checker" "$metrics_path"
}

check_reference_metrics() {
  local actual_metrics=$1
  local expected_metrics=$2

  "$compare_metrics_bin" "$actual_metrics" "$expected_metrics" 0.0005
}

check_reference_images() {
  local output_dir=$1
  local name=

  while IFS= read -r name; do
    [[ -z "$name" || "${name:0:1}" == "#" ]] && continue
    "$compare_png_bin" "$output_dir/$name" "$reference_dir/$name"
  done < "$dataset_dir/list.txt"
}

run_image_chapter() {
  local chapter=$1
  local workdir=
  local expected_count=
  local metrics_path=
  local output_dir=
  local binary_name=

  if ! build_chapter "$chapter"; then
    chapter_fail "$chapter" "build failed"
    return 1
  fi

  if ! ensure_reference_assets "$chapter"; then
    chapter_fail "$chapter" "reference assets are incomplete"
    return 1
  fi

  workdir=$(mktemp -d "${TMPDIR:-/tmp}/ex4os-${chapter}.XXXXXX")
  cp -r "$dataset_dir" "$workdir/images"
  metrics_path="$workdir/$(lab_metrics_path "$chapter")"
  output_dir="$workdir/$(lab_output_dir "$chapter")"
  binary_name=$(lab_binary_name "$chapter")

  if ! (cd "$workdir" && LAB_CHAPTER="$chapter" run_with_timeout "$(lab_timeout_seconds "$chapter")" "$(chapter_binary_path "$chapter")"); then
    rm -rf "$workdir"
    chapter_fail "$chapter" "$binary_name failed or timed out"
    return 1
  fi

  expected_count=$(list_count)
  if ! check_metrics_file "$metrics_path" "$expected_count"; then
    rm -rf "$workdir"
    chapter_fail "$chapter" "invalid metrics.csv"
    return 1
  fi

  if ! check_reference_metrics "$metrics_path" "$metrics_reference_dir/${chapter}_metrics.csv"; then
    rm -rf "$workdir"
    chapter_fail "$chapter" "metrics.csv does not match reference"
    return 1
  fi

  if ! check_reference_images "$output_dir"; then
    rm -rf "$workdir"
    chapter_fail "$chapter" "output images do not match reference"
    return 1
  fi

  rm -rf "$workdir"
  echo "$chapter: pass"
}

check_ch1_source() {
  local source_path="$repo_root/$(lab_entry_source ch1)"

  if ! grep -Eq '^#define[[:space:]]+IMAGE_PARALLEL_FOR[[:space:]]+_Pragma' "$source_path"; then
    echo "src/ch1/main.c must define IMAGE_PARALLEL_FOR with OpenMP" >&2
    return 1
  fi
}

grade_ch1() {
  if ! check_ch1_source; then
    chapter_fail "ch1" "required OpenMP pragma macro is missing"
    return 1
  fi

  run_image_chapter "ch1"
}

check_ch4_source() {
  local source_path="$repo_root/$(lab_entry_source ch4)"
  local symbol=

  for symbol in thread_pool_init thread_pool_submit thread_pool_wait thread_pool_destroy; do
    if ! grep -Eq "${symbol}[[:space:]]*\\(" "$source_path"; then
      echo "missing required call in src/ch4/main.c: ${symbol}(...)" >&2
      return 1
    fi
  done
}

grade_ch2() {
  run_image_chapter "ch2"
}

grade_ch3() {
  if ! build_chapter "ch3"; then
    chapter_fail "ch3" "build failed"
    return 1
  fi

  if run_with_timeout "$(lab_timeout_seconds ch3)" "$(chapter_binary_path ch3)"; then
    echo "ch3: pass"
    return 0
  fi

  chapter_fail "ch3" "$(basename "$(chapter_binary_path ch3)") failed or timed out"
}

grade_ch4() {
  if ! check_ch4_source; then
    chapter_fail "ch4" "src/ch4/main.c is missing required thread pool calls"
    return 1
  fi

  run_image_chapter "ch4"
}

main() {
  local chapter=${1:-all}
  local failures=0

  case "$chapter" in
    all)
      grade_ch1 || failures=1
      grade_ch2 || failures=1
      grade_ch3 || failures=1
      grade_ch4 || failures=1
      ;;
    ch1)
      grade_ch1 || failures=1
      ;;
    ch2)
      grade_ch2 || failures=1
      ;;
    ch3)
      grade_ch3 || failures=1
      ;;
    ch4)
      grade_ch4 || failures=1
      ;;
    *)
      usage
      ;;
  esac

  if [[ "$failures" -ne 0 ]]; then
    exit 1
  fi

  if [[ "$chapter" == "all" ]]; then
    printf '\033[1;32mAll chapters passed. Lab complete.\033[0m\n'
  fi
}

main "$@"
