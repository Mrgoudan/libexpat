#!/usr/bin/env bash
# Build/verify helper for the BiSheng C port. Usage: bsc.sh {configure|build|test|check <file>|strict|asan|all}
set -euo pipefail

REPO=$(cd "$(dirname "$(readlink -f "$0")")/.." && pwd)
SRC=$REPO/expat
BSC_CLANG=${BSC_CLANG:-/home/ziruichen/bsd/llvm-project-dup/build/bin/clang}
LIBCBS=${LIBCBS:-/home/ziruichen/bsd/llvm-project-dup/libcbs/src}
BUILD_ROOT=${BUILD_ROOT:-$REPO/build}
JOBS=${JOBS:-8}

COMMON_CMAKE=(-DEXPAT_BUILD_TESTS=ON -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TOOLS=ON -DEXPAT_SHARED_LIBS=OFF)

configure() {
  mkdir -p "$BUILD_ROOT/bsc"
  cmake -S "$SRC" -B "$BUILD_ROOT/bsc" -DCMAKE_C_COMPILER="$BSC_CLANG" \
    -DEXPAT_BSC=ON -DEXPAT_BSC_LIBCBS="$LIBCBS" "${COMMON_CMAKE[@]}" "$@"
}

build() {
  [ -f "$BUILD_ROOT/bsc/CMakeCache.txt" ] || configure
  make -C "$BUILD_ROOT/bsc" -j"$JOBS"
}

run_tests() {
  "$BUILD_ROOT/bsc/tests/runtests"
}

check() {
  [ -f "$BUILD_ROOT/bsc/expat_config.h" ] || configure >/dev/null
  local f=$1; shift
  case $f in /*) ;; *) f=$SRC/lib/$f ;; esac
  "$BSC_CLANG" -x bsc -fsyntax-only -DHAVE_EXPAT_CONFIG_H \
    -I"$BUILD_ROOT/bsc" -I"$SRC/lib" -I"$LIBCBS/bishengc_safety" \
    -Wno-nullability-completeness "$@" "$f"
}

# Strict mode: checkers also run outside _Safe. Expect noise on unported code.
strict() {
  mkdir -p "$BUILD_ROOT/bsc-strict"
  cmake -S "$SRC" -B "$BUILD_ROOT/bsc-strict" -DCMAKE_C_COMPILER="$BSC_CLANG" \
    -DEXPAT_BSC=ON -DEXPAT_BSC_LIBCBS="$LIBCBS" \
    -DEXPAT_BSC_CHECKS="-nullability-check=all -uninit-check=all -ferror-limit=0" \
    "${COMMON_CMAKE[@]}" >/dev/null
  make -C "$BUILD_ROOT/bsc-strict" -k -j1 expat 2>&1 | grep -E "error:|warning:" || true
}

# Plain C + ASan/UBSan build with system clang, for reproducing bug candidates.
asan() {
  mkdir -p "$BUILD_ROOT/asan"
  cmake -S "$SRC" -B "$BUILD_ROOT/asan" -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -O1" \
    -DEXPAT_BUILD_FUZZERS=OFF "${COMMON_CMAKE[@]}" >/dev/null
  make -C "$BUILD_ROOT/asan" -j"$JOBS"
  "$BUILD_ROOT/asan/tests/runtests"
}

cmd=${1:-all}; shift || true
case $cmd in
  configure) configure "$@" ;;
  build) build ;;
  test) run_tests ;;
  check) check "$@" ;;
  strict) strict ;;
  asan) asan ;;
  all) build && run_tests ;;
  *) echo "usage: $0 {configure|build|test|check <file>|strict|asan|all}" >&2; exit 2 ;;
esac
