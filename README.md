# TeknaRadio

Workshopkurs om trådløs radiokommunikasjon for elever (~13–16 år), utviklet for Tekna.

Elevene bruker Arduino UNO R4 WiFi og NRF24L01-radiomoduler til å bygge sine egne radiosendere og -mottakere — fra enkel koblingstest til frittstående SMS-radio med potmeter.

---

## Hardware per gruppe (2 elever)

| Antall | Komponent | Tilkobling |
|--------|-----------|------------|
| 2 | Arduino UNO R4 WiFi | — |
| 2 | NRF24L01 + HW-200-adapter | CE→9, CSN→10, SCK→13, MOSI→11, MISO→12 |
| 2 | Passiv summer | Pin 3 (+), GND (–) |
| 2 | Trykknapp A | Pin 2, GND |
| 2 | Trykknapp B | Pin 4, GND |
| 2 | Potensiometer WH148 | A0 (midtben), 5V, GND |
| 1 | Breadboard 400 hull | — |
| 14+ | Jumperkabler | — |

> **Viktig:** Bruk HW-200-adapteren mellom NRF24L01 og Arduino. Adapteren tåler 5V — den løse modulen gjør det ikke.
> Pin 13 er i bruk av radioen (SCK). Bruk aldri `LED_BUILTIN` — bruk LED-matrisen.

Læreren trenger én ekstra Arduino + NRF24L01 + HW-200 for å kjøre `LaererHub`.

---

## Kom i gang

### Nødvendige Arduino-biblioteker

Installer via Arduino IDE (Tools → Manage Libraries):

- **RF24** av TMRh20
- **ArduinoGraphics** (følg med board-pakken, men installer manuelt om nødvendig)
- **Arduino_LED_Matrix** (følger med `arduino:renesas_uno` board-pakken)

### Board-konfig

Board: **Arduino UNO R4 WiFi**  
(Tools → Board → Arduino UNO R4 WiFi)

### Oppstart for læreren

1. Last opp `LaererHub/LaererHub.ino` til lærerens Arduino
2. Koble til projektor via USB og åpne Serial Monitor (9600 baud)
3. Hub er klar når den viser `LAERER-HUB er klar` i Serial Monitor

---

## Leksjoner

### Leksjon 1 — Er radioen koblet til?

Elevene kobler NRF24L01 til Arduino og sjekker at det virker.

- **Knapp A:** Tester om radioen svarer (starter radioen på nytt og sjekker SPI-kontakt)
- **Knapp B:** Sender ping til lærers hub — ACK bekrefter at huben mottok, og lærerens skjerm viser gruppenummeret

Ingen kodeoppgave — koden kjører ferdig. Elevene setter bare `#define GRUPPE`. RF24-biblioteket er ferdig installert.

**Fil:** `Leksjon1/Leksjon1_RadioTest/Leksjon1_RadioTest.ino`  
**Sjekkliste:** `Leksjon1/Leksjon1_Sjekkliste.md`

---

### Leksjon 2 — To grupper snakker sammen

Alle enheter kjører identisk kode. Ingen rollevalg — to grupper deler én felles pipe og snakker begge veier.

- To innstillinger: `#define GRUPPE` (0–9, identitet) og `#define PIPE` (1–5). Begge gruppene i et par setter **samme PIPE**.
- **Knapp A:** Tenn LED-skjermen til alle andre på pipen + melodi opp (C→E→G)
- **Knapp B:** Slukk de andres skjerm + melodi ned (G→E→C)
- Serial Monitor: skriv `PA` / `AV` for samme effekt, eller send fritekst (vises med `G{nr}:`-prefix hos de andre)

Ingen kodeoppgave — koden kjører ferdig. Fokus er å bruke og forstå radioen.

**Fil:** `Leksjon2/Leksjon2_Kommunikasjon/Leksjon2_Kommunikasjon.ino`  
**Sjekkliste:** `Leksjon2/Leksjon2_Sjekkliste.md`

---

### Leksjon 3 — Frittstående SMS-radio

Samme delte pipe som Leksjon 2 — de samme to gruppene samarbeider. Enheten trenger ikke PC; potmeter og knapper gjør alt.

1. Drei potmeter for å velge bokstaver/tall. **Knapp A:** legg til tegn. **Knapp B:** send.
2. Meldinger sendes med navn-prefix: `ALPHA: HALLO`
3. Mottatte meldinger ruller over LED-matrisen + pip
4. **(Valgfritt, krever PC):** Serial Monitor viser alle meldinger, og du kan skrive melding + Enter for å sende. Fungerer helt uten PC — ingen feilmelding.

Elevene endrer `#define NAVN` og `#define PIPE` øverst i koden.

**Fil:** `Leksjon3/Leksjon3_SMS/Leksjon3_SMS.ino`  
**Sjekkliste:** `Leksjon3/Leksjon3_Sjekkliste.md`

---

## Filstruktur

```
TeknaRadio/
├── LaererHub/
│   └── LaererHub.ino          ← lærers enhet (alle leksjoner, hører alle pipes)
├── Leksjon1/
│   ├── Leksjon1_RadioTest/
│   │   └── Leksjon1_RadioTest.ino
│   └── Leksjon1_Sjekkliste.md
├── Leksjon2/
│   ├── Leksjon2_Kommunikasjon/
│   │   └── Leksjon2_Kommunikasjon.ino
│   └── Leksjon2_Sjekkliste.md
├── Leksjon3/
│   ├── Leksjon3_SMS/
│   │   └── Leksjon3_SMS.ino
│   └── Leksjon3_Sjekkliste.md
└── Demo/
    └── MeldingsDemo/
        └── MeldingsDemo.ino   ← demo-sketch for presentasjon
```

---

## Tekniske detaljer

- **Kanal:** 80 (2480 MHz — øvre del av 2.4 GHz-båndet)
- **Hastighet:** 250 kbps (bedre rekkevidde)
- **Effekt:** PA_HIGH
- **Topologi L1:** Alle elever → hub (`HUB00`), Auto-ACK på
- **Topologi L2 og L3:** Delt pipe per par (`"{PIPE}SAMT"`, PIPE 1–5), begge veier, Auto-ACK av
- **Hub:** Lytter på `HUB00` + alle 5 samtale-pipene samtidig (per-pipe Auto-ACK)

---

Utviklet av Lektor Sjøvik for Tekna-workshopkurset.
