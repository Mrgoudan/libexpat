# Exposing candidate 16 (dtdCopy key aliasing) at compile time with BiSheng C

Method: function-invariant reading. `dtdCopy()` rebuilds each element type's
`defaultAttForName` hash table using `attributeName = oldE->defaultAtts[i].id->name`,
a string that lives in the OLD DTD's `pool`, as the NEW table's key. Every sibling
table in `dtdCopy` first copies its key into `newDtd->pool`. Invariant that is
violated: a hash-table key must be owned by the same DTD's pool as the table.

## The bug class in BiSheng C (real compiler runs)

`pool_wrong.cbs` models the shape: a `Pool` owns its key bytes, a key is a
`_Borrow` into the pool, and the key is used after the pool is freed.

```
/home/ziruichen/bsd/llvm-project-dup/build/bin/clang -x bsc <incs> pool_wrong.cbs <safe.cbs>
# error: cannot move out of 'oldDtd' because it is borrowed        <-- rejected at COMPILE time
```

Corrected ordering (`pool_right`, key used before the pool is freed) compiles
rc=0 and runs valgrind-clean.

## What plain C does with the identical logic

`pool_wrong_c.c` is the same code with the annotations erased.

```
/usr/bin/clang -O0 pool_wrong_c.c            # compiles clean, rc=0
/usr/bin/clang -fsanitize=address pool_wrong_c.c && ./a.out
# heap-use-after-free in key_first  <-- only caught at RUN time, only if you happen to run ASan
```

So the borrow annotation turns a runtime-only, input-dependent use-after-free
into a compile-time error. `uaf_wrong.cbs` / `uaf_right.cbs` are the minimal
`int *`-level version of the same contrast.

## Why the live expat port does not already error here

The port keeps `HASH_TABLE` keys as raw `KEY` (Category-C: interior pointers into
pool blocks) because BiSheng C struct `_Borrow` fields cannot borrow from another
heap object's storage (manual §3.2.6). Annotating keys as pool borrows is the
rewrite that would make the checker reject `dtdCopy`'s aliasing; it requires the
pool-owns-keys refactor sketched here, which is beyond the "no redesign" scope of
this port. The models above prove the checker catches the class once the ownership
is expressed.
