// Fuzz harness: unknown-encoding callbacks, tiny-chunk GetBuffer/ParseBuffer, stop/resume from every handler.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "expat.h"

static uint8_t g_flags;

static int XMLCALL
convert(void *data, const char *s) {
  (void)data;
  const unsigned char c = (unsigned char)s[0];
  if (c >= 0xF0)
    return 0x10000 + ((c & 0x0F) << 8) + (unsigned char)s[1];
  if (c >= 0xE0)
    return 0x800 + ((c & 0x0F) << 8) + (unsigned char)s[1];
  return 0x100 + (c & 0x7F);
}
static int XMLCALL
unknownEncoding(void *data, const XML_Char *name, XML_Encoding *info) {
  (void)data; (void)name;
  for (int i = 0; i < 256; i++) {
    if (i < 0x80)
      info->map[i] = i;
    else if (i < 0xC0)
      info->map[i] = (g_flags & 32) ? -1 : 0x100 + i;
    else if (i < 0xE0)
      info->map[i] = 0xFF00 + i;
    else if (i < 0xF0)
      info->map[i] = -2;
    else
      info->map[i] = -3;
  }
  info->data = NULL;
  info->convert = convert;
  info->release = NULL;
  return XML_STATUS_OK;
}
static void
maybeStop(XML_Parser p, const XML_Char *s, int len) {
  if (len > 1 && s[0] == 's')
    XML_StopParser(p, s[1] == 'r' ? XML_FALSE : XML_TRUE);
}
static void XMLCALL
chars(void *ud, const XML_Char *s, int len) { maybeStop((XML_Parser)ud, s, len); }
static void XMLCALL
startEl(void *ud, const XML_Char *name, const XML_Char **atts) {
  (void)atts;
  maybeStop((XML_Parser)ud, name, 2);
}
static void XMLCALL
endEl(void *ud, const XML_Char *name) { maybeStop((XML_Parser)ud, name, 2); }
static void XMLCALL
pi(void *ud, const XML_Char *target, const XML_Char *data) { (void)data; maybeStop((XML_Parser)ud, target, 2); }
static void XMLCALL
comment(void *ud, const XML_Char *data) { maybeStop((XML_Parser)ud, data, 2); }
static void XMLCALL
entityDecl(void *ud, const XML_Char *name, int isPE, const XML_Char *value, int len,
           const XML_Char *base, const XML_Char *sys, const XML_Char *pub, const XML_Char *notation) {
  (void)isPE; (void)value; (void)len; (void)base; (void)sys; (void)pub; (void)notation;
  maybeStop((XML_Parser)ud, name, 2);
}
static void XMLCALL
eldecl(void *ud, const XML_Char *name, XML_Content *model) {
  maybeStop((XML_Parser)ud, name, 2);
  XML_FreeContentModel((XML_Parser)ud, model);
}
static void XMLCALL
attlist(void *ud, const XML_Char *el, const XML_Char *attname, const XML_Char *type,
        const XML_Char *dflt, int isrequired) {
  (void)el; (void)type; (void)dflt; (void)isrequired;
  maybeStop((XML_Parser)ud, attname, 2);
}
static void XMLCALL
startDoctype(void *ud, const XML_Char *name, const XML_Char *sysid, const XML_Char *pubid, int hasInternal) {
  (void)sysid; (void)pubid; (void)hasInternal;
  maybeStop((XML_Parser)ud, name, 2);
}
static void XMLCALL
xmldecl(void *ud, const XML_Char *version, const XML_Char *encoding, int standalone) {
  (void)version; (void)standalone;
  if (encoding)
    maybeStop((XML_Parser)ud, encoding, 2);
}
static void XMLCALL
dflt(void *ud, const XML_Char *s, int len) { (void)ud; (void)s; (void)len; }

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 2)
    return 0;
  g_flags = data[0];
  data++; size--;
  XML_Parser p = (g_flags & 1) ? XML_ParserCreateNS(NULL, '\n') : XML_ParserCreate((g_flags & 64) ? "X-FUZZ" : NULL);
  if (! p)
    return 0;
  XML_SetHashSalt(p, 0x5eed);
  XML_SetUserData(p, p);
  XML_SetUnknownEncodingHandler(p, unknownEncoding, NULL);
  XML_SetElementHandler(p, startEl, endEl);
  XML_SetCharacterDataHandler(p, chars);
  XML_SetProcessingInstructionHandler(p, pi);
  XML_SetCommentHandler(p, comment);
  XML_SetEntityDeclHandler(p, entityDecl);
  XML_SetElementDeclHandler(p, eldecl);
  XML_SetAttlistDeclHandler(p, attlist);
  XML_SetStartDoctypeDeclHandler(p, startDoctype);
  XML_SetXmlDeclHandler(p, xmldecl);
  if (g_flags & 2)
    XML_SetDefaultHandler(p, dflt);
  if (g_flags & 4)
    XML_SetParamEntityParsing(p, XML_PARAM_ENTITY_PARSING_ALWAYS);
  if (g_flags & 8)
    XML_SetReparseDeferralEnabled(p, XML_FALSE);

  const size_t chunk = 1 + (g_flags >> 4 & 7);
  size_t off = 0;
  enum XML_Status st = XML_STATUS_OK;
  int rounds = 0;
  while (off < size && rounds++ < 100000) {
    size_t n = size - off < chunk ? size - off : chunk;
    void *buf = XML_GetBuffer(p, (int)n);
    if (! buf)
      break;
    memcpy(buf, data + off, n);
    off += n;
    st = XML_ParseBuffer(p, (int)n, off == size);
    int guard = 0;
    while (st == XML_STATUS_SUSPENDED && guard++ < 8)
      st = XML_ResumeParser(p);
    if (st == XML_STATUS_ERROR)
      break;
  }
  XML_GetCurrentLineNumber(p);
  XML_GetCurrentColumnNumber(p);
  XML_GetInputContext(p, NULL, NULL);
  XML_ParserFree(p);
  return 0;
}
