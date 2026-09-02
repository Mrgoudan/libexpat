# libexpat → BiSheng C: plan, workflow, todos, bug log

Branch `bsc-port` on fork `git@github.com:Mrgoudan/libexpat.git` (remote `fork`).
Base: upstream `master` d9087a1c (expat 2.8.4, 2026-08-31).

## Goal and acceptance criteria

1. **Whole port**: `expat/lib/{xmlparse,xmltok,xmlrole}.c` (+ the `xmltok_impl.c`,
   `xmltok_ns.c` templates and internal headers) compile as BSC with `_Safe` functions,
   `_Owned`/`_Borrow` on every pointer in every signature and owning struct field, and the
   existing suite passes: `Checks: 4884, Failed: 0`.
2. **Find an unfound CVE**: at least one memory-safety or logic bug that BSC's checkers
   (borrow, nullability, init) surface during the port, reproduced on the plain-C ASan
   build, and not already listed in `expat/Changes`. A bug matching an open entry of
   upstream issue #1160 (unfixed, non-public) also counts.

Constraints: no `_Owned struct`, no libcbs containers, no member functions, no
`-fbsc-experimental`; keep expat's structs and the `XML_Memory_Handling_Suite` hooks.

## Toolchain (done)

| Item | Value |
|---|---|
| BSC compiler | `/home/ziruichen/bsd/llvm-project-dup/build/bin/clang` (clang 15 fork) |
| libcbs | `/home/ziruichen/bsd/llvm-project-dup/libcbs/src` |
| CMake switch | `-DEXPAT_BSC=ON -DEXPAT_BSC_LIBCBS=<libcbs/src> [-DEXPAT_BSC_CHECKS="..."]` (lib only; tests stay C) |
| Helper | `expat/bsc.sh {configure,build,test,check <file>,strict,asan,all}`; build trees under `build/` (git-ignored) |
| Baseline | plain C: 4884/0. BSC mode, unmodified sources: 4884/0 after `operator`→`heapOp` rename |
| Strict probe | `bsc.sh strict` (`-nullability-check=all -uninit-check=all`, unported code): 4584 errors. Per file: xmlparse.c 2086, xmltok_impl.c 1852, xmltok.c 356, xmlrole.c 179, siphash.h 93, xmltok_ns.c 18. Per kind: 4517 "possibly-null status pointer" (unannotated params, noise until `_Nonnull`/`_Nullable` is added), 67 "use of (possibly) uninitialized value" (first triage list, see Phase 1). |

## Workflow

### Per-function loop (the port)

1. Pick the next function from the cluster order below. Read it and every caller.
2. Annotate the signature: `_Safe`; each pointer param/return gets `_Owned`, `_Borrow`,
   `const _Borrow`, or stays raw with a one-line reason (cursor, `T **` out-param,
   function pointer). Add `_Nullable` where NULL is a legal value.
3. Body: `&x` → `&_Const x`/`&_Mut x`, `NULL` → `nullptr`, allocator result →
   `__take_from_raw`, free → `__move_to_raw` + `FREE`.
4. `expat/bsc.sh check <file>`. For each error the compiler prints:
   a. Ask whether the input the checker describes can actually happen at runtime
      (a NULL from `lookup()` on a miss, a `TAG` popped after `REALLOC` of its buffer,
      a field never written in `parserInit`). If yes, add a row to §Bug candidates
      before changing the code.
   b. Fix the annotation or restructure the statement so it compiles in `_Safe`.
   c. Only then wrap the single failing statement in `_Unsafe`.
5. `expat/bsc.sh all` must stay at 4884/0. Commit per cluster on `bsc-port`.

### Bug-hunting loop (runs alongside the port)

Signal sources, strongest first:
- **Nullability**: a `_Safe` function dereferences a value the checker proves `_Nullable`
  (allocator result, `lookup()` miss, unset handler, `m_parentParser`, `m_dtd`).
- **Borrow checker**: a `_Borrow` still live across `REALLOC`/`poolGrow`/`FREE` of its
  owner (the expat UAF shape of CVE-2022-40674, CVE-2022-43680, CVE-2024-45491).
- **Init analysis**: a struct field read before it is written (`parserInit`, `dtdReset`,
  `TAG`/`BINDING` taken from a free list, `OPEN_INTERNAL_ENTITY` recycling).
- **Ownership audit**: a field with two owners or none (`m_dtd` shared with child parsers,
  `XML_Content` handed to the application, `m_buffer` allocated outside `MALLOC` tracking).

For each candidate: write a minimal XML input (or a `runtests`-style test), run it on
`expat/bsc.sh asan`, then classify: already fixed (`expat/Changes`), open in #1160, or new.
New findings go privately to the maintainer per `SECURITY.md` before any public commit
mentions them.

## Ownership map (fill in while porting; the free sites are the source of truth)

| Struct / field | Decision | Freed by |
|---|---|---|
| `XML_ParserStruct.m_buffer` | `char *_Owned _Nullable` (raw `.malloc_fcn`, not MALLOC) | `XML_ParserFree`, `XML_GetBuffer` |
| `m_dataBuf`, `m_atts`, `m_attInfo`, `m_nsAtts`, `m_groupConnector`, `m_unknownEncodingMem` | `_Owned _Nullable` | `XML_ParserFree`, `parserCreate` on allocation failure |
| `m_protocolEncodingName` | `const XML_Char *_Owned _Nullable` (freed through a cast) | `XML_ParserFree`, `XML_SetEncoding`, `XML_ParserReset` |
| `m_dtd` | owned unless `m_isParamEntity` (child shares the parent's DTD) | `XML_ParserFree` via `dtdDestroy(.., !m_parentParser)`; decide raw+`_Unsafe` vs. explicit flag |
| `m_tagStack`, `m_freeTagList` (TAG list), `TAG.buf.raw` | raw intrusive list nodes; `buf.raw` `_Owned` | `XML_ParserFree`, `parserInit` |
| `TAG.parent`, `TAG.bindings`, `TAG.rawName`, `TAG.name.*` | raw: back-ref and interior pointers into `buf` | — |
| `BINDING.uri` | `XML_Char *_Owned _Nullable` | `destroyBindings`, `addBinding` |
| `BINDING.prefix/attId/nextTagBinding/prevPrefixBinding`, `PREFIX.binding` | raw list links / borrows into DTD tables | — |
| `m_openInternalEntities`, `m_openAttributeEntities`, `m_openValueEntities`, `m_freeEntities` | raw intrusive lists of MALLOC'd nodes | `XML_ParserFree` |
| `STRING_POOL.blocks/freeBlocks` (BLOCK list) | raw intrusive list | `poolDestroy` |
| `STRING_POOL.start/ptr/end` | raw interior cursors | — |
| `HASH_TABLE.v` | `NAMED **` `_Owned _Nullable` array; entries live in `dtd->pool` | `hashTableDestroy` |
| `DTD.scaffold`, `DTD.scaffIndex` | `_Owned _Nullable` | `dtdDestroy`, `dtdReset` |
| `ELEMENT_TYPE.defaultAtts` | `_Owned _Nullable` | `dtdDestroy`, `dtdReset` |
| `XML_Content` from `build_model` | `_Owned`, transferred to the application | app via `XML_FreeContentModel` |
| `ENCODING *m_encoding`, `m_internalEncoding` | static tables or `m_unknownEncodingMem`: likely raw | — |
| `m_parentParser`, `m_externalEntityRefHandlerArg` | raw back-refs | — |

## Todos

### Phase 0 — setup (done 2026-09-02)
- [x] Add fork remote, create branch `bsc-port`
- [x] Plain-C baseline build + tests (4884/0)
- [x] BSC toolchain located; `-x bsc` syntax check of the three core files
- [x] Rename `operator` param in `expat_heap_stat` (BSC keyword)
- [x] `EXPAT_BSC` CMake option (lib-only `-x bsc`, links `bishengc_safety.cbs`); BSC-mode tests 4884/0
- [x] `expat/bsc.sh` helper, `CLAUDE.md` compile command, this plan
- [ ] Commit Phase 0 and push `bsc-port` to `fork` (needs user go-ahead)

### Phase 1 — allocation plumbing (xmlparse.c)
- [ ] Add `#include "bishengc_safety.hbs"` to xmlparse.c; confirm `bsc.sh all` still 4884/0
- [ ] Wrap `MALLOC/REALLOC/FREE` and raw `.malloc_fcn` calls: `__take_from_raw` after alloc,
      `__move_to_raw` before free; cover the `expat_malloc`/`expat_realloc`/`expat_free` tracker (`XML_GE == 1`)
- [ ] `REALLOC` sites (xmlparse.c lines 1036, 3146, 3518, 3920, 4125, 4539, 6014, 6043, 8482): move old out, realloc, re-wrap old when realloc returns NULL
- [ ] Fill the Ownership map above for every field; note conditional owners
- [ ] Triage the 67 strict-mode init-analysis hits (line:variable, `?` = possibly uninitialized). Expect most to be locals initialised through a pointer (`sip24_init(&key)`, `hashTableIterInit(&iter)`), which need `__attribute__((ensure_init))` on the initialiser rather than a code change; anything left is a bug candidate:
      - `siphash.h` `sip24_valid`: 364:k, 370:in, 370:k
      - `xmlparse.c` `accountingDiffTolerated`: 8800:levelsAwayFromRootParser, 8828:levelsAwayFromRootParser
      - `xmlparse.c` `appendAttributeValue`: 6724:buf, 6735:buf
      - `xmlparse.c` `copyEntityTable`: 7892:iter, 7897:iter
      - `xmlparse.c` `doContent`: 3555:name, 3668:buf, 3669:buf
      - `xmlparse.c` `dtdCopy`: 7740:iter, 7743:iter, 7753:iter, 7760:iter, 7788:iter, 7793:iter
      - `xmlparse.c` `dtdDestroy`: 7706:iter, 7708:iter
      - `xmlparse.c` `dtdReset`: 7664:iter, 7666:iter
      - `xmlparse.c` `gather_time_entropy`: 1107:tv, 1116:tv.tv_usec
      - `xmlparse.c` `generate_hash_secret_salt`: 1152:entropy, 1153:entropy, 1172:entropy.k, 1180:entropy.k, 1181:entropy
      - `xmlparse.c` `getContext`: 7487:iter, 7489:iter, 7516:iter, 7518:iter
      - `xmlparse.c` `handleUnknownEncoding`: 4994:info, 5012:info.map
      - `xmlparse.c` `hash`: 7988:key, 7989:key, 7989:state, 7990:state, 7991:state
      - `xmlparse.c` `processEntity`: 6494?:openEntityList, 6495?:openEntityList
      - `xmlparse.c` `storeAtts`: 4149:sip_key, 4150:sip_key, 4150:sip_state, 4179:sip_state, 4184:sip_state, 4192:sip_state
      - `xmlparse.c` `storeEntityValue`: 7012:buf, 7022:buf
      - `xmltok.c` `toAscii`: 1043:buf, 1045:buf, 1048:buf
      - `xmltok.c` `unknown_toUtf8`: 1376:buf, 1379:buf
      - `xmltok_impl.c` `PREFIX`: 296:tok, 309:tok, 319:tok, 327:tok

### Phase 2 — xmlrole.c (54 functions, 1257 lines)
- [ ] `PROLOG_STATE *state` → `_Borrow`; `enc` → `const ENCODING *_Borrow`; `ptr/end` stay raw cursors
- [ ] Mark all handlers `_Safe`; run `bsc.sh check xmlrole.c`; log every error
- [ ] `bsc.sh all` 4884/0; commit "xmlrole.c: BSC port"

### Phase 3 — tokenizer: xmltok.c + xmltok_impl.c + xmltok_ns.c (~137 functions)
- [ ] `ENCODING` vtable: decide `const ENCODING *_Borrow` vs raw for `enc` (function-pointer table called through macros)
- [ ] `xmltok_impl.c` template: port once, verify the normal/big2/little2 instantiations compile
- [ ] `utf8Convert`/`utf16Convert` `fromP`/`toP` double pointers: raw with reason
- [ ] `XmlInitUnknownEncoding` / `unknown_*` over `m_unknownEncodingMem`: ownership
- [ ] `siphash.h`: `_Nonnull` on `sip_*` params (strict-probe noise source)
- [ ] `bsc.sh all` 4884/0; commit

### Phase 4 — xmlparse.c by cluster (188 functions, 9469 lines)
- [ ] 4a **string pool + hash table + copyString** (`pool*`, `hash`, `lookup`, `lookupWithLength`, `hashTable*`, `copyString`, `keylen`, `keyeq`) — CVE-2022-25314/25315/22825, CVE-2024-45492, CVE-2026-56408
- [ ] 4b **DTD lifecycle** (`dtdCreate/Reset/Destroy/Copy`, `copyEntityTable`, `getElementType`, `getAttributeId`, `defineAttribute`, `setElementTypePrefix`) — CVE-2024-45491, CVE-2022-22824, CVE-2026-56405
- [ ] 4c **tags + bindings + attributes** (`storeRawNames`, `storeAtts`, `addBinding`, `freeBindings`, `destroyBindings`, `moveToFreeBindingList`, `is_rfc3986_uri_char`) — CVE-2022-22822/22827, CVE-2026-56403/56404, CVE-2026-66046
- [ ] 4d **entities** (`processEntity`, `internalEntityProcessor`, `storeEntityValue`, `callStoreEntityValue`, `storeSelfEntityValue`, `moveEntityList`, `entityTracking*`) — CVE-2022-40674/43680, CVE-2024-8176, CVE-2026-50219/56131/56412
- [ ] 4e **content model** (`nextScaffoldPart`, `build_model`, `XML_FreeContentModel`) — CVE-2022-22823/22826, CVE-2026-56132
- [ ] 4f **processors** (`doContent`, `doProlog`, `doCdataSection`, `doIgnoreSection`, `epilogProcessor`, `*Processor`, `callProcessor`, `triggerReenter`) — largest functions; most raw cursors live here
- [ ] 4g **parser lifecycle + public API** (`parserCreate/Init`, `XML_ParserReset`, `XML_ParserFree`, `XML_ExternalEntityParserCreate`, `XML_Parse/ParseBuffer/GetBuffer/StopParser/ResumeParser`, `setParserBuffer`, `XML_SetEncoding`, `getContext/setContext`) — CVE-2024-45490/50602, CVE-2026-56406/76957
- [ ] 4h **misc** (`generate_hash_secret_salt`, entropy, accounting/heap tracker, `XML_ErrorString`, getters/setters)
- [ ] No bare `T *` left in a signature without a reason comment; every `_Unsafe` re-checked by deleting it and recompiling

### Phase 5 — bug hunting and CVE verification
- [ ] Keep §Bug candidates current during every phase
- [ ] For each candidate: minimal input → `bsc.sh asan` reproduction → classify (Changes / #1160 / new)
- [ ] Re-run `bsc.sh strict` on fully ported files and triage what remains
- [ ] Draft the private report for confirmed new bugs (per `SECURITY.md`); request a CVE once the maintainer confirms

### Phase 6 — wrap-up
- [ ] Final `bsc.sh all` (4884/0) and `bsc.sh asan` green; metrics: counts of `_Safe`, `_Owned`, `_Borrow`, `_Nullable`, `_Unsafe`
- [ ] Push `bsc-port`, open PR on the fork with the report

## Bug candidates

| # | Where (file:line, function) | BSC signal | Hypothesis | Status |
|---|---|---|---|---|
| 1 | `xmltok.c` `toAscii`, `char buf[1]` | init analysis: "use of uninitialized value: buf" at `p == buf` and `buf[0]` | `buf[0]` is only read after `utf8Convert` advanced `p` past it, so the read is always of a written byte; the checker cannot see the write through `&p`. Zero-initialised the array (behaviour unchanged). | false positive |
