// Felles for hub-mockupene: en falsk hub som skriver linjer i samme format som Hub.ino,
// og modellen som leser dem. Den ekte siden bytter bare ut simulatoren med linjene fra serveren.
window.Hub = (function () {
  var S = { navn: [], med: {}, boks: {}, hendelser: [] };
  var lyttere = [], hendelseLyttere = [], pulsLyttere = [], nr = 0;

  function boks(n) {
    return S.boks[n] || (S.boks[n] = { navn: n, partner: null, sist: 0, forst: 0, tent: false, harTent: 0, sendt: 0, fatt: 0 });
  }
  function logg(h) {
    h.id = ++nr;
    S.hendelser.unshift(h);
    if (S.hendelser.length > 200) S.hendelser.pop();
    hendelseLyttere.forEach(function (f) { f(h); });
  }
  function varsle() { lyttere.forEach(function (f) { f(); }); }

  function linje(l, t) {
    t = t || Date.now();
    var f = l.split("|"), k = f[0], a, b, NAVN_OK = /^[A-Z0-9]{1,8}$/;
    // Navn settes rett inn i HTML flere steder, så alt som ikke ser ut som et kallenavn, stoppes her.
    if (k === "NAVN" ? !(f[1] || "").split(",").every(function (n) { return NAVN_OK.test(n); })
      : !NAVN_OK.test(f[1]) || (k !== "SENDT" && !NAVN_OK.test(f[2]))) return;
    if (k === "NAVN") {
      S.navn = f[1].split(",");
      S.navn.forEach(function (n) { boks(n); if (!(n in S.med)) S.med[n] = true; });
    } else if (k === "ONLINE" || k === "PULS") {
      a = boks(f[1]); a.partner = f[2]; a.sist = t;
      if (!a.forst) a.forst = t;
      if (k === "ONLINE") logg({ t: t, type: "online", fra: f[1] });
    } else if (k === "TENN" || k === "SLUKK") {
      a = boks(f[1]); b = boks(f[2]); a.sist = t;
      b.tent = k === "TENN";
      if (k === "TENN") a.harTent++;
      logg({ t: t, type: k === "TENN" ? "tenn" : "slukk", fra: f[1], til: f[2] });
    } else if (k === "MSG") {
      a = boks(f[1]); a.sist = t; a.sendt++;
      if (f[2] !== "ALLE") boks(f[2]).fatt++;
      logg({ t: t, type: "msg", fra: f[1], til: f[2], tekst: f.slice(3).join("|") });
    } else if (k === "SENDT") {
      logg({ t: t, type: "msg", fra: "HUB", til: f[1], tekst: f.slice(2).join("|") });
    }
    varsle();
  }

  var TEKST = { fri: "ikke med", venter: "ikke hørt", borte: "mistet kontakt", nett: "på nett", tent: "skjerm tent" };
  function status(n) {
    var b = boks(n);
    if (!S.med[n]) return "fri";
    if (!b.sist) return "venter";
    if (Date.now() - b.sist > 15000) return "borte";
    return b.tent ? "tent" : "nett";
  }
  function statustekst(n) { return TEKST[status(n)]; }

  // Parene bygges av det boksene selv har meldt som partner.
  function par() {
    var brukt = {}, ut = [];
    S.navn.forEach(function (n) {
      var p = S.boks[n].partner;
      if (brukt[n] || !p || !S.boks[p] || brukt[p]) return;
      if (!S.med[n] && !S.med[p]) return;
      brukt[n] = brukt[p] = 1;
      ut.push(S.navn.indexOf(n) < S.navn.indexOf(p) ? [n, p] : [p, n]);
    });
    S.navn.forEach(function (n) { if (!brukt[n] && S.med[n]) ut.push([n, null]); });
    return ut;
  }
  function parTil(n) { return par().filter(function (p) { return p.indexOf(n) > -1; })[0] || null; }
  function forhold(h, p) {
    if (h.til === "ALLE") return "alle";
    return p.indexOf(h.fra) > -1 || p.indexOf(h.til) > -1;
  }
  function setning(h) {
    if (h.type === "msg") return { hvem: h.fra + " til " + (h.til === "ALLE" ? "alle" : h.til), tekst: h.tekst };
    if (h.type === "tenn") return { hvem: h.fra + " tente skjermen til " + h.til };
    if (h.type === "slukk") return { hvem: h.fra + " slukket skjermen til " + h.til };
    return { hvem: h.fra + " er på nett" };
  }
  function merke(h) {
    if (h.type !== "msg") return "";
    if (h.til === "ALLE") return '<span class="merke alle">til alle</span>';
    if (h.fra === "HUB") return '<span class="merke alle">fra HUB</span>';
    if (boks(h.fra).partner !== h.til) return '<span class="merke">utenfor paret</span>';
    return "";
  }
  function kl(t) { return new Date(t).toLocaleTimeString("nb-NO", { hour12: false }); }
  function esc(s) { return String(s).replace(/[&<>"]/g, function (c) { return { "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" }[c]; }); }

  // Topplinje med faner og oppsett, og sendelinja nederst. Likt i alle tre mockupene.
  function skall(faner) {
    var t = document.getElementById("topp"), b = document.getElementById("bunn");
    t.className = "topp";
    t.innerHTML = '<img src="tekna-logo.svg" alt="Tekna" width="39" height="56"><h1>TeknaRadio Hub</h1>' +
      '<div class="faner" role="tablist">' + faner.map(function (f, i) {
        return '<button role="tab" aria-selected="' + (i === 0) + '" data-f="' + f.id + '">' + f.navn + '</button>';
      }).join("") + '</div>' +
      '<div class="hoyre"><span>Eksempeldata. Ingen hub er koblet til.</span>' +
      '<details class="med"><summary id="medtall"></summary><div class="medliste" id="medliste"></div></details></div>';
    t.querySelector(".faner").onclick = function (e) {
      var id = e.target.dataset.f;
      if (!id) return;
      [].forEach.call(this.children, function (k) { k.setAttribute("aria-selected", k === e.target); });
      faner.forEach(function (f) { document.getElementById("vis-" + f.id).hidden = f.id !== id; });
    };
    var ml = document.getElementById("medliste");
    ml.innerHTML = S.navn.map(function (n) {
      return '<label><input type="checkbox" id="med-' + n + '" value="' + n + '"' + (S.med[n] ? " checked" : "") + '>' + n + '</label>';
    }).join("");
    ml.onchange = function (e) { S.med[e.target.value] = e.target.checked; varsle(); };

    b.innerHTML = '<form class="send" id="sendskjema" novalidate>' +
      '<div class="felt"><label for="til">Til</label><select id="til"></select></div>' +
      '<div class="felt"><label for="tekst" id="tekstlabel">Melding</label><input id="tekst" maxlength="28" autocomplete="off"></div>' +
      '<button>Send</button></form>';
    var til = document.getElementById("til"), tekst = document.getElementById("tekst"), lab = document.getElementById("tekstlabel");
    function teller() { lab.textContent = "Melding (" + tekst.value.length + " av 28 tegn)"; }
    tekst.oninput = function () { tekst.removeAttribute("aria-invalid"); teller(); };
    document.getElementById("sendskjema").onsubmit = function (e) {
      e.preventDefault();
      var v = tekst.value.trim();
      if (!v) { tekst.setAttribute("aria-invalid", "true"); lab.textContent = "Skriv en melding først"; tekst.focus(); return; }
      linje("SENDT|" + til.value + "|" + v);
      tekst.value = ""; teller();
    };
    function oppdater() {
      var medNavn = S.navn.filter(function (n) { return S.med[n]; });
      document.getElementById("medtall").textContent = "Med i dag: " + medNavn.length + " av " + S.navn.length;
      var opt = '<option value="ALLE">Alle</option>' + medNavn.map(function (n) { return "<option>" + n + "</option>"; }).join("");
      if (til.dataset.o !== opt) { var v = til.value; til.innerHTML = opt; til.dataset.o = opt; til.value = v; if (!til.value) til.value = "ALLE"; }
    }
    lyttere.push(oppdater); oppdater(); teller();
  }

  // Lista «Hørt på radioen» med valg mellom alt og ett par.
  function feed(el) {
    el.classList.add("feed");
    el.innerHTML = '<header><h2>Hørt på radioen</h2><div class="felt"><label for="vis">Vis</label><select id="vis"></select></div></header><ol></ol>';
    var sel = el.querySelector("select"), ol = el.querySelector("ol"), sist = "";
    function tegn() {
      var pr = par().filter(function (p) { return p[1]; });
      var opt = '<option value="">Alt</option>' + pr.map(function (p) { return '<option value="' + p[0] + '">' + p[0] + " og " + p[1] + "</option>"; }).join("");
      if (sel.dataset.o !== opt) { var v = sel.value; sel.innerHTML = opt; sel.dataset.o = opt; sel.value = v; if (sel.value !== v) sel.value = ""; }
      var valgt = pr.filter(function (p) { return p[0] === sel.value; })[0];
      var rader = S.hendelser.filter(function (h) { return !valgt || forhold(h, valgt); }).slice(0, 40);
      var sig = sel.value + "|" + (rader[0] ? rader[0].id : "") + "|" + rader.length;
      if (sig === sist) return;
      sist = sig;
      ol.innerHTML = rader.map(function (h) {
        var s = setning(h);
        return "<li><time>" + kl(h.t) + '</time><span class="hvem">' + s.hvem + merke(h) + "</span>" +
          (s.tekst ? '<span class="tekst">' + esc(s.tekst) + "</span>" : "") + "</li>";
      }).join("") || '<li class="tom">Ingenting hørt ennå.</li>';
    }
    sel.onchange = tegn;
    lyttere.push(tegn); tegn();
    return { velg: function (n) { var p = parTil(n); sel.value = p && p[1] ? p[0] : ""; tegn(); } };
  }

  // ---- Simulatoren: oppdiktet trafikk ----
  var NAVN = ["HAWK", "JET", "ICE", "WOLF", "ACE", "PUMA", "BOLT", "ORCA", "ION", "FOX", "LYNX", "NOVA"];
  var PARTNER = {}, sim = {};
  NAVN.forEach(function (n, i) { PARTNER[n] = NAVN[i % 2 ? i - 1 : i + 1]; sim[n] = { pa: n !== "ORCA" && n !== "LYNX", neste: 0 }; });
  var FRASER = ["Hei! Ser du denne?", "HEI", "Funker det hos deg?", "TEST 1 2 3", "Kom dette fram?", "SVAR HVIS DU SER", "Vi er i gang", "Prøver uten PC nå", "OK", "JA"];

  (function seed() {
    var n0 = Date.now(), s = function (sek, l) { linje(l, n0 + sek * 1000); };
    S.med.LYNX = false;
    linje("NAVN|" + NAVN.join(","));
    NAVN.forEach(function (n, i) { if (sim[n].pa) s(-600 + i * 9, "ONLINE|" + n + "|" + PARTNER[n]); });
    s(-240, "TENN|JET|HAWK"); s(-225, "SLUKK|JET|HAWK"); s(-210, "TENN|ICE|WOLF"); s(-200, "SLUKK|ICE|WOLF");
    s(-150, "MSG|ACE|ALLE|Vi har første melding!"); s(-120, "MSG|JET|HAWK|Funker det?"); s(-100, "MSG|HAWK|JET|JA");
    s(-80, "MSG|HAWK|PUMA|Hvem er du sammen med?"); s(-60, "MSG|ICE|WOLF|Kjører forsøk 2 nå");
    s(-40, "TENN|ACE|PUMA"); s(-20, "MSG|FOX|ION|Hei! Ser du denne?"); s(-8, "TENN|FOX|ION");
    NAVN.forEach(function (n) { if (sim[n].pa) s(-1, "PULS|" + n + "|" + PARTNER[n]); });
  })();

  function tilfeldig(a) { return a[Math.floor(Math.random() * a.length)]; }
  function aktive() { return NAVN.filter(function (n) { return sim[n].pa && S.med[n]; }); }
  function handling() {
    var a = aktive(), ipar = a.filter(function (n) { return a.indexOf(PARTNER[n]) > -1; }), n = tilfeldig(ipar), r = Math.random();
    if (n) {
      var p = PARTNER[n];
      if (r < 0.35) linje((boks(p).tent ? "SLUKK|" : "TENN|") + n + "|" + p);
      else if (r < 0.8) linje("MSG|" + n + "|" + p + "|" + tilfeldig(FRASER));
      else if (r < 0.9) linje("MSG|" + n + "|" + tilfeldig(a.filter(function (x) { return x !== n && x !== p; })) + "|Hvem er du sammen med?");
      else linje("MSG|" + n + "|ALLE|" + tilfeldig(["Hei alle sammen", "Vi har første melding!"]));
    }
    setTimeout(handling, 2600 + Math.random() * 2600);
  }
  function start() {
    var naa = Date.now();
    NAVN.forEach(function (n) { sim[n].neste = naa + Math.random() * 5000; });
    setInterval(function () {
      var t = Date.now();
      aktive().forEach(function (n) {
        if (t < sim[n].neste) return;
        sim[n].neste = t + 5000;
        linje("PULS|" + n + "|" + PARTNER[n]);
        pulsLyttere.forEach(function (f) { f(n); });
      });
      varsle();
    }, 1000);
    setTimeout(handling, 3000);
    setTimeout(function () { sim.ORCA.pa = true; linje("ONLINE|ORCA|BOLT"); }, 9000);
    (function utfall() {
      setTimeout(function () { sim.WOLF.pa = false; }, 28000);
      setTimeout(function () { sim.WOLF.pa = true; }, 62000);
      setTimeout(utfall, 110000);
    })();
  }

  return {
    S: S, boks: boks, linje: linje, status: status, statustekst: statustekst, par: par, parTil: parTil,
    forhold: forhold, setning: setning, merke: merke, kl: kl, esc: esc, skall: skall, feed: feed, start: start,
    ved: function (f) { lyttere.push(f); }, vedHendelse: function (f) { hendelseLyttere.push(f); }, vedPuls: function (f) { pulsLyttere.push(f); }
  };
})();
