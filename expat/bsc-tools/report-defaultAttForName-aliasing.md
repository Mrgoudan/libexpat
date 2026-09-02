# Draft: `dtdCopy` stores parent-DTD pool strings as keys of the child's `defaultAttForName` tables

Status: draft for the maintainer, not sent. Found while porting libexpat to BiSheng C
(ownership annotation of `NAMED.name` / hash-table keys), libexpat 2.8.4, `xmlparse.c`.

## What

`dtdCopy()` (called from `XML_ExternalEntityParserCreate()` with a non-NULL context)
rebuilds each element type's `defaultAttForName` hash table (added in 2.8.4 for the
CVE-2026-66046 fix) with

```c
const XML_Char *const attributeName = oldE->defaultAtts[i].id->name;
...
NAME_AND_DEFAULT_ATTRIBUTE *const nameAndDefaultAttribute
    = (NAME_AND_DEFAULT_ATTRIBUTE *)lookup(parser, &(newE->defaultAttForName),
                                           attributeName, sizeof(...));
```

`lookup()` stores the key pointer as-is (`table->v[i]->name = name`), so the new table's
keys point into the *old* DTD's `pool`. Every other table copied by `dtdCopy()` first
duplicates the key into `newDtd->pool` (`poolCopyString`) and then inserts the copy.

## Why it matters

The child parser's DTD is meant to be self-contained after `dtdCopy()` (its own pool,
its own entries). With this aliasing, `XML_ParserReset()` or `XML_ParserFree()` on the
parent while a child parser is still in use leaves the child's `defaultAttForName`
keys dangling (`poolClear` recycles blocks; `poolDestroy` frees them). A later
`storeAtts()` in the child then compares attribute names against freed or reused
memory in `keyeq()`.

In practice a child parser also reaches into its parent through `m_parentParser`
for allocation accounting, so keeping a child alive past its parent is already
unsupported; this is a consistency defect rather than an independently reachable
memory-safety bug. It is cheap to fix and removes a hidden cross-parser lifetime
dependency.

## Suggested fix

Use the already-copied attribute id's name (which lives in `newDtd->pool`) as the key:

```c
newE->defaultAtts[i].id = (ATTRIBUTE_ID *)lookup(oldParser, &(newDtd->attributeIds),
                                                 attributeName, 0);
...
lookup(parser, &(newE->defaultAttForName), newE->defaultAtts[i].id->name, sizeof(...));
```

(`newE->defaultAtts[i].id` is non-NULL because `attributeIds` was copied just before.)

## How it was found

BiSheng C ownership annotation: `HASH_TABLE.v` became an owned array of entries whose
`name` keys must be owned by the same DTD's pool; `dtdCopy` was the one site where a key
crossed DTD ownership without a copy.
