// Fuzz harness for DTD/parameter-entity/external-subset paths with suspend/resume.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "expat.h"

static const uint8_t *g_ext;
static size_t g_ext_len;
static int g_depth;

static int XMLCALL
extRef(XML_Parser parser, const XML_Char *context, const XML_Char *base,
       const XML_Char *systemId, const XML_Char *publicId) {
  (void)base; (void)systemId; (void)publicId;
  if (g_depth > 2)
    return XML_STATUS_OK;
  XML_Parser ext = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (! ext)
    return XML_STATUS_ERROR;
  g_depth++;
  XML_Parse(ext, (const char *)g_ext, (int)(g_ext_len / 2), 0);
  XML_Parse(ext, (const char *)g_ext + g_ext_len / 2, (int)(g_ext_len - g_ext_len / 2), 1);
  g_depth--;
  XML_ParserFree(ext);
  return XML_STATUS_OK;
}
static void XMLCALL
chars(void *ud, const XML_Char *s, int len) {
  XML_Parser p = (XML_Parser)ud;
  if (len > 1 && s[0] == 's')
    XML_StopParser(p, s[1] == 'r' ? XML_FALSE : XML_TRUE);
}
static void XMLCALL
startEl(void *ud, const XML_Char *name, const XML_Char **atts) {
  (void)name; (void)atts;
  XML_DefaultCurrent((XML_Parser)ud);
}
static void XMLCALL
eldecl(void *ud, const XML_Char *name, XML_Content *model) {
  (void)name;
  XML_FreeContentModel((XML_Parser)ud, model);
}
static void XMLCALL
dflt(void *ud, const XML_Char *s, int len) { (void)ud; (void)s; (void)len; }
static void XMLCALL
skipped(void *ud, const XML_Char *name, int isPE) { (void)ud; (void)name; (void)isPE; }

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 2)
    return 0;
  const uint8_t flags = data[0];
  data++; size--;
  const uint8_t *sep = memchr(data, 0, size);
  size_t mainLen = sep ? (size_t)(sep - data) : size;
  g_ext = sep ? sep + 1 : (const uint8_t *)"";
  g_ext_len = sep ? size - mainLen - 1 : 0;
  g_depth = 0;

  XML_Parser p = (flags & 1) ? XML_ParserCreateNS(NULL, ' ') : XML_ParserCreate(NULL);
  if (! p)
    return 0;
  XML_SetHashSalt(p, 0x5eed);
  XML_SetUserData(p, p);
  XML_SetParamEntityParsing(p, (flags & 2) ? XML_PARAM_ENTITY_PARSING_ALWAYS
                                           : XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE);
  XML_SetExternalEntityRefHandler(p, extRef);
  XML_SetElementHandler(p, startEl, NULL);
  XML_SetCharacterDataHandler(p, chars);
  XML_SetElementDeclHandler(p, eldecl);
  XML_SetDefaultHandler(p, dflt);
  XML_SetSkippedEntityHandler(p, skipped);
  if (flags & 4)
    XML_UseForeignDTD(p, XML_TRUE);
  if (flags & 8)
    XML_SetReturnNSTriplet(p, 1);

  size_t half = mainLen / 2;
  enum XML_Status st = XML_Parse(p, (const char *)data, (int)half, 0);
  int guard = 0;
  while (st == XML_STATUS_SUSPENDED && guard++ < 8)
    st = XML_ResumeParser(p);
  if (st != XML_STATUS_ERROR) {
    st = XML_Parse(p, (const char *)data + half, (int)(mainLen - half), 1);
    guard = 0;
    while (st == XML_STATUS_SUSPENDED && guard++ < 8)
      st = XML_ResumeParser(p);
  }
  XML_GetCurrentLineNumber(p);
  if (flags & 16)
    XML_ParserReset(p, NULL);
  XML_ParserFree(p);
  return 0;
}
