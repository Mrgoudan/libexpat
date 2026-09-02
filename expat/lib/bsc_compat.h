/* BiSheng C annotations compile away under a plain C compiler. */
#ifndef BSC_COMPAT_H
#define BSC_COMPAT_H

#ifdef __bishengc
#  include "bishengc_safety.hbs"
#else
#  define _Safe
#  define _Unsafe
#  define _Borrow
#  define _Owned
#  define _ArrayElem
#  define _Mut
#  define _Const
#  if ! defined(__clang__)
#    define _Nullable
#    define _Nonnull
#  endif
#  define __take_from_raw(p) (p)
#  define __move_to_raw(p) (p)
#  define __take_array_from_raw(p) (p)
#  define __move_array_to_raw(p) (p)
#  if ! defined(__cplusplus)                                                   \
      && (! defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L)
#    define nullptr ((void *)0)
#  endif
#  define safe_swap(a, b)                                                      \
    do {                                                                       \
      __typeof__(*(a)) bsc_compat_tmp = *(a);                                  \
      *(a) = *(b);                                                             \
      *(b) = bsc_compat_tmp;                                                   \
    } while (0)
#endif

#endif /* BSC_COMPAT_H */
