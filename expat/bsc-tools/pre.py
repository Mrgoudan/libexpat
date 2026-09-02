"""Hand-written rewrites applied to the ORIGINAL xmlparse.c before the generic pass."""
import re
p = '/home/ziruichen/bsd/libexpat/expat/lib/xmlparse.c'
s = open(p).read()

def rep(old, new, count=1):
    global s
    assert old in s, old[:70]
    s = s.replace(old, new, count)

def scoped(begin_anchor, end_anchor, fn):
    """Apply fn to the text between the two anchors (end exclusive of end_anchor)."""
    global s
    a = s.index(begin_anchor)
    b = s.index(end_anchor, a)
    s = s[:a] + fn(s[a:b]) + s[b:]

# _Safe declarations of public functions that are called before their definition
rep('''#define EXPAND_SPARE 24
''', '''#define EXPAND_SPARE 24

_Safe XML_Parser _Owned _Nullable XMLCALL XML_ParserCreate_MM(
    const XML_Char *encodingName,
    const XML_Memory_Handling_Suite *_Borrow _Nullable memsuite,
    const XML_Char *nameSep);
_Safe void XMLCALL XML_ParserFree(XML_Parser _Owned _Nullable parser);
_Safe void *_Nullable XMLCALL XML_GetBuffer(XML_Parser _Borrow _Nullable parser,
                                            int len);
_Safe enum XML_Status XMLCALL XML_ParseBuffer(XML_Parser _Borrow _Nullable parser,
                                              int len, int isFinal);
''')

# raw back-reference helpers: pools and tables keep a raw parser pointer
rep('''#  define FREE(parser, p) (parser->m_mem.free_fcn((p)))
#endif''', '''#  define FREE(parser, p) (parser->m_mem.free_fcn((p)))
#endif
// Borrow the parser behind a raw back-reference for one allocator call.
#define PARSER_OF(x) (_Unsafe(&_Mut *(x)->parser))
// Raw alias of a borrowed parser, for storage in raw back-reference fields.
#define RAW_PARSER(p) (_Unsafe((XML_Parser)(void *)(p)))''')
s = re.sub(r'\b(MALLOC|REALLOC|FREE|FREE_ARRAY|REALLOC_ARRAY)\((table|pool)->parser,', r'\1(PARSER_OF(\2),', s)

# ++/-- value uses are void in _Safe
s = re.sub(r'appAtts\[attIndex\+\+\] = ([^;]+);', r'appAtts[attIndex] = \1;\n      attIndex++;', s)
rep("""      while (nPrefixes >> parser->m_nsAttsPower++)
        ;""", """      for (;;) {
        const unsigned char oldPower = parser->m_nsAttsPower;
        parser->m_nsAttsPower++;
        if (! (nPrefixes >> oldPower))
          break;
      }""")
rep("""      for (j = nsAttsSize; j != 0;)
        parser->m_nsAtts[--j].version = version;""", """      for (j = nsAttsSize; j != 0;) {
        j--;
        parser->m_nsAtts[j].version = version;
      }""")
rep("""        if (! --nPrefixes) {""", """        nPrefixes--;
        if (! nPrefixes) {""")
rep("""  next = (int)dtd->scaffCount++;""", """  next = (int)dtd->scaffCount;
  dtd->scaffCount++;""")
rep("""  return tv.tv_usec;""", """  return (unsigned long)tv.tv_usec;""")

# callback body keeps a raw parser; borrow it for the _Safe helpers
rep("""  XML_Parser parser = data;
  beforeHandler(parser);
  const int result
      = parser->m_unknownEncodingConvert(parser->m_unknownEncodingData, p);
  afterHandler(parser);""", """  XML_Parser parser = data;
  beforeHandler(&_Mut *parser);
  const int result
      = parser->m_unknownEncodingConvert(parser->m_unknownEncodingData, p);
  afterHandler(&_Mut *parser);""")

# raw self pointers handed to callbacks and back-references
s = s.replace('parser->m_externalEntityRefHandlerArg = parser;', 'parser->m_externalEntityRefHandlerArg = RAW_PARSER(&_Mut *parser);')
s = s.replace('parser->m_handlerArg = parser;', 'parser->m_handlerArg = _Unsafe((void *)&_Mut *parser);')
rep('          info.convert ? callUnknownEncodingConvert : NULL, parser)', '          info.convert ? callUnknownEncodingConvert : NULL,\n          _Unsafe((void *)parser))')

# poolInit / hashTableInit store a raw back-reference: take the parser raw
rep('static void FASTCALL hashTableInit(HASH_TABLE *table, XML_Parser parser);', 'static void FASTCALL hashTableInit(HASH_TABLE *table, XML_Parser rawParser);')
rep('static void FASTCALL poolInit(STRING_POOL *pool, XML_Parser parser);', 'static void FASTCALL poolInit(STRING_POOL *pool, XML_Parser rawParser);')
rep("""hashTableInit(HASH_TABLE *p, XML_Parser parser) {
  p->power = 0;
  p->size = 0;
  p->used = 0;
  p->v = NULL;
  p->parser = parser;""", """hashTableInit(HASH_TABLE *p, XML_Parser rawParser) {
  p->power = 0;
  p->size = 0;
  p->used = 0;
  p->v = NULL;
  p->parser = rawParser;""")
rep("""poolInit(STRING_POOL *pool, XML_Parser parser) {""", """poolInit(STRING_POOL *pool, XML_Parser rawParser) {""")
rep("""  pool->parser = parser;""", """  pool->parser = rawParser;""")
s = s.replace('hashTableInit(&(elementType->defaultAttForName), parser);', 'hashTableInit(&(elementType->defaultAttForName), RAW_PARSER(parser));')
s = s.replace('hashTableInit(&(newE->defaultAttForName), parser);', 'hashTableInit(&(newE->defaultAttForName), RAW_PARSER(parser));')
s = s.replace('hashTableInit(&(ret->defaultAttForName), getRootParserOf(parser, NULL));', 'hashTableInit(&(ret->defaultAttForName), RAW_PARSER(getRootParserOf(parser, NULL)));')
scoped('dtdCreate(XML_Parser parser) {', '\n}\n', lambda t: t.replace(', parser);', ', RAW_PARSER(parser));'))

# getRootParserOf: walk the raw parent chain, hand back a borrow
rep("""static XML_Parser
getRootParserOf(XML_Parser parser, unsigned int *outLevelDiff) {
  XML_Parser rootParser = parser;
  unsigned int stepsTakenUpwards = 0;
  while (rootParser->m_parentParser) {
    rootParser = rootParser->m_parentParser;
    stepsTakenUpwards++;
  }
  assert(! rootParser->m_parentParser);
  if (outLevelDiff != NULL) {
    *outLevelDiff = stepsTakenUpwards;
  }
  return rootParser;
}""", """_Safe static XML_Parser _Borrow
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
}""")

# allocator: copy the suite before deriving the root borrow; pass tracker values by copy
rep("""  const XML_Parser rootParser = getRootParserOf(parser, NULL);
  assert(rootParser->m_parentParser == NULL);

  const size_t bytesToAllocate""", """  const XML_Memory_Handling_Suite mem = parser->m_mem;
  const XML_Parser rootParser = getRootParserOf(parser, NULL);
  assert(rootParser->m_parentParser == NULL);

  const size_t bytesToAllocate""")
rep("""  void *const mallocedPtr = parser->m_mem.malloc_fcn(bytesToAllocate);""", """  void *const mallocedPtr = mem.malloc_fcn(bytesToAllocate);""")
rep("""  const XML_Parser rootParser = getRootParserOf(parser, NULL);
  assert(rootParser->m_parentParser == NULL);

  // Extract original size""", """  const XML_Memory_Handling_Suite mem = parser->m_mem;
  const XML_Parser rootParser = getRootParserOf(parser, NULL);
  assert(rootParser->m_parentParser == NULL);

  // Extract original size""")
rep("""  mallocedPtr = parser->m_mem.realloc_fcn(
      mallocedPtr, sizeof(size_t) + EXPAT_MALLOC_PADDING + size);""", """  mallocedPtr = mem.realloc_fcn(mallocedPtr,
                                sizeof(size_t) + EXPAT_MALLOC_PADDING + size);""")
s = re.sub(r"(\s*)expat_heap_stat\((\w+), ([^,]+), ([^,]+),\s*\2->m_alloc_tracker\.bytesAllocated,\s*\2->m_alloc_tracker\.peakBytesAllocated, (\w+)\);",
           lambda m: "%s{%s  const XmlBigCount curBytes = %s->m_alloc_tracker.bytesAllocated;%s  const XmlBigCount peakBytes = %s->m_alloc_tracker.peakBytesAllocated;%s  expat_heap_stat(%s, %s, %s, curBytes, peakBytes, %s);%s}" % (
               m.group(1), m.group(1), m.group(2), m.group(1), m.group(2), m.group(1), m.group(2), m.group(3), m.group(4), m.group(5), m.group(1)), s)

# init analysis: locals filled through out-pointers in unsafe calls
rep("""  struct timeval tv;""", """  struct timeval tv = {0, 0};""")
rep("""  struct sipkey entropy;""", """  struct sipkey entropy = {{0, 0}};""")

# narrowing conversions made explicit
rep("""  const size_t have_now = EXPAT_SAFE_PTR_DIFF(end, start);""", """  const size_t have_now = (size_t)EXPAT_SAFE_PTR_DIFF(end, start);""")
rep("""    size_t available_buffer
        = EXPAT_SAFE_PTR_DIFF(parser->m_bufferPtr, parser->m_buffer);""", """    size_t available_buffer
        = (size_t)EXPAT_SAFE_PTR_DIFF(parser->m_bufferPtr, parser->m_buffer);""")
rep("""    const size_t available = pool->end - pool->ptr;""", """    const size_t available = (size_t)(pool->end - pool->ptr);""")
rep("""          j = uriHash & mask; /* index into hash table */""", """          j = (unsigned int)(uriHash & mask); /* index into hash table */""")
rep("""      if (! poolAppendChars(pool, buf, n))
        return XML_ERROR_NO_MEMORY;""", """      if (! poolAppendChars(pool, buf, (size_t)n))
        return XML_ERROR_NO_MEMORY;""")
rep("""      if (! poolAppendChars(pool, buf, n)) {""", """      if (! poolAppendChars(pool, buf, (size_t)n)) {""")
rep("""  if (n > 0 && ! poolAppendChars(pool, s, n))""", """  if (n > 0 && ! poolAppendChars(pool, s, (size_t)n))""")
rep("""          if (! poolAppendChars(&dtd->pool, name, i))""", """          if (! poolAppendChars(&dtd->pool, name, (size_t)i))""")
rep("""      unsigned char newPower = table->power + 1;""", """      unsigned char newPower = (unsigned char)(table->power + 1);""")
rep("""        const bool prefixNameUsed = prefix && prefix->name == prefixName;""", """        const bool prefixNameUsed = (bool)(prefix && prefix->name == prefixName);""")

# borrows handed to _Safe helpers
rep("""  dtdReset(parser->m_dtd, parser);""", """  {
    DTD *dtdRaw = parser->m_dtd;
    if (dtdRaw != NULL) {
      DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);
      dtdReset(dtd, parser);
    }
  }""")
rep("""  STRING_POOL *pool = &(dtd->entityValuePool);""", """  STRING_POOL *_Borrow pool = &dtd->entityValuePool;""")
s = s.replace("copy_salt_to_sipkey(parser, &key);", "copy_salt_to_sipkey(parser, &_Mut key);")
s = s.replace("copy_salt_to_sipkey(parser, &sip_key);", "copy_salt_to_sipkey(parser, &_Mut sip_key);")
rep("""  unsigned int levelsAwayFromRootParser;
  const XML_Parser rootParser
      = getRootParserOf(originParser, &levelsAwayFromRootParser);
  assert(! rootParser->m_parentParser);

  const int isDirect
      = (account == XML_ACCOUNT_DIRECT) && (originParser == rootParser);""", """  unsigned int levelsAwayFromRootParser = 0;
  (void)getRootParserOf(originParser, &_Mut levelsAwayFromRootParser);
  const int originIsRoot = (originParser->m_parentParser == NULL);
  const XML_Parser rootParser = getRootParserOf(originParser, NULL);
  assert(! rootParser->m_parentParser);

  const int isDirect = (account == XML_ACCOUNT_DIRECT) && originIsRoot;""")

# parserCreate: raw during construction, borrowed for initialisation, owned at the end
s = s.replace("""                               const XML_Char *nameSep, DTD *dtd,
                               XML_Parser parentParser);""", """                               const XML_Char *nameSep, DTD *dtd,
                               XML_Parser _Borrow _Nullable parentParser);""")
s = s.replace("""             DTD *dtd, XML_Parser parentParser) {""", """             DTD *dtd, XML_Parser _Borrow _Nullable parentParser) {""")
def fix_parser_create(t):
    head, tail = t.split("""  if (! parser)
    return parser;
""", 1)
    tail = tail.replace('parser->', 'p->')
    tail = tail.replace('FREE(parser, parser);', 'FREE(p, rawParser);')
    tail = tail.replace('FREE(parser, ', 'FREE(p, ')
    tail = tail.replace('FREE_ARRAY(parser, ', 'FREE_ARRAY(p, ')
    tail = tail.replace('MALLOC(parser, ', 'MALLOC(p, ')
    tail = tail.replace('dtdCreate(parser)', 'dtdCreate(p)')
    tail = tail.replace('getRootParserOf(parser, NULL)', 'getRootParserOf(p, NULL)')
    tail = tail.replace('p->m_parentParser = parentParser;', 'p->m_parentParser = RAW_PARSER(parentParser);')
    tail = tail.replace('poolInit(&p->m_tempPool, parser);', 'poolInit(&_Mut p->m_tempPool, rawParser);')
    tail = tail.replace('poolInit(&p->m_temp2Pool, parser);', 'poolInit(&_Mut p->m_temp2Pool, rawParser);')
    tail = tail.replace('parserInit(parser, encodingName);', 'parserInit(p, encodingName);')
    tail = tail.replace('    XML_ParserFree(parser);\n    return NULL;', '    _Unsafe XML_ParserFree(__take_from_raw(rawParser));\n    return NULL;')
    tail = tail.replace('\n  return parser;\n}', '\n  return _Unsafe(__take_from_raw(rawParser));\n}')
    assert 'parser;' not in tail.replace('rawParser;', '').replace('parentParser', ''), tail
    return head + """  if (! parser)
    return NULL;
  XML_Parser rawParser = parser;
  XML_Parser _Borrow p = _Unsafe(&_Mut *parser);
""" + tail
scoped('static XML_Parser\nparserCreate(', '\n\nstatic void\nparserInit(', fix_parser_create)

# XML_ExternalEntityParserCreate: stash from the borrowed old parser, own the new one
rep("""  XML_Parser parser = oldParser;
  DTD *newDtd = NULL;""", """  DTD *newDtd = NULL;""")
scoped('  /* Stash the original parser contents on the stack */', '  if (parser->m_ns) {\n    XML_Char tmp[2] = {parser->m_namespaceSeparator, 0};',
       lambda t: t.replace('= parser->m_', '= oldParser->m_'))
rep("""  if (parser->m_ns) {
    XML_Char tmp[2] = {parser->m_namespaceSeparator, 0};
    parser = parserCreate(encodingName, &parser->m_mem, tmp, newDtd, oldParser);
  } else {
    parser
        = parserCreate(encodingName, &parser->m_mem, NULL, newDtd, oldParser);
  }

  if (! parser)
    return NULL;
""", """  const XML_Memory_Handling_Suite oldMem = oldParser->m_mem;
  XML_Parser _Owned _Nullable parser = NULL;
  if (oldParser->m_ns) {
    XML_Char tmp[2] = {oldParser->m_namespaceSeparator, 0};
    parser = parserCreate(encodingName, &_Const oldMem, tmp, newDtd, oldParser);
  } else {
    parser = parserCreate(encodingName, &_Const oldMem, NULL, newDtd, oldParser);
  }

  if (parser == NULL)
    return parser;
""")
rep("""  if (oldExternalEntityRefHandlerArg != oldParser)""", """  if (_Unsafe(oldExternalEntityRefHandlerArg != (XML_Parser)(void *)oldParser))""")
rep("""  parser->m_parentParser = oldParser;
#ifdef XML_DTD
  parser->m_paramEntityParsing = oldParamEntityParsing;""", """  parser->m_parentParser = RAW_PARSER(oldParser);
#ifdef XML_DTD
  parser->m_paramEntityParsing = oldParamEntityParsing;""")
rep("""    if (! dtdCopy(oldParser, parser->m_dtd, oldDtd, parser)
        || ! setContext(parser, context)) {
      XML_ParserFree(parser);
      return NULL;
    }""", """    DTD *newDtdRaw = parser->m_dtd;
    const DTD *oldDtdRaw = oldParser->m_dtd;
    if (newDtdRaw == NULL || oldDtdRaw == NULL) {
      XML_ParserFree(parser);
      return NULL;
    }
    DTD *_Borrow newDtdB = _Unsafe(&_Mut *newDtdRaw);
    const DTD *_Borrow oldDtdB = _Unsafe(&_Const *oldDtdRaw);
    if (! dtdCopy(oldParser, newDtdB, oldDtdB, &_Mut *parser)
        || ! setContext(&_Mut *parser, context)) {
      XML_ParserFree(parser);
      return NULL;
    }""")

# XML_ParserFree owns the parser: reborrow it for helpers, free the struct through a raw alias last
def fix_parser_free(t):
    t = t.replace("""  if ((parser == NULL) || isCalledFromInsideHandler(parser))
    return;""", """  if (parser == NULL)
    return;
  if (isCalledFromInsideHandler(&_Mut *parser)) {
    // Refusing to free from inside a handler keeps the parser alive on purpose.
    _Unsafe { (void)__move_to_raw(parser); }
    return;
  }""")
    t = t.replace('destroyBindings(p->bindings, parser);', 'destroyBindings(p->bindings, &_Mut *parser);')
    t = t.replace('destroyBindings(parser->m_freeBindingList, parser);', 'destroyBindings(parser->m_freeBindingList, &_Mut *parser);')
    t = t.replace('destroyBindings(parser->m_inheritedBindings, parser);', 'destroyBindings(parser->m_inheritedBindings, &_Mut *parser);')
    t = t.replace('dtdDestroy(parser->m_dtd, (XML_Bool)! parser->m_parentParser, parser);', 'dtdDestroy(parser->m_dtd, (XML_Bool)! parser->m_parentParser, &_Mut *parser);')
    t = t.replace("""  if (parser->m_unknownEncodingRelease)
    callUnknownEncodingRelease(parser);
  FREE(parser, parser);""", """  if (parser->m_unknownEncodingRelease)
    callUnknownEncodingRelease(&_Mut *parser);
  _Unsafe {
    XML_Parser rawParser = __move_to_raw(parser);
    XML_Parser rawParserAlias = rawParser;
    FREE(&_Mut *rawParser, rawParserAlias);
  }""")
    t = t.replace('FREE(parser, ', 'FREE(&_Mut *parser, ')
    t = t.replace('FREE_ARRAY(parser, ', 'FREE_ARRAY(&_Mut *parser, ')
    return t
scoped('XML_ParserFree(XML_Parser parser) {', '\n}\n', fix_parser_free)

# pool accessors as _Safe inline functions (macro arguments were borrowed twice)
rep("""#define poolStart(pool) ((pool)->start)
#define poolLength(pool) ((pool)->ptr - (pool)->start)
#define poolChop(pool) ((void)--(pool->ptr))
#define poolLastChar(pool) (((pool)->ptr)[-1])
#define poolDiscard(pool) ((pool)->ptr = (pool)->start)
#define poolFinish(pool) ((pool)->start = (pool)->ptr)""", """_Safe static inline XML_Char *_Nullable
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
}""")
# out-parameter contract for the hash salt; locals filled by unsafe siphash calls
rep("static void copy_salt_to_sipkey(XML_Parser parser, struct sipkey *key);", "static void copy_salt_to_sipkey(XML_Parser parser, struct sipkey *_Borrow __attribute__((ensure_init)) key);")
rep("copy_salt_to_sipkey(XML_Parser parser, struct sipkey *key) {", "copy_salt_to_sipkey(XML_Parser parser, struct sipkey *_Borrow __attribute__((ensure_init)) key) {")
rep("""  struct siphash state;
  struct sipkey key;""", """  struct siphash state = {0, 0, 0, 0, {0}, NULL, 0};
  struct sipkey key = {{0, 0}};""")
rep("""        struct siphash sip_state;
        struct sipkey sip_key;""", """        struct siphash sip_state = {0, 0, 0, 0, {0}, NULL, 0};
        struct sipkey sip_key = {{0, 0}};""")
rep("""      TAG_NAME name;""", """      TAG_NAME name = {NULL, NULL, NULL, 0, 0, 0};""")
s = s.replace("""      XML_Char buf[XML_ENCODE_MAX];""", """      XML_Char buf[XML_ENCODE_MAX] = {0};""")
s = s.replace("""        XML_Char buf[XML_ENCODE_MAX];""", """        XML_Char buf[XML_ENCODE_MAX] = {0};""")

# pool borrowed alongside the parser: go through a raw alias / hoist the value first
rep("""      const enum XML_Error result = storeAttributeValue(
          parser, enc, isCdata, parser->m_atts[i].valuePtr,
          parser->m_atts[i].valueEnd, &parser->m_tempPool, account);""", """      STRING_POOL *tempPoolRaw = _Unsafe(&(parser->m_tempPool));
      const enum XML_Error result = storeAttributeValue(
          parser, enc, isCdata, parser->m_atts[i].valuePtr,
          parser->m_atts[i].valueEnd, _Unsafe(&_Mut *tempPoolRaw), account);""")
rep("""      e = (ENTITY *)lookup(parser, &dtd->generalEntities,
                           poolStart(&parser->m_tempPool), 0);""", """      const XML_Char *entityName = poolStart(&parser->m_tempPool);
      e = (ENTITY *)lookup(parser, &dtd->generalEntities, entityName, 0);""")
rep("""      if (addBinding(parser, prefix, NULL, poolStart(&parser->m_tempPool),
                     &parser->m_inheritedBindings)
          != XML_ERROR_NONE)""", """      const XML_Char *bindingUri = poolStart(&parser->m_tempPool);
      if (addBinding(parser, prefix, NULL, bindingUri,
                     &parser->m_inheritedBindings)
          != XML_ERROR_NONE)""")
rep("""  return (strncmp(s1, s2, s1len) == 0 && s2[s1len] == '\\0') ? XML_TRUE
                                                            : XML_FALSE;""", """  return (XML_Bool)((strncmp(s1, s2, s1len) == 0 && s2[s1len] == '\\0')
                        ? XML_TRUE
                        : XML_FALSE);""")

# explicit null contracts on raw parameters and fields (checked at every call site)
s = re.sub(r'\bconst ENCODING \*enc\b', r'const ENCODING *_Nonnull enc', s)
s = s.replace('  const ENCODING *m_encoding;', '  const ENCODING *_Nonnull m_encoding;')
s = s.replace('  const ENCODING *m_internalEncoding;', '  const ENCODING *_Nonnull m_internalEncoding;')
s = re.sub(r'\bconst char \*\*(endPtr|startPtr)\b', r'const char **_Nonnull \1', s)
s = re.sub(r'\bconst char \*\*nextPtr\b', r'const char **_Nonnull nextPtr', s)
# storeEntityValue / appendAttributeValue accept a NULL nextPtr
s = re.sub(r'(storeEntityValue\([^;{]*?)const char \*\*_Nonnull nextPtr', r'\1const char **_Nullable nextPtr', s, flags=re.S)
s = re.sub(r'(appendAttributeValue\([^;{]*?)const char \*\*_Nonnull nextPtr', r'\1const char **_Nullable nextPtr', s, flags=re.S)
s = re.sub(r'\bENTITY \*entity(?=\s*[,)])', r'ENTITY *_Nonnull entity', s)
s = re.sub(r'\bPREFIX \*prefix(?=\s*[,)])', r'PREFIX *_Nonnull prefix', s)
s = re.sub(r'\bconst ATTRIBUTE_ID \*attId(?=\s*[,)])', r'const ATTRIBUTE_ID *_Nullable attId', s)
s = re.sub(r'\bATTRIBUTE_ID \*attId(?=\s*[,)])', r'ATTRIBUTE_ID *_Nonnull attId', s)
s = re.sub(r'\bELEMENT_TYPE \*(type|elementType)(?=\s*[,)])', r'ELEMENT_TYPE *_Nonnull \1', s)
s = re.sub(r'\bBINDING \*\*bindingsPtr\b', r'BINDING **_Nonnull bindingsPtr', s)
s = re.sub(r'\bKEY (name|s|s2)(?=\s*[,)])', r'KEY _Nonnull \1', s)
s = re.sub(r'\bconst XML_Char \*(s|context|encodingName)(?=\s*[,)])', r'const XML_Char *_Nonnull \1', s)
s = re.sub(r'\bconst XML_Char \*value(?=\s*[,)])', r'const XML_Char *_Nullable value', s)
s = s.replace('handleUnknownEncoding(XML_Parser parser, const XML_Char *_Nonnull encodingName)', 'handleUnknownEncoding(XML_Parser parser, const XML_Char *_Nullable encodingName)')
s = s.replace('static enum XML_Error handleUnknownEncoding(XML_Parser parser,\n                                            const XML_Char *_Nonnull encodingName);', 'static enum XML_Error handleUnknownEncoding(XML_Parser parser,\n                                            const XML_Char *_Nullable encodingName);')
s = re.sub(r'\bXML_Char \*(s|publicId)\) \{', r'XML_Char *_Nonnull \1) {', s)
s = s.replace('static void FASTCALL normalizePublicId(XML_Char *s);', 'static void FASTCALL normalizePublicId(XML_Char *_Nonnull s);')

# TAG_NAME.str is the element name; always set before storeAtts
rep("""typedef struct {
  const XML_Char *str;
  const XML_Char *localPart;""", """typedef struct {
  const XML_Char *_Nonnull str;
  const XML_Char *localPart;""")
rep("""      TAG_NAME name = {NULL, NULL, NULL, 0, 0, 0};
      name.str = poolStoreString(&parser->m_tempPool, enc, rawName,
                                 rawName + XmlNameLength(enc, rawName));
      if (! name.str)
        return XML_ERROR_NO_MEMORY;""", """      const XML_Char *nameStr = poolStoreString(
          &parser->m_tempPool, enc, rawName, rawName + XmlNameLength(enc, rawName));
      if (! nameStr)
        return XML_ERROR_NO_MEMORY;
      TAG_NAME name = {nameStr, NULL, NULL, 0, 0, 0};""")
rep("""  tagNamePtr->str = _Unsafe((XML_Char *)&_Mut *binding->uri);
  return XML_ERROR_NONE;""", """  XML_Char *bindingUriView = _Unsafe((XML_Char *)&_Mut *binding->uri);
  if (bindingUriView == NULL)
    return XML_ERROR_UNEXPECTED_STATE;
  tagNamePtr->str = bindingUriView;
  return XML_ERROR_NONE;""")
# expanded attribute names and bound prefix names are never NULL here (role machine); make it explicit
rep("""        {
          const size_t len = xcslen(s) + /*null terminator*/ 1;
          if (! poolAppendChars(&parser->m_tempPool, s, len))
            return XML_ERROR_NO_MEMORY;
        }""", """        if (s == NULL)
          return XML_ERROR_UNEXPECTED_STATE;
        {
          const size_t len = xcslen(s) + /*null terminator*/ 1;
          if (! poolAppendChars(&parser->m_tempPool, s, len))
            return XML_ERROR_NO_MEMORY;
        }""")
rep("""          s = b->prefix->name;
          const size_t len = xcslen(s) + /*null terminator*/ 1;
          if (! poolAppendChars(&parser->m_tempPool, s, len))
            return XML_ERROR_NO_MEMORY;""", """          s = b->prefix->name;
          if (s == NULL)
            return XML_ERROR_UNEXPECTED_STATE;
          const size_t len = xcslen(s) + /*null terminator*/ 1;
          if (! poolAppendChars(&parser->m_tempPool, s, len))
            return XML_ERROR_NO_MEMORY;""")
# declaration state must be complete before an attribute default is defined
rep("""      if (dtd->keepProcessing) {
        if (! defineAttribute(parser->m_declElementType,
                              parser->m_declAttributeId,""", """      if (dtd->keepProcessing) {
        if (parser->m_declElementType == NULL
            || parser->m_declAttributeId == NULL)
          return XML_ERROR_UNEXPECTED_STATE;
        if (! defineAttribute(parser->m_declElementType,
                              parser->m_declAttributeId,""")
rep("""        attVal = poolStart(&dtd->pool);
        poolFinish(&dtd->pool);
        /* ID attributes aren't allowed to have a default */
        if (! defineAttribute(""", """        attVal = poolStart(&dtd->pool);
        poolFinish(&dtd->pool);
        if (parser->m_declElementType == NULL
            || parser->m_declAttributeId == NULL)
          return XML_ERROR_UNEXPECTED_STATE;
        /* ID attributes aren't allowed to have a default */
        if (! defineAttribute(""")

# hash-table entries always carry a name (set right after insertion)
rep("""typedef struct {
  KEY name;
} NAMED;""", """typedef struct {
  KEY _Nonnull name;
} NAMED;""")
rep("""          KEY const key = table->v[i]->name;
          unsigned long newHash = hash(parser, key, keylen(key));""", """          KEY const key = table->v[i]->name;
          if (key == NULL) {
            FREE(PARSER_OF(table), newV);
            return NULL;
          }
          unsigned long newHash = hash(parser, key, keylen(key));""")
# pool copies return the (possibly NULL) block start, not the _Nonnull input
s = s.replace("""  s = pool->start;
  poolFinish(pool);
  return s;""", """  const XML_Char *copy = pool->start;
  poolFinish(pool);
  return copy;""")
# getContext / setContext: URI and prefix strings are non-empty by construction
rep("""    if (! poolAppendChars(
            &parser->m_tempPool,
            _Unsafe((const XML_Char *)&_Const *dtd->defaultPrefix.binding->uri),
            len)) {""", """    const XML_Char *defaultUri
        = _Unsafe((const XML_Char *)&_Const *dtd->defaultPrefix.binding->uri);
    if (defaultUri == NULL)
      return NULL;
    if (! poolAppendChars(&parser->m_tempPool, defaultUri, len)) {""")
rep("""    if (! poolAppendChars(&parser->m_tempPool,
                          _Unsafe((const XML_Char *)&_Const *prefix->binding->uri),
                          len))
      return NULL;""", """    const XML_Char *prefixUri
        = _Unsafe((const XML_Char *)&_Const *prefix->binding->uri);
    if (prefixUri == NULL)
      return NULL;
    if (! poolAppendChars(&parser->m_tempPool, prefixUri, len))
      return NULL;""")
rep("""        const XML_Char *const prefixName = poolCopyStringNoFinish(
            &dtd->pool, poolStart(&parser->m_tempPool));""", """        const XML_Char *const tempName = poolStart(&parser->m_tempPool);
        if (tempName == NULL)
          return XML_FALSE;
        const XML_Char *const prefixName
            = poolCopyStringNoFinish(&dtd->pool, tempName);""")

# an open entity record always names its entity
rep("""  struct open_internal_entity *next;
  ENTITY *entity;""", """  struct open_internal_entity *next;
  ENTITY *_Nonnull entity;""")

# configuration variants: XML_CONTEXT_BYTES == 0 and XML_GE == 0 code
rep("""    nLeftOver = s + len - end;""", """    nLeftOver = (int)(s + len - end);""")
rep("""  STRING_POOL *const pool = &(parser->m_dtd->entityValuePool);""", """  STRING_POOL *_Borrow pool = _Unsafe(&_Mut parser->m_dtd->entityValuePool);""")

rep("""    const char *end;
    int nLeftOver;
    enum XML_Status result;""", """    const char *end = NULL;
    int nLeftOver;
    enum XML_Status result;""")
rep("""    parser->m_errorCode
        = callProcessor(parser, s, parser->m_parseEndPtr = s + len, &end);""", """    parser->m_parseEndPtr = s + len;
    parser->m_errorCode
        = callProcessor(parser, s, parser->m_parseEndPtr, &end);""")

open(p, 'w').write(s)
print('pre ok')
