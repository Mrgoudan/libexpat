#include <stdlib.h>
// Same logic as pool_wrong.cbs with BSC annotations erased (what plain C sees).
typedef struct { char *data; } Pool;
static Pool *pool_new(char k){ Pool *p=malloc(sizeof(Pool)); p->data=malloc(1); *p->data=k; return p; }
static const char *pool_key(const Pool *p){ return p->data; }
static void pool_free(Pool *p){ free(p->data); free(p); }
static char key_first(const char *k){ return *k; }
static char dtdCopy_wrong(void){
  Pool *oldDtd=pool_new('a');
  Pool *newDtd=pool_new('b');
  const char *key=pool_key(oldDtd);
  pool_free(oldDtd);              // owner freed while key still held
  char c=key_first(key);         // use-after-free
  pool_free(newDtd);
  return c;
}
int main(void){ return (int)dtdCopy_wrong(); }
