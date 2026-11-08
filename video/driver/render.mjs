// node driver/render.mjs <out.mp4> <public folder>... [-- extra remotion args]
// Renders the Full composition without filling C:. Remotion copies the whole public dir into Temp before a render, and public/ holds
// every old video's traces (5+ GB), so: a public dir of hard links to only the folders named (no copy, same drive), and Temp on G:
// under video/tmp. Both are emptied before each render. Example:
//   node driver/render.mjs out/perf1_update0170.mp4 update0170 web
import { spawnSync } from "node:child_process";
import { mkdirSync, rmSync, readdirSync, statSync, linkSync } from "node:fs";
import { join, dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const root = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const argv = process.argv.slice(2), cut = argv.indexOf("--");
const [out, ...dirs] = cut < 0 ? argv : argv.slice(0, cut);
const extra = cut < 0 ? [] : argv.slice(cut + 1);
if (!out || !dirs.length) {
  console.error("usage: node driver/render.mjs <out.mp4> <public folder>... [-- remotion args]");
  process.exit(2);
}
const pub = join(root, "tmp", "public"), temp = join(root, "tmp", "temp");
rmSync(pub, { recursive: true, force: true });
rmSync(temp, { recursive: true, force: true });
mkdirSync(temp, { recursive: true });

function link(src, dst) {                       // hard-link a tree: no bytes copied
  mkdirSync(dst, { recursive: true });
  for (const n of readdirSync(src)) {
    const s = join(src, n), d = join(dst, n);
    if (statSync(s).isDirectory()) link(s, d);
    else linkSync(s, d);
  }
}
for (const d of dirs) link(join(root, "public", d), join(pub, d));

const env = { ...process.env, TEMP: temp, TMP: temp, TMPDIR: temp };
const npx = process.platform === "win32" ? "npx.cmd" : "npx";
const r = spawnSync(npx, ["remotion", "render", "src/index.jsx", process.env.COMP || "Full", out, "--overwrite", "--concurrency=1", `--public-dir=${pub}`, ...extra],
  { cwd: root, env, stdio: "inherit", shell: process.platform === "win32" });
rmSync(temp, { recursive: true, force: true });
process.exit(r.status ?? 1);
