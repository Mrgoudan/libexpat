# BSC port generators for xmlparse.c

`expat/lib/xmlparse.c` on this branch is produced from upstream xmlparse.c by:

1. `pre_own.py` — owned array fields and their allocation/free sites (`TAKE_ARRAY`, `REALLOC_ARRAY`, `FREE_ARRAY`)
2. `pre.py` — hand-written rewrites (parser lifecycle, root lookup, pool accessors, init contracts)
3. `passAB.py` — `_Safe` on every function, `_Borrow` parameters, `nullptr`, borrowed pool/table arguments, `_Unsafe` on callback and libc calls
4. `passC.py <file> <clang.log>` — compiler-driven: wraps the smallest statement or expression the compiler rejected, inserts `void *` casts, hoists field reads that conflict with a borrowed `parser`; iterate with a fresh `-ferror-limit=0` log until clean
5. `post.py` — statement-level `_Unsafe call(...);` for readability

Input: `git show ba929152:expat/lib/xmlparse.c` (upstream d9087a1c plus the BSC keyword rename and prolog-state borrows). Paths are absolute for this machine; adjust `p = ...` at the top of each script.
