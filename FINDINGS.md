# Findings report — libexpat 2.8.4 BiSheng C port

Base: upstream `master` d9087a1c (libexpat 2.8.4, no commits since).
Branch: `bsc-port` on `git@github.com:Mrgoudan/libexpat.git`.
Every result below comes from a real compiler or sanitizer run; nothing is inferred.

## Summary

| | Result |
|---|---|
| Confirmed real defects | 2 (both new, neither previously reported) |
| Confirmed exploitable CVEs | 0 |
| Candidates raised and argued down | 15 |
| Fuzz executions, all clean | ~340 million (ASan+UBSan, halt-on-UB, slowness-directed) |
| Side finding | 1 reproducible BiSheng C compiler crash |

Both confirmed defects are ones the current port does **not** flag, each for a documented
reason. For each, real compiler runs show which annotation change makes the checker reject it.

---

## Defect 1 — `dtdCopy` stores a parent-pool pointer as a child-table key

| | Location |
|---|---|
| Raw upstream | `xmlparse.c:7826` (key taken), `xmlparse.c:7840` (key stored) |
| Our BSC port | `expat/lib/xmlparse.c:8090` and `:8104` |
| Report | `expat/bsc-tools/report-defaultAttForName-aliasing.md`, `expat/bsc-tools/exposure/defect-explained.md` |
| PoC | `expat/bsc-tools/poc16.c` |

### The code

```c
const XML_Char *const attributeName = oldE->defaultAtts[i].id->name;   /* points into oldDtd->pool */
...
NAME_AND_DEFAULT_ATTRIBUTE *const nameAndDefaultAttribute
    = (NAME_AND_DEFAULT_ATTRIBUTE *)lookup(
        parser, &(newE->defaultAttForName), attributeName,             /* stored as newDtd's key */
        sizeof(NAME_AND_DEFAULT_ATTRIBUTE));
```

`lookup` on the insert path stores the key pointer verbatim (`xmlparse.c:8100`:
`table->v[i]->name = name;`). Every sibling table in the same function copies its key into the
new pool first (`xmlparse.c:7746`, `:7767`, `:7796`, all `poolCopyString(&(newDtd->pool), ...)`).
Only `defaultAttForName` skips the copy, so only its keys can dangle.

### Classification

Temporal memory-safety bug: CWE-825 expired-pointer dereference / CWE-416 use-after-free.
The dereference is a **read**, inside `keyeq()`, when the child later resolves an attribute in
`storeAtts`. It is a latent dangling alias at copy time; it becomes an out-of-lifetime access
only once the parent DTD's pool is freed.

### Reachability — not exploitable

Reaching the freed read requires resetting or freeing the **parent** parser before the **child**
external-entity parser, which the API docs forbid ("free subparsers prior to freeing their
related parent parser"). The PoC confirms this: the reset variant does not fault, and the
free-first variant faults in `getRootParserOf`, i.e. the generic parent-reference contract, not
on this key. Adjacent to the recently fixed CVE-2026-76641 in the same loop, but distinct.

### Why our port does not flag it, and what would

Hash keys stay raw `KEY` (Category C: interior pointers into pool blocks) because BiSheng C
struct `_Borrow` fields cannot borrow another heap object's storage (manual §3.2.6). Typing keys
as borrows into the owning pool is the rewrite that exposes it. Real runs on the model:

```
$ clang -x bsc <incs> pool_wrong.cbs <safe.cbs>
error: cannot move out of 'oldDtd' because it is borrowed      # rejected at COMPILE time

$ clang -O0 pool_wrong_c.c                                     # identical logic, annotations erased
                                                               # compiles clean, rc=0
$ clang -fsanitize=address pool_wrong_c.c && ./a.out
heap-use-after-free in key_first                               # caught only at RUN time
```

Files: `expat/bsc-tools/exposure/pool_wrong.cbs`, `pool_wrong_c.c`, `uaf_wrong.cbs`, `uaf_right.cbs`.

---

## Defect 2 — null-pointer arithmetic in the `XML_CONTEXT_BYTES == 0` epilogue

| | Location |
|---|---|
| Raw upstream | `xmlparse.c:2418` |
| Our BSC port | `expat/lib/xmlparse.c:2542` |
| Report | `expat/bsc-tools/exposure/defect2-null-buffer-arithmetic.md` |
| PoC | `expat/bsc-tools/exposure/poc_c1_nullbuffer.c` |

### The code

```c
    nLeftOver = s + len - end;
    if (nLeftOver) {
      /* ... only here is XML_GetBuffer() called, i.e. only here is
         parser->m_buffer ever allocated on this path ... */
      memcpy(parser->m_buffer, end, nLeftOver);
    }
    parser->m_bufferPtr = parser->m_buffer;
    parser->m_bufferEnd = parser->m_buffer + nLeftOver;   /* 2418: NULL + 0 when nLeftOver == 0 */
```

### Trigger and evidence

Build with `EXPAT_CONTEXT_BYTES=0` (a supported option), then make the first `XML_Parse` call
with `isFinal = 0` on input that is fully consumed:

```c
XML_Parser p = XML_ParserCreate(NULL);
XML_Parse(p, "<r>", 3, 0);      /* isFinal = 0 */
```

```
$ ./poc '<r>'
xmlparse.c:2418:44: runtime error: applying zero offset to null pointer
SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior xmlparse.c:2418:44

[instrumented] reached context0 epilogue: m_buffer=(nil) nLeftOver=0
```

The `isFinal = 1` case returns earlier and never reaches the line, which is why the defect needs
a non-final parse. This cost me four failed reproduction attempts before instrumentation showed
the branch was never entered.

### Classification and severity

Undefined behaviour per C17 6.5.6 (pointer arithmetic on a null pointer). Benign on mainstream
compilers: no trap, no out-of-bounds, no exploitable consequence. It sits on the ordinary
streaming code path in that build configuration.

### Why nobody has seen it

Only present in the non-default `EXPAT_CONTEXT_BYTES=0` build. The test suite and every fuzzer
here build with 1024, where the identical input is clean. That is precisely the blind spot.

### Suggested fix

```c
    parser->m_bufferEnd = parser->m_buffer ? parser->m_buffer + nLeftOver : NULL;
```

### Why our port does not flag it, and what would

`m_buffer` was deliberately left a raw `char *` (`expat/lib/xmlparse.c:764`) as a Category-C
parse window with four cursor aliases, allocated by plain `.malloc_fcn` rather than the tracked
allocator. Annotating it `_Nullable` turns the defect into a compile error. Real runs:

| Construct | BiSheng C result |
|---|---|
| `base + n` where `base` is `char *_Borrow _Nullable` | `error: invalid operands to binary expression` |
| `*p->buf` unguarded, `buf` possibly null | `error: cannot dereference possibly-null status pointer 'buf'` |
| the same dereference after a null check | accepted |

Files: `expat/bsc-tools/exposure/c1_wrong.cbs`, `c1_deref.cbs`.

---

## Side finding — BiSheng C compiler crash

Not a libexpat bug. The BiSheng C compiler (clang 15 fork) segfaults instead of emitting a
diagnostic on this input:

```c
struct PS { void (*fn)(int); int x; };
_Safe void f(struct PS *raw) {
  _Unsafe((&_Mut *raw)->fn(1));
}
```

It dies in `BSCBorrowChecker.cpp`, in `ActionExtract::VisitCallExpr` reached from
`RegionCheck::PopulateInference` / `Liveness::Walk`. Reading a plain data member the same way
does not crash, so it is specific to the function-pointer call node. Hit while porting
`XML_ParserFree` under `XML_GE == 0`, where the allocator macro expands to exactly this shape;
worked around with `_Safe` inline allocator wrappers taking `T *_Borrow`.

Reproducer: `expat/bsc-tools/compiler-crash/`.

---

## What was ruled out

Fifteen further candidates were raised by invariant reading and argued down to invariants of the
xmlrole state machine, the entity stack, or live asserts. The load-bearing ones:

- NULL `m_openInternalEntities` on the internal-encoding path (the CVE-2024-50602 shape).
- Tag stack dereference at end tags.
- Declaration state at attribute defaults, and `m_groupConnector[level]` bounds.
- `attIndex < nDefaultAtts` for the isCdata hash lookup (the CVE-2026-66046 fix).
- `storeRawNames` and `storeAtts` buffer reallocs (both rebase every outstanding pointer).
- Unterminated-key `keyeq`, `normalizePublicId`/`normalizeLines`, char-ref encode bounds.

Dynamic evidence supporting these: roughly 340 million executions across seven harnesses
(external entities with suspend/resume, unknown-encoding callbacks with 1-8 byte chunks,
UTF-8/UTF-16/ISO-8859-1 pinned parsers, a halt-on-first-UB build, and a slowness-directed
fuzzer) with zero crashes, zero UBSan errors and zero slow units; a full-corpus replay under
`-fno-sanitize-recover=all` returned clean; and ten adversarial input families scaled linearly
under a 1x/2x/4x/8x sweep, so there is no quadratic on direct input.

## On the live upstream CVE

The one publicly-listed unfixed vulnerability against this code is CVE-2025-66382 (issue #1076),
a denial of service where a ~2 MiB file costs 25 to 100 seconds. Its trigger is confidential
under the maintainer's NDA. The slowness-directed fuzzer and the scaling sweep were built to
rediscover it blind; everything measured linear, so it was not reproduced.
