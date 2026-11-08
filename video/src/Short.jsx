// A Short: 1080 x 1920, a slice of the take (from .. from + dur seconds) with its own music: the screen big at the top, the card under it,
// the panel with its cue rings at the foot. Made from the same trace as the long video.
import React, { useLayoutEffect, useRef } from "react";
import { AbsoluteFill, Audio, interpolate, spring, staticFile, useCurrentFrame, useVideoConfig } from "remotion";
import { loadFont } from "@remotion/google-fonts/BarlowSemiCondensed";
import { useTrace, drawFb, target, Chip, ORANGE, INK, DIM, BG } from "./EmuScene.jsx";
import { Panel, SCREEN } from "./Panel.jsx";

const { fontFamily } = loadFont("normal", { weights: ["400", "500", "600", "700"] });

export const SHORTS = [
  { id: "ShortTheory", dir: "update0170", from: 21.0, dur: 25, hook: "The FM-1 knows music theory" },
  { id: "ShortFills", dir: "update0170", from: 45.5, dur: 38.5, hook: "Progressions on the black keys" },
  { id: "ShortBass", dir: "update0170", from: 86.9, dur: 35, hook: "A bass line that follows chords" },
];

export function Short({ dir, from, hook }) {
  const frame = useCurrentFrame(), { fps, durationInFrames } = useVideoConfig();
  const t = from + frame / fps, f0 = Math.round(from * fps);
  const fade = interpolate(frame, [0, 8, durationInFrames - 12, durationInFrames], [0, 1, 1, 0], { extrapolateLeft: "clamp", extrapolateRight: "clamp" });
  const d = useTrace(dir);
  const big = useRef(null), small = useRef(null);
  const n = d ? Math.min(f0 + frame, d.tr.frames.length - 1) : 0;
  useLayoutEffect(() => { if (d) { drawFb(big.current, d.fb, d.lastFb[n]); drawFb(small.current, d.fb, d.lastFb[n]); } });
  const audio = <Audio src={staticFile(`${dir}/audio.wav`)} startFrom={f0}
    volume={(x) => interpolate(x, [0, 8, durationInFrames - 12, durationInFrames], [0, 1, 1, 0], { extrapolateLeft: "clamp", extrapolateRight: "clamp" })} />;
  if (!d) return <AbsoluteFill style={{ background: BG }}>{audio}</AbsoluteFill>;
  const f = d.tr.frames[n], ev = d.tr.events;
  const text = [...ev].reverse().find((e) => e.kind === "text" && e.t <= t) || { title: "", body: "", chips: [] };
  const st = spring({ frame: Math.round((t - (text.t ?? 0)) * fps), fps, config: { damping: 18, stiffness: 120 } });
  const rings = ev.filter((e) => e.kind === "ring" && t >= e.t - 0.05 && t < e.t + (e.dur ?? 1));
  const readout = [...ev].reverse().find((e) => e.kind === "readout" && e.t <= t && t < e.t + (e.dur ?? 3));
  const pulse = 0.65 + 0.35 * Math.sin(t * 9), lit = new Set(rings.map((r) => r.chip).filter(Boolean));
  const hookO = interpolate(frame, [0, 10], [0, 1], { extrapolateRight: "clamp" });
  return (
    <AbsoluteFill style={{ background: `radial-gradient(ellipse at 50% 20%, #26272e, ${BG} 70%)`, fontFamily, color: INK, opacity: fade }}>
      {audio}
      <div style={{ position: "absolute", left: 60, right: 60, top: 70, opacity: hookO }}>
        <div style={{ fontSize: 30, letterSpacing: ".2em", color: ORANGE, fontWeight: 700 }}>PERF-1 0.17.0 · FREE FIRMWARE FOR THE M-VAVE FM-1</div>
        <div style={{ fontSize: 64, fontWeight: 700, lineHeight: 1.08, marginTop: 10 }}>{hook}</div>
      </div>
      <div style={{ position: "absolute", left: 90, top: 270, width: 900, height: 900, borderRadius: 36, background: "#0b0b0d", border: "3px solid #3a3d42" }}>
        <canvas ref={big} width={240} height={240} style={{ position: "absolute", left: 35, top: 35, width: 830, height: 830, imageRendering: "pixelated", borderRadius: 6 }} />
      </div>
      {readout && (
        <div style={{ position: "absolute", left: 50, width: 980, top: 1180, textAlign: "center" }}>
          <span style={{ display: "inline-block", padding: "6px 26px", borderRadius: 16, background: "#0b0b0dee", fontSize: 54, fontWeight: 700, letterSpacing: ".05em", color: ORANGE }}>{readout.text}</span>
        </div>
      )}
      <div style={{ position: "absolute", left: 60, right: 60, top: 1272, opacity: Math.min(1, st * 1.4), transform: `translateY(${(1 - st) * 20}px)` }}>
        <div style={{ fontSize: 26, letterSpacing: ".16em", fontWeight: 600, color: ORANGE }}>{text.label}</div>
        <div style={{ fontSize: 58, fontWeight: 700, lineHeight: 1.1, marginTop: 4 }}>{text.title}</div>
        <div style={{ margin: "14px -8px 0", fontSize: 28, whiteSpace: "nowrap" }}>
          {(text.chips || []).map((c, i) => c.startsWith("*") ? <Chip key={i} on={lit.has(c.slice(1))}>{c.slice(1)}</Chip> : <span key={i} style={{ color: DIM, margin: "0 8px" }}>{c}</span>)}
        </div>
      </div>
      <div style={{ position: "absolute", left: 210, top: 1462, width: 660, height: 423, borderRadius: 22, overflow: "hidden", background: "#16171b", border: "2px solid #2f3138" }}>
        <div style={{ position: "absolute", left: 0, top: 0, width: 1000, height: 640, transformOrigin: "0 0", transform: "scale(0.66)" }}>
          <Panel frame={f} angles={d.ang[n]}>
            {rings.flatMap((r, j) => target(r.target).map((o, i) => <rect key={`${j}-${i}`} x={o.x - 5} y={o.y - 5} width={o.w + 10} height={o.h + 10} rx={o.r} fill="none" stroke={ORANGE} strokeWidth={5} opacity={pulse} />))}
          </Panel>
          <canvas ref={small} width={240} height={240} style={{ position: "absolute", left: SCREEN.x, top: SCREEN.y, width: SCREEN.w, height: SCREEN.h, imageRendering: "pixelated" }} />
        </div>
      </div>
    </AbsoluteFill>
  );
}
