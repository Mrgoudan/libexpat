#!/usr/bin/env bash
# Regenerate expat/lib/xmlparse.c from the pre-port base and drive passC over every build configuration.
set -u
T=$(cd "$(dirname "$(readlink -f "$0")")" && pwd)
REPO=/home/ziruichen/bsd/libexpat
F=$REPO/expat/lib/xmlparse.c
BSC=/home/ziruichen/bsd/llvm-project-dup/build/bin/clang
LIBCBS=/home/ziruichen/bsd/llvm-project-dup/libcbs/src
CONFIGS="bsc bsc-attrinfo bsc-nodtd bsc-context0"
comp() { # $1 = config dir
  $BSC -x bsc -fsyntax-only -ferror-limit=0 -DHAVE_EXPAT_CONFIG_H -I$REPO/build/$1 -I$REPO/expat/lib \
    -I$LIBCBS/bishengc_safety -Wno-nullability-completeness $F > $REPO/build/passC.log 2>&1
  grep -c ' error: ' $REPO/build/passC.log
}
git -C $REPO show ba929152:expat/lib/xmlparse.c > $F || exit 1
python3 $T/pre_own.py && python3 $T/pre.py && python3 $T/passAB.py || exit 1
for round in 1 2 3; do
  for cfg in $CONFIGS; do
    for i in 1 2 3 4 5 6 7 8; do
      n=$(comp $cfg)
      [ "$n" = 0 ] && break
      python3 $T/passC.py $F $REPO/build/passC.log > /dev/null
    done
    echo "round $round $cfg: $(comp $cfg) errors"
  done
done
python3 $T/post.py $F
for cfg in $CONFIGS; do
  echo "final $cfg: $(comp $cfg) errors, $(grep -c 'warning:' $REPO/build/passC.log) warnings"
  grep -A2 ' error: ' $REPO/build/passC.log | head -12
done
