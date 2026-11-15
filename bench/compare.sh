#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
# SPDX-License-Identifier: MIT
#
# Compares the performance of two libcbor versions. See bench/README.md.
#
# Builds libcbor at BASE and at HEAD, builds this checkout's harness against
# each, measures every case, and prints a Markdown report. Instruction counts
# need valgrind; without it, the script measures wall-clock time instead.

set -euo pipefail

usage() {
  cat >&2 <<EOF
Usage: $0 [options]

  --base REF       Baseline commit (default: master)
  --head REF       Commit to compare (default: the working tree, including
                   uncommitted changes)
  --filter REGEX   Only run cases matching REGEX (grep -E), e.g. '^load/'
  --mode MODE      callgrind (default if valgrind is installed) or time
  --static         Build static libraries (default: shared)
  --lto            Build libcbor and the harness with LTO; with --static,
                   this is whole-program LTO (default: off)
  --out DIR        Keep builds and results in DIR (default: a temporary
                   directory, removed on exit)
  --jobs N         Parallel callgrind runs (default: number of CPUs)
  --fail           Exit with status 1 if compare.py reports a regression
EOF
  exit 2
}

BASE=master
HEAD=
FILTER=.
MODE=
SHARED=ON
LTO=OFF
OUT=
JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
FAIL=

while [[ $# -gt 0 ]]; do
  case "$1" in
    --base) BASE="$2"; shift 2 ;;
    --head) HEAD="$2"; shift 2 ;;
    --filter) FILTER="$2"; shift 2 ;;
    --mode) MODE="$2"; shift 2 ;;
    --static) SHARED=OFF; shift ;;
    --lto) LTO=ON; shift ;;
    --out) OUT="$2"; shift 2 ;;
    --jobs) JOBS="$2"; shift 2 ;;
    --fail) FAIL=--fail; shift ;;
    *) usage ;;
  esac
done

if [[ -z "$MODE" ]]; then
  if command -v valgrind >/dev/null; then MODE="callgrind"; else MODE="time"; fi
fi
if [[ "$MODE" != callgrind && "$MODE" != time ]]; then usage; fi

BENCH_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO=$(git -C "$BENCH_DIR" rev-parse --show-toplevel)

if [[ -n "$OUT" ]]; then
  mkdir -p "$OUT"
  WORK=$(cd "$OUT" && pwd)
else
  WORK=$(mktemp -d)
  trap 'rm -rf "$WORK"' EXIT
fi

log() { echo "[compare] $*" >&2; }

# Export a commit's tree, or use the working tree when REF is empty.
source_tree() {
  local side=$1 ref=$2
  if [[ -z "$ref" ]]; then
    echo "$REPO"
    return
  fi
  local dir="$WORK/src-$side"
  rm -rf "$dir"
  mkdir -p "$dir"
  git -C "$REPO" archive "$ref" | tar -x -C "$dir"
  echo "$dir"
}

build_side() {
  local side=$1 src=$2
  local lib="$WORK/lib-$side" prefix="$WORK/prefix-$side"
  log "$side: building libcbor from $src"
  cmake -S "$src" -B "$lib" -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS="$SHARED" \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE="$LTO" \
    -DWITH_TESTS=OFF -DWITH_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX="$prefix" \
    >"$WORK/cmake-$side.log"
  cmake --build "$lib" --parallel >>"$WORK/cmake-$side.log"
  cmake --install "$lib" >>"$WORK/cmake-$side.log"
  log "$side: building the harness"
  cmake -S "$BENCH_DIR" -B "$WORK/bench-$side" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION="$LTO" \
    -DCMAKE_PREFIX_PATH="$prefix" >>"$WORK/cmake-$side.log"
  cmake --build "$WORK/bench-$side" >>"$WORK/cmake-$side.log"
}

# Prints "case<TAB>instructions<TAB>N" for one case.
callgrind_case() {
  local bin=$1 out=$2 name=$3
  local file="$out/${name//\//_}.callgrind"
  valgrind --tool=callgrind --toggle-collect='bench_measured*' \
    --callgrind-out-file="$file" "$bin" run "$name" >/dev/null 2>"$file.log"
  local total
  total=$(awk '/^totals:/ { print $2; exit }' "$file")
  if [[ -z "$total" ]]; then
    echo "callgrind produced no totals for $name, see $file.log" >&2
    return 1
  fi
  printf '%s\tinstructions\t%s\n' "$name" "$total"
}
export -f callgrind_case

library_size() {
  local prefix=$1 file
  # LTO objects hold compiler IR, not code.
  [[ "$LTO" == OFF ]] || return 0
  if [[ "$SHARED" == ON ]]; then
    file=$(find "$prefix" -name 'libcbor.so*' -type f -o -name 'libcbor*.dylib' -type f | head -n 1)
  else
    file=$(find "$prefix" -name 'libcbor.a' | head -n 1)
  fi
  [[ -n "$file" ]] || return 0
  # Berkeley format; the last line holds the (total) text size.
  local text
  text=$(size -t "$file" 2>/dev/null | awk 'END { print $1 }') || return 0
  if [[ "$text" =~ ^[0-9]+$ ]]; then
    printf 'library\ttext_bytes\t%s\n' "$text"
  fi
}

measure_side() {
  local side=$1
  local bin="$WORK/bench-$side/libcbor_bench"
  local results="$WORK/results-$side.tsv"
  # Shared builds: load this side's library, wherever it was installed.
  local libdir
  libdir=$(dirname "$(find "$WORK/prefix-$side" \( -name 'libcbor.so*' \
    -o -name 'libcbor*.dylib' -o -name 'libcbor.a' \) | head -n 1)")
  export LD_LIBRARY_PATH="$libdir" DYLD_LIBRARY_PATH="$libdir"
  log "$side: checking the harness"
  "$bin" check
  local cases
  cases=$("$bin" list | cut -f1 | grep -E -- "$FILTER" || true)
  if [[ -z "$cases" ]]; then
    log "no cases match $FILTER"
    exit 2
  fi
  local count
  count=$(echo "$cases" | wc -l | tr -d ' ')
  library_size "$WORK/prefix-$side" >"$results"
  log "$side: measuring allocations ($count cases)"
  # shellcheck disable=SC2086
  "$bin" alloc $cases >>"$results"
  if [[ "$MODE" == callgrind ]]; then
    log "$side: counting instructions with callgrind ($JOBS jobs)"
    local out="$WORK/callgrind-$side"
    mkdir -p "$out"
    echo "$cases" | xargs -P "$JOBS" -I{} \
      bash -c 'callgrind_case "$@"' _ "$bin" "$out" {} >>"$results"
  else
    log "$side: timing"
    # shellcheck disable=SC2086
    "$bin" time $cases >>"$results"
  fi
}

compiler_version() {
  local cc
  cc=$(sed -n 's/^CMAKE_C_COMPILER:[A-Z]*=//p' "$WORK/lib-head/CMakeCache.txt")
  "${cc:-cc}" --version | head -n 1
}

BASE_SRC=$(source_tree base "$BASE")
HEAD_SRC=$(source_tree head "$HEAD")
build_side base "$BASE_SRC"
build_side head "$HEAD_SRC"
measure_side base
measure_side head

if [[ -n "$HEAD" ]]; then
  HEAD_LABEL="$HEAD ($(git -C "$REPO" rev-parse --short "$HEAD"))"
elif ! git -C "$REPO" diff --quiet HEAD; then
  HEAD_LABEL="working tree ($(git -C "$REPO" rev-parse --short HEAD) + changes)"
elif [[ -z "$HEAD" ]]; then
  HEAD_LABEL="working tree ($(git -C "$REPO" rev-parse --short HEAD))"
fi
python3 "$BENCH_DIR/compare.py" $FAIL \
  --base-label "$BASE ($(git -C "$REPO" rev-parse --short "$BASE"))" \
  --head-label "$HEAD_LABEL" \
  --config "$MODE, $([[ $SHARED == ON ]] && echo shared || echo static), LTO $LTO, $(compiler_version)" \
  "$WORK/results-base.tsv" "$WORK/results-head.tsv" | tee "$WORK/report.md"
