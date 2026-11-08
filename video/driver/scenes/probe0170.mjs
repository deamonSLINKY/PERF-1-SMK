// a look before the 0.17.0 video: DISCO 03 resumed at power-on, THEORY for two minutes (not part of the video)
import { Emu } from "../emu.mjs";

export async function prepare0170(wasm) {
  const e = await Emu.create({ wasmPath: wasm, seed: 0 });
  await e.wait(3600);
  e.press("GLO"); await e.wait(400); await e.turn("PRESETS", 10, 300); await e.wait(400); e.release("GLO"); await e.wait(800);   // DISCO
  await e.turn("PRESETS", 2, 300); await e.wait(1500);                                                                          // 03
  await e.tap("HOME", 60); await e.wait(120); await e.tap("HOME", 60); await e.wait(800);                                       // the visualizers
  await e.turn("SELECT", 8, 300); await e.wait(800);                                                                            // THEORY
  await e.tap("HOME"); await e.wait(1000);
  await e.tap("GLO"); await e.wait(150); await e.tap("GLO"); await e.wait(400);                                                // the set list
  e.spin("KNOB2", 1); await e.wait(300);                                    // slot 1: the piece playing
  e.spin("KNOB1", 1); await e.wait(200); e.spin("KNOB2", 1); await e.wait(200); e.spin("KNOB2", 1); await e.wait(200); e.spin("KNOB3", 4); await e.wait(300);   // slot 2: FUNK
  e.spin("KNOB1", 1); await e.wait(200); e.spin("KNOB2", 1); await e.wait(200); e.spin("KNOB2", 3); await e.wait(200); await e.wait(300);                    // slot 3: SYNTHWAVE
  await e.tap("HOME"); await e.wait(800);
  await e.wait(24000);                                                       // played long enough to come back to
  await e.tap("PLAY"); await e.wait(6000);                                   // stopped: the place written
  return { store: e.dumpStore() };
}

export default {
  seed: 7, fps: 30,
  prepare: prepare0170,
  async run(s) {
    s.start(30);
    await s.wait(10000);
    await s.tap("HOME", 60); await s.wait(120); await s.tap("HOME", 60);
    await s.wait(110000);
    await s.tap("GLO"); await s.wait(150); await s.tap("GLO"); await s.wait(3000);
  },
};
