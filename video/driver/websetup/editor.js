if (!localStorage.getItem("hyp1-emu-midimap")) {
  const m = {};
  const L = [[70, 24, "Bass: Cutoff"], [71, 25, "Bass: Reso"], [72, 34, "Acid: Cutoff"], [73, 35, "Acid: Reso"], [74, 9, "Pad: Bright"], [75, 67, "Space: Echo"], [76, 69, "Space: Reverb"], [77, 47, "Drums: Swing"]];
  for (const [cc, p, name] of L) m["c:0:" + cc] = { t: "p" + p, mode: "abs", name };
  m["c:0:78"] = { t: "p161", mode: "rel64", name: "Drums: Ghosts" };
  localStorage.setItem("hyp1-emu-midimap", JSON.stringify(m));
  return "reload";
}
const b = document.getElementById("ctllearn"); b.disabled = false; b.click();
document.getElementById("ctlon").hidden = true;
document.getElementById("ctlbox").open = true;
document.getElementById("ctlstat").textContent = "CC 72 ch 1 → Acid: Cutoff. Click another setting, or Done.";
document.getElementById("ctl").scrollIntoView({ block: "start" }); window.scrollBy(0, -20);
return "ok";
