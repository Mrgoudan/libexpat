"""Ownership pass on the ORIGINAL xmlparse.c: owned array fields and their alloc/free sites.
Runs before pre.py."""
import re
p = '/home/ziruichen/bsd/libexpat/expat/lib/xmlparse.c'
s = open(p).read()

def rep(old, new, count=1):
    global s
    assert old in s, old[:70]
    s = s.replace(old, new, count)

# ---- struct fields
rep('''  NAMED **v;
  unsigned char power;''', '''  NAMED **_Owned _ArrayElem _Nullable v;
  unsigned char power;''')
rep('''  XML_Char *uri;
  size_t uriLen;''', '''  XML_Char *_Owned _ArrayElem _Nullable uri;
  size_t uriLen;''')
rep('''  DEFAULT_ATTRIBUTE *defaultAtts;
  HASH_TABLE defaultAttForName;''', '''  DEFAULT_ATTRIBUTE *_Owned _ArrayElem _Nullable defaultAtts;
  HASH_TABLE defaultAttForName;''')
rep('''  XML_Char *m_dataBuf;
  XML_Char *m_dataBufEnd;''', '''  XML_Char *_Owned _ArrayElem _Nullable m_dataBuf;
  XML_Char *m_dataBufEnd;''')
rep('''  const XML_Char *m_protocolEncodingName;''', '''  XML_Char *_Owned _ArrayElem _Nullable m_protocolEncodingName;''')
rep('''  void *m_unknownEncodingMem;''', '''  char *_Owned _ArrayElem _Nullable m_unknownEncodingMem;''')
rep('''  ATTRIBUTE *m_atts;
  NS_ATT *m_nsAtts;''', '''  ATTRIBUTE *_Owned _ArrayElem _Nullable m_atts;
  NS_ATT *_Owned _ArrayElem _Nullable m_nsAtts;''')
rep('''  XML_AttrInfo *m_attInfo;''', '''  XML_AttrInfo *_Owned _ArrayElem _Nullable m_attInfo;''')
rep('''  char *m_groupConnector;''', '''  char *_Owned _ArrayElem _Nullable m_groupConnector;''')

# ---- helpers
rep('''#  define FREE(parser, p) (parser->m_mem.free_fcn((p)))
#endif''', '''#  define FREE(parser, p) (parser->m_mem.free_fcn((p)))
#endif
// Take ownership of a freshly allocated array; a NULL result stays NULL.
#define TAKE_ARRAY(T, raw) (_Unsafe(__take_array_from_raw((T *)(raw))))
// Release an owned array field through a borrowed owner.
#define FREE_ARRAY(parser, field, T)                                          \\
  do {                                                                         \\
    T *_Owned _ArrayElem _Nullable bsc_tmp = nullptr;                          \\
    safe_swap(&_Mut(field), &_Mut bsc_tmp);                                    \\
    if (bsc_tmp != nullptr)                                                    \\
      _Unsafe FREE(parser, __move_array_to_raw(bsc_tmp));                      \\
  } while (0)
// Grow an owned array field in place; on failure the old buffer stays owned.
#define REALLOC_ARRAY(parser, field, T, bytes, ok)                             \\
  do {                                                                         \\
    T *_Owned _ArrayElem _Nullable bsc_old = nullptr;                          \\
    T *bsc_raw = nullptr;                                                      \\
    safe_swap(&_Mut(field), &_Mut bsc_old);                                    \\
    if (bsc_old != nullptr)                                                    \\
      _Unsafe bsc_raw = __move_array_to_raw(bsc_old);                          \\
    T *bsc_new = _Unsafe((T *)REALLOC(parser, bsc_raw, (bytes)));              \\
    if (bsc_new == nullptr) {                                                  \\
      _Unsafe { (field) = __take_array_from_raw(bsc_raw); }                    \\
      (ok) = XML_FALSE;                                                        \\
    } else {                                                                   \\
      _Unsafe { (field) = __take_array_from_raw(bsc_new); }                    \\
      (ok) = XML_TRUE;                                                         \\
    }                                                                          \\
  } while (0)''')

# ---- parserCreate
rep('''  parser->m_atts = MALLOC(parser, parser->m_attsSize * sizeof(ATTRIBUTE));
  if (parser->m_atts == NULL) {
    FREE(parser, parser);
    return NULL;
  }
#ifdef XML_ATTR_INFO
  parser->m_attInfo = MALLOC(parser, parser->m_attsSize * sizeof(XML_AttrInfo));
  if (parser->m_attInfo == NULL) {
    FREE(parser, parser->m_atts);
    FREE(parser, parser);
    return NULL;
  }
#endif
  parser->m_dataBuf = MALLOC(parser, INIT_DATA_BUF_SIZE * sizeof(XML_Char));
  if (parser->m_dataBuf == NULL) {
    FREE(parser, parser->m_atts);
#ifdef XML_ATTR_INFO
    FREE(parser, parser->m_attInfo);
#endif
    FREE(parser, parser);
    return NULL;
  }
  parser->m_dataBufEnd = parser->m_dataBuf + INIT_DATA_BUF_SIZE;''',
'''  parser->m_atts = TAKE_ARRAY(
      ATTRIBUTE, MALLOC(parser, parser->m_attsSize * sizeof(ATTRIBUTE)));
  if (parser->m_atts == NULL) {
    FREE(parser, parser);
    return NULL;
  }
#ifdef XML_ATTR_INFO
  parser->m_attInfo = TAKE_ARRAY(
      XML_AttrInfo, MALLOC(parser, parser->m_attsSize * sizeof(XML_AttrInfo)));
  if (parser->m_attInfo == NULL) {
    FREE_ARRAY(parser, parser->m_atts, ATTRIBUTE);
    FREE(parser, parser);
    return NULL;
  }
#endif
  parser->m_dataBuf = TAKE_ARRAY(
      XML_Char, MALLOC(parser, INIT_DATA_BUF_SIZE * sizeof(XML_Char)));
  if (parser->m_dataBuf == NULL) {
    FREE_ARRAY(parser, parser->m_atts, ATTRIBUTE);
#ifdef XML_ATTR_INFO
    FREE_ARRAY(parser, parser->m_attInfo, XML_AttrInfo);
#endif
    FREE(parser, parser);
    return NULL;
  }
  parser->m_dataBufEnd
      = _Unsafe((XML_Char *)&_Mut *parser->m_dataBuf + INIT_DATA_BUF_SIZE);''')
rep('''    if (parser->m_dtd == NULL) {
      FREE(parser, parser->m_dataBuf);
      FREE(parser, parser->m_atts);
#ifdef XML_ATTR_INFO
      FREE(parser, parser->m_attInfo);
#endif
      FREE(parser, parser);
      return NULL;
    }''', '''    if (parser->m_dtd == NULL) {
      FREE_ARRAY(parser, parser->m_dataBuf, XML_Char);
      FREE_ARRAY(parser, parser->m_atts, ATTRIBUTE);
#ifdef XML_ATTR_INFO
      FREE_ARRAY(parser, parser->m_attInfo, XML_AttrInfo);
#endif
      FREE(parser, parser);
      return NULL;
    }''')

# ---- XML_ParserReset / XML_SetEncoding
rep('''  FREE(parser, parser->m_unknownEncodingMem);
  if (parser->m_unknownEncodingRelease)
    callUnknownEncodingRelease(parser);
  poolClear(&parser->m_tempPool);
  poolClear(&parser->m_temp2Pool);
  FREE(parser, (void *)parser->m_protocolEncodingName);
  parser->m_protocolEncodingName = NULL;''', '''  FREE_ARRAY(parser, parser->m_unknownEncodingMem, char);
  if (parser->m_unknownEncodingRelease)
    callUnknownEncodingRelease(parser);
  poolClear(&parser->m_tempPool);
  poolClear(&parser->m_temp2Pool);
  FREE_ARRAY(parser, parser->m_protocolEncodingName, XML_Char);''')
rep('''  /* Get rid of any previous encoding name */
  FREE(parser, (void *)parser->m_protocolEncodingName);''', '''  /* Get rid of any previous encoding name */
  FREE_ARRAY(parser, parser->m_protocolEncodingName, XML_Char);''')

# ---- destroyBindings / XML_ParserFree
rep('''    bindings = b->nextTagBinding;
    FREE(parser, b->uri);
    FREE(parser, b);''', '''    bindings = b->nextTagBinding;
    _Unsafe FREE_ARRAY(parser, b->uri, XML_Char);
    FREE(parser, b);''')
rep('''  poolDestroy(&parser->m_temp2Pool);
  FREE(parser, (void *)parser->m_protocolEncodingName);
#ifdef XML_DTD''', '''  poolDestroy(&parser->m_temp2Pool);
  FREE_ARRAY(&_Mut *parser, parser->m_protocolEncodingName, XML_Char);
#ifdef XML_DTD''')
rep('''  FREE(parser, parser->m_atts);
#ifdef XML_ATTR_INFO
  FREE(parser, parser->m_attInfo);
#endif
  FREE(parser, parser->m_groupConnector);
  // NOTE: We are avoiding FREE(..) here because parser->m_buffer
  //       is not being allocated with MALLOC(..) but with plain
  //       .malloc_fcn(..).
  parser->m_mem.free_fcn(parser->m_buffer);
  FREE(parser, parser->m_dataBuf);
  FREE(parser, parser->m_nsAtts);
  FREE(parser, parser->m_unknownEncodingMem);''', '''  FREE_ARRAY(&_Mut *parser, parser->m_atts, ATTRIBUTE);
#ifdef XML_ATTR_INFO
  FREE_ARRAY(&_Mut *parser, parser->m_attInfo, XML_AttrInfo);
#endif
  FREE_ARRAY(&_Mut *parser, parser->m_groupConnector, char);
  // NOTE: We are avoiding FREE(..) here because parser->m_buffer
  //       is not being allocated with MALLOC(..) but with plain
  //       .malloc_fcn(..).
  parser->m_mem.free_fcn(parser->m_buffer);
  FREE_ARRAY(&_Mut *parser, parser->m_dataBuf, XML_Char);
  FREE_ARRAY(&_Mut *parser, parser->m_nsAtts, NS_ATT);
  FREE_ARRAY(&_Mut *parser, parser->m_unknownEncodingMem, char);''')

# ---- storeAtts
rep('''  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
  int attIndex = 0;
  XML_Char *uri;''', '''  DTD *const dtd = parser->m_dtd; /* save one level of indirection */
  int attIndex = 0;
  XML_Char *uri;
  if (parser->m_atts == NULL)
    return XML_ERROR_NO_MEMORY;''')
rep('''  size_t n = (size_t)XmlGetAttributes(enc, attStr, (int)parser->m_attsSize,
                                      parser->m_atts);''', '''  size_t n = (size_t)XmlGetAttributes(
      enc, attStr, (int)parser->m_attsSize,
      _Unsafe((ATTRIBUTE *)&_Mut *parser->m_atts));''')
rep('''    ATTRIBUTE *const temp = REALLOC(parser, parser->m_atts,
                                    parser->m_attsSize * sizeof(ATTRIBUTE));
    if (temp == NULL) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }
    parser->m_atts = temp;''', '''    XML_Bool attsOk = XML_FALSE;
    REALLOC_ARRAY(parser, parser->m_atts, ATTRIBUTE,
                  parser->m_attsSize * sizeof(ATTRIBUTE), attsOk);
    if (! attsOk) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }''')
rep('''    XML_AttrInfo *const temp2 = REALLOC(
        parser, parser->m_attInfo, parser->m_attsSize * sizeof(XML_AttrInfo));
    if (temp2 == NULL) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }
    parser->m_attInfo = temp2;''', '''    XML_Bool attInfoOk = XML_FALSE;
    REALLOC_ARRAY(parser, parser->m_attInfo, XML_AttrInfo,
                  parser->m_attsSize * sizeof(XML_AttrInfo), attInfoOk);
    if (! attInfoOk) {
      parser->m_attsSize = oldAttsSize;
      return XML_ERROR_NO_MEMORY;
    }''')
rep('''      XmlGetAttributes(enc, attStr, (int)n, parser->m_atts);
    }
  }

  /* the attribute list for the application */
  const XML_Char **const appAtts = (const XML_Char **)parser->m_atts;
  for (size_t i = 0; i < n; i++) {
    ATTRIBUTE *currAtt = &parser->m_atts[i];
#ifdef XML_ATTR_INFO
    XML_AttrInfo *currAttInfo = &parser->m_attInfo[i];
#endif''', '''      XmlGetAttributes(enc, attStr, (int)n,
                       _Unsafe((ATTRIBUTE *)&_Mut *parser->m_atts));
    }
  }
  if (parser->m_atts == NULL)
    return XML_ERROR_NO_MEMORY;

  /* the attribute list for the application */
  const XML_Char **const appAtts
      = _Unsafe((const XML_Char **)(ATTRIBUTE *)&_Mut *parser->m_atts);
  for (size_t i = 0; i < n; i++) {
    ATTRIBUTE *currAtt = _Unsafe((ATTRIBUTE *)&_Mut parser->m_atts[i]);
#ifdef XML_ATTR_INFO
    XML_AttrInfo *currAttInfo = _Unsafe((XML_AttrInfo *)&_Mut parser->m_attInfo[i]);
#endif''')
rep('''          const DEFAULT_ATTRIBUTE *const att
              = elementType->defaultAtts + nameAndDefaultAttribute->attIndex;''', '''          const DEFAULT_ATTRIBUTE *const att
              = _Unsafe((const DEFAULT_ATTRIBUTE *)&_Const *elementType->defaultAtts
                        + nameAndDefaultAttribute->attIndex);''')
rep('''    const DEFAULT_ATTRIBUTE *da = elementType->defaultAtts + i;''', '''    const DEFAULT_ATTRIBUTE *da
        = _Unsafe((const DEFAULT_ATTRIBUTE *)&_Const *elementType->defaultAtts + i);''')
rep('''      NS_ATT *const temp
          = REALLOC(parser, parser->m_nsAtts, nsAttsSize * sizeof(NS_ATT));
      if (! temp) {
        /* Restore actual size of memory in m_nsAtts */
        parser->m_nsAttsPower = oldNsAttsPower;
        return XML_ERROR_NO_MEMORY;
      }
      parser->m_nsAtts = temp;
      version = 0; /* force re-initialization of m_nsAtts hash table */
    }''', '''      XML_Bool nsAttsOk = XML_FALSE;
      REALLOC_ARRAY(parser, parser->m_nsAtts, NS_ATT, nsAttsSize * sizeof(NS_ATT),
                    nsAttsOk);
      if (! nsAttsOk) {
        /* Restore actual size of memory in m_nsAtts */
        parser->m_nsAttsPower = oldNsAttsPower;
        return XML_ERROR_NO_MEMORY;
      }
      version = 0; /* force re-initialization of m_nsAtts hash table */
    }
    if (parser->m_nsAtts == NULL)
      return XML_ERROR_NO_MEMORY;''')
rep('''        if (! poolAppendChars(&parser->m_tempPool, b->uri, b->uriLen))
          return XML_ERROR_NO_MEMORY;

        sip24_update(&sip_state, b->uri, b->uriLen * sizeof(XML_Char));''', '''        const XML_Char *bUri = _Unsafe((const XML_Char *)&_Const *b->uri);
        if (! poolAppendChars(&parser->m_tempPool, bUri, b->uriLen))
          return XML_ERROR_NO_MEMORY;

        sip24_update(&sip_state, bUri, b->uriLen * sizeof(XML_Char));''')
rep('''    uri = MALLOC(parser, (totalLen + EXPAND_SPARE) * sizeof(XML_Char));
    if (! uri)
      return XML_ERROR_NO_MEMORY;
    binding->uriAlloc = totalLen + EXPAND_SPARE;
    memcpy(uri, binding->uri, binding->uriLen * sizeof(XML_Char));
    for (TAG *p = parser->m_tagStack; p; p = p->parent)
      if (p->name.str == binding->uri)
        p->name.str = uri;
    FREE(parser, binding->uri);
    binding->uri = uri;
  }
  /* if m_namespaceSeparator != '\\0' then uri includes it already */
  uri = binding->uri + binding->uriLen;''', '''    uri = MALLOC(parser, (totalLen + EXPAND_SPARE) * sizeof(XML_Char));
    if (! uri)
      return XML_ERROR_NO_MEMORY;
    binding->uriAlloc = totalLen + EXPAND_SPARE;
    XML_Char *oldUri = _Unsafe((XML_Char *)&_Mut *binding->uri);
    memcpy(uri, oldUri, binding->uriLen * sizeof(XML_Char));
    for (TAG *p = parser->m_tagStack; p; p = p->parent)
      if (p->name.str == oldUri)
        p->name.str = uri;
    _Unsafe FREE_ARRAY(parser, binding->uri, XML_Char);
    _Unsafe binding->uri = __take_array_from_raw(uri);
  }
  /* if m_namespaceSeparator != '\\0' then uri includes it already */
  uri = _Unsafe((XML_Char *)&_Mut *binding->uri + binding->uriLen);''')
rep('''  tagNamePtr->str = binding->uri;
  return XML_ERROR_NONE;''', '''  tagNamePtr->str = _Unsafe((XML_Char *)&_Mut *binding->uri);
  return XML_ERROR_NONE;''')

s = s.replace('(const XML_Char **)parser->m_atts)', '(const XML_Char **)(ATTRIBUTE *)&_Mut *parser->m_atts)')

# ---- addBinding
rep('''      XML_Char *temp
          = REALLOC(parser, b->uri, sizeof(XML_Char) * (len + EXPAND_SPARE));
      if (temp == NULL)
        return XML_ERROR_NO_MEMORY;
      b->uri = temp;
      b->uriAlloc = len + EXPAND_SPARE;''', '''      XML_Bool uriOk = XML_FALSE;
      _Unsafe REALLOC_ARRAY(parser, b->uri, XML_Char,
                            sizeof(XML_Char) * (len + EXPAND_SPARE), uriOk);
      if (! uriOk)
        return XML_ERROR_NO_MEMORY;
      b->uriAlloc = len + EXPAND_SPARE;''')
rep('''    b->uri = MALLOC(parser, sizeof(XML_Char) * (len + EXPAND_SPARE));
    if (! b->uri) {
      FREE(parser, b);
      return XML_ERROR_NO_MEMORY;
    }
    b->uriAlloc = len + EXPAND_SPARE;
  }
  b->uriLen = len;
  memcpy(b->uri, uri, len * sizeof(XML_Char));''', '''    XML_Char *newUri = MALLOC(parser, sizeof(XML_Char) * (len + EXPAND_SPARE));
    if (! newUri) {
      FREE(parser, b);
      return XML_ERROR_NO_MEMORY;
    }
    _Unsafe b->uri = __take_array_from_raw(newUri);
    b->uriAlloc = len + EXPAND_SPARE;
  }
  b->uriLen = len;
  memcpy(_Unsafe((XML_Char *)&_Mut *b->uri), uri, len * sizeof(XML_Char));''')

# ---- getContext
rep('''    if (! poolAppendChars(&parser->m_tempPool, dtd->defaultPrefix.binding->uri,
                          len)) {''', '''    if (! poolAppendChars(
            &parser->m_tempPool,
            _Unsafe((const XML_Char *)&_Const *dtd->defaultPrefix.binding->uri),
            len)) {''')
rep('''    if (! poolAppendChars(&parser->m_tempPool, prefix->binding->uri, len))
      return NULL;''', '''    if (! poolAppendChars(&parser->m_tempPool,
                          _Unsafe((const XML_Char *)&_Const *prefix->binding->uri),
                          len))
      return NULL;''')

# ---- unknown encoding memory
rep('''      parser->m_unknownEncodingMem = MALLOC(parser, XmlSizeOfUnknownEncoding());''', '''      parser->m_unknownEncodingMem = TAKE_ARRAY(
          char, MALLOC(parser, (size_t)XmlSizeOfUnknownEncoding()));''')
rep('''      enc = (parser->m_ns ? XmlInitUnknownEncodingNS : XmlInitUnknownEncoding)(
          parser->m_unknownEncodingMem, info.map,''', '''      enc = (parser->m_ns ? XmlInitUnknownEncodingNS : XmlInitUnknownEncoding)(
          _Unsafe((void *)&_Mut *parser->m_unknownEncodingMem), info.map,''')

# ---- protocol encoding name
rep('''#else
  s = parser->m_protocolEncodingName;
#endif
  if ((parser->m_ns ? XmlInitEncodingNS : XmlInitEncoding)(
          &parser->m_initEncoding, &parser->m_encoding, s))
    return XML_ERROR_NONE;
  return handleUnknownEncoding(parser, parser->m_protocolEncodingName);''', '''#else
  s = (parser->m_protocolEncodingName == NULL)
          ? NULL
          : _Unsafe((const char *)&_Const *parser->m_protocolEncodingName);
#endif
  if ((parser->m_ns ? XmlInitEncodingNS : XmlInitEncoding)(
          &parser->m_initEncoding, &parser->m_encoding, s))
    return XML_ERROR_NONE;
  const XML_Char *protocolEncodingName
      = (parser->m_protocolEncodingName == NULL)
            ? NULL
            : _Unsafe((const XML_Char *)&_Const *parser->m_protocolEncodingName);
  return handleUnknownEncoding(parser, protocolEncodingName);''')

# ---- copyString hands out an owned array
rep('''static XML_Char *copyString(const XML_Char *s, XML_Parser parser);''', '''static XML_Char *_Owned _ArrayElem _Nullable copyString(const XML_Char *s,
                                                        XML_Parser parser);''')
rep('''static XML_Char *
copyString(const XML_Char *s, XML_Parser parser) {''', '''static XML_Char *_Owned _ArrayElem _Nullable
copyString(const XML_Char *s, XML_Parser parser) {''')
rep('''  /* Copy the original into place */
  memcpy(result, s, bytesRequired);

  return result;
}''', '''  /* Copy the original into place */
  memcpy(result, s, bytesRequired);

  return TAKE_ARRAY(XML_Char, result);
}''')

# ---- hash table slots
rep('''    table->v = MALLOC(table->parser, tsize);
    if (! table->v) {
      table->size = 0;
      return NULL;
    }
    memset(table->v, 0, tsize);
    i = hash(parser, name, nameLen) & ((unsigned long)table->size - 1);
  } else {
    unsigned long h = hash(parser, name, nameLen);''', '''    table->v = TAKE_ARRAY(NAMED *, MALLOC(table->parser, tsize));
    if (table->v == NULL) {
      table->size = 0;
      return NULL;
    }
    memset(_Unsafe((NAMED **)&_Mut *table->v), 0, tsize);
    i = hash(parser, name, nameLen) & ((unsigned long)table->size - 1);
  } else {
    if (table->v == NULL)
      return NULL;
    unsigned long h = hash(parser, name, nameLen);''')
rep('''      FREE(table->parser, table->v);
      table->v = newV;''', '''      FREE_ARRAY(table->parser, table->v, NAMED *);
      table->v = TAKE_ARRAY(NAMED *, newV);
      if (table->v == NULL)
        return NULL;''')
rep('''hashTableClear(HASH_TABLE *table) {
  size_t i;
  for (i = 0; i < table->size; i++) {''', '''hashTableClear(HASH_TABLE *table) {
  size_t i;
  if (table->v == NULL)
    return;
  for (i = 0; i < table->size; i++) {''')
rep('''hashTableDestroy(HASH_TABLE *table) {
  size_t i;
  for (i = 0; i < table->size; i++)
    FREE(table->parser, table->v[i]);
  FREE(table->parser, table->v);
}''', '''hashTableDestroy(HASH_TABLE *table) {
  size_t i;
  if (table->v == NULL)
    return;
  for (i = 0; i < table->size; i++)
    FREE(table->parser, table->v[i]);
  FREE_ARRAY(table->parser, table->v, NAMED *);
}''')
rep('''  iter->p = table->v;
  iter->end = iter->p ? iter->p + table->size : NULL;''', '''  iter->p = (table->v == NULL) ? NULL : _Unsafe((NAMED **)&_Const *table->v);
  iter->end = iter->p ? iter->p + table->size : NULL;''')

# ---- default attributes
rep('''    DEFAULT_ATTRIBUTE *const temp = REALLOC(
        parser, type->defaultAtts, (count * sizeof(DEFAULT_ATTRIBUTE)));
    if (temp == NULL)
      return 0;
    type->allocDefaultAtts = count;
    type->defaultAtts = temp;
  }
  att = type->defaultAtts + type->nDefaultAtts;''', '''    XML_Bool defaultAttsOk = XML_FALSE;
    _Unsafe REALLOC_ARRAY(parser, type->defaultAtts, DEFAULT_ATTRIBUTE,
                          count * sizeof(DEFAULT_ATTRIBUTE), defaultAttsOk);
    if (! defaultAttsOk)
      return 0;
    type->allocDefaultAtts = count;
  }
  att = _Unsafe((DEFAULT_ATTRIBUTE *)&_Mut *type->defaultAtts
                + type->nDefaultAtts);''')
s = s.replace('''    hashTableDestroy(&(e->defaultAttForName));
    FREE(parser, e->defaultAtts);''', '''    hashTableDestroy(&(e->defaultAttForName));
    _Unsafe FREE_ARRAY(parser, e->defaultAtts, DEFAULT_ATTRIBUTE);''')
rep('''      newE->defaultAtts
          = MALLOC(parser, oldE->nDefaultAtts * sizeof(DEFAULT_ATTRIBUTE));
      if (! newE->defaultAtts) {
        return 0;
      }''', '''      DEFAULT_ATTRIBUTE *newDefaultAtts
          = MALLOC(parser, oldE->nDefaultAtts * sizeof(DEFAULT_ATTRIBUTE));
      if (! newDefaultAtts) {
        return 0;
      }
      _Unsafe newE->defaultAtts = __take_array_from_raw(newDefaultAtts);''')

# ---- group connector
rep('''          char *const new_connector = REALLOC(parser, parser->m_groupConnector,
                                              parser->m_groupSize *= 2);
          if (new_connector == NULL) {
            parser->m_groupSize /= 2;
            return XML_ERROR_NO_MEMORY;
          }
          parser->m_groupConnector = new_connector;
        } else {
          parser->m_groupConnector = MALLOC(parser, parser->m_groupSize = 32);
          if (! parser->m_groupConnector) {
            parser->m_groupSize = 0;
            return XML_ERROR_NO_MEMORY;
          }
        }
      }
      parser->m_groupConnector[parser->m_prologState.level] = 0;''', '''          parser->m_groupSize *= 2;
          XML_Bool connectorOk = XML_FALSE;
          REALLOC_ARRAY(parser, parser->m_groupConnector, char,
                        parser->m_groupSize, connectorOk);
          if (! connectorOk) {
            parser->m_groupSize /= 2;
            return XML_ERROR_NO_MEMORY;
          }
        } else {
          parser->m_groupSize = 32;
          parser->m_groupConnector
              = TAKE_ARRAY(char, MALLOC(parser, parser->m_groupSize));
          if (! parser->m_groupConnector) {
            parser->m_groupSize = 0;
            return XML_ERROR_NO_MEMORY;
          }
        }
      }
      if (parser->m_groupConnector == NULL)
        return XML_ERROR_UNEXPECTED_STATE;
      parser->m_groupConnector[parser->m_prologState.level] = 0;''')
rep('''    case XML_ROLE_GROUP_SEQUENCE:
      if (parser->m_groupConnector[parser->m_prologState.level] == ASCII_PIPE)''', '''    case XML_ROLE_GROUP_SEQUENCE:
      if (parser->m_groupConnector == NULL)
        return XML_ERROR_UNEXPECTED_STATE;
      if (parser->m_groupConnector[parser->m_prologState.level] == ASCII_PIPE)''')
rep('''    case XML_ROLE_GROUP_CHOICE:
      if (parser->m_groupConnector[parser->m_prologState.level] == ASCII_COMMA)''', '''    case XML_ROLE_GROUP_CHOICE:
      if (parser->m_groupConnector == NULL)
        return XML_ERROR_UNEXPECTED_STATE;
      if (parser->m_groupConnector[parser->m_prologState.level] == ASCII_COMMA)''')

# ---- data buffer views
s = re.sub(r'\(ICHAR \*\)parser->m_dataBuf\b', '(ICHAR *)&_Mut *parser->m_dataBuf', s)
s = re.sub(r'(m_handlerArg, )parser->m_dataBuf,', r'\1(XML_Char *)&_Mut *parser->m_dataBuf,', s)
s = s.replace('''      parser->m_defaultHandler(parser->m_handlerArg, parser->m_dataBuf,
                               (int)(dataPtr - (ICHAR *)&_Mut *parser->m_dataBuf));''', '''      parser->m_defaultHandler(parser->m_handlerArg,
                               (XML_Char *)&_Mut *parser->m_dataBuf,
                               (int)(dataPtr - (ICHAR *)&_Mut *parser->m_dataBuf));''')
assert not [l for l in s.split('\n') if 'parser->m_dataBuf,' in l and '&_Mut' not in l and 'FREE_ARRAY' not in l]

open(p, 'w').write(s)
print('pre_own ok')
