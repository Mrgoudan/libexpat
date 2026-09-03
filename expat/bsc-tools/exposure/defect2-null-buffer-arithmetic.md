# Defect 2 — null-pointer arithmetic in the `XML_CONTEXT_BYTES == 0` parse epilogue

**Status: CONFIRMED, reproduced on pristine upstream libexpat 2.8.4. Not previously reported.**
Class: undefined behaviour (C17 6.5.6 — pointer arithmetic on a null pointer), CWE-476-adjacent.
Impact: benign on mainstream compilers (no trap, no OOB), but it is UB the standard does not
permit and it is on the ordinary streaming code path in that build configuration.

## Trigger

Build libexpat with `EXPAT_CONTEXT_BYTES=0` (a supported option), then make the very first
`XML_Parse()` call with `isFinal = 0` on input that is fully consumed:

```c
XML_Parser p = XML_ParserCreate(NULL);
XML_Parse(p, "<r>", 3, 0);      /* isFinal = 0 */
```

Real run against unmodified upstream sources:

```
$ clang -fsanitize=pointer-overflow,null,undefined -DHAVE_EXPAT_CONFIG_H \
        -I<config with XML_CONTEXT_BYTES 0> -I<expat/lib> <lib sources> poc_c1_nullbuffer.c -o poc
$ ./poc '<r>'
xmlparse.c:2418:44: runtime error: applying zero offset to null pointer
SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior xmlparse.c:2418:44
```

Instrumented run showing the state at that line:

```
[C1] reached context0 epilogue: m_buffer=(nil) nLeftOver=0
```

The same input in the **default** `XML_CONTEXT_BYTES=1024` build is clean, which is why
neither the test suite nor the fuzzers (which build with 1024) ever saw it.

## Why it happens

`m_buffer` is only allocated inside the `if (nLeftOver)` branch, via `XML_GetBuffer`.
On a first, fully-consumed, non-final parse `nLeftOver == 0`, so that branch is skipped and
`m_buffer` is still `NULL` when the epilogue does arithmetic on it. Note the `isFinal = 1`
case returns earlier and never reaches the line, which is why the defect needs `isFinal = 0`.

---

## UPPER HALF — with BiSheng C annotations (the defect is a compile error)

The single annotation that matters is that the parse-window base pointer may be null:

```c
struct P { char *_Owned _Nullable buf; };          // m_buffer: may be NULL

_Safe void epilogue_wrong(struct P *_Borrow p, size_t nLeftOver) {
  char *_Borrow _Nullable base = (p->buf == nullptr) ? nullptr : &_Mut *p->buf;
  char *_Borrow end = base + nLeftOver;            // <-- rejected
  (void)end;
}
```

```
$ clang -x bsc <incs> c1_wrong.cbs <safe.cbs>
c1_wrong.cbs:7:28: error: invalid operands to binary expression
                   ('char *_Borrow _Nullable' and 'size_t')
```

The type system refuses pointer arithmetic on a possibly-null pointer outright. The related
dereference form is rejected with a dedicated diagnostic, and the guarded form is accepted:

```c
_Safe char wrong(struct P *_Borrow p) { return *p->buf; }
//   error: cannot dereference possibly-null status pointer 'buf'
_Safe char right(struct P *_Borrow p) {
  if (p->buf == nullptr) return 0;
  return *p->buf;                                   // accepted
}
```

## LOWER HALF — the raw upstream C source (the defect is silent)

`xmlparse.c`, libexpat 2.8.4, inside `#if XML_CONTEXT_BYTES == 0` in `XML_Parse`:

```c
    nLeftOver = s + len - end;
    if (nLeftOver) {
      /* ... only here is XML_GetBuffer() called, i.e. only here is
         parser->m_buffer ever allocated on this path ... */
      memcpy(parser->m_buffer, end, nLeftOver);
    }
    parser->m_bufferPtr = parser->m_buffer;
    parser->m_bufferEnd = parser->m_buffer + nLeftOver;   /* 2418: NULL + 0 when nLeftOver == 0 */
    parser->m_positionPtr = parser->m_bufferPtr;
    parser->m_parseEndPtr = parser->m_bufferEnd;
```

`m_buffer` is a bare `char *`; nothing in the type records that it may be null, so the
compiler is silent and only UBSan, in that one build configuration, on a non-final parse,
ever notices.

## Suggested fix

Guard the arithmetic (or allocate unconditionally):

```c
    parser->m_bufferPtr = parser->m_buffer;
    parser->m_bufferEnd = parser->m_buffer ? parser->m_buffer + nLeftOver : NULL;
```

## Note on our own port

The BiSheng C port in this tree does **not** catch this: `m_buffer` was deliberately left a
raw `char *` (a Category-C parse window with four cursor aliases, allocated by plain
`.malloc_fcn` rather than the tracked allocator). Annotating it `_Nullable` is the rewrite
that turns this defect into a compile error, as the models above show.

## Files

- `poc_c1_nullbuffer.c` — the reproducer driver (takes the document as argv[1]).
- `c1_wrong.cbs` — arithmetic on a possibly-null pointer, rejected by BiSheng C.
- `c1_deref.cbs` — unguarded vs guarded dereference, rejected vs accepted.
