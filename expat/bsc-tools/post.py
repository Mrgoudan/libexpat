"""Cosmetic post-pass: statement-level `_Unsafe(call(...));` becomes `_Unsafe call(...);`."""
import re, sys
path = sys.argv[1]
src = open(path).read()
out = []; i = 0; n = 0
rx = re.compile(r'(?m)^([ \t]*)_Unsafe\(')
while True:
    m = rx.search(src, i)
    if not m:
        out.append(src[i:]); break
    p = m.end() - 1  # the '('
    depth = 0; k = p
    while True:
        c = src[k]
        if c == '(': depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0: break
        elif c in '"\'':
            q = c; k += 1
            while src[k] != q:
                if src[k] == '\\': k += 1
                k += 1
        k += 1
    body = src[p + 1:k]
    rest = src[k + 1:]
    is_stmt = rest.startswith(';') and re.match(r'[ \t]*(//.*|/\*.*\*/[ \t]*)?\n', rest[1:]) and body.count('\n') <= 12
    callee = re.match(r'[A-Za-z_]\w*(?=\()', body.lstrip())
    macro_wrapped = callee and (callee.group(0) in ('XmlConvert', 'XmlUpdatePosition', 'XmlUtf8Convert', 'XmlUtf16Convert', 'MUST_CONVERT') or callee.group(0).endswith('Tok'))
    if is_stmt and callee and macro_wrapped:
        out.append(src[i:m.start()] + m.group(1) + '(void)_Unsafe(' + body + ');')
        i = k + 2; n += 1
    elif is_stmt and callee and not callee.group(0).startswith('Xml'):
        out.append(src[i:m.start()] + m.group(1) + '_Unsafe ' + body.strip() + ';')
        i = k + 2; n += 1
    elif is_stmt and callee and callee.group(0) in ('XmlInitEncoding', 'XmlInitEncodingNS'):
        out.append(src[i:m.start()] + m.group(1) + '_Unsafe ' + body.strip() + ';')
        i = k + 2; n += 1
    else:
        out.append(src[i:m.end()]); i = m.end()
open(path, 'w').write(''.join(out))
print('post: statement forms', n)

# collapse the generator's double parentheses: _Unsafe((body)) -> _Unsafe(body)
src = open(path).read()
out = []; i = 0; m2 = 0
while True:
    j = src.find('_Unsafe((', i)
    if j < 0:
        out.append(src[i:]); break
    p = j + 7  # outer '('
    depth = 0; k = p
    while True:
        c = src[k]
        if c == '(': depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0: break
        elif c in '"\'':
            q = c; k += 1
            while src[k] != q:
                if src[k] == '\\': k += 1
                k += 1
        k += 1
    inner = src[p + 1:k]           # starts with '(' and ends with ')' if the inner pair wraps everything
    depth = 0; wraps = inner.startswith('(')
    if wraps:
        for idx, c in enumerate(inner):
            if c == '(': depth += 1
            elif c == ')':
                depth -= 1
                if depth == 0 and idx != len(inner) - 1:
                    wraps = False; break
    if wraps:
        out.append(src[i:j] + '_Unsafe(' + inner[1:-1] + ')'); m2 += 1
    else:
        out.append(src[i:p + 1])
    i = k + 1 if wraps else p + 1
open(path, 'w').write(''.join(out))
print('post: double parens collapsed', m2)
