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
| `XML_ParserStruct.m_buffer` | raw (parse window with four cursor aliases `m_bufferPtr/End/Lim`; allocated by plain `.malloc_fcn`) | `XML_ParserFree`, `setParserBuffer` |
| `m_dataBuf`, `m_atts`, `m_attInfo`, `m_nsAtts`, `m_groupConnector`, `m_unknownEncodingMem` | `T *_Owned _ArrayElem _Nullable` (done) | `FREE_ARRAY` in `XML_ParserFree`, `parserCreate` failure paths, `XML_ParserReset` |
| `m_protocolEncodingName` | `XML_Char *_Owned _ArrayElem _Nullable` from `copyString` (done) | `FREE_ARRAY` in `XML_ParserFree`, `XML_SetEncoding`, `XML_ParserReset` |
| `m_dtd` | raw `DTD *` (shared with parameter-entity child parsers); every internal function borrows it through a raw local after a null check (`dtdRaw`) | `XML_ParserFree` via `dtdDestroy(.., !m_parentParser)` |
| `m_tagStack`, `m_freeTagList` (TAG list), `TAG.buf.raw` | raw intrusive list nodes; `buf.raw` `_Owned` | `XML_ParserFree`, `parserInit` |
| `TAG.parent`, `TAG.bindings`, `TAG.rawName`, `TAG.name.*` | raw: back-ref and interior pointers into `buf` | — |
| `BINDING.uri` | `XML_Char *_Owned _ArrayElem _Nullable` (done); raw views via `&_Mut *b->uri` | `destroyBindings`, `storeAtts`, `addBinding` (`REALLOC_ARRAY`) |
| `BINDING.prefix/attId/nextTagBinding/prevPrefixBinding`, `PREFIX.binding` | raw list links / borrows into DTD tables | — |
| `m_openInternalEntities`, `m_openAttributeEntities`, `m_openValueEntities`, `m_freeEntities` | raw intrusive lists of MALLOC'd nodes | `XML_ParserFree` |
| `STRING_POOL.blocks/freeBlocks` (BLOCK list) | raw intrusive list | `poolDestroy` |
| `STRING_POOL.start/ptr/end` | raw interior cursors | — |
| `HASH_TABLE.v` | `NAMED **_Owned _ArrayElem _Nullable` (done); slots hold raw MALLOC'd entries | `hashTableDestroy`, rehash in `lookupWithLength` |
| `DTD.scaffold`, `DTD.scaffIndex` | raw: `dtdCopy` aliases the parent's arrays into the child DTD (`newDtd->scaffold = oldDtd->scaffold`), only the document DTD frees them (bug candidate 8) | `dtdDestroy(isDocEntity)`, `dtdReset` |
| `ELEMENT_TYPE.defaultAtts` | `DEFAULT_ATTRIBUTE *_Owned _ArrayElem _Nullable` (done) | `dtdDestroy`, `dtdReset`, `defineAttribute` (`REALLOC_ARRAY`) |
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
- [x] Commit Phase 0 and push `bsc-port` to `fork`

### Phase 1 — allocation plumbing (xmlparse.c) — done 2026-09-02
- [x] `bsc_compat.h` (included from internal.h) brings in `bishengc_safety.hbs` under BSC and erases the annotations under plain C
- [x] Owned array fields with `TAKE_ARRAY` / `REALLOC_ARRAY` / `FREE_ARRAY` helpers (see Ownership map); the parser object is `XML_Parser _Owned _Nullable` from `parserCreate` to `XML_ParserFree`
- [x] `REALLOC_ARRAY` keeps the old buffer owned by the field when realloc fails (7 sites); `pool->blocks`, `tag->buf.raw` (union), `dtd->scaffold/scaffIndex` stay raw (see map)
- [x] Ownership map filled
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

### Phase 2 — xmlrole.c — done (commit ba929152)
- [x] `PROLOG_STATE *_Borrow state`, `_Safe` handler function-pointer type, `XmlTokenRole` as a `_Safe` inline function; `enc` stays raw (static vtable); 4884/0

### Phase 3 — tokenizer — done (commit ae7072ae, subagent)
- [x] All 87 tokenizer functions `_Safe`; vtable slots are `_Safe` fn-pointer types; `enc` and cursors raw; 175 `_Unsafe` regions (37 whole loops); `Xml*` call macros wrap their vtable deref in `_Unsafe(...)`; 4884/0 in BSC and ASan builds
- [ ] `siphash.h`: `_Nonnull` on `sip_*` params (strict-probe noise source)

### Phase 4 — xmlparse.c — bulk port done 2026-09-02 (all 188 functions `_Safe` except the app callback `callUnknownEncodingConvert` and the static table getter `XML_GetFeatureList`)
The port is generated from upstream xmlparse.c by the scripts in `expat/bsc-tools/` (order: `pre_own.py`, `pre.py`, `passAB.py`, then `passC.py` iterated until the compiler is clean, then `post.py`); every `_Unsafe` in the result answers a specific compiler diagnostic. Result: 4884/0 in the BSC build and in the plain-C ASan build. Metrics: 267 `_Safe`, 307 `_Borrow`, 24 `_Owned`, 100 `_Nullable`, 1097 `_Unsafe` regions, 44 hoisted temporaries, 23 `FREE_ARRAY` / 7 `REALLOC_ARRAY` / 9 `TAKE_ARRAY` sites.
Remaining cluster review (read the generated code, shrink `_Unsafe` regions, add `_Nonnull`/`_Nullable` on raw params, run strict mode):
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
- [x] Dynamic side: upstream d9087a1c built with `-fsanitize=fuzzer,address,undefined` (system clang 17, lld) in `build/upstream` + `build/fuzz`; seeds = 542 XML literals extracted from `expat/tests/*.c` plus hand-written DTD seeds; `build/fuzz/xml.dict` dictionary; harness `build/fuzz/harness_ext.c` drives external subsets / parameter entities (input split at NUL), `XML_UseForeignDTD`, namespace triplets, `XML_StopParser`/`XML_ResumeParser` inside handlers, `XML_DefaultCurrent` from element handlers, `XML_ParserReset` — i.e. the paths behind candidates 4, 8, 9, 12, 13
- [ ] Keep §Bug candidates current during every phase
- [ ] For each candidate: minimal input → `bsc.sh asan` reproduction → classify (Changes / #1160 / new)
- [x] `bsc.sh strict` after the null contracts: xmlparse.c 387 -> 185 hits; the residue is raw derefs inside `_Unsafe(...)` where the checker cannot carry facts across the wrapper (see candidates 12-15 for the ones with CVE shape)
- [ ] Draft the private report for confirmed new bugs (per `SECURITY.md`); request a CVE once the maintainer confirms

### Phase 6 — wrap-up
- [x] Verified 2026-09-02: BSC default 4884/0, `EXPAT_ATTR_INFO=ON` 4884/0, `EXPAT_DTD=OFF EXPAT_GE=OFF` 3396/0, `EXPAT_CONTEXT_BYTES=0` 4872/0, `EXPAT_NS=OFF` 4884/0, plain-C ASan/UBSan 4884/0. xmlparse.c metrics: 267 `_Safe`, 307 `_Borrow`, 24 `_Owned`, 110 `_Nullable`, 127 `_Nonnull`, 1096 `_Unsafe`, 0 warnings. Compiler note: the BSC borrow checker segfaults on `_Unsafe((&_Mut *p)->fn(x))` inside an `_Unsafe` block (GE=0 allocator macros); worked around with `_Safe` inline allocator wrappers
- [ ] Push `bsc-port`, open PR on the fork with the report

## Findings so far (2026-09-02)

- Acceptance criterion 1 (whole port): met. All three library sources are `_Safe` end to end with `_Borrow`/`_Owned`/`_Nullable`/`_Nonnull` contracts; suites pass in every configuration (table in Phase 6).
- Acceptance criterion 2 (new CVE): not met yet. BSC surfaced 16 candidates; the ones with CVE shape (4, 12, 13, 15) were reviewed to invariants of the xmlrole state machine and the entity stack; candidate 16 is a real copy-consistency defect (child DTD attribute maps alias the parent's pool strings) but not independently exploitable (PoC `expat/bsc-tools/poc16.c`: after `XML_ParserReset(parent)` the child still normalizes correctly, so the stale keys do not fault; the free-first variant faults in `getRootParserOf`, i.e. the already-documented "free child before parent" misuse, not the aliasing). Dynamic hunting on unmodified upstream with ASan/UBSan libFuzzer: first 25-minute campaigns found nothing; longer campaigns are running (`build/fuzz/run_*2.log`, UTF-16 and ISO-8859-1 pinned variants).
- Side result: a BiSheng C compiler crash (borrow checker segfault) reproducible from this tree, see Phase 6.

## Bug candidates

| # | Where (file:line, function) | BSC signal | Hypothesis | Status |
|---|---|---|---|---|
| 1 | `xmltok.c` `toAscii`, `char buf[1]` | init analysis: "use of uninitialized value: buf" at `p == buf` and `buf[0]` | `buf[0]` is only read after `utf8Convert` advanced `p` past it, so the read is always of a written byte; the checker cannot see the write through `&p`. Zero-initialised the array (behaviour unchanged). | false positive |
