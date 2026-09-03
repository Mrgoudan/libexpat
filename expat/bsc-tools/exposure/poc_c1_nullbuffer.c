#include "expat.h"
#include <string.h>
int main(int argc, char **argv){
  const char *s = (argc>1) ? argv[1] : "<r>";
  XML_Parser p = XML_ParserCreate(NULL);
  XML_Parse(p, s, (int)strlen(s), 0);   /* isFinal = 0 : no early return */
  XML_ParserFree(p);
  return 0;
}
