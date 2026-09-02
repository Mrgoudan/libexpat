# BiSheng C compiler crash found while porting libexpat

`borrow-checker-fnptr-call.c` makes clang 15.0.4 (BiSheng C fork at
/home/ziruichen/bsd/llvm-project-dup, commit f0642d48fda5) segfault:

```
clang -x bsc -fsyntax-only borrow-checker-fnptr-call.c
# clang frontend command failed with exit code 139
# #4 ... ActionExtract::Visit(clang::Stmt*) BSCBorrowChecker.cpp
# #13 clang::borrow::Liveness::Walk<... RegionCheck::PopulateInference ...>
```

Trigger: a call through a function-pointer member reached via a borrow of a raw
pointer, inside an `_Unsafe(...)` expression in a `_Safe` function
(`_Unsafe((&_Mut *raw)->fn(1))`). Reading a data member the same way
(`_Unsafe((&_Mut *raw)->x)`) does not crash. Hit in libexpat's `XML_ParserFree`
under `XML_GE == 0`, where `FREE(&_Mut *rawParser, alias)` expanded to that shape;
worked around by calling a `_Safe` inline wrapper instead.
