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
    if rest.startswith(';') and re.match(r'[ \t]*(//.*|/\*.*\*/[ \t]*)?\n', rest[1:]) and re.match(r'(?!Xml|MUST_CONVERT)[A-Za-z_]\w*\(', body.lstrip()) and body.count('\n') <= 6:
        out.append(src[i:m.start()] + m.group(1) + '_Unsafe ' + body.strip() + ';')
        i = k + 2; n += 1
    else:
        out.append(src[i:m.end()]); i = m.end()
open(path, 'w').write(''.join(out))
print('post: statement forms', n)
