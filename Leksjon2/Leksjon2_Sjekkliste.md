# Leksjon 2 – Sjekkliste
### Gruppe: _______ &nbsp;&nbsp; Pipe: _______ &nbsp;&nbsp; &nbsp; Navn: _______________________________

---

## Del 1 – Montering

Ingen nytt utstyr fra Leksjon 1. Sjekk at alt sitter som det skal.

- [ ] NRF24L01 + HW-200-adapter koblet til pinner 9, 10, 11, 12, 13
- [ ] Passiv buzzer koblet til pin 3
- [ ] Knapp A koblet til pin 2 (én ben til pin, andre ben til GND)
- [ ] Knapp B koblet til pin 4 (samme oppsett)
- [ ] USB-kabel koblet mellom Arduino og PC

---

## Del 2 – Arduino IDE

- [ ] Åpnet filen `Leksjon2_Kommunikasjon.ino` i Arduino IDE
- [ ] Valgt riktig kort: **Tools → Board → Arduino UNO R4 WiFi**
- [ ] Valgt riktig port: **Tools → Port**
- [ ] Endret `#define GRUPPE` til gruppenummeret ditt (0–9): **_______**
- [ ] Endret `#define PIPE` til samtale-pipen læreren gir dere (1–5): **_______**
  - **Begge gruppene** som skal snakke sammen setter **samme PIPE**.
  - Det er PIPE som kobler dere sammen — ikke gruppenummeret.
- [ ] Kompilert og lastet opp til Arduino (✓ pil-knappen)

> **Slik fungerer det:** Begge gruppene sender og lytter på samme pipe, begge veier.
> Gruppenummeret deres vises foran meldingene, så de andre ser hvem som skriver.

---

## Del 3 – Oppstartstest

- [ ] Åpnet Serial Monitor (Sjekk at det står *9600 baud*) i Arduino IDE
- [ ] Ved oppstart viser matrisen gruppenummeret ditt kort, og Serial Monitor viser
      `Gruppe: N | Pipe: M` — sjekk at PIPE stemmer med den andre gruppa
      **Tips** Trykk på *reset*-knappen dersom du ikke ser meldingen i Serial Monitor.
- [ ] Trykk **Knapp A** → **de andres** skjerm tennes (full lysmatrise + melodi *opp*)
- [ ] Trykk **Knapp B** → **de andres** skjerm slukkes (mørk matrise + melodi *ned*
- [ ] Skriv en tekstmelding i Serial Monitor og trykk Enter (f.eks. `HELLO WORLD`)
      → den andre gruppa ser `G{ditt nummer}: HELLO WORLD!` i sin Serial Monitor
- [ ] Dere får melding fra den andre gruppa i deres Serial Monitor
- [ ] Forsøk å skrive PA eller AV i Serial monitor. Hva skjer da?

---
Når dere har fått til alt over:
**→ Gå videre til Leksjon 3**
