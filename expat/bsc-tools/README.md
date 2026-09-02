# BSC port generators for xmlparse.c

`expat/lib/xmlparse.c` on this branch is produced from upstream xmlparse.c by:

1. `pre_own.py` — owned array fields and their allocation/free sites (`TAKE_ARRAY`, `REALLOC_ARRAY`, `FREE_ARRAY`)
2. `pre.py` — hand-written rewrites (parser lifecycle, root lookup, pool accessors, init contracts)
3. `passAB.py` — `_Safe` on every function, `_Borrow` parameters, `nullptr`, borrowed pool/table arguments, `_Unsafe` on callback and libc calls
4. `passC.py <file> <clang.log>` — compiler-driven: wraps the smallest statement or expression the compiler rejected, inserts `void *` casts, hoists field reads that conflict with a borrowed `parser`; iterate with a fresh `-ferror-limit=0` log until clean
5. `post.py` — statement-level `_Unsafe call(...);` for readability

`run_pipeline.sh` runs all of the above and drives `passC.py` against four configurations
(default, `EXPAT_ATTR_INFO=ON`, `EXPAT_DTD=OFF EXPAT_GE=OFF`, `EXPAT_CONTEXT_BYTES=0`), whose
build trees must exist under `build/bsc*`; every variant must compile clean before `post.py` runs.

Input: `git show ba929152:expat/lib/xmlparse.c` (upstream d9087a1c plus the BSC keyword rename and prolog-state borrows). Paths are absolute for this machine; adjust `p = ...` at the top of each script.

## fuzz/

libFuzzer harnesses used for the bug hunt, built against unmodified upstream sources:

```
clang -fsanitize=fuzzer,address,undefined -O1 -g -fuse-ld=lld -DHAVE_EXPAT_CONFIG_H \
      -I<build dir with expat_config.h> -Iexpat/lib \
      expat/lib/xmlparse.c expat/lib/xmltok.c expat/lib/xmlrole.c \
      expat/lib/random_getentropy.c expat/lib/random_getrandom.c expat/lib/random_dev_urandom.c \
      expat/bsc-tools/fuzz/harness_ext.c -o harness_ext
./harness_ext -dict=expat/bsc-tools/fuzz/xml.dict <corpus dir>
```

- `harness_ext.c`: external subsets and parameter entities (input split at NUL), foreign DTD, namespace triplets, stop/resume, `XML_DefaultCurrent`, reset.
- `harness_enc.c`: unknown-encoding callbacks, 1..8 byte `XML_GetBuffer`/`XML_ParseBuffer` chunks, stop/resume from every handler.
