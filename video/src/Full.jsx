// The whole video: a cold open (a few seconds of the best moment, with its music), the title over that music fading out, then the tour
// (one trace: one take of the device).
import React from "react";
import { Audio, Sequence, Series, interpolate, staticFile } from "remotion";
import { EmuScene } from "./EmuScene.jsx";
import { Title } from "./Cards.jsx";

export const TITLE_FRAMES = 135;
export const TOUR = "update0170";   // the trace this video renders (public/<dir>): the 0.17.0 update; "update0150" the 0.15.0 one, "tour" the 0.14.0 tour
export const EDITION = "THE 0.17.0 UPDATE";
export const COLD_FROM = 31.5 * 30;  // the cold open: the take from 0:31.5 (G7 turns red, V OF THE NEXT, into C7) ...
export const COLD_FRAMES = 6 * 30;   // ... for 6 s; 0 for none

export function Full({ frames }) {
  const lead = COLD_FRAMES + TITLE_FRAMES;
  return (
    <>
      {COLD_FRAMES > 0 && (                // the cold open's music, carried on under the title and faded out before the boot
        <Sequence durationInFrames={lead}>
          <Audio src={staticFile(`${TOUR}/audio.wav`)} startFrom={COLD_FROM}
            volume={(f) => interpolate(f, [0, 6, COLD_FRAMES + 30, lead - 6], [0, 1, 1, 0], { extrapolateLeft: "clamp", extrapolateRight: "clamp" })} />
        </Sequence>
      )}
      <Series>
        {COLD_FRAMES > 0 && (
          <Series.Sequence durationInFrames={COLD_FRAMES}>
            <Sequence from={-COLD_FROM} durationInFrames={COLD_FROM + COLD_FRAMES}><EmuScene dir={TOUR} mute /></Sequence>
          </Series.Sequence>
        )}
        <Series.Sequence durationInFrames={TITLE_FRAMES}><Title edition={EDITION} /></Series.Sequence>
        <Series.Sequence durationInFrames={frames}><EmuScene dir={TOUR} /></Series.Sequence>
      </Series>
    </>
  );
}
