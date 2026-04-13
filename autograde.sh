#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "$0")" && pwd)
source "$repo_root/tools/lab_meta.sh"

dataset_dir="$repo_root/images"
compare_png_bin="$repo_root/build/tools/compare_png"
metrics_checker="$repo_root/tools/check_metrics.awk"
metrics_threshold_checker="$repo_root/tools/check_metrics_thresholds.awk"

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

ensure_runtime_tools() {
  if [[ ! -x "$compare_png_bin" ]]; then
    echo "PNG checker was not built under $repo_root/build/tools" >&2
    return 1
  fi

  if [[ ! -f "$metrics_threshold_checker" ]]; then
    echo "missing threshold checker: $metrics_threshold_checker" >&2
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

check_output_images() {
  local output_dir=$1
  local name=

  while IFS= read -r name; do
    [[ -z "$name" || "${name:0:1}" == "#" ]] && continue
    if [[ ! -f "$output_dir/$name" ]]; then
      echo "missing output image: $output_dir/$name" >&2
      return 1
    fi
    "$compare_png_bin" "$output_dir/$name" "$output_dir/$name"
  done < "$dataset_dir/list.txt"
}

chapter_metric_bounds() {
  case "${1:-}" in
    ch1)
      printf '%s\n' \
        "21.10 21.25" \
        "30.20 30.35" \
        "0" \
        "0 0" \
        "0 0"
      ;;
    ch2 | ch4)
      printf '%s\n' \
        "21.10 21.25" \
        "30.20 30.35" \
        "1" \
        "0.8840 0.8890" \
        "0.9780 0.9830"
      ;;
    *)
      return 1
      ;;
  esac
}

check_metrics_thresholds() {
  local chapter=$1
  local metrics_path=$2
  local expected_count=$3
  local -a metric_lines
  local psnr_before_bounds=
  local psnr_after_bounds=
  local require_ssim=
  local ssim_before_bounds=
  local ssim_after_bounds=

  if ! mapfile -t metric_lines < <(chapter_metric_bounds "$chapter"); then
    echo "missing metric bounds for chapter: $chapter" >&2
    return 1
  fi

  psnr_before_bounds=${metric_lines[0]}
  psnr_after_bounds=${metric_lines[1]}
  require_ssim=${metric_lines[2]}
  ssim_before_bounds=${metric_lines[3]}
  ssim_after_bounds=${metric_lines[4]}

  awk \
    -v expected_count="$expected_count" \
    -v require_ssim="$require_ssim" \
    -v psnr_before_min="${psnr_before_bounds%% *}" \
    -v psnr_before_max="${psnr_before_bounds##* }" \
    -v psnr_after_min="${psnr_after_bounds%% *}" \
    -v psnr_after_max="${psnr_after_bounds##* }" \
    -v ssim_before_min="${ssim_before_bounds%% *}" \
    -v ssim_before_max="${ssim_before_bounds##* }" \
    -v ssim_after_min="${ssim_after_bounds%% *}" \
    -v ssim_after_max="${ssim_after_bounds##* }" \
    -f "$metrics_threshold_checker" \
    "$metrics_path"
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

  if ! ensure_runtime_tools; then
    chapter_fail "$chapter" "runtime grading tools are incomplete"
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

  if ! check_metrics_thresholds "$chapter" "$metrics_path" "$expected_count"; then
    rm -rf "$workdir"
    chapter_fail "$chapter" "metrics.csv is outside the expected metric range"
    return 1
  fi

  if ! check_output_images "$output_dir"; then
    rm -rf "$workdir"
    chapter_fail "$chapter" "output images are missing or unreadable"
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
