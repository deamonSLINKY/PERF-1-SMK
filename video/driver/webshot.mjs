// node driver/webshot.mjs <url> <out.png> [setup.js] [waitMs] [width] [height]
// A screenshot of a web page in headless Edge (Chrome DevTools protocol), for the video's web stills. setup.js (optional) runs in the page
// once it has loaded (an async function body; it may return a number: the scroll position to shoot from); a localStorage it sets survives
// a reload when it returns "reload".
import { spawn, spawnSync } from "node:child_process";
import { readFileSync, writeFileSync, mkdtempSync, mkdirSync, rmSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { join, dirname } from "node:path";

const [url, out, setup, waitMs = "6000", W = "1600", H = "900"] = process.argv.slice(2);
const EDGE = "C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe", PORT = 9333;
const TMP = join(dirname(fileURLToPath(import.meta.url)), "..", "tmp");   /* on G:, not C:'s Temp (each Edge profile is ~225 MB) */
mkdirSync(TMP, { recursive: true });
const prof = mkdtempSync(join(TMP, "perf1shot-"));
const edge = spawn(EDGE, ["--headless=new", `--remote-debugging-port=${PORT}`, `--user-data-dir=${prof}`, `--window-size=${W},${H}`, "--hide-scrollbars",
  "--no-first-run", "--no-default-browser-check", "--disable-extensions", "about:blank"], { stdio: "ignore" });
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
let tabs = null;
for (let i = 0; i < 50 && !tabs; i++) { await sleep(200); try { tabs = await (await fetch(`http://127.0.0.1:${PORT}/json/list`)).json(); } catch (_) { /* not up yet */ } }
const page = tabs.find((t) => t.type === "page");
const ws = new WebSocket(page.webSocketDebuggerUrl);
await new Promise((r) => ws.addEventListener("open", r));
let id = 0; const wait = new Map();
ws.addEventListener("message", (e) => { const m = JSON.parse(e.data); if (m.id && wait.has(m.id)) { wait.get(m.id)(m); wait.delete(m.id); } });
const cmd = (method, params = {}) => new Promise((r) => { const n = ++id; wait.set(n, r); ws.send(JSON.stringify({ id: n, method, params })); });
const evalJs = async (src) => (await cmd("Runtime.evaluate", { expression: src, awaitPromise: true, returnByValue: true })).result?.result?.value;

await cmd("Emulation.setDeviceMetricsOverride", { width: +W, height: +H, deviceScaleFactor: 1, mobile: false });
await cmd("Page.enable");
await cmd("Page.navigate", { url });
await sleep(+waitMs);
if (setup) {
  const body = readFileSync(setup, "utf8");
  let r = await evalJs(`(async () => { ${body} })()`);
  if (r === "reload") { await cmd("Page.reload"); await sleep(+waitMs); r = await evalJs(`(async () => { ${body.replace(/return "reload";?/, "")} })()`); }
  await sleep(1200);
}
const shot = await cmd("Page.captureScreenshot", { format: "png" });
writeFileSync(out, Buffer.from(shot.result.data, "base64"));
console.log("saved", out);
ws.close();
/* Quit the whole browser: edge.kill() (and taskkill on its pid) only stopped the launcher and left 17-36 Edge processes running,
 * holding the profile open. Browser.close on the browser's own socket, then anything still using this profile is stopped. */
try {
  const bw = new WebSocket((await (await fetch(`http://127.0.0.1:${PORT}/json/version`)).json()).webSocketDebuggerUrl);
  await new Promise((r) => bw.addEventListener("open", r));
  bw.send(JSON.stringify({ id: 1, method: "Browser.close" }));
  await sleep(2000);
} catch (_) { /* already gone */ }
if (process.platform === "win32")
  spawnSync("powershell.exe", ["-NoProfile", "-Command", `Get-CimInstance Win32_Process -Filter "Name='msedge.exe'" | Where-Object { $_.CommandLine -like '*${prof.split(/[\\/]/).pop()}*' } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }`], { stdio: "ignore" });
else edge.kill();
await sleep(1000);
try { rmSync(prof, { recursive: true, force: true, maxRetries: 5, retryDelay: 500 }); } catch (e) { console.warn("left", prof, e.code); }
