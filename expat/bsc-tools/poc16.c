#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "expat.h"
static XML_Parser g_child;
static int XMLCALL ref(XML_Parser p, const XML_Char *ctx, const XML_Char *b, const XML_Char *s, const XML_Char *pub) {
  (void)b; (void)s; (void)pub;
  g_child = XML_ExternalEntityParserCreate(p, ctx, NULL);  /* kept alive on purpose */
  return XML_STATUS_OK;
}
static void XMLCALL start(void *ud, const XML_Char *name, const XML_Char **atts) {
  (void)ud;
  for (int i = 0; atts[i]; i += 2) printf("  %s %s=[%s]\n", name, atts[i], atts[i + 1]);
}
int main(int argc, char **argv) {
  const char *doc = "<!DOCTYPE doc [<!ENTITY e SYSTEM 'x'><!ELEMENT doc ANY><!ELEMENT tag EMPTY>"
                    "<!ATTLIST tag first CDATA #IMPLIED><!ATTLIST tag second NMTOKENS #IMPLIED>]><doc>&e;</doc>";
  XML_Parser parent = XML_ParserCreate(NULL);
  XML_SetExternalEntityRefHandler(parent, ref);
  if (XML_Parse(parent, doc, (int)strlen(doc), 1) != XML_STATUS_OK) { puts("parent parse failed"); return 1; }
  XML_SetStartElementHandler(g_child, start);
  if (argc > 1 && !strcmp(argv[1], "free")) {
    XML_ParserFree(parent);                       /* documented misuse: parent freed first */
  } else {
    XML_ParserReset(parent, NULL);                /* parent reused for another document */
    const char *doc2 = "<!DOCTYPE d [<!ATTLIST tag zzzzzzzzzzzzzzzzzzzzzzzz CDATA #IMPLIED>"
                       "<!ATTLIST tag yyyyyyyyyyyyyyyyyyyyyyyy CDATA #IMPLIED>]><d/>";
    XML_Parse(parent, doc2, (int)strlen(doc2), 1);
  }
  const char *ent = "<tag second=' a  b '/>";
  puts(argc > 1 ? "after XML_ParserFree(parent):" : "after XML_ParserReset(parent) + second document:");
  XML_Parse(g_child, ent, (int)strlen(ent), 1);
  return 0;
}
