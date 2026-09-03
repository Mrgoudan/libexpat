# The `dtdCopy` key-aliasing defect: BiSheng C annotation vs. raw C

**Defect (CWE-825 expired-pointer dereference / CWE-416 use-after-free, read).**
In `dtdCopy` (libexpat 2.8.4, `xmlparse.c:7826`), the child DTD's per-element
attribute map `defaultAttForName` is given a **key that points into the *old*
(parent) DTD's string pool** instead of a copy in the child's own pool. Every
sibling table in the same function copies its key with `poolCopyString(&newDtd->pool,…)`
first; this one path does not. The child's hash table therefore stores a pointer
whose owner is the parent's pool. When the parent is reset or freed while the child
parser is still used, a later attribute lookup in the child reads freed memory
(`keyeq`).

The whole point of the two halves below: **the ownership is invisible in the raw C
type `const XML_Char *` (bottom), but becomes a compile-time error the moment the
key is typed as a borrow into the pool that owns it (top).**

---

## UPPER HALF — with BiSheng C ownership annotations (the defect is a compile error)

The key annotation decisions, expressed on the data structures:

```c
// A pool OWNS the bytes of every interned string (its keys live in these blocks).
typedef struct {
  BLOCK *_Owned _Nullable blocks;       // heap-owned key storage
  /* ... ptr/start/end cursors into `blocks` ... */
} STRING_POOL;

// A hash-table key is a BORROW into the pool that interned it. Its lifetime is
// therefore tied to that specific pool. This is the fact the raw `KEY` hides.
typedef const XML_Char *_Borrow KEY;    // borrow, not a bare pointer

typedef struct { KEY name; } NAMED;     // table entry keyed by a pool borrow

// dtdCopy, element-type default-attribute loop, annotated.
// oldDtd and newDtd own DISJOINT pools (newDtd->pool vs oldDtd->pool).
_Safe static int
dtdCopy(XML_Parser _Borrow oldParser, DTD *_Borrow newDtd,
        const DTD *_Borrow oldDtd, XML_Parser _Borrow parser) {
  /* ... */
  for (size_t i = 0; i < newE->nDefaultAtts; i++) {

    // `attributeName` borrows from oldDtd->pool (it is oldDtd's interned name).
    KEY attributeName = oldE->defaultAtts[i].id->name;      // borrow of oldDtd->pool

    // Look up the already-copied id in the NEW tables — read-only, fine.
    newE->defaultAtts[i].id =
        lookup(oldParser, &_Mut newDtd->attributeIds, attributeName, 0);

    /* ... copy value into newDtd->pool ... */

    // BUG: this INSERTS `attributeName` as the key of a table owned by newDtd.
    // We are storing a borrow of oldDtd->pool into a structure whose lifetime is
    // newDtd's. The borrow checker rejects it:
    //
    //   error: cannot move out of 'oldDtd' because it is borrowed
    //   (equivalently: key borrows oldDtd->pool but is stored for newDtd)
    //
    NAME_AND_DEFAULT_ATTRIBUTE *nameAndDefaultAttribute =
        lookup(parser, &_Mut newE->defaultAttForName,
               attributeName,                                // <-- borrow escapes here
               sizeof(NAME_AND_DEFAULT_ATTRIBUTE));
  }
}

// The FIX the checker forces: copy the key into newDtd's own pool first, so the
// stored key is a borrow of newDtd->pool (same lifetime as the table).
KEY attributeName = poolCopyString(&_Mut newDtd->pool,
                                   oldE->defaultAtts[i].id->name);   // borrow of newDtd->pool
```

Real compiler behaviour on the minimal model of this shape (a pool that owns its
key bytes, a key that borrows into it, the key used after the pool is freed):

```
$ /home/ziruichen/bsd/llvm-project-dup/build/bin/clang -x bsc <incs> pool_wrong.cbs <safe.cbs>
pool_wrong.cbs: error: cannot move out of 'oldDtd' because it is borrowed
# rejected at COMPILE time — see expat/bsc-tools/exposure/pool_wrong.cbs

# corrected ordering (key copied into the new pool) compiles rc=0 and runs valgrind-clean:
# see expat/bsc-tools/exposure/pool_right.cbs
```

---

## LOWER HALF — the raw upstream C source (the defect is silent)

Types as they actually are — nothing distinguishes an owned string from a borrowed
one; `const XML_Char *` and `KEY` are bare pointers:

```c
typedef const XML_Char *KEY;            /* xmlparse.c: no lifetime information */

typedef struct { KEY name; } NAMED;

typedef struct {                        /* STRING_POOL — owns the key bytes */
  BLOCK *blocks;
  BLOCK *freeBlocks;
  const XML_Char *end;
  XML_Char *ptr;
  XML_Char *start;
  XML_Parser parser;
} STRING_POOL;
```

The defect, verbatim from `xmlparse.c` (libexpat 2.8.4), lines 7825-7841:

```c
    for (size_t i = 0; i < newE->nDefaultAtts; i++) {
      const XML_Char *const attributeName = oldE->defaultAtts[i].id->name;   // 7826: points into oldDtd->pool
      newE->defaultAtts[i].id = (ATTRIBUTE_ID *)lookup(
          oldParser, &(newDtd->attributeIds), attributeName, 0);             // read-only lookup: ok
      newE->defaultAtts[i].isCdata = oldE->defaultAtts[i].isCdata;
      if (oldE->defaultAtts[i].value) {
        newE->defaultAtts[i].value
            = poolCopyString(&(newDtd->pool), oldE->defaultAtts[i].value);   // value IS copied into newDtd->pool
      } else
        newE->defaultAtts[i].value = NULL;

      NAME_AND_DEFAULT_ATTRIBUTE *const nameAndDefaultAttribute
          = (NAME_AND_DEFAULT_ATTRIBUTE *)lookup(
              parser, &(newE->defaultAttForName), attributeName,             // 7840: BUG — stores oldDtd->pool
              sizeof(NAME_AND_DEFAULT_ATTRIBUTE));                           //        pointer as newDtd's key
      if (! nameAndDefaultAttribute) {
        return 0;
      }
```

Inside `lookup`, the insert path stores the key pointer verbatim
(`xmlparse.c:8100`):

```c
  table->v[i]->name = name;   // NOTE: This requires and assumes zero termination!
```

Contrast — how every *other* table in the same `dtdCopy` copies its key into the
new pool first (`xmlparse.c:7746`, `7767`, `7796`):

```c
    name = poolCopyString(&(newDtd->pool), oldP->name);   /* prefixes  */
    name = poolCopyString(&(newDtd->pool), oldA->name);   /* attributeIds */
    name = poolCopyString(&(newDtd->pool), oldE->name);   /* elementTypes */
```

Only `defaultAttForName` skips the copy, so only its keys can dangle.

---

## What each toolchain does with the same logic

| | Raw C (bottom) | BiSheng C (top) |
|---|---|---|
| Compile | clean, `rc=0` | **error: cannot move out of 'oldDtd' because it is borrowed** |
| Detection | run time only, and only if AddressSanitizer is on | compile time |
| The signal | `heap-use-after-free READ in keyeq` (see `pool_wrong_c.c` under ASan) | a borrow outliving its owner |

The models that produce these exact results are `pool_wrong.cbs` / `pool_right.cbs`
(BiSheng C) and `pool_wrong_c.c` (plain C) in this directory; the corresponding
upstream report is `../report-defaultAttForName-aliasing.md`.

**Reachability caveat (honest).** Turning the dangling key into an actual freed
read requires resetting or freeing the *parent* parser before the *child*
external-entity parser — which the API docs forbid ("free subparsers prior to
freeing their related parent parser"). So this is a real ownership/lifetime defect,
adjacent to the recently fixed CVE-2026-76641 in the same loop, but not an
independently reachable vulnerability. Its value here is that BiSheng C's borrow
checker turns the latent dangling alias into a compile-time error.
