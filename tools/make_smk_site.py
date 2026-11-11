#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""PERF-1 SMK's test site (GitHub Pages or any static host), from an SMK release build:

  python3 tools/build.py --board smk37 --release X.Y.Z      (build/perf1-smk-X.Y.Z.fwsc)
  python3 tools/make_smk_site.py X.Y.Z [OUTDIR] [--repo URL]

OUTDIR (default build/smk-site) gets index.html (web/smk_installer.html with fm1pkg.js, fm1ota.js and fm1idle.js
inlined), firmware/perf1-smk-X.Y.Z.fwsc, controls/index.html (docs/SMK37.md) and .nojekyll. Every link is relative,
so the same folder works on https://USER.github.io/REPO/ and on a local server (python -m http.server)."""
import html
import json
import re
import shutil
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SRC / "web"))
from make_site import product_of, strip_module  # noqa: E402


def md_inline(s):
    s = html.escape(s, quote=False)
    s = re.sub(r"`([^`]+)`", r"<code>\1</code>", s)
    s = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", s)
    s = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r'<a href="\2">\1</a>', s)
    return s


def md_to_html(md):
    """the few Markdown forms the SMK docs use: headings, paragraphs, lists, tables, code blocks"""
    out, para, lines, i = [], [], md.replace("\r\n", "\n").split("\n"), 0

    def flush():
        if para:
            out.append("<p>" + md_inline(" ".join(para)) + "</p>")
            para.clear()
    while i < len(lines):
        ln = lines[i]
        if ln.startswith("```"):
            flush()
            j = i + 1
            while j < len(lines) and not lines[j].startswith("```"):
                j += 1
            out.append("<pre>" + html.escape("\n".join(lines[i + 1:j])) + "</pre>")
            i = j + 1
            continue
        m = re.match(r"(#{1,4}) (.*)", ln)
        if m:
            flush()
            n = len(m.group(1))
            out.append(f"<h{n}>{md_inline(m.group(2))}</h{n}>")
        elif ln.startswith("|"):
            flush()
            rows = []
            while i < len(lines) and lines[i].startswith("|"):
                rows.append([c.strip() for c in lines[i].strip().strip("|").split("|")])
                i += 1
            head, body = rows[0], [r for r in rows[1:] if not all(re.fullmatch(r":?-+:?", c) for c in r)]
            t = "<div class=tbl><table><tr>" + "".join(f"<th>{md_inline(c)}</th>" for c in head) + "</tr>"
            t += "".join("<tr>" + "".join(f"<td>{md_inline(c)}</td>" for c in r) + "</tr>" for r in body)
            out.append(t + "</table></div>")
            continue
        elif re.match(r"\s*[-*] ", ln):
            flush()
            items = []
            while i < len(lines) and re.match(r"\s*[-*] ", lines[i]):
                items.append(re.sub(r"\s*[-*] ", "", lines[i], count=1))
                i += 1
            out.append("<ul>" + "".join(f"<li>{md_inline(x)}</li>" for x in items) + "</ul>")
            continue
        elif not ln.strip():
            flush()
        else:
            para.append(ln.strip())
        i += 1
    flush()
    return "\n".join(out)


PAGE = """<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>PERF-1 SMK controls (BETA test build)</title>
<style>
:root {{ --ground: #130a0e; --card: #211219; --ink: #f6ece0; --muted: #bba79a; --rule: #4a2a35; --red: #e0344f; --pink: #ff8f8f; color-scheme: dark; }}
body {{ margin: 0; background: var(--ground); color: var(--ink); font: 400 16px/1.6 system-ui, sans-serif; }}
main {{ max-width: 860px; margin: 0 auto; padding: 24px 16px 56px; }}
h1 {{ font-size: 1.6rem; }} h2 {{ color: var(--pink); font-size: 1.1rem; text-transform: uppercase; letter-spacing: .05em;
  border-top: 1px solid var(--rule); padding-top: 1rem; margin-top: 2rem; }}
a {{ color: var(--ink); text-decoration-color: var(--red); }}
.tbl {{ overflow-x: auto; margin: 0 0 14px; }} table {{ border-collapse: collapse; font-size: 15px; }}
th, td {{ border-bottom: 1px solid var(--rule); padding: 5px 9px; text-align: left; vertical-align: top; }}
th {{ color: var(--muted); font-size: 13px; }}
code {{ font: 14px ui-monospace, Consolas, monospace; background: var(--card); padding: 0 4px; border-radius: 3px; }}
pre {{ background: var(--card); padding: 10px; overflow-x: auto; }}
.beta {{ background: repeating-linear-gradient(-45deg, #ffd447 0 14px, #1a0c03 14px 28px); padding: 6px 0; }}
.beta p {{ margin: 0 auto; max-width: 860px; background: #1a0c03; color: #ffd447; font-weight: 700; letter-spacing: .06em;
  text-align: center; padding: 6px 12px; text-transform: uppercase; font-size: 15px; }}
</style></head><body><div class="beta"><p>Beta test build &middot; not a release &middot; controls may change</p></div><main>
<p><a href="../">&larr; Install</a></p>
{body}
</main></body></html>
"""


def main():
    args = [a for a in sys.argv[1:]]
    repo = "https://github.com/deamonSLINKY/PERF-1-SMK"
    if "--repo" in args:
        k = args.index("--repo")
        repo = args[k + 1]
        del args[k:k + 2]
    if not args:
        sys.exit(__doc__)
    ver = args[0]
    out = Path(args[1]) if len(args) > 1 else SRC / "build" / "smk-site"
    pkg = SRC / "build" / f"perf1-smk-{ver}.fwsc"
    raw = pkg.read_bytes()
    product = product_of(raw)
    if not re.fullmatch(r"SMK-37 Pro_7\d{6}", product):
        sys.exit(f"{pkg}: identity {product!r}: make an SMK release build (tools/build.py --board smk37 --release {ver})")
    if b"FELUCCA-LOADER-1" not in raw:
        sys.exit(f"{pkg}: no update loader in it")
    page = (SRC / "web" / "smk_installer.html").read_text(encoding="utf-8")
    for mark in ("/*LIB*/", "/*META*/"):
        if page.count(mark) != 1:
            sys.exit(f"web/smk_installer.html must contain {mark} once")
    lib = "\n".join(strip_module((SRC / "web" / f).read_text(encoding="utf-8")) for f in ("fm1pkg.js", "fm1ota.js", "fm1idle.js"))
    name = f"perf1-smk-{ver}.fwsc"
    meta = json.dumps({"version": ver, "product": product, "pkg": "firmware/" + name, "repo": repo})
    page = page.replace("/*LIB*/", lib).replace("/*META*/", meta)
    if out.exists():
        shutil.rmtree(out)
    (out / "firmware").mkdir(parents=True)
    (out / "controls").mkdir()
    (out / "index.html").write_text(page, encoding="utf-8", newline="\n")
    shutil.copy(pkg, out / "firmware" / name)
    md = (SRC / "docs" / "SMK37.md").read_text(encoding="utf-8")
    (out / "controls" / "index.html").write_text(PAGE.format(body=md_to_html(md)), encoding="utf-8", newline="\n")
    (out / ".nojekyll").write_text("", encoding="utf-8")
    print(f"site     {out}  PERF-1 SMK {ver} ({product}), {len(raw)} B package")


if __name__ == "__main__":
    main()
