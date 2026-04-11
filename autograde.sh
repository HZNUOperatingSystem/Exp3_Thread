#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "$0")" && pwd)
dataset_dir="$repo_root/image"
reference_dir="$repo_root/reference/expected"
metrics_reference_dir="$repo_root/reference/metrics"
compare_png_bin="$repo_root/build/tools/compare_png"
compare_metrics_bin="$repo_root/build/tools/compare_metrics"
metrics_checker="$repo_root/tools/check_metrics.awk"

usage() {
  echo "usage: $0 [ch1|ch2|ch3|ch4]" >&2
  exit 1
}

list_count() {
  grep -v '^[[:space:]]*$' "$dataset_dir/list.txt" | grep -vc '^#'
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

grade_ch1() {
  local workdir=
  local expected_count=

  make -C "$repo_root" ch1 tools >/dev/null

  if ! grep -Eq '^#define[[:space:]]+IMAGE_PARALLEL_FOR[[:space:]]+_Pragma' "$repo_root/ch1/main.c"; then
    echo "ch1: fail"
    echo "  reason: ch1/main.c must define IMAGE_PARALLEL_FOR with OpenMP" >&2
    return 1
  fi

  workdir=$(mktemp -d /tmp/ex4os-ch1.XXXXXX)
  cp -r "$dataset_dir" "$workdir/image"

  if ! (cd "$workdir" && timeout 60s "$repo_root/$CH1_TARGET_PATH"); then
    rm -rf "$workdir"
    echo "ch1: fail"
    echo "  reason: image_batch failed or timed out" >&2
    return 1
  fi

  expected_count=$(list_count)
  if ! check_metrics_file "$workdir/ch1/output/metrics.csv" "$expected_count"; then
    rm -rf "$workdir"
    echo "ch1: fail"
    echo "  reason: invalid metrics.csv" >&2
    return 1
  fi

  if ! check_reference_metrics "$workdir/ch1/output/metrics.csv" "$metrics_reference_dir/ch1_metrics.csv"; then
    rm -rf "$workdir"
    echo "ch1: fail"
    echo "  reason: metrics.csv does not match reference" >&2
    return 1
  fi

  if ! check_reference_images "$workdir/ch1/output"; then
    rm -rf "$workdir"
    echo "ch1: fail"
    echo "  reason: output images do not match reference" >&2
    return 1
  fi

  rm -rf "$workdir"
  echo "ch1: pass"
}

grade_ch2() {
  local workdir=
  local expected_count=

  make -C "$repo_root" ch2 tools >/dev/null

  workdir=$(mktemp -d /tmp/ex4os-ch2.XXXXXX)
  cp -r "$dataset_dir" "$workdir/image"

  if ! (cd "$workdir" && timeout 60s "$repo_root/$CH2_TARGET_PATH"); then
    rm -rf "$workdir"
    echo "ch2: fail"
    echo "  reason: ssim_batch failed or timed out" >&2
    return 1
  fi

  expected_count=$(list_count)
  if ! check_metrics_file "$workdir/ch2/output/metrics.csv" "$expected_count"; then
    rm -rf "$workdir"
    echo "ch2: fail"
    echo "  reason: invalid metrics.csv" >&2
    return 1
  fi

  if ! check_reference_metrics "$workdir/ch2/output/metrics.csv" "$metrics_reference_dir/ch2_metrics.csv"; then
    rm -rf "$workdir"
    echo "ch2: fail"
    echo "  reason: metrics.csv does not match reference" >&2
    return 1
  fi

  if ! check_reference_images "$workdir/ch2/output"; then
    rm -rf "$workdir"
    echo "ch2: fail"
    echo "  reason: output images do not match reference" >&2
    return 1
  fi

  rm -rf "$workdir"
  echo "ch2: pass"
}

check_ch4_source() {
  local source_path="$repo_root/ch4/main.c"
  local symbol=

  for symbol in thread_pool_init thread_pool_submit thread_pool_wait thread_pool_destroy; do
    if ! grep -Eq "${symbol}[[:space:]]*\\(" "$source_path"; then
      echo "missing required call in ch4/main.c: ${symbol}(...)" >&2
      return 1
    fi
  done
}

grade_ch3() {
  make -C "$repo_root" ch3 >/dev/null

  if timeout 20s "$repo_root/$CH3_TARGET_PATH"; then
    echo "ch3: pass"
    return 0
  fi

  echo "ch3: fail"
  echo "  reason: thread_pool_test failed or timed out" >&2
  return 1
}

grade_ch4() {
  local workdir=
  local expected_count=

  make -C "$repo_root" ch4 tools >/dev/null

  if ! check_ch4_source; then
    echo "ch4: fail"
    return 1
  fi

  workdir=$(mktemp -d /tmp/ex4os-ch4.XXXXXX)
  cp -r "$dataset_dir" "$workdir/image"

  if ! (cd "$workdir" && timeout 60s "$repo_root/$CH4_TARGET_PATH"); then
    rm -rf "$workdir"
    echo "ch4: fail"
    echo "  reason: pool_batch failed or timed out" >&2
    return 1
  fi

  expected_count=$(list_count)
  if ! check_metrics_file "$workdir/ch4/output/metrics.csv" "$expected_count"; then
    rm -rf "$workdir"
    echo "ch4: fail"
    echo "  reason: invalid metrics.csv" >&2
    return 1
  fi

  if ! check_reference_metrics "$workdir/ch4/output/metrics.csv" "$metrics_reference_dir/ch4_metrics.csv"; then
    rm -rf "$workdir"
    echo "ch4: fail"
    echo "  reason: metrics.csv does not match reference" >&2
    return 1
  fi

  if ! check_reference_images "$workdir/ch4/output"; then
    rm -rf "$workdir"
    echo "ch4: fail"
    echo "  reason: output images do not match reference" >&2
    return 1
  fi

  rm -rf "$workdir"
  echo "ch4: pass"
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

CH1_TARGET_PATH=build/ch1/image_batch
CH2_TARGET_PATH=build/ch2/ssim_batch
CH3_TARGET_PATH=build/ch3/thread_pool_test
CH4_TARGET_PATH=build/ch4/pool_batch

main "$@"
