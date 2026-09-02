#!/usr/bin/env python3
"""Compiler-driven _Unsafe insertion: wrap the smallest statement/expression around each
safe-zone violation the compiler reported. Usage: passC.py <file.c> <clang.log>"""
import re, sys

WRAP_KINDS = (
    "operator used by raw pointer type is forbidden",
    "operator is forbidden in the safe zone",
    "_Unsafe function call is forbidden",
    "on a raw pointer is not supported in the safe zone",
    "access to a union field is _Unsafe",
    "is forbidden in the safe zone",
)
SKIP_SUBSTR = ("implicit conversion",)
OWNED_NAMES = {"m_atts", "m_attInfo", "m_nsAtts", "m_groupConnector", "m_dataBuf", "m_unknownEncodingMem", "m_protocolEncodingName", "v", "defaultAtts"}

path, log = sys.argv[1], sys.argv[2]
src = open(path).read()
n = len(src)
lines = src.split("\n")
line_off = [0]
for l in lines:
    line_off.append(line_off[-1] + len(l) + 1)

# code mask: False inside comments, string/char literals and preprocessor lines
code = [True] * n
i = 0
while i < n:
    c = src[i]
    if src.startswith("/*", i):
        j = src.index("*/", i) + 2
        for k in range(i, j): code[k] = False
        i = j
    elif src.startswith("//", i):
        j = src.index("\n", i)
        for k in range(i, j): code[k] = False
        i = j
    elif c == '"' or c == "'":
        j = i + 1
        while src[j] != c:
            if src[j] == "\\": j += 1
            j += 1
        for k in range(i, j + 1): code[k] = False
        i = j + 1
    elif c == "#" and src[:i].rstrip(" \t").endswith("\n") or (c == "#" and i == 0):
        j = src.index("\n", i)
        while src[j - 1] == "\\":
            j = src.index("\n", j + 1)
        for k in range(i, j): code[k] = False
        i = j
    else:
        i += 1

HOIST_RE = re.compile(r"cannot use '(.+)' because it was mutably borrowed")
hoists = []  # (line, col)
errs = []
for l in open(log):
    mh = re.match(r"^" + re.escape(path) + r":(\d+):(\d+): error: cannot use '.+' because it was mutably borrowed", l.rstrip())
    if mh:
        hoists.append((int(mh.group(1)), int(mh.group(2))))
        continue
    m = re.match(r"^" + re.escape(path) + r":(\d+):(\d+): error: (.*)$", l.rstrip())
    if not m:
        continue
    msg = m.group(3)
    if any(k in msg for k in SKIP_SUBSTR):
        continue
    mn = re.match(r"cannot dereference possibly-null status pointer '(\w+)'", msg)
    if mn and mn.group(1) not in OWNED_NAMES:
        errs.append((int(m.group(1)), int(m.group(2)), msg)); continue
    if any(k in msg for k in WRAP_KINDS):
        errs.append((int(m.group(1)), int(m.group(2)), msg))

def skip_ws(i):
    while i < n and (src[i].isspace() or not code[i]):
        i += 1
    return i

def fwd_end(i):
    depth = 0
    while i < n:
        if code[i]:
            c = src[i]
            if c in "([": depth += 1
            elif c in ")]": depth -= 1
            elif depth == 0 and c in ";{": return i
        i += 1
    return i

def match_paren(i):
    assert src[i] == "("
    depth = 0
    while True:
        if code[i]:
            c = src[i]
            if c == "(": depth += 1
            elif c == ")":
                depth -= 1
                if depth == 0: return i
        i += 1

import bisect
_starts = []
_sdepth = {}
_init_ranges = []
_d = 0
_stack = []
_prev = ""
for _k in range(n):
    if not code[_k]: continue
    _c = src[_k]
    if _c.isspace(): continue
    if _c in "([": _d += 1
    elif _c in ")]": _d -= 1
    elif _d == 0 and _c in ";{}":
        if _c == "{":
            if _prev in "=,{" and (_prev != "{" or (_stack and _stack[-1][0] == "init")):
                _stack.append(("init", _k))
            else:
                _stack.append(("block", _k))
        elif _c == "}":
            if _stack:
                kind, at = _stack.pop()
                if kind == "init": _init_ranges.append((at, _k))
        if not (_stack and _stack[-1][0] == "init"):
            _starts.append(_k + 1); _sdepth[_k + 1] = len([x for x in _stack if x[0] == "block"])
    _prev = _c

def in_initializer(off):
    return any(a <= off <= b for (a, b) in _init_ranges)

def stmt_start(off):
    j = bisect.bisect_right(_starts, off) - 1
    if j < 0 or _sdepth[_starts[j]] == 0 or in_initializer(off):
        raise ValueError("file scope or initializer")
    i = skip_ws(_starts[j] if j >= 0 else 0)
    while True:
        m = re.match(r"(case\b[^:\n]*:|default\s*:|[A-Za-z_]\w*\s*:(?!:))\s*", src[i:])
        if m:
            i = skip_ws(i + m.end())
        else:
            break
    return i

def match_brace(i):
    assert src[i] == "{"
    depth = 0
    while True:
        if code[i]:
            c = src[i]
            if c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0: return i
        i += 1

def full_stmt_end(i):
    """End (exclusive) of the statement starting after a for/if header at i."""
    i = skip_ws(i)
    if src[i] == "{":
        return match_brace(i) + 1
    return fwd_end(i) + 1

def stmt_extent(i):
    """End (exclusive) of the full statement starting at i, including if/else chains."""
    i = skip_ws(i)
    m = re.match(r"(else\s+if|if|while|for|switch|do|else)\b", src[i:])
    kw = m.group(1) if m else None
    if kw in ("if", "else if", "while", "for", "switch"):
        p = src.index("(", i + len(kw))
        q = match_paren(p)
        e = stmt_extent(q + 1)
        if kw in ("if", "else if"):
            j = skip_ws(e)
            if src.startswith("else", j) and not (src[j + 4].isalnum() or src[j + 4] == "_"):
                return stmt_extent(j + 4)
        return e
    if kw == "do":
        e = stmt_extent(i + 2)
        j = src.index("while", e)
        return fwd_end(j) + 1
    if kw == "else":
        return stmt_extent(i + 4)
    if src[i] == "{":
        return match_brace(i) + 1
    if src.startswith("_Unsafe", i):
        j = skip_ws(i + 7)
        if src[j] == "{":
            return match_brace(j) + 1
    return fwd_end(i) + 1

def top_level_eq(a, b):
    depth = 0
    for k in range(a, b):
        if not code[k]: continue
        c = src[k]
        if c in "([": depth += 1
        elif c in ")]": depth -= 1
        elif depth == 0 and c == "=" and src[k + 1] != "=" and src[k - 1] not in "=!<>+-*/%&|^":
            return k
    return None

DECL_RE = r"\s*(const\s+|static\s+|unsigned\s+|struct\s+|enum\s+|volatile\s+)*[A-Za-z_]\w*(\s*\*+\s*|\s+)(const\s+)?[A-Za-z_]\w*(\[[^\]]*\])?\s*=$"

def classify(start, off):
    while True:
        m = re.match(r"(else\s+if|if|while|switch|for|return|else|do)\b", src[start:])
        kw = m.group(1) if m else None
        if kw in ("if", "else if", "while", "switch", "for"):
            p = src.index("(", start + len(kw))
            q = match_paren(p)
            if p < off < q:
                if kw == "for":
                    return ("stmt", start, full_stmt_end(q + 1))
                if False:
                    parts = []; depth = 0; s0 = p + 1
                    for k in range(p + 1, q):
                        if not code[k]: continue
                        c = src[k]
                        if c in "([": depth += 1
                        elif c in ")]": depth -= 1
                        elif c == ";" and depth == 0:
                            parts.append((s0, k)); s0 = k + 1
                    parts.append((s0, q))
                    for (a, b) in parts:
                        if a <= off <= b:
                            eq = top_level_eq(a, b)
                            if eq is not None and re.match(DECL_RE, src[a:eq + 1]):
                                return ("expr", skip_ws(eq + 1), b)
                            return ("expr", skip_ws(a), b)
                    return ("expr", p + 1, q)
                return ("expr", p + 1, q)
            start = skip_ws(q + 1)
            continue
        if kw == "else" or kw == "do":
            start = skip_ws(start + len(kw))
            continue
        if kw == "return":
            e = fwd_end(start)
            return ("expr", skip_ws(start + 6), e)
        if src[start] == "{":
            start = skip_ws(start + 1)
            continue
        e = fwd_end(start)
        eq = top_level_eq(start, e)
        if eq is not None and off > eq and re.match(DECL_RE, src[start:eq + 1]):
            return ("expr", skip_ws(eq + 1), e)
        return ("stmt", start, e)

regions = {}
casts = {}
manual = []
for (ln, col, msg) in errs:
    off = line_off[ln - 1] + col - 1
    if lines[ln - 1].lstrip().startswith("#"):
        manual.append((ln, col, msg, "macro definition")); continue
    try:
        start = stmt_start(off)
        kind, a, b = classify(start, off)
    except Exception as ex:  # noqa
        manual.append((ln, col, msg, "parse failure %r" % ex)); continue
    text = src[a:b]
    if text.lstrip().startswith("_Unsafe") or a > off or b < off:
        mc = re.match(r"conversion from type 'void \*(?:const)?' to '(.+?)'", msg)
        if mc and text.lstrip().startswith("_Unsafe("):
            t = re.sub(r"\s*const$", "", mc.group(1).split(" (aka")[0])
            casts[a + text.index("_Unsafe(") + 8] = "(" + t + ")"
            continue
        manual.append((ln, col, msg, "already _Unsafe: " + text.strip()[:70])); continue
    regions[(a, b)] = kind

# ---- hoist field reads of a borrowed variable out of the call arguments
CHAIN_RE = re.compile(r"[A-Za-z_]\w*(?:(?:->|\.)\w+|\[[^\]]*\])*")
hoist_edits = {}  # stmt_start -> list of (a, b, temp)
hcount = max([int(x) for x in re.findall(r"bsc_h(\d+)", src)] + [-1]) + 1
def hoist_site(st, off):
    """Descend from a statement start to the statement that must receive the temporaries.
    Returns (start, needs_braces)."""
    wrap = False
    while True:
        m = re.match(r"(else\s+if|if|while|switch|for|else|do)\b", src[st:])
        kw = m.group(1) if m else None
        if kw == "else":
            st = skip_ws(st + 4); wrap = True; continue
        if kw == "do":
            st = skip_ws(st + 2); wrap = True; continue
        if kw in ("if", "else if", "while", "switch", "for"):
            if kw == "else if":
                st = skip_ws(st + 4); wrap = True; continue
            p = src.index("(", st + len(kw))
            q = match_paren(p)
            if p < off < q:
                return st, wrap
            st = skip_ws(q + 1); wrap = True; continue
        if src[st] == "{":
            st = skip_ws(st + 1); wrap = False; continue
        return st, wrap

for (ln, col) in hoists:
    off = line_off[ln - 1] + col - 1
    a = off
    while a > 0 and (src[a - 1].isalnum() or src[a - 1] in "_>-.]"):
        a -= 1
        if src[a] == "]":
            while src[a] != "[": a -= 1
    m = CHAIN_RE.match(src, a)
    if not m:
        manual.append((ln, col, "hoist", "no chain")); continue
    b = m.end()
    pre = src[:a].rstrip()
    if pre.endswith("_Unsafe(&"):
        a = len(pre) - 9
        if src[b] == ")": b += 1
    elif pre.endswith("&") and not pre.endswith("&&"):
        a = len(pre) - 1
    try:
        st0 = stmt_start(off)
        st, wrap = hoist_site(st0, off)
    except ValueError as ex:
        manual.append((ln, col, "hoist", str(ex))); continue
    prevcode = src[:st].rstrip()
    while prevcode and not code[len(prevcode) - 1]:
        prevcode = prevcode[:len(prevcode) - 1].rstrip()
    if prevcode.endswith(":") and not prevcode.endswith("::"):
        wrap = True
    hoist_edits.setdefault(st, [False, []])
    hoist_edits[st][0] = hoist_edits[st][0] or wrap
    hoist_edits[st][1].append((a, b))

for st, (after_label, items) in hoist_edits.items():
    items = sorted(set(items))
    e = fwd_end(st)
    is_decl = re.match(DECL_RE, src[st:top_level_eq(st, e) + 1] if top_level_eq(st, e) else "") is not None
    if after_label and is_decl:
        manual.append((st, 0, "hoist", "declaration after label")); continue
    indent = re.match(r"[ \t]*", lines[src[:st].count("\n")]).group(0)
    decls = ""
    repl = []
    for (a, b) in items:
        name = "bsc_h%d" % hcount; hcount += 1
        decls += "__auto_type " + name + " = " + src[a:b] + ";\n" + indent
        repl.append((a, b, name))
    regions[("hoist", st)] = (decls, repl, after_label)

keys = sorted([r for r in regions if r[0] != "hoist"], key=lambda r: (r[0], -r[1]))
kept = []
for r in keys:
    if kept and kept[-1][0] <= r[0] and r[1] <= kept[-1][1]:
        continue
    kept.append(r)

out = src
edits = [(a, b, regions[(a, b)]) for (a, b) in kept] + [(a, a, "cast") for a in casts]
for r in regions:
    if r[0] == "hoist":
        decls, repl, after_label = regions[r]
        for (a, b, name) in repl:
            edits.append((a, b, "repl:" + name))
        edits.append((r[1], r[1], "decl:" + ("{ " if after_label else "") + decls))
        if after_label:
            ext = stmt_extent(r[1])
            edits.append((ext, ext, "close"))
for (a, b, kind) in sorted(edits, key=lambda e: (e[0], e[1]), reverse=True):
    body = out[a:b]
    if kind.startswith("repl:"):
        out = out[:a] + kind[5:] + out[b:]
    elif kind.startswith("decl:"):
        out = out[:a] + kind[5:] + out[a:]
    elif kind == "close":
        out = out[:a] + " }" + out[a:]
    elif kind == "cast":
        out = out[:a] + casts[a] + out[a:]
    elif kind == "expr":
        out = out[:a] + "_Unsafe((" + body.rstrip() + "))" + body[len(body.rstrip()):] + out[b:]
    elif body.lstrip().startswith("("):
        out = out[:a] + "_Unsafe { " + body + out[b] + " }" + out[b + 1:]
    else:
        out = out[:a] + "_Unsafe " + body + out[b:]
open(path, "w").write(out)
print("wrapped regions:", len(kept), "casts:", len(casts), "hoists:", hcount, "errors handled:", len(errs) - len(manual), "manual:", len(manual))
for m in manual[:40]:
    print("MANUAL %s:%s: %s -- %s" % m)
