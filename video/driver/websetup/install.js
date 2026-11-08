const el = [...document.querySelectorAll("h2,h3,summary,legend")].find((e) => /full speed/i.test(e.textContent));
if (el) { const d = el.closest("details"); if (d) d.open = true; el.scrollIntoView({ block: "start" }); window.scrollBy(0, -40); }
return el ? el.textContent : "none";
