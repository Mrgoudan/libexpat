struct PS { void (*fn)(int); int x; };
_Safe void f(struct PS *raw) {
  _Unsafe((&_Mut *raw)->fn(1));
}
