/* 13c4e8da8fccffb0e8e599684e0d447ad14c1bb0b48792cf5dd77d8712301871 (2.8.4+)
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
   Copyright (c) 2000-2006 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2001-2002 Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2002-2016 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2005-2009 Steven Solie <steven@solie.ca>
   Copyright (c) 2016      Eric Rahm <erahm@mozilla.com>
   Copyright (c) 2016-2026 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2016      Gaurav <g.gupta@samsung.com>
   Copyright (c) 2016      Thomas Beutlich <tc@tbeu.de>
   Copyright (c) 2016      Gustavo Grieco <gustavo.grieco@imag.fr>
   Copyright (c) 2016      Pascal Cuoq <cuoq@trust-in-soft.com>
   Copyright (c) 2016      Ed Schouten <ed@nuxi.nl>
   Copyright (c) 2017-2022 Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2017      Václav Slavík <vaclav@slavik.io>
   Copyright (c) 2017      Viktor Szakats <commit@vsz.me>
   Copyright (c) 2017      Chanho Park <chanho61.park@samsung.com>
   Copyright (c) 2017      Rolf Eike Beer <eike@sf-mail.de>
   Copyright (c) 2017      Hans Wennborg <hans@chromium.org>
   Copyright (c) 2018      Anton Maklakov <antmak.pub@gmail.com>
   Copyright (c) 2018      Benjamin Peterson <benjamin@python.org>
   Copyright (c) 2018      Marco Maggi <marco.maggi-ipsu@poste.it>
   Copyright (c) 2018      Mariusz Zaborski <oshogbo@vexillium.org>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Copyright (c) 2019-2020 Ben Wagner <bungeman@chromium.org>
   Copyright (c) 2019      Vadim Zeitlin <vadim@zeitlins.org>
   Copyright (c) 2021      Donghee Na <donghee.na@python.org>
   Copyright (c) 2022      Samanta Navarro <ferivoz@riseup.net>
   Copyright (c) 2022      Jeffrey Walton <noloader@gmail.com>
   Copyright (c) 2022      Jann Horn <jannh@google.com>
   Copyright (c) 2022      Sean McBride <sean@rogue-research.com>
   Copyright (c) 2023      Owain Davies <owaind@bath.edu>
   Copyright (c) 2023-2024 Sony Corporation / Snild Dolkow <snild@sony.com>
   Copyright (c) 2024-2025 Berkay Eren Ürün <berkay.ueruen@siemens.com>
   Copyright (c) 2024      Hanno Böck <hanno@gentoo.org>
   Copyright (c) 2025-2026 Matthew Fernandez <matthew.fernandez@gmail.com>
   Copyright (c) 2025      Atrem Borovik <polzovatellllk@gmail.com>
   Copyright (c) 2025      Alfonso Gregory <gfunni234@gmail.com>
   Copyright (c) 2026      Rosen Penev <rosenp@gmail.com>
   Copyright (c) 2026      Francesco Bertolaccini
   Copyright (c) 2026      Christian Ng <christianrng@berkeley.edu>
   Copyright (c) 2026      Nick Begg <nick@stunttruck.net>
   Copyright (c) 2026      Kartik Kenchi <netliomax25@gmail.com>
   Copyright (c) 2026      Haris Hussain <hextheshadow0x@gmail.com>
   Copyright (c) 2026      Evgeny Kotkov <kotkov@apache.org>
   Copyright (c) 2026      Darren Carreras <carrerasdarren@gmail.com>
   Copyright (c) 2026      Alberto Maschietto <albertomaschietto9@gmail.com>
   Copyright (c) 2026      Zeyou Liu <zeyouliu@tencent.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.

   SPDX-License-Identifier: MIT
*/

#define XML_BUILDING_EXPAT 1

#include "expat_config.h"

#if ! defined(XML_GE) || (1 - XML_GE - 1 == 2) || (XML_GE < 0) || (XML_GE > 1)
#  error XML_GE (for general entities) must be defined, non-empty, either 1 or 0 (0 to disable, 1 to enable; 1 is a common default)
#endif

#if defined(XML_DTD) && XML_GE == 0
#  error Either undefine XML_DTD or define XML_GE to 1.
#endif

#if ! defined(XML_CONTEXT_BYTES) || (1 - XML_CONTEXT_BYTES - 1 == 2)           \
    || (XML_CONTEXT_BYTES + 0 < 0)
#  error XML_CONTEXT_BYTES must be defined, non-empty and >=0 (0 to disable, >=1 to enable; 1024 is a common default)
#endif

#include <stdbool.h>
#include <stddef.h>
#include <string.h> /* _Unsafe(memset()), _Unsafe(memcpy()) */
#include <assert.h>
#include <limits.h> /* INT_MAX, UINT_MAX */
#include <stdio.h>  /* fprintf */
#include <stdlib.h> /* getenv */
#include <stdint.h> /* SIZE_MAX, UINT64_MAX, uint64_t, uintptr_t */
#include <math.h>   /* isnan */
#include <errno.h>

#ifdef _WIN32
#  define getpid GetCurrentProcessId
#else
#  include <sys/time.h>  /* _Unsafe(gettimeofday()) */
#  include <sys/types.h> /* _Unsafe(getpid()) */
#  include <unistd.h>    /* _Unsafe(getpid()) */
#  include <fcntl.h>     /* O_RDONLY */
#  include <errno.h>
#endif

#ifdef _WIN32
#  include "winconfig.h"
#endif

#include "ascii.h"
#include "expat.h"
#include "siphash.h"
#include "xcsinc.c"

#if defined(HAVE_ARC4RANDOM)
#  include "random_arc4random.h"
#endif /* defined(HAVE_ARC4RANDOM) */

#if defined(HAVE_ARC4RANDOM_BUF)
#  include "random_arc4random_buf.h"
#endif // defined(HAVE_ARC4RANDOM_BUF)

#if defined(XML_DEV_URANDOM)
#  include "random_dev_urandom.h"
#endif /* defined(XML_DEV_URANDOM) */

#if defined(HAVE_GETENTROPY)
#  include "random_getentropy.h"
#endif // defined(HAVE_GETENTROPY)

#if defined(HAVE_GETRANDOM) || defined(HAVE_SYSCALL_GETRANDOM)
#  include "random_getrandom.h"
#endif /* defined(HAVE_GETRANDOM) || defined(HAVE_SYSCALL_GETRANDOM) */

#if defined(_WIN32)
#  include "random_rand_s.h"
#endif /* defined(_WIN32) */

#if ! defined(HAVE_GETRANDOM) && ! defined(HAVE_SYSCALL_GETRANDOM)             \
    && ! defined(HAVE_ARC4RANDOM_BUF) && ! defined(HAVE_ARC4RANDOM)            \
    && ! defined(HAVE_GETENTROPY) && ! defined(XML_DEV_URANDOM)                \
    && ! defined(_WIN32) && ! defined(XML_POOR_ENTROPY)
#  error You do not have support for any sources of high quality entropy \
    enabled.  For end user security, that is probably not what you want. \
    \
    Your options include: \
      * Linux >=3.17 + glibc >=2.25 (getrandom): HAVE_GETRANDOM, \
      * Linux >=3.17 + glibc (including <2.25) (syscall SYS_getrandom): HAVE_SYSCALL_GETRANDOM, \
      * BSD / macOS >=10.7 / glibc >=2.36 (arc4random_buf): HAVE_ARC4RANDOM_BUF, \
      * BSD / macOS (including <10.7) / glibc >=2.36 (arc4random): HAVE_ARC4RANDOM, \
      * BSD / macOS >=10.12 / glibc >=2.25 (getentropy): HAVE_GETENTROPY, \
      * Linux (including <3.17) / BSD / macOS (including <10.7) / Solaris >=8 (/dev/urandom): XML_DEV_URANDOM, \
      * Windows >=Vista (rand_s): _WIN32. \
    \
    If you insist on not using any of these, bypass this error by defining \
    XML_POOR_ENTROPY and be vulnerable to hash flooding; you have been warned. \
    \
    If you have reasons to patch this detection code away or need changes \
    to the build system, please open a bug.  Thank you!
#endif

#ifdef XML_UNICODE
#  define XML_ENCODE_MAX XML_UTF16_ENCODE_MAX
#  define XmlConvert XmlUtf16Convert
#  define XmlGetInternalEncoding XmlGetUtf16InternalEncoding
#  define XmlGetInternalEncodingNS XmlGetUtf16InternalEncodingNS
#  define XmlEncode XmlUtf16Encode
#  define MUST_CONVERT(enc, s) (! (enc)->isUtf16 || (((uintptr_t)(s)) & 1))
typedef unsigned short ICHAR;
#else
#  define XML_ENCODE_MAX XML_UTF8_ENCODE_MAX
#  define XmlConvert XmlUtf8Convert
#  define XmlGetInternalEncoding XmlGetUtf8InternalEncoding
#  define XmlGetInternalEncodingNS XmlGetUtf8InternalEncodingNS
#  define XmlEncode XmlUtf8Encode
#  define MUST_CONVERT(enc, s) (_Unsafe(! (enc)->isUtf8))
typedef char ICHAR;
#endif

#ifndef XML_NS

#  define XmlInitEncodingNS XmlInitEncoding
#  define XmlInitUnknownEncodingNS XmlInitUnknownEncoding
#  undef XmlGetInternalEncodingNS
#  define XmlGetInternalEncodingNS XmlGetInternalEncoding
#  define XmlParseXmlDeclNS XmlParseXmlDecl

#endif

#ifdef XML_UNICODE

#  ifdef XML_UNICODE_WCHAR_T
#    define XML_T(x) (const wchar_t) x
#    define XML_L(x) L##x
#  else
#    define XML_T(x) (const unsigned short)x
#    define XML_L(x) x
#  endif

#else

#  define XML_T(x) x
#  define XML_L(x) x

#endif

/* Round up n to be a multiple of sz, where sz is a power of 2. */
#define ROUND_UP(n, sz) (((n) + ((sz) - 1)) & ~((sz) - 1))

/* Do safe (nullptr-aware) pointer arithmetic */
#define EXPAT_SAFE_PTR_DIFF(p, q) (((p) && (q)) ? ((p) - (q)) : 0)

#define EXPAT_MIN(a, b) (((a) < (b)) ? (a) : (b))

#include "internal.h"
#include "xmltok.h"
#include "xmlrole.h"

typedef const XML_Char *KEY;

typedef struct {
  KEY _Nonnull name;
} NAMED;

typedef struct {
  NAMED **_Owned _ArrayElem _Nullable v;
  unsigned char power;
  size_t size;
  size_t used;
  XML_Parser parser;
} HASH_TABLE;

_Safe static size_t keylen(KEY _Nonnull s);

_Safe static void copy_salt_to_sipkey(XML_Parser _Borrow parser, struct sipkey *_Borrow __attribute__((ensure_init)) key);

/* For probing (after a collision) we need a step size relative prime
   to the hash table size, which is a power of 2. We use double-hashing,
   since we can calculate a second hash value cheaply by taking those bits
   of the first hash value that were discarded (masked out) when the table
   index was calculated: index = hash & mask, where mask = table->size - 1.
   We limit the maximum step size to table->size / 4 (mask >> 2) and make
   it odd, since odd numbers are always relative prime to a power of 2.
*/
#define SECOND_HASH(hash, mask, power)                                         \
  ((((hash) & ~(mask)) >> ((power) - 1)) & ((mask) >> 2))
#define PROBE_STEP(hash, mask, power)                                          \
  ((unsigned char)((SECOND_HASH(hash, mask, power)) | 1))

typedef struct {
  NAMED **p;
  NAMED **end;
} HASH_TABLE_ITER;

#define INIT_TAG_BUF_SIZE 32 /* must be a multiple of sizeof(XML_Char) */
#define INIT_DATA_BUF_SIZE 1024
#define INIT_ATTS_SIZE 16
#define INIT_ATTS_VERSION 0xFFFFFFFF
#define INIT_BLOCK_SIZE 1024
#define INIT_BUFFER_SIZE 1024

#define EXPAND_SPARE 24

_Safe XML_Parser _Owned _Nullable XMLCALL XML_ParserCreate_MM(
    const XML_Char *_Nonnull encodingName,
    const XML_Memory_Handling_Suite *_Borrow _Nullable memsuite,
    const XML_Char *nameSep);
_Safe void XMLCALL XML_ParserFree(XML_Parser _Owned _Nullable parser);
_Safe void *_Nullable XMLCALL XML_GetBuffer(XML_Parser _Borrow _Nullable parser,
                                            int len);
_Safe enum XML_Status XMLCALL XML_ParseBuffer(XML_Parser _Borrow _Nullable parser,
                                              int len, int isFinal);

typedef struct binding {
  struct prefix *prefix;
  struct binding *nextTagBinding;
  struct binding *prevPrefixBinding;
  const struct attribute_id *attId;
  XML_Char *_Owned _ArrayElem _Nullable uri;
  size_t uriLen;
  size_t uriAlloc;
} BINDING;

typedef struct prefix {
  const XML_Char *name;
  BINDING *binding;
} PREFIX;

typedef struct {
  const XML_Char *_Nonnull str;
  const XML_Char *localPart;
  const XML_Char *prefix;
  size_t strLen;
  size_t uriLen;
  size_t prefixLen;
} TAG_NAME;

/* TAG represents an open element.
   The name of the element is stored in both the document and API
   encodings.  The memory buffer 'buf' is a separately-allocated
   memory area which stores the name.  During the XML_Parse()/
   XML_ParseBuffer() when the element is open, the memory for the 'raw'
   version of the name (in the document encoding) is shared with the
   document buffer.  If the element is open across calls to
   XML_Parse()/XML_ParseBuffer(), the buffer is re-allocated to
   contain the 'raw' name as well.

   A parser reuses these structures, maintaining a list of allocated
   TAG objects in a free list.
*/
typedef struct tag {
  struct tag *parent;  /* parent of this element */
  const char *rawName; /* tagName in the original encoding */
  int rawNameLength;
  TAG_NAME name; /* tagName in the API encoding */
  union {
    char *raw;     /* for byte-level access (rawName storage) */
    XML_Char *str; /* for character-level access (converted name) */
  } buf;           /* buffer for name components */
  char *bufEnd;    /* end of the buffer */
  BINDING *bindings;
} TAG;

typedef struct {
  const XML_Char *name;
  const XML_Char *textPtr;
  int textLen;   /* length in XML_Chars */
  int processed; /* # of processed bytes - when suspended */
  const XML_Char *systemId;
  const XML_Char *base;
  const XML_Char *publicId;
  const XML_Char *notation;
  bool open;
  XML_Bool hasMore; /* true if entity has not been completely processed */
  /* An entity can be open while being already completely processed (hasMore ==
    XML_FALSE). The reason is the delayed closing of entities until their inner
    entities are processed and closed */
  XML_Bool is_param;
  XML_Bool is_internal; /* true if declared in internal subset outside PE */
} ENTITY;

typedef struct {
  enum XML_Content_Type type;
  enum XML_Content_Quant quant;
  const XML_Char *name;
  int firstchild;
  int lastchild;
  int childcnt;
  int nextsib;
} CONTENT_SCAFFOLD;

#define INIT_SCAFFOLD_ELEMENTS 32

typedef struct block {
  struct block *next;
  int size;
  XML_Char s[];
} BLOCK;

typedef struct {
  BLOCK *blocks;
  BLOCK *freeBlocks;
  const XML_Char *end;
  XML_Char *ptr;
  XML_Char *start;
  XML_Parser parser;
} STRING_POOL;

/* The XML_Char before the name is used to determine whether
   an attribute has been specified. */
typedef struct attribute_id {
  XML_Char *name;
  PREFIX *prefix;
  XML_Bool maybeTokenized;
  XML_Bool xmlns;
} ATTRIBUTE_ID;

typedef struct {
  const ATTRIBUTE_ID *id;
  XML_Bool isCdata;
  const XML_Char *value;
} DEFAULT_ATTRIBUTE;

// This structure allows mapping attribute names to instances of
// `DEFAULT_ATTRIBUTE`.
typedef struct {
  // Member `name` goes first to make this structure compatible with structure
  // `NAMED` (further up), which is needed to support use of structure
  // `NAME_AND_DEFAULT_ATTRIBUTE` in a hash table as implemented by function
  // `lookup` (further down).
  const XML_Char *name;
  // We would store a `DEFAULT_ATTRIBUTE *` here but the backing array
  // can be reallocated which would invalidate the pointer. Using an index
  // into the array instead, avoids that problem.
  size_t attIndex;
  // This is set to `false` by function `lookup`.
  bool initialized;
} NAME_AND_DEFAULT_ATTRIBUTE;

typedef struct {
  unsigned long version;
  unsigned long hash;
  const XML_Char *uriName;
} NS_ATT;

typedef struct {
  const XML_Char *name;
  PREFIX *prefix;
  const ATTRIBUTE_ID *idAtt;
  size_t nDefaultAtts;
  size_t allocDefaultAtts;
  DEFAULT_ATTRIBUTE *_Owned _ArrayElem _Nullable defaultAtts;
  HASH_TABLE defaultAttForName;
} ELEMENT_TYPE;

typedef struct {
  HASH_TABLE generalEntities;
  HASH_TABLE elementTypes;
  HASH_TABLE attributeIds;
  HASH_TABLE prefixes;
  STRING_POOL pool;
  STRING_POOL entityValuePool;
  /* false once a parameter entity reference has been skipped */
  XML_Bool keepProcessing;
  /* true once an internal or external PE reference has been encountered;
     this includes the reference to an external subset */
  XML_Bool hasParamEntityRefs;
  XML_Bool standalone;
#ifdef XML_DTD
  /* indicates if external PE has been read */
  XML_Bool paramEntityRead;
  HASH_TABLE paramEntities;
#endif /* XML_DTD */
  PREFIX defaultPrefix;
  /* === scaffolding for building content model === */
  XML_Bool in_eldecl;
  CONTENT_SCAFFOLD *scaffold;
  unsigned contentStringLen;
  unsigned scaffSize;
  unsigned scaffCount;
  int scaffLevel;
  int *scaffIndex;
  size_t scaffIndexSize;
} DTD;

enum EntityType {
  ENTITY_INTERNAL,
  ENTITY_ATTRIBUTE,
  ENTITY_VALUE,
};

typedef struct open_internal_entity {
  const char *internalEventPtr;
  const char *internalEventEndPtr;
  struct open_internal_entity *next;
  ENTITY *_Nonnull entity;
  int startTagLevel;
  XML_Bool betweenDecl; /* WFC: PE Between Declarations */
  enum EntityType type;
} OPEN_INTERNAL_ENTITY;

enum XML_Account {
  XML_ACCOUNT_DIRECT,           /* bytes directly passed to the Expat parser */
  XML_ACCOUNT_ENTITY_EXPANSION, /* intermediate bytes produced during entity
                                   expansion */
  XML_ACCOUNT_NONE              /* i.e. do not account, was accounted already */
};

#if XML_GE == 1
typedef unsigned long long XmlBigCount;
typedef struct accounting {
  XmlBigCount countBytesDirect;
  XmlBigCount countBytesIndirect;
  unsigned long debugLevel;
  float maximumAmplificationFactor; // >=1.0
  unsigned long long activationThresholdBytes;
} ACCOUNTING;

typedef struct MALLOC_TRACKER {
  XmlBigCount bytesAllocated;
  XmlBigCount peakBytesAllocated; // updated live only for debug level >=2
  unsigned long debugLevel;
  float maximumAmplificationFactor; // >=1.0
  XmlBigCount activationThresholdBytes;
} MALLOC_TRACKER;

typedef struct entity_stats {
  unsigned int countEverOpened;
  unsigned int currentDepth;
  unsigned int maximumDepthSeen;
  unsigned long debugLevel;
} ENTITY_STATS;
#endif /* XML_GE == 1 */

typedef _Safe enum XML_Error PTRCALL Processor(XML_Parser _Borrow parser,
                                               const char *start,
                                         const char *end, const char **_Nonnull endPtr);

static Processor prologProcessor;
static Processor prologInitProcessor;
static Processor contentProcessor;
static Processor cdataSectionProcessor;
#ifdef XML_DTD
static Processor ignoreSectionProcessor;
static Processor externalParEntProcessor;
static Processor externalParEntInitProcessor;
static Processor entityValueProcessor;
static Processor entityValueInitProcessor;
#endif /* XML_DTD */
static Processor epilogProcessor;
static Processor errorProcessor;
static Processor externalEntityInitProcessor;
static Processor externalEntityInitProcessor2;
static Processor externalEntityInitProcessor3;
static Processor externalEntityContentProcessor;
static Processor internalEntityProcessor;

_Safe static enum XML_Error handleUnknownEncoding(XML_Parser _Borrow parser,
                                            const XML_Char *_Nullable encodingName);
_Safe static enum XML_Error processXmlDecl(XML_Parser _Borrow parser, int isGeneralTextEntity,
                                     const char *s, const char *next);
_Safe static enum XML_Error initializeEncoding(XML_Parser _Borrow parser);
_Safe static enum XML_Error doProlog(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                               const char *s, const char *end, int tok,
                               const char *next, const char **_Nonnull nextPtr,
                               XML_Bool haveMore, XML_Bool allowClosingDoctype,
                               enum XML_Account account);
_Safe static enum XML_Error processEntity(XML_Parser _Borrow parser, ENTITY *_Nonnull entity,
                                    XML_Bool betweenDecl, enum EntityType type);
_Safe static enum XML_Error doContent(XML_Parser _Borrow parser, int startTagLevel,
                                const ENCODING *_Nonnull enc, const char *start,
                                const char *end, const char **_Nonnull endPtr,
                                XML_Bool haveMore, enum XML_Account account);
_Safe static enum XML_Error doCdataSection(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                                     const char **_Nonnull startPtr, const char *end,
                                     const char **_Nonnull nextPtr, XML_Bool haveMore,
                                     enum XML_Account account);
#ifdef XML_DTD
_Safe static enum XML_Error doIgnoreSection(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                                      const char **_Nonnull startPtr, const char *end,
                                      const char **_Nonnull nextPtr, XML_Bool haveMore);
#endif /* XML_DTD */

_Safe static void freeBindings(XML_Parser _Borrow parser, BINDING *bindings);
_Safe static enum XML_Error storeAtts(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                                const char *attStr, TAG_NAME *_Borrow tagNamePtr,
                                BINDING **_Nonnull bindingsPtr,
                                enum XML_Account account);
_Safe static enum XML_Error addBinding(XML_Parser _Borrow parser, PREFIX *_Nonnull prefix,
                                 const ATTRIBUTE_ID *_Nullable attId, const XML_Char *uri,
                                 BINDING **_Nonnull bindingsPtr);
_Safe static int defineAttribute(ELEMENT_TYPE *_Nonnull type, ATTRIBUTE_ID *_Nonnull attId,
                           XML_Bool isCdata, XML_Bool isId,
                           const XML_Char *_Nullable value, XML_Parser _Borrow parser);
_Safe static enum XML_Error storeAttributeValue(XML_Parser _Borrow parser,
                                          const ENCODING *_Nonnull enc, XML_Bool isCdata,
                                          const char *ptr, const char *end,
                                          STRING_POOL *_Borrow pool,
                                          enum XML_Account account);
static enum XML_Error
appendAttributeValue(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, XML_Bool isCdata,
                     const char *ptr, const char *end, STRING_POOL *_Borrow pool,
                     enum XML_Account account, const char **_Nullable nextPtr);
_Safe static ATTRIBUTE_ID *getAttributeId(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                                    const char *start, const char *end);
_Safe static int setElementTypePrefix(XML_Parser _Borrow parser, ELEMENT_TYPE *_Nonnull elementType);
#if XML_GE == 1
_Safe static enum XML_Error storeEntityValue(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                                       const char *start, const char *end,
                                       enum XML_Account account,
                                       const char **_Nullable nextPtr);
_Safe static enum XML_Error callStoreEntityValue(XML_Parser _Borrow parser,
                                           const ENCODING *_Nonnull enc,
                                           const char *start, const char *end,
                                           enum XML_Account account);
#else
_Safe static enum XML_Error storeSelfEntityValue(XML_Parser _Borrow parser, ENTITY *_Nonnull entity);
#endif
_Safe static int reportProcessingInstruction(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                                       const char *start, const char *end);
_Safe static int reportComment(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                         const char *start, const char *end);
_Safe static void reportDefault(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                          const char *start, const char *end);

_Safe static const XML_Char *getContext(XML_Parser _Borrow parser);
_Safe static XML_Bool setContext(XML_Parser _Borrow parser, const XML_Char *_Nonnull context);

_Safe static void FASTCALL normalizePublicId(XML_Char *_Nonnull s);

_Safe static DTD *dtdCreate(XML_Parser _Borrow parser);
/* do not call if m_parentParser != nullptr */
_Safe static void dtdReset(DTD *_Borrow p, XML_Parser _Borrow parser);
_Safe static void dtdDestroy(DTD *p, XML_Bool isDocEntity, XML_Parser _Borrow parser);
_Safe static int dtdCopy(XML_Parser _Borrow oldParser, DTD *_Borrow newDtd, const DTD *_Borrow oldDtd,
                   XML_Parser _Borrow parser);
_Safe static int copyEntityTable(XML_Parser _Borrow oldParser, HASH_TABLE *_Borrow newTable,
                           STRING_POOL *_Borrow newPool, const HASH_TABLE *_Borrow oldTable);
_Safe static NAMED *lookupWithLength(XML_Parser _Borrow parser, HASH_TABLE *_Borrow table, KEY _Nonnull name,
                               size_t nameLen, size_t createSize);
_Safe static NAMED *lookup(XML_Parser _Borrow parser, HASH_TABLE *_Borrow table, KEY _Nonnull name,
                     size_t createSize);
_Safe static void FASTCALL hashTableInit(HASH_TABLE *_Borrow table, XML_Parser rawParser);
_Safe static void FASTCALL hashTableClear(HASH_TABLE *_Borrow table);
_Safe static void FASTCALL hashTableDestroy(HASH_TABLE *_Borrow table);
_Safe static void FASTCALL hashTableIterInit(HASH_TABLE_ITER *_Borrow iter,
                                       const HASH_TABLE *_Borrow table);
_Safe static NAMED *FASTCALL hashTableIterNext(HASH_TABLE_ITER *_Borrow iter);

_Safe static void FASTCALL poolInit(STRING_POOL *_Borrow pool, XML_Parser rawParser);
_Safe static void FASTCALL poolClear(STRING_POOL *_Borrow pool);
_Safe static void FASTCALL poolDestroy(STRING_POOL *_Borrow pool);
_Safe static XML_Char *poolAppend(STRING_POOL *_Borrow pool, const ENCODING *_Nonnull enc,
                            const char *ptr, const char *end);
_Safe static XML_Char *poolStoreString(STRING_POOL *_Borrow pool, const ENCODING *_Nonnull enc,
                                 const char *ptr, const char *end);
_Safe static XML_Bool FASTCALL poolGrow(STRING_POOL *_Borrow pool);
_Safe static bool FASTCALL poolGrowUntil(STRING_POOL *_Borrow pool, size_t needed);
_Safe static const XML_Char *FASTCALL poolCopyString(STRING_POOL *_Borrow pool,
                                               const XML_Char *_Nonnull s);
_Safe static const XML_Char *FASTCALL poolCopyStringNoFinish(STRING_POOL *_Borrow pool,
                                                       const XML_Char *_Nonnull s);
_Safe static const XML_Char *poolCopyStringN(STRING_POOL *_Borrow pool, const XML_Char *_Nonnull s,
                                       int n);
_Safe static const XML_Char *FASTCALL poolAppendString(STRING_POOL *_Borrow pool,
                                                 const XML_Char *_Nonnull s);

_Safe static int FASTCALL nextScaffoldPart(XML_Parser _Borrow parser);
_Safe static XML_Content *build_model(XML_Parser _Borrow parser);
_Safe static ELEMENT_TYPE *getElementType(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                                    const char *ptr, const char *end);

_Safe static XML_Char *_Owned _ArrayElem _Nullable copyString(const XML_Char *_Nonnull s,
                                                        XML_Parser _Borrow parser);

_Safe static struct sipkey generate_hash_secret_salt(void);
_Safe static XML_Bool startParsing(XML_Parser _Borrow parser);

_Safe static XML_Parser _Owned _Nullable parserCreate(const XML_Char *_Nonnull encodingName,
                               const XML_Memory_Handling_Suite *_Borrow _Nullable memsuite,
                               const XML_Char *nameSep, DTD *dtd,
                               XML_Parser _Borrow _Nullable parentParser);

_Safe static void parserInit(XML_Parser _Borrow parser, const XML_Char *_Nonnull encodingName);

#if XML_GE == 1
_Safe static float accountingGetCurrentAmplification(XML_Parser _Borrow rootParser);
_Safe static void accountingReportStats(XML_Parser _Borrow originParser, const char *epilog);
_Safe static void accountingOnAbort(XML_Parser _Borrow originParser);
_Safe static void accountingReportDiff(XML_Parser _Borrow rootParser,
                                 unsigned int levelsAwayFromRootParser,
                                 const char *before, const char *after,
                                 ptrdiff_t bytesMore, int source_line,
                                 enum XML_Account account);
_Safe static XML_Bool accountingDiffTolerated(XML_Parser _Borrow originParser, int tok,
                                        const char *before, const char *after,
                                        int source_line,
                                        enum XML_Account account);

_Safe static void entityTrackingReportStats(XML_Parser _Borrow parser, ENTITY *_Nonnull entity,
                                      const char *action, int sourceLine);
_Safe static void entityTrackingOnOpen(XML_Parser _Borrow parser, ENTITY *_Nonnull entity,
                                 int sourceLine);
_Safe static void entityTrackingOnClose(XML_Parser _Borrow parser, ENTITY *_Nonnull entity,
                                  int sourceLine);
#endif /* XML_GE == 1 */

_Safe static XML_Parser _Borrow getRootParserOf(XML_Parser _Borrow parser,
                                  unsigned int *_Borrow _Nullable outLevelDiff);

_Safe static unsigned long getDebugLevel(const char *variableName,
                                   unsigned long defaultDebugLevel);

_Safe static bool poolAppendChar(STRING_POOL *_Borrow pool, XML_Char c);

_Safe static bool poolAppendChars(STRING_POOL *_Borrow pool, const XML_Char *_Nonnull s, size_t len);

_Safe static inline XML_Char *_Nullable
poolStart(const STRING_POOL *_Borrow pool) {
  return pool->start;
}
_Safe static inline ptrdiff_t
poolLength(const STRING_POOL *_Borrow pool) {
  return _Unsafe(pool->ptr - pool->start);
}
_Safe static inline void
poolChop(STRING_POOL *_Borrow pool) {
  _Unsafe pool->ptr--;
}
_Safe static inline XML_Char
poolLastChar(const STRING_POOL *_Borrow pool) {
  return _Unsafe(pool->ptr[-1]);
}
_Safe static inline void
poolDiscard(STRING_POOL *_Borrow pool) {
  pool->ptr = pool->start;
}
_Safe static inline void
poolFinish(STRING_POOL *_Borrow pool) {
  pool->start = pool->ptr;
}

_Safe bool
poolAppendChar(STRING_POOL *_Borrow pool, XML_Char c) {
  if (pool->ptr == pool->end && ! poolGrow(pool))
    return false;

  _Unsafe *(pool->ptr)++ = c;
  return true;
}

_Safe bool
poolAppendChars(STRING_POOL *_Borrow pool, const XML_Char *_Nonnull s, size_t len) {
  // Detect and prevent integer overflow
  if (len > SIZE_MAX / sizeof(XML_Char))
    return false;

  if (! poolGrowUntil(pool, len))
    return false;

  _Unsafe memcpy(pool->ptr, s, len * sizeof(XML_Char));
  pool->ptr += len;

  return true;
}

#if ! defined(XML_TESTING)
const
#endif
    XML_Bool g_reparseDeferralEnabledDefault
    = XML_TRUE; // write ONLY in runtests.c
#if defined(XML_TESTING)
unsigned int g_bytesScanned = 0; // used for testing only
#endif

struct XML_ParserStruct {
  /* The first member must be m_userData so that the XML_GetUserData
     macro works. */
  void *m_userData;
  void *m_handlerArg;

  // How the four parse buffer pointers below relate in time and space:
  //
  //   m_buffer <= m_bufferPtr <= m_bufferEnd  <= m_bufferLim
  //   |           |              |               |
  //   <--parsed-->|              |               |
  //               <---parsing--->|               |
  //                              <--unoccupied-->|
  //   <---------total-malloced/realloced-------->|

  char *m_buffer; // malloc/realloc base pointer of parse buffer
  const XML_Memory_Handling_Suite m_mem;
  const char *m_bufferPtr; // first character to be parsed
  char *m_bufferEnd;       // past last character to be parsed
  const char *m_bufferLim; // allocated end of m_buffer

  uint64_t m_parseEndByteIndex;
  const char *m_parseEndPtr;
  size_t m_partialTokenBytesBefore; /* used in heuristic to avoid O(n^2) */
  XML_Bool m_reparseDeferralEnabled;
  int m_lastBufferRequestSize;
  XML_Char *_Owned _ArrayElem _Nullable m_dataBuf;
  XML_Char *m_dataBufEnd;
  XML_StartElementHandler m_startElementHandler;
  XML_EndElementHandler m_endElementHandler;
  XML_CharacterDataHandler m_characterDataHandler;
  XML_ProcessingInstructionHandler m_processingInstructionHandler;
  XML_CommentHandler m_commentHandler;
  XML_StartCdataSectionHandler m_startCdataSectionHandler;
  XML_EndCdataSectionHandler m_endCdataSectionHandler;
  XML_DefaultHandler m_defaultHandler;
  XML_StartDoctypeDeclHandler m_startDoctypeDeclHandler;
  XML_EndDoctypeDeclHandler m_endDoctypeDeclHandler;
  XML_UnparsedEntityDeclHandler m_unparsedEntityDeclHandler;
  XML_NotationDeclHandler m_notationDeclHandler;
  XML_StartNamespaceDeclHandler m_startNamespaceDeclHandler;
  XML_EndNamespaceDeclHandler m_endNamespaceDeclHandler;
  XML_NotStandaloneHandler m_notStandaloneHandler;
  XML_ExternalEntityRefHandler m_externalEntityRefHandler;
  XML_Parser m_externalEntityRefHandlerArg;
  XML_SkippedEntityHandler m_skippedEntityHandler;
  XML_UnknownEncodingHandler m_unknownEncodingHandler;
  XML_ElementDeclHandler m_elementDeclHandler;
  XML_AttlistDeclHandler m_attlistDeclHandler;
  XML_EntityDeclHandler m_entityDeclHandler;
  XML_XmlDeclHandler m_xmlDeclHandler;
  const ENCODING *_Nonnull m_encoding;
  INIT_ENCODING m_initEncoding;
  const ENCODING *_Nonnull m_internalEncoding;
  XML_Char *_Owned _ArrayElem _Nullable m_protocolEncodingName;
  XML_Bool m_ns;
  XML_Bool m_ns_triplets;
  char *_Owned _ArrayElem _Nullable m_unknownEncodingMem;
  void *m_unknownEncodingData;
  void *m_unknownEncodingHandlerData;
  // Application callback invoked by callUnknownEncodingConvert.
  int(XMLCALL *m_unknownEncodingConvert)(void *, const char *);
  void(XMLCALL *m_unknownEncodingRelease)(void *);
  PROLOG_STATE m_prologState;
  Processor *m_processor;
  enum XML_Error m_errorCode;
  const char *m_eventPtr;
  const char *m_eventEndPtr;
  const char *m_positionPtr;
  OPEN_INTERNAL_ENTITY *m_openInternalEntities;
  OPEN_INTERNAL_ENTITY *m_openAttributeEntities;
  OPEN_INTERNAL_ENTITY *m_openValueEntities;
  OPEN_INTERNAL_ENTITY *m_freeEntities;
  XML_Bool m_defaultExpandInternalEntities;
  int m_tagLevel;
  ENTITY *m_declEntity;
  const XML_Char *m_doctypeName;
  const XML_Char *m_doctypeSysid;
  const XML_Char *m_doctypePubid;
  const XML_Char *m_declAttributeType;
  const XML_Char *m_declNotationName;
  const XML_Char *m_declNotationPublicId;
  ELEMENT_TYPE *m_declElementType;
  ATTRIBUTE_ID *m_declAttributeId;
  XML_Bool m_declAttributeIsCdata;
  XML_Bool m_declAttributeIsId;
  DTD *m_dtd;
  const XML_Char *m_curBase;
  TAG *m_tagStack;
  TAG *m_freeTagList;
  BINDING *m_inheritedBindings;
  BINDING *m_freeBindingList;
  size_t m_attsSize;
  int m_nSpecifiedAtts;
  int m_idAttIndex;
  ATTRIBUTE *_Owned _ArrayElem _Nullable m_atts;
  NS_ATT *_Owned _ArrayElem _Nullable m_nsAtts;
  unsigned long m_nsAttsVersion;
  unsigned char m_nsAttsPower;
#ifdef XML_ATTR_INFO
  XML_AttrInfo *_Owned _ArrayElem _Nullable m_attInfo;
#endif
  POSITION m_position;
  STRING_POOL m_tempPool;
  STRING_POOL m_temp2Pool;
  char *_Owned _ArrayElem _Nullable m_groupConnector;
  size_t m_groupSize;
  XML_Char m_namespaceSeparator;
  XML_Parser m_parentParser;
  XML_ParsingStatus m_parsingStatus;
#ifdef XML_DTD
  XML_Bool m_isParamEntity;
  XML_Bool m_useForeignDTD;
  enum XML_ParamEntityParsing m_paramEntityParsing;
#endif
  struct sipkey m_hash_secret_salt_128;
  XML_Bool m_hash_secret_salt_set;
#if XML_GE == 1
  ACCOUNTING m_accounting;
  MALLOC_TRACKER m_alloc_tracker;
  ENTITY_STATS m_entity_stats;
#endif
  XML_Bool m_reenter;
  unsigned m_handlerCallDepth;
};

#if XML_GE == 1
#  define MALLOC(parser, s) (expat_malloc((parser), (s), __LINE__))
#  define REALLOC(parser, p, s) (expat_realloc((parser), (p), (s), __LINE__))
#  define FREE(parser, p) (expat_free((parser), (p), __LINE__))
#else
#  define MALLOC(parser, s) (_Unsafe((parser)->m_mem.malloc_fcn((s))))
#  define REALLOC(parser, p, s) (_Unsafe((parser)->m_mem.realloc_fcn((p), (s))))
#  define FREE(parser, p) (_Unsafe((parser)->m_mem.free_fcn((p))))
#endif
// Borrow the parser behind a raw back-reference for one allocator call.
#define PARSER_OF(x) (_Unsafe(&_Mut *(x)->parser))
// Raw alias of a borrowed parser, for storage in raw back-reference fields.
#define RAW_PARSER(p) (_Unsafe((XML_Parser)(void *)(p)))
// Take ownership of a freshly allocated array; a nullptr result stays nullptr.
#define TAKE_ARRAY(T, raw) (_Unsafe(__take_array_from_raw((T *)(raw))))
// Release an owned array field through a borrowed owner.
#define FREE_ARRAY(parser, field, T)                                          \
  do {                                                                         \
    T *_Owned _ArrayElem _Nullable bsc_tmp = nullptr;                          \
    safe_swap(&_Mut(field), &_Mut bsc_tmp);                                    \
    if (bsc_tmp != nullptr)                                                    \
      _Unsafe FREE(parser, __move_array_to_raw(bsc_tmp));                      \
  } while (0)
// Grow an owned array field in place; on failure the old buffer stays owned.
#define REALLOC_ARRAY(parser, field, T, bytes, ok)                             \
  do {                                                                         \
    T *_Owned _ArrayElem _Nullable bsc_old = nullptr;                          \
    T *bsc_raw = nullptr;                                                      \
    safe_swap(&_Mut(field), &_Mut bsc_old);                                    \
    if (bsc_old != nullptr)                                                    \
      _Unsafe bsc_raw = __move_array_to_raw(bsc_old);                          \
    T *bsc_new = _Unsafe((T *)REALLOC(parser, bsc_raw, (bytes)));              \
    if (bsc_new == nullptr) {                                                  \
      _Unsafe { (field) = __take_array_from_raw(bsc_raw); }                    \
      (ok) = XML_FALSE;                                                        \
    } else {                                                                   \
      _Unsafe { (field) = __take_array_from_raw(bsc_new); }                    \
      (ok) = XML_TRUE;                                                         \
    }                                                                          \
  } while (0)

#if XML_GE == 1
_Safe static void
expat_heap_stat(XML_Parser _Borrow rootParser, char heapOp, XmlBigCount absDiff,
                XmlBigCount newTotal, XmlBigCount peakTotal, int sourceLine) {
  // NOTE: This can be +infinity or -nan
  const float amplification
      = (float)newTotal / (float)rootParser->m_accounting.countBytesDirect;
  _Unsafe fprintf(
      stderr,
      "expat: Allocations(%p): Direct " EXPAT_FMT_ULL("10") ", allocated %c" EXPAT_FMT_ULL(
          "10") " to " EXPAT_FMT_ULL("10") " (" EXPAT_FMT_ULL("10") " peak), amplification %8.2f (xmlparse.c:%d)\n",
      (void *)rootParser, rootParser->m_accounting.countBytesDirect, heapOp,
      absDiff, newTotal, peakTotal, (double)amplification, sourceLine);
}

_Safe static bool
expat_heap_increase_tolerable(XML_Parser _Borrow rootParser, XmlBigCount increase,
                              int sourceLine) {
  _Unsafe assert(rootParser != nullptr);
  _Unsafe assert(increase > 0);

  XmlBigCount newTotal = 0;
  bool tolerable = true;

  // Detect integer overflow
  if ((XmlBigCount)-1 - rootParser->m_alloc_tracker.bytesAllocated < increase) {
    tolerable = false;
  } else {
    newTotal = rootParser->m_alloc_tracker.bytesAllocated + increase;

    if (newTotal >= rootParser->m_alloc_tracker.activationThresholdBytes) {
      _Unsafe assert(newTotal > 0);
      // NOTE: This can be +infinity when dividing by zero but not -nan
      const float amplification
          = (float)newTotal / (float)rootParser->m_accounting.countBytesDirect;
      if (amplification
          > rootParser->m_alloc_tracker.maximumAmplificationFactor) {
        tolerable = false;
      }
    }
  }

  if (! tolerable && (rootParser->m_alloc_tracker.debugLevel >= 1)) {
    expat_heap_stat(rootParser, '+', increase, newTotal, newTotal, sourceLine);
  }

  return tolerable;
}

#  if defined(XML_TESTING)
_Safe void *
#  else
_Safe static void *
#  endif
expat_malloc(XML_Parser _Borrow parser, size_t size, int sourceLine) {
  // Detect integer overflow
  if (SIZE_MAX - size < sizeof(size_t) + EXPAT_MALLOC_PADDING) {
    return nullptr;
  }

  const XML_Memory_Handling_Suite mem = parser->m_mem;
  XML_Parser _Borrow rootParser = getRootParserOf(parser, nullptr);
  _Unsafe assert(rootParser->m_parentParser == nullptr);

  const size_t bytesToAllocate = sizeof(size_t) + EXPAT_MALLOC_PADDING + size;

  if ((XmlBigCount)-1 - rootParser->m_alloc_tracker.bytesAllocated
      < bytesToAllocate) {
    return nullptr; // i.e. signal integer overflow as out-of-memory
  }

  if (! expat_heap_increase_tolerable(rootParser, bytesToAllocate,
                                      sourceLine)) {
    return nullptr; // i.e. signal violation as out-of-memory
  }

  // Actually allocate
  void *const mallocedPtr = _Unsafe((mem.malloc_fcn(bytesToAllocate)));

  if (mallocedPtr == nullptr) {
    return nullptr;
  }

  // Update in-block recorded size
  _Unsafe *(size_t *)mallocedPtr = size;

  // Update accounting
  rootParser->m_alloc_tracker.bytesAllocated += bytesToAllocate;

  // Report as needed
  if (rootParser->m_alloc_tracker.debugLevel >= 2) {
    if (rootParser->m_alloc_tracker.bytesAllocated
        > rootParser->m_alloc_tracker.peakBytesAllocated) {
      rootParser->m_alloc_tracker.peakBytesAllocated
          = rootParser->m_alloc_tracker.bytesAllocated;
    }
    {
      const XmlBigCount curBytes = rootParser->m_alloc_tracker.bytesAllocated;
      const XmlBigCount peakBytes = rootParser->m_alloc_tracker.peakBytesAllocated;
      expat_heap_stat(rootParser, '+', bytesToAllocate, curBytes, peakBytes, sourceLine);
    }
  }

  return _Unsafe(((char *)mallocedPtr + sizeof(size_t) + EXPAT_MALLOC_PADDING));
}

#  if defined(XML_TESTING)
_Safe void
#  else
_Safe static void
#  endif
expat_free(XML_Parser _Borrow parser, void *ptr, int sourceLine) {
  _Unsafe assert(parser != nullptr);

  if (ptr == nullptr) {
    return;
  }

  XML_Parser _Borrow rootParser = getRootParserOf(parser, nullptr);
  _Unsafe assert(rootParser->m_parentParser == nullptr);

  // Extract size (to the eyes of malloc_fcn/realloc_fcn) and
  // the original pointer returned by malloc/realloc
  void *const mallocedPtr = _Unsafe(((char *)ptr - EXPAT_MALLOC_PADDING - sizeof(size_t)));
  const size_t bytesAllocated
      = _Unsafe((sizeof(size_t) + EXPAT_MALLOC_PADDING + *(size_t *)mallocedPtr));

  // Update accounting
  _Unsafe assert(rootParser->m_alloc_tracker.bytesAllocated >= bytesAllocated);
  rootParser->m_alloc_tracker.bytesAllocated -= bytesAllocated;

  // Report as needed
  if (rootParser->m_alloc_tracker.debugLevel >= 2) {
    {
      const XmlBigCount curBytes = rootParser->m_alloc_tracker.bytesAllocated;
      const XmlBigCount peakBytes = rootParser->m_alloc_tracker.peakBytesAllocated;
      expat_heap_stat(rootParser, '-', bytesAllocated, curBytes, peakBytes, sourceLine);
    }
  }

  // NOTE: This may be freeing rootParser, so freeing has to come last
  _Unsafe parser->m_mem.free_fcn(mallocedPtr);
}

#  if defined(XML_TESTING)
_Safe void *
#  else
_Safe static void *
#  endif
expat_realloc(XML_Parser _Borrow parser, void *ptr, size_t size, int sourceLine) {
  _Unsafe assert(parser != nullptr);

  if (ptr == nullptr) {
    return expat_malloc(parser, size, sourceLine);
  }

  if (size == 0) {
    expat_free(parser, ptr, sourceLine);
    return nullptr;
  }

  const XML_Memory_Handling_Suite mem = parser->m_mem;
  XML_Parser _Borrow rootParser = getRootParserOf(parser, nullptr);
  _Unsafe assert(rootParser->m_parentParser == nullptr);

  // Extract original size (to the eyes of the caller) and the original
  // pointer returned by malloc/realloc
  void *mallocedPtr = _Unsafe(((char *)ptr - EXPAT_MALLOC_PADDING - sizeof(size_t)));
  const size_t prevSize = _Unsafe((*(size_t *)mallocedPtr));

  // Classify upcoming change
  const bool isIncrease = (size > prevSize);
  const size_t absDiff
      = (size > prevSize) ? (size - prevSize) : (prevSize - size);

  // Ask for permission from accounting
  if (isIncrease) {
    if (! expat_heap_increase_tolerable(rootParser, absDiff, sourceLine)) {
      return nullptr; // i.e. signal violation as out-of-memory
    }
  }

  // NOTE: Integer overflow detection has already been done for us
  //       by expat_heap_increase_tolerable(..) above
  _Unsafe assert(SIZE_MAX - sizeof(size_t) - EXPAT_MALLOC_PADDING >= size);

  // Actually allocate
  _Unsafe mallocedPtr = mem.realloc_fcn(mallocedPtr,
                                sizeof(size_t) + EXPAT_MALLOC_PADDING + size);

  if (mallocedPtr == nullptr) {
    return nullptr;
  }

  // Update accounting
  if (isIncrease) {
    _Unsafe assert((XmlBigCount)-1 - rootParser->m_alloc_tracker.bytesAllocated
           >= absDiff);
    rootParser->m_alloc_tracker.bytesAllocated += absDiff;
  } else { // i.e. decrease
    _Unsafe assert(rootParser->m_alloc_tracker.bytesAllocated >= absDiff);
    rootParser->m_alloc_tracker.bytesAllocated -= absDiff;
  }

  // Report as needed
  if (rootParser->m_alloc_tracker.debugLevel >= 2) {
    if (rootParser->m_alloc_tracker.bytesAllocated
        > rootParser->m_alloc_tracker.peakBytesAllocated) {
      rootParser->m_alloc_tracker.peakBytesAllocated
          = rootParser->m_alloc_tracker.bytesAllocated;
    }
    {
      const XmlBigCount curBytes = rootParser->m_alloc_tracker.bytesAllocated;
      const XmlBigCount peakBytes = rootParser->m_alloc_tracker.peakBytesAllocated;
      expat_heap_stat(rootParser, isIncrease ? '+' : '-', absDiff, curBytes, peakBytes, sourceLine);
    }
  }

  // Update in-block recorded size
  _Unsafe *(size_t *)mallocedPtr = size;

  return _Unsafe(((char *)mallocedPtr + sizeof(size_t) + EXPAT_MALLOC_PADDING));
}
#endif // XML_GE == 1

_Safe XML_Parser _Owned _Nullable XMLCALL
XML_ParserCreate(const XML_Char *_Nonnull encodingName) {
  return XML_ParserCreate_MM(encodingName, nullptr, nullptr);
}

_Safe XML_Parser _Owned _Nullable XMLCALL
XML_ParserCreateNS(const XML_Char *_Nonnull encodingName, XML_Char nsSep) {
  XML_Char tmp[2] = {nsSep, 0};
  return XML_ParserCreate_MM(encodingName, nullptr, tmp);
}

// "xml=http://www.w3.org/XML/1998/namespace"
static const XML_Char implicitContext[]
    = {ASCII_x,     ASCII_m,     ASCII_l,      ASCII_EQUALS, ASCII_h,
       ASCII_t,     ASCII_t,     ASCII_p,      ASCII_COLON,  ASCII_SLASH,
       ASCII_SLASH, ASCII_w,     ASCII_w,      ASCII_w,      ASCII_PERIOD,
       ASCII_w,     ASCII_3,     ASCII_PERIOD, ASCII_o,      ASCII_r,
       ASCII_g,     ASCII_SLASH, ASCII_X,      ASCII_M,      ASCII_L,
       ASCII_SLASH, ASCII_1,     ASCII_9,      ASCII_9,      ASCII_8,
       ASCII_SLASH, ASCII_n,     ASCII_a,      ASCII_m,      ASCII_e,
       ASCII_s,     ASCII_p,     ASCII_a,      ASCII_c,      ASCII_e,
       '\0'};

#if ! defined(HAVE_ARC4RANDOM_BUF) && ! defined(HAVE_ARC4RANDOM)

_Safe static unsigned long
gather_time_entropy(void) {
#  ifdef _WIN32
  FILETIME ft;
  _Unsafe GetSystemTimeAsFileTime(_Unsafe(&ft)); /* never fails */
  return ft.dwHighDateTime ^ ft.dwLowDateTime;
#  else
  struct timeval tv = {0, 0};
  int gettimeofday_res;

  gettimeofday_res = _Unsafe(gettimeofday(_Unsafe(&tv), nullptr));

#    if defined(NDEBUG)
  (void)gettimeofday_res;
#    else
  _Unsafe assert(gettimeofday_res == 0);
#    endif /* defined(NDEBUG) */

  /* Microseconds time is <20 bits entropy */
  return (unsigned long)tv.tv_usec;
#  endif
}

#endif /* ! defined(HAVE_ARC4RANDOM_BUF) && ! defined(HAVE_ARC4RANDOM) */

_Safe static struct sipkey
ENTROPY_DEBUG(const char *label, struct sipkey entropy_128) {
  if (getDebugLevel("EXPAT_ENTROPY_DEBUG", 0) >= 1u) {
    _Unsafe fprintf(stderr,
            "expat: Entropy: %s --> [0x" EXPAT_FMT_LLX(
                "016") ", 0x" EXPAT_FMT_LLX("016") "] (16 bytes)\n",
            label, (unsigned long long)entropy_128.k[0],
            (unsigned long long)entropy_128.k[1]);
  }
  return entropy_128;
}

_Safe static struct sipkey
generate_hash_secret_salt(void) {
  struct sipkey entropy = {{0, 0}};

  /* "Failproof" high quality providers: */
#if defined(HAVE_ARC4RANDOM_BUF)
  _Unsafe writeRandomBytes_arc4random_buf(_Unsafe(&entropy), sizeof(entropy));
  return ENTROPY_DEBUG("arc4random_buf", entropy);
#elif defined(HAVE_ARC4RANDOM)
  _Unsafe writeRandomBytes_arc4random(_Unsafe(&entropy), sizeof(entropy));
  return ENTROPY_DEBUG("arc4random", entropy);
#else
  /* Try high quality providers first .. */
#  ifdef _WIN32
  if (_Unsafe(writeRandomBytes_rand_s(_Unsafe(&entropy), sizeof(entropy)))) {
    return ENTROPY_DEBUG("rand_s", entropy);
  }
#  elif defined(HAVE_GETENTROPY)
  if (_Unsafe(writeRandomBytes_getentropy(_Unsafe(&entropy), sizeof(entropy)))) {
    return ENTROPY_DEBUG("getentropy", entropy);
  }
  _Unsafe errno = 0;
#  elif defined(HAVE_GETRANDOM) || defined(HAVE_SYSCALL_GETRANDOM)
  if (_Unsafe(writeRandomBytes_getrandom_nonblock(_Unsafe(&entropy), sizeof(entropy)))) {
    return ENTROPY_DEBUG("getrandom", entropy);
  }
#  endif
#  if ! defined(_WIN32) && defined(XML_DEV_URANDOM)
  if (_Unsafe(writeRandomBytes_dev_urandom(_Unsafe(&entropy), sizeof(entropy)))) {
    return ENTROPY_DEBUG("/dev/urandom", entropy);
  }
#  endif /* ! defined(_WIN32) && defined(XML_DEV_URANDOM) */
  /* .. and self-made low quality for backup: */

  entropy.k[0] = 0;
  entropy.k[1] = gather_time_entropy();
#  if ! defined(__wasi__)
  /* Process ID is 0 bits entropy if attacker has local access */
  entropy.k[1] ^= _Unsafe(getpid());
#  endif

  /* Factors are 2^31-1 and 2^61-1 (Mersenne primes M31 and M61) */
  if (sizeof(unsigned long) == 4) {
    entropy.k[1] *= 2147483647;
    return ENTROPY_DEBUG("fallback(4)", entropy);
  } else {
    entropy.k[1] *= 2305843009213693951ULL;
    return ENTROPY_DEBUG("fallback(8)", entropy);
  }
#endif
}

_Safe static void
beforeHandler(XML_Parser _Borrow parser) {
  _Unsafe assert(parser->m_handlerCallDepth < UINT_MAX);
  parser->m_handlerCallDepth++;
}

_Safe static void
afterHandler(XML_Parser _Borrow parser) {
  _Unsafe assert(parser->m_handlerCallDepth > 0);
  parser->m_handlerCallDepth--;
}

_Safe static bool
isCalledFromInsideHandler(XML_Parser _Borrow parser) {
  return parser->m_handlerCallDepth > 0;
}

_Safe static void
callUnknownEncodingRelease(XML_Parser _Borrow parser) {
  beforeHandler(parser);
  _Unsafe(parser->m_unknownEncodingRelease(parser->m_unknownEncodingData));
  afterHandler(parser);
  parser->m_unknownEncodingRelease = nullptr;
  parser->m_unknownEncodingData = nullptr;
}

static int XMLCALL
callUnknownEncodingConvert(void *data, const char *p) {
  XML_Parser parser = data;
  beforeHandler(&_Mut *parser);
  const int result
      = _Unsafe(parser->m_unknownEncodingConvert(parser->m_unknownEncodingData, p));
  afterHandler(&_Mut *parser);
  return result;
}

_Safe static enum XML_Error
callProcessor(XML_Parser _Borrow parser, const char *start, const char *end,
              const char **_Nonnull endPtr) {
  const size_t have_now = (size_t)EXPAT_SAFE_PTR_DIFF(end, start);

  if (parser->m_reparseDeferralEnabled
      && ! parser->m_parsingStatus.finalBuffer) {
    // Heuristic: don't try to parse a partial token again until the amount of
    // available data has increased significantly.
    const size_t had_before = parser->m_partialTokenBytesBefore;
    // ...but *do* try anyway if we're close to causing a reallocation.
    size_t available_buffer
        = (size_t)EXPAT_SAFE_PTR_DIFF(parser->m_bufferPtr, parser->m_buffer);
#if XML_CONTEXT_BYTES > 0
    available_buffer -= EXPAT_MIN(available_buffer, XML_CONTEXT_BYTES);
#endif
    available_buffer
        += EXPAT_SAFE_PTR_DIFF(parser->m_bufferLim, parser->m_bufferEnd);
    // m_lastBufferRequestSize is never assigned a value < 0, so the cast is ok
    const bool enough
        = (have_now >= 2 * had_before)
          || ((size_t)parser->m_lastBufferRequestSize > available_buffer);

    if (! enough) {
      _Unsafe *endPtr = start; // callers may expect this to be set
      return XML_ERROR_NONE;
    }
  }
#if defined(XML_TESTING)
  g_bytesScanned += (unsigned)have_now;
#endif
  // Run in a loop to eliminate dangerous recursion depths
  enum XML_Error ret;
  _Unsafe *endPtr = start;
  while (1) {
    // Use endPtr as the new start in each iteration, since it will
    // be set to the next start point by m_processor.
    __auto_type bsc_h0 = parser->m_processor;
    ret = _Unsafe(bsc_h0(parser, *endPtr, end, endPtr));

    // Make parsing status (and in particular XML_SUSPENDED) take
    // precedence over re-enter flag when they disagree
    if (parser->m_parsingStatus.parsing != XML_PARSING) {
      parser->m_reenter = XML_FALSE;
    }

    if (! parser->m_reenter) {
      break;
    }

    parser->m_reenter = XML_FALSE;
    if (ret != XML_ERROR_NONE)
      return ret;
  }

  if (ret == XML_ERROR_NONE) {
    // if we consumed nothing, remember what we had on this parse attempt.
    if (_Unsafe((*endPtr == start))) {
      parser->m_partialTokenBytesBefore = have_now;
    } else {
      parser->m_partialTokenBytesBefore = 0;
    }
  }
  return ret;
}

static XML_Bool /* only valid for root parser */
startParsing(XML_Parser _Borrow parser) {
  /* hash functions must be initialized before setContext() is called */
  if (parser->m_hash_secret_salt_set != XML_TRUE) {
    parser->m_hash_secret_salt_128 = generate_hash_secret_salt();
    parser->m_hash_secret_salt_set = XML_TRUE;
  }
  if (parser->m_ns) {
    /* implicit context only set for root parser, since child
       parsers (i.e. external entity parsers) will inherit it
    */
    return setContext(parser, implicitContext);
  }
  return XML_TRUE;
}

_Safe XML_Parser _Owned _Nullable XMLCALL
XML_ParserCreate_MM(const XML_Char *_Nonnull encodingName,
                    const XML_Memory_Handling_Suite *_Borrow _Nullable memsuite,
                    const XML_Char *nameSep) {
  return parserCreate(encodingName, memsuite, nameSep, nullptr, nullptr);
}

_Safe static XML_Parser _Owned _Nullable
parserCreate(const XML_Char *_Nonnull encodingName,
             const XML_Memory_Handling_Suite *_Borrow _Nullable memsuite, const XML_Char *nameSep,
             DTD *dtd, XML_Parser _Borrow _Nullable parentParser) {
  XML_Parser parser = nullptr;

#if XML_GE == 1
  const size_t increase
      = sizeof(size_t) + EXPAT_MALLOC_PADDING + sizeof(struct XML_ParserStruct);

  if (parentParser != nullptr) {
    XML_Parser _Borrow rootParser = getRootParserOf(parentParser, nullptr);
    if (! expat_heap_increase_tolerable(rootParser, increase, __LINE__)) {
      return nullptr;
    }
  }
#else
  UNUSED_P(parentParser);
#endif

  if (memsuite) {
    XML_Memory_Handling_Suite *mtemp;
#if XML_GE == 1
    void *const sizeAndParser
        = _Unsafe((memsuite->malloc_fcn(sizeof(size_t) + EXPAT_MALLOC_PADDING
                               + sizeof(struct XML_ParserStruct))));
    if (sizeAndParser != nullptr) {
      _Unsafe *(size_t *)sizeAndParser = sizeof(struct XML_ParserStruct);
      _Unsafe parser = (XML_Parser)((char *)sizeAndParser + sizeof(size_t)
                            + EXPAT_MALLOC_PADDING);
#else
    parser = memsuite->malloc_fcn(sizeof(struct XML_ParserStruct));
    if (parser != nullptr) {
#endif
      _Unsafe mtemp = (XML_Memory_Handling_Suite *)_Unsafe(_Unsafe(&parser->m_mem));
      _Unsafe mtemp->malloc_fcn = memsuite->malloc_fcn;
      _Unsafe mtemp->realloc_fcn = memsuite->realloc_fcn;
      _Unsafe mtemp->free_fcn = memsuite->free_fcn;
    }
  } else {
    XML_Memory_Handling_Suite *mtemp;
#if XML_GE == 1
    void *const sizeAndParser = _Unsafe((malloc(sizeof(size_t) + EXPAT_MALLOC_PADDING
                                       + sizeof(struct XML_ParserStruct))));
    if (sizeAndParser != nullptr) {
      _Unsafe *(size_t *)sizeAndParser = sizeof(struct XML_ParserStruct);
      _Unsafe parser = (XML_Parser)((char *)sizeAndParser + sizeof(size_t)
                            + EXPAT_MALLOC_PADDING);
#else
    parser = malloc(sizeof(struct XML_ParserStruct));
    if (parser != nullptr) {
#endif
      _Unsafe mtemp = (XML_Memory_Handling_Suite *)_Unsafe(_Unsafe(&parser->m_mem));
      _Unsafe mtemp->malloc_fcn = malloc;
      _Unsafe mtemp->realloc_fcn = realloc;
      _Unsafe mtemp->free_fcn = free;
    }
  } // cppcheck-suppress[memleak symbolName=sizeAndParser] // Cppcheck >=2.18.0

  if (! parser)
    return nullptr;
  XML_Parser rawParser = parser;
  XML_Parser _Borrow p = _Unsafe(&_Mut *parser);

#if XML_GE == 1
  // Initialize .m_alloc_tracker
  _Unsafe memset(&p->m_alloc_tracker, 0, sizeof(MALLOC_TRACKER));
  if (parentParser == nullptr) {
    p->m_alloc_tracker.debugLevel
        = getDebugLevel("EXPAT_MALLOC_DEBUG", 0u);
    p->m_alloc_tracker.maximumAmplificationFactor
        = EXPAT_ALLOC_TRACKER_MAXIMUM_AMPLIFICATION_DEFAULT;
    p->m_alloc_tracker.activationThresholdBytes
        = EXPAT_ALLOC_TRACKER_ACTIVATION_THRESHOLD_DEFAULT;

    // NOTE: This initialization needs to come this early because these fields
    //       are read by allocation tracking code
    p->m_parentParser = nullptr;
    p->m_accounting.countBytesDirect = 0;
  } else {
    p->m_parentParser = RAW_PARSER(parentParser);
  }

  // Record XML_ParserStruct allocation we did a few lines up before
  XML_Parser _Borrow rootParser = getRootParserOf(p, nullptr);
  _Unsafe assert(rootParser->m_parentParser == nullptr);
  _Unsafe assert(SIZE_MAX - rootParser->m_alloc_tracker.bytesAllocated >= increase);
  rootParser->m_alloc_tracker.bytesAllocated += increase;

  // Report on allocation
  if (rootParser->m_alloc_tracker.debugLevel >= 2) {
    if (rootParser->m_alloc_tracker.bytesAllocated
        > rootParser->m_alloc_tracker.peakBytesAllocated) {
      rootParser->m_alloc_tracker.peakBytesAllocated
          = rootParser->m_alloc_tracker.bytesAllocated;
    }

    {

      const XmlBigCount curBytes = rootParser->m_alloc_tracker.bytesAllocated;

      const XmlBigCount peakBytes = rootParser->m_alloc_tracker.peakBytesAllocated;

      expat_heap_stat(rootParser, '+', increase, curBytes, peakBytes, __LINE__);

    }
  }
#else
  p->m_parentParser = nullptr;
#endif // XML_GE == 1

  p->m_buffer = nullptr;
  p->m_bufferLim = nullptr;

  p->m_attsSize = INIT_ATTS_SIZE;
  __auto_type bsc_h1 = p->m_attsSize;
  p->m_atts = TAKE_ARRAY(
      ATTRIBUTE, MALLOC(p, bsc_h1 * sizeof(ATTRIBUTE)));
  if (p->m_atts == nullptr) {
    _Unsafe FREE(p, rawParser);
    return nullptr;
  }
#ifdef XML_ATTR_INFO
  p->m_attInfo = TAKE_ARRAY(
      XML_AttrInfo, MALLOC(p, p->m_attsSize * sizeof(XML_AttrInfo)));
  if (p->m_attInfo == nullptr) {
    FREE_ARRAY(p, p->m_atts, ATTRIBUTE);
    FREE(p, rawParser);
    return nullptr;
  }
#endif
  p->m_dataBuf = TAKE_ARRAY(
      XML_Char, MALLOC(p, INIT_DATA_BUF_SIZE * sizeof(XML_Char)));
  if (p->m_dataBuf == nullptr) {
    FREE_ARRAY(p, p->m_atts, ATTRIBUTE);
#ifdef XML_ATTR_INFO
    FREE_ARRAY(p, p->m_attInfo, XML_AttrInfo);
#endif
    _Unsafe FREE(p, rawParser);
    return nullptr;
  }
  p->m_dataBufEnd
      = _Unsafe((XML_Char *)&_Mut *p->m_dataBuf + INIT_DATA_BUF_SIZE);

  if (dtd)
    p->m_dtd = dtd;
  else {
    p->m_dtd = dtdCreate(p);
    if (p->m_dtd == nullptr) {
      FREE_ARRAY(p, p->m_dataBuf, XML_Char);
      FREE_ARRAY(p, p->m_atts, ATTRIBUTE);
#ifdef XML_ATTR_INFO
      FREE_ARRAY(p, p->m_attInfo, XML_AttrInfo);
#endif
      _Unsafe FREE(p, rawParser);
      return nullptr;
    }
  }

  p->m_freeBindingList = nullptr;
  p->m_freeTagList = nullptr;
  p->m_freeEntities = nullptr;

  p->m_groupSize = 0;
  p->m_groupConnector = nullptr;

  p->m_unknownEncodingHandler = nullptr;
  p->m_unknownEncodingHandlerData = nullptr;

  p->m_namespaceSeparator = ASCII_EXCL;
  p->m_ns = XML_FALSE;
  p->m_ns_triplets = XML_FALSE;

  p->m_nsAtts = nullptr;
  p->m_nsAttsVersion = 0;
  p->m_nsAttsPower = 0;

  p->m_protocolEncodingName = nullptr;

  poolInit(&_Mut p->m_tempPool, rawParser);
  poolInit(&_Mut p->m_temp2Pool, rawParser);
  parserInit(p, encodingName);

  if (encodingName && ! p->m_protocolEncodingName) {
    if (dtd) {
      // We need to stop the upcoming call to XML_ParserFree from happily
      // destroying p->m_dtd because the DTD is shared with the parent
      // parser and the only guard that keeps XML_ParserFree from destroying
      // p->m_dtd is p->m_isParamEntity but it will be set to
      // XML_TRUE only later in XML_ExternalEntityParserCreate (or not at all).
      p->m_dtd = nullptr;
    }
    _Unsafe XML_ParserFree(__take_from_raw(rawParser));
    return nullptr;
  }

  if (nameSep) {
    p->m_ns = XML_TRUE;
    p->m_internalEncoding = _Unsafe(XmlGetInternalEncodingNS());
    _Unsafe p->m_namespaceSeparator = *nameSep;
  } else {
    p->m_internalEncoding = _Unsafe(XmlGetInternalEncoding());
  }

  return _Unsafe(__take_from_raw(rawParser));
}

_Safe static void
parserInit(XML_Parser _Borrow parser, const XML_Char *_Nonnull encodingName) {
  parser->m_processor = prologInitProcessor;
  XmlPrologStateInit(&_Mut parser->m_prologState);
  if (encodingName != nullptr) {
    parser->m_protocolEncodingName = copyString(encodingName, parser);
  }
  parser->m_curBase = nullptr;
  _Unsafe(XmlInitEncoding(_Unsafe(&parser->m_initEncoding), _Unsafe(&parser->m_encoding), 0));
  parser->m_userData = nullptr;
  parser->m_handlerArg = nullptr;
  parser->m_startElementHandler = nullptr;
  parser->m_endElementHandler = nullptr;
  parser->m_characterDataHandler = nullptr;
  parser->m_processingInstructionHandler = nullptr;
  parser->m_commentHandler = nullptr;
  parser->m_startCdataSectionHandler = nullptr;
  parser->m_endCdataSectionHandler = nullptr;
  parser->m_defaultHandler = nullptr;
  parser->m_startDoctypeDeclHandler = nullptr;
  parser->m_endDoctypeDeclHandler = nullptr;
  parser->m_unparsedEntityDeclHandler = nullptr;
  parser->m_notationDeclHandler = nullptr;
  parser->m_startNamespaceDeclHandler = nullptr;
  parser->m_endNamespaceDeclHandler = nullptr;
  parser->m_notStandaloneHandler = nullptr;
  parser->m_externalEntityRefHandler = nullptr;
  parser->m_externalEntityRefHandlerArg = RAW_PARSER(&_Mut *parser);
  parser->m_skippedEntityHandler = nullptr;
  parser->m_elementDeclHandler = nullptr;
  parser->m_attlistDeclHandler = nullptr;
  parser->m_entityDeclHandler = nullptr;
  parser->m_xmlDeclHandler = nullptr;
  parser->m_bufferPtr = parser->m_buffer;
  parser->m_bufferEnd = parser->m_buffer;
  parser->m_parseEndByteIndex = 0;
  parser->m_parseEndPtr = nullptr;
  parser->m_partialTokenBytesBefore = 0;
  parser->m_reparseDeferralEnabled = g_reparseDeferralEnabledDefault;
  parser->m_lastBufferRequestSize = 0;
  parser->m_declElementType = nullptr;
  parser->m_declAttributeId = nullptr;
  parser->m_declEntity = nullptr;
  parser->m_doctypeName = nullptr;
  parser->m_doctypeSysid = nullptr;
  parser->m_doctypePubid = nullptr;
  parser->m_declAttributeType = nullptr;
  parser->m_declNotationName = nullptr;
  parser->m_declNotationPublicId = nullptr;
  parser->m_declAttributeIsCdata = XML_FALSE;
  parser->m_declAttributeIsId = XML_FALSE;
  _Unsafe memset(_Unsafe(&parser->m_position), 0, sizeof(POSITION));
  parser->m_errorCode = XML_ERROR_NONE;
  parser->m_eventPtr = nullptr;
  parser->m_eventEndPtr = nullptr;
  parser->m_positionPtr = nullptr;
  parser->m_openInternalEntities = nullptr;
  parser->m_openAttributeEntities = nullptr;
  parser->m_openValueEntities = nullptr;
  parser->m_defaultExpandInternalEntities = XML_TRUE;
  parser->m_tagLevel = 0;
  parser->m_tagStack = nullptr;
  parser->m_inheritedBindings = nullptr;
  parser->m_nSpecifiedAtts = 0;
  parser->m_unknownEncodingMem = nullptr;
  parser->m_unknownEncodingConvert = nullptr;
  parser->m_unknownEncodingRelease = nullptr;
  parser->m_unknownEncodingData = nullptr;
  parser->m_parsingStatus.parsing = XML_INITIALIZED;
  // Reentry can only be triggered inside m_processor calls
  parser->m_reenter = XML_FALSE;
  parser->m_handlerCallDepth = 0;
#ifdef XML_DTD
  parser->m_isParamEntity = XML_FALSE;
  parser->m_useForeignDTD = XML_FALSE;
  parser->m_paramEntityParsing = XML_PARAM_ENTITY_PARSING_NEVER;
#endif
  parser->m_hash_secret_salt_128.k[0] = 0;
  parser->m_hash_secret_salt_128.k[1] = 0;
  parser->m_hash_secret_salt_set = XML_FALSE;

#if XML_GE == 1
  _Unsafe memset(_Unsafe(&parser->m_accounting), 0, sizeof(ACCOUNTING));
  parser->m_accounting.debugLevel = getDebugLevel("EXPAT_ACCOUNTING_DEBUG", 0u);
  parser->m_accounting.maximumAmplificationFactor
      = EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT;
  parser->m_accounting.activationThresholdBytes
      = EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT;

  _Unsafe memset(_Unsafe(&parser->m_entity_stats), 0, sizeof(ENTITY_STATS));
  parser->m_entity_stats.debugLevel = getDebugLevel("EXPAT_ENTITY_DEBUG", 0u);
#endif
}

/* moves list of bindings to m_freeBindingList */
_Safe static void FASTCALL
moveToFreeBindingList(XML_Parser _Borrow parser, BINDING *bindings) {
  while (bindings) {
    BINDING *b = bindings;
    _Unsafe bindings = bindings->nextTagBinding;
    _Unsafe b->nextTagBinding = parser->m_freeBindingList;
    parser->m_freeBindingList = b;
  }
}

/* Moves a list of entities onto the start of another list. */
_Safe static void
moveEntityList(OPEN_INTERNAL_ENTITY **dst, OPEN_INTERNAL_ENTITY **src) {
  _Unsafe for (OPEN_INTERNAL_ENTITY *head = *src; head != nullptr;) {
    OPEN_INTERNAL_ENTITY *const openEntity = head;
    head = head->next;
    openEntity->next = *dst;
    *dst = openEntity;
  }
}

_Safe XML_Bool XMLCALL
XML_ParserReset(XML_Parser _Borrow _Nullable parser, const XML_Char *_Nonnull encodingName) {
  TAG *tStk;

  if ((parser == nullptr) || isCalledFromInsideHandler(parser))
    return XML_FALSE;

  if (parser->m_parentParser)
    return XML_FALSE;
  /* move m_tagStack to m_freeTagList */
  tStk = parser->m_tagStack;
  while (tStk) {
    TAG *tag = tStk;
    _Unsafe tStk = tStk->parent;
    _Unsafe tag->parent = parser->m_freeTagList;
    _Unsafe moveToFreeBindingList(parser, tag->bindings);
    _Unsafe tag->bindings = nullptr;
    parser->m_freeTagList = tag;
  }
  /* move m_openInternalEntities to m_freeEntities */
  moveEntityList(_Unsafe(&parser->m_freeEntities), _Unsafe(&parser->m_openInternalEntities));
  /* move m_openAttributeEntities to m_freeEntities (i.e. same task but for
   * attributes) */
  moveEntityList(_Unsafe(&parser->m_freeEntities), _Unsafe(&parser->m_openAttributeEntities));
  /* move m_openValueEntities to m_freeEntities (i.e. same task but for value
   * entities) */
  moveEntityList(_Unsafe(&parser->m_freeEntities), _Unsafe(&parser->m_openValueEntities));
  __auto_type bsc_h2 = parser->m_inheritedBindings;
  moveToFreeBindingList(parser, bsc_h2);
  FREE_ARRAY(parser, parser->m_unknownEncodingMem, char);
  if (parser->m_unknownEncodingRelease)
    callUnknownEncodingRelease(parser);
  poolClear(&_Mut parser->m_tempPool);
  poolClear(&_Mut parser->m_temp2Pool);
  FREE_ARRAY(parser, parser->m_protocolEncodingName, XML_Char);
  parserInit(parser, encodingName);
  {
    DTD *dtdRaw = parser->m_dtd;
    if (dtdRaw != nullptr) {
      DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
      dtdReset(dtd, parser);
    }
  }
  return XML_TRUE;
}

_Safe static XML_Bool
parserBusy(XML_Parser _Borrow parser) {
  switch (parser->m_parsingStatus.parsing) {
  case XML_PARSING:
  case XML_SUSPENDED:
    return XML_TRUE;
  case XML_INITIALIZED:
  case XML_FINISHED:
  default:
    return XML_FALSE;
  }
}

_Safe enum XML_Status XMLCALL
XML_SetEncoding(XML_Parser _Borrow _Nullable parser, const XML_Char *_Nonnull encodingName) {
  if (parser == nullptr)
    return XML_STATUS_ERROR;
  /* Block after XML_Parse()/XML_ParseBuffer() has been called.
     XXX There's no way for the caller to determine which of the
     XXX possible error cases caused the XML_STATUS_ERROR return.
  */
  if (parserBusy(parser))
    return XML_STATUS_ERROR;

  /* Get rid of any previous encoding name */
  FREE_ARRAY(parser, parser->m_protocolEncodingName, XML_Char);

  if (encodingName == nullptr)
    /* No new encoding name */
    parser->m_protocolEncodingName = nullptr;
  else {
    /* Copy the new encoding name into allocated memory */
    parser->m_protocolEncodingName = copyString(encodingName, parser);
    if (! parser->m_protocolEncodingName)
      return XML_STATUS_ERROR;
  }
  return XML_STATUS_OK;
}

_Safe XML_Parser _Owned _Nullable XMLCALL
XML_ExternalEntityParserCreate(XML_Parser _Borrow _Nullable oldParser, const XML_Char *_Nonnull context,
                               const XML_Char *_Nonnull encodingName) {
  DTD *newDtd = nullptr;
  DTD *oldDtd;
  XML_StartElementHandler oldStartElementHandler;
  XML_EndElementHandler oldEndElementHandler;
  XML_CharacterDataHandler oldCharacterDataHandler;
  XML_ProcessingInstructionHandler oldProcessingInstructionHandler;
  XML_CommentHandler oldCommentHandler;
  XML_StartCdataSectionHandler oldStartCdataSectionHandler;
  XML_EndCdataSectionHandler oldEndCdataSectionHandler;
  XML_DefaultHandler oldDefaultHandler;
  XML_UnparsedEntityDeclHandler oldUnparsedEntityDeclHandler;
  XML_NotationDeclHandler oldNotationDeclHandler;
  XML_StartNamespaceDeclHandler oldStartNamespaceDeclHandler;
  XML_EndNamespaceDeclHandler oldEndNamespaceDeclHandler;
  XML_NotStandaloneHandler oldNotStandaloneHandler;
  XML_ExternalEntityRefHandler oldExternalEntityRefHandler;
  XML_SkippedEntityHandler oldSkippedEntityHandler;
  XML_UnknownEncodingHandler oldUnknownEncodingHandler;
  void *oldUnknownEncodingHandlerData;
  XML_ElementDeclHandler oldElementDeclHandler;
  XML_AttlistDeclHandler oldAttlistDeclHandler;
  XML_EntityDeclHandler oldEntityDeclHandler;
  XML_XmlDeclHandler oldXmlDeclHandler;
  ELEMENT_TYPE *oldDeclElementType;

  void *oldUserData;
  void *oldHandlerArg;
  XML_Bool oldDefaultExpandInternalEntities;
  XML_Parser oldExternalEntityRefHandlerArg;
#ifdef XML_DTD
  enum XML_ParamEntityParsing oldParamEntityParsing;
  int oldInEntityValue;
#endif
  XML_Bool oldns_triplets;
  /* Note that the new parser shares the same hash secret as the old
     parser, so that dtdCopy and copyEntityTable can lookup values
     from hash tables associated with either parser without us having
     to worry which hash secrets each table has.
  */
  struct sipkey oldhash_secret_salt_128;
  XML_Bool oldhash_secret_salt_set;
  XML_Bool oldReparseDeferralEnabled;

  /* Validate the oldParser parameter before we pull everything out of it */
  if (oldParser == nullptr)
    return nullptr;

  /* Stash the original parser contents on the stack */
  oldDtd = oldParser->m_dtd;
  oldStartElementHandler = oldParser->m_startElementHandler;
  oldEndElementHandler = oldParser->m_endElementHandler;
  oldCharacterDataHandler = oldParser->m_characterDataHandler;
  oldProcessingInstructionHandler = oldParser->m_processingInstructionHandler;
  oldCommentHandler = oldParser->m_commentHandler;
  oldStartCdataSectionHandler = oldParser->m_startCdataSectionHandler;
  oldEndCdataSectionHandler = oldParser->m_endCdataSectionHandler;
  oldDefaultHandler = oldParser->m_defaultHandler;
  oldUnparsedEntityDeclHandler = oldParser->m_unparsedEntityDeclHandler;
  oldNotationDeclHandler = oldParser->m_notationDeclHandler;
  oldStartNamespaceDeclHandler = oldParser->m_startNamespaceDeclHandler;
  oldEndNamespaceDeclHandler = oldParser->m_endNamespaceDeclHandler;
  oldNotStandaloneHandler = oldParser->m_notStandaloneHandler;
  oldExternalEntityRefHandler = oldParser->m_externalEntityRefHandler;
  oldSkippedEntityHandler = oldParser->m_skippedEntityHandler;
  oldUnknownEncodingHandler = oldParser->m_unknownEncodingHandler;
  oldUnknownEncodingHandlerData = oldParser->m_unknownEncodingHandlerData;
  oldElementDeclHandler = oldParser->m_elementDeclHandler;
  oldAttlistDeclHandler = oldParser->m_attlistDeclHandler;
  oldEntityDeclHandler = oldParser->m_entityDeclHandler;
  oldXmlDeclHandler = oldParser->m_xmlDeclHandler;
  oldDeclElementType = oldParser->m_declElementType;

  oldUserData = oldParser->m_userData;
  oldHandlerArg = oldParser->m_handlerArg;
  oldDefaultExpandInternalEntities = oldParser->m_defaultExpandInternalEntities;
  oldExternalEntityRefHandlerArg = oldParser->m_externalEntityRefHandlerArg;
#ifdef XML_DTD
  oldParamEntityParsing = oldParser->m_paramEntityParsing;
  oldInEntityValue = oldParser->m_prologState.inEntityValue;
#endif
  oldns_triplets = oldParser->m_ns_triplets;
  /* Note that the new parser shares the same hash secret as the old
     parser, so that dtdCopy and copyEntityTable can lookup values
     from hash tables associated with either parser without us having
     to worry which hash secrets each table has.
  */
  oldhash_secret_salt_128 = oldParser->m_hash_secret_salt_128;
  oldhash_secret_salt_set = oldParser->m_hash_secret_salt_set;
  oldReparseDeferralEnabled = oldParser->m_reparseDeferralEnabled;

#ifdef XML_DTD
  if (! context)
    newDtd = oldDtd;
#endif /* XML_DTD */

  const XML_Memory_Handling_Suite oldMem = oldParser->m_mem;
  XML_Parser _Owned _Nullable parser = nullptr;
  if (oldParser->m_ns) {
    XML_Char tmp[2] = {oldParser->m_namespaceSeparator, 0};
    parser = parserCreate(encodingName, &_Const oldMem, tmp, newDtd, oldParser);
  } else {
    parser = parserCreate(encodingName, &_Const oldMem, nullptr, newDtd, oldParser);
  }

  if (parser == nullptr)
    return parser;

  parser->m_startElementHandler = oldStartElementHandler;
  parser->m_endElementHandler = oldEndElementHandler;
  parser->m_characterDataHandler = oldCharacterDataHandler;
  parser->m_processingInstructionHandler = oldProcessingInstructionHandler;
  parser->m_commentHandler = oldCommentHandler;
  parser->m_startCdataSectionHandler = oldStartCdataSectionHandler;
  parser->m_endCdataSectionHandler = oldEndCdataSectionHandler;
  parser->m_defaultHandler = oldDefaultHandler;
  parser->m_unparsedEntityDeclHandler = oldUnparsedEntityDeclHandler;
  parser->m_notationDeclHandler = oldNotationDeclHandler;
  parser->m_startNamespaceDeclHandler = oldStartNamespaceDeclHandler;
  parser->m_endNamespaceDeclHandler = oldEndNamespaceDeclHandler;
  parser->m_notStandaloneHandler = oldNotStandaloneHandler;
  parser->m_externalEntityRefHandler = oldExternalEntityRefHandler;
  parser->m_skippedEntityHandler = oldSkippedEntityHandler;
  parser->m_unknownEncodingHandler = oldUnknownEncodingHandler;
  parser->m_unknownEncodingHandlerData = oldUnknownEncodingHandlerData;
  parser->m_elementDeclHandler = oldElementDeclHandler;
  parser->m_attlistDeclHandler = oldAttlistDeclHandler;
  parser->m_entityDeclHandler = oldEntityDeclHandler;
  parser->m_xmlDeclHandler = oldXmlDeclHandler;
  parser->m_declElementType = oldDeclElementType;
  parser->m_userData = oldUserData;
  if (oldUserData == oldHandlerArg)
    parser->m_handlerArg = parser->m_userData;
  else
    parser->m_handlerArg = _Unsafe((void *)&_Mut *parser);
  if (_Unsafe(oldExternalEntityRefHandlerArg != (XML_Parser)(void *)oldParser))
    parser->m_externalEntityRefHandlerArg = oldExternalEntityRefHandlerArg;
  parser->m_defaultExpandInternalEntities = oldDefaultExpandInternalEntities;
  parser->m_ns_triplets = oldns_triplets;
  parser->m_hash_secret_salt_128 = oldhash_secret_salt_128;
  parser->m_hash_secret_salt_set = oldhash_secret_salt_set;
  parser->m_reparseDeferralEnabled = oldReparseDeferralEnabled;
  parser->m_parentParser = RAW_PARSER(oldParser);
#ifdef XML_DTD
  parser->m_paramEntityParsing = oldParamEntityParsing;
  parser->m_prologState.inEntityValue = oldInEntityValue;
  if (context) {
#endif /* XML_DTD */
    DTD *newDtdRaw = parser->m_dtd;
    const DTD *oldDtdRaw = oldParser->m_dtd;
    if (newDtdRaw == nullptr || oldDtdRaw == nullptr) {
      XML_ParserFree(parser);
      return nullptr;
    }
    DTD *_Borrow newDtdB = _Unsafe(&_Mut *newDtdRaw);
    const DTD *_Borrow oldDtdB = _Unsafe(&_Const *oldDtdRaw);
    if (! dtdCopy(oldParser, newDtdB, oldDtdB, &_Mut *parser)
        || ! setContext(&_Mut *parser, context)) {
      XML_ParserFree(parser);
      return nullptr;
    }
    parser->m_processor = externalEntityInitProcessor;
#ifdef XML_DTD
  } else {
    /* The DTD instance referenced by parser->m_dtd is shared between the
       document's root parser and external PE parsers, therefore one does not
       need to call setContext. In addition, one also *must* not call
       setContext, because this would overwrite existing prefix->binding
       pointers in parser->m_dtd with ones that get destroyed with the external
       PE parser. This would leave those prefixes with dangling pointers.
    */
    parser->m_isParamEntity = XML_TRUE;
    XmlPrologStateInitExternalEntity(&_Mut parser->m_prologState);
    parser->m_processor = externalParEntInitProcessor;
  }
#endif /* XML_DTD */
  return parser;
}

_Safe static void FASTCALL
destroyBindings(BINDING *bindings, XML_Parser _Borrow parser) {
  for (;;) {
    BINDING *b = bindings;
    if (! b)
      break;
    _Unsafe bindings = b->nextTagBinding;
    _Unsafe FREE_ARRAY(parser, b->uri, XML_Char);
    _Unsafe FREE(parser, b);
  }
}

_Safe void XMLCALL
XML_ParserFree(XML_Parser _Owned _Nullable parser) {
  TAG *tagList;
  if (parser == nullptr)
    return;
  if (isCalledFromInsideHandler(&_Mut *parser)) {
    // Refusing to free from inside a handler keeps the parser alive on purpose.
    _Unsafe { (void)__move_to_raw(parser); }
    return;
  }
  /* free m_tagStack and m_freeTagList */
  tagList = parser->m_tagStack;
  for (;;) {
    TAG *p;
    if (tagList == nullptr) {
      if (parser->m_freeTagList == nullptr)
        break;
      tagList = parser->m_freeTagList;
      parser->m_freeTagList = nullptr;
    }
    p = tagList;
    _Unsafe tagList = tagList->parent;
    _Unsafe FREE(&_Mut *parser, p->buf.raw);
    _Unsafe destroyBindings(p->bindings, &_Mut *parser);
    _Unsafe FREE(&_Mut *parser, p);
  }
  /* free m_openInternalEntities */
  for (OPEN_INTERNAL_ENTITY *entityList = parser->m_openInternalEntities;
       entityList != nullptr;) {
    OPEN_INTERNAL_ENTITY *const openEntity = entityList;
    _Unsafe entityList = entityList->next;
    _Unsafe FREE(&_Mut *parser, openEntity);
  }
  /* free m_openAttributeEntities */
  for (OPEN_INTERNAL_ENTITY *entityList = parser->m_openAttributeEntities;
       entityList != nullptr;) {
    OPEN_INTERNAL_ENTITY *const openEntity = entityList;
    _Unsafe entityList = entityList->next;
    _Unsafe FREE(&_Mut *parser, openEntity);
  }
  /* free m_openValueEntities */
  for (OPEN_INTERNAL_ENTITY *entityList = parser->m_openValueEntities;
       entityList != nullptr;) {
    OPEN_INTERNAL_ENTITY *const openEntity = entityList;
    _Unsafe entityList = entityList->next;
    _Unsafe FREE(&_Mut *parser, openEntity);
  }
  /* free m_freeEntities */
  for (OPEN_INTERNAL_ENTITY *entityList = parser->m_freeEntities;
       entityList != nullptr;) {
    OPEN_INTERNAL_ENTITY *const openEntity = entityList;
    _Unsafe entityList = entityList->next;
    _Unsafe FREE(&_Mut *parser, openEntity);
  }
  parser->m_freeEntities = nullptr;
  destroyBindings(parser->m_freeBindingList, &_Mut *parser);
  destroyBindings(parser->m_inheritedBindings, &_Mut *parser);
  poolDestroy(&_Mut parser->m_tempPool);
  poolDestroy(&_Mut parser->m_temp2Pool);
  FREE_ARRAY(&_Mut *parser, parser->m_protocolEncodingName, XML_Char);
#ifdef XML_DTD
  /* external parameter entity parsers share the DTD structure
     parser->m_dtd with the root parser, so we must not destroy it
  */
  if (! parser->m_isParamEntity && parser->m_dtd)
#else
  if (parser->m_dtd)
#endif /* XML_DTD */
    dtdDestroy(parser->m_dtd, (XML_Bool)! parser->m_parentParser, &_Mut *parser);
  FREE_ARRAY(&_Mut *parser, parser->m_atts, ATTRIBUTE);
#ifdef XML_ATTR_INFO
  FREE_ARRAY(&_Mut *parser, parser->m_attInfo, XML_AttrInfo);
#endif
  FREE_ARRAY(&_Mut *parser, parser->m_groupConnector, char);
  // NOTE: We are avoiding FREE(..) here because parser->m_buffer
  //       is not being allocated with MALLOC(..) but with plain
  //       .malloc_fcn(..).
  _Unsafe parser->m_mem.free_fcn(parser->m_buffer);
  FREE_ARRAY(&_Mut *parser, parser->m_dataBuf, XML_Char);
  FREE_ARRAY(&_Mut *parser, parser->m_nsAtts, NS_ATT);
  FREE_ARRAY(&_Mut *parser, parser->m_unknownEncodingMem, char);
  if (parser->m_unknownEncodingRelease)
    callUnknownEncodingRelease(&_Mut *parser);
  _Unsafe {
    XML_Parser rawParser = __move_to_raw(parser);
    XML_Parser rawParserAlias = rawParser;
    FREE(&_Mut *rawParser, rawParserAlias);
  }
}

_Safe void XMLCALL
XML_UseParserAsHandlerArg(XML_Parser _Borrow _Nullable parser) {
  if (parser != nullptr)
    parser->m_handlerArg = _Unsafe((void *)&_Mut *parser);
}

_Safe enum XML_Error XMLCALL
XML_UseForeignDTD(XML_Parser _Borrow _Nullable parser, XML_Bool useDTD) {
  if (parser == nullptr)
    return XML_ERROR_INVALID_ARGUMENT;
#ifdef XML_DTD
  /* block after XML_Parse()/XML_ParseBuffer() has been called */
  if (parserBusy(parser))
    return XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING;
  parser->m_useForeignDTD = useDTD;
  return XML_ERROR_NONE;
#else
  UNUSED_P(useDTD);
  return XML_ERROR_FEATURE_REQUIRES_XML_DTD;
#endif
}

_Safe void XMLCALL
XML_SetReturnNSTriplet(XML_Parser _Borrow _Nullable parser, int do_nst) {
  if (parser == nullptr)
    return;
  /* block after XML_Parse()/XML_ParseBuffer() has been called */
  if (parserBusy(parser))
    return;
  parser->m_ns_triplets = do_nst ? XML_TRUE : XML_FALSE;
}

_Safe void XMLCALL
XML_SetUserData(XML_Parser _Borrow _Nullable parser, void *p) {
  if (parser == nullptr)
    return;
  if (parser->m_handlerArg == parser->m_userData)
    parser->m_handlerArg = parser->m_userData = p;
  else
    parser->m_userData = p;
}

_Safe enum XML_Status XMLCALL
XML_SetBase(XML_Parser _Borrow _Nullable parser, const XML_Char *p) {
  if (parser == nullptr)
    return XML_STATUS_ERROR;
  if (p) {
    p = poolCopyString(_Unsafe(&_Mut parser->m_dtd->pool), p);
    if (! p)
      return XML_STATUS_ERROR;
    parser->m_curBase = p;
  } else
    parser->m_curBase = nullptr;
  return XML_STATUS_OK;
}

_Safe const XML_Char *XMLCALL
XML_GetBase(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return nullptr;
  return parser->m_curBase;
}

_Safe int XMLCALL
XML_GetSpecifiedAttributeCount(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return -1;
  return parser->m_nSpecifiedAtts;
}

_Safe int XMLCALL
XML_GetIdAttributeIndex(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return -1;
  return parser->m_idAttIndex;
}

#ifdef XML_ATTR_INFO
_Safe const XML_AttrInfo *XMLCALL
XML_GetAttributeInfo(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return nullptr;
  return parser->m_attInfo;
}
#endif

_Safe void XMLCALL
XML_SetElementHandler(XML_Parser _Borrow _Nullable parser, XML_StartElementHandler start,
                      XML_EndElementHandler end) {
  if (parser == nullptr)
    return;
  parser->m_startElementHandler = start;
  parser->m_endElementHandler = end;
}

_Safe void XMLCALL
XML_SetStartElementHandler(XML_Parser _Borrow _Nullable parser, XML_StartElementHandler start) {
  if (parser != nullptr)
    parser->m_startElementHandler = start;
}

_Safe void XMLCALL
XML_SetEndElementHandler(XML_Parser _Borrow _Nullable parser, XML_EndElementHandler end) {
  if (parser != nullptr)
    parser->m_endElementHandler = end;
}

_Safe void XMLCALL
XML_SetCharacterDataHandler(XML_Parser _Borrow _Nullable parser,
                            XML_CharacterDataHandler handler) {
  if (parser != nullptr)
    parser->m_characterDataHandler = handler;
}

_Safe void XMLCALL
XML_SetProcessingInstructionHandler(XML_Parser _Borrow _Nullable parser,
                                    XML_ProcessingInstructionHandler handler) {
  if (parser != nullptr)
    parser->m_processingInstructionHandler = handler;
}

_Safe void XMLCALL
XML_SetCommentHandler(XML_Parser _Borrow _Nullable parser, XML_CommentHandler handler) {
  if (parser != nullptr)
    parser->m_commentHandler = handler;
}

_Safe void XMLCALL
XML_SetCdataSectionHandler(XML_Parser _Borrow _Nullable parser,
                           XML_StartCdataSectionHandler start,
                           XML_EndCdataSectionHandler end) {
  if (parser == nullptr)
    return;
  parser->m_startCdataSectionHandler = start;
  parser->m_endCdataSectionHandler = end;
}

_Safe void XMLCALL
XML_SetStartCdataSectionHandler(XML_Parser _Borrow _Nullable parser,
                                XML_StartCdataSectionHandler start) {
  if (parser != nullptr)
    parser->m_startCdataSectionHandler = start;
}

_Safe void XMLCALL
XML_SetEndCdataSectionHandler(XML_Parser _Borrow _Nullable parser,
                              XML_EndCdataSectionHandler end) {
  if (parser != nullptr)
    parser->m_endCdataSectionHandler = end;
}

_Safe void XMLCALL
XML_SetDefaultHandler(XML_Parser _Borrow _Nullable parser, XML_DefaultHandler handler) {
  if (parser == nullptr)
    return;
  parser->m_defaultHandler = handler;
  parser->m_defaultExpandInternalEntities = XML_FALSE;
}

_Safe void XMLCALL
XML_SetDefaultHandlerExpand(XML_Parser _Borrow _Nullable parser, XML_DefaultHandler handler) {
  if (parser == nullptr)
    return;
  parser->m_defaultHandler = handler;
  parser->m_defaultExpandInternalEntities = XML_TRUE;
}

_Safe void XMLCALL
XML_SetDoctypeDeclHandler(XML_Parser _Borrow _Nullable parser, XML_StartDoctypeDeclHandler start,
                          XML_EndDoctypeDeclHandler end) {
  if (parser == nullptr)
    return;
  parser->m_startDoctypeDeclHandler = start;
  parser->m_endDoctypeDeclHandler = end;
}

_Safe void XMLCALL
XML_SetStartDoctypeDeclHandler(XML_Parser _Borrow _Nullable parser,
                               XML_StartDoctypeDeclHandler start) {
  if (parser != nullptr)
    parser->m_startDoctypeDeclHandler = start;
}

_Safe void XMLCALL
XML_SetEndDoctypeDeclHandler(XML_Parser _Borrow _Nullable parser, XML_EndDoctypeDeclHandler end) {
  if (parser != nullptr)
    parser->m_endDoctypeDeclHandler = end;
}

_Safe void XMLCALL
XML_SetUnparsedEntityDeclHandler(XML_Parser _Borrow _Nullable parser,
                                 XML_UnparsedEntityDeclHandler handler) {
  if (parser != nullptr)
    parser->m_unparsedEntityDeclHandler = handler;
}

_Safe void XMLCALL
XML_SetNotationDeclHandler(XML_Parser _Borrow _Nullable parser, XML_NotationDeclHandler handler) {
  if (parser != nullptr)
    parser->m_notationDeclHandler = handler;
}

_Safe void XMLCALL
XML_SetNamespaceDeclHandler(XML_Parser _Borrow _Nullable parser,
                            XML_StartNamespaceDeclHandler start,
                            XML_EndNamespaceDeclHandler end) {
  if (parser == nullptr)
    return;
  parser->m_startNamespaceDeclHandler = start;
  parser->m_endNamespaceDeclHandler = end;
}

_Safe void XMLCALL
XML_SetStartNamespaceDeclHandler(XML_Parser _Borrow _Nullable parser,
                                 XML_StartNamespaceDeclHandler start) {
  if (parser != nullptr)
    parser->m_startNamespaceDeclHandler = start;
}

_Safe void XMLCALL
XML_SetEndNamespaceDeclHandler(XML_Parser _Borrow _Nullable parser,
                               XML_EndNamespaceDeclHandler end) {
  if (parser != nullptr)
    parser->m_endNamespaceDeclHandler = end;
}

_Safe void XMLCALL
XML_SetNotStandaloneHandler(XML_Parser _Borrow _Nullable parser,
                            XML_NotStandaloneHandler handler) {
  if (parser != nullptr)
    parser->m_notStandaloneHandler = handler;
}

_Safe void XMLCALL
XML_SetExternalEntityRefHandler(XML_Parser _Borrow _Nullable parser,
                                XML_ExternalEntityRefHandler handler) {
  if (parser != nullptr)
    parser->m_externalEntityRefHandler = handler;
}

_Safe void XMLCALL
XML_SetExternalEntityRefHandlerArg(XML_Parser _Borrow _Nullable parser, void *arg) {
  if (parser == nullptr)
    return;
  if (arg)
    _Unsafe parser->m_externalEntityRefHandlerArg = (XML_Parser)arg;
  else
    parser->m_externalEntityRefHandlerArg = RAW_PARSER(&_Mut *parser);
}

_Safe void XMLCALL
XML_SetSkippedEntityHandler(XML_Parser _Borrow _Nullable parser,
                            XML_SkippedEntityHandler handler) {
  if (parser != nullptr)
    parser->m_skippedEntityHandler = handler;
}

_Safe void XMLCALL
XML_SetUnknownEncodingHandler(XML_Parser _Borrow _Nullable parser,
                              XML_UnknownEncodingHandler handler, void *data) {
  if (parser == nullptr)
    return;
  parser->m_unknownEncodingHandler = handler;
  parser->m_unknownEncodingHandlerData = data;
}

_Safe void XMLCALL
XML_SetElementDeclHandler(XML_Parser _Borrow _Nullable parser, XML_ElementDeclHandler eldecl) {
  if (parser != nullptr)
    parser->m_elementDeclHandler = eldecl;
}

_Safe void XMLCALL
XML_SetAttlistDeclHandler(XML_Parser _Borrow _Nullable parser, XML_AttlistDeclHandler attdecl) {
  if (parser != nullptr)
    parser->m_attlistDeclHandler = attdecl;
}

_Safe void XMLCALL
XML_SetEntityDeclHandler(XML_Parser _Borrow _Nullable parser, XML_EntityDeclHandler handler) {
  if (parser != nullptr)
    parser->m_entityDeclHandler = handler;
}

_Safe void XMLCALL
XML_SetXmlDeclHandler(XML_Parser _Borrow _Nullable parser, XML_XmlDeclHandler handler) {
  if (parser != nullptr)
    parser->m_xmlDeclHandler = handler;
}

_Safe int XMLCALL
XML_SetParamEntityParsing(XML_Parser _Borrow _Nullable parser,
                          enum XML_ParamEntityParsing peParsing) {
  if (parser == nullptr)
    return 0;
  /* block after XML_Parse()/XML_ParseBuffer() has been called */
  if (parserBusy(parser))
    return 0;
#ifdef XML_DTD
  parser->m_paramEntityParsing = peParsing;
  return 1;
#else
  return peParsing == XML_PARAM_ENTITY_PARSING_NEVER;
#endif
}

// DEPRECATED since Expat 2.8.0.
_Safe int XMLCALL
XML_SetHashSalt(XML_Parser _Borrow _Nullable parser, unsigned long hash_salt) {
  if (parser == nullptr)
    return 0;

  XML_Parser _Borrow rootParser = getRootParserOf(parser, nullptr);
  _Unsafe assert(! rootParser->m_parentParser);

  /* block after XML_Parse()/XML_ParseBuffer() has been called */
  if (parserBusy(rootParser))
    return 0;

  rootParser->m_hash_secret_salt_128.k[0] = 0;
  rootParser->m_hash_secret_salt_128.k[1] = hash_salt;

  if (hash_salt != 0) { // to remain backwards compatible
    rootParser->m_hash_secret_salt_set = XML_TRUE;

    if (sizeof(unsigned long) == 4)
      ENTROPY_DEBUG("explicit(4)", rootParser->m_hash_secret_salt_128);
    else
      ENTROPY_DEBUG("explicit(8)", rootParser->m_hash_secret_salt_128);
  }

  return 1;
}

_Safe XML_Bool XMLCALL
XML_SetHashSalt16Bytes(XML_Parser _Borrow _Nullable parser, const uint8_t entropy[16]) {
  if (parser == nullptr)
    return XML_FALSE;

  if (entropy == nullptr)
    return XML_FALSE;

  XML_Parser _Borrow rootParser = getRootParserOf(parser, nullptr);
  _Unsafe assert(! rootParser->m_parentParser);

  /* block after XML_Parse()/XML_ParseBuffer() has been called */
  if (parserBusy(rootParser))
    return XML_FALSE;

  _Unsafe sip_tokey(_Unsafe(_Unsafe(&rootParser->m_hash_secret_salt_128)), entropy);

  rootParser->m_hash_secret_salt_set = XML_TRUE;

  ENTROPY_DEBUG("explicit(16)", rootParser->m_hash_secret_salt_128);

  return XML_TRUE;
}

_Safe enum XML_Status XMLCALL
XML_Parse(XML_Parser _Borrow _Nullable parser, const char *s, int len, int isFinal) {
  if ((parser == nullptr) || (len < 0) || ((s == nullptr) && (len != 0))) {
    if (parser != nullptr)
      parser->m_errorCode = XML_ERROR_INVALID_ARGUMENT;
    return XML_STATUS_ERROR;
  }
  if (isCalledFromInsideHandler(parser))
    return XML_STATUS_ERROR;
  switch (parser->m_parsingStatus.parsing) {
  case XML_SUSPENDED:
    parser->m_errorCode = XML_ERROR_SUSPENDED;
    return XML_STATUS_ERROR;
  case XML_FINISHED:
    parser->m_errorCode = XML_ERROR_FINISHED;
    return XML_STATUS_ERROR;
  case XML_INITIALIZED:
    if (parser->m_parentParser == nullptr && ! startParsing(parser)) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      return XML_STATUS_ERROR;
    }
    EXPAT_FALLTHROUGH;
  default:
    parser->m_parsingStatus.parsing = XML_PARSING;
  }

#if XML_CONTEXT_BYTES == 0
  if (parser->m_bufferPtr == parser->m_bufferEnd) {
    const char *end;
    int nLeftOver;
    enum XML_Status result;
    /* Detect overflow (a+b > MAX <==> b > MAX-a) */
    if ((uint64_t)len > UINT64_MAX - parser->m_parseEndByteIndex) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      parser->m_eventPtr = parser->m_eventEndPtr = nullptr;
      parser->m_processor = errorProcessor;
      return XML_STATUS_ERROR;
    }
    // though this isn't a buffer request, we assume that `len` is the app's
    // preferred buffer fill size, and therefore save it here.
    parser->m_lastBufferRequestSize = len;
    parser->m_parseEndByteIndex += len;
    parser->m_positionPtr = s;
    parser->m_parsingStatus.finalBuffer = (XML_Bool)isFinal;

    parser->m_errorCode
        = callProcessor(parser, s, parser->m_parseEndPtr = s + len, _Unsafe(&end));

    if (parser->m_errorCode != XML_ERROR_NONE) {
      parser->m_eventEndPtr = parser->m_eventPtr;
      parser->m_processor = errorProcessor;
      return XML_STATUS_ERROR;
    } else {
      switch (parser->m_parsingStatus.parsing) {
      case XML_SUSPENDED:
        result = XML_STATUS_SUSPENDED;
        break;
      case XML_INITIALIZED:
      case XML_PARSING:
        if (isFinal) {
          parser->m_parsingStatus.parsing = XML_FINISHED;
          return XML_STATUS_OK;
        }
        EXPAT_FALLTHROUGH;
      default:
        result = XML_STATUS_OK;
      }
    }

    XmlUpdatePosition(parser->m_encoding, parser->m_positionPtr, end,
                      _Unsafe(&parser->m_position));
    nLeftOver = s + len - end;
    if (nLeftOver) {
      // Back up and restore the parsing status to avoid XML_ERROR_SUSPENDED
      // (and XML_ERROR_FINISHED) from XML_GetBuffer.
      const enum XML_Parsing originalStatus = parser->m_parsingStatus.parsing;
      parser->m_parsingStatus.parsing = XML_PARSING;
      void *const temp = XML_GetBuffer(parser, nLeftOver);
      parser->m_parsingStatus.parsing = originalStatus;
      // GetBuffer may have overwritten this, but we want to remember what the
      // app requested, not how many bytes were left over after parsing.
      parser->m_lastBufferRequestSize = len;
      if (temp == nullptr) {
        // NOTE: parser->m_errorCode has already been set by XML_GetBuffer().
        parser->m_eventPtr = parser->m_eventEndPtr = nullptr;
        parser->m_processor = errorProcessor;
        return XML_STATUS_ERROR;
      }
      // Since we know that the buffer was empty and XML_CONTEXT_BYTES is 0, we
      // don't have any data to preserve, and can copy straight into the start
      // of the buffer rather than the GetBuffer return pointer (which may be
      // pointing further into the allocated buffer).
      _Unsafe memcpy(parser->m_buffer, end, nLeftOver);
    }
    parser->m_bufferPtr = parser->m_buffer;
    parser->m_bufferEnd = parser->m_buffer + nLeftOver;
    parser->m_positionPtr = parser->m_bufferPtr;
    parser->m_parseEndPtr = parser->m_bufferEnd;
    parser->m_eventPtr = parser->m_bufferPtr;
    parser->m_eventEndPtr = parser->m_bufferPtr;
    return result;
  }
#endif /* XML_CONTEXT_BYTES == 0 */
  void *buff = XML_GetBuffer(parser, len);
  if (buff == nullptr)
    return XML_STATUS_ERROR;
  if (len > 0) {
    _Unsafe assert(s != nullptr); // make sure s==nullptr && len!=0 was rejected above
    _Unsafe memcpy(buff, s, len);
  }
  return XML_ParseBuffer(parser, len, isFinal);
}

_Safe enum XML_Status XMLCALL
XML_ParseBuffer(XML_Parser _Borrow _Nullable parser, int len, int isFinal) {
  const char *start;
  enum XML_Status result = XML_STATUS_OK;

  if ((parser == nullptr) || isCalledFromInsideHandler(parser))
    return XML_STATUS_ERROR;

  if (len < 0) {
    parser->m_errorCode = XML_ERROR_INVALID_ARGUMENT;
    return XML_STATUS_ERROR;
  }

  switch (parser->m_parsingStatus.parsing) {
  case XML_SUSPENDED:
    parser->m_errorCode = XML_ERROR_SUSPENDED;
    return XML_STATUS_ERROR;
  case XML_FINISHED:
    parser->m_errorCode = XML_ERROR_FINISHED;
    return XML_STATUS_ERROR;
  case XML_INITIALIZED:
    /* Has someone called XML_GetBuffer successfully before? */
    if (! parser->m_bufferPtr) {
      parser->m_errorCode = XML_ERROR_NO_BUFFER;
      return XML_STATUS_ERROR;
    }

    if (parser->m_parentParser == nullptr && ! startParsing(parser)) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      return XML_STATUS_ERROR;
    }
    EXPAT_FALLTHROUGH;
  default:
    parser->m_parsingStatus.parsing = XML_PARSING;
  }

  // Detect and avoid integer overflow
  if ((uint64_t)len > UINT64_MAX - parser->m_parseEndByteIndex) {
    parser->m_errorCode = XML_ERROR_NO_MEMORY;
    parser->m_eventPtr = parser->m_eventEndPtr = nullptr;
    parser->m_processor = errorProcessor;
    return XML_STATUS_ERROR;
  }

  start = parser->m_bufferPtr;
  parser->m_positionPtr = start;
  parser->m_bufferEnd += len;
  parser->m_parseEndPtr = parser->m_bufferEnd;
  parser->m_parseEndByteIndex += len;
  parser->m_parsingStatus.finalBuffer = (XML_Bool)isFinal;

  __auto_type bsc_h3 = parser->m_parseEndPtr;
  __auto_type bsc_h4 = _Unsafe(&parser->m_bufferPtr);
  parser->m_errorCode = callProcessor(parser, start, bsc_h3,
                                      bsc_h4);

  if (parser->m_errorCode != XML_ERROR_NONE) {
    parser->m_eventEndPtr = parser->m_eventPtr;
    parser->m_processor = errorProcessor;
    return XML_STATUS_ERROR;
  } else {
    switch (parser->m_parsingStatus.parsing) {
    case XML_SUSPENDED:
      result = XML_STATUS_SUSPENDED;
      break;
    case XML_INITIALIZED:
    case XML_PARSING:
      if (isFinal) {
        parser->m_parsingStatus.parsing = XML_FINISHED;
        return result;
      }
      break;
    default:; /* should not happen */
    }
  }

  XmlUpdatePosition(parser->m_encoding, parser->m_positionPtr,
                    parser->m_bufferPtr, _Unsafe(&parser->m_position));
  parser->m_positionPtr = parser->m_bufferPtr;
  return result;
}

/* Modifies `parser`’s buffer to be backed by `newBuf`. */
_Safe static void
setParserBuffer(XML_Parser _Borrow parser, char *newBuf, int newBufSize, int keep) {
  parser->m_bufferLim = newBuf + newBufSize;
  if (parser->m_bufferPtr) {
    const int parsing
        = (int)EXPAT_SAFE_PTR_DIFF(parser->m_bufferEnd, parser->m_bufferPtr);
    _Unsafe memcpy(newBuf, parser->m_bufferPtr - keep, parsing + keep);
    // NOTE: We are avoiding FREE(..) here because parser->m_buffer
    //       is not being allocated with MALLOC(..) but with plain
    //       .malloc_fcn(..).
    _Unsafe parser->m_mem.free_fcn(parser->m_buffer);
    parser->m_buffer = newBuf;
    parser->m_bufferEnd = newBuf + parsing + keep;
    parser->m_bufferPtr = newBuf + keep;
  } else {
    /* This must be a brand new buffer with no data in it yet */
    parser->m_buffer = newBuf;
    parser->m_bufferEnd = newBuf;
    parser->m_bufferPtr = newBuf;
  }
}

_Safe void *XMLCALL
XML_GetBuffer(XML_Parser _Borrow _Nullable parser, int len) {
  if ((parser == nullptr) || isCalledFromInsideHandler(parser))
    return nullptr;
  if (len < 0) {
    parser->m_errorCode = XML_ERROR_NO_MEMORY;
    return nullptr;
  }
  switch (parser->m_parsingStatus.parsing) {
  case XML_SUSPENDED:
    parser->m_errorCode = XML_ERROR_SUSPENDED;
    return nullptr;
  case XML_FINISHED:
    parser->m_errorCode = XML_ERROR_FINISHED;
    return nullptr;
  default:;
  }

  // whether or not the request succeeds, `len` seems to be the app's preferred
  // buffer fill size; remember it.
  parser->m_lastBufferRequestSize = len;
  if (len > EXPAT_SAFE_PTR_DIFF(parser->m_bufferLim, parser->m_bufferEnd)
      || parser->m_buffer == nullptr) {
    /* Do not invoke signed arithmetic overflow: */
    int neededSize = (int)((unsigned)len
                           + (unsigned)EXPAT_SAFE_PTR_DIFF(
                               parser->m_bufferEnd, parser->m_bufferPtr));
    if (neededSize < 0) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      return nullptr;
    }
#if XML_CONTEXT_BYTES > 0
    const int parsed
        = (int)EXPAT_SAFE_PTR_DIFF(parser->m_bufferPtr, parser->m_buffer);
    int keep = parsed;
    if (keep > XML_CONTEXT_BYTES)
      keep = XML_CONTEXT_BYTES;
    /* Detect and prevent integer overflow */
    if (keep > INT_MAX - neededSize) {
      parser->m_errorCode = XML_ERROR_NO_MEMORY;
      return nullptr;
    }
#else
    int keep = 0;
#endif /* XML_CONTEXT_BYTES > 0 */
    neededSize += keep;
    if (parser->m_buffer && parser->m_bufferPtr
        && neededSize
               <= EXPAT_SAFE_PTR_DIFF(parser->m_bufferLim, parser->m_buffer)) {
#if XML_CONTEXT_BYTES > 0
      if (keep < parsed) {
        int offset = parsed - keep;
        /* The buffer pointers cannot be nullptr here; we have at least some bytes
         * in the buffer */
        _Unsafe memmove(parser->m_buffer, _Unsafe(&parser->m_buffer[offset]),
                parser->m_bufferEnd - parser->m_bufferPtr + keep);
        parser->m_bufferEnd -= offset;
        parser->m_bufferPtr -= offset;
      }
#else
      _Unsafe memmove(parser->m_buffer, parser->m_bufferPtr,
              EXPAT_SAFE_PTR_DIFF(parser->m_bufferEnd, parser->m_bufferPtr));
      parser->m_bufferEnd
          = parser->m_buffer
            + EXPAT_SAFE_PTR_DIFF(parser->m_bufferEnd, parser->m_bufferPtr);
      parser->m_bufferPtr = parser->m_buffer;
#endif /* XML_CONTEXT_BYTES > 0 */
    } else {
      int bufferSize
          = (int)EXPAT_SAFE_PTR_DIFF(parser->m_bufferLim, parser->m_buffer);
      if (bufferSize == 0)
        bufferSize = INIT_BUFFER_SIZE;
      do {
        /* Do not invoke signed arithmetic overflow: */
        bufferSize = (int)(2U * (unsigned)bufferSize);
      } while (bufferSize < neededSize && bufferSize > 0);
      if (bufferSize <= 0) {
        parser->m_errorCode = XML_ERROR_NO_MEMORY;
        return nullptr;
      }
      // NOTE: We are avoiding MALLOC(..) here to leave limiting
      //       the input size to the application using Expat.
      char *const newBuf = _Unsafe((char *)(parser->m_mem.malloc_fcn(bufferSize)));
      if (newBuf == nullptr) {
        parser->m_errorCode = XML_ERROR_NO_MEMORY;
        return nullptr;
      }
      setParserBuffer(parser, newBuf, bufferSize, keep);
    }
    parser->m_eventPtr = parser->m_eventEndPtr = nullptr;
    parser->m_positionPtr = nullptr;
  }
  return parser->m_bufferEnd;
}

_Safe static void
triggerReenter(XML_Parser _Borrow parser) {
  parser->m_reenter = XML_TRUE;
}

_Safe enum XML_Status XMLCALL
XML_StopParser(XML_Parser _Borrow _Nullable parser, XML_Bool resumable) {
  if (parser == nullptr)
    return XML_STATUS_ERROR;
  switch (parser->m_parsingStatus.parsing) {
  case XML_INITIALIZED:
    parser->m_errorCode = XML_ERROR_NOT_STARTED;
    return XML_STATUS_ERROR;
  case XML_SUSPENDED:
    if (resumable) {
      parser->m_errorCode = XML_ERROR_SUSPENDED;
      return XML_STATUS_ERROR;
    }
    parser->m_parsingStatus.parsing = XML_FINISHED;
    break;
  case XML_FINISHED:
    parser->m_errorCode = XML_ERROR_FINISHED;
    return XML_STATUS_ERROR;
  case XML_PARSING:
    if (resumable) {
#ifdef XML_DTD
      if (parser->m_isParamEntity) {
        parser->m_errorCode = XML_ERROR_SUSPEND_PE;
        return XML_STATUS_ERROR;
      }
#endif
      parser->m_parsingStatus.parsing = XML_SUSPENDED;
    } else
      parser->m_parsingStatus.parsing = XML_FINISHED;
    break;
  default:
    _Unsafe assert(0);
  }
  return XML_STATUS_OK;
}

_Safe enum XML_Status XMLCALL
XML_ResumeParser(XML_Parser _Borrow _Nullable parser) {
  enum XML_Status result = XML_STATUS_OK;

  if ((parser == nullptr) || isCalledFromInsideHandler(parser))
    return XML_STATUS_ERROR;
  if (parser->m_parsingStatus.parsing != XML_SUSPENDED) {
    parser->m_errorCode = XML_ERROR_NOT_SUSPENDED;
    return XML_STATUS_ERROR;
  }
  parser->m_parsingStatus.parsing = XML_PARSING;

  __auto_type bsc_h5 = parser->m_bufferPtr;
  __auto_type bsc_h6 = parser->m_parseEndPtr;
  __auto_type bsc_h7 = _Unsafe(&parser->m_bufferPtr);
  parser->m_errorCode = callProcessor(
      parser, bsc_h5, bsc_h6, bsc_h7);

  if (parser->m_errorCode != XML_ERROR_NONE) {
    parser->m_eventEndPtr = parser->m_eventPtr;
    parser->m_processor = errorProcessor;
    return XML_STATUS_ERROR;
  } else {
    switch (parser->m_parsingStatus.parsing) {
    case XML_SUSPENDED:
      result = XML_STATUS_SUSPENDED;
      break;
    case XML_INITIALIZED:
    case XML_PARSING:
      if (parser->m_parsingStatus.finalBuffer) {
        parser->m_parsingStatus.parsing = XML_FINISHED;
        return result;
      }
      break;
    default:;
    }
  }

  XmlUpdatePosition(parser->m_encoding, parser->m_positionPtr,
                    parser->m_bufferPtr, _Unsafe(&parser->m_position));
  parser->m_positionPtr = parser->m_bufferPtr;
  return result;
}

_Safe void XMLCALL
XML_GetParsingStatus(XML_Parser _Borrow _Nullable parser, XML_ParsingStatus *status) {
  if (parser == nullptr)
    return;
  _Unsafe assert(status != nullptr);
  _Unsafe *status = parser->m_parsingStatus;
}

_Safe enum XML_Error XMLCALL
XML_GetErrorCode(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return XML_ERROR_INVALID_ARGUMENT;
  return parser->m_errorCode;
}

_Safe XML_Index XMLCALL
XML_GetCurrentByteIndex(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return -1;
  if (parser->m_eventPtr) {
    // NOTE: XML_Index is known to wrap around for >2 GiB content
    //       on 32bit machines and 64bit Windows, unless (non-default and
    //       uncommon) XML_LARGE_SIZE is defined.
    //       That's a bug and it only lives on because we cannot break
    //       ABI compatibility of public API.
    return (XML_Index)(parser->m_parseEndByteIndex
                       - (parser->m_parseEndPtr - parser->m_eventPtr));
  }
  return -1;
}

_Safe int XMLCALL
XML_GetCurrentByteCount(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return 0;
  if (parser->m_eventEndPtr && parser->m_eventPtr)
    return (int)(parser->m_eventEndPtr - parser->m_eventPtr);
  return 0;
}

_Safe const char *XMLCALL
XML_GetInputContext(XML_Parser _Borrow _Nullable parser, int *offset, int *size) {
#if XML_CONTEXT_BYTES > 0
  if (parser == nullptr)
    return nullptr;
  if (parser->m_eventPtr && parser->m_buffer) {
    if (offset != nullptr)
      _Unsafe *offset = (int)(parser->m_eventPtr - parser->m_buffer);
    if (size != nullptr)
      _Unsafe *size = (int)(parser->m_bufferEnd - parser->m_buffer);
    return parser->m_buffer;
  }
#else
  (void)parser;
  (void)offset;
  (void)size;
#endif /* XML_CONTEXT_BYTES > 0 */
  return nullptr;
}

_Safe XML_Size XMLCALL
XML_GetCurrentLineNumber(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return 0;
  if (parser->m_eventPtr && parser->m_eventPtr >= parser->m_positionPtr) {
    XmlUpdatePosition(parser->m_encoding, parser->m_positionPtr,
                      parser->m_eventPtr, _Unsafe(&parser->m_position));
    parser->m_positionPtr = parser->m_eventPtr;
  }
  // NOTE: XML_Size is known to wrap around for >4 GiB content
  //       on 32bit machines and 64bit Windows, unless (non-default and
  //       uncommon) XML_LARGE_SIZE is defined.
  //       That's a bug and it only lives on because we cannot break
  //       ABI compatibility of public API.
  return (XML_Size)(parser->m_position.lineNumber + 1);
}

_Safe XML_Size XMLCALL
XML_GetCurrentColumnNumber(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return 0;
  if (parser->m_eventPtr && parser->m_eventPtr >= parser->m_positionPtr) {
    XmlUpdatePosition(parser->m_encoding, parser->m_positionPtr,
                      parser->m_eventPtr, _Unsafe(&parser->m_position));
    parser->m_positionPtr = parser->m_eventPtr;
  }
  // NOTE: XML_Size is known to wrap around for >4 GiB content
  //       on 32bit machines and 64bit Windows, unless (non-default and
  //       uncommon) XML_LARGE_SIZE is defined.
  //       That's a bug and it only lives on because we cannot break
  //       ABI compatibility of public API.
  return (XML_Size)parser->m_position.columnNumber;
}

_Safe void XMLCALL
XML_FreeContentModel(XML_Parser _Borrow _Nullable parser, XML_Content *model) {
  if (parser == nullptr)
    return;

  // NOTE: We are avoiding FREE(..) here because the content model
  //       has been created using plain .malloc_fcn(..) rather than MALLOC(..).
  _Unsafe parser->m_mem.free_fcn(model);
}

_Safe void *XMLCALL
XML_MemMalloc(XML_Parser _Borrow _Nullable parser, size_t size) {
  if (parser == nullptr)
    return nullptr;

  // NOTE: We are avoiding MALLOC(..) here to not include
  //       user allocations with allocation tracking and limiting.
  return _Unsafe((parser->m_mem.malloc_fcn(size)));
}

_Safe void *XMLCALL
XML_MemRealloc(XML_Parser _Borrow _Nullable parser, void *ptr, size_t size) {
  if (parser == nullptr)
    return nullptr;

  // NOTE: We are avoiding REALLOC(..) here to not include
  //       user allocations with allocation tracking and limiting.
  return _Unsafe((parser->m_mem.realloc_fcn(ptr, size)));
}

_Safe void XMLCALL
XML_MemFree(XML_Parser _Borrow _Nullable parser, void *ptr) {
  if (parser == nullptr)
    return;

  // NOTE: We are avoiding FREE(..) here because XML_MemMalloc and
  //       XML_MemRealloc are not using MALLOC(..) and REALLOC(..)
  //       but plain .malloc_fcn(..) and .realloc_fcn(..), internally.
  _Unsafe parser->m_mem.free_fcn(ptr);
}

_Safe void XMLCALL
XML_DefaultCurrent(XML_Parser _Borrow _Nullable parser) {
  if (parser == nullptr)
    return;
  if (parser->m_defaultHandler) {
    if (parser->m_openInternalEntities)
      { __auto_type bsc_h8 = parser->m_internalEncoding;
      __auto_type bsc_h9 = _Unsafe((parser->m_openInternalEntities->internalEventPtr));
      __auto_type bsc_h10 = _Unsafe((parser->m_openInternalEntities->internalEventEndPtr));
      _Unsafe reportDefault(parser, bsc_h8,
                    bsc_h9,
                    bsc_h10); }
    else
      { __auto_type bsc_h11 = parser->m_encoding;
      __auto_type bsc_h12 = parser->m_eventPtr;
      __auto_type bsc_h13 = parser->m_eventEndPtr;
      reportDefault(parser, bsc_h11, bsc_h12,
                    bsc_h13); }
  }
}

_Safe const XML_LChar *XMLCALL
XML_ErrorString(enum XML_Error code) {
  switch (code) {
  case XML_ERROR_NONE:
    return nullptr;
  case XML_ERROR_NO_MEMORY:
    return XML_L("out of memory");
  case XML_ERROR_SYNTAX:
    return XML_L("syntax error");
  case XML_ERROR_NO_ELEMENTS:
    return XML_L("no element found");
  case XML_ERROR_INVALID_TOKEN:
    return XML_L("not well-formed (invalid token)");
  case XML_ERROR_UNCLOSED_TOKEN:
    return XML_L("unclosed token");
  case XML_ERROR_PARTIAL_CHAR:
    return XML_L("partial character");
  case XML_ERROR_TAG_MISMATCH:
    return XML_L("mismatched tag");
  case XML_ERROR_DUPLICATE_ATTRIBUTE:
    return XML_L("duplicate attribute");
  case XML_ERROR_JUNK_AFTER_DOC_ELEMENT:
    return XML_L("junk after document element");
  case XML_ERROR_PARAM_ENTITY_REF:
    return XML_L("illegal parameter entity reference");
  case XML_ERROR_UNDEFINED_ENTITY:
    return XML_L("undefined entity");
  case XML_ERROR_RECURSIVE_ENTITY_REF:
    return XML_L("recursive entity reference");
  case XML_ERROR_ASYNC_ENTITY:
    return XML_L("asynchronous entity");
  case XML_ERROR_BAD_CHAR_REF:
    return XML_L("reference to invalid character number");
  case XML_ERROR_BINARY_ENTITY_REF:
    return XML_L("reference to binary entity");
  case XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF:
    return XML_L("reference to external entity in attribute");
  case XML_ERROR_MISPLACED_XML_PI:
    return XML_L("XML or text declaration not at start of entity");
  case XML_ERROR_UNKNOWN_ENCODING:
    return XML_L("unknown encoding");
  case XML_ERROR_INCORRECT_ENCODING:
    return XML_L("encoding specified in XML declaration is incorrect");
  case XML_ERROR_UNCLOSED_CDATA_SECTION:
    return XML_L("unclosed CDATA section");
  case XML_ERROR_EXTERNAL_ENTITY_HANDLING:
    return XML_L("error in processing external entity reference");
  case XML_ERROR_NOT_STANDALONE:
    return XML_L("document is not standalone");
  case XML_ERROR_UNEXPECTED_STATE:
    return XML_L("unexpected parser state - please send a bug report");
  case XML_ERROR_ENTITY_DECLARED_IN_PE:
    return XML_L("entity declared in parameter entity");
  case XML_ERROR_FEATURE_REQUIRES_XML_DTD:
    return XML_L("requested feature requires XML_DTD support in Expat");
  case XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING:
    return XML_L("cannot change setting once parsing has begun");
  /* Added in 1.95.7. */
  case XML_ERROR_UNBOUND_PREFIX:
    return XML_L("unbound prefix");
  /* Added in 1.95.8. */
  case XML_ERROR_UNDECLARING_PREFIX:
    return XML_L("must not undeclare prefix");
  case XML_ERROR_INCOMPLETE_PE:
    return XML_L("incomplete markup in parameter entity");
  case XML_ERROR_XML_DECL:
    return XML_L("XML declaration not well-formed");
  case XML_ERROR_TEXT_DECL:
    return XML_L("text declaration not well-formed");
  case XML_ERROR_PUBLICID:
    return XML_L("illegal character(s) in public id");
  case XML_ERROR_SUSPENDED:
    return XML_L("parser suspended");
  case XML_ERROR_NOT_SUSPENDED:
    return XML_L("parser not suspended");
  case XML_ERROR_ABORTED:
    return XML_L("parsing aborted");
  case XML_ERROR_FINISHED:
    return XML_L("parsing finished");
  case XML_ERROR_SUSPEND_PE:
    return XML_L("cannot suspend in external parameter entity");
  /* Added in 2.0.0. */
  case XML_ERROR_RESERVED_PREFIX_XML:
    return XML_L(
        "reserved prefix (xml) must not be undeclared or bound to another namespace name");
  case XML_ERROR_RESERVED_PREFIX_XMLNS:
    return XML_L("reserved prefix (xmlns) must not be declared or undeclared");
  case XML_ERROR_RESERVED_NAMESPACE_URI:
    return XML_L(
        "prefix must not be bound to one of the reserved namespace names");
  /* Added in 2.2.5. */
  case XML_ERROR_INVALID_ARGUMENT: /* Constant added in 2.2.1, already */
    return XML_L("invalid argument");
    /* Added in 2.3.0. */
  case XML_ERROR_NO_BUFFER:
    return XML_L(
        "a successful prior call to function XML_GetBuffer is required");
  /* Added in 2.4.0. */
  case XML_ERROR_AMPLIFICATION_LIMIT_BREACH:
    return XML_L(
        "limit on input amplification factor (from DTD and entities) breached");
  /* Added in 2.6.4. */
  case XML_ERROR_NOT_STARTED:
    return XML_L("parser not started");
  }
  return nullptr;
}

_Safe const XML_LChar *XMLCALL
XML_ExpatVersion(void) {
  /* V1 is used to string-ize the version number. However, it would
     string-ize the actual version macro *names* unless we get them
     substituted before being passed to V1. CPP is defined to expand
     a macro, then rescan for more expansions. Thus, we use V2 to expand
     the version macros, then CPP will expand the resulting V1() macro
     with the correct numerals. */
  /* ### I'm assuming cpp is portable in this respect... */

#define V1(a, b, c) XML_L(#a) XML_L(".") XML_L(#b) XML_L(".") XML_L(#c)
#define V2(a, b, c) XML_L("expat_") V1(a, b, c)

  return V2(XML_MAJOR_VERSION, XML_MINOR_VERSION, XML_MICRO_VERSION);

#undef V1
#undef V2
}

_Safe XML_Expat_Version XMLCALL
XML_ExpatVersionInfo(void) {
  XML_Expat_Version version;

  version.major = XML_MAJOR_VERSION;
  version.minor = XML_MINOR_VERSION;
  version.micro = XML_MICRO_VERSION;

  return version;
}

const XML_Feature *XMLCALL
XML_GetFeatureList(void) {
  static const XML_Feature features[] = {
      {XML_FEATURE_SIZEOF_XML_CHAR, XML_L("sizeof(XML_Char)"),
       sizeof(XML_Char)},
      {XML_FEATURE_SIZEOF_XML_LCHAR, XML_L("sizeof(XML_LChar)"),
       sizeof(XML_LChar)},
#ifdef XML_UNICODE
      {XML_FEATURE_UNICODE, XML_L("XML_UNICODE"), 0},
#endif
#ifdef XML_UNICODE_WCHAR_T
      {XML_FEATURE_UNICODE_WCHAR_T, XML_L("XML_UNICODE_WCHAR_T"), 0},
#endif
#ifdef XML_DTD
      {XML_FEATURE_DTD, XML_L("XML_DTD"), 0},
#endif
#if XML_CONTEXT_BYTES > 0
      {XML_FEATURE_CONTEXT_BYTES, XML_L("XML_CONTEXT_BYTES"),
       XML_CONTEXT_BYTES},
#endif
#ifdef XML_MIN_SIZE
      {XML_FEATURE_MIN_SIZE, XML_L("XML_MIN_SIZE"), 0},
#endif
#ifdef XML_NS
      {XML_FEATURE_NS, XML_L("XML_NS"), 0},
#endif
#ifdef XML_LARGE_SIZE
      {XML_FEATURE_LARGE_SIZE, XML_L("XML_LARGE_SIZE"), 0},
#endif
#ifdef XML_ATTR_INFO
      {XML_FEATURE_ATTR_INFO, XML_L("XML_ATTR_INFO"), 0},
#endif
#if XML_GE == 1
      /* Added in Expat 2.4.0 for XML_DTD defined and
       * added in Expat 2.6.0 for XML_GE == 1. */
      {XML_FEATURE_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT,
       XML_L("XML_BLAP_MAX_AMP"),
       (long int)
           EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT},
      {XML_FEATURE_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT,
       XML_L("XML_BLAP_ACT_THRES"),
       EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT},
      /* Added in Expat 2.6.0. */
      {XML_FEATURE_GE, XML_L("XML_GE"), 0},
      /* Added in Expat 2.7.2. */
      {XML_FEATURE_ALLOC_TRACKER_MAXIMUM_AMPLIFICATION_DEFAULT,
       XML_L("XML_AT_MAX_AMP"),
       (long int)EXPAT_ALLOC_TRACKER_MAXIMUM_AMPLIFICATION_DEFAULT},
      {XML_FEATURE_ALLOC_TRACKER_ACTIVATION_THRESHOLD_DEFAULT,
       XML_L("XML_AT_ACT_THRES"),
       (long int)EXPAT_ALLOC_TRACKER_ACTIVATION_THRESHOLD_DEFAULT},
#endif
      {XML_FEATURE_END, nullptr, 0}};

  return features;
}

#if XML_GE == 1
_Safe XML_Bool XMLCALL
XML_SetBillionLaughsAttackProtectionMaximumAmplification(
    XML_Parser _Borrow parser, float maximumAmplificationFactor) {
  if ((parser == nullptr) || (parser->m_parentParser != nullptr)
      || isnan(maximumAmplificationFactor)
      || (maximumAmplificationFactor < 1.0f)) {
    return XML_FALSE;
  }
  parser->m_accounting.maximumAmplificationFactor = maximumAmplificationFactor;
  return XML_TRUE;
}

_Safe XML_Bool XMLCALL
XML_SetBillionLaughsAttackProtectionActivationThreshold(
    XML_Parser _Borrow parser, unsigned long long activationThresholdBytes) {
  if ((parser == nullptr) || (parser->m_parentParser != nullptr)) {
    return XML_FALSE;
  }
  parser->m_accounting.activationThresholdBytes = activationThresholdBytes;
  return XML_TRUE;
}

_Safe XML_Bool XMLCALL
XML_SetAllocTrackerMaximumAmplification(XML_Parser _Borrow _Nullable parser,
                                        float maximumAmplificationFactor) {
  if ((parser == nullptr) || (parser->m_parentParser != nullptr)
      || isnan(maximumAmplificationFactor)
      || (maximumAmplificationFactor < 1.0f)) {
    return XML_FALSE;
  }
  parser->m_alloc_tracker.maximumAmplificationFactor
      = maximumAmplificationFactor;
  return XML_TRUE;
}

_Safe XML_Bool XMLCALL
XML_SetAllocTrackerActivationThreshold(
    XML_Parser _Borrow parser, unsigned long long activationThresholdBytes) {
  if ((parser == nullptr) || (parser->m_parentParser != nullptr)) {
    return XML_FALSE;
  }
  parser->m_alloc_tracker.activationThresholdBytes = activationThresholdBytes;
  return XML_TRUE;
}
#endif /* XML_GE == 1 */

_Safe XML_Bool XMLCALL
XML_SetReparseDeferralEnabled(XML_Parser _Borrow _Nullable parser, XML_Bool enabled) {
  if (parser != nullptr && (enabled == XML_TRUE || enabled == XML_FALSE)) {
    parser->m_reparseDeferralEnabled = enabled;
    return XML_TRUE;
  }
  return XML_FALSE;
}

/* Initially tag->rawName always points into the parse buffer;
   for those TAG instances opened while the current parse buffer was
   processed, and not yet closed, we need to store tag->rawName in a more
   permanent location, since the parse buffer is about to be discarded.
*/
_Safe static XML_Bool
storeRawNames(XML_Parser _Borrow parser) {
  TAG *tag = parser->m_tagStack;
  while (tag) {
    size_t bufSize;
    size_t nameLen = _Unsafe((sizeof(XML_Char) * (tag->name.strLen + 1)));
    size_t rawNameLen;
    char *rawNameBuf = _Unsafe((tag->buf.raw + nameLen));
    /* Stop if already stored.  Since m_tagStack is a stack, we can stop
       at the first entry that has already been copied; everything
       below it in the stack is already been accounted for in a
       previous call to this function.
    */
    if (_Unsafe((tag->rawName == rawNameBuf)))
      break;
    /* For reuse purposes we need to ensure that the
       size of tag->buf is a multiple of sizeof(XML_Char).
    */
    _Unsafe rawNameLen = ROUND_UP(tag->rawNameLength, sizeof(XML_Char));
    /* Detect and prevent integer overflow. */
    if (rawNameLen > SIZE_MAX - nameLen)
      return XML_FALSE;
    bufSize = nameLen + rawNameLen;
    if (_Unsafe((bufSize > (size_t)(tag->bufEnd - tag->buf.raw)))) {
      char *temp = _Unsafe((char *)(REALLOC(parser, tag->buf.raw, bufSize)));
      if (temp == nullptr)
        return XML_FALSE;
      /* if tag->name.str points to tag->buf.str (only when namespace
         processing is off) then we have to update it
      */
      if (_Unsafe((tag->name.str == tag->buf.str)))
        _Unsafe tag->name.str = (XML_Char *)temp;
      /* if tag->name.localPart is set (when namespace processing is on)
         then update it as well, since it will always point into tag->buf
      */
      if (_Unsafe((tag->name.localPart)))
        _Unsafe tag->name.localPart
            = (XML_Char *)temp + (tag->name.localPart - tag->buf.str);
      _Unsafe tag->buf.raw = temp;
      _Unsafe tag->bufEnd = temp + bufSize;
      rawNameBuf = temp + nameLen;
    }
    _Unsafe memcpy(rawNameBuf, tag->rawName, tag->rawNameLength);
    _Unsafe tag->rawName = rawNameBuf;
    _Unsafe tag = tag->parent;
  }
  return XML_TRUE;
}

_Safe static enum XML_Error PTRCALL
contentProcessor(XML_Parser _Borrow parser, const char *start, const char *end,
                 const char **_Nonnull endPtr) {
  __auto_type bsc_h14 = parser->m_encoding;
  __auto_type bsc_h15 = parser->m_parsingStatus.finalBuffer;
  enum XML_Error result = doContent(
      parser, parser->m_parentParser ? 1 : 0, bsc_h14, start, end,
      endPtr, (XML_Bool)! bsc_h15,
      XML_ACCOUNT_DIRECT);
  if (result == XML_ERROR_NONE) {
    if (! storeRawNames(parser))
      return XML_ERROR_NO_MEMORY;
  }
  return result;
}

_Safe static enum XML_Error PTRCALL
externalEntityInitProcessor(XML_Parser _Borrow parser, const char *start,
                            const char *end, const char **_Nonnull endPtr) {
  enum XML_Error result = initializeEncoding(parser);
  if (result != XML_ERROR_NONE)
    return result;
  parser->m_processor = externalEntityInitProcessor2;
  return externalEntityInitProcessor2(parser, start, end, endPtr);
}

_Safe static enum XML_Error PTRCALL
externalEntityInitProcessor2(XML_Parser _Borrow parser, const char *start,
                             const char *end, const char **_Nonnull endPtr) {
  const char *next = start; /* XmlContentTok doesn't always set the last arg */
  int tok = XmlContentTok(parser->m_encoding, start, end, _Unsafe(&next));
  switch (tok) {
  case XML_TOK_BOM:
#if XML_GE == 1
    if (! accountingDiffTolerated(parser, tok, start, next, __LINE__,
                                  XML_ACCOUNT_DIRECT)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif /* XML_GE == 1 */

    /* If we are at the end of the buffer, this would cause the next stage,
       i.e. externalEntityInitProcessor3, to pass control directly to
       doContent (by detecting XML_TOK_NONE) without processing any xml text
       declaration - causing the error XML_ERROR_MISPLACED_XML_PI in doContent.
    */
    if (next == end && ! parser->m_parsingStatus.finalBuffer) {
      _Unsafe *endPtr = next;
      return XML_ERROR_NONE;
    }
    start = next;
    break;
  case XML_TOK_PARTIAL:
    if (! parser->m_parsingStatus.finalBuffer) {
      _Unsafe *endPtr = start;
      return XML_ERROR_NONE;
    }
    parser->m_eventPtr = start;
    return XML_ERROR_UNCLOSED_TOKEN;
  case XML_TOK_PARTIAL_CHAR:
    if (! parser->m_parsingStatus.finalBuffer) {
      _Unsafe *endPtr = start;
      return XML_ERROR_NONE;
    }
    parser->m_eventPtr = start;
    return XML_ERROR_PARTIAL_CHAR;
  }
  parser->m_processor = externalEntityInitProcessor3;
  return externalEntityInitProcessor3(parser, start, end, endPtr);
}

_Safe static enum XML_Error PTRCALL
externalEntityInitProcessor3(XML_Parser _Borrow parser, const char *start,
                             const char *end, const char **_Nonnull endPtr) {
  int tok;
  const char *next = start; /* XmlContentTok doesn't always set the last arg */
  parser->m_eventPtr = start;
  tok = XmlContentTok(parser->m_encoding, start, end, _Unsafe(&next));
  /* Note: These bytes are accounted later in:
           - processXmlDecl
           - externalEntityContentProcessor
  */
  parser->m_eventEndPtr = next;

  switch (tok) {
  case XML_TOK_XML_DECL: {
    enum XML_Error result;
    result = processXmlDecl(parser, 1, start, next);
    if (result != XML_ERROR_NONE)
      return result;
    switch (parser->m_parsingStatus.parsing) {
    case XML_SUSPENDED:
      _Unsafe *endPtr = next;
      return XML_ERROR_NONE;
    case XML_FINISHED:
      return XML_ERROR_ABORTED;
    case XML_PARSING:
      if (parser->m_reenter) {
        return XML_ERROR_UNEXPECTED_STATE; // LCOV_EXCL_LINE
      }
      EXPAT_FALLTHROUGH;
    default:
      start = next;
    }
  } break;
  case XML_TOK_PARTIAL:
    if (! parser->m_parsingStatus.finalBuffer) {
      _Unsafe *endPtr = start;
      return XML_ERROR_NONE;
    }
    return XML_ERROR_UNCLOSED_TOKEN;
  case XML_TOK_PARTIAL_CHAR:
    if (! parser->m_parsingStatus.finalBuffer) {
      _Unsafe *endPtr = start;
      return XML_ERROR_NONE;
    }
    return XML_ERROR_PARTIAL_CHAR;
  }
  parser->m_processor = externalEntityContentProcessor;
  parser->m_tagLevel = 1;
  return externalEntityContentProcessor(parser, start, end, endPtr);
}

_Safe static enum XML_Error PTRCALL
externalEntityContentProcessor(XML_Parser _Borrow parser, const char *start,
                               const char *end, const char **_Nonnull endPtr) {
  __auto_type bsc_h16 = parser->m_encoding;
  __auto_type bsc_h17 = parser->m_parsingStatus.finalBuffer;
  enum XML_Error result
      = doContent(parser, 1, bsc_h16, start, end, endPtr,
                  (XML_Bool)! bsc_h17,
                  XML_ACCOUNT_ENTITY_EXPANSION);
  if (result == XML_ERROR_NONE) {
    if (! storeRawNames(parser))
      return XML_ERROR_NO_MEMORY;
  }
  return result;
}

_Safe static enum XML_Error
doContent(XML_Parser _Borrow parser, int startTagLevel, const ENCODING *_Nonnull enc,
          const char *s, const char *end, const char **_Nonnull nextPtr,
          XML_Bool haveMore, enum XML_Account account) {
  /* save one level of indirection */
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return XML_ERROR_UNEXPECTED_STATE;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);

  const char **eventPP;
  const char **eventEndPP;
  if (enc == parser->m_encoding) {
    eventPP = _Unsafe(&parser->m_eventPtr);
    eventEndPP = _Unsafe(&parser->m_eventEndPtr);
  } else {
    eventPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventPtr));
    eventEndPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventEndPtr));
  }
  _Unsafe *eventPP = s;

  for (;;) {
    const char *next = s; /* XmlContentTok doesn't always set the last arg */
    int tok = XmlContentTok(enc, s, end, _Unsafe(&next));
#if XML_GE == 1
    const char *accountAfter
        = ((tok == XML_TOK_TRAILING_RSQB) || (tok == XML_TOK_TRAILING_CR))
              ? (haveMore ? s /* i.e. 0 bytes */ : end)
              : next;
    if (! accountingDiffTolerated(parser, tok, s, accountAfter, __LINE__,
                                  account)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif
    _Unsafe *eventEndPP = next;
    switch (tok) {
    case XML_TOK_TRAILING_CR:
      if (haveMore) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      _Unsafe *eventEndPP = end;
      if (parser->m_characterDataHandler) {
        XML_Char c = 0xA;
        beforeHandler(parser);
        _Unsafe(parser->m_characterDataHandler(parser->m_handlerArg, _Unsafe(&c), 1));
        afterHandler(parser);
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, end);
      /* We are at the end of the final buffer, should we check for
         XML_SUSPENDED, XML_FINISHED?
      */
      if (startTagLevel == 0)
        return XML_ERROR_NO_ELEMENTS;
      if (parser->m_tagLevel != startTagLevel)
        return XML_ERROR_ASYNC_ENTITY;
      _Unsafe *nextPtr = end;
      return XML_ERROR_NONE;
    case XML_TOK_NONE:
      if (haveMore) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      if (startTagLevel > 0) {
        if (parser->m_tagLevel != startTagLevel)
          return XML_ERROR_ASYNC_ENTITY;
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_NO_ELEMENTS;
    case XML_TOK_INVALID:
      _Unsafe *eventPP = next;
      return XML_ERROR_INVALID_TOKEN;
    case XML_TOK_PARTIAL:
      if (haveMore) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_UNCLOSED_TOKEN;
    case XML_TOK_PARTIAL_CHAR:
      if (haveMore) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_PARTIAL_CHAR;
    case XML_TOK_ENTITY_REF: {
      const XML_Char *name;
      ENTITY *_Nonnull entity;
      XML_Char ch = (XML_Char)XmlPredefinedEntityName(
          enc, s + _Unsafe(enc->minBytesPerChar), next - _Unsafe(enc->minBytesPerChar));
      if (ch) {
#if XML_GE == 1
        /* NOTE: We are replacing 4-6 characters original input for 1 character
         *       so there is no amplification and hence recording without
         *       protection. */
        accountingDiffTolerated(parser, tok, (char *)_Unsafe(&ch),
                                ((char *)_Unsafe(&ch)) + sizeof(XML_Char), __LINE__,
                                XML_ACCOUNT_ENTITY_EXPANSION);
#endif /* XML_GE == 1 */
        if (parser->m_characterDataHandler) {
          beforeHandler(parser);
          _Unsafe(parser->m_characterDataHandler(parser->m_handlerArg, _Unsafe(&ch), 1));
          afterHandler(parser);
        } else if (parser->m_defaultHandler)
          reportDefault(parser, enc, s, next);
        break;
      }
      name = poolStoreString(&_Mut dtd->pool, enc, s + _Unsafe(enc->minBytesPerChar),
                             next - _Unsafe(enc->minBytesPerChar));
      if (! name)
        return XML_ERROR_NO_MEMORY;
      _Unsafe entity = (ENTITY *)lookup(parser, &_Mut dtd->generalEntities, name, 0);
      poolDiscard(&_Mut dtd->pool);
      /* First, determine if a check for an existing declaration is needed;
         if yes, check that the entity exists, and that it is internal,
         otherwise call the skipped entity or default handler.
      */
      if (! dtd->hasParamEntityRefs || dtd->standalone) {
        if (! entity)
          return XML_ERROR_UNDEFINED_ENTITY;
        else if (_Unsafe((! entity->is_internal)))
          return XML_ERROR_ENTITY_DECLARED_IN_PE;
      } else if (! entity) {
        if (parser->m_skippedEntityHandler) {
          beforeHandler(parser);
          _Unsafe(parser->m_skippedEntityHandler(parser->m_handlerArg, name, 0));
          afterHandler(parser);
        } else if (parser->m_defaultHandler)
          reportDefault(parser, enc, s, next);
        break;
      }
      if (_Unsafe((entity->open)))
        return XML_ERROR_RECURSIVE_ENTITY_REF;
      if (_Unsafe((entity->notation)))
        return XML_ERROR_BINARY_ENTITY_REF;
      if (_Unsafe((entity->textPtr))) {
        enum XML_Error result;
        if (! parser->m_defaultExpandInternalEntities) {
          if (parser->m_skippedEntityHandler) {
            beforeHandler(parser);
            _Unsafe(parser->m_skippedEntityHandler(parser->m_handlerArg, entity->name,
                                           0));
            afterHandler(parser);
          } else if (parser->m_defaultHandler)
            reportDefault(parser, enc, s, next);
          break;
        }
        result = processEntity(parser, entity, XML_FALSE, ENTITY_INTERNAL);
        if (result != XML_ERROR_NONE)
          return result;
      } else if (parser->m_externalEntityRefHandler) {
        const XML_Char *context;
        _Unsafe entity->open = true;
        context = getContext(parser);
        _Unsafe entity->open = false;
        if (! context)
          return XML_ERROR_NO_MEMORY;
        beforeHandler(parser);
        const int status = _Unsafe(parser->m_externalEntityRefHandler(
            parser->m_externalEntityRefHandlerArg, context, entity->base,
            entity->systemId, entity->publicId));
        afterHandler(parser);
        if (! status)
          return XML_ERROR_EXTERNAL_ENTITY_HANDLING;
        poolDiscard(&_Mut parser->m_tempPool);
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      break;
    }
    case XML_TOK_START_TAG_NO_ATTS:
    case XML_TOK_START_TAG_WITH_ATTS: {
      TAG *tag;
      enum XML_Error result;
      XML_Char *toPtr;
      if (parser->m_freeTagList) {
        tag = parser->m_freeTagList;
        _Unsafe parser->m_freeTagList = parser->m_freeTagList->parent;
      } else {
        _Unsafe tag = MALLOC(parser, sizeof(TAG));
        if (! tag)
          return XML_ERROR_NO_MEMORY;
        _Unsafe tag->buf.raw = MALLOC(parser, INIT_TAG_BUF_SIZE);
        if (_Unsafe((! tag->buf.raw))) {
          _Unsafe FREE(parser, tag);
          return XML_ERROR_NO_MEMORY;
        }
        _Unsafe tag->bufEnd = tag->buf.raw + INIT_TAG_BUF_SIZE;
      }
      _Unsafe tag->bindings = nullptr;
      _Unsafe tag->parent = parser->m_tagStack;
      parser->m_tagStack = tag;
      _Unsafe tag->name.localPart = nullptr;
      _Unsafe tag->name.prefix = nullptr;
      _Unsafe tag->rawName = s + _Unsafe(enc->minBytesPerChar);
      _Unsafe tag->rawNameLength = XmlNameLength(enc, tag->rawName);
      ++parser->m_tagLevel;
      {
        const char *rawNameEnd = _Unsafe((tag->rawName + tag->rawNameLength));
        const char *fromPtr = _Unsafe((tag->rawName));
        _Unsafe toPtr = tag->buf.str;
        for (;;) {
          const enum XML_Convert_Result convert_res
              = _Unsafe(XmlConvert(enc, _Unsafe(&fromPtr), rawNameEnd, (ICHAR **)_Unsafe(&toPtr),
                           (ICHAR *)tag->bufEnd - 1));
          const size_t convLen = _Unsafe(((size_t)(toPtr - tag->buf.str)));
          if ((fromPtr >= rawNameEnd)
              || (convert_res == XML_CONVERT_INPUT_INCOMPLETE)) {
            _Unsafe tag->name.strLen = convLen;
            break;
          }
          if (_Unsafe((SIZE_MAX / 2 < (size_t)(tag->bufEnd - tag->buf.raw))))
            return XML_ERROR_NO_MEMORY;
          const size_t bufSize = _Unsafe(((size_t)(tag->bufEnd - tag->buf.raw) * 2));
          {
            char *temp = _Unsafe((char *)(REALLOC(parser, tag->buf.raw, bufSize)));
            if (temp == nullptr)
              return XML_ERROR_NO_MEMORY;
            _Unsafe tag->buf.raw = temp;
            _Unsafe tag->bufEnd = temp + bufSize;
            toPtr = (XML_Char *)temp + convLen;
          }
        }
      }
      _Unsafe tag->name.str = tag->buf.str;
      _Unsafe *toPtr = XML_T('\0');
      result
          = storeAtts(parser, enc, s, _Unsafe(&_Mut tag->name), _Unsafe(&tag->bindings), account);
      if (result)
        return result;
      if (parser->m_startElementHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_startElementHandler(parser->m_handlerArg, tag->name.str,
                                      (const XML_Char **)(ATTRIBUTE *)&_Mut *parser->m_atts));
        afterHandler(parser);
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      poolClear(&_Mut parser->m_tempPool);
      break;
    }
    case XML_TOK_EMPTY_ELEMENT_NO_ATTS:
    case XML_TOK_EMPTY_ELEMENT_WITH_ATTS: {
      const char *rawName = s + _Unsafe(enc->minBytesPerChar);
      enum XML_Error result;
      BINDING *bindings = nullptr;
      XML_Bool noElmHandlers = XML_TRUE;
      const XML_Char *nameStr = poolStoreString(
          &_Mut parser->m_tempPool, enc, rawName, rawName + XmlNameLength(enc, rawName));
      if (! nameStr)
        return XML_ERROR_NO_MEMORY;
      TAG_NAME name = {nameStr, nullptr, nullptr, 0, 0, 0};
      poolFinish(&_Mut parser->m_tempPool);
      result = storeAtts(parser, enc, s, &_Mut name, _Unsafe(_Unsafe(&bindings)),
                         XML_ACCOUNT_NONE /* token spans whole start tag */);
      if (result != XML_ERROR_NONE) {
        freeBindings(parser, bindings);
        return result;
      }
      poolFinish(&_Mut parser->m_tempPool);
      if (parser->m_startElementHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_startElementHandler(parser->m_handlerArg, name.str,
                                      (const XML_Char **)(ATTRIBUTE *)&_Mut *parser->m_atts));
        afterHandler(parser);
        noElmHandlers = XML_FALSE;
      }
      if (parser->m_endElementHandler) {
        if (parser->m_startElementHandler)
          _Unsafe *eventPP = *eventEndPP;
        beforeHandler(parser);
        _Unsafe(parser->m_endElementHandler(parser->m_handlerArg, name.str));
        afterHandler(parser);
        noElmHandlers = XML_FALSE;
      }
      if (noElmHandlers && parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      poolClear(&_Mut parser->m_tempPool);
      freeBindings(parser, bindings);
    }
      if ((parser->m_tagLevel == 0)
          && (parser->m_parsingStatus.parsing != XML_FINISHED)) {
        if (parser->m_parsingStatus.parsing == XML_SUSPENDED
            || (parser->m_parsingStatus.parsing == XML_PARSING
                && parser->m_reenter))
          parser->m_processor = epilogProcessor;
        else
          return epilogProcessor(parser, next, end, nextPtr);
      }
      break;
    case XML_TOK_END_TAG:
      if (parser->m_tagLevel == startTagLevel)
        return XML_ERROR_ASYNC_ENTITY;
      else {
        int len;
        const char *rawName;
        TAG *tag = parser->m_tagStack;
        rawName = s + _Unsafe(enc->minBytesPerChar) * 2;
        len = XmlNameLength(enc, rawName);
        if (_Unsafe((len != tag->rawNameLength
            || _Unsafe(memcmp(tag->rawName, rawName, len)) != 0))) {
          _Unsafe *eventPP = rawName;
          return XML_ERROR_TAG_MISMATCH;
        }
        _Unsafe parser->m_tagStack = tag->parent;
        _Unsafe tag->parent = parser->m_freeTagList;
        parser->m_freeTagList = tag;
        --parser->m_tagLevel;
        if (parser->m_endElementHandler) {
          const XML_Char *localPart;
          const XML_Char *prefix;
          XML_Char *uri;
          _Unsafe localPart = tag->name.localPart;
          if (parser->m_ns && localPart) {
            /* localPart and prefix may have been overwritten in
               tag->name.str, since this points to the binding->uri
               buffer which gets reused; so we have to add them again
            */
            _Unsafe uri = (XML_Char *)tag->name.str + tag->name.uriLen;
            /* don't need to check for space - already done in storeAtts() */
            while (_Unsafe((*localPart)))
              _Unsafe *uri++ = *localPart++;
            _Unsafe prefix = tag->name.prefix;
            if (parser->m_ns_triplets && prefix) {
              _Unsafe *uri++ = parser->m_namespaceSeparator;
              while (_Unsafe((*prefix)))
                _Unsafe *uri++ = *prefix++;
            }
            _Unsafe *uri = XML_T('\0');
          }
          beforeHandler(parser);
          _Unsafe(parser->m_endElementHandler(parser->m_handlerArg, tag->name.str));
          afterHandler(parser);
        } else if (parser->m_defaultHandler)
          reportDefault(parser, enc, s, next);
        while (_Unsafe((tag->bindings))) {
          BINDING *b = _Unsafe((tag->bindings));
          if (parser->m_endNamespaceDeclHandler) {
            beforeHandler(parser);
            _Unsafe(parser->m_endNamespaceDeclHandler(parser->m_handlerArg,
                                              b->prefix->name));
            afterHandler(parser);
          }
          _Unsafe tag->bindings = tag->bindings->nextTagBinding;
          _Unsafe b->nextTagBinding = parser->m_freeBindingList;
          parser->m_freeBindingList = b;
          _Unsafe b->prefix->binding = b->prevPrefixBinding;
        }
        if ((parser->m_tagLevel == 0)
            && (parser->m_parsingStatus.parsing != XML_FINISHED)) {
          if (parser->m_parsingStatus.parsing == XML_SUSPENDED
              || (parser->m_parsingStatus.parsing == XML_PARSING
                  && parser->m_reenter))
            parser->m_processor = epilogProcessor;
          else
            return epilogProcessor(parser, next, end, nextPtr);
        }
      }
      break;
    case XML_TOK_CHAR_REF: {
      int n = XmlCharRefNumber(enc, s);
      if (n < 0)
        return XML_ERROR_BAD_CHAR_REF;
      if (parser->m_characterDataHandler) {
        XML_Char buf[XML_ENCODE_MAX] = {0};
        beforeHandler(parser);
        _Unsafe(parser->m_characterDataHandler(parser->m_handlerArg, buf,
                                       _Unsafe(XmlEncode(n, (ICHAR *)buf))));
        afterHandler(parser);
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
    } break;
    case XML_TOK_XML_DECL:
      return XML_ERROR_MISPLACED_XML_PI;
    case XML_TOK_DATA_NEWLINE:
      if (parser->m_characterDataHandler) {
        XML_Char c = 0xA;
        beforeHandler(parser);
        _Unsafe(parser->m_characterDataHandler(parser->m_handlerArg, _Unsafe(&c), 1));
        afterHandler(parser);
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      break;
    case XML_TOK_CDATA_SECT_OPEN: {
      enum XML_Error result;
      if (parser->m_startCdataSectionHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_startCdataSectionHandler(parser->m_handlerArg));
        afterHandler(parser);
        /* BEGIN disabled code */
        /* Suppose you doing a transformation on a document that involves
           changing only the character data.  You set up a defaultHandler
           and a characterDataHandler.  The defaultHandler simply copies
           characters through.  The characterDataHandler does the
           transformation and writes the characters out escaping them as
           necessary.  This case will fail to work if we leave out the
           following two lines (because & and < inside CDATA sections will
           be incorrectly escaped).

           However, now we have a start/endCdataSectionHandler, so it seems
           easier to let the user deal with this.
        */
      } else if ((0) && parser->m_characterDataHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_characterDataHandler(parser->m_handlerArg, (XML_Char *)&_Mut *parser->m_dataBuf,
                                       0));
        afterHandler(parser);
        /* END disabled code */
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      result
          = doCdataSection(parser, enc, _Unsafe(&next), end, nextPtr, haveMore, account);
      if (result != XML_ERROR_NONE)
        return result;
      else if (! next) {
        parser->m_processor = cdataSectionProcessor;
        return result;
      }
    } break;
    case XML_TOK_TRAILING_RSQB:
      if (haveMore) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      if (parser->m_characterDataHandler) {
        if (MUST_CONVERT(enc, s)) {
          ICHAR *dataPtr = _Unsafe(((ICHAR *)&_Mut *parser->m_dataBuf));
          _Unsafe(XmlConvert(enc, _Unsafe(&s), end, _Unsafe(&dataPtr), (ICHAR *)parser->m_dataBufEnd));
          beforeHandler(parser);
          _Unsafe(parser->m_characterDataHandler(
              parser->m_handlerArg, (XML_Char *)&_Mut *parser->m_dataBuf,
              (int)(dataPtr - (ICHAR *)&_Mut *parser->m_dataBuf)));
          afterHandler(parser);
        } else {
          beforeHandler(parser);
          _Unsafe(parser->m_characterDataHandler(
              parser->m_handlerArg, (const XML_Char *)s,
              (int)((const XML_Char *)end - (const XML_Char *)s)));
          afterHandler(parser);
        }
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, end);
      /* We are at the end of the final buffer, should we check for
         XML_SUSPENDED, XML_FINISHED?
      */
      if (startTagLevel == 0) {
        _Unsafe *eventPP = end;
        return XML_ERROR_NO_ELEMENTS;
      }
      if (parser->m_tagLevel != startTagLevel) {
        _Unsafe *eventPP = end;
        return XML_ERROR_ASYNC_ENTITY;
      }
      _Unsafe *nextPtr = end;
      return XML_ERROR_NONE;
    case XML_TOK_DATA_CHARS: {
      XML_CharacterDataHandler charDataHandler = parser->m_characterDataHandler;
      if (charDataHandler) {
        if (MUST_CONVERT(enc, s)) {
          for (;;) {
            ICHAR *dataPtr = _Unsafe(((ICHAR *)&_Mut *parser->m_dataBuf));
            const enum XML_Convert_Result convert_res = _Unsafe(XmlConvert(
                enc, _Unsafe(&s), next, _Unsafe(&dataPtr), (ICHAR *)parser->m_dataBufEnd));
            _Unsafe *eventEndPP = s;
            beforeHandler(parser);
            _Unsafe charDataHandler(parser->m_handlerArg, (XML_Char *)&_Mut *parser->m_dataBuf,
                            (int)(dataPtr - (ICHAR *)&_Mut *parser->m_dataBuf));
            afterHandler(parser);
            if ((convert_res == XML_CONVERT_COMPLETED)
                || (convert_res == XML_CONVERT_INPUT_INCOMPLETE))
              break;
            _Unsafe *eventPP = s;
          }
        } else {
          beforeHandler(parser);
          _Unsafe charDataHandler(parser->m_handlerArg, (const XML_Char *)s,
                          (int)((const XML_Char *)next - (const XML_Char *)s));
          afterHandler(parser);
        }
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
    } break;
    case XML_TOK_PI:
      if (! reportProcessingInstruction(parser, enc, s, next))
        return XML_ERROR_NO_MEMORY;
      break;
    case XML_TOK_COMMENT:
      if (! reportComment(parser, enc, s, next))
        return XML_ERROR_NO_MEMORY;
      break;
    default:
      /* All of the tokens produced by XmlContentTok() have their own
       * explicit cases, so this default is not strictly necessary.
       * However it is a useful safety net, so we retain the code and
       * simply exclude it from the coverage tests.
       *
       * LCOV_EXCL_START
       */
      if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      break;
      /* LCOV_EXCL_STOP */
    }
    switch (parser->m_parsingStatus.parsing) {
    case XML_SUSPENDED:
      _Unsafe *eventPP = next;
      _Unsafe *nextPtr = next;
      return XML_ERROR_NONE;
    case XML_FINISHED:
      _Unsafe *eventPP = next;
      return XML_ERROR_ABORTED;
    case XML_PARSING:
      if (parser->m_reenter) {
        _Unsafe *nextPtr = next;
        return XML_ERROR_NONE;
      }
      EXPAT_FALLTHROUGH;
    default:;
      _Unsafe *eventPP = s = next;
    }
  }
  /* not reached */
}

/* This function does not call free() on the allocated memory, merely
 * moving it to the parser's m_freeBindingList where it can be freed or
 * reused as appropriate.
 */
_Safe static void
freeBindings(XML_Parser _Borrow parser, BINDING *bindings) {
  while (bindings) {
    BINDING *b = bindings;

    /* m_startNamespaceDeclHandler will have been called for this
     * binding in addBindings(), so call the end handler now.
     */
    if (parser->m_endNamespaceDeclHandler) {
      beforeHandler(parser);
      _Unsafe(parser->m_endNamespaceDeclHandler(parser->m_handlerArg, b->prefix->name));
      afterHandler(parser);
    }

    _Unsafe bindings = bindings->nextTagBinding;
    _Unsafe b->nextTagBinding = parser->m_freeBindingList;
    parser->m_freeBindingList = b;
    _Unsafe b->prefix->binding = b->prevPrefixBinding;
  }
}

/* Precondition: all arguments must be non-nullptr;
   Purpose:
   - normalize attributes
   - check attributes for well-formedness
   - generate namespace aware attribute names (URI, prefix)
   - build list of attributes for startElementHandler
   - default attributes
   - process namespace declarations (check and report them)
   - generate namespace aware element name (URI, prefix)
*/
_Safe static enum XML_Error
storeAtts(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char *attStr,
          TAG_NAME *_Borrow tagNamePtr, BINDING **_Nonnull bindingsPtr,
          enum XML_Account account) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return XML_ERROR_UNEXPECTED_STATE;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  int attIndex = 0;
  XML_Char *uri;
  if (parser->m_atts == nullptr)
    return XML_ERROR_NO_MEMORY;
  int nPrefixes = 0;
  BINDING *binding;
  const XML_Char *localPart;

  /* lookup the element type name */
  ELEMENT_TYPE *elementType
      = _Unsafe(((ELEMENT_TYPE *)lookup(parser, &_Mut dtd->elementTypes, tagNamePtr->str, 0)));
  if (! elementType) {
    const XML_Char *name = poolCopyString(&_Mut dtd->pool, tagNamePtr->str);
    if (! name)
      return XML_ERROR_NO_MEMORY;
    _Unsafe elementType = (ELEMENT_TYPE *)lookup(parser, &_Mut dtd->elementTypes, name,
                                         sizeof(ELEMENT_TYPE));
    if (! elementType)
      return XML_ERROR_NO_MEMORY;
    if (_Unsafe((! elementType->defaultAttForName.parser)))
      hashTableInit(_Unsafe(&_Mut elementType->defaultAttForName), RAW_PARSER(parser));
    if (parser->m_ns && ! setElementTypePrefix(parser, elementType))
      return XML_ERROR_NO_MEMORY;
  }
  const size_t nDefaultAtts = _Unsafe((elementType->nDefaultAtts));

  /* Detect and prevent integer overflow. */
  if (parser->m_attsSize > (size_t)INT_MAX)
    return XML_ERROR_NO_MEMORY;

  /* get the attributes from the tokenizer */
  size_t n = (size_t)XmlGetAttributes(
      enc, attStr, (int)parser->m_attsSize,
      _Unsafe((ATTRIBUTE *)&_Mut *parser->m_atts));

  /* Detect and prevent integer overflow */
  if (n > SIZE_MAX - nDefaultAtts) {
    return XML_ERROR_NO_MEMORY;
  }

  if (n + nDefaultAtts > parser->m_attsSize) {
    size_t oldAttsSize = parser->m_attsSize;

    /* Detect and prevent integer overflow */
    if ((nDefaultAtts > SIZE_MAX - INIT_ATTS_SIZE)
        || (n > SIZE_MAX - (nDefaultAtts + INIT_ATTS_SIZE))) {
      return XML_ERROR_NO_MEMORY;
    }

    parser->m_attsSize = n + nDefaultAtts + INIT_ATTS_SIZE;

    /* Detect and prevent integer overflow. */
    if (parser->m_attsSize > SIZE_MAX / sizeof(ATTRIBUTE)) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }

    XML_Bool attsOk = XML_FALSE;
    __auto_type bsc_h38 = parser->m_attsSize;
    REALLOC_ARRAY(parser, parser->m_atts, ATTRIBUTE,
                  bsc_h38 * sizeof(ATTRIBUTE), attsOk);
    if (! attsOk) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }
#ifdef XML_ATTR_INFO
    /* Detect and prevent integer overflow. */
    if (parser->m_attsSize > SIZE_MAX / sizeof(XML_AttrInfo)) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }

    XML_Bool attInfoOk = XML_FALSE;
    REALLOC_ARRAY(parser, parser->m_attInfo, XML_AttrInfo,
                  parser->m_attsSize * sizeof(XML_AttrInfo), attInfoOk);
    if (! attInfoOk) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }
#endif
    if (n > oldAttsSize) {
      /* Detect and prevent integer overflow. */
      if (n > (size_t)INT_MAX)
        return XML_ERROR_NO_MEMORY;
      XmlGetAttributes(enc, attStr, (int)n,
                       _Unsafe((ATTRIBUTE *)&_Mut *parser->m_atts));
    }
  }
  if (parser->m_atts == nullptr)
    return XML_ERROR_NO_MEMORY;

  /* the attribute list for the application */
  const XML_Char **const appAtts
      = _Unsafe((const XML_Char **)(ATTRIBUTE *)&_Mut *parser->m_atts);
  for (size_t i = 0; i < n; i++) {
    ATTRIBUTE *currAtt = _Unsafe((ATTRIBUTE *)&_Mut parser->m_atts[i]);
#ifdef XML_ATTR_INFO
    XML_AttrInfo *currAttInfo = _Unsafe((XML_AttrInfo *)&_Mut parser->m_attInfo[i]);
#endif
    /* add the name and value to the attribute list */
    ATTRIBUTE_ID *attId
        = _Unsafe((getAttributeId(parser, enc, currAtt->name,
                         currAtt->name + XmlNameLength(enc, currAtt->name))));
    if (! attId)
      return XML_ERROR_NO_MEMORY;
#ifdef XML_ATTR_INFO
    // NOTE: XML_Index is known to wrap around for >2 GiB content
    //       on 32bit machines and 64bit Windows, unless (non-default and
    //       uncommon) XML_LARGE_SIZE is defined.
    //       That's a bug and it only lives on because we cannot break
    //       ABI compatibility of public API.
    currAttInfo->nameStart
        = (XML_Index)(parser->m_parseEndByteIndex
                      - (parser->m_parseEndPtr - currAtt->name));
    currAttInfo->nameEnd
        = currAttInfo->nameStart + XmlNameLength(enc, currAtt->name);
    currAttInfo->valueStart
        = (XML_Index)(parser->m_parseEndByteIndex
                      - (parser->m_parseEndPtr - currAtt->valuePtr));
    currAttInfo->valueEnd
        = (XML_Index)(parser->m_parseEndByteIndex
                      - (parser->m_parseEndPtr - currAtt->valueEnd));
#endif
    /* Detect duplicate attributes by their QNames. This does not work when
       namespace processing is turned on and different prefixes for the same
       namespace are used. For this case we have a check further down.
    */
    if (_Unsafe(((attId->name)[-1]))) {
      if (enc == parser->m_encoding)
        parser->m_eventPtr = parser->m_atts[i].name;
      return XML_ERROR_DUPLICATE_ATTRIBUTE;
    }
    _Unsafe { (attId->name)[-1] = 1; }
    _Unsafe appAtts[attIndex] = attId->name;
      attIndex++;
    if (! parser->m_atts[i].normalized) {
      XML_Bool isCdata = XML_TRUE;

      /* figure out whether declared as other than CDATA */
      if (_Unsafe((attId->maybeTokenized))) {
        NAME_AND_DEFAULT_ATTRIBUTE *const nameAndDefaultAttribute
            = _Unsafe(((NAME_AND_DEFAULT_ATTRIBUTE *)lookup(
                parser, _Unsafe(&_Mut elementType->defaultAttForName), attId->name, 0)));
        if (nameAndDefaultAttribute != nullptr) {
          _Unsafe assert(nameAndDefaultAttribute->attIndex < elementType->nDefaultAtts);
          const DEFAULT_ATTRIBUTE *const att
              = _Unsafe((const DEFAULT_ATTRIBUTE *)&_Const *elementType->defaultAtts
                        + nameAndDefaultAttribute->attIndex);
          _Unsafe isCdata = att->isCdata;
        }
      }

      /* normalize the attribute value */
      STRING_POOL *tempPoolRaw = _Unsafe(_Unsafe(_Unsafe(&parser->m_tempPool)));
      __auto_type bsc_h39 = parser->m_atts[i].valuePtr;
      __auto_type bsc_h40 = parser->m_atts[i].valueEnd;
      const enum XML_Error result = storeAttributeValue(
          parser, enc, isCdata, bsc_h39,
          bsc_h40, _Unsafe(&_Mut *tempPoolRaw), account);
      if (result)
        return result;
      appAtts[attIndex] = poolStart(&_Mut parser->m_tempPool);
      poolFinish(&_Mut parser->m_tempPool);
    } else {
      /* the value did not need normalizing */
      appAtts[attIndex] = poolStoreString(&_Mut parser->m_tempPool, enc,
                                          parser->m_atts[i].valuePtr,
                                          parser->m_atts[i].valueEnd);
      if (appAtts[attIndex] == 0)
        return XML_ERROR_NO_MEMORY;
      poolFinish(&_Mut parser->m_tempPool);
    }
    /* handle prefixed attribute names */
    if (_Unsafe((attId->prefix))) {
      if (_Unsafe((attId->xmlns))) {
        /* deal with namespace declarations here */
        enum XML_Error result = _Unsafe((addBinding(parser, attId->prefix, attId,
                                           appAtts[attIndex], bindingsPtr)));
        if (result)
          return result;
        --attIndex;
      } else {
        /* deal with other prefixed names later */
        attIndex++;
        nPrefixes++;
        _Unsafe { (attId->name)[-1] = 2; }
      }
    } else
      attIndex++;
  }

  /* set-up for XML_GetSpecifiedAttributeCount and XML_GetIdAttributeIndex */
  parser->m_nSpecifiedAtts = attIndex;
  if (_Unsafe((elementType->idAtt && (elementType->idAtt->name)[-1]))) {
    for (int i = 0; i < attIndex; i += 2)
      if (_Unsafe((appAtts[i] == elementType->idAtt->name))) {
        parser->m_idAttIndex = i;
        break;
      }
  } else
    parser->m_idAttIndex = -1;

  /* do attribute defaulting */
  for (size_t i = 0; i < nDefaultAtts; i++) {
    const DEFAULT_ATTRIBUTE *da
        = _Unsafe((const DEFAULT_ATTRIBUTE *)&_Const *elementType->defaultAtts + i);
    if (_Unsafe((! (da->id->name)[-1] && da->value))) {
      if (_Unsafe((da->id->prefix))) {
        if (_Unsafe((da->id->xmlns))) {
          enum XML_Error result = _Unsafe((addBinding(parser, da->id->prefix, da->id,
                                             da->value, bindingsPtr)));
          if (result)
            return result;
        } else {
          _Unsafe { (da->id->name)[-1] = 2; }
          nPrefixes++;
          _Unsafe appAtts[attIndex] = da->id->name;
      attIndex++;
          _Unsafe appAtts[attIndex] = da->value;
      attIndex++;
        }
      } else {
        _Unsafe { (da->id->name)[-1] = 1; }
        _Unsafe appAtts[attIndex] = da->id->name;
      attIndex++;
        _Unsafe appAtts[attIndex] = da->value;
      attIndex++;
      }
    }
  }
  appAtts[attIndex] = 0;

  /* expand prefixed attribute names, check for duplicates,
     and clear flags that say whether attributes were specified */
  int i = 0;
  if (nPrefixes) {
    unsigned int j; /* hash table index */
    unsigned long version = parser->m_nsAttsVersion;

    /* Detect and prevent invalid shift */
    if (parser->m_nsAttsPower >= sizeof(unsigned int) * 8 /* bits per byte */) {
      return XML_ERROR_NO_MEMORY;
    }

    unsigned int nsAttsSize = 1u << parser->m_nsAttsPower;
    unsigned char oldNsAttsPower = parser->m_nsAttsPower;
    /* size of hash table must be at least 2 * (# of prefixed attributes) */
    if (parser->m_nsAttsPower == 0
        || (nPrefixes >> (parser->m_nsAttsPower - 1))) {
      /* hash table size must also be a power of 2 and >= 8 */
      for (;;) {
        const unsigned char oldPower = parser->m_nsAttsPower;
        parser->m_nsAttsPower++;
        if (! (nPrefixes >> oldPower))
          break;
      }
      if (parser->m_nsAttsPower < 3)
        parser->m_nsAttsPower = 3;

      /* Detect and prevent invalid shift */
      if (parser->m_nsAttsPower >= sizeof(nsAttsSize) * 8 /* bits per byte */) {
        /* Restore actual size of memory in m_nsAtts */
        parser->m_nsAttsPower = oldNsAttsPower;
        return XML_ERROR_NO_MEMORY;
      }

      nsAttsSize = 1u << parser->m_nsAttsPower;

      /* Detect and prevent integer overflow.
       * The preprocessor guard addresses the "always false" warning
       * from -Wtype-limits on platforms where
       * sizeof(unsigned int) < sizeof(size_t), e.g. on x86_64. */
#if UINT_MAX >= SIZE_MAX
      if (nsAttsSize > SIZE_MAX / sizeof(NS_ATT)) {
        /* Restore actual size of memory in m_nsAtts */
        parser->m_nsAttsPower = oldNsAttsPower;
        return XML_ERROR_NO_MEMORY;
      }
#endif

      XML_Bool nsAttsOk = XML_FALSE;
      REALLOC_ARRAY(parser, parser->m_nsAtts, NS_ATT, nsAttsSize * sizeof(NS_ATT),
                    nsAttsOk);
      if (! nsAttsOk) {
        /* Restore actual size of memory in m_nsAtts */
        parser->m_nsAttsPower = oldNsAttsPower;
        return XML_ERROR_NO_MEMORY;
      }
      version = 0; /* force re-initialization of m_nsAtts hash table */
    }
    if (parser->m_nsAtts == nullptr)
      return XML_ERROR_NO_MEMORY;
    /* using a version flag saves us from initializing m_nsAtts every time */
    if (! version) { /* initialize version flags when version wraps around */
      version = INIT_ATTS_VERSION;
      for (j = nsAttsSize; j != 0;) {
        j--;
        parser->m_nsAtts[j].version = version;
      }
    }
    _Unsafe parser->m_nsAttsVersion = --version;

    /* expand prefixed names and check for duplicates */
    for (; i < attIndex; i += 2) {
      const XML_Char *s = appAtts[i];
      if (_Unsafe((s[-1] == 2))) { /* prefixed */
        struct siphash sip_state = {0, 0, 0, 0, {0}, nullptr, 0};
        struct sipkey sip_key = {{0, 0}};

        copy_salt_to_sipkey(parser, &_Mut sip_key);
        _Unsafe sip24_init(_Unsafe(&sip_state), _Unsafe(&sip_key));

        _Unsafe { ((XML_Char *)s)[-1] = 0; } /* clear flag */
        ATTRIBUTE_ID *const id
            = _Unsafe(((ATTRIBUTE_ID *)lookup(parser, &_Mut dtd->attributeIds, s, 0)));
        if (_Unsafe((! id || ! id->prefix))) {
          /* This code is walking through the appAtts array, dealing
           * with (in this case) a prefixed attribute name.  To be in
           * the array, the attribute must have already been bound, so
           * has to have passed through the hash table lookup once
           * already.  That implies that an entry for it already
           * exists, so the lookup above will return a pointer to
           * already allocated memory.  There is no opportunity for
           * the allocator to fail, so the condition above cannot be
           * fulfilled.
           *
           * Since it is difficult to be certain that the above
           * analysis is complete, we retain the test and merely
           * remove the code from coverage tests.
           */
          return XML_ERROR_NO_MEMORY; /* LCOV_EXCL_LINE */
        }
        const BINDING *const b = _Unsafe((id->prefix->binding));
        if (! b)
          return XML_ERROR_UNBOUND_PREFIX;

        const XML_Char *bUri = _Unsafe((const XML_Char *)&_Const *b->uri);
        if (_Unsafe((! poolAppendChars(&_Mut parser->m_tempPool, bUri, b->uriLen))))
          return XML_ERROR_NO_MEMORY;

        _Unsafe sip24_update(_Unsafe(&sip_state), bUri, b->uriLen * sizeof(XML_Char));

        while (_Unsafe((*s++ != XML_T(ASCII_COLON))))
          ;

        _Unsafe sip24_update(_Unsafe(&sip_state), s, keylen(s) * sizeof(XML_Char));

        if (s == nullptr)
          return XML_ERROR_UNEXPECTED_STATE;
        {
          const size_t len = _Unsafe(xcslen(s)) + /*null terminator*/ 1;
          if (! poolAppendChars(&_Mut parser->m_tempPool, s, len))
            return XML_ERROR_NO_MEMORY;
        }

        const unsigned long uriHash = (unsigned long)_Unsafe(sip24_final(_Unsafe(&sip_state)));

        { /* Check hash table for duplicate of expanded name (uriName).
             Derived from code in lookup(parser, HASH_TABLE *_Borrow table, ...).
          */
          unsigned char step = 0;
          unsigned long mask = nsAttsSize - 1;
          j = (unsigned int)(uriHash & mask); /* index into hash table */
          while (parser->m_nsAtts[j].version == version) {
            /* for speed we compare stored hash values first */
            if (uriHash == parser->m_nsAtts[j].hash) {
              const XML_Char *s1 = poolStart(&_Mut parser->m_tempPool);
              const XML_Char *s2 = parser->m_nsAtts[j].uriName;
              /* s1 is null terminated, but not s2 */
              _Unsafe for (; *s1 == *s2 && *s1 != 0; s1++, s2++)
                ;
              if (_Unsafe((*s1 == 0)))
                return XML_ERROR_DUPLICATE_ATTRIBUTE;
            }
            if (! step)
              step = PROBE_STEP(uriHash, mask, parser->m_nsAttsPower);
            j < step ? (j += nsAttsSize - step) : (j -= step);
          }
        }

        if (parser->m_ns_triplets) { /* append namespace separator and prefix */
          _Unsafe parser->m_tempPool.ptr[-1] = parser->m_namespaceSeparator;
          _Unsafe s = b->prefix->name;
          if (s == nullptr)
            return XML_ERROR_UNEXPECTED_STATE;
          const size_t len = _Unsafe(xcslen(s)) + /*null terminator*/ 1;
          if (! poolAppendChars(&_Mut parser->m_tempPool, s, len))
            return XML_ERROR_NO_MEMORY;
        }

        /* store expanded name in attribute list */
        s = poolStart(&_Mut parser->m_tempPool);
        poolFinish(&_Mut parser->m_tempPool);
        appAtts[i] = s;

        /* fill empty slot with new version, uriName and hash value */
        parser->m_nsAtts[j].version = version;
        parser->m_nsAtts[j].hash = uriHash;
        parser->m_nsAtts[j].uriName = s;

        nPrefixes--;
        if (! nPrefixes) {
          i += 2;
          break;
        }
      } else                     /* not prefixed */
        _Unsafe { ((XML_Char *)s)[-1] = 0; } /* clear flag */
    }
  }
  /* clear flags for the remaining attributes */
  for (; i < attIndex; i += 2)
    _Unsafe { ((XML_Char *)(appAtts[i]))[-1] = 0; }
  _Unsafe for (binding = *bindingsPtr; binding; binding = binding->nextTagBinding)
    binding->attId->name[-1] = 0;

  if (! parser->m_ns)
    return XML_ERROR_NONE;

  /* expand the element type name */
  if (_Unsafe((elementType->prefix))) {
    _Unsafe binding = elementType->prefix->binding;
    if (! binding)
      return XML_ERROR_UNBOUND_PREFIX;
    localPart = tagNamePtr->str;
    while (_Unsafe((*localPart++ != XML_T(ASCII_COLON))))
      ;
  } else if (dtd->defaultPrefix.binding) {
    binding = dtd->defaultPrefix.binding;
    localPart = tagNamePtr->str;
  } else
    return XML_ERROR_NONE;
  size_t prefixLen = 0;
  if (_Unsafe((parser->m_ns_triplets && binding->prefix->name)))
    prefixLen = _Unsafe(xcslen(binding->prefix->name)) + /*null terminator*/ 1;
  tagNamePtr->localPart = localPart;
  _Unsafe tagNamePtr->uriLen = binding->uriLen;
  _Unsafe tagNamePtr->prefix = binding->prefix->name;
  tagNamePtr->prefixLen = prefixLen;

  const size_t localPartLen = _Unsafe(xcslen(localPart)) + /*null terminator*/ 1;

  /* Detect and prevent integer overflow */
  if (_Unsafe((binding->uriLen > SIZE_MAX - prefixLen
      || localPartLen > SIZE_MAX - (binding->uriLen + prefixLen)))) {
    return XML_ERROR_NO_MEMORY;
  }

  const size_t totalLen = _Unsafe((localPartLen + binding->uriLen + prefixLen));
  if (_Unsafe((totalLen > binding->uriAlloc))) {
    /* Detect and prevent integer overflow */
    if (totalLen > SIZE_MAX - EXPAND_SPARE
        || totalLen + EXPAND_SPARE > SIZE_MAX / sizeof(XML_Char)) {
      return XML_ERROR_NO_MEMORY;
    }

    _Unsafe uri = MALLOC(parser, (totalLen + EXPAND_SPARE) * sizeof(XML_Char));
    if (! uri)
      return XML_ERROR_NO_MEMORY;
    _Unsafe binding->uriAlloc = totalLen + EXPAND_SPARE;
    XML_Char *oldUri = _Unsafe((XML_Char *)&_Mut *binding->uri);
    _Unsafe memcpy(uri, oldUri, binding->uriLen * sizeof(XML_Char));
    _Unsafe for (TAG *p = parser->m_tagStack; p; p = p->parent)
      if (p->name.str == oldUri)
        p->name.str = uri;
    _Unsafe FREE_ARRAY(parser, binding->uri, XML_Char);
    _Unsafe binding->uri = __take_array_from_raw(uri);
  }
  /* if m_namespaceSeparator != '\0' then uri includes it already */
  uri = _Unsafe((XML_Char *)&_Mut *binding->uri + binding->uriLen);
  /* Detect and prevent integer overflow */
  if (localPartLen > SIZE_MAX / sizeof(XML_Char)) {
    return XML_ERROR_NO_MEMORY;
  }
  _Unsafe memcpy(uri, localPart, localPartLen * sizeof(XML_Char));
  /* we always have a namespace separator between localPart and prefix */
  if (prefixLen) {
    uri += localPartLen - 1;
    _Unsafe *uri = parser->m_namespaceSeparator; /* replace null terminator */
    _Unsafe memcpy(uri + 1, binding->prefix->name, prefixLen * sizeof(XML_Char));
  }
  XML_Char *bindingUriView = _Unsafe((XML_Char *)&_Mut *binding->uri);
  if (bindingUriView == nullptr)
    return XML_ERROR_UNEXPECTED_STATE;
  tagNamePtr->str = bindingUriView;
  return XML_ERROR_NONE;
}

_Safe static XML_Bool
is_rfc3986_uri_char(XML_Char candidate) {
  // For the RFC 3986 ANBF grammar see
  // https://datatracker.ietf.org/doc/html/rfc3986#appendix-A

  switch (candidate) {
  // From rule "ALPHA" (uppercase half)
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':

  // From rule "ALPHA" (lowercase half)
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':

  // From rule "DIGIT"
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':

  // From rule "pct-encoded"
  case '%':

  // From rule "unreserved"
  case '-':
  case '.':
  case '_':
  case '~':

  // From rule "gen-delims"
  case ':':
  case '/':
  case '?':
  case '#':
  case '[':
  case ']':
  case '@':

  // From rule "sub-delims"
  case '!':
  case '$':
  case '&':
  case '\'':
  case '(':
  case ')':
  case '*':
  case '+':
  case ',':
  case ';':
  case '=':
    return XML_TRUE;

  default:
    return XML_FALSE;
  }
}

/* addBinding() overwrites the value of prefix->binding without checking.
   Therefore one must keep track of the old value outside of addBinding().
*/
_Safe static enum XML_Error
addBinding(XML_Parser _Borrow parser, PREFIX *_Nonnull prefix, const ATTRIBUTE_ID *_Nullable attId,
           const XML_Char *uri, BINDING **_Nonnull bindingsPtr) {
  // "http://www.w3.org/XML/1998/namespace"
  static const XML_Char xmlNamespace[]
      = {ASCII_h,      ASCII_t,     ASCII_t,     ASCII_p,      ASCII_COLON,
         ASCII_SLASH,  ASCII_SLASH, ASCII_w,     ASCII_w,      ASCII_w,
         ASCII_PERIOD, ASCII_w,     ASCII_3,     ASCII_PERIOD, ASCII_o,
         ASCII_r,      ASCII_g,     ASCII_SLASH, ASCII_X,      ASCII_M,
         ASCII_L,      ASCII_SLASH, ASCII_1,     ASCII_9,      ASCII_9,
         ASCII_8,      ASCII_SLASH, ASCII_n,     ASCII_a,      ASCII_m,
         ASCII_e,      ASCII_s,     ASCII_p,     ASCII_a,      ASCII_c,
         ASCII_e,      '\0'};
  static const size_t xmlLen = sizeof(xmlNamespace) / sizeof(XML_Char) - 1;
  // "http://www.w3.org/2000/xmlns/"
  static const XML_Char xmlnsNamespace[]
      = {ASCII_h,     ASCII_t,      ASCII_t, ASCII_p, ASCII_COLON,  ASCII_SLASH,
         ASCII_SLASH, ASCII_w,      ASCII_w, ASCII_w, ASCII_PERIOD, ASCII_w,
         ASCII_3,     ASCII_PERIOD, ASCII_o, ASCII_r, ASCII_g,      ASCII_SLASH,
         ASCII_2,     ASCII_0,      ASCII_0, ASCII_0, ASCII_SLASH,  ASCII_x,
         ASCII_m,     ASCII_l,      ASCII_n, ASCII_s, ASCII_SLASH,  '\0'};
  static const size_t xmlnsLen = sizeof(xmlnsNamespace) / sizeof(XML_Char) - 1;

  XML_Bool mustBeXML = XML_FALSE;
  XML_Bool isXML = XML_TRUE;
  XML_Bool isXMLNS = XML_TRUE;

  BINDING *b;
  size_t len;

  /* empty URI is only valid for default namespace per XML NS 1.0 (not 1.1) */
  if (_Unsafe((*uri == XML_T('\0') && prefix->name)))
    return XML_ERROR_UNDECLARING_PREFIX;

  if (_Unsafe((prefix->name && prefix->name[0] == XML_T(ASCII_x)
      && prefix->name[1] == XML_T(ASCII_m)
      && prefix->name[2] == XML_T(ASCII_l)))) {
    /* Not allowed to bind xmlns */
    if (_Unsafe((prefix->name[3] == XML_T(ASCII_n) && prefix->name[4] == XML_T(ASCII_s)
        && prefix->name[5] == XML_T('\0'))))
      return XML_ERROR_RESERVED_PREFIX_XMLNS;

    if (_Unsafe((prefix->name[3] == XML_T('\0'))))
      mustBeXML = XML_TRUE;
  }

  _Unsafe for (len = 0; uri[len]; len++) {
    /* Detect and prevent integer overflow */
    if (len == SIZE_MAX) {
      return XML_ERROR_NO_MEMORY;
    }
    if (isXML && (len > xmlLen || uri[len] != xmlNamespace[len]))
      isXML = XML_FALSE;

    if (! mustBeXML && isXMLNS
        && (len > xmlnsLen || uri[len] != xmlnsNamespace[len]))
      isXMLNS = XML_FALSE;

    // NOTE: While Expat does not validate namespace URIs against RFC 3986
    //       today (and is not REQUIRED to do so with regard to the XML 1.0
    //       namespaces specification) we have to at least make sure, that
    //       the application on top of Expat (that is likely splitting expanded
    //       element names ("qualified names") of form
    //       "[uri sep] local [sep prefix] '\0'" back into 1, 2 or 3 pieces
    //       in its element handler code) cannot be confused by an attacker
    //       putting additional namespace separator characters into namespace
    //       declarations.  That would be ambiguous and not to be expected.
    //
    //       While the HTML API docs of function XML_ParserCreateNS have been
    //       advising against use of a namespace separator character that can
    //       appear in a URI for >20 years now, some widespread applications
    //       are using URI characters (':' (colon) in particular) for a
    //       namespace separator, in practice.  To keep these applications
    //       functional, we only reject namespaces URIs containing the
    //       application-chosen namespace separator if the chosen separator
    //       is a non-URI character with regard to RFC 3986.
    if (parser->m_ns && (uri[len] == parser->m_namespaceSeparator)
        && ! is_rfc3986_uri_char(uri[len])) {
      return XML_ERROR_SYNTAX;
    }
  }
  isXML = isXML && len == xmlLen;
  isXMLNS = isXMLNS && len == xmlnsLen;

  if (mustBeXML != isXML)
    return mustBeXML ? XML_ERROR_RESERVED_PREFIX_XML
                     : XML_ERROR_RESERVED_NAMESPACE_URI;

  if (isXMLNS)
    return XML_ERROR_RESERVED_NAMESPACE_URI;

  if (parser->m_namespaceSeparator) {
    /* Detect and prevent integer overflow */
    if (len == SIZE_MAX) {
      return XML_ERROR_NO_MEMORY;
    }
    len++;
  }
  if (parser->m_freeBindingList) {
    b = parser->m_freeBindingList;
    if (_Unsafe((len > b->uriAlloc))) {
      /* Detect and prevent integer overflow */
      if (len > SIZE_MAX - EXPAND_SPARE
          || len + EXPAND_SPARE > SIZE_MAX / sizeof(XML_Char)) {
        return XML_ERROR_NO_MEMORY;
      }

      XML_Bool uriOk = XML_FALSE;
      _Unsafe REALLOC_ARRAY(parser, b->uri, XML_Char,
                            sizeof(XML_Char) * (len + EXPAND_SPARE), uriOk);
      if (! uriOk)
        return XML_ERROR_NO_MEMORY;
      _Unsafe b->uriAlloc = len + EXPAND_SPARE;
    }
    _Unsafe parser->m_freeBindingList = b->nextTagBinding;
  } else {
    _Unsafe b = MALLOC(parser, sizeof(BINDING));
    if (! b)
      return XML_ERROR_NO_MEMORY;

    /* Detect and prevent integer overflow */
    if (len > SIZE_MAX - EXPAND_SPARE
        || len + EXPAND_SPARE > SIZE_MAX / sizeof(XML_Char)) {
      _Unsafe FREE(parser, b);
      return XML_ERROR_NO_MEMORY;
    }

    XML_Char *newUri = _Unsafe((XML_Char *)(MALLOC(parser, sizeof(XML_Char) * (len + EXPAND_SPARE))));
    if (! newUri) {
      _Unsafe FREE(parser, b);
      return XML_ERROR_NO_MEMORY;
    }
    _Unsafe b->uri = __take_array_from_raw(newUri);
    _Unsafe b->uriAlloc = len + EXPAND_SPARE;
  }
  _Unsafe b->uriLen = len;
  _Unsafe memcpy(_Unsafe((XML_Char *)&_Mut *b->uri), uri, len * sizeof(XML_Char));
  if (parser->m_namespaceSeparator)
    _Unsafe b->uri[len - 1] = parser->m_namespaceSeparator;
  _Unsafe b->prefix = prefix;
  _Unsafe b->attId = attId;
  _Unsafe b->prevPrefixBinding = prefix->binding;
  /* nullptr binding when default namespace undeclared */
  if (_Unsafe((*uri == XML_T('\0') && prefix == _Unsafe(_Unsafe(&parser->m_dtd->defaultPrefix)))))
    _Unsafe prefix->binding = nullptr;
  else
    _Unsafe prefix->binding = b;
  _Unsafe b->nextTagBinding = *bindingsPtr;
  _Unsafe *bindingsPtr = b;
  /* if attId == nullptr then we are not starting a namespace scope */
  if (attId && parser->m_startNamespaceDeclHandler) {
    beforeHandler(parser);
    _Unsafe(parser->m_startNamespaceDeclHandler(parser->m_handlerArg, prefix->name,
                                        prefix->binding ? uri : 0));
    afterHandler(parser);
  }
  return XML_ERROR_NONE;
}

/* The idea here is to avoid using stack for each CDATA section when
   the whole file is parsed with one call.
*/
_Safe static enum XML_Error PTRCALL
cdataSectionProcessor(XML_Parser _Borrow parser, const char *start, const char *end,
                      const char **_Nonnull endPtr) {
  __auto_type bsc_h18 = parser->m_encoding;
  __auto_type bsc_h19 = parser->m_parsingStatus.finalBuffer;
  enum XML_Error result = doCdataSection(
      parser, bsc_h18, _Unsafe(&start), end, endPtr,
      (XML_Bool)! bsc_h19, XML_ACCOUNT_DIRECT);
  if (result != XML_ERROR_NONE)
    return result;
  if (start) {
    if (parser->m_parentParser) { /* we are parsing an external entity */
      parser->m_processor = externalEntityContentProcessor;
      return externalEntityContentProcessor(parser, start, end, endPtr);
    } else {
      parser->m_processor = contentProcessor;
      return contentProcessor(parser, start, end, endPtr);
    }
  }
  return result;
}

/* startPtr gets set to non-null if the section is closed, and to null if
   the section is not yet closed.
*/
_Safe static enum XML_Error
doCdataSection(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char **_Nonnull startPtr,
               const char *end, const char **_Nonnull nextPtr, XML_Bool haveMore,
               enum XML_Account account) {
  const char *s = _Unsafe((*startPtr));
  const char **eventPP;
  const char **eventEndPP;
  if (enc == parser->m_encoding) {
    eventPP = _Unsafe(&parser->m_eventPtr);
    _Unsafe *eventPP = s;
    eventEndPP = _Unsafe(&parser->m_eventEndPtr);
  } else {
    eventPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventPtr));
    eventEndPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventEndPtr));
  }
  _Unsafe *eventPP = s;
  _Unsafe *startPtr = nullptr;

  for (;;) {
    const char *next = s; /* in case of XML_TOK_NONE or XML_TOK_PARTIAL */
    int tok = XmlCdataSectionTok(enc, s, end, _Unsafe(&next));
#if XML_GE == 1
    if (! accountingDiffTolerated(parser, tok, s, next, __LINE__, account)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#else
    UNUSED_P(account);
#endif
    _Unsafe *eventEndPP = next;
    switch (tok) {
    case XML_TOK_CDATA_SECT_CLOSE:
      if (parser->m_endCdataSectionHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_endCdataSectionHandler(parser->m_handlerArg));
        afterHandler(parser);
      }
      /* BEGIN disabled code */
      /* see comment under XML_TOK_CDATA_SECT_OPEN */
      else if ((0) && parser->m_characterDataHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_characterDataHandler(parser->m_handlerArg, (XML_Char *)&_Mut *parser->m_dataBuf,
                                       0));
        afterHandler(parser);
        /* END disabled code */
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      _Unsafe *startPtr = next;
      _Unsafe *nextPtr = next;
      if (parser->m_parsingStatus.parsing == XML_FINISHED)
        return XML_ERROR_ABORTED;
      else
        return XML_ERROR_NONE;
    case XML_TOK_DATA_NEWLINE:
      if (parser->m_characterDataHandler) {
        XML_Char c = 0xA;
        beforeHandler(parser);
        _Unsafe(parser->m_characterDataHandler(parser->m_handlerArg, _Unsafe(&c), 1));
        afterHandler(parser);
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      break;
    case XML_TOK_DATA_CHARS: {
      XML_CharacterDataHandler charDataHandler = parser->m_characterDataHandler;
      if (charDataHandler) {
        if (MUST_CONVERT(enc, s)) {
          for (;;) {
            ICHAR *dataPtr = _Unsafe(((ICHAR *)&_Mut *parser->m_dataBuf));
            const enum XML_Convert_Result convert_res = _Unsafe(XmlConvert(
                enc, _Unsafe(&s), next, _Unsafe(&dataPtr), (ICHAR *)parser->m_dataBufEnd));
            _Unsafe *eventEndPP = next;
            beforeHandler(parser);
            _Unsafe charDataHandler(parser->m_handlerArg, (XML_Char *)&_Mut *parser->m_dataBuf,
                            (int)(dataPtr - (ICHAR *)&_Mut *parser->m_dataBuf));
            afterHandler(parser);
            if ((convert_res == XML_CONVERT_COMPLETED)
                || (convert_res == XML_CONVERT_INPUT_INCOMPLETE))
              break;
            _Unsafe *eventPP = s;
          }
        } else {
          beforeHandler(parser);
          _Unsafe charDataHandler(parser->m_handlerArg, (const XML_Char *)s,
                          (int)((const XML_Char *)next - (const XML_Char *)s));
          afterHandler(parser);
        }
      } else if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
    } break;
    case XML_TOK_INVALID:
      _Unsafe *eventPP = next;
      return XML_ERROR_INVALID_TOKEN;
    case XML_TOK_PARTIAL_CHAR:
      if (haveMore) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_PARTIAL_CHAR;
    case XML_TOK_PARTIAL:
    case XML_TOK_NONE:
      if (haveMore) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_UNCLOSED_CDATA_SECTION;
    default:
      /* Every token returned by XmlCdataSectionTok() has its own
       * explicit case, so this default case will never be executed.
       * We retain it as a safety net and exclude it from the coverage
       * statistics.
       *
       * LCOV_EXCL_START
       */
      _Unsafe *eventPP = next;
      return XML_ERROR_UNEXPECTED_STATE;
      /* LCOV_EXCL_STOP */
    }

    switch (parser->m_parsingStatus.parsing) {
    case XML_SUSPENDED:
      _Unsafe *eventPP = next;
      _Unsafe *nextPtr = next;
      return XML_ERROR_NONE;
    case XML_FINISHED:
      _Unsafe *eventPP = next;
      return XML_ERROR_ABORTED;
    case XML_PARSING:
      if (parser->m_reenter) {
        return XML_ERROR_UNEXPECTED_STATE; // LCOV_EXCL_LINE
      }
      EXPAT_FALLTHROUGH;
    default:;
      _Unsafe *eventPP = s = next;
    }
  }
  /* not reached */
}

#ifdef XML_DTD

/* The idea here is to avoid using stack for each IGNORE section when
   the whole file is parsed with one call.
*/
_Safe static enum XML_Error PTRCALL
ignoreSectionProcessor(XML_Parser _Borrow parser, const char *start, const char *end,
                       const char **_Nonnull endPtr) {
  __auto_type bsc_h20 = parser->m_encoding;
  __auto_type bsc_h21 = parser->m_parsingStatus.finalBuffer;
  enum XML_Error result
      = doIgnoreSection(parser, bsc_h20, _Unsafe(&start), end, endPtr,
                        (XML_Bool)! bsc_h21);
  if (result != XML_ERROR_NONE)
    return result;
  if (start) {
    parser->m_processor = prologProcessor;
    return prologProcessor(parser, start, end, endPtr);
  }
  return result;
}

/* startPtr gets set to non-null is the section is closed, and to null
   if the section is not yet closed.
*/
_Safe static enum XML_Error
doIgnoreSection(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char **_Nonnull startPtr,
                const char *end, const char **_Nonnull nextPtr, XML_Bool haveMore) {
  const char *next = _Unsafe((*startPtr)); /* in case of XML_TOK_NONE or XML_TOK_PARTIAL */
  int tok;
  const char *s = _Unsafe((*startPtr));
  const char **eventPP;
  const char **eventEndPP;
  if (enc == parser->m_encoding) {
    eventPP = _Unsafe(&parser->m_eventPtr);
    _Unsafe *eventPP = s;
    eventEndPP = _Unsafe(&parser->m_eventEndPtr);
  } else {
    /* It's not entirely clear, but it seems the following two lines
     * of code cannot be executed.  The only occasions on which 'enc'
     * is not 'encoding' are when this function is called
     * from the internal entity processing, and IGNORE sections are an
     * error in internal entities.
     *
     * Since it really isn't clear that this is true, we keep the code
     * and just remove it from our coverage tests.
     *
     * LCOV_EXCL_START
     */
    eventPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventPtr));
    eventEndPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventEndPtr));
    /* LCOV_EXCL_STOP */
  }
  _Unsafe *eventPP = s;
  _Unsafe *startPtr = nullptr;
  tok = XmlIgnoreSectionTok(enc, s, end, _Unsafe(&next));
#  if XML_GE == 1
  if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                XML_ACCOUNT_DIRECT)) {
    accountingOnAbort(parser);
    return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
  }
#  endif
  _Unsafe *eventEndPP = next;
  switch (tok) {
  case XML_TOK_IGNORE_SECT:
    if (parser->m_defaultHandler)
      reportDefault(parser, enc, s, next);
    _Unsafe *startPtr = next;
    _Unsafe *nextPtr = next;
    if (parser->m_parsingStatus.parsing == XML_FINISHED)
      return XML_ERROR_ABORTED;
    else
      return XML_ERROR_NONE;
  case XML_TOK_INVALID:
    _Unsafe *eventPP = next;
    return XML_ERROR_INVALID_TOKEN;
  case XML_TOK_PARTIAL_CHAR:
    if (haveMore) {
      _Unsafe *nextPtr = s;
      return XML_ERROR_NONE;
    }
    return XML_ERROR_PARTIAL_CHAR;
  case XML_TOK_PARTIAL:
  case XML_TOK_NONE:
    if (haveMore) {
      _Unsafe *nextPtr = s;
      return XML_ERROR_NONE;
    }
    return XML_ERROR_SYNTAX; /* XML_ERROR_UNCLOSED_IGNORE_SECTION */
  default:
    /* All of the tokens that XmlIgnoreSectionTok() returns have
     * explicit cases to handle them, so this default case is never
     * executed.  We keep it as a safety net anyway, and remove it
     * from our test coverage statistics.
     *
     * LCOV_EXCL_START
     */
    _Unsafe *eventPP = next;
    return XML_ERROR_UNEXPECTED_STATE;
    /* LCOV_EXCL_STOP */
  }
  /* not reached */
}

#endif /* XML_DTD */

_Safe static enum XML_Error
initializeEncoding(XML_Parser _Borrow parser) {
  const char *s;
#ifdef XML_UNICODE
  char encodingBuf[128];
  /* See comments about `protocolEncodingName` in parserInit() */
  if (! parser->m_protocolEncodingName)
    s = nullptr;
  else {
    int i;
    for (i = 0; parser->m_protocolEncodingName[i]; i++) {
      if (i == sizeof(encodingBuf) - 1
          || (parser->m_protocolEncodingName[i] & ~0x7f) != 0) {
        encodingBuf[0] = '\0';
        break;
      }
      encodingBuf[i] = (char)parser->m_protocolEncodingName[i];
    }
    encodingBuf[i] = '\0';
    s = encodingBuf;
  }
#else
  s = (parser->m_protocolEncodingName == nullptr)
          ? nullptr
          : _Unsafe((const char *)&_Const *parser->m_protocolEncodingName);
#endif
  if (_Unsafe((parser->m_ns ? XmlInitEncodingNS : XmlInitEncoding)(
          _Unsafe(&parser->m_initEncoding), _Unsafe(&parser->m_encoding), s)))
    return XML_ERROR_NONE;
  const XML_Char *protocolEncodingName
      = (parser->m_protocolEncodingName == nullptr)
            ? nullptr
            : _Unsafe((const XML_Char *)&_Const *parser->m_protocolEncodingName);
  return handleUnknownEncoding(parser, protocolEncodingName);
}

_Safe static enum XML_Error
processXmlDecl(XML_Parser _Borrow parser, int isGeneralTextEntity, const char *s,
               const char *next) {
  const char *encodingName = nullptr;
  const XML_Char *storedEncName = nullptr;
  const ENCODING *newEncoding = nullptr;
  const char *version = nullptr;
  const char *versionend = nullptr;
  const XML_Char *storedversion = nullptr;
  int standalone = -1;

#if XML_GE == 1
  if (! accountingDiffTolerated(parser, XML_TOK_XML_DECL, s, next, __LINE__,
                                XML_ACCOUNT_DIRECT)) {
    accountingOnAbort(parser);
    return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
  }
#endif

  if (! _Unsafe((parser->m_ns ? XmlParseXmlDeclNS : XmlParseXmlDecl)(
          isGeneralTextEntity, parser->m_encoding, s, next, _Unsafe(&parser->m_eventPtr),
          _Unsafe(&version), _Unsafe(&versionend), _Unsafe(&encodingName), _Unsafe(&newEncoding), _Unsafe(&standalone)))) {
    if (isGeneralTextEntity)
      return XML_ERROR_TEXT_DECL;
    else
      return XML_ERROR_XML_DECL;
  }
  if (! isGeneralTextEntity && standalone == 1) {
    _Unsafe parser->m_dtd->standalone = XML_TRUE;
#ifdef XML_DTD
    if (parser->m_paramEntityParsing
        == XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE)
      parser->m_paramEntityParsing = XML_PARAM_ENTITY_PARSING_NEVER;
#endif /* XML_DTD */
  }
  if (parser->m_xmlDeclHandler) {
    if (encodingName != nullptr) {
      storedEncName = poolStoreString(
          &_Mut parser->m_temp2Pool, parser->m_encoding, encodingName,
          encodingName + XmlNameLength(parser->m_encoding, encodingName));
      if (! storedEncName)
        return XML_ERROR_NO_MEMORY;
      poolFinish(&_Mut parser->m_temp2Pool);
    }
    if (version) {
      storedversion
          = poolStoreString(&_Mut parser->m_temp2Pool, parser->m_encoding, version,
                            versionend - _Unsafe(parser->m_encoding->minBytesPerChar));
      if (! storedversion)
        return XML_ERROR_NO_MEMORY;
    }
    beforeHandler(parser);
    _Unsafe(parser->m_xmlDeclHandler(parser->m_handlerArg, storedversion, storedEncName,
                             standalone));
    afterHandler(parser);
  } else if (parser->m_defaultHandler)
    { __auto_type bsc_h22 = parser->m_encoding;
    reportDefault(parser, bsc_h22, s, next); }
  if (parser->m_protocolEncodingName == nullptr) {
    if (newEncoding) {
      /* Check that the specified encoding does not conflict with what
       * the parser has already deduced.  Do we have the same number
       * of bytes in the smallest representation of a character?  If
       * this is UTF-16, is it the same endianness?
       */
      if (_Unsafe(newEncoding->minBytesPerChar) != _Unsafe(parser->m_encoding->minBytesPerChar)
          || (_Unsafe(newEncoding->minBytesPerChar) == 2
              && newEncoding != parser->m_encoding)) {
        parser->m_eventPtr = encodingName;
        return XML_ERROR_INCORRECT_ENCODING;
      }
      parser->m_encoding = newEncoding;
    } else if (encodingName) {
      enum XML_Error result;
      if (! storedEncName) {
        storedEncName = poolStoreString(
            &_Mut parser->m_temp2Pool, parser->m_encoding, encodingName,
            encodingName + XmlNameLength(parser->m_encoding, encodingName));
        if (! storedEncName)
          return XML_ERROR_NO_MEMORY;
      }
      result = handleUnknownEncoding(parser, storedEncName);
      poolClear(&_Mut parser->m_temp2Pool);
      if (result == XML_ERROR_UNKNOWN_ENCODING)
        parser->m_eventPtr = encodingName;
      return result;
    }
  }

  if (storedEncName || storedversion)
    poolClear(&_Mut parser->m_temp2Pool);

  return XML_ERROR_NONE;
}

_Safe static enum XML_Error
handleUnknownEncoding(XML_Parser _Borrow parser, const XML_Char *_Nullable encodingName) {
  if (parser->m_unknownEncodingHandler) {
    XML_Encoding info;
    int i;
    for (i = 0; i < 256; i++)
      info.map[i] = -1;
    info.convert = nullptr;
    info.data = nullptr;
    info.release = nullptr;
    beforeHandler(parser);
    const int status = _Unsafe(parser->m_unknownEncodingHandler(
        parser->m_unknownEncodingHandlerData, encodingName, _Unsafe(&info)));
    afterHandler(parser);

    parser->m_unknownEncodingRelease = info.release;
    parser->m_unknownEncodingData = info.data;

    if (status) {
      ENCODING *enc;
      parser->m_unknownEncodingMem = TAKE_ARRAY(
          char, MALLOC(parser, (size_t)_Unsafe(XmlSizeOfUnknownEncoding())));
      if (! parser->m_unknownEncodingMem) {
        if (parser->m_unknownEncodingRelease)
          callUnknownEncodingRelease(parser);
        else
          parser->m_unknownEncodingData = nullptr;
        return XML_ERROR_NO_MEMORY;
      }
      parser->m_unknownEncodingConvert = info.convert;
      enc = _Unsafe((parser->m_ns ? XmlInitUnknownEncodingNS : XmlInitUnknownEncoding)(
          _Unsafe((void *)&_Mut *parser->m_unknownEncodingMem), info.map,
          info.convert ? callUnknownEncodingConvert : nullptr,
          _Unsafe((void *)parser)));
      if (enc) {
        parser->m_encoding = enc;
        return XML_ERROR_NONE;
      }
      parser->m_unknownEncodingConvert = nullptr;
    }
    if (parser->m_unknownEncodingRelease != nullptr)
      callUnknownEncodingRelease(parser);
    else
      parser->m_unknownEncodingData = nullptr;
  }
  return XML_ERROR_UNKNOWN_ENCODING;
}

_Safe static enum XML_Error PTRCALL
prologInitProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                    const char **_Nonnull nextPtr) {
  enum XML_Error result = initializeEncoding(parser);
  if (result != XML_ERROR_NONE)
    return result;
  parser->m_processor = prologProcessor;
  return prologProcessor(parser, s, end, nextPtr);
}

#ifdef XML_DTD

_Safe static enum XML_Error PTRCALL
externalParEntInitProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                            const char **_Nonnull nextPtr) {
  enum XML_Error result = initializeEncoding(parser);
  if (result != XML_ERROR_NONE)
    return result;

  /* we know now that XML_Parse(Buffer) has been called,
     so we consider the external parameter entity read */
  _Unsafe parser->m_dtd->paramEntityRead = XML_TRUE;

  if (parser->m_prologState.inEntityValue) {
    parser->m_processor = entityValueInitProcessor;
    return entityValueInitProcessor(parser, s, end, nextPtr);
  } else {
    parser->m_processor = externalParEntProcessor;
    return externalParEntProcessor(parser, s, end, nextPtr);
  }
}

_Safe static enum XML_Error PTRCALL
entityValueInitProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                         const char **_Nonnull nextPtr) {
  int tok;
  const char *start = s;
  const char *next = start;
  parser->m_eventPtr = start;

  for (;;) {
    tok = XmlPrologTok(parser->m_encoding, start, end, _Unsafe(&next));
    /* Note: Except for XML_TOK_BOM below, these bytes are accounted later in:
             - storeEntityValue
             - processXmlDecl
    */
    parser->m_eventEndPtr = next;
    if (tok <= 0) {
      if (! parser->m_parsingStatus.finalBuffer && tok != XML_TOK_INVALID) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      switch (tok) {
      case XML_TOK_INVALID:
        return XML_ERROR_INVALID_TOKEN;
      case XML_TOK_PARTIAL:
        return XML_ERROR_UNCLOSED_TOKEN;
      case XML_TOK_PARTIAL_CHAR:
        return XML_ERROR_PARTIAL_CHAR;
      case XML_TOK_NONE: /* start == end */
      default:
        break;
      }
      /* found end of entity value - can store it now */
      __auto_type bsc_h23 = parser->m_encoding;
      return storeEntityValue(parser, bsc_h23, s, end,
                              XML_ACCOUNT_DIRECT, nullptr);
    } else if (tok == XML_TOK_XML_DECL) {
      enum XML_Error result;
      result = processXmlDecl(parser, 0, start, next);
      if (result != XML_ERROR_NONE)
        return result;
      /* At this point, m_parsingStatus.parsing cannot be XML_SUSPENDED.  For
       * that to happen, a parameter entity parsing handler must have attempted
       * to suspend the parser, which fails and raises an error.  The parser can
       * be aborted, but can't be suspended.
       */
      if (parser->m_parsingStatus.parsing == XML_FINISHED)
        return XML_ERROR_ABORTED;
      _Unsafe *nextPtr = next;
      /* stop scanning for text declaration - we found one */
      parser->m_processor = entityValueProcessor;
      return entityValueProcessor(parser, next, end, nextPtr);
    }
    /* XmlPrologTok has now set the encoding based on the BOM it found, and we
       must move s and nextPtr forward to consume the BOM.

       If we didn't, and got XML_TOK_NONE from the next XmlPrologTok call, we
       would leave the BOM in the buffer and return. On the next call to this
       function, our XmlPrologTok call would return XML_TOK_INVALID, since it
       is not valid to have multiple BOMs.
    */
    else if (tok == XML_TOK_BOM) {
#  if XML_GE == 1
      if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                    XML_ACCOUNT_DIRECT)) {
        accountingOnAbort(parser);
        return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
      }
#  endif

      _Unsafe *nextPtr = next;
      s = next;
    }
    /* If we get this token, we have the start of what might be a
       normal tag, but not a declaration (i.e. it doesn't begin with
       "<!" or "<?").  In a DTD context, that isn't legal.
    */
    else if (tok == XML_TOK_INSTANCE_START) {
      _Unsafe *nextPtr = next;
      return XML_ERROR_SYNTAX;
    }
    start = next;
    parser->m_eventPtr = start;
  }
}

_Safe static enum XML_Error PTRCALL
externalParEntProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                        const char **_Nonnull nextPtr) {
  const char *next = s;
  int tok;

  tok = XmlPrologTok(parser->m_encoding, s, end, _Unsafe(&next));
  if (tok <= 0) {
    if (! parser->m_parsingStatus.finalBuffer && tok != XML_TOK_INVALID) {
      _Unsafe *nextPtr = s;
      return XML_ERROR_NONE;
    }
    switch (tok) {
    case XML_TOK_INVALID:
      return XML_ERROR_INVALID_TOKEN;
    case XML_TOK_PARTIAL:
      return XML_ERROR_UNCLOSED_TOKEN;
    case XML_TOK_PARTIAL_CHAR:
      return XML_ERROR_PARTIAL_CHAR;
    case XML_TOK_NONE: /* start == end */
    default:
      break;
    }
  }
  /* This would cause the next stage, i.e. doProlog to be passed XML_TOK_BOM.
     However, when parsing an external subset, doProlog will not accept a BOM
     as valid, and report a syntax error, so we have to skip the BOM, and
     account for the BOM bytes.
  */
  else if (tok == XML_TOK_BOM) {
    if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                  XML_ACCOUNT_DIRECT)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }

    s = next;
    tok = XmlPrologTok(parser->m_encoding, s, end, _Unsafe(&next));
  }

  parser->m_processor = prologProcessor;
  __auto_type bsc_h24 = parser->m_encoding;
  __auto_type bsc_h25 = parser->m_parsingStatus.finalBuffer;
  return doProlog(parser, bsc_h24, s, end, tok, next, nextPtr,
                  (XML_Bool)! bsc_h25, XML_TRUE,
                  XML_ACCOUNT_DIRECT);
}

_Safe static enum XML_Error PTRCALL
entityValueProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                     const char **_Nonnull nextPtr) {
  const char *start = s;
  const char *next = s;
  const ENCODING *_Nonnull enc = parser->m_encoding;
  int tok;

  for (;;) {
    tok = XmlPrologTok(enc, start, end, _Unsafe(&next));
    /* Note: These bytes are accounted later in:
             - storeEntityValue
    */
    if (tok <= 0) {
      if (! parser->m_parsingStatus.finalBuffer && tok != XML_TOK_INVALID) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      switch (tok) {
      case XML_TOK_INVALID:
        return XML_ERROR_INVALID_TOKEN;
      case XML_TOK_PARTIAL:
        return XML_ERROR_UNCLOSED_TOKEN;
      case XML_TOK_PARTIAL_CHAR:
        return XML_ERROR_PARTIAL_CHAR;
      case XML_TOK_NONE: /* start == end */
      default:
        break;
      }
      /* found end of entity value - can store it now */
      return storeEntityValue(parser, enc, s, end, XML_ACCOUNT_DIRECT, nullptr);
    }
    /* If we get this token, we have the start of what might be a
       normal tag, but not a declaration (i.e. it doesn't begin with
       "<!" or "<?").  In a DTD context, that isn't legal.
    */
    else if (tok == XML_TOK_INSTANCE_START) {
      _Unsafe *nextPtr = next;
      return XML_ERROR_SYNTAX;
    }

    start = next;
  }
}

#endif /* XML_DTD */

_Safe static enum XML_Error PTRCALL
prologProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                const char **_Nonnull nextPtr) {
  const char *next = s;
  int tok = XmlPrologTok(parser->m_encoding, s, end, _Unsafe(&next));
  __auto_type bsc_h26 = parser->m_encoding;
  __auto_type bsc_h27 = parser->m_parsingStatus.finalBuffer;
  return doProlog(parser, bsc_h26, s, end, tok, next, nextPtr,
                  (XML_Bool)! bsc_h27, XML_TRUE,
                  XML_ACCOUNT_DIRECT);
}

_Safe static enum XML_Error
doProlog(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char *s, const char *end,
         int tok, const char *next, const char **_Nonnull nextPtr, XML_Bool haveMore,
         XML_Bool allowClosingDoctype, enum XML_Account account) {
#ifdef XML_DTD
  static const XML_Char externalSubsetName[] = {ASCII_HASH, '\0'};
#endif /* XML_DTD */
  static const XML_Char atypeCDATA[]
      = {ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0'};
  static const XML_Char atypeID[] = {ASCII_I, ASCII_D, '\0'};
  static const XML_Char atypeIDREF[]
      = {ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, '\0'};
  static const XML_Char atypeIDREFS[]
      = {ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, ASCII_S, '\0'};
  static const XML_Char atypeENTITY[]
      = {ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T, ASCII_Y, '\0'};
  static const XML_Char atypeENTITIES[]
      = {ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T,
         ASCII_I, ASCII_E, ASCII_S, '\0'};
  static const XML_Char atypeNMTOKEN[]
      = {ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K, ASCII_E, ASCII_N, '\0'};
  static const XML_Char atypeNMTOKENS[]
      = {ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K,
         ASCII_E, ASCII_N, ASCII_S, '\0'};
  static const XML_Char notationPrefix[]
      = {ASCII_N, ASCII_O, ASCII_T, ASCII_A,      ASCII_T,
         ASCII_I, ASCII_O, ASCII_N, ASCII_LPAREN, '\0'};
  static const XML_Char enumValueSep[] = {ASCII_PIPE, '\0'};
  static const XML_Char enumValueStart[] = {ASCII_LPAREN, '\0'};

#ifndef XML_DTD
  UNUSED_P(account);
#endif

  /* save one level of indirection */
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return XML_ERROR_UNEXPECTED_STATE;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);

  const char **eventPP;
  const char **eventEndPP;
  enum XML_Content_Quant quant;

  if (enc == parser->m_encoding) {
    eventPP = _Unsafe(&parser->m_eventPtr);
    eventEndPP = _Unsafe(&parser->m_eventEndPtr);
  } else {
    eventPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventPtr));
    eventEndPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventEndPtr));
  }

  for (;;) {
    int role;
    XML_Bool handleDefault = XML_TRUE;
    _Unsafe *eventPP = s;
    _Unsafe *eventEndPP = next;
    if (tok <= 0) {
      if (haveMore && tok != XML_TOK_INVALID) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      switch (tok) {
      case XML_TOK_INVALID:
        _Unsafe *eventPP = next;
        return XML_ERROR_INVALID_TOKEN;
      case XML_TOK_PARTIAL:
        return XML_ERROR_UNCLOSED_TOKEN;
      case XML_TOK_PARTIAL_CHAR:
        return XML_ERROR_PARTIAL_CHAR;
      case -XML_TOK_PROLOG_S:
        tok = -tok;
        break;
      case XML_TOK_NONE:
#ifdef XML_DTD
        /* for internal PE NOT referenced between declarations */
        if (_Unsafe((enc != parser->m_encoding
            && ! parser->m_openInternalEntities->betweenDecl))) {
          _Unsafe *nextPtr = s;
          return XML_ERROR_NONE;
        }
        /* WFC: PE Between Declarations - must check that PE contains
           complete markup, not only for external PEs, but also for
           internal PEs if the reference occurs between declarations.
        */
        if (parser->m_isParamEntity || enc != parser->m_encoding) {
          if (XmlTokenRole(&_Mut parser->m_prologState, XML_TOK_NONE, end, end, enc)
              == XML_ROLE_ERROR)
            return XML_ERROR_INCOMPLETE_PE;
          _Unsafe *nextPtr = s;
          return XML_ERROR_NONE;
        }
#endif /* XML_DTD */
        return XML_ERROR_NO_ELEMENTS;
      default:
        tok = -tok;
        next = end;
        break;
      }
    }
    role = XmlTokenRole(&_Mut parser->m_prologState, tok, s, next, enc);
#if XML_GE == 1
    switch (role) {
    case XML_ROLE_INSTANCE_START: // bytes accounted in contentProcessor
    case XML_ROLE_XML_DECL:       // bytes accounted in processXmlDecl
#  ifdef XML_DTD
    case XML_ROLE_TEXT_DECL: // bytes accounted in processXmlDecl
#  endif
      break;
    default:
      if (! accountingDiffTolerated(parser, tok, s, next, __LINE__, account)) {
        accountingOnAbort(parser);
        return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
      }
    }
#endif
    switch (role) {
    case XML_ROLE_XML_DECL: {
      enum XML_Error result = processXmlDecl(parser, 0, s, next);
      if (result != XML_ERROR_NONE)
        return result;
      enc = parser->m_encoding;
      handleDefault = XML_FALSE;
    } break;
    case XML_ROLE_DOCTYPE_NAME:
      if (parser->m_startDoctypeDeclHandler) {
        parser->m_doctypeName
            = poolStoreString(&_Mut parser->m_tempPool, enc, s, next);
        if (! parser->m_doctypeName)
          return XML_ERROR_NO_MEMORY;
        poolFinish(&_Mut parser->m_tempPool);
        parser->m_doctypePubid = nullptr;
        handleDefault = XML_FALSE;
      }
      parser->m_doctypeSysid = nullptr; /* always initialize to nullptr */
      break;
    case XML_ROLE_DOCTYPE_INTERNAL_SUBSET:
      if (parser->m_startDoctypeDeclHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_startDoctypeDeclHandler(
            parser->m_handlerArg, parser->m_doctypeName, parser->m_doctypeSysid,
            parser->m_doctypePubid, 1));
        afterHandler(parser);
        parser->m_doctypeName = nullptr;
        poolClear(&_Mut parser->m_tempPool);
        handleDefault = XML_FALSE;
      }
      break;
#ifdef XML_DTD
    case XML_ROLE_TEXT_DECL: {
      enum XML_Error result = processXmlDecl(parser, 1, s, next);
      if (result != XML_ERROR_NONE)
        return result;
      enc = parser->m_encoding;
      handleDefault = XML_FALSE;
    } break;
#endif /* XML_DTD */
    case XML_ROLE_DOCTYPE_PUBLIC_ID:
#ifdef XML_DTD
      parser->m_useForeignDTD = XML_FALSE;
      _Unsafe parser->m_declEntity = (ENTITY *)lookup(
          parser, &_Mut dtd->paramEntities, externalSubsetName, sizeof(ENTITY));
      if (! parser->m_declEntity)
        return XML_ERROR_NO_MEMORY;
#endif /* XML_DTD */
      dtd->hasParamEntityRefs = XML_TRUE;
      if (parser->m_startDoctypeDeclHandler) {
        XML_Char *pubId;
        if (! XmlIsPublicId(enc, s, next, eventPP))
          return XML_ERROR_PUBLICID;
        pubId = poolStoreString(&_Mut parser->m_tempPool, enc,
                                s + _Unsafe(enc->minBytesPerChar),
                                next - _Unsafe(enc->minBytesPerChar));
        if (! pubId)
          return XML_ERROR_NO_MEMORY;
        normalizePublicId(pubId);
        poolFinish(&_Mut parser->m_tempPool);
        parser->m_doctypePubid = pubId;
        handleDefault = XML_FALSE;
        goto alreadyChecked;
      }
      EXPAT_FALLTHROUGH;
    case XML_ROLE_ENTITY_PUBLIC_ID:
      if (! XmlIsPublicId(enc, s, next, eventPP))
        return XML_ERROR_PUBLICID;
    alreadyChecked:
      if (dtd->keepProcessing && parser->m_declEntity) {
        XML_Char *tem
            = poolStoreString(&_Mut dtd->pool, enc, s + _Unsafe(enc->minBytesPerChar),
                              next - _Unsafe(enc->minBytesPerChar));
        if (! tem)
          return XML_ERROR_NO_MEMORY;
        normalizePublicId(tem);
        _Unsafe parser->m_declEntity->publicId = tem;
        poolFinish(&_Mut dtd->pool);
        /* Don't suppress the default handler if we fell through from
         * the XML_ROLE_DOCTYPE_PUBLIC_ID case.
         */
        if (parser->m_entityDeclHandler && role == XML_ROLE_ENTITY_PUBLIC_ID)
          handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_DOCTYPE_CLOSE:
      if (allowClosingDoctype != XML_TRUE) {
        /* Must not close doctype from within expanded parameter entities */
        return XML_ERROR_INVALID_TOKEN;
      }

      if (parser->m_doctypeName) {
        beforeHandler(parser);
        _Unsafe(parser->m_startDoctypeDeclHandler(
            parser->m_handlerArg, parser->m_doctypeName, parser->m_doctypeSysid,
            parser->m_doctypePubid, 0));
        afterHandler(parser);
        poolClear(&_Mut parser->m_tempPool);
        handleDefault = XML_FALSE;
      }
      /* parser->m_doctypeSysid will be non-nullptr in the case of a previous
         XML_ROLE_DOCTYPE_SYSTEM_ID, even if parser->m_startDoctypeDeclHandler
         was not set, indicating an external subset
      */
#ifdef XML_DTD
      if (parser->m_doctypeSysid || parser->m_useForeignDTD) {
        XML_Bool hadParamEntityRefs = dtd->hasParamEntityRefs;
        dtd->hasParamEntityRefs = XML_TRUE;
        if (parser->m_paramEntityParsing
            && parser->m_externalEntityRefHandler) {
          _Unsafe ENTITY *_Nonnull entity = (ENTITY *)lookup(parser, &_Mut dtd->paramEntities,
                                            externalSubsetName, sizeof(ENTITY));
          if (! entity) {
            /* The external subset name "#" will have already been
             * inserted into the hash table at the start of the
             * external entity parsing, so no allocation will happen
             * and lookup() cannot fail.
             */
            return XML_ERROR_NO_MEMORY; /* LCOV_EXCL_LINE */
          }
          if (parser->m_useForeignDTD)
            _Unsafe entity->base = parser->m_curBase;
          dtd->paramEntityRead = XML_FALSE;
          beforeHandler(parser);
          const int status = _Unsafe(parser->m_externalEntityRefHandler(
              parser->m_externalEntityRefHandlerArg, 0, entity->base,
              entity->systemId, entity->publicId));
          afterHandler(parser);
          if (! status)
            return XML_ERROR_EXTERNAL_ENTITY_HANDLING;
          if (dtd->paramEntityRead) {
            if (! dtd->standalone && parser->m_notStandaloneHandler) {
              beforeHandler(parser);
              const int handlerStatus
                  = _Unsafe(parser->m_notStandaloneHandler(parser->m_handlerArg));
              afterHandler(parser);
              if (! handlerStatus)
                return XML_ERROR_NOT_STANDALONE;
            }
          }
          /* if we didn't read the foreign DTD then this means that there
             is no external subset and we must reset dtd->hasParamEntityRefs
          */
          else if (! parser->m_doctypeSysid)
            dtd->hasParamEntityRefs = hadParamEntityRefs;
          /* end of DTD - no need to update dtd->keepProcessing */
        }
        parser->m_useForeignDTD = XML_FALSE;
      }
#endif /* XML_DTD */
      if (parser->m_endDoctypeDeclHandler) {
        beforeHandler(parser);
        _Unsafe(parser->m_endDoctypeDeclHandler(parser->m_handlerArg));
        afterHandler(parser);
        handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_INSTANCE_START:
#ifdef XML_DTD
      /* if there is no DOCTYPE declaration then now is the
         last chance to read the foreign DTD
      */
      if (parser->m_useForeignDTD) {
        XML_Bool hadParamEntityRefs = dtd->hasParamEntityRefs;
        dtd->hasParamEntityRefs = XML_TRUE;
        if (parser->m_paramEntityParsing
            && parser->m_externalEntityRefHandler) {
          _Unsafe ENTITY *_Nonnull entity = (ENTITY *)lookup(parser, &_Mut dtd->paramEntities,
                                            externalSubsetName, sizeof(ENTITY));
          if (! entity)
            return XML_ERROR_NO_MEMORY;
          _Unsafe entity->base = parser->m_curBase;
          dtd->paramEntityRead = XML_FALSE;
          beforeHandler(parser);
          const int status = _Unsafe(parser->m_externalEntityRefHandler(
              parser->m_externalEntityRefHandlerArg, 0, entity->base,
              entity->systemId, entity->publicId));
          afterHandler(parser);
          if (! status)
            return XML_ERROR_EXTERNAL_ENTITY_HANDLING;
          if (dtd->paramEntityRead) {
            if (! dtd->standalone && parser->m_notStandaloneHandler) {
              beforeHandler(parser);
              const int handlerStatus
                  = _Unsafe(parser->m_notStandaloneHandler(parser->m_handlerArg));
              afterHandler(parser);
              if (! handlerStatus)
                return XML_ERROR_NOT_STANDALONE;
            }
          }
          /* if we didn't read the foreign DTD then this means that there
             is no external subset and we must reset dtd->hasParamEntityRefs
          */
          else
            dtd->hasParamEntityRefs = hadParamEntityRefs;
          /* end of DTD - no need to update dtd->keepProcessing */
        }
      }
#endif /* XML_DTD */
      parser->m_processor = contentProcessor;
      return contentProcessor(parser, s, end, nextPtr);
    case XML_ROLE_ATTLIST_ELEMENT_NAME:
      parser->m_declElementType = getElementType(parser, enc, s, next);
      if (! parser->m_declElementType)
        return XML_ERROR_NO_MEMORY;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_NAME:
      parser->m_declAttributeId = getAttributeId(parser, enc, s, next);
      if (! parser->m_declAttributeId)
        return XML_ERROR_NO_MEMORY;
      parser->m_declAttributeIsCdata = XML_FALSE;
      parser->m_declAttributeType = nullptr;
      parser->m_declAttributeIsId = XML_FALSE;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_CDATA:
      parser->m_declAttributeIsCdata = XML_TRUE;
      parser->m_declAttributeType = atypeCDATA;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_ID:
      parser->m_declAttributeIsId = XML_TRUE;
      parser->m_declAttributeType = atypeID;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_IDREF:
      parser->m_declAttributeType = atypeIDREF;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_IDREFS:
      parser->m_declAttributeType = atypeIDREFS;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_ENTITY:
      parser->m_declAttributeType = atypeENTITY;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_ENTITIES:
      parser->m_declAttributeType = atypeENTITIES;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_NMTOKEN:
      parser->m_declAttributeType = atypeNMTOKEN;
      goto checkAttListDeclHandler;
    case XML_ROLE_ATTRIBUTE_TYPE_NMTOKENS:
      parser->m_declAttributeType = atypeNMTOKENS;
    checkAttListDeclHandler:
      if (dtd->keepProcessing && parser->m_attlistDeclHandler)
        handleDefault = XML_FALSE;
      break;
    case XML_ROLE_ATTRIBUTE_ENUM_VALUE:
    case XML_ROLE_ATTRIBUTE_NOTATION_VALUE:
      if (dtd->keepProcessing && parser->m_attlistDeclHandler) {
        const XML_Char *prefix;
        if (parser->m_declAttributeType) {
          prefix = enumValueSep;
        } else {
          prefix = (role == XML_ROLE_ATTRIBUTE_NOTATION_VALUE ? notationPrefix
                                                              : enumValueStart);
        }
        if (! poolAppendString(&_Mut parser->m_tempPool, prefix))
          return XML_ERROR_NO_MEMORY;
        if (! poolAppend(&_Mut parser->m_tempPool, enc, s, next))
          return XML_ERROR_NO_MEMORY;
        parser->m_declAttributeType = parser->m_tempPool.start;
        handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_IMPLIED_ATTRIBUTE_VALUE:
    case XML_ROLE_REQUIRED_ATTRIBUTE_VALUE:
      if (dtd->keepProcessing) {
        if (parser->m_declElementType == nullptr
            || parser->m_declAttributeId == nullptr)
          return XML_ERROR_UNEXPECTED_STATE;
        if (! defineAttribute(parser->m_declElementType,
                              parser->m_declAttributeId,
                              parser->m_declAttributeIsCdata,
                              parser->m_declAttributeIsId, 0, parser))
          return XML_ERROR_NO_MEMORY;
        if (parser->m_attlistDeclHandler && parser->m_declAttributeType) {
          if (_Unsafe((*parser->m_declAttributeType == XML_T(ASCII_LPAREN)
              || (*parser->m_declAttributeType == XML_T(ASCII_N)
                  && parser->m_declAttributeType[1] == XML_T(ASCII_O))))) {
            /* Enumerated or Notation type */
            if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T(ASCII_RPAREN))
                || ! poolAppendChar(&_Mut parser->m_tempPool, XML_T('\0')))
              return XML_ERROR_NO_MEMORY;
            parser->m_declAttributeType = parser->m_tempPool.start;
            poolFinish(&_Mut parser->m_tempPool);
          }
          _Unsafe *eventEndPP = s;
          beforeHandler(parser);
          _Unsafe(parser->m_attlistDeclHandler(
              parser->m_handlerArg, parser->m_declElementType->name,
              parser->m_declAttributeId->name, parser->m_declAttributeType, 0,
              role == XML_ROLE_REQUIRED_ATTRIBUTE_VALUE));
          afterHandler(parser);
          handleDefault = XML_FALSE;
        }
      }
      poolClear(&_Mut parser->m_tempPool);
      break;
    case XML_ROLE_DEFAULT_ATTRIBUTE_VALUE:
    case XML_ROLE_FIXED_ATTRIBUTE_VALUE:
      if (dtd->keepProcessing) {
        const XML_Char *attVal;
        __auto_type bsc_h41 = parser->m_declAttributeIsCdata;
        enum XML_Error result = storeAttributeValue(
            parser, enc, bsc_h41,
            s + _Unsafe(enc->minBytesPerChar), next - _Unsafe(enc->minBytesPerChar), &_Mut dtd->pool,
            XML_ACCOUNT_NONE);
        if (result)
          return result;
        attVal = poolStart(&_Mut dtd->pool);
        poolFinish(&_Mut dtd->pool);
        if (parser->m_declElementType == nullptr
            || parser->m_declAttributeId == nullptr)
          return XML_ERROR_UNEXPECTED_STATE;
        /* ID attributes aren't allowed to have a default */
        if (! defineAttribute(
                parser->m_declElementType, parser->m_declAttributeId,
                parser->m_declAttributeIsCdata, XML_FALSE, attVal, parser))
          return XML_ERROR_NO_MEMORY;
        if (parser->m_attlistDeclHandler && parser->m_declAttributeType) {
          if (_Unsafe((*parser->m_declAttributeType == XML_T(ASCII_LPAREN)
              || (*parser->m_declAttributeType == XML_T(ASCII_N)
                  && parser->m_declAttributeType[1] == XML_T(ASCII_O))))) {
            /* Enumerated or Notation type */
            if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T(ASCII_RPAREN))
                || ! poolAppendChar(&_Mut parser->m_tempPool, XML_T('\0')))
              return XML_ERROR_NO_MEMORY;
            parser->m_declAttributeType = parser->m_tempPool.start;
            poolFinish(&_Mut parser->m_tempPool);
          }
          _Unsafe *eventEndPP = s;
          beforeHandler(parser);
          _Unsafe(parser->m_attlistDeclHandler(
              parser->m_handlerArg, parser->m_declElementType->name,
              parser->m_declAttributeId->name, parser->m_declAttributeType,
              attVal, role == XML_ROLE_FIXED_ATTRIBUTE_VALUE));
          afterHandler(parser);
          poolClear(&_Mut parser->m_tempPool);
          handleDefault = XML_FALSE;
        }
      }
      break;
    case XML_ROLE_ENTITY_VALUE:
      if (dtd->keepProcessing) {
#if XML_GE == 1
        // This will store the given replacement text in
        // parser->m_declEntity->textPtr.
        enum XML_Error result = callStoreEntityValue(
            parser, enc, s + _Unsafe(enc->minBytesPerChar), next - _Unsafe(enc->minBytesPerChar),
            XML_ACCOUNT_NONE);
        if (parser->m_declEntity) {
          /* Detect and prevent signed integer overflow */
          if ((size_t)poolLength(&_Mut dtd->entityValuePool) > (size_t)INT_MAX) {
            return XML_ERROR_NO_MEMORY;
          }
          _Unsafe parser->m_declEntity->textPtr = poolStart(&_Mut dtd->entityValuePool);
          _Unsafe parser->m_declEntity->textLen
              = (int)(poolLength(&_Mut dtd->entityValuePool));
          poolFinish(&_Mut dtd->entityValuePool);
          if (parser->m_entityDeclHandler) {
            _Unsafe *eventEndPP = s;
            beforeHandler(parser);
            _Unsafe(parser->m_entityDeclHandler(
                parser->m_handlerArg, parser->m_declEntity->name,
                parser->m_declEntity->is_param, parser->m_declEntity->textPtr,
                parser->m_declEntity->textLen, parser->m_curBase, 0, 0, 0));
            afterHandler(parser);
            handleDefault = XML_FALSE;
          }
        } else
          poolDiscard(&_Mut dtd->entityValuePool);
        if (result != XML_ERROR_NONE)
          return result;
#else
        // This will store "&amp;entity123;" in parser->m_declEntity->textPtr
        // to end up as "&entity123;" in the handler.
        if (parser->m_declEntity != nullptr) {
          const enum XML_Error result
              = storeSelfEntityValue(parser, parser->m_declEntity);
          if (result != XML_ERROR_NONE)
            return result;

          if (parser->m_entityDeclHandler) {
            *eventEndPP = s;
            beforeHandler(parser);
            _Unsafe(parser->m_entityDeclHandler(
                parser->m_handlerArg, parser->m_declEntity->name,
                parser->m_declEntity->is_param, parser->m_declEntity->textPtr,
                parser->m_declEntity->textLen, parser->m_curBase, 0, 0, 0));
            afterHandler(parser);
            handleDefault = XML_FALSE;
          }
        }
#endif
      }
      break;
    case XML_ROLE_DOCTYPE_SYSTEM_ID:
#ifdef XML_DTD
      parser->m_useForeignDTD = XML_FALSE;
#endif /* XML_DTD */
      dtd->hasParamEntityRefs = XML_TRUE;
      if (parser->m_startDoctypeDeclHandler) {
        parser->m_doctypeSysid = poolStoreString(&_Mut parser->m_tempPool, enc,
                                                 s + _Unsafe(enc->minBytesPerChar),
                                                 next - _Unsafe(enc->minBytesPerChar));
        if (parser->m_doctypeSysid == nullptr)
          return XML_ERROR_NO_MEMORY;
        poolFinish(&_Mut parser->m_tempPool);
        handleDefault = XML_FALSE;
      }
#ifdef XML_DTD
      else
        /* use externalSubsetName to make parser->m_doctypeSysid non-nullptr
           for the case where no parser->m_startDoctypeDeclHandler is set */
        parser->m_doctypeSysid = externalSubsetName;
#endif /* XML_DTD */
      if (! dtd->standalone
#ifdef XML_DTD
          && ! parser->m_paramEntityParsing
#endif /* XML_DTD */
          && parser->m_notStandaloneHandler) {
        beforeHandler(parser);
        const int status = _Unsafe(parser->m_notStandaloneHandler(parser->m_handlerArg));
        afterHandler(parser);
        if (! status)
          return XML_ERROR_NOT_STANDALONE;
      }
#ifndef XML_DTD
      break;
#else  /* XML_DTD */
      if (! parser->m_declEntity) {
        _Unsafe parser->m_declEntity = (ENTITY *)lookup(
            parser, &_Mut dtd->paramEntities, externalSubsetName, sizeof(ENTITY));
        if (! parser->m_declEntity)
          return XML_ERROR_NO_MEMORY;
        _Unsafe parser->m_declEntity->publicId = nullptr;
      }
#endif /* XML_DTD */
      EXPAT_FALLTHROUGH;
    case XML_ROLE_ENTITY_SYSTEM_ID:
      if (dtd->keepProcessing && parser->m_declEntity) {
        _Unsafe parser->m_declEntity->systemId
            = poolStoreString(&_Mut dtd->pool, enc, s + _Unsafe(enc->minBytesPerChar),
                              next - _Unsafe(enc->minBytesPerChar));
        if (_Unsafe((! parser->m_declEntity->systemId)))
          return XML_ERROR_NO_MEMORY;
        _Unsafe parser->m_declEntity->base = parser->m_curBase;
        poolFinish(&_Mut dtd->pool);
        /* Don't suppress the default handler if we fell through from
         * the XML_ROLE_DOCTYPE_SYSTEM_ID case.
         */
        if (parser->m_entityDeclHandler && role == XML_ROLE_ENTITY_SYSTEM_ID)
          handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_ENTITY_COMPLETE:
#if XML_GE == 0
      // This will store "&amp;entity123;" in entity->textPtr
      // to end up as "&entity123;" in the handler.
      if (parser->m_declEntity != nullptr) {
        const enum XML_Error result
            = storeSelfEntityValue(parser, parser->m_declEntity);
        if (result != XML_ERROR_NONE)
          return result;
      }
#endif
      if (dtd->keepProcessing && parser->m_declEntity
          && parser->m_entityDeclHandler) {
        _Unsafe *eventEndPP = s;
        beforeHandler(parser);
        _Unsafe(parser->m_entityDeclHandler(
            parser->m_handlerArg, parser->m_declEntity->name,
            parser->m_declEntity->is_param, 0, 0, parser->m_declEntity->base,
            parser->m_declEntity->systemId, parser->m_declEntity->publicId, 0));
        afterHandler(parser);
        handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_ENTITY_NOTATION_NAME:
      if (dtd->keepProcessing && parser->m_declEntity) {
        _Unsafe parser->m_declEntity->notation
            = poolStoreString(&_Mut dtd->pool, enc, s, next);
        if (_Unsafe((! parser->m_declEntity->notation)))
          return XML_ERROR_NO_MEMORY;
        poolFinish(&_Mut dtd->pool);
        if (parser->m_unparsedEntityDeclHandler) {
          _Unsafe *eventEndPP = s;
          beforeHandler(parser);
          _Unsafe(parser->m_unparsedEntityDeclHandler(
              parser->m_handlerArg, parser->m_declEntity->name,
              parser->m_declEntity->base, parser->m_declEntity->systemId,
              parser->m_declEntity->publicId, parser->m_declEntity->notation));
          afterHandler(parser);
          handleDefault = XML_FALSE;
        } else if (parser->m_entityDeclHandler) {
          _Unsafe *eventEndPP = s;
          beforeHandler(parser);
          _Unsafe(parser->m_entityDeclHandler(
              parser->m_handlerArg, parser->m_declEntity->name, 0, 0, 0,
              parser->m_declEntity->base, parser->m_declEntity->systemId,
              parser->m_declEntity->publicId, parser->m_declEntity->notation));
          afterHandler(parser);
          handleDefault = XML_FALSE;
        }
      }
      break;
    case XML_ROLE_GENERAL_ENTITY_NAME: {
      if (XmlPredefinedEntityName(enc, s, next)) {
        parser->m_declEntity = nullptr;
        break;
      }
      if (dtd->keepProcessing) {
        const XML_Char *name = poolStoreString(&_Mut dtd->pool, enc, s, next);
        if (! name)
          return XML_ERROR_NO_MEMORY;
        _Unsafe parser->m_declEntity = (ENTITY *)lookup(parser, &_Mut dtd->generalEntities,
                                                name, sizeof(ENTITY));
        if (! parser->m_declEntity)
          return XML_ERROR_NO_MEMORY;
        if (_Unsafe((parser->m_declEntity->name != name))) {
          poolDiscard(&_Mut dtd->pool);
          parser->m_declEntity = nullptr;
        } else {
          poolFinish(&_Mut dtd->pool);
          _Unsafe parser->m_declEntity->publicId = nullptr;
          _Unsafe parser->m_declEntity->is_param = XML_FALSE;
          /* if we have a parent parser or are reading an internal parameter
             entity, then the entity declaration is not considered "internal"
          */
          _Unsafe parser->m_declEntity->is_internal
              = ! (parser->m_parentParser || parser->m_openInternalEntities);
          if (parser->m_entityDeclHandler)
            handleDefault = XML_FALSE;
        }
      } else {
        poolDiscard(&_Mut dtd->pool);
        parser->m_declEntity = nullptr;
      }
    } break;
    case XML_ROLE_PARAM_ENTITY_NAME:
#ifdef XML_DTD
      if (dtd->keepProcessing) {
        const XML_Char *name = poolStoreString(&_Mut dtd->pool, enc, s, next);
        if (! name)
          return XML_ERROR_NO_MEMORY;
        _Unsafe parser->m_declEntity = (ENTITY *)lookup(parser, &_Mut dtd->paramEntities,
                                                name, sizeof(ENTITY));
        if (! parser->m_declEntity)
          return XML_ERROR_NO_MEMORY;
        if (_Unsafe((parser->m_declEntity->name != name))) {
          poolDiscard(&_Mut dtd->pool);
          parser->m_declEntity = nullptr;
        } else {
          poolFinish(&_Mut dtd->pool);
          _Unsafe parser->m_declEntity->publicId = nullptr;
          _Unsafe parser->m_declEntity->is_param = XML_TRUE;
          /* if we have a parent parser or are reading an internal parameter
             entity, then the entity declaration is not considered "internal"
          */
          _Unsafe parser->m_declEntity->is_internal
              = ! (parser->m_parentParser || parser->m_openInternalEntities);
          if (parser->m_entityDeclHandler)
            handleDefault = XML_FALSE;
        }
      } else {
        poolDiscard(&_Mut dtd->pool);
        parser->m_declEntity = nullptr;
      }
#else  /* not XML_DTD */
      parser->m_declEntity = nullptr;
#endif /* XML_DTD */
      break;
    case XML_ROLE_NOTATION_NAME:
      parser->m_declNotationPublicId = nullptr;
      parser->m_declNotationName = nullptr;
      if (parser->m_notationDeclHandler) {
        parser->m_declNotationName
            = poolStoreString(&_Mut parser->m_tempPool, enc, s, next);
        if (! parser->m_declNotationName)
          return XML_ERROR_NO_MEMORY;
        poolFinish(&_Mut parser->m_tempPool);
        handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_NOTATION_PUBLIC_ID:
      if (! XmlIsPublicId(enc, s, next, eventPP))
        return XML_ERROR_PUBLICID;
      if (parser
              ->m_declNotationName) { /* means m_notationDeclHandler != nullptr */
        XML_Char *tem = poolStoreString(&_Mut parser->m_tempPool, enc,
                                        s + _Unsafe(enc->minBytesPerChar),
                                        next - _Unsafe(enc->minBytesPerChar));
        if (! tem)
          return XML_ERROR_NO_MEMORY;
        normalizePublicId(tem);
        parser->m_declNotationPublicId = tem;
        poolFinish(&_Mut parser->m_tempPool);
        handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_NOTATION_SYSTEM_ID:
      if (parser->m_declNotationName && parser->m_notationDeclHandler) {
        const XML_Char *systemId = poolStoreString(&_Mut parser->m_tempPool, enc,
                                                   s + _Unsafe(enc->minBytesPerChar),
                                                   next - _Unsafe(enc->minBytesPerChar));
        if (! systemId)
          return XML_ERROR_NO_MEMORY;
        _Unsafe *eventEndPP = s;
        beforeHandler(parser);
        _Unsafe(parser->m_notationDeclHandler(
            parser->m_handlerArg, parser->m_declNotationName, parser->m_curBase,
            systemId, parser->m_declNotationPublicId));
        afterHandler(parser);
        handleDefault = XML_FALSE;
      }
      poolClear(&_Mut parser->m_tempPool);
      break;
    case XML_ROLE_NOTATION_NO_SYSTEM_ID:
      if (parser->m_declNotationPublicId && parser->m_notationDeclHandler) {
        _Unsafe *eventEndPP = s;
        beforeHandler(parser);
        _Unsafe(parser->m_notationDeclHandler(
            parser->m_handlerArg, parser->m_declNotationName, parser->m_curBase,
            0, parser->m_declNotationPublicId));
        afterHandler(parser);
        handleDefault = XML_FALSE;
      }
      poolClear(&_Mut parser->m_tempPool);
      break;
    case XML_ROLE_ERROR:
      switch (tok) {
      case XML_TOK_PARAM_ENTITY_REF:
        /* PE references in internal subset are
           not allowed within declarations. */
        return XML_ERROR_PARAM_ENTITY_REF;
      case XML_TOK_XML_DECL:
        return XML_ERROR_MISPLACED_XML_PI;
      default:
        return XML_ERROR_SYNTAX;
      }
#ifdef XML_DTD
    case XML_ROLE_IGNORE_SECT: {
      enum XML_Error result;
      if (parser->m_defaultHandler)
        reportDefault(parser, enc, s, next);
      handleDefault = XML_FALSE;
      result = doIgnoreSection(parser, enc, _Unsafe(&next), end, nextPtr, haveMore);
      if (result != XML_ERROR_NONE)
        return result;
      else if (! next) {
        parser->m_processor = ignoreSectionProcessor;
        return result;
      }
    } break;
#endif /* XML_DTD */
    case XML_ROLE_GROUP_OPEN:
      if (parser->m_prologState.level >= parser->m_groupSize) {
        if (parser->m_groupSize) {
          /* Detect and prevent integer overflow */
          if (parser->m_groupSize > SIZE_MAX / 2) {
            return XML_ERROR_NO_MEMORY;
          }

          parser->m_groupSize *= 2;
          XML_Bool connectorOk = XML_FALSE;
          __auto_type bsc_h42 = parser->m_groupSize;
          REALLOC_ARRAY(parser, parser->m_groupConnector, char,
                        bsc_h42, connectorOk);
          if (! connectorOk) {
            parser->m_groupSize /= 2;
            return XML_ERROR_NO_MEMORY;
          }
        } else {
          parser->m_groupSize = 32;
          __auto_type bsc_h43 = parser->m_groupSize;
          parser->m_groupConnector
              = TAKE_ARRAY(char, MALLOC(parser, bsc_h43));
          if (! parser->m_groupConnector) {
            parser->m_groupSize = 0;
            return XML_ERROR_NO_MEMORY;
          }
        }
      }
      if (parser->m_groupConnector == nullptr)
        return XML_ERROR_UNEXPECTED_STATE;
      parser->m_groupConnector[parser->m_prologState.level] = 0;
      if (dtd->in_eldecl) {
        int myindex = nextScaffoldPart(parser);
        if (myindex < 0)
          return XML_ERROR_NO_MEMORY;
        _Unsafe assert(dtd->scaffIndex != nullptr);
        if ((size_t)dtd->scaffLevel >= dtd->scaffIndexSize) {
          /* Detect and prevent integer overflow */
          if (dtd->scaffIndexSize > SIZE_MAX / 2 / sizeof(int)) {
            return XML_ERROR_NO_MEMORY;
          }
          _Unsafe assert(dtd->scaffIndexSize > 0);
          const size_t new_size = dtd->scaffIndexSize * 2;
          int *const new_scaff_index
              = _Unsafe((int *)(REALLOC(parser, dtd->scaffIndex, new_size * sizeof(int))));
          if (new_scaff_index == nullptr) {
            return XML_ERROR_NO_MEMORY;
          }
          dtd->scaffIndex = new_scaff_index;
          dtd->scaffIndexSize = new_size;
        }
        dtd->scaffIndex[dtd->scaffLevel] = myindex;
        dtd->scaffLevel++;
        _Unsafe dtd->scaffold[myindex].type = XML_CTYPE_SEQ;
        if (parser->m_elementDeclHandler)
          handleDefault = XML_FALSE;
      }
      break;
    case XML_ROLE_GROUP_SEQUENCE:
      if (parser->m_groupConnector == nullptr)
        return XML_ERROR_UNEXPECTED_STATE;
      if (parser->m_groupConnector[parser->m_prologState.level] == ASCII_PIPE)
        return XML_ERROR_SYNTAX;
      parser->m_groupConnector[parser->m_prologState.level] = ASCII_COMMA;
      if (dtd->in_eldecl && parser->m_elementDeclHandler)
        handleDefault = XML_FALSE;
      break;
    case XML_ROLE_GROUP_CHOICE:
      if (parser->m_groupConnector == nullptr)
        return XML_ERROR_UNEXPECTED_STATE;
      if (parser->m_groupConnector[parser->m_prologState.level] == ASCII_COMMA)
        return XML_ERROR_SYNTAX;
      if (_Unsafe((dtd->in_eldecl
          && ! parser->m_groupConnector[parser->m_prologState.level]
          && (dtd->scaffold[dtd->scaffIndex[dtd->scaffLevel - 1]].type
              != XML_CTYPE_MIXED)))) {
        _Unsafe dtd->scaffold[dtd->scaffIndex[dtd->scaffLevel - 1]].type
            = XML_CTYPE_CHOICE;
        if (parser->m_elementDeclHandler)
          handleDefault = XML_FALSE;
      }
      parser->m_groupConnector[parser->m_prologState.level] = ASCII_PIPE;
      break;
    case XML_ROLE_PARAM_ENTITY_REF:
#ifdef XML_DTD
    case XML_ROLE_INNER_PARAM_ENTITY_REF:
      dtd->hasParamEntityRefs = XML_TRUE;
      if (! parser->m_paramEntityParsing)
        dtd->keepProcessing = dtd->standalone;
      else {
        const XML_Char *name;
        ENTITY *_Nonnull entity;
        name = poolStoreString(&_Mut dtd->pool, enc, s + _Unsafe(enc->minBytesPerChar),
                               next - _Unsafe(enc->minBytesPerChar));
        if (! name)
          return XML_ERROR_NO_MEMORY;
        _Unsafe entity = (ENTITY *)lookup(parser, &_Mut dtd->paramEntities, name, 0);
        poolDiscard(&_Mut dtd->pool);
        /* first, determine if a check for an existing declaration is needed;
           if yes, check that the entity exists, and that it is internal,
           otherwise call the skipped entity handler
        */
        if (parser->m_prologState.documentEntity
            && (dtd->standalone ? ! parser->m_openInternalEntities
                                : ! dtd->hasParamEntityRefs)) {
          if (! entity)
            return XML_ERROR_UNDEFINED_ENTITY;
          else if (_Unsafe((! entity->is_internal))) {
            /* It's hard to exhaustively search the code to be sure,
             * but there doesn't seem to be a way of executing the
             * following line.  There are two cases:
             *
             * If 'standalone' is false, the DTD must have no
             * parameter entities or we wouldn't have passed the outer
             * 'if' statement.  That means the only entity in the hash
             * table is the external subset name "#" which cannot be
             * given as a parameter entity name in XML syntax, so the
             * lookup must have returned nullptr and we don't even reach
             * the test for an internal entity.
             *
             * If 'standalone' is true, it does not seem to be
             * possible to create entities taking this code path that
             * are not internal entities, so fail the test above.
             *
             * Because this analysis is very uncertain, the code is
             * being left in place and merely removed from the
             * coverage test statistics.
             */
            return XML_ERROR_ENTITY_DECLARED_IN_PE; /* LCOV_EXCL_LINE */
          }
        } else if (! entity) {
          dtd->keepProcessing = dtd->standalone;
          /* cannot report skipped entities in declarations */
          if ((role == XML_ROLE_PARAM_ENTITY_REF)
              && parser->m_skippedEntityHandler) {
            beforeHandler(parser);
            _Unsafe(parser->m_skippedEntityHandler(parser->m_handlerArg, name, 1));
            afterHandler(parser);
            handleDefault = XML_FALSE;
          }
          break;
        }
        if (_Unsafe((entity->open)))
          return XML_ERROR_RECURSIVE_ENTITY_REF;
        if (_Unsafe((entity->textPtr))) {
          enum XML_Error result;
          XML_Bool betweenDecl
              = (role == XML_ROLE_PARAM_ENTITY_REF ? XML_TRUE : XML_FALSE);
          result = processEntity(parser, entity, betweenDecl, ENTITY_INTERNAL);
          if (result != XML_ERROR_NONE)
            return result;
          handleDefault = XML_FALSE;
          break;
        }
        if (parser->m_externalEntityRefHandler) {
          dtd->paramEntityRead = XML_FALSE;
          _Unsafe entity->open = true;
          entityTrackingOnOpen(parser, entity, __LINE__);
          beforeHandler(parser);
          const int status = _Unsafe(parser->m_externalEntityRefHandler(
              parser->m_externalEntityRefHandlerArg, 0, entity->base,
              entity->systemId, entity->publicId));
          afterHandler(parser);
          if (! status) {
            entityTrackingOnClose(parser, entity, __LINE__);
            _Unsafe entity->open = false;
            return XML_ERROR_EXTERNAL_ENTITY_HANDLING;
          }
          entityTrackingOnClose(parser, entity, __LINE__);
          _Unsafe entity->open = false;
          handleDefault = XML_FALSE;
          if (! dtd->paramEntityRead) {
            dtd->keepProcessing = dtd->standalone;
            break;
          }
        } else {
          dtd->keepProcessing = dtd->standalone;
          break;
        }
      }
#endif /* XML_DTD */
      if (! dtd->standalone && parser->m_notStandaloneHandler) {
        beforeHandler(parser);
        const int status = _Unsafe(parser->m_notStandaloneHandler(parser->m_handlerArg));
        afterHandler(parser);
        if (! status)
          return XML_ERROR_NOT_STANDALONE;
      }
      break;

      /* Element declaration stuff */

    case XML_ROLE_ELEMENT_NAME:
      if (parser->m_elementDeclHandler) {
        parser->m_declElementType = getElementType(parser, enc, s, next);
        if (! parser->m_declElementType)
          return XML_ERROR_NO_MEMORY;
        dtd->scaffLevel = 0;
        dtd->scaffCount = 0;
        dtd->in_eldecl = XML_TRUE;
        handleDefault = XML_FALSE;
      }
      break;

    case XML_ROLE_CONTENT_ANY:
    case XML_ROLE_CONTENT_EMPTY:
      if (dtd->in_eldecl) {
        if (parser->m_elementDeclHandler) {
          // NOTE: We are avoiding MALLOC(..) here to so that
          //       applications that are not using XML_FreeContentModel but
          //       plain free(..) or .free_fcn() to free the content model's
          //       memory are safe.
          XML_Content *content = _Unsafe((XML_Content *)(parser->m_mem.malloc_fcn(sizeof(XML_Content))));
          if (! content)
            return XML_ERROR_NO_MEMORY;
          _Unsafe content->quant = XML_CQUANT_NONE;
          _Unsafe content->name = nullptr;
          _Unsafe content->numchildren = 0;
          _Unsafe content->children = nullptr;
          _Unsafe content->type = ((role == XML_ROLE_CONTENT_ANY) ? XML_CTYPE_ANY
                                                          : XML_CTYPE_EMPTY);
          _Unsafe *eventEndPP = s;
          beforeHandler(parser);
          _Unsafe(parser->m_elementDeclHandler(
              parser->m_handlerArg, parser->m_declElementType->name, content));
          afterHandler(parser);
          handleDefault = XML_FALSE;
        }
        dtd->in_eldecl = XML_FALSE;
      }
      break;

    case XML_ROLE_CONTENT_PCDATA:
      if (dtd->in_eldecl) {
        _Unsafe dtd->scaffold[dtd->scaffIndex[dtd->scaffLevel - 1]].type
            = XML_CTYPE_MIXED;
        if (parser->m_elementDeclHandler)
          handleDefault = XML_FALSE;
      }
      break;

    case XML_ROLE_CONTENT_ELEMENT:
      quant = XML_CQUANT_NONE;
      goto elementContent;
    case XML_ROLE_CONTENT_ELEMENT_OPT:
      quant = XML_CQUANT_OPT;
      goto elementContent;
    case XML_ROLE_CONTENT_ELEMENT_REP:
      quant = XML_CQUANT_REP;
      goto elementContent;
    case XML_ROLE_CONTENT_ELEMENT_PLUS:
      quant = XML_CQUANT_PLUS;
    elementContent:
      if (dtd->in_eldecl) {
        ELEMENT_TYPE *el;
        const XML_Char *name;
        size_t nameLen;
        const char *nxt
            = (quant == XML_CQUANT_NONE ? next : next - _Unsafe(enc->minBytesPerChar));
        int myindex = nextScaffoldPart(parser);
        if (myindex < 0)
          return XML_ERROR_NO_MEMORY;
        _Unsafe dtd->scaffold[myindex].type = XML_CTYPE_NAME;
        _Unsafe dtd->scaffold[myindex].quant = quant;
        el = getElementType(parser, enc, s, nxt);
        if (! el)
          return XML_ERROR_NO_MEMORY;
        _Unsafe name = el->name;
        _Unsafe dtd->scaffold[myindex].name = name;
        nameLen = _Unsafe(xcslen(name)) + /*null terminator*/ 1;

        /* Detect and prevent integer overflow */
        if (nameLen > UINT_MAX - dtd->contentStringLen) {
          return XML_ERROR_NO_MEMORY;
        }

        dtd->contentStringLen += (unsigned)nameLen;
        if (parser->m_elementDeclHandler)
          handleDefault = XML_FALSE;
      }
      break;

    case XML_ROLE_GROUP_CLOSE:
      quant = XML_CQUANT_NONE;
      goto closeGroup;
    case XML_ROLE_GROUP_CLOSE_OPT:
      quant = XML_CQUANT_OPT;
      goto closeGroup;
    case XML_ROLE_GROUP_CLOSE_REP:
      quant = XML_CQUANT_REP;
      goto closeGroup;
    case XML_ROLE_GROUP_CLOSE_PLUS:
      quant = XML_CQUANT_PLUS;
    closeGroup:
      if (dtd->in_eldecl) {
        if (parser->m_elementDeclHandler)
          handleDefault = XML_FALSE;
        dtd->scaffLevel--;
        _Unsafe dtd->scaffold[dtd->scaffIndex[dtd->scaffLevel]].quant = quant;
        if (dtd->scaffLevel == 0) {
          if (! handleDefault) {
            XML_Content *model = build_model(parser);
            if (! model)
              return XML_ERROR_NO_MEMORY;
            _Unsafe *eventEndPP = s;
            beforeHandler(parser);
            _Unsafe(parser->m_elementDeclHandler(
                parser->m_handlerArg, parser->m_declElementType->name, model));
            afterHandler(parser);
          }
          dtd->in_eldecl = XML_FALSE;
          dtd->contentStringLen = 0;
        }
      }
      break;
      /* End element declaration stuff */

    case XML_ROLE_PI:
      if (! reportProcessingInstruction(parser, enc, s, next))
        return XML_ERROR_NO_MEMORY;
      handleDefault = XML_FALSE;
      break;
    case XML_ROLE_COMMENT:
      if (! reportComment(parser, enc, s, next))
        return XML_ERROR_NO_MEMORY;
      handleDefault = XML_FALSE;
      break;
    case XML_ROLE_NONE:
      switch (tok) {
      case XML_TOK_BOM:
        handleDefault = XML_FALSE;
        break;
      }
      break;
    case XML_ROLE_DOCTYPE_NONE:
      if (parser->m_startDoctypeDeclHandler)
        handleDefault = XML_FALSE;
      break;
    case XML_ROLE_ENTITY_NONE:
      if (dtd->keepProcessing && parser->m_entityDeclHandler)
        handleDefault = XML_FALSE;
      break;
    case XML_ROLE_NOTATION_NONE:
      if (parser->m_notationDeclHandler)
        handleDefault = XML_FALSE;
      break;
    case XML_ROLE_ATTLIST_NONE:
      if (dtd->keepProcessing && parser->m_attlistDeclHandler)
        handleDefault = XML_FALSE;
      break;
    case XML_ROLE_ELEMENT_NONE:
      if (parser->m_elementDeclHandler)
        handleDefault = XML_FALSE;
      break;
    } /* end of big switch */

    if (handleDefault && parser->m_defaultHandler)
      reportDefault(parser, enc, s, next);

    switch (parser->m_parsingStatus.parsing) {
    case XML_SUSPENDED:
      _Unsafe *nextPtr = next;
      return XML_ERROR_NONE;
    case XML_FINISHED:
      return XML_ERROR_ABORTED;
    case XML_PARSING:
      if (parser->m_reenter) {
        _Unsafe *nextPtr = next;
        return XML_ERROR_NONE;
      }
      EXPAT_FALLTHROUGH;
    default:
      s = next;
      tok = XmlPrologTok(enc, s, end, _Unsafe(&next));
    }
  }
  /* not reached */
}

_Safe static enum XML_Error PTRCALL
epilogProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                const char **_Nonnull nextPtr) {
  parser->m_processor = epilogProcessor;
  parser->m_eventPtr = s;
  for (;;) {
    const char *next = nullptr;
    int tok = XmlPrologTok(parser->m_encoding, s, end, _Unsafe(&next));
#if XML_GE == 1
    if (! accountingDiffTolerated(parser, tok, s, next, __LINE__,
                                  XML_ACCOUNT_DIRECT)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif
    parser->m_eventEndPtr = next;
    switch (tok) {
    /* report partial linebreak - it might be the last token */
    case -XML_TOK_PROLOG_S:
      if (parser->m_defaultHandler) {
        __auto_type bsc_h28 = parser->m_encoding;
        reportDefault(parser, bsc_h28, s, next);
        if (parser->m_parsingStatus.parsing == XML_FINISHED)
          return XML_ERROR_ABORTED;
      }
      _Unsafe *nextPtr = next;
      return XML_ERROR_NONE;
    case XML_TOK_NONE:
      _Unsafe *nextPtr = s;
      return XML_ERROR_NONE;
    case XML_TOK_PROLOG_S:
      if (parser->m_defaultHandler)
        { __auto_type bsc_h29 = parser->m_encoding;
        reportDefault(parser, bsc_h29, s, next); }
      break;
    case XML_TOK_PI:
      { __auto_type bsc_h30 = parser->m_encoding;
      if (! reportProcessingInstruction(parser, bsc_h30, s, next))
        return XML_ERROR_NO_MEMORY; }
      break;
    case XML_TOK_COMMENT:
      { __auto_type bsc_h31 = parser->m_encoding;
      if (! reportComment(parser, bsc_h31, s, next))
        return XML_ERROR_NO_MEMORY; }
      break;
    case XML_TOK_INVALID:
      parser->m_eventPtr = next;
      return XML_ERROR_INVALID_TOKEN;
    case XML_TOK_PARTIAL:
      if (! parser->m_parsingStatus.finalBuffer) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_UNCLOSED_TOKEN;
    case XML_TOK_PARTIAL_CHAR:
      if (! parser->m_parsingStatus.finalBuffer) {
        _Unsafe *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_PARTIAL_CHAR;
    default:
      return XML_ERROR_JUNK_AFTER_DOC_ELEMENT;
    }
    switch (parser->m_parsingStatus.parsing) {
    case XML_SUSPENDED:
      parser->m_eventPtr = next;
      _Unsafe *nextPtr = next;
      return XML_ERROR_NONE;
    case XML_FINISHED:
      parser->m_eventPtr = next;
      return XML_ERROR_ABORTED;
    case XML_PARSING:
      if (parser->m_reenter) {
        return XML_ERROR_UNEXPECTED_STATE; // LCOV_EXCL_LINE
      }
      EXPAT_FALLTHROUGH;
    default:;
      parser->m_eventPtr = s = next;
    }
  }
}

_Safe static enum XML_Error
processEntity(XML_Parser _Borrow parser, ENTITY *_Nonnull entity, XML_Bool betweenDecl,
              enum EntityType type) {
  OPEN_INTERNAL_ENTITY *openEntity, **openEntityList;
  OPEN_INTERNAL_ENTITY **const freeEntityList = _Unsafe(&parser->m_freeEntities);
  switch (type) {
  case ENTITY_INTERNAL:
    parser->m_processor = internalEntityProcessor;
    openEntityList = _Unsafe(&parser->m_openInternalEntities);
    break;
  case ENTITY_ATTRIBUTE:
    openEntityList = _Unsafe(&parser->m_openAttributeEntities);
    break;
  case ENTITY_VALUE:
    openEntityList = _Unsafe(&parser->m_openValueEntities);
    break;
    /* default case serves merely as a safety net in case of a
     * wrong entityType. Therefore we exclude the following lines
     * from the test coverage.
     *
     * LCOV_EXCL_START
     */
  default:
    // Should not reach here
    _Unsafe assert(0);
    /* LCOV_EXCL_STOP */
  }

  if (_Unsafe((*freeEntityList))) {
    _Unsafe openEntity = *freeEntityList;
    _Unsafe *freeEntityList = openEntity->next;
  } else {
    _Unsafe openEntity = MALLOC(parser, sizeof(OPEN_INTERNAL_ENTITY));
    if (! openEntity)
      return XML_ERROR_NO_MEMORY;
  }
  _Unsafe entity->open = true;
  _Unsafe entity->hasMore = XML_TRUE;
#if XML_GE == 1
  entityTrackingOnOpen(parser, entity, __LINE__);
#endif
  _Unsafe entity->processed = 0;
  _Unsafe openEntity->next = *openEntityList;
  _Unsafe *openEntityList = openEntity;
  _Unsafe openEntity->entity = entity;
  _Unsafe openEntity->type = type;
  _Unsafe openEntity->startTagLevel = parser->m_tagLevel;
  _Unsafe openEntity->betweenDecl = betweenDecl;
  _Unsafe openEntity->internalEventPtr = nullptr;
  _Unsafe openEntity->internalEventEndPtr = nullptr;

  // Only internal entities make use of the reenter flag
  // therefore no need to set it for other entity types
  if (type == ENTITY_INTERNAL) {
    triggerReenter(parser);
  }
  return XML_ERROR_NONE;
}

_Safe static enum XML_Error PTRCALL
internalEntityProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
                        const char **_Nonnull nextPtr) {
  UNUSED_P(s);
  UNUSED_P(end);
  UNUSED_P(nextPtr);
  ENTITY *_Nonnull entity;
  const char *textStart, *textEnd;
  const char *next;
  enum XML_Error result;
  OPEN_INTERNAL_ENTITY *openEntity = parser->m_openInternalEntities;
  if (! openEntity)
    return XML_ERROR_UNEXPECTED_STATE;

  _Unsafe entity = openEntity->entity;

  // This will return early
  if (_Unsafe((entity->hasMore))) {
    _Unsafe textStart = ((const char *)entity->textPtr) + entity->processed;
    _Unsafe textEnd = (const char *)(entity->textPtr + entity->textLen);
    /* Set a safe default value in case 'next' does not get set */
    next = textStart;

    if (_Unsafe((entity->is_param))) {
      int tok
          = XmlPrologTok(parser->m_internalEncoding, textStart, textEnd, _Unsafe(&next));
      __auto_type bsc_h32 = parser->m_internalEncoding;
      result = doProlog(parser, bsc_h32, textStart, textEnd,
                        tok, next, _Unsafe(&next), XML_FALSE, XML_FALSE,
                        XML_ACCOUNT_ENTITY_EXPANSION);
    } else {
      __auto_type bsc_h33 = parser->m_internalEncoding;
      _Unsafe result = doContent(parser, openEntity->startTagLevel,
                         bsc_h33, textStart, textEnd, _Unsafe(&next),
                         XML_FALSE, XML_ACCOUNT_ENTITY_EXPANSION);
    }

    if (result != XML_ERROR_NONE)
      return result;
    // Check if entity is complete, if not, mark down how much of it is
    // processed
    if (textEnd != next
        && (parser->m_parsingStatus.parsing == XML_SUSPENDED
            || (parser->m_parsingStatus.parsing == XML_PARSING
                && parser->m_reenter))) {
      _Unsafe entity->processed = (int)(next - (const char *)entity->textPtr);
      return result;
    }

    // Entity is complete. We cannot close it here since we need to first
    // process its possible inner entities (which are added to the
    // m_openInternalEntities during doProlog or doContent calls above)
    _Unsafe entity->hasMore = XML_FALSE;
    if (_Unsafe((! entity->is_param
        && (openEntity->startTagLevel != parser->m_tagLevel)))) {
      return XML_ERROR_ASYNC_ENTITY;
    }
    triggerReenter(parser);
    return result;
  } // End of entity processing, "if" block will return here

  // Remove fully processed openEntity from open entity list.
#if XML_GE == 1
  entityTrackingOnClose(parser, entity, __LINE__);
#endif
  // openEntity is m_openInternalEntities' head, as we set it at the start of
  // this function and we skipped doProlog and doContent calls with hasMore set
  // to false. This means we can directly remove the head of
  // m_openInternalEntities
  _Unsafe assert(parser->m_openInternalEntities == openEntity);
  _Unsafe entity->open = false;
  _Unsafe parser->m_openInternalEntities = parser->m_openInternalEntities->next;

  /* put openEntity back in list of free instances */
  _Unsafe openEntity->next = parser->m_freeEntities;
  parser->m_freeEntities = openEntity;

  if (parser->m_openInternalEntities == nullptr) {
    _Unsafe parser->m_processor = entity->is_param ? prologProcessor : contentProcessor;
  }
  triggerReenter(parser);
  return XML_ERROR_NONE;
}

_Safe static enum XML_Error PTRCALL
errorProcessor(XML_Parser _Borrow parser, const char *s, const char *end,
               const char **_Nonnull nextPtr) {
  UNUSED_P(s);
  UNUSED_P(end);
  UNUSED_P(nextPtr);
  return parser->m_errorCode;
}

_Safe static enum XML_Error
storeAttributeValue(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, XML_Bool isCdata,
                    const char *ptr, const char *end, STRING_POOL *_Borrow pool,
                    enum XML_Account account) {
  const char *next = ptr;
  enum XML_Error result = XML_ERROR_NONE;

  while (1) {
    if (! parser->m_openAttributeEntities) {
      _Unsafe result = appendAttributeValue(parser, enc, isCdata, next, end, pool,
                                    account, _Unsafe(&next));
    } else {
      OPEN_INTERNAL_ENTITY *const openEntity = parser->m_openAttributeEntities;
      if (! openEntity)
        return XML_ERROR_UNEXPECTED_STATE;

      ENTITY *const entity = _Unsafe((openEntity->entity));
      const char *const textStart
          = _Unsafe((((const char *)entity->textPtr) + entity->processed));
      const char *const textEnd
          = _Unsafe(((const char *)(entity->textPtr + entity->textLen)));
      /* Set a safe default value in case 'next' does not get set */
      const char *nextInEntity = textStart;
      if (_Unsafe((entity->hasMore))) {
        __auto_type bsc_h34 = parser->m_internalEncoding;
        _Unsafe result = appendAttributeValue(
            parser, bsc_h34, isCdata, textStart, textEnd,
            pool, XML_ACCOUNT_ENTITY_EXPANSION, _Unsafe(&nextInEntity));
        if (result != XML_ERROR_NONE)
          break;
        // Check if entity is complete, if not, mark down how much of it is
        // processed. A XML_SUSPENDED check here is not required as
        // appendAttributeValue will never suspend the parser.
        if (nextInEntity < textEnd) {
          _Unsafe entity->processed
              = (int)(nextInEntity - (const char *)entity->textPtr);
          continue;
        }
        _Unsafe assert(nextInEntity == textEnd);

        // Entity is complete. We cannot close it here since we need to first
        // process its possible inner entities (which are added to the
        // m_openAttributeEntities during appendAttributeValue)
        _Unsafe entity->hasMore = XML_FALSE;
        continue;
      } // End of entity processing, "if" block skips the rest

      // Remove fully processed openEntity from open entity list.
#if XML_GE == 1
      entityTrackingOnClose(parser, entity, __LINE__);
#endif
      // openEntity is m_openAttributeEntities' head, since we set it at the
      // start of this function and because we skipped appendAttributeValue call
      // with hasMore set to false. This means we can directly remove the head
      // of m_openAttributeEntities
      _Unsafe assert(parser->m_openAttributeEntities == openEntity);
      _Unsafe entity->open = false;
      _Unsafe parser->m_openAttributeEntities = parser->m_openAttributeEntities->next;

      /* put openEntity back in list of free instances */
      _Unsafe openEntity->next = parser->m_freeEntities;
      parser->m_freeEntities = openEntity;
    }

    // Break if an error occurred or there is nothing left to process
    if (result || (parser->m_openAttributeEntities == nullptr && end == next)) {
      break;
    }
  }

  if (result)
    return result;
  if (! isCdata && poolLength(pool) && poolLastChar(pool) == 0x20)
    poolChop(pool);
  if (! poolAppendChar(pool, XML_T('\0')))
    return XML_ERROR_NO_MEMORY;
  return XML_ERROR_NONE;
}

_Safe static enum XML_Error
appendAttributeValue(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, XML_Bool isCdata,
                     const char *ptr, const char *end, STRING_POOL *_Borrow pool,
                     enum XML_Account account, const char **_Nullable nextPtr) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return XML_ERROR_UNEXPECTED_STATE;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
#ifndef XML_DTD
  UNUSED_P(account);
#endif

  for (;;) {
    const char *next
        = ptr; /* XmlAttributeValueTok doesn't always set the last arg */
    int tok = XmlAttributeValueTok(enc, ptr, end, _Unsafe(&next));
#if XML_GE == 1
    if (! accountingDiffTolerated(parser, tok, ptr, next, __LINE__, account)) {
      accountingOnAbort(parser);
      return XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
    }
#endif
    switch (tok) {
    case XML_TOK_NONE:
      if (nextPtr) {
        _Unsafe *nextPtr = next;
      }
      return XML_ERROR_NONE;
    case XML_TOK_INVALID:
      if (enc == parser->m_encoding)
        parser->m_eventPtr = next;
      return XML_ERROR_INVALID_TOKEN;
    case XML_TOK_PARTIAL:
      if (enc == parser->m_encoding)
        parser->m_eventPtr = ptr;
      return XML_ERROR_INVALID_TOKEN;
    case XML_TOK_CHAR_REF: {
      XML_Char buf[XML_ENCODE_MAX] = {0};
      int n = XmlCharRefNumber(enc, ptr);
      if (n < 0) {
        if (enc == parser->m_encoding)
          parser->m_eventPtr = ptr;
        return XML_ERROR_BAD_CHAR_REF;
      }
      if (! isCdata && n == 0x20 /* space */
          && (poolLength(pool) == 0 || poolLastChar(pool) == 0x20))
        break;
      n = _Unsafe(XmlEncode(n, (ICHAR *)buf));
      /* The _Unsafe(XmlEncode()) functions can never return 0 here.  That
       * error return happens if the code point passed in is either
       * negative or greater than or equal to 0x110000.  The
       * XmlCharRefNumber() functions will all return a number
       * strictly less than 0x110000 or a negative value if an error
       * occurred.  The negative value is intercepted above, so
       * _Unsafe(XmlEncode()) is never passed a value it might return an
       * error for.
       */

      if (! poolAppendChars(pool, buf, (size_t)n))
        return XML_ERROR_NO_MEMORY;
    } break;
    case XML_TOK_DATA_CHARS:
      if (! poolAppend(pool, enc, ptr, next))
        return XML_ERROR_NO_MEMORY;
      break;
    case XML_TOK_TRAILING_CR:
      next = ptr + _Unsafe(enc->minBytesPerChar);
      EXPAT_FALLTHROUGH;
    case XML_TOK_ATTRIBUTE_VALUE_S:
    case XML_TOK_DATA_NEWLINE:
      if (! isCdata && (poolLength(pool) == 0 || poolLastChar(pool) == 0x20))
        break;
      if (! poolAppendChar(pool, 0x20))
        return XML_ERROR_NO_MEMORY;
      break;
    case XML_TOK_ENTITY_REF: {
      const XML_Char *name;
      ENTITY *_Nonnull entity;
      bool checkEntityDecl;
      XML_Char ch = (XML_Char)XmlPredefinedEntityName(
          enc, ptr + _Unsafe(enc->minBytesPerChar), next - _Unsafe(enc->minBytesPerChar));
      if (ch) {
#if XML_GE == 1
        /* NOTE: We are replacing 4-6 characters original input for 1 character
         *       so there is no amplification and hence recording without
         *       protection. */
        accountingDiffTolerated(parser, tok, (char *)_Unsafe(&ch),
                                ((char *)_Unsafe(&ch)) + sizeof(XML_Char), __LINE__,
                                XML_ACCOUNT_ENTITY_EXPANSION);
#endif /* XML_GE == 1 */
        if (! poolAppendChar(pool, ch))
          return XML_ERROR_NO_MEMORY;
        break;
      }
      name = poolStoreString(&_Mut parser->m_temp2Pool, enc,
                             ptr + _Unsafe(enc->minBytesPerChar),
                             next - _Unsafe(enc->minBytesPerChar));
      if (! name)
        return XML_ERROR_NO_MEMORY;
      _Unsafe entity = (ENTITY *)lookup(parser, &_Mut dtd->generalEntities, name, 0);
      poolDiscard(&_Mut parser->m_temp2Pool);
      /* First, determine if a check for an existing declaration is needed;
         if yes, check that the entity exists, and that it is internal.
      */
      if (pool == &_Mut dtd->pool) /* are we called from prolog? */
        checkEntityDecl =
#ifdef XML_DTD
            parser->m_prologState.documentEntity &&
#endif /* XML_DTD */
            (dtd->standalone ? ! parser->m_openInternalEntities
                             : ! dtd->hasParamEntityRefs);
      else /* if (pool == &_Mut parser->m_tempPool): we are called from content */
        checkEntityDecl = ! dtd->hasParamEntityRefs || dtd->standalone;
      if (checkEntityDecl) {
        if (! entity)
          return XML_ERROR_UNDEFINED_ENTITY;
        else if (_Unsafe((! entity->is_internal)))
          return XML_ERROR_ENTITY_DECLARED_IN_PE;
      } else if (! entity) {
        /* Cannot report skipped entity here - see comments on
           parser->m_skippedEntityHandler.
        if (parser->m_skippedEntityHandler) {
          beforeHandler(parser);
          _Unsafe(parser->m_skippedEntityHandler(parser->m_handlerArg, name, 0));
          afterHandler(parser);
        }
        */
        /* Cannot call the default handler because this would be
           out of sync with the call to the startElementHandler.
        if ((pool == &_Mut parser->m_tempPool) && parser->m_defaultHandler)
          reportDefault(parser, enc, ptr, next);
        */
        break;
      }
      if (_Unsafe((entity->open))) {
        if (enc == parser->m_encoding) {
          /* It does not appear that this line can be executed.
           *
           * The "if (entity->open)" check catches recursive entity
           * definitions.  In order to be called with an open
           * entity, it must have gone through this code before and
           * been through the recursive call to
           * appendAttributeValue() some lines below.  That call
           * sets the local encoding ("enc") to the parser's
           * internal encoding (internal_utf8 or internal_utf16),
           * which can never be the same as the principle encoding.
           * It doesn't appear there is another code path that gets
           * here with entity->open being TRUE.
           *
           * Since it is not certain that this logic is watertight,
           * we keep the line and merely exclude it from coverage
           * tests.
           */
          parser->m_eventPtr = ptr; /* LCOV_EXCL_LINE */
        }
        return XML_ERROR_RECURSIVE_ENTITY_REF;
      }
      if (_Unsafe((entity->notation))) {
        if (enc == parser->m_encoding)
          parser->m_eventPtr = ptr;
        return XML_ERROR_BINARY_ENTITY_REF;
      }
      if (_Unsafe((! entity->textPtr))) {
        if (enc == parser->m_encoding)
          parser->m_eventPtr = ptr;
        return XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF;
      } else {
        enum XML_Error result;
        result = processEntity(parser, entity, XML_FALSE, ENTITY_ATTRIBUTE);
        if ((result == XML_ERROR_NONE) && (nextPtr != nullptr)) {
          _Unsafe *nextPtr = next;
        }
        return result;
      }
    } break;
    default:
      /* The only token returned by XmlAttributeValueTok() that does
       * not have an explicit case here is XML_TOK_PARTIAL_CHAR.
       * Getting that would require an entity name to contain an
       * incomplete XML character (e.g. \xE2\x82); however previous
       * tokenisers will have already recognised and rejected such
       * names before XmlAttributeValueTok() gets a look-in.  This
       * default case should be retained as a safety net, but the code
       * excluded from coverage tests.
       *
       * LCOV_EXCL_START
       */
      if (enc == parser->m_encoding)
        parser->m_eventPtr = ptr;
      return XML_ERROR_UNEXPECTED_STATE;
      /* LCOV_EXCL_STOP */
    }
    ptr = next;
  }
  /* not reached */
}

#if XML_GE == 1
_Safe static enum XML_Error
storeEntityValue(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                 const char *entityTextPtr, const char *entityTextEnd,
                 enum XML_Account account, const char **_Nullable nextPtr) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return XML_ERROR_UNEXPECTED_STATE;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  STRING_POOL *_Borrow pool = &_Mut dtd->entityValuePool;
  enum XML_Error result = XML_ERROR_NONE;
#  ifdef XML_DTD
  int oldInEntityValue = parser->m_prologState.inEntityValue;
  parser->m_prologState.inEntityValue = 1;
#  else
  UNUSED_P(account);
#  endif /* XML_DTD */
  /* never return Null for the value argument in EntityDeclHandler,
     since this would indicate an external entity; therefore we
     have to make sure that entityValuePool.start is not null */
  if (! pool->blocks) {
    if (! poolGrow(pool))
      return XML_ERROR_NO_MEMORY;
  }

  const char *next = entityTextPtr;

  /* Nothing to tokenize. */
  if (entityTextPtr >= entityTextEnd) {
    result = XML_ERROR_NONE;
    goto endEntityValue;
  }

  for (;;) {
    next
        = entityTextPtr; /* XmlEntityValueTok doesn't always set the last arg */
    int tok = XmlEntityValueTok(enc, entityTextPtr, entityTextEnd, _Unsafe(&next));

    if (! accountingDiffTolerated(parser, tok, entityTextPtr, next, __LINE__,
                                  account)) {
      accountingOnAbort(parser);
      result = XML_ERROR_AMPLIFICATION_LIMIT_BREACH;
      goto endEntityValue;
    }

    switch (tok) {
    case XML_TOK_PARAM_ENTITY_REF:
#  ifdef XML_DTD
      if (parser->m_isParamEntity || enc != parser->m_encoding) {
        const XML_Char *name;
        ENTITY *_Nonnull entity;
        name = poolStoreString(&_Mut parser->m_tempPool, enc,
                               entityTextPtr + _Unsafe(enc->minBytesPerChar),
                               next - _Unsafe(enc->minBytesPerChar));
        if (! name) {
          result = XML_ERROR_NO_MEMORY;
          goto endEntityValue;
        }
        _Unsafe entity = (ENTITY *)lookup(parser, &_Mut dtd->paramEntities, name, 0);
        poolDiscard(&_Mut parser->m_tempPool);
        if (! entity) {
          /* not a well-formedness error - see XML 1.0: WFC Entity Declared */
          /* cannot report skipped entity here - see comments on
             parser->m_skippedEntityHandler
          if (parser->m_skippedEntityHandler) {
            beforeHandler(parser);
            _Unsafe(parser->m_skippedEntityHandler(parser->m_handlerArg, name, 0));
            afterHandler(parser);
          }
          */
          dtd->keepProcessing = dtd->standalone;
          goto endEntityValue;
        }
        if (_Unsafe((entity->open || (entity == parser->m_declEntity)))) {
          if (enc == parser->m_encoding)
            parser->m_eventPtr = entityTextPtr;
          result = XML_ERROR_RECURSIVE_ENTITY_REF;
          goto endEntityValue;
        }
        if (_Unsafe((entity->systemId))) {
          if (parser->m_externalEntityRefHandler) {
            dtd->paramEntityRead = XML_FALSE;
            _Unsafe entity->open = true;
            entityTrackingOnOpen(parser, entity, __LINE__);
            beforeHandler(parser);
            const int status = _Unsafe(parser->m_externalEntityRefHandler(
                parser->m_externalEntityRefHandlerArg, 0, entity->base,
                entity->systemId, entity->publicId));
            afterHandler(parser);
            if (! status) {
              entityTrackingOnClose(parser, entity, __LINE__);
              _Unsafe entity->open = false;
              result = XML_ERROR_EXTERNAL_ENTITY_HANDLING;
              goto endEntityValue;
            }
            entityTrackingOnClose(parser, entity, __LINE__);
            _Unsafe entity->open = false;
            if (! dtd->paramEntityRead)
              dtd->keepProcessing = dtd->standalone;
          } else
            dtd->keepProcessing = dtd->standalone;
        } else {
          result = processEntity(parser, entity, XML_FALSE, ENTITY_VALUE);
          goto endEntityValue;
        }
        break;
      }
#  endif /* XML_DTD */
      /* In the internal subset, PE references are not legal
         within markup declarations, e.g entity values in this case. */
      parser->m_eventPtr = entityTextPtr;
      result = XML_ERROR_PARAM_ENTITY_REF;
      goto endEntityValue;
    case XML_TOK_NONE:
      result = XML_ERROR_NONE;
      goto endEntityValue;
    case XML_TOK_ENTITY_REF:
    case XML_TOK_DATA_CHARS:
      if (! poolAppend(pool, enc, entityTextPtr, next)) {
        result = XML_ERROR_NO_MEMORY;
        goto endEntityValue;
      }
      break;
    case XML_TOK_TRAILING_CR:
      next = entityTextPtr + _Unsafe(enc->minBytesPerChar);
      EXPAT_FALLTHROUGH;
    case XML_TOK_DATA_NEWLINE:
      if (! poolAppendChar(pool, 0xA)) {
        result = XML_ERROR_NO_MEMORY;
        goto endEntityValue;
      }
      break;
    case XML_TOK_CHAR_REF: {
      XML_Char buf[XML_ENCODE_MAX] = {0};
      int n = XmlCharRefNumber(enc, entityTextPtr);
      if (n < 0) {
        if (enc == parser->m_encoding)
          parser->m_eventPtr = entityTextPtr;
        result = XML_ERROR_BAD_CHAR_REF;
        goto endEntityValue;
      }
      n = _Unsafe(XmlEncode(n, (ICHAR *)buf));
      /* The _Unsafe(XmlEncode()) functions can never return 0 here.  That
       * error return happens if the code point passed in is either
       * negative or greater than or equal to 0x110000.  The
       * XmlCharRefNumber() functions will all return a number
       * strictly less than 0x110000 or a negative value if an error
       * occurred.  The negative value is intercepted above, so
       * _Unsafe(XmlEncode()) is never passed a value it might return an
       * error for.
       */
      if (! poolAppendChars(pool, buf, (size_t)n)) {
        result = XML_ERROR_NO_MEMORY;
        goto endEntityValue;
      }
    } break;
    case XML_TOK_PARTIAL:
      if (enc == parser->m_encoding)
        parser->m_eventPtr = entityTextPtr;
      result = XML_ERROR_INVALID_TOKEN;
      goto endEntityValue;
    case XML_TOK_INVALID:
      if (enc == parser->m_encoding)
        parser->m_eventPtr = next;
      result = XML_ERROR_INVALID_TOKEN;
      goto endEntityValue;
    default:
      /* This default case should be unnecessary -- all the tokens
       * that XmlEntityValueTok() can return have their own explicit
       * cases -- but should be retained for safety.  We do however
       * exclude it from the coverage statistics.
       *
       * LCOV_EXCL_START
       */
      if (enc == parser->m_encoding)
        parser->m_eventPtr = entityTextPtr;
      result = XML_ERROR_UNEXPECTED_STATE;
      goto endEntityValue;
      /* LCOV_EXCL_STOP */
    }
    entityTextPtr = next;
  }
endEntityValue:
#  ifdef XML_DTD
  parser->m_prologState.inEntityValue = oldInEntityValue;
#  endif /* XML_DTD */
  // If 'nextPtr' is given, it should be updated during the processing
  if (nextPtr != nullptr) {
    _Unsafe *nextPtr = next;
  }
  return result;
}

_Safe static enum XML_Error
callStoreEntityValue(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                     const char *entityTextPtr, const char *entityTextEnd,
                     enum XML_Account account) {
  const char *next = entityTextPtr;
  enum XML_Error result = XML_ERROR_NONE;
  while (1) {
    if (! parser->m_openValueEntities) {
      result
          = storeEntityValue(parser, enc, next, entityTextEnd, account, _Unsafe(&next));
    } else {
      OPEN_INTERNAL_ENTITY *const openEntity = parser->m_openValueEntities;
      if (! openEntity)
        return XML_ERROR_UNEXPECTED_STATE;

      ENTITY *const entity = _Unsafe((openEntity->entity));
      const char *const textStart
          = _Unsafe((((const char *)entity->textPtr) + entity->processed));
      const char *const textEnd
          = _Unsafe(((const char *)(entity->textPtr + entity->textLen)));
      /* Set a safe default value in case 'next' does not get set */
      const char *nextInEntity = textStart;
      if (_Unsafe((entity->hasMore))) {
        __auto_type bsc_h35 = parser->m_internalEncoding;
        result = storeEntityValue(parser, bsc_h35, textStart,
                                  textEnd, XML_ACCOUNT_ENTITY_EXPANSION,
                                  _Unsafe(&nextInEntity));
        if (result != XML_ERROR_NONE)
          break;
        // Check if entity is complete, if not, mark down how much of it is
        // processed. A XML_SUSPENDED check here is not required as
        // appendAttributeValue will never suspend the parser.
        if (textEnd != nextInEntity) {
          _Unsafe entity->processed
              = (int)(nextInEntity - (const char *)entity->textPtr);
          continue;
        }

        // Entity is complete. We cannot close it here since we need to first
        // process its possible inner entities (which are added to the
        // m_openValueEntities during storeEntityValue)
        _Unsafe entity->hasMore = XML_FALSE;
        continue;
      } // End of entity processing, "if" block skips the rest

      // Remove fully processed openEntity from open entity list.
#  if XML_GE == 1
      entityTrackingOnClose(parser, entity, __LINE__);
#  endif
      // openEntity is m_openValueEntities' head, since we set it at the
      // start of this function and because we skipped storeEntityValue call
      // with hasMore set to false. This means we can directly remove the head
      // of m_openValueEntities
      _Unsafe assert(parser->m_openValueEntities == openEntity);
      _Unsafe entity->open = false;
      _Unsafe parser->m_openValueEntities = parser->m_openValueEntities->next;

      /* put openEntity back in list of free instances */
      _Unsafe openEntity->next = parser->m_freeEntities;
      parser->m_freeEntities = openEntity;
    }

    // Break if an error occurred or there is nothing left to process
    if (result
        || (parser->m_openValueEntities == nullptr && entityTextEnd == next)) {
      break;
    }
  }

  return result;
}

#else /* XML_GE == 0 */

_Safe static enum XML_Error
storeSelfEntityValue(XML_Parser _Borrow parser, ENTITY *_Nonnull entity) {
  // This will store "&amp;entity123;" in entity->textPtr
  // to end up as "&entity123;" in the handler.
  const char *const entity_start = "&amp;";
  const char *const entity_end = ";";

  STRING_POOL *const pool = _Unsafe(&_Mut parser->m_dtd->entityValuePool);
  if (! poolAppendString(pool, entity_start)
      || ! poolAppendString(pool, entity->name)
      || ! poolAppendString(pool, entity_end)) {
    poolDiscard(pool);
    return XML_ERROR_NO_MEMORY;
  }

  /* Detect and prevent signed integer overflow */
  if ((size_t)poolLength(pool) > (size_t)INT_MAX) {
    poolDiscard(pool);
    return XML_ERROR_NO_MEMORY;
  }
  entity->textPtr = poolStart(pool);
  entity->textLen = (int)(poolLength(pool));
  poolFinish(pool);

  return XML_ERROR_NONE;
}

#endif /* XML_GE == 0 */

_Safe static void FASTCALL
normalizeLines(XML_Char *_Nonnull s) {
  XML_Char *p;
  _Unsafe for (;; s++) {
    if (*s == XML_T('\0'))
      return;
    if (*s == 0xD)
      break;
  }
  p = s;
  do {
    if (_Unsafe((*s == 0xD))) {
      _Unsafe *p++ = 0xA;
      if (_Unsafe((*++s == 0xA)))
        _Unsafe s++;
    } else
      _Unsafe *p++ = *s++;
  } while (_Unsafe((*s)));
  _Unsafe *p = XML_T('\0');
}

_Safe static int
reportProcessingInstruction(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc,
                            const char *start, const char *end) {
  const XML_Char *target;
  XML_Char *data;
  const char *tem;
  if (! parser->m_processingInstructionHandler) {
    if (parser->m_defaultHandler)
      reportDefault(parser, enc, start, end);
    return 1;
  }
  start += _Unsafe(enc->minBytesPerChar) * 2;
  tem = start + XmlNameLength(enc, start);
  target = poolStoreString(&_Mut parser->m_tempPool, enc, start, tem);
  if (! target)
    return 0;
  poolFinish(&_Mut parser->m_tempPool);
  data = poolStoreString(&_Mut parser->m_tempPool, enc, XmlSkipS(enc, tem),
                         end - _Unsafe(enc->minBytesPerChar) * 2);
  if (! data)
    return 0;
  normalizeLines(data);
  beforeHandler(parser);
  _Unsafe(parser->m_processingInstructionHandler(parser->m_handlerArg, target, data));
  afterHandler(parser);
  poolClear(&_Mut parser->m_tempPool);
  return 1;
}

_Safe static int
reportComment(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char *start,
              const char *end) {
  XML_Char *data;
  if (! parser->m_commentHandler) {
    if (parser->m_defaultHandler)
      reportDefault(parser, enc, start, end);
    return 1;
  }
  data = poolStoreString(&_Mut parser->m_tempPool, enc,
                         start + _Unsafe(enc->minBytesPerChar) * 4,
                         end - _Unsafe(enc->minBytesPerChar) * 3);
  if (! data)
    return 0;
  normalizeLines(data);
  beforeHandler(parser);
  _Unsafe(parser->m_commentHandler(parser->m_handlerArg, data));
  afterHandler(parser);
  poolClear(&_Mut parser->m_tempPool);
  return 1;
}

_Safe static void
reportDefault(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char *s,
              const char *end) {
  if (MUST_CONVERT(enc, s)) {
    enum XML_Convert_Result convert_res;
    const char **eventPP;
    const char **eventEndPP;
    if (enc == parser->m_encoding) {
      eventPP = _Unsafe(&parser->m_eventPtr);
      eventEndPP = _Unsafe(&parser->m_eventEndPtr);
    } else {
      /* To get here, two things must be true; the parser must be
       * using a character encoding that is not the same as the
       * encoding passed in, and the encoding passed in must need
       * conversion to the internal format (UTF-8 unless XML_UNICODE
       * is defined).  The only occasions on which the encoding passed
       * in is not the same as the parser's encoding are when it is
       * the internal encoding (e.g. a previously defined parameter
       * entity, already converted to internal format).  This by
       * definition doesn't need conversion, so the whole branch never
       * gets executed.
       *
       * For safety's sake we don't delete these lines and merely
       * exclude them from coverage statistics.
       *
       * LCOV_EXCL_START
       */
      eventPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventPtr));
      eventEndPP = _Unsafe(_Unsafe(&parser->m_openInternalEntities->internalEventEndPtr));
      /* LCOV_EXCL_STOP */
    }
    do {
      ICHAR *dataPtr = _Unsafe(((ICHAR *)&_Mut *parser->m_dataBuf));
      convert_res
          = _Unsafe(XmlConvert(enc, _Unsafe(&s), end, _Unsafe(&dataPtr), (ICHAR *)parser->m_dataBufEnd));
      _Unsafe *eventEndPP = s;
      beforeHandler(parser);
      _Unsafe(parser->m_defaultHandler(parser->m_handlerArg, (XML_Char *)&_Mut *parser->m_dataBuf,
                               (int)(dataPtr - (ICHAR *)&_Mut *parser->m_dataBuf)));
      afterHandler(parser);
      _Unsafe *eventPP = s;
    } while ((convert_res != XML_CONVERT_COMPLETED)
             && (convert_res != XML_CONVERT_INPUT_INCOMPLETE));
  } else {
    beforeHandler(parser);
    _Unsafe(parser->m_defaultHandler(
        parser->m_handlerArg, (const XML_Char *)s,
        (int)((const XML_Char *)end - (const XML_Char *)s)));
    afterHandler(parser);
  }
}

_Safe static int
defineAttribute(ELEMENT_TYPE *_Nonnull type, ATTRIBUTE_ID *_Nonnull attId, XML_Bool isCdata,
                XML_Bool isId, const XML_Char *_Nullable value, XML_Parser _Borrow parser) {
  DEFAULT_ATTRIBUTE *att;
  if (value || isId) {
    /* The handling of default attributes gets messed up if we have
       a default which duplicates a non-default. */
    NAMED *const nameFound
        = _Unsafe((lookup(parser, _Unsafe(&_Mut type->defaultAttForName), attId->name, 0)));
    if (nameFound)
      return 1;
    if (_Unsafe((isId && ! type->idAtt && ! attId->xmlns)))
      _Unsafe type->idAtt = attId;
  }
  if (_Unsafe((type->nDefaultAtts == type->allocDefaultAtts))) {
    /* Detect and prevent integer overflow */
    if (_Unsafe((type->allocDefaultAtts > SIZE_MAX / 2))) {
      return 0;
    }

    size_t count = _Unsafe((type->allocDefaultAtts * 2));
    if (count == 0) {
      count = 8;
    }

    /* Detect and prevent integer overflow. */
    if (count > SIZE_MAX / sizeof(DEFAULT_ATTRIBUTE)) {
      return 0;
    }

    XML_Bool defaultAttsOk = XML_FALSE;
    _Unsafe REALLOC_ARRAY(parser, type->defaultAtts, DEFAULT_ATTRIBUTE,
                          count * sizeof(DEFAULT_ATTRIBUTE), defaultAttsOk);
    if (! defaultAttsOk)
      return 0;
    _Unsafe type->allocDefaultAtts = count;
  }
  att = _Unsafe((DEFAULT_ATTRIBUTE *)&_Mut *type->defaultAtts
                + type->nDefaultAtts);
  _Unsafe att->id = attId;
  _Unsafe att->value = value;
  _Unsafe att->isCdata = isCdata;
  if (! isCdata)
    _Unsafe attId->maybeTokenized = XML_TRUE;

  NAME_AND_DEFAULT_ATTRIBUTE *const nameAndDefaultAttribute
      = _Unsafe(((NAME_AND_DEFAULT_ATTRIBUTE *)lookup(
          parser, _Unsafe(&_Mut type->defaultAttForName), attId->name,
          sizeof(NAME_AND_DEFAULT_ATTRIBUTE))));
  if (! nameAndDefaultAttribute)
    return 0;

  _Unsafe assert(nameAndDefaultAttribute->name == attId->name);

  // NOTE: The XML 1.0r4 spec says:
  // "When more than one definition is provided for the same attribute of a
  // given element type, the first declaration is binding and later
  // declarations are ignored."
  if (_Unsafe((! nameAndDefaultAttribute->initialized))) {
    _Unsafe nameAndDefaultAttribute->attIndex = type->nDefaultAtts;
    _Unsafe nameAndDefaultAttribute->initialized = true;
  }

  _Unsafe type->nDefaultAtts += 1;
  return 1;
}

_Safe static int
setElementTypePrefix(XML_Parser _Borrow parser, ELEMENT_TYPE *_Nonnull elementType) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return 0;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  const XML_Char *name;
  _Unsafe for (name = elementType->name; *name; name++) {
    if (*name == XML_T(ASCII_COLON)) {
      PREFIX *prefix;
      const XML_Char *s;
      for (s = elementType->name; s != name; s++) {
        if (! poolAppendChar(&_Mut dtd->pool, *s))
          return 0;
      }
      if (! poolAppendChar(&_Mut dtd->pool, XML_T('\0')))
        return 0;
      prefix = (PREFIX *)lookup(parser, &_Mut dtd->prefixes, poolStart(&_Mut dtd->pool),
                                sizeof(PREFIX));
      if (! prefix)
        return 0;
      if (prefix->name == poolStart(&_Mut dtd->pool))
        poolFinish(&_Mut dtd->pool);
      else
        poolDiscard(&_Mut dtd->pool);
      elementType->prefix = prefix;
      break;
    }
  }
  return 1;
}

_Safe static ATTRIBUTE_ID *
getAttributeId(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char *start,
               const char *end) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return nullptr;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  ATTRIBUTE_ID *id;
  const XML_Char *name;
  if (! poolAppendChar(&_Mut dtd->pool, XML_T('\0')))
    return nullptr;
  name = poolStoreString(&_Mut dtd->pool, enc, start, end);
  if (! name)
    return nullptr;
  /* skip quotation mark - its storage will be reused (like in name[-1]) */
  _Unsafe ++name;
  _Unsafe id = (ATTRIBUTE_ID *)lookup(parser, &_Mut dtd->attributeIds, name,
                              sizeof(ATTRIBUTE_ID));
  if (! id)
    return nullptr;
  if (_Unsafe((id->name != name)))
    poolDiscard(&_Mut dtd->pool);
  else {
    poolFinish(&_Mut dtd->pool);
    if (! parser->m_ns)
      ;
    else if (name[0] == XML_T(ASCII_x) && name[1] == XML_T(ASCII_m)
             && name[2] == XML_T(ASCII_l) && name[3] == XML_T(ASCII_n)
             && name[4] == XML_T(ASCII_s)
             && (name[5] == XML_T('\0') || name[5] == XML_T(ASCII_COLON))) {
      if (name[5] == XML_T('\0'))
        _Unsafe id->prefix = _Unsafe(&dtd->defaultPrefix);
      else
        _Unsafe id->prefix = (PREFIX *)lookup(parser, &_Mut dtd->prefixes, name + 6,
                                      sizeof(PREFIX));
      _Unsafe id->xmlns = XML_TRUE;
    } else {
      int i;
      for (i = 0; name[i]; i++) {
        /* Detect and prevent signed integer overflow */
        if (i == INT_MAX) {
          return nullptr;
        }
        /* attributes without prefix are *not* in the default namespace */
        if (name[i] == XML_T(ASCII_COLON)) {
          if (! poolAppendChars(&_Mut dtd->pool, name, (size_t)i))
            return nullptr;
          if (! poolAppendChar(&_Mut dtd->pool, XML_T('\0')))
            return nullptr;
          _Unsafe id->prefix = (PREFIX *)lookup(parser, &_Mut dtd->prefixes,
                                        poolStart(&_Mut dtd->pool), sizeof(PREFIX));
          if (_Unsafe((! id->prefix)))
            return nullptr;
          if (_Unsafe((id->prefix->name == poolStart(&_Mut dtd->pool))))
            poolFinish(&_Mut dtd->pool);
          else
            poolDiscard(&_Mut dtd->pool);
          break;
        }
      }
    }
  }
  return id;
}

#define CONTEXT_SEP XML_T(ASCII_FF)

_Safe static const XML_Char *
getContext(XML_Parser _Borrow parser) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return nullptr;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  HASH_TABLE_ITER iter = {nullptr, nullptr};
  XML_Bool needSep = XML_FALSE;

  if (dtd->defaultPrefix.binding) {
    if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T(ASCII_EQUALS)))
      return nullptr;
    size_t len = _Unsafe((dtd->defaultPrefix.binding->uriLen));
    if (parser->m_namespaceSeparator)
      len--;
    const XML_Char *defaultUri
        = _Unsafe((const XML_Char *)&_Const *dtd->defaultPrefix.binding->uri);
    if (defaultUri == nullptr)
      return nullptr;
    if (! poolAppendChars(&_Mut parser->m_tempPool, defaultUri, len)) {
      /* Because of memory caching, I don't believe this line can be
       * executed.
       *
       * This is part of a loop copying the default prefix binding
       * URI into the parser's temporary string pool.  Previously,
       * that URI was copied into the same string pool, with a
       * terminating NUL character, as part of setContext().  When
       * the pool was cleared, that leaves a block definitely big
       * enough to hold the URI on the free block list of the pool.
       * The URI copy in getContext() therefore cannot run out of
       * memory.
       *
       * If the pool is used between the setContext() and
       * getContext() calls, the worst it can do is leave a bigger
       * block on the front of the free list.  Given that this is
       * all somewhat inobvious and program logic can be changed, we
       * don't delete the line but we do exclude it from the test
       * coverage statistics.
       */
      return nullptr; /* LCOV_EXCL_LINE */
    }
    needSep = XML_TRUE;
  }

  hashTableIterInit(&_Mut iter, &_Mut dtd->prefixes);
  for (;;) {
    PREFIX *prefix = _Unsafe(((PREFIX *)hashTableIterNext(&_Mut iter)));
    if (! prefix)
      break;
    if (_Unsafe((! prefix->binding))) {
      /* This test appears to be (justifiable) paranoia.  There does
       * not seem to be a way of injecting a prefix without a binding
       * that doesn't get errored long before this function is called.
       * The test should remain for safety's sake, so we instead
       * exclude the following line from the coverage statistics.
       */
      continue; /* LCOV_EXCL_LINE */
    }
    if (needSep && ! poolAppendChar(&_Mut parser->m_tempPool, CONTEXT_SEP))
      return nullptr;
    if (_Unsafe((! poolAppendChars(&_Mut parser->m_tempPool, prefix->name,
                          _Unsafe(xcslen(prefix->name))))))
      return nullptr;
    if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T(ASCII_EQUALS)))
      return nullptr;
    size_t len = _Unsafe((prefix->binding->uriLen));
    if (parser->m_namespaceSeparator)
      len--;
    const XML_Char *prefixUri
        = _Unsafe((const XML_Char *)&_Const *prefix->binding->uri);
    if (prefixUri == nullptr)
      return nullptr;
    if (! poolAppendChars(&_Mut parser->m_tempPool, prefixUri, len))
      return nullptr;
    needSep = XML_TRUE;
  }

  hashTableIterInit(&_Mut iter, &_Mut dtd->generalEntities);
  for (;;) {
    ENTITY *e = _Unsafe(((ENTITY *)hashTableIterNext(&_Mut iter)));
    if (! e)
      break;
    if (_Unsafe((! e->open)))
      continue;
    if (needSep && ! poolAppendChar(&_Mut parser->m_tempPool, CONTEXT_SEP))
      return nullptr;
    if (_Unsafe((! poolAppendChars(&_Mut parser->m_tempPool, e->name, _Unsafe(xcslen(e->name))))))
      return nullptr;
    needSep = XML_TRUE;
  }

  if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T('\0')))
    return nullptr;
  return parser->m_tempPool.start;
}

_Safe static XML_Bool
setContext(XML_Parser _Borrow parser, const XML_Char *_Nonnull context) {
  if (context == nullptr) {
    return XML_FALSE;
  }

  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return XML_FALSE;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  const XML_Char *s = context;

  while (_Unsafe((*context != XML_T('\0')))) {
    if (_Unsafe((*s == CONTEXT_SEP || *s == XML_T('\0')))) {
      ENTITY *e;
      if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T('\0')))
        return XML_FALSE;
      const XML_Char *entityName = poolStart(&_Mut parser->m_tempPool);
      _Unsafe e = (ENTITY *)lookup(parser, &_Mut dtd->generalEntities, entityName, 0);
      if (e)
        _Unsafe e->open = true;
      if (_Unsafe((*s != XML_T('\0'))))
        _Unsafe s++;
      context = s;
      poolDiscard(&_Mut parser->m_tempPool);
    } else if (_Unsafe((*s == XML_T(ASCII_EQUALS)))) {
      PREFIX *prefix;
      if (poolLength(&_Mut parser->m_tempPool) == 0)
        prefix = _Unsafe(&dtd->defaultPrefix);
      else {
        if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T('\0')))
          return XML_FALSE;
        const XML_Char *const tempName = poolStart(&_Mut parser->m_tempPool);
        if (tempName == nullptr)
          return XML_FALSE;
        const XML_Char *const prefixName
            = poolCopyStringNoFinish(&_Mut dtd->pool, tempName);
        if (! prefixName) {
          return XML_FALSE;
        }

        _Unsafe prefix = (PREFIX *)lookup(parser, &_Mut dtd->prefixes, prefixName,
                                  sizeof(PREFIX));

        const bool prefixNameUsed = _Unsafe(((bool)(prefix && prefix->name == prefixName)));
        if (prefixNameUsed)
          poolFinish(&_Mut dtd->pool);
        else
          poolDiscard(&_Mut dtd->pool);

        if (! prefix)
          return XML_FALSE;

        poolDiscard(&_Mut parser->m_tempPool);
      }
      _Unsafe for (context = s + 1; *context != CONTEXT_SEP && *context != XML_T('\0');
           context++)
        if (! poolAppendChar(&_Mut parser->m_tempPool, *context))
          return XML_FALSE;
      if (! poolAppendChar(&_Mut parser->m_tempPool, XML_T('\0')))
        return XML_FALSE;
      const XML_Char *bindingUri = poolStart(&_Mut parser->m_tempPool);
      __auto_type bsc_h36 = _Unsafe(&parser->m_inheritedBindings);
      if (addBinding(parser, prefix, nullptr, bindingUri,
                     bsc_h36)
          != XML_ERROR_NONE)
        return XML_FALSE;
      poolDiscard(&_Mut parser->m_tempPool);
      if (_Unsafe((*context != XML_T('\0'))))
        _Unsafe ++context;
      s = context;
    } else {
      if (_Unsafe((! poolAppendChar(&_Mut parser->m_tempPool, *s))))
        return XML_FALSE;
      _Unsafe s++;
    }
  }
  return XML_TRUE;
}

_Safe static void FASTCALL
normalizePublicId(XML_Char *_Nonnull publicId) {
  XML_Char *p = publicId;
  XML_Char *s;
  _Unsafe for (s = publicId; *s; s++) {
    switch (*s) {
    case 0x20:
    case 0xD:
    case 0xA:
      if (p != publicId && p[-1] != 0x20)
        *p++ = 0x20;
      break;
    default:
      *p++ = *s;
    }
  }
  if (p != publicId && p[-1] == 0x20)
    _Unsafe --p;
  _Unsafe *p = XML_T('\0');
}

_Safe static DTD *
dtdCreate(XML_Parser _Borrow parser) {
  DTD *p = _Unsafe((DTD *)(MALLOC(parser, sizeof(DTD))));
  if (p == nullptr)
    return p;
  _Unsafe poolInit(&_Mut p->pool, RAW_PARSER(parser));
  _Unsafe poolInit(&_Mut p->entityValuePool, RAW_PARSER(parser));
  _Unsafe hashTableInit(&_Mut p->generalEntities, RAW_PARSER(parser));
  _Unsafe hashTableInit(&_Mut p->elementTypes, RAW_PARSER(parser));
  _Unsafe hashTableInit(&_Mut p->attributeIds, RAW_PARSER(parser));
  _Unsafe hashTableInit(&_Mut p->prefixes, RAW_PARSER(parser));
#ifdef XML_DTD
  _Unsafe p->paramEntityRead = XML_FALSE;
  _Unsafe hashTableInit(&_Mut p->paramEntities, RAW_PARSER(parser));
#endif /* XML_DTD */
  _Unsafe p->defaultPrefix.name = nullptr;
  _Unsafe p->defaultPrefix.binding = nullptr;

  _Unsafe p->in_eldecl = XML_FALSE;
  _Unsafe p->scaffIndex = nullptr;
  _Unsafe p->scaffIndexSize = 0;
  _Unsafe p->scaffold = nullptr;
  _Unsafe p->scaffLevel = 0;
  _Unsafe p->scaffSize = 0;
  _Unsafe p->scaffCount = 0;
  _Unsafe p->contentStringLen = 0;

  _Unsafe p->keepProcessing = XML_TRUE;
  _Unsafe p->hasParamEntityRefs = XML_FALSE;
  _Unsafe p->standalone = XML_FALSE;
  return p;
}

_Safe static void
dtdReset(DTD *_Borrow p, XML_Parser _Borrow parser) {
  HASH_TABLE_ITER iter = {nullptr, nullptr};
  hashTableIterInit(&_Mut iter, &_Mut p->elementTypes);
  for (;;) {
    ELEMENT_TYPE *e = _Unsafe(((ELEMENT_TYPE *)hashTableIterNext(&_Mut iter)));
    if (! e)
      break;
    hashTableDestroy(_Unsafe(&_Mut e->defaultAttForName));
    _Unsafe FREE_ARRAY(parser, e->defaultAtts, DEFAULT_ATTRIBUTE);
  }
  hashTableClear(&_Mut p->generalEntities);
#ifdef XML_DTD
  p->paramEntityRead = XML_FALSE;
  hashTableClear(&_Mut p->paramEntities);
#endif /* XML_DTD */
  hashTableClear(&_Mut p->elementTypes);
  hashTableClear(&_Mut p->attributeIds);
  hashTableClear(&_Mut p->prefixes);
  poolClear(&_Mut p->pool);
  poolClear(&_Mut p->entityValuePool);
  p->defaultPrefix.name = nullptr;
  p->defaultPrefix.binding = nullptr;

  p->in_eldecl = XML_FALSE;

  FREE(parser, p->scaffIndex);
  p->scaffIndex = nullptr;
  p->scaffIndexSize = 0;
  _Unsafe FREE(parser, p->scaffold);
  p->scaffold = nullptr;

  p->scaffLevel = 0;
  p->scaffSize = 0;
  p->scaffCount = 0;
  p->contentStringLen = 0;

  p->keepProcessing = XML_TRUE;
  p->hasParamEntityRefs = XML_FALSE;
  p->standalone = XML_FALSE;
}

_Safe static void
dtdDestroy(DTD *p, XML_Bool isDocEntity, XML_Parser _Borrow parser) {
  HASH_TABLE_ITER iter = {nullptr, nullptr};
  _Unsafe hashTableIterInit(&_Mut iter, &_Mut p->elementTypes);
  for (;;) {
    ELEMENT_TYPE *e = _Unsafe(((ELEMENT_TYPE *)hashTableIterNext(&_Mut iter)));
    if (! e)
      break;
    hashTableDestroy(_Unsafe(&_Mut e->defaultAttForName));
    _Unsafe FREE_ARRAY(parser, e->defaultAtts, DEFAULT_ATTRIBUTE);
  }
  _Unsafe hashTableDestroy(&_Mut p->generalEntities);
#ifdef XML_DTD
  _Unsafe hashTableDestroy(&_Mut p->paramEntities);
#endif /* XML_DTD */
  _Unsafe hashTableDestroy(&_Mut p->elementTypes);
  _Unsafe hashTableDestroy(&_Mut p->attributeIds);
  _Unsafe hashTableDestroy(&_Mut p->prefixes);
  _Unsafe poolDestroy(&_Mut p->pool);
  _Unsafe poolDestroy(&_Mut p->entityValuePool);
  if (isDocEntity) {
    _Unsafe FREE(parser, p->scaffIndex);
    _Unsafe FREE(parser, p->scaffold);
  }
  _Unsafe FREE(parser, p);
}

/* Do a deep copy of the DTD. Return 0 for out of memory, non-zero otherwise.
   The new DTD has already been initialized.
*/
_Safe static int
dtdCopy(XML_Parser _Borrow oldParser, DTD *_Borrow newDtd, const DTD *_Borrow oldDtd,
        XML_Parser _Borrow parser) {
  HASH_TABLE_ITER iter = {nullptr, nullptr};

  /* Copy the prefix table. */

  hashTableIterInit(&_Mut iter, &_Const oldDtd->prefixes);
  for (;;) {
    const XML_Char *name;
    const PREFIX *oldP = _Unsafe(((PREFIX *)hashTableIterNext(&_Mut iter)));
    if (! oldP)
      break;
    _Unsafe name = poolCopyString(&_Mut newDtd->pool, oldP->name);
    if (! name)
      return 0;
    if (! lookup(oldParser, &_Mut newDtd->prefixes, name, sizeof(PREFIX)))
      return 0;
  }

  hashTableIterInit(&_Mut iter, &_Const oldDtd->attributeIds);

  /* Copy the attribute id table. */

  for (;;) {
    ATTRIBUTE_ID *newA;
    const XML_Char *name;
    const ATTRIBUTE_ID *oldA = _Unsafe(((ATTRIBUTE_ID *)hashTableIterNext(&_Mut iter)));

    if (! oldA)
      break;
    /* Remember to allocate the scratch byte before the name. */
    if (! poolAppendChar(&_Mut newDtd->pool, XML_T('\0')))
      return 0;
    _Unsafe name = poolCopyString(&_Mut newDtd->pool, oldA->name);
    if (! name)
      return 0;
    _Unsafe ++name;
    _Unsafe newA = (ATTRIBUTE_ID *)lookup(oldParser, &_Mut newDtd->attributeIds, name,
                                  sizeof(ATTRIBUTE_ID));
    if (! newA)
      return 0;
    _Unsafe newA->maybeTokenized = oldA->maybeTokenized;
    if (_Unsafe((oldA->prefix))) {
      _Unsafe newA->xmlns = oldA->xmlns;
      if (_Unsafe((oldA->prefix == _Unsafe(&oldDtd->defaultPrefix))))
        _Unsafe newA->prefix = _Unsafe(&newDtd->defaultPrefix);
      else
        _Unsafe newA->prefix = (PREFIX *)lookup(oldParser, &_Mut newDtd->prefixes,
                                        oldA->prefix->name, 0);
    }
  }

  /* Copy the element type table. */

  hashTableIterInit(&_Mut iter, &_Const oldDtd->elementTypes);

  for (;;) {
    ELEMENT_TYPE *newE;
    const XML_Char *name;
    const ELEMENT_TYPE *oldE = _Unsafe(((ELEMENT_TYPE *)hashTableIterNext(&_Mut iter)));
    if (! oldE)
      break;
    _Unsafe name = poolCopyString(&_Mut newDtd->pool, oldE->name);
    if (! name)
      return 0;
    _Unsafe newE = (ELEMENT_TYPE *)lookup(oldParser, &_Mut newDtd->elementTypes, name,
                                  sizeof(ELEMENT_TYPE));
    if (! newE)
      return 0;

    if (_Unsafe((! newE->defaultAttForName.parser)))
      hashTableInit(_Unsafe(&_Mut newE->defaultAttForName), RAW_PARSER(parser));

    if (_Unsafe((oldE->nDefaultAtts))) {
      /* Detect and prevent integer overflow. */
      if (_Unsafe((oldE->nDefaultAtts > SIZE_MAX / sizeof(DEFAULT_ATTRIBUTE)))) {
        return 0;
      }
      DEFAULT_ATTRIBUTE *newDefaultAtts
          = _Unsafe((DEFAULT_ATTRIBUTE *)(MALLOC(parser, oldE->nDefaultAtts * sizeof(DEFAULT_ATTRIBUTE))));
      if (! newDefaultAtts) {
        return 0;
      }
      _Unsafe newE->defaultAtts = __take_array_from_raw(newDefaultAtts);
    }
    if (_Unsafe((oldE->idAtt)))
      _Unsafe newE->idAtt = (ATTRIBUTE_ID *)lookup(oldParser, &_Mut newDtd->attributeIds,
                                           oldE->idAtt->name, 0);
    _Unsafe newE->allocDefaultAtts = newE->nDefaultAtts = oldE->nDefaultAtts;
    if (_Unsafe((oldE->prefix)))
      _Unsafe newE->prefix = (PREFIX *)lookup(oldParser, &_Mut newDtd->prefixes,
                                      oldE->prefix->name, 0);
    _Unsafe for (size_t i = 0; i < newE->nDefaultAtts; i++) {
      const XML_Char *const attributeName = oldE->defaultAtts[i].id->name;
      newE->defaultAtts[i].id = (ATTRIBUTE_ID *)lookup(
          oldParser, &_Mut newDtd->attributeIds, attributeName, 0);
      newE->defaultAtts[i].isCdata = oldE->defaultAtts[i].isCdata;
      if (oldE->defaultAtts[i].value) {
        newE->defaultAtts[i].value
            = poolCopyString(&_Mut newDtd->pool, oldE->defaultAtts[i].value);
        if (! newE->defaultAtts[i].value)
          return 0;
      } else
        newE->defaultAtts[i].value = nullptr;

      NAME_AND_DEFAULT_ATTRIBUTE *const nameAndDefaultAttribute
          = (NAME_AND_DEFAULT_ATTRIBUTE *)lookup(
              parser, _Unsafe(&_Mut newE->defaultAttForName), attributeName,
              sizeof(NAME_AND_DEFAULT_ATTRIBUTE));
      if (! nameAndDefaultAttribute) {
        return 0;
      }

      // NOTE: The XML 1.0r4 spec says:
      // "When more than one definition is provided for the same attribute of a
      // given element type, the first declaration is binding and later
      // declarations are ignored."
      if (! nameAndDefaultAttribute->initialized) {
        nameAndDefaultAttribute->attIndex = i;
        nameAndDefaultAttribute->initialized = true;
      }
    }
  }

  /* Copy the entity tables. */
  if (! copyEntityTable(oldParser, &_Mut newDtd->generalEntities, &_Mut newDtd->pool,
                        &_Const oldDtd->generalEntities))
    return 0;

#ifdef XML_DTD
  if (! copyEntityTable(oldParser, &_Mut newDtd->paramEntities, &_Mut newDtd->pool,
                        &_Const oldDtd->paramEntities))
    return 0;
  newDtd->paramEntityRead = oldDtd->paramEntityRead;
#endif /* XML_DTD */

  newDtd->keepProcessing = oldDtd->keepProcessing;
  newDtd->hasParamEntityRefs = oldDtd->hasParamEntityRefs;
  newDtd->standalone = oldDtd->standalone;

  /* Don't want deep copying for scaffolding */
  newDtd->in_eldecl = oldDtd->in_eldecl;
  newDtd->scaffold = oldDtd->scaffold;
  newDtd->contentStringLen = oldDtd->contentStringLen;
  newDtd->scaffSize = oldDtd->scaffSize;
  newDtd->scaffLevel = oldDtd->scaffLevel;
  newDtd->scaffIndex = oldDtd->scaffIndex;
  newDtd->scaffIndexSize = oldDtd->scaffIndexSize;

  return 1;
} /* End dtdCopy */

_Safe static int
copyEntityTable(XML_Parser _Borrow oldParser, HASH_TABLE *_Borrow newTable,
                STRING_POOL *_Borrow newPool, const HASH_TABLE *_Borrow oldTable) {
  HASH_TABLE_ITER iter = {nullptr, nullptr};
  const XML_Char *cachedOldBase = nullptr;
  const XML_Char *cachedNewBase = nullptr;

  hashTableIterInit(&_Mut iter, oldTable);

  for (;;) {
    ENTITY *newE;
    const XML_Char *name;
    const ENTITY *oldE = _Unsafe(((ENTITY *)hashTableIterNext(&_Mut iter)));
    if (! oldE)
      break;
    _Unsafe name = poolCopyString(newPool, oldE->name);
    if (! name)
      return 0;
    _Unsafe newE = (ENTITY *)lookup(oldParser, newTable, name, sizeof(ENTITY));
    if (! newE)
      return 0;
    if (_Unsafe((oldE->systemId))) {
      const XML_Char *tem = _Unsafe((poolCopyString(newPool, oldE->systemId)));
      if (! tem)
        return 0;
      _Unsafe newE->systemId = tem;
      if (_Unsafe((oldE->base))) {
        if (_Unsafe((oldE->base == cachedOldBase)))
          _Unsafe newE->base = cachedNewBase;
        else {
          _Unsafe cachedOldBase = oldE->base;
          tem = poolCopyString(newPool, cachedOldBase);
          if (! tem)
            return 0;
          _Unsafe cachedNewBase = newE->base = tem;
        }
      }
      if (_Unsafe((oldE->publicId))) {
        _Unsafe tem = poolCopyString(newPool, oldE->publicId);
        if (! tem)
          return 0;
        _Unsafe newE->publicId = tem;
      }
    } else {
      const XML_Char *tem
          = _Unsafe((poolCopyStringN(newPool, oldE->textPtr, oldE->textLen)));
      if (! tem)
        return 0;
      _Unsafe newE->textPtr = tem;
      _Unsafe newE->textLen = oldE->textLen;
    }
    if (_Unsafe((oldE->notation))) {
      const XML_Char *tem = _Unsafe((poolCopyString(newPool, oldE->notation)));
      if (! tem)
        return 0;
      _Unsafe newE->notation = tem;
    }
    _Unsafe newE->is_param = oldE->is_param;
    _Unsafe newE->is_internal = oldE->is_internal;
  }
  return 1;
}

#define INIT_POWER 6

// Compares two strings `s1` and `s2` whereas:
// - `s2` is zero-terminated but
// - `s1` is made up of exactly (not just up to) `s1len` non-zero characters.
_Safe static XML_Bool FASTCALL
keyeq(KEY s1, size_t s1len, KEY _Nonnull s2) {
#ifdef XML_UNICODE
#  ifdef XML_UNICODE_WCHAR_T
  return (_Unsafe(wcsncmp(s1, s2, s1len)) == 0 && s2[s1len] == L'\0') ? XML_TRUE
                                                             : XML_FALSE;
#  else
  for (; s1len > 0 && *s1 == *s2; s1len--, s1++, s2++)
    ; /* no loop body! */
  return ((s1len == 0) && (*s2 == 0)) ? XML_TRUE : XML_FALSE;
#  endif
#else
  return (XML_Bool)((_Unsafe(strncmp(s1, s2, s1len)) == 0 && s2[s1len] == '\0')
                        ? XML_TRUE
                        : XML_FALSE);
#endif
}

_Safe static size_t
keylen(KEY _Nonnull s) {
  return _Unsafe(xcslen(s));
}

_Safe static void
copy_salt_to_sipkey(XML_Parser _Borrow parser, struct sipkey *_Borrow __attribute__((ensure_init)) key) {
  XML_Parser _Borrow rootParser = getRootParserOf(parser, nullptr);
  _Unsafe assert(! rootParser->m_parentParser);

  *key = rootParser->m_hash_secret_salt_128;
}

_Safe static unsigned long FASTCALL
hash(XML_Parser _Borrow parser, KEY _Nonnull s, size_t keyLen) {
  struct siphash state = {0, 0, 0, 0, {0}, nullptr, 0};
  struct sipkey key = {{0, 0}};
  (void)sip24_valid;
  copy_salt_to_sipkey(parser, &_Mut key);
  _Unsafe sip24_init(_Unsafe(&state), _Unsafe(&key));
  _Unsafe sip24_update(_Unsafe(&state), s, keyLen * sizeof(XML_Char));
  return (unsigned long)_Unsafe(sip24_final(_Unsafe(&state)));
}

// Function `lookupWithLength` can be used to either…
//
// a) check whether an element with key `name` exists in the given hash table
//    (read-only mode where `createSize == 0`) or
//
// b) check whether an element with key `name` exists in the given hash table
//    *and* insert it if missing (i.e. read-write mode where `createSize != 0`.
//
// When inserting, a block of `createSize` number of bytes will be allocated
// and set to zero, and the resulting block of memory will be considered
// to start with a `NAMED` structure, and `->name = name;` is performed.
// The fact that all other bytes in the structure are initially zero can
// be used to tell cases "existed and found" and "newly inserted" apart
// with the structure returned.
//
// NOTE: Read-only lookup does not need zero-terminated keys but
//       read-write mode does, because keys can be re-hashed later and the
//       hash table does not store key length information.
//
_Safe static NAMED *
lookupWithLength(XML_Parser _Borrow parser, HASH_TABLE *_Borrow table, KEY _Nonnull name, size_t nameLen,
                 size_t createSize) {
  size_t i;
  if (table->size == 0) {
    size_t tsize;
    if (! createSize)
      return nullptr;
    table->power = INIT_POWER;
    /* table->size is a power of 2 */
    table->size = (size_t)1 << INIT_POWER;
    tsize = table->size * sizeof(NAMED *);
    table->v = TAKE_ARRAY(NAMED *, MALLOC(PARSER_OF(table), tsize));
    if (table->v == nullptr) {
      table->size = 0;
      return nullptr;
    }
    _Unsafe memset(_Unsafe((NAMED **)&_Mut *table->v), 0, tsize);
    i = hash(parser, name, nameLen) & ((unsigned long)table->size - 1);
  } else {
    if (table->v == nullptr)
      return nullptr;
    unsigned long h = hash(parser, name, nameLen);
    unsigned long mask = (unsigned long)table->size - 1;
    unsigned char step = 0;
    i = h & mask;
    while (table->v[i]) {
      if (_Unsafe((keyeq(name, nameLen, table->v[i]->name))))
        return table->v[i];
      if (! step)
        step = PROBE_STEP(h, mask, table->power);
      i < step ? (i += table->size - step) : (i -= step);
    }
    if (! createSize)
      return nullptr;

    /* check for overflow (table is half full) */
    if (table->used >> (table->power - 1)) {
      unsigned char newPower = (unsigned char)(table->power + 1);

      /* Detect and prevent invalid shift */
      if (newPower >= sizeof(unsigned long) * 8 /* bits per byte */) {
        return nullptr;
      }

      size_t newSize = (size_t)1 << newPower;
      unsigned long newMask = (unsigned long)newSize - 1;

      /* Detect and prevent integer overflow */
      if (newSize > SIZE_MAX / sizeof(NAMED *)) {
        return nullptr;
      }

      size_t tsize = newSize * sizeof(NAMED *);
      NAMED **newV = _Unsafe((NAMED **)(MALLOC(PARSER_OF(table), tsize)));
      if (! newV)
        return nullptr;
      _Unsafe memset(newV, 0, tsize);
      for (i = 0; i < table->size; i++)
        if (table->v[i]) {
          KEY const key = _Unsafe((table->v[i]->name));
          if (key == nullptr) {
            _Unsafe FREE(PARSER_OF(table), newV);
            return nullptr;
          }
          unsigned long newHash = hash(parser, key, keylen(key));
          size_t j = newHash & newMask;
          step = 0;
          while (newV[j]) {
            if (! step)
              step = PROBE_STEP(newHash, newMask, newPower);
            j < step ? (j += newSize - step) : (j -= step);
          }
          newV[j] = table->v[i];
        }
      FREE_ARRAY(PARSER_OF(table), table->v, NAMED *);
      table->v = TAKE_ARRAY(NAMED *, newV);
      if (table->v == nullptr)
        return nullptr;
      table->power = newPower;
      table->size = newSize;
      i = h & newMask;
      step = 0;
      while (table->v[i]) {
        if (! step)
          step = PROBE_STEP(h, newMask, newPower);
        i < step ? (i += newSize - step) : (i -= step);
      }
    }
  }
  _Unsafe assert(createSize >= sizeof(NAMED));
  _Unsafe table->v[i] = MALLOC(PARSER_OF(table), createSize);
  if (! table->v[i])
    return nullptr;
  _Unsafe memset(table->v[i], 0, createSize);
  _Unsafe table->v[i]->name = name; // NOTE: This requires and assumes zero termination!
  (table->used)++;
  return table->v[i];
}

// Function `lookup` can be used to either…
//
// a) check whether an element with key `name` exists in the given hash table
//    (read-only mode where `createSize == 0`) or
//
// b) check whether an element with key `name` exists in the given hash table
//    *and* insert it if missing (i.e. read-write mode where `createSize != 0`.
//
// When inserting, a block of `createSize` number of bytes will be allocated
// and set to zero, and the resulting block of memory will be considered
// to start with a `NAMED` structure, and `->name = name;` is performed.
// The fact that all other bytes in the structure are initially zero can
// be used to tell cases "existed and found" and "newly inserted" apart
// with the structure returned.
//
_Safe static NAMED *
lookup(XML_Parser _Borrow parser, HASH_TABLE *_Borrow table, KEY _Nonnull name, size_t createSize) {
  return lookupWithLength(parser, table, name, keylen(name), createSize);
}

_Safe static void FASTCALL
hashTableClear(HASH_TABLE *_Borrow table) {
  size_t i;
  if (table->v == nullptr)
    return;
  for (i = 0; i < table->size; i++) {
    _Unsafe FREE(PARSER_OF(table), table->v[i]);
    table->v[i] = nullptr;
  }
  table->used = 0;
}

_Safe static void FASTCALL
hashTableDestroy(HASH_TABLE *_Borrow table) {
  size_t i;
  if (table->v == nullptr)
    return;
  for (i = 0; i < table->size; i++)
    _Unsafe FREE(PARSER_OF(table), table->v[i]);
  FREE_ARRAY(PARSER_OF(table), table->v, NAMED *);
}

_Safe static void FASTCALL
hashTableInit(HASH_TABLE *_Borrow p, XML_Parser rawParser) {
  p->power = 0;
  p->size = 0;
  p->used = 0;
  p->v = nullptr;
  p->parser = rawParser;
}

_Safe static void FASTCALL
hashTableIterInit(HASH_TABLE_ITER *_Borrow iter, const HASH_TABLE *_Borrow table) {
  iter->p = (table->v == nullptr) ? nullptr : _Unsafe((NAMED **)&_Const *table->v);
  iter->end = iter->p ? iter->p + table->size : nullptr;
}

_Safe static NAMED *FASTCALL
hashTableIterNext(HASH_TABLE_ITER *_Borrow iter) {
  while (iter->p != iter->end) {
    NAMED *tem = _Unsafe((*(iter->p)++));
    if (tem)
      return tem;
  }
  return nullptr;
}

_Safe static void FASTCALL
poolInit(STRING_POOL *_Borrow pool, XML_Parser rawParser) {
  pool->blocks = nullptr;
  pool->freeBlocks = nullptr;
  pool->start = nullptr;
  pool->ptr = nullptr;
  pool->end = nullptr;
  pool->parser = rawParser;
}

_Safe static void FASTCALL
poolClear(STRING_POOL *_Borrow pool) {
  if (! pool->freeBlocks)
    pool->freeBlocks = pool->blocks;
  else {
    BLOCK *p = pool->blocks;
    while (p) {
      BLOCK *tem = _Unsafe((p->next));
      _Unsafe p->next = pool->freeBlocks;
      pool->freeBlocks = p;
      p = tem;
    }
  }
  pool->blocks = nullptr;
  pool->start = nullptr;
  pool->ptr = nullptr;
  pool->end = nullptr;
}

_Safe static void FASTCALL
poolDestroy(STRING_POOL *_Borrow pool) {
  BLOCK *p = pool->blocks;
  while (p) {
    BLOCK *tem = _Unsafe((p->next));
    _Unsafe FREE(PARSER_OF(pool), p);
    p = tem;
  }
  p = pool->freeBlocks;
  while (p) {
    BLOCK *tem = _Unsafe((p->next));
    _Unsafe FREE(PARSER_OF(pool), p);
    p = tem;
  }
}

_Safe static XML_Char *
poolAppend(STRING_POOL *_Borrow pool, const ENCODING *_Nonnull enc, const char *ptr,
           const char *end) {
  if (! pool->ptr && ! poolGrow(pool))
    return nullptr;
  for (;;) {
    const enum XML_Convert_Result convert_res = _Unsafe(XmlConvert(
        enc, _Unsafe(&ptr), end, (ICHAR **)&(pool->ptr), (const ICHAR *)pool->end));
    if ((convert_res == XML_CONVERT_COMPLETED)
        || (convert_res == XML_CONVERT_INPUT_INCOMPLETE))
      break;
    if (! poolGrow(pool))
      return nullptr;
  }
  return pool->start;
}

_Safe static const XML_Char *FASTCALL
poolCopyString(STRING_POOL *_Borrow pool, const XML_Char *_Nonnull s) {
  if (! poolAppendChars(pool, s, _Unsafe(xcslen(s)) + /*null terminator*/ 1))
    return nullptr;
  const XML_Char *copy = pool->start;
  poolFinish(pool);
  return copy;
}

// A version of `poolCopyString` that does not call `poolFinish`
// and reverts any partial advancement upon failure.
_Safe static const XML_Char *FASTCALL
poolCopyStringNoFinish(STRING_POOL *_Borrow pool, const XML_Char *_Nonnull s) {
  const XML_Char *const original = s;
  do {
    if (_Unsafe((! poolAppendChar(pool, *s)))) {
      // Revert any previously successful advancement
      const ptrdiff_t advancedBy = s - original;
      if (advancedBy > 0)
        pool->ptr -= advancedBy;
      return nullptr;
    }
  } while (_Unsafe((*s++)));
  return pool->start;
}

_Safe static const XML_Char *
poolCopyStringN(STRING_POOL *_Borrow pool, const XML_Char *_Nonnull s, int n) {
  if (! pool->ptr && ! poolGrow(pool)) {
    /* The following line is unreachable given the current usage of
     * poolCopyStringN().  Currently it is called from exactly one
     * place to copy the text of a simple general entity.  By that
     * point, the name of the entity is already stored in the pool, so
     * pool->ptr cannot be nullptr.
     *
     * If poolCopyStringN() is used elsewhere as it well might be,
     * this line may well become executable again.  Regardless, this
     * sort of check shouldn't be removed lightly, so we just exclude
     * it from the coverage statistics.
     */
    return nullptr; /* LCOV_EXCL_LINE */
  }
  if (n > 0 && ! poolAppendChars(pool, s, (size_t)n))
    return nullptr;
  const XML_Char *copy = pool->start;
  poolFinish(pool);
  return copy;
}

_Safe static const XML_Char *FASTCALL
poolAppendString(STRING_POOL *_Borrow pool, const XML_Char *_Nonnull s) {
  if (! poolAppendChars(pool, s, _Unsafe(xcslen(s))))
    return nullptr;
  return pool->start;
}

_Safe static XML_Char *
poolStoreString(STRING_POOL *_Borrow pool, const ENCODING *_Nonnull enc, const char *ptr,
                const char *end) {
  if (! poolAppend(pool, enc, ptr, end))
    return nullptr;
  if (! poolAppendChar(pool, 0))
    return nullptr;
  return pool->start;
}

_Safe static size_t
poolBytesToAllocateFor(int blockSize) {
  /* Unprotected math would be:
  ** return offsetof(BLOCK, s) + blockSize * sizeof(XML_Char);
  **
  ** Detect overflow, avoiding _signed_ overflow undefined behavior
  ** For a + b * c we check b * c in isolation first, so that addition of a
  ** on top has no chance of making us accept a small non-negative number
  */
  const size_t stretch = sizeof(XML_Char); /* can be 4 bytes */

  if (blockSize <= 0)
    return 0;

  if (blockSize > (int)(INT_MAX / stretch))
    return 0;

  {
    const int stretchedBlockSize = blockSize * (int)stretch;
    const int bytesToAllocate
        = (int)(offsetof(BLOCK, s) + (unsigned)stretchedBlockSize);
    if (bytesToAllocate < 0)
      return 0;

    return (size_t)bytesToAllocate;
  }
}

_Safe static XML_Bool FASTCALL
poolGrow(STRING_POOL *_Borrow pool) {
  if (pool->freeBlocks) {
    if (pool->start == nullptr) {
      pool->blocks = pool->freeBlocks;
      _Unsafe pool->freeBlocks = pool->freeBlocks->next;
      _Unsafe pool->blocks->next = nullptr;
      _Unsafe pool->start = pool->blocks->s;
      _Unsafe pool->end = pool->start + pool->blocks->size;
      pool->ptr = pool->start;
      return XML_TRUE;
    }
    if (_Unsafe((pool->end - pool->start < pool->freeBlocks->size))) {
      BLOCK *tem = _Unsafe((pool->freeBlocks->next));
      _Unsafe pool->freeBlocks->next = pool->blocks;
      pool->blocks = pool->freeBlocks;
      pool->freeBlocks = tem;
      _Unsafe memcpy(pool->blocks->s, pool->start,
             (pool->end - pool->start) * sizeof(XML_Char));
      _Unsafe pool->ptr = pool->blocks->s + EXPAT_SAFE_PTR_DIFF(pool->ptr, pool->start);
      _Unsafe pool->start = pool->blocks->s;
      _Unsafe pool->end = pool->start + pool->blocks->size;
      return XML_TRUE;
    }
  }
  if (_Unsafe((pool->blocks && pool->start == pool->blocks->s))) {
    BLOCK *temp;
    int blockSize = (int)((unsigned)(pool->end - pool->start) * 2U);
    size_t bytesToAllocate;

    /* NOTE: Needs to be calculated prior to calling `realloc`
             to avoid dangling pointers: */
    const ptrdiff_t offsetInsideBlock
        = EXPAT_SAFE_PTR_DIFF(pool->ptr, pool->start);

    if (blockSize < 0) {
      /* This condition traps a situation where either more than
       * INT_MAX/2 bytes have already been allocated.  This isn't
       * readily testable, since it is unlikely that an average
       * machine will have that much memory, so we exclude it from the
       * coverage statistics.
       */
      return XML_FALSE; /* LCOV_EXCL_LINE */
    }

    bytesToAllocate = poolBytesToAllocateFor(blockSize);
    if (bytesToAllocate == 0)
      return XML_FALSE;

    _Unsafe temp = REALLOC(PARSER_OF(pool), pool->blocks, bytesToAllocate);
    if (temp == nullptr)
      return XML_FALSE;
    pool->blocks = temp;
    _Unsafe pool->blocks->size = blockSize;
    _Unsafe pool->ptr = pool->blocks->s + offsetInsideBlock;
    _Unsafe pool->start = pool->blocks->s;
    pool->end = pool->start + blockSize;
  } else {
    BLOCK *tem;
    int blockSize = (int)(pool->end - pool->start);
    size_t bytesToAllocate;

    if (blockSize < 0) {
      /* This condition traps a situation where either more than
       * INT_MAX bytes have already been allocated (which is prevented
       * by various pieces of program logic, not least this one, never
       * mind the unlikelihood of actually having that much memory) or
       * the pool control fields have been corrupted (which could
       * conceivably happen in an extremely buggy user handler
       * function).  Either way it isn't readily testable, so we
       * exclude it from the coverage statistics.
       */
      return XML_FALSE; /* LCOV_EXCL_LINE */
    }

    if (blockSize < INIT_BLOCK_SIZE)
      blockSize = INIT_BLOCK_SIZE;
    else {
      /* Detect overflow, avoiding _signed_ overflow undefined behavior */
      if ((int)((unsigned)blockSize * 2U) < 0) {
        return XML_FALSE;
      }
      blockSize *= 2;
    }

    bytesToAllocate = poolBytesToAllocateFor(blockSize);
    if (bytesToAllocate == 0)
      return XML_FALSE;

    _Unsafe tem = MALLOC(PARSER_OF(pool), bytesToAllocate);
    if (! tem)
      return XML_FALSE;
    _Unsafe tem->size = blockSize;
    _Unsafe tem->next = pool->blocks;
    pool->blocks = tem;
    if (pool->ptr != pool->start)
      _Unsafe memcpy(tem->s, pool->start,
             EXPAT_SAFE_PTR_DIFF(pool->ptr, pool->start) * sizeof(XML_Char));
    _Unsafe pool->ptr = tem->s + EXPAT_SAFE_PTR_DIFF(pool->ptr, pool->start);
    _Unsafe pool->start = tem->s;
    _Unsafe pool->end = tem->s + blockSize;
  }
  return XML_TRUE;
}

_Safe static bool FASTCALL
poolGrowUntil(STRING_POOL *_Borrow pool, size_t needed) {
  for (;;) {
    const size_t available = (size_t)(pool->end - pool->ptr);
    if (available >= needed) {
      return true;
    }
    if (! poolGrow(pool)) {
      return false;
    }
  }
}

_Safe static int FASTCALL
nextScaffoldPart(XML_Parser _Borrow parser) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return -1;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  CONTENT_SCAFFOLD *me;
  int next;

  if (! dtd->scaffIndex) {
    /* Detect and prevent integer overflow. */
    if (parser->m_groupSize > SIZE_MAX / sizeof(int)) {
      return -1;
    }
    __auto_type bsc_h37 = parser->m_groupSize;
    _Unsafe dtd->scaffIndex = MALLOC(parser, bsc_h37 * sizeof(int));
    if (! dtd->scaffIndex)
      return -1;
    dtd->scaffIndexSize = parser->m_groupSize;
    dtd->scaffIndex[0] = 0;
  }

  // Will casting to int be safe further down?
  if (dtd->scaffCount > INT_MAX) {
    return -1;
  }

  if (dtd->scaffCount >= dtd->scaffSize) {
    CONTENT_SCAFFOLD *temp;
    if (dtd->scaffold) {
      /* Detect and prevent integer overflow */
      if (dtd->scaffSize > UINT_MAX / 2u) {
        return -1;
      }
      /* Detect and prevent integer overflow.
       * The preprocessor guard addresses the "always false" warning
       * from -Wtype-limits on platforms where
       * sizeof(unsigned int) < sizeof(size_t), e.g. on x86_64. */
#if UINT_MAX >= SIZE_MAX
      if (dtd->scaffSize > SIZE_MAX / 2u / sizeof(CONTENT_SCAFFOLD)) {
        return -1;
      }
#endif

      _Unsafe temp = REALLOC(parser, dtd->scaffold,
                     dtd->scaffSize * 2 * sizeof(CONTENT_SCAFFOLD));
      if (temp == nullptr)
        return -1;
      dtd->scaffSize *= 2;
    } else {
      _Unsafe temp = MALLOC(parser, INIT_SCAFFOLD_ELEMENTS * sizeof(CONTENT_SCAFFOLD));
      if (temp == nullptr)
        return -1;
      dtd->scaffSize = INIT_SCAFFOLD_ELEMENTS;
    }
    dtd->scaffold = temp;
  }
  next = (int)dtd->scaffCount;
  dtd->scaffCount++;
  _Unsafe me = &dtd->scaffold[next];
  if (dtd->scaffLevel) {
    CONTENT_SCAFFOLD *parent
        = _Unsafe((&dtd->scaffold[dtd->scaffIndex[dtd->scaffLevel - 1]]));
    if (_Unsafe((parent->lastchild))) {
      _Unsafe dtd->scaffold[parent->lastchild].nextsib = next;
    }
    if (_Unsafe((! parent->childcnt)))
      _Unsafe parent->firstchild = next;
    _Unsafe parent->lastchild = next;
    _Unsafe parent->childcnt++;
  }
  _Unsafe me->firstchild = me->lastchild = me->childcnt = me->nextsib = 0;
  return next;
}

_Safe static XML_Content *
build_model(XML_Parser _Borrow parser) {
  /* Function build_model transforms the existing parser->m_dtd->scaffold
   * array of CONTENT_SCAFFOLD tree nodes into a new array of
   * XML_Content tree nodes followed by a gapless list of zero-terminated
   * strings. */
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return nullptr;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  XML_Content *ret;
  XML_Char *str; /* the current string writing location */

  /* Detect and prevent integer overflow.
   * The preprocessor guard addresses the "always false" warning
   * from -Wtype-limits on platforms where
   * sizeof(unsigned int) < sizeof(size_t), e.g. on x86_64. */
#if UINT_MAX >= SIZE_MAX
  if (dtd->scaffCount > SIZE_MAX / sizeof(XML_Content)) {
    return nullptr;
  }
  if (dtd->contentStringLen > SIZE_MAX / sizeof(XML_Char)) {
    return nullptr;
  }
#endif
  if (dtd->scaffCount * sizeof(XML_Content)
      > SIZE_MAX - dtd->contentStringLen * sizeof(XML_Char)) {
    return nullptr;
  }

  const size_t allocsize = (dtd->scaffCount * sizeof(XML_Content)
                            + (dtd->contentStringLen * sizeof(XML_Char)));

  // NOTE: We are avoiding MALLOC(..) here to so that
  //       applications that are not using XML_FreeContentModel but plain
  //       free(..) or .free_fcn() to free the content model's memory are safe.
  _Unsafe ret = parser->m_mem.malloc_fcn(allocsize);
  if (! ret)
    return nullptr;

  /* What follows is an iterative implementation (of what was previously done
   * recursively in a dedicated function called "build_node".  The old recursive
   * build_node could be forced into stack exhaustion from input as small as a
   * few megabyte, and so that was a security issue.  Hence, a function call
   * stack is avoided now by resolving recursion.)
   *
   * The iterative approach works as follows:
   *
   * - We have two writing pointers, both walking up the result array; one does
   *   the work, the other creates "jobs" for its colleague to do, and leads
   *   the way:
   *
   *   - The faster one, pointer jobDest, always leads and writes "what job
   *     to do" by the other, once they reach that place in the
   *     array: leader "jobDest" stores the source node array index (relative
   *     to array dtd->scaffold) in field "numchildren".
   *
   *   - The slower one, pointer dest, looks at the value stored in the
   *     "numchildren" field (which actually holds a source node array index
   *     at that time) and puts the real data from dtd->scaffold in.
   *
   * - Before the loop starts, jobDest writes source array index 0
   *   (where the root node is located) so that dest will have something to do
   *   when it starts operation.
   *
   * - Whenever nodes with children are encountered, jobDest appends
   *   them as new jobs, in order.  As a result, tree node siblings are
   *   adjacent in the resulting array, for example:
   *
   *     [0] root, has two children
   *       [1] first child of 0, has three children
   *         [3] first child of 1, does not have children
   *         [4] second child of 1, does not have children
   *         [5] third child of 1, does not have children
   *       [2] second child of 0, does not have children
   *
   *   Or (the same data) presented in flat array view:
   *
   *     [0] root, has two children
   *
   *     [1] first child of 0, has three children
   *     [2] second child of 0, does not have children
   *
   *     [3] first child of 1, does not have children
   *     [4] second child of 1, does not have children
   *     [5] third child of 1, does not have children
   *
   * - The algorithm repeats until all target array indices have been processed.
   */
  XML_Content *dest = ret; /* tree node writing location, moves upwards */
  XML_Content *const destLimit = _Unsafe((&ret[dtd->scaffCount]));
  XML_Content *jobDest = ret; /* next free writing location in target array */
  _Unsafe str = (XML_Char *)&ret[dtd->scaffCount];

  /* Add the starting job, the root node (index 0) of the source tree  */
  _Unsafe { (jobDest++)->numchildren = 0; }

  _Unsafe for (; dest < destLimit; dest++) {
    /* Retrieve source tree array index from job storage */
    const int src_node = (int)dest->numchildren;

    /* Convert item */
    dest->type = dtd->scaffold[src_node].type;
    dest->quant = dtd->scaffold[src_node].quant;
    if (dest->type == XML_CTYPE_NAME) {
      const XML_Char *src;
      dest->name = str;
      src = dtd->scaffold[src_node].name;

      const size_t nameLen = _Unsafe(xcslen(src)) + /* null terminator*/ 1;

      // Detect and prevent integer overflow
      if (nameLen > SIZE_MAX / sizeof(XML_Char)) {
        // NOTE: We are avoiding FREE(..) here because the model
        //       is not being allocated with MALLOC(..) but with plain
        //       .malloc_fcn(..).
        parser->m_mem.free_fcn(ret);
        return nullptr;
      }

      _Unsafe memcpy(str, src, nameLen * sizeof(XML_Char));
      str += nameLen;

      dest->numchildren = 0;
      dest->children = nullptr;
    } else {
      unsigned int i;
      int cn;
      dest->name = nullptr;
      dest->numchildren = dtd->scaffold[src_node].childcnt;
      dest->children = jobDest;

      /* Append scaffold indices of children to array */
      for (i = 0, cn = dtd->scaffold[src_node].firstchild;
           i < dest->numchildren; i++, cn = dtd->scaffold[cn].nextsib)
        (jobDest++)->numchildren = (unsigned int)cn;
    }
  }

  return ret;
}

_Safe static ELEMENT_TYPE *
getElementType(XML_Parser _Borrow parser, const ENCODING *_Nonnull enc, const char *ptr,
               const char *end) {
  DTD *dtdRaw = parser->m_dtd;
  if (dtdRaw == nullptr)
    return nullptr;
  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
  const XML_Char *name = poolStoreString(&_Mut dtd->pool, enc, ptr, end);
  ELEMENT_TYPE *ret;

  if (! name)
    return nullptr;
  _Unsafe ret = (ELEMENT_TYPE *)lookup(parser, &_Mut dtd->elementTypes, name,
                               sizeof(ELEMENT_TYPE));
  if (! ret)
    return nullptr;
  if (_Unsafe((! ret->defaultAttForName.parser)))
    hashTableInit(_Unsafe(&_Mut ret->defaultAttForName), RAW_PARSER(getRootParserOf(parser, nullptr)));
  if (_Unsafe((ret->name != name)))
    poolDiscard(&_Mut dtd->pool);
  else {
    poolFinish(&_Mut dtd->pool);
    if (! setElementTypePrefix(parser, ret))
      return nullptr;
  }
  return ret;
}

_Safe static XML_Char *_Owned _ArrayElem _Nullable
copyString(const XML_Char *_Nonnull s, XML_Parser _Borrow parser) {
  /* First determine how long the string is */
  const size_t charsRequired = _Unsafe(xcslen(s)) + /*null terminator*/ 1;

  /* Detect and prevent integer overflow */
  if (charsRequired > SIZE_MAX / sizeof(XML_Char))
    return nullptr;

  const size_t bytesRequired = charsRequired * sizeof(XML_Char);

  /* Now allocate space for the copy */
  XML_Char *const result = _Unsafe((XML_Char *)(MALLOC(parser, bytesRequired)));

  if (result == nullptr)
    return nullptr;

  /* Copy the original into place */
  _Unsafe memcpy(result, s, bytesRequired);

  return TAKE_ARRAY(XML_Char, result);
}

#if XML_GE == 1

_Safe static float
accountingGetCurrentAmplification(XML_Parser _Borrow rootParser) {
  //                                          1.........1.........12 => 22
  const size_t lenOfShortestInclude = sizeof("<!ENTITY a SYSTEM 'b'>") - 1;
  const XmlBigCount countBytesOutput
      = rootParser->m_accounting.countBytesDirect
        + rootParser->m_accounting.countBytesIndirect;
  const float amplificationFactor
      = rootParser->m_accounting.countBytesDirect
            ? ((float)countBytesOutput
               / (float)(rootParser->m_accounting.countBytesDirect))
            : ((float)(lenOfShortestInclude
                       + rootParser->m_accounting.countBytesIndirect)
               / (float)lenOfShortestInclude);
  _Unsafe assert(! rootParser->m_parentParser);
  return amplificationFactor;
}

_Safe static void
accountingReportStats(XML_Parser _Borrow originParser, const char *epilog) {
  XML_Parser _Borrow rootParser = getRootParserOf(originParser, nullptr);
  _Unsafe assert(! rootParser->m_parentParser);

  if (rootParser->m_accounting.debugLevel == 0u) {
    return;
  }

  const float amplificationFactor
      = accountingGetCurrentAmplification(rootParser);
  _Unsafe fprintf(stderr,
          "expat: Accounting(%p): Direct " EXPAT_FMT_ULL(
              "10") ", indirect " EXPAT_FMT_ULL("10") ", amplification %8.2f%s",
          (void *)rootParser, rootParser->m_accounting.countBytesDirect,
          rootParser->m_accounting.countBytesIndirect,
          (double)amplificationFactor, epilog);
}

_Safe static void
accountingOnAbort(XML_Parser _Borrow originParser) {
  accountingReportStats(originParser, " ABORTING\n");
}

_Safe static void
accountingReportDiff(XML_Parser _Borrow rootParser,
                     unsigned int levelsAwayFromRootParser, const char *before,
                     const char *after, ptrdiff_t bytesMore, int source_line,
                     enum XML_Account account) {
  _Unsafe assert(! rootParser->m_parentParser);

  _Unsafe fprintf(stderr,
          " (+" EXPAT_FMT_PTRDIFF_T("6") " bytes %s|%u, xmlparse.c:%d) %*s\"",
          bytesMore, (account == XML_ACCOUNT_DIRECT) ? "DIR" : "EXP",
          levelsAwayFromRootParser, source_line, 10, "");

  const char ellipis[] = "[..]";
  const size_t ellipsisLength = sizeof(ellipis) /* because compile-time */ - 1;
  const unsigned int contextLength = 10;

  /* Note: Performance is of no concern here */
  const char *walker = before;
  if ((rootParser->m_accounting.debugLevel >= 3u)
      || (after - before)
             <= (ptrdiff_t)(contextLength + ellipsisLength + contextLength)) {
    _Unsafe for (; walker < after; walker++) {
      _Unsafe fprintf(stderr, "%s", unsignedCharToPrintable(walker[0]));
    }
  } else {
    _Unsafe for (; walker < before + contextLength; walker++) {
      _Unsafe fprintf(stderr, "%s", unsignedCharToPrintable(walker[0]));
    }
    _Unsafe fprintf(stderr, ellipis);
    walker = after - contextLength;
    _Unsafe for (; walker < after; walker++) {
      _Unsafe fprintf(stderr, "%s", unsignedCharToPrintable(walker[0]));
    }
  }
  _Unsafe fprintf(stderr, "\"\n");
}

_Safe static XML_Bool
accountingDiffTolerated(XML_Parser _Borrow originParser, int tok, const char *before,
                        const char *after, int source_line,
                        enum XML_Account account) {
  /* Note: We need to check the token type *first* to be sure that
   *       we can even access variable <after>, safely.
   *       E.g. for XML_TOK_NONE <after> may hold an invalid pointer. */
  switch (tok) {
  case XML_TOK_INVALID:
  case XML_TOK_PARTIAL:
  case XML_TOK_PARTIAL_CHAR:
  case XML_TOK_NONE:
    return XML_TRUE;
  }

  if (account == XML_ACCOUNT_NONE)
    return XML_TRUE; /* because these bytes have been accounted for, already */

  unsigned int levelsAwayFromRootParser = 0;
  (void)getRootParserOf(originParser, &_Mut levelsAwayFromRootParser);
  const int originIsRoot = (originParser->m_parentParser == nullptr);
  XML_Parser _Borrow rootParser = getRootParserOf(originParser, nullptr);
  _Unsafe assert(! rootParser->m_parentParser);

  const int isDirect = (account == XML_ACCOUNT_DIRECT) && originIsRoot;
  const ptrdiff_t bytesMore = after - before;

  XmlBigCount *const additionTarget
      = isDirect ? _Unsafe(&rootParser->m_accounting.countBytesDirect)
                 : _Unsafe(&rootParser->m_accounting.countBytesIndirect);

  /* Detect and avoid integer overflow */
  if (_Unsafe((*additionTarget > (XmlBigCount)(-1) - (XmlBigCount)bytesMore)))
    return XML_FALSE;
  _Unsafe *additionTarget += bytesMore;

  const XmlBigCount countBytesOutput
      = rootParser->m_accounting.countBytesDirect
        + rootParser->m_accounting.countBytesIndirect;
  const float amplificationFactor
      = accountingGetCurrentAmplification(rootParser);
  const XML_Bool tolerated
      = (countBytesOutput < rootParser->m_accounting.activationThresholdBytes)
        || (amplificationFactor
            <= rootParser->m_accounting.maximumAmplificationFactor);

  if (rootParser->m_accounting.debugLevel >= 2u) {
    accountingReportStats(rootParser, "");
    accountingReportDiff(rootParser, levelsAwayFromRootParser, before, after,
                         bytesMore, source_line, account);
  }

  return tolerated;
}

_Safe unsigned long long
testingAccountingGetCountBytesDirect(XML_Parser _Borrow _Nullable parser) {
  if (! parser)
    return 0;
  return parser->m_accounting.countBytesDirect;
}

_Safe unsigned long long
testingAccountingGetCountBytesIndirect(XML_Parser _Borrow _Nullable parser) {
  if (! parser)
    return 0;
  return parser->m_accounting.countBytesIndirect;
}

_Safe static void
entityTrackingReportStats(XML_Parser _Borrow rootParser, ENTITY *_Nonnull entity,
                          const char *action, int sourceLine) {
  _Unsafe assert(! rootParser->m_parentParser);
  if (rootParser->m_entity_stats.debugLevel == 0u)
    return;

#  if defined(XML_UNICODE)
  const char *const entityName = "[..]";
#  else
  const char *const entityName = _Unsafe((entity->name));
#  endif

  const bool limitingWanted = rootParser->m_entity_stats.debugLevel < 2;
  const int maxLimitedDepth = 10; // somewhat arbitrary
  const int candidateIndentDepth
      = (int)rootParser->m_entity_stats.currentDepth - 1;
  const bool limitingNeeded
      = limitingWanted && (candidateIndentDepth > maxLimitedDepth);
  const char *const ellipisOrEmpty = limitingNeeded ? " [..] " : "";
  const int indentDepth
      = limitingNeeded ? (maxLimitedDepth - /* make space for ellipis */ 2)
                       : candidateIndentDepth;

  _Unsafe(fprintf(
      stderr,
      "expat: Entities(%p): Count %9u, depth %2u/%2u %*s%s%s%s; %s length %d (xmlparse.c:%d)\n",
      (void *)rootParser, rootParser->m_entity_stats.countEverOpened,
      rootParser->m_entity_stats.currentDepth,
      rootParser->m_entity_stats.maximumDepthSeen, indentDepth * 2, "",
      ellipisOrEmpty, entity->is_param ? "%" : "&", entityName, action,
      entity->textLen, sourceLine));
}

_Safe static void
entityTrackingOnOpen(XML_Parser _Borrow originParser, ENTITY *_Nonnull entity, int sourceLine) {
  XML_Parser _Borrow rootParser = getRootParserOf(originParser, nullptr);
  _Unsafe assert(! rootParser->m_parentParser);

  rootParser->m_entity_stats.countEverOpened++;
  rootParser->m_entity_stats.currentDepth++;
  if (rootParser->m_entity_stats.currentDepth
      > rootParser->m_entity_stats.maximumDepthSeen) {
    rootParser->m_entity_stats.maximumDepthSeen++;
  }

  entityTrackingReportStats(rootParser, entity, "OPEN ", sourceLine);
}

_Safe static void
entityTrackingOnClose(XML_Parser _Borrow originParser, ENTITY *_Nonnull entity, int sourceLine) {
  XML_Parser _Borrow rootParser = getRootParserOf(originParser, nullptr);
  _Unsafe assert(! rootParser->m_parentParser);

  entityTrackingReportStats(rootParser, entity, "CLOSE", sourceLine);
  rootParser->m_entity_stats.currentDepth--;
}

#endif /* XML_GE == 1 */

_Safe static XML_Parser _Borrow
getRootParserOf(XML_Parser _Borrow parser,
                unsigned int *_Borrow _Nullable outLevelDiff) {
  XML_Parser rootParser = parser->m_parentParser;
  unsigned int stepsTakenUpwards = 0;
  if (rootParser == nullptr) {
    if (outLevelDiff != nullptr)
      *outLevelDiff = 0;
    return parser;
  }
  stepsTakenUpwards = 1;
  while (_Unsafe(rootParser->m_parentParser) != nullptr) {
    _Unsafe rootParser = rootParser->m_parentParser;
    stepsTakenUpwards++;
  }
  if (outLevelDiff != nullptr)
    *outLevelDiff = stepsTakenUpwards;
  return _Unsafe(&_Mut *rootParser);
}

#if XML_GE == 1

_Safe const char *
unsignedCharToPrintable(unsigned char c) {
  switch (c) {
  case 0:
    return "\\0";
  case 1:
    return "\\x1";
  case 2:
    return "\\x2";
  case 3:
    return "\\x3";
  case 4:
    return "\\x4";
  case 5:
    return "\\x5";
  case 6:
    return "\\x6";
  case 7:
    return "\\x7";
  case 8:
    return "\\x8";
  case 9:
    return "\\t";
  case 10:
    return "\\n";
  case 11:
    return "\\xB";
  case 12:
    return "\\xC";
  case 13:
    return "\\r";
  case 14:
    return "\\xE";
  case 15:
    return "\\xF";
  case 16:
    return "\\x10";
  case 17:
    return "\\x11";
  case 18:
    return "\\x12";
  case 19:
    return "\\x13";
  case 20:
    return "\\x14";
  case 21:
    return "\\x15";
  case 22:
    return "\\x16";
  case 23:
    return "\\x17";
  case 24:
    return "\\x18";
  case 25:
    return "\\x19";
  case 26:
    return "\\x1A";
  case 27:
    return "\\x1B";
  case 28:
    return "\\x1C";
  case 29:
    return "\\x1D";
  case 30:
    return "\\x1E";
  case 31:
    return "\\x1F";
  case 32:
    return " ";
  case 33:
    return "!";
  case 34:
    return "\\\"";
  case 35:
    return "#";
  case 36:
    return "$";
  case 37:
    return "%";
  case 38:
    return "&";
  case 39:
    return "'";
  case 40:
    return "(";
  case 41:
    return ")";
  case 42:
    return "*";
  case 43:
    return "+";
  case 44:
    return ",";
  case 45:
    return "-";
  case 46:
    return ".";
  case 47:
    return "/";
  case 48:
    return "0";
  case 49:
    return "1";
  case 50:
    return "2";
  case 51:
    return "3";
  case 52:
    return "4";
  case 53:
    return "5";
  case 54:
    return "6";
  case 55:
    return "7";
  case 56:
    return "8";
  case 57:
    return "9";
  case 58:
    return ":";
  case 59:
    return ";";
  case 60:
    return "<";
  case 61:
    return "=";
  case 62:
    return ">";
  case 63:
    return "?";
  case 64:
    return "@";
  case 65:
    return "A";
  case 66:
    return "B";
  case 67:
    return "C";
  case 68:
    return "D";
  case 69:
    return "E";
  case 70:
    return "F";
  case 71:
    return "G";
  case 72:
    return "H";
  case 73:
    return "I";
  case 74:
    return "J";
  case 75:
    return "K";
  case 76:
    return "L";
  case 77:
    return "M";
  case 78:
    return "N";
  case 79:
    return "O";
  case 80:
    return "P";
  case 81:
    return "Q";
  case 82:
    return "R";
  case 83:
    return "S";
  case 84:
    return "T";
  case 85:
    return "U";
  case 86:
    return "V";
  case 87:
    return "W";
  case 88:
    return "X";
  case 89:
    return "Y";
  case 90:
    return "Z";
  case 91:
    return "[";
  case 92:
    return "\\\\";
  case 93:
    return "]";
  case 94:
    return "^";
  case 95:
    return "_";
  case 96:
    return "`";
  case 97:
    return "a";
  case 98:
    return "b";
  case 99:
    return "c";
  case 100:
    return "d";
  case 101:
    return "e";
  case 102:
    return "f";
  case 103:
    return "g";
  case 104:
    return "h";
  case 105:
    return "i";
  case 106:
    return "j";
  case 107:
    return "k";
  case 108:
    return "l";
  case 109:
    return "m";
  case 110:
    return "n";
  case 111:
    return "o";
  case 112:
    return "p";
  case 113:
    return "q";
  case 114:
    return "r";
  case 115:
    return "s";
  case 116:
    return "t";
  case 117:
    return "u";
  case 118:
    return "v";
  case 119:
    return "w";
  case 120:
    return "x";
  case 121:
    return "y";
  case 122:
    return "z";
  case 123:
    return "{";
  case 124:
    return "|";
  case 125:
    return "}";
  case 126:
    return "~";
  case 127:
    return "\\x7F";
  case 128:
    return "\\x80";
  case 129:
    return "\\x81";
  case 130:
    return "\\x82";
  case 131:
    return "\\x83";
  case 132:
    return "\\x84";
  case 133:
    return "\\x85";
  case 134:
    return "\\x86";
  case 135:
    return "\\x87";
  case 136:
    return "\\x88";
  case 137:
    return "\\x89";
  case 138:
    return "\\x8A";
  case 139:
    return "\\x8B";
  case 140:
    return "\\x8C";
  case 141:
    return "\\x8D";
  case 142:
    return "\\x8E";
  case 143:
    return "\\x8F";
  case 144:
    return "\\x90";
  case 145:
    return "\\x91";
  case 146:
    return "\\x92";
  case 147:
    return "\\x93";
  case 148:
    return "\\x94";
  case 149:
    return "\\x95";
  case 150:
    return "\\x96";
  case 151:
    return "\\x97";
  case 152:
    return "\\x98";
  case 153:
    return "\\x99";
  case 154:
    return "\\x9A";
  case 155:
    return "\\x9B";
  case 156:
    return "\\x9C";
  case 157:
    return "\\x9D";
  case 158:
    return "\\x9E";
  case 159:
    return "\\x9F";
  case 160:
    return "\\xA0";
  case 161:
    return "\\xA1";
  case 162:
    return "\\xA2";
  case 163:
    return "\\xA3";
  case 164:
    return "\\xA4";
  case 165:
    return "\\xA5";
  case 166:
    return "\\xA6";
  case 167:
    return "\\xA7";
  case 168:
    return "\\xA8";
  case 169:
    return "\\xA9";
  case 170:
    return "\\xAA";
  case 171:
    return "\\xAB";
  case 172:
    return "\\xAC";
  case 173:
    return "\\xAD";
  case 174:
    return "\\xAE";
  case 175:
    return "\\xAF";
  case 176:
    return "\\xB0";
  case 177:
    return "\\xB1";
  case 178:
    return "\\xB2";
  case 179:
    return "\\xB3";
  case 180:
    return "\\xB4";
  case 181:
    return "\\xB5";
  case 182:
    return "\\xB6";
  case 183:
    return "\\xB7";
  case 184:
    return "\\xB8";
  case 185:
    return "\\xB9";
  case 186:
    return "\\xBA";
  case 187:
    return "\\xBB";
  case 188:
    return "\\xBC";
  case 189:
    return "\\xBD";
  case 190:
    return "\\xBE";
  case 191:
    return "\\xBF";
  case 192:
    return "\\xC0";
  case 193:
    return "\\xC1";
  case 194:
    return "\\xC2";
  case 195:
    return "\\xC3";
  case 196:
    return "\\xC4";
  case 197:
    return "\\xC5";
  case 198:
    return "\\xC6";
  case 199:
    return "\\xC7";
  case 200:
    return "\\xC8";
  case 201:
    return "\\xC9";
  case 202:
    return "\\xCA";
  case 203:
    return "\\xCB";
  case 204:
    return "\\xCC";
  case 205:
    return "\\xCD";
  case 206:
    return "\\xCE";
  case 207:
    return "\\xCF";
  case 208:
    return "\\xD0";
  case 209:
    return "\\xD1";
  case 210:
    return "\\xD2";
  case 211:
    return "\\xD3";
  case 212:
    return "\\xD4";
  case 213:
    return "\\xD5";
  case 214:
    return "\\xD6";
  case 215:
    return "\\xD7";
  case 216:
    return "\\xD8";
  case 217:
    return "\\xD9";
  case 218:
    return "\\xDA";
  case 219:
    return "\\xDB";
  case 220:
    return "\\xDC";
  case 221:
    return "\\xDD";
  case 222:
    return "\\xDE";
  case 223:
    return "\\xDF";
  case 224:
    return "\\xE0";
  case 225:
    return "\\xE1";
  case 226:
    return "\\xE2";
  case 227:
    return "\\xE3";
  case 228:
    return "\\xE4";
  case 229:
    return "\\xE5";
  case 230:
    return "\\xE6";
  case 231:
    return "\\xE7";
  case 232:
    return "\\xE8";
  case 233:
    return "\\xE9";
  case 234:
    return "\\xEA";
  case 235:
    return "\\xEB";
  case 236:
    return "\\xEC";
  case 237:
    return "\\xED";
  case 238:
    return "\\xEE";
  case 239:
    return "\\xEF";
  case 240:
    return "\\xF0";
  case 241:
    return "\\xF1";
  case 242:
    return "\\xF2";
  case 243:
    return "\\xF3";
  case 244:
    return "\\xF4";
  case 245:
    return "\\xF5";
  case 246:
    return "\\xF6";
  case 247:
    return "\\xF7";
  case 248:
    return "\\xF8";
  case 249:
    return "\\xF9";
  case 250:
    return "\\xFA";
  case 251:
    return "\\xFB";
  case 252:
    return "\\xFC";
  case 253:
    return "\\xFD";
  case 254:
    return "\\xFE";
  case 255:
    return "\\xFF";
  // LCOV_EXCL_START
  default:
    _Unsafe assert(0); /* never gets here */
    return "dead code";
  }
  _Unsafe assert(0); /* never gets here */
  // LCOV_EXCL_STOP
}

#endif /* XML_GE == 1 */

_Safe static unsigned long
getDebugLevel(const char *variableName, unsigned long defaultDebugLevel) {
  const char *const valueOrNull = _Unsafe(getenv(variableName));
  if (valueOrNull == nullptr) {
    return defaultDebugLevel;
  }
  const char *const value = valueOrNull;

  _Unsafe errno = 0;
  char *afterValue = nullptr;
  unsigned long debugLevel = _Unsafe(strtoul(value, _Unsafe(&afterValue), 10));
  if (_Unsafe(((errno != 0) || (afterValue == value) || (afterValue[0] != '\0')))) {
    _Unsafe errno = 0;
    return defaultDebugLevel;
  }

  return debugLevel;
}
