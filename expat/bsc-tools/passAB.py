import re, sys
p='/home/ziruichen/bsd/libexpat/expat/lib/xmlparse.c'
s=open(p).read()

def wrap_calls(s, prefix_re):
    """Wrap every call whose callee text matches prefix_re (ending at '(') in _Unsafe(...)."""
    out=[]; i=0; rx=re.compile(prefix_re)
    while True:
        m=rx.search(s,i)
        if not m: out.append(s[i:]); break
        j=m.start(); k=m.end(); depth=1
        while depth:
            c=s[k]; depth+=(c=='(')-(c==')'); k+=1
        # skip if already directly wrapped
        if s[max(0,j-8):j].endswith('_Unsafe('):
            out.append(s[i:k]); i=k; continue
        out.append(s[i:j]+'_Unsafe('+s[j:k]+')'); i=k
    return ''.join(out)

# 1. Processor typedef
s=s.replace('typedef enum XML_Error PTRCALL Processor(XML_Parser parser, const char *start,',
            'typedef _Safe enum XML_Error PTRCALL Processor(XML_Parser _Borrow parser,\n                                               const char *start,')
# 2. parser params
s=re.sub(r'\bXML_Parser (parser|oldParser|rootParser|originParser|parentParser)(?=\s*[,)])', r'XML_Parser _Borrow \1', s)
s=re.sub(r'^(XML_\w+)\(XML_Parser _Borrow (parser|oldParser)', r'\1(XML_Parser _Borrow _Nullable \2', s, flags=re.M)
s=s.replace('XML_ParserFree(XML_Parser _Borrow _Nullable parser)','XML_ParserFree(XML_Parser _Owned _Nullable parser)')
s=re.sub(r'^(testingAccountingGetCountBytes\w+)\(XML_Parser _Borrow parser', r'\1(XML_Parser _Borrow _Nullable parser', s, flags=re.M)
s=s.replace('const XML_Parser rootParser','XML_Parser _Borrow rootParser')
# 3. return types
for name in ['XML_ParserCreate','XML_ParserCreateNS','XML_ParserCreate_MM','XML_ExternalEntityParserCreate']:
    s=s.replace('XML_Parser XMLCALL\n'+name+'(', 'XML_Parser _Owned _Nullable XMLCALL\n'+name+'(')
s=s.replace('static XML_Parser\nparserCreate(','static XML_Parser _Owned _Nullable\nparserCreate(')
s=s.replace('static XML_Parser parserCreate(','static XML_Parser _Owned _Nullable parserCreate(')
s=s.replace('static XML_Parser\ngetRootParserOf(','static XML_Parser _Borrow\ngetRootParserOf(')
s=s.replace('static XML_Parser getRootParserOf(','static XML_Parser _Borrow getRootParserOf(')
# 6. other borrowed params
s=re.sub(r'\b(HASH_TABLE|STRING_POOL|HASH_TABLE_ITER) \*(\w+)(?=\s*[,)])', r'\1 *_Borrow \2', s)
s=re.sub(r'\bconst (HASH_TABLE|DTD) \*(\w+)(?=\s*[,)])', r'const \1 *_Borrow \2', s)
s=re.sub(r'\bDTD \*newDtd(?=\s*[,)])', 'DTD *_Borrow newDtd', s)
s=s.replace('dtdReset(DTD *p,','dtdReset(DTD *_Borrow p,')
s=re.sub(r'\bTAG_NAME \*tagNamePtr\b','TAG_NAME *_Borrow tagNamePtr', s)
s=re.sub(r'\bstruct sipkey \*key\b','struct sipkey *_Borrow key', s)
s=re.sub(r'\bunsigned int \*outLevelDiff\b','unsigned int *_Borrow _Nullable outLevelDiff', s)
s=re.sub(r'\bconst XML_Memory_Handling_Suite \*memsuite\b','const XML_Memory_Handling_Suite *_Borrow _Nullable memsuite', s)
# 7. NULL
s=re.sub(r'\bNULL\b','nullptr',s)
# 11. dtd locals
DTD_ERR={'doContent':'XML_ERROR_UNEXPECTED_STATE','storeAtts':'XML_ERROR_UNEXPECTED_STATE','doProlog':'XML_ERROR_UNEXPECTED_STATE','appendAttributeValue':'XML_ERROR_UNEXPECTED_STATE','storeEntityValue':'XML_ERROR_UNEXPECTED_STATE','setElementTypePrefix':'0','getAttributeId':'nullptr','getContext':'nullptr','setContext':'XML_FALSE','nextScaffoldPart':'-1','build_model':'nullptr','getElementType':'nullptr'}
def dtd_local(m):
    fn=[x for x in re.finditer(r'^([A-Za-z_]\w*)\(', s[:m.start()], flags=re.M)][-1].group(1)
    return ('DTD *dtdRaw = parser->m_dtd;\n  if (dtdRaw == nullptr)\n    return %s;\n  DTD *_Borrow dtd = _Unsafe(&_Mut *dtdRaw);' % DTD_ERR[fn])
s=re.sub(r'DTD \*const dtd = parser->m_dtd;( /\* save one level of indirection \*/)?', dtd_local, s)
# 8. address-of rewrites
for f in ['m_tempPool','m_temp2Pool']:
    s=s.replace('&parser->'+f,'&_Mut parser->'+f)
s=re.sub(r'&\((dtd|newDtd|p)->(pool|entityValuePool|generalEntities|elementTypes|attributeIds|prefixes|paramEntities)\)', r'&_Mut \1->\2', s)
s=re.sub(r'&(dtd|newDtd|p)->(pool|entityValuePool|generalEntities|elementTypes|attributeIds|prefixes|paramEntities)\b', r'&_Mut \1->\2', s)
s=re.sub(r'&\(oldDtd->(\w+)\)', r'&_Const oldDtd->\1', s)
s=re.sub(r'&\(?parser->m_dtd->(pool|entityValuePool)\)?', r'_Unsafe(&_Mut parser->m_dtd->\1)', s)
s=re.sub(r'&(dtd|newDtd|oldDtd|parser->m_dtd)->defaultPrefix\b', r'_Unsafe(&\1->defaultPrefix)', s)
s=re.sub(r'&\((\w+)->defaultAttForName\)', r'_Unsafe(&_Mut \1->defaultAttForName)', s)
s=re.sub(r'&\((parser|rootParser)->(m_\w+(?:->\w+|\.\w+)?)\)', r'_Unsafe(&\1->\2)', s)
s=re.sub(r'&(parser|rootParser)->(m_\w+(?:->\w+|\.\w+)?(?:\[[^\]]*\])?)', r'_Unsafe(&\1->\2)', s)
s=s.replace('_Unsafe(&parser->m_prologState)','&_Mut parser->m_prologState')
s=s.replace('&(tag->name)','_Unsafe(&_Mut tag->name)').replace('&(tag->bindings)','_Unsafe(&tag->bindings)')
s=re.sub(r'\bhashTableIterInit\(&iter,', 'hashTableIterInit(&_Mut iter,', s)
s=re.sub(r'\bhashTableIterNext\(&iter\)', 'hashTableIterNext(&_Mut iter)', s)
s=s.replace('storeAtts(parser, enc, s, &name, &bindings,','storeAtts(parser, enc, s, &_Mut name, _Unsafe(&bindings),')
s=re.sub(r'(?<![_\w])&(?!_Mut|_Const|&)(\w+)(?=\s*[,)])', r'_Unsafe(&\1)', s)  # remaining &local args
s=s.replace('HASH_TABLE_ITER iter;','HASH_TABLE_ITER iter = {nullptr, nullptr};')
# 9. unsafe calls
s=wrap_calls(s, r'\b(?:parser|rootParser)->m_\w+(?:Handler|Release|Convert)\(')
s=wrap_calls(s, r'\bparser->m_processor\(')
s=wrap_calls(s, r'\bcharDataHandler\(')
s=wrap_calls(s, r'\(parser->m_ns \? Xml\w+ : Xml\w+\)\(')
s=wrap_calls(s, r'\b(?:XmlGetInternalEncoding|XmlGetInternalEncodingNS|XmlSizeOfUnknownEncoding|XmlInitEncoding|XmlInitEncodingNS|XmlUtf8Encode|XmlUtf16Encode|XmlConvert|XmlEncode|memcpy|memmove|memset|memcmp|strncmp|wcsncmp|xcslen|fprintf|sip24_\w+|getpid|getenv|strtoul|gettimeofday|writeRandomBytes_\w+|assert|GetSystemTimeAsFileTime)\(')
# 10. minBytesPerChar
s=re.sub(r'(?<!_Unsafe\()\b(enc|newEncoding|parser->m_encoding)->minBytesPerChar\b', r'_Unsafe(\1->minBytesPerChar)', s)
# 13. pool macros
s=s.replace('#define poolLength(pool) ((pool)->ptr - (pool)->start)','#define poolLength(pool) (_Unsafe((pool)->ptr - (pool)->start))')
s=s.replace('#define poolChop(pool) ((void)--(pool->ptr))','#define poolChop(pool) (_Unsafe((void)--(pool->ptr)))')
s=s.replace('#define poolLastChar(pool) (((pool)->ptr)[-1])','#define poolLastChar(pool) (_Unsafe(((pool)->ptr)[-1]))')
s=s.replace('#  define MUST_CONVERT(enc, s) (! (enc)->isUtf8)','#  define MUST_CONVERT(enc, s) (_Unsafe(! (enc)->isUtf8))')
s=s.replace('#  define MUST_CONVERT(enc, s) (! (enc)->isUtf16)','#  define MUST_CONVERT(enc, s) (_Unsafe(! (enc)->isUtf16))')
# MALLOC/REALLOC/FREE: borrow the parser argument so raw and borrowed callers both work
s=s.replace('#  define MALLOC(parser, s) (parser->m_mem.malloc_fcn((s)))','#  define MALLOC(parser, s) (_Unsafe((parser)->m_mem.malloc_fcn((s))))')
s=s.replace('#  define REALLOC(parser, p, s) (parser->m_mem.realloc_fcn((p), (s)))','#  define REALLOC(parser, p, s) (_Unsafe((parser)->m_mem.realloc_fcn((p), (s))))')
s=s.replace('#  define FREE(parser, p) (parser->m_mem.free_fcn((p)))','#  define FREE(parser, p) (_Unsafe((parser)->m_mem.free_fcn((p))))')

# 4/5. _Safe on definitions and prototypes
lines=s.split('\n')
EXCLUDE={'callUnknownEncodingConvert','XML_GetFeatureList'}
def stmt_end(i):
    depth=0; j=i
    while j<len(lines):
        for c in lines[j]:
            if c=='(': depth+=1
            elif c==')': depth-=1
            elif depth==0 and c in ';{': return c
        j+=1
    return ';'
mark=set()
for i,l in enumerate(lines):
    m=re.match(r'^([A-Za-z_]\w*)\(', l)
    if m and m.group(1) not in EXCLUDE and stmt_end(i)=='{':
        k=i-1
        while k>=0:
            t=lines[k]
            if t.strip()=='' or t.rstrip().endswith((';','}','*/')) or t.lstrip().startswith(('//','/*')): break
            if not t.startswith('#'): mark.add(k)
            k-=1
    if re.match(r'^static .*\(', l) and stmt_end(i)==';':
        mark.add(i)
for k in mark:
    if not lines[k].startswith('_Safe '): lines[k]='_Safe '+lines[k]
s='\n'.join(lines)
open(p,'w').write(s)
print('safe marks',len(mark))
