# libexpat → BiSheng C port

This fork ports `expat/lib` to BiSheng C (BSC) in place. Files keep their `.c`/`.h`
names and are compiled with `-x bsc`. Plan, todo list and bug log: [BSC_PORT.md](BSC_PORT.md).
Load `/c-to-bsc` before touching library sources; `/bsc-design` before changing any
struct field qualifier or function signature.

## Port rules (agreed with the user, override skill defaults where they conflict)

- Features in scope: `_Safe` zones, `_Borrow`, `_Owned`, nullability and init analysis.
- No redesign: no `_Owned struct`, no libcbs containers, no member functions, no `-fbsc-experimental`.
  Keep expat's own structs; only add qualifiers to existing pointer fields/params/returns.
- Keep the `XML_Memory_Handling_Suite` allocator hooks (public API, exercised by the tests).
  `MALLOC/REALLOC/FREE` stay; ownership enters via `__take_from_raw` right after the
  allocator call and leaves via `__move_to_raw` right before `FREE`.
- Every `_Unsafe` must be justified by a real compiler diagnostic. Before silencing any
  diagnostic, ask whether it is a real bug and log it in BSC_PORT.md §Bug candidates.
- Never speculate about compiler behaviour: run the compiler.

## BSC Project Compile Command

```
Compiler: /home/ziruichen/bsd/llvm-project-dup/build/bin/clang   (custom clang 15, NOT system clang)
Libcbs:   /home/ziruichen/bsd/llvm-project-dup/libcbs/src
Helper:   /home/ziruichen/bsd/libexpat/expat/bsc.sh {configure|build|test|check <file>|strict|asan|all}

Verify one file (after `bsc.sh configure` has produced build/bsc/expat_config.h):
  /home/ziruichen/bsd/llvm-project-dup/build/bin/clang -x bsc -fsyntax-only \
      -DHAVE_EXPAT_CONFIG_H -I/home/ziruichen/bsd/libexpat/build/bsc \
      -I/home/ziruichen/bsd/libexpat/expat/lib \
      -I/home/ziruichen/bsd/llvm-project-dup/libcbs/src/bishengc_safety \
      -Wno-nullability-completeness /home/ziruichen/bsd/libexpat/expat/lib/<file>.c

Full build + tests (library as BSC, tests as plain C):
  /home/ziruichen/bsd/libexpat/expat/bsc.sh all
  (= cmake -DEXPAT_BSC=ON -DEXPAT_BSC_LIBCBS=<libcbs/src> ... ; make ; tests/runtests)
Expected: "Checks: 4884, Failed: 0" (baseline from upstream d9087a1c, expat 2.8.4).
```

## Code Verification

After editing any file under `expat/lib`, run the per-file verify command, then
`expat/bsc.sh all`. Fix every diagnostic before reporting the task complete. Report the
exact commands run and their exit codes.
