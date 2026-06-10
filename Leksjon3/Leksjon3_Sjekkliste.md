# Leksjon 3 – Sjekkliste
### Gruppe: _______ &nbsp;&nbsp; Pipe: _______ &nbsp;&nbsp;&nbsp; Navn: _______________________________

---

## Del 1 – Montering

- [ ] Samme oppkobling som Leksjon 2 (NRF24L01 + HW-200, buzzer, 2 knapper)

**Legg til potensiometer:**

- [ ] Midt-ben → **Pin A0**
- [ ] Høyre ytterbein → **5V**
- [ ] Venstre ytterbein → **GND**
- [ ] USB-kabel kan kobles til PC — men er **ikke nødvendig**. Radioen fungerer helt
      uten laptop. Serial Monitor er bare en bonus.

---

## Del 2 – Arduino IDE

- [ ] Åpnet filen `Leksjon3_SMS.ino` i Arduino IDE
- [ ] Valgt riktig kort: **Tools → Board → Arduino UNO R4 WiFi**
- [ ] Valgt riktig port: **Tools → Port**
- [ ] `#define NAVN` endret til et gruppenavn dere velger selv (maks 8 tegn, bare store bokstaver/tall, ingen mellomrom — f.eks. `"ALPHA"`, `"BRAVO"`, `"TEKNA"`)
- [ ] `#define PIPE` endret til samtale-pipen læreren gir dere (1–5): **_______**
  - **Begge gruppene** i paret setter **samme PIPE** som i Leksjon 2.
- [ ] Koden lastet opp til Arduino

> **Samme radio som Leksjon 2:** Dere deler én pipe og snakker begge veier. Det nye
> her er at dere skriver meldinger med potmeteret i stedet for tastaturet.

---

## Del 3 – Test

- [ ] Slå på/restart Arduino: matrisen viser tegn-velgeren med en gang 
- [ ] Drei potmeter — se at det aktuelle tegnet endrer seg på matrisen
- [ ] Legg til 2–3 tegn med Knapp A (du hører et pip per tegn)
- [ ] Trykk Knapp B — meldingen sendes med NAVN-prefix (`ALPHA: HEI`)
- [ ] Den andre gruppa (samme PIPE) får meldingen: den ruller over matrisen deres
- [ ] Få en melding tilbake — se at den ruller over din matrise
- [ ] **(Hvis du har PC):** Åpne Serial Monitor (9600 baud) — meldinger vises også der,
      og du kan skrive en melding + Enter for å sende uten potmeteret
- [ ] **Test uten PC:** Koble fra USB-strøm via et powerbank/USB-lader i stedet — alt
      skal fungere uten feilmelding
- [ ] La alle prøve å skrive en melding og sende den til den andre gruppa

---

Her må jeg skrive litt tekst, slik at neste overskrift kommer på ny side.

---

## Refleksjonsspørsmål

> **Spørsmål 1:** Hva er poenget med å bruke potmeter som input?
>
> Svar: __________________________________________________________________________________________
>
> __________________________________________________________________________________________

> **Spørsmål 2:** Hvilke bruksområder kan du tenke deg for dette appratet som dere nå har laget? 
>
> Svar: __________________________________________________________________________________________
>
> __________________________________________________________________________________________
> 
>> **Spørsmål 3:** Du kan koble alle mulig slags sensorer til en Arduino UNO. Kan dere tenke på noe som hadde vært kult å lage? 
>
> Svar: __________________________________________________________________________________________
>
> __________________________________________________________________________________________

---

## Ferdig! Bra jobba!
