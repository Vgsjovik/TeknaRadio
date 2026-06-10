# TeknaRadio

**Trådløs radiokommunikasjon med Arduino — workshopkurs for elever (13–16 år)**

Elevene bygger sin egen radiosender og -mottaker, og opplever hvordan trådløs
kommunikasjon faktisk fungerer: adresser, kanaler, kvitteringer og kollisjoner —
ikke som teori, men som knapper de trykker på og meldinger som ruller over en
LED-skjerm de har koblet opp selv.

## Produktet

Hver gruppe (2 elever) får en 3D-printet boks med:

| Komponent | Funksjon |
|-----------|----------|
| Arduino UNO R4 WiFi | Hjernen — med innebygd 12×8 LED-matrise som skjerm |
| NRF24L01-radiomodul | Sender og mottar på 2,4 GHz (samme bånd som WiFi) |
| To trykknapper | Test, send og skriv |
| Passiv summer | Lyd-feedback: pip, melodier og meldingsvarsler |
| Potensiometer | «Tastatur» — drei for å velge bokstaver |
| Breadboard og kabler | Elevene kobler alt selv |

Læreren har en egen hub som hører all radiotrafikk i klasserommet og viser den
live på projektor — som en chat-app, med egne samtaletråder per gruppepar.
Læreren kan også sende meldinger tilbake: til én gruppe eller til hele klassen.

## Leksjonene

### Leksjon 1 — Er radioen koblet til?
Elevene kobler radiomodulen til Arduino og verifiserer at den virker — uten
PC-skjerm. Knapp A tester tilkoblingen (✓ eller ✗ på LED-matrisen). Knapp B
sender en ping til læreren — og radioens innebygde kvitteringsmekanisme (ACK)
forteller eleven om meldingen kom frem. Gruppenummeret dukker opp på
projektoren: *du er på nett.*

### Leksjon 2 — To grupper snakker sammen
To grupper deler én radiokanal («pipe») og kommuniserer begge veier med
identisk kode — ingen sender/mottaker-roller. Knapp A tenner skjermen til den
andre gruppa (med stigende melodi), Knapp B slukker den. Via Serial Monitor
kan gruppene chatte fritt. Her oppdager elevene kringkastingens natur: alle på
samme adresse hører alt.

### Leksjon 3 — Frittstående SMS-radio
Enheten klarer seg helt uten PC. Elevene skriver meldinger tegn for tegn med
potensiometeret (frekvensvektet tegnsett — vanlige bokstaver er lettest å
treffe), legger til tegn med Knapp A og sender med Knapp B. Meldingene ruller
over mottakerens LED-matrise med navneprefiks: `ALPHA: HEI BRAVO`. En
radiotelegraf i lomma — bygget på tre leksjoner.

## Pedagogisk profil

- **Ingen kodeoppgaver** — all kode kjører ferdig ut av boksen. Elevene endrer
  kun innstillinger øverst i filen (gruppenummer, kanal, kallenavn) og bruker
  tiden på å *forstå og bruke* radioen.
- **Umiddelbar feedback** — hver handling gir lyd og lys. Feilkobling oppdages
  med ett knappetrykk.
- **Ekte teknologi** — samme radiobrikke, frekvensbånd og protokollbegreper
  (adresser, ACK, kanaler) som profesjonell IoT bruker.

---

*Utviklet av Vegard Sjøvik for Tekna-kurs, 2026.
Kode og elevmateriell: [github.com/Vgsjovik/TeknaRadio](https://github.com/Vgsjovik/TeknaRadio)*
