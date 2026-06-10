# TeknaRadio – TODO

_Oppdateres og committer etter hver sesjon._

---

## Status per 2026-06-09

### Leksjon 1 ✅
- [x] Én kodefil (RadioTest.ino) — ingen fasit-fil
- [x] Kodeoppgaver FJERNET (skapte forvirring) — kjører nå ferdig ut av boksen 2026-06-09
- [x] Knapp A bruker konfigurerRadio()/radio.begin() (pålitelig), ikke isChipConnected()
- [x] Mottak fra hub i loop
- [x] Kompilert OK (arduino-cli, 26%)

### Leksjon 2 ✅ (omskrevet til delt-pipe-modell)
- [x] Identisk kode på alle enheter, INGEN rollevalg
- [x] To grupper deler én pipe `"{PIPE}SAMT"`, begge veier, Auto-ACK av
- [x] `#define GRUPPE` (0–9, etikett) + `#define PIPE` (1–5, parring)
- [x] Knapp A/B tenn/slukk LED-skjerm hos de andre + melodi opp/ned
- [x] Serial-kommandoer (PA/AV) og fritekst med `G{nr}:`-prefix
- [x] Kompilert OK (arduino-cli, 26% flash)
- [ ] Fysisk test på hardware (generalprøve)

### Leksjon 3 ✅ (omskrevet — samme comms-kjerne som L2)
- [x] Én kodefil (SMS.ino), samme delte pipe som L2
- [x] Destinasjonsvalg fjernet — potmeter velger kun tegn
- [x] Potmeter + knapper for meldingsbygging (frekvensvektet tegnsett)
- [x] Serial Monitor valgfritt — ingen feil/blokkering uten PC
- [x] `#define NAVN` + `#define PIPE`
- [x] Kompilert OK (arduino-cli, 27% flash)
- [ ] Fysisk test på hardware (generalprøve)

### LaererHub ✅ (oppdatert — hører alt)
- [x] Lytter på HUB00 (pipe 0, Auto-ACK på) + samtale 1–5 (pipe 1–5, Auto-ACK av)
- [x] Per-pipe Auto-ACK → én sketch for alle tre leksjoner
- [x] Skiller L1-ping fra samtale via `radio.available(&pipe)`
- [x] Kompilert OK (arduino-cli, 26% flash)
- [ ] Fysisk test

### GitHub ✅
- [x] Repo opprettet: github.com/Vgsjovik/TeknaRadio
- [x] README basert på PROSJEKTPLAN
- [x] Kode pushet

---

## Utestående

- [ ] Fysisk test av all kode på ekte hardware (alle tre leksjoner + hub)
- [ ] Verifisere rekkevidde med PA_HIGH utendørs
- [ ] Webside/Python-script for bedre hub-visning (erstatter Serial Monitor) — til later

### ~~Refaktor: Forenkle L2 adressering~~ ✅ GJORT 2026-06-09
L2 og L3 er nå omskrevet til delt-pipe-modell: én felles adresse `"{PIPE}SAMT"` til både lytting og sending, ingen rollevalg, Auto-ACK av. To grupper parres ved samme PIPE (1–5). Se `Radiokanal_og_Adresser.md`.
