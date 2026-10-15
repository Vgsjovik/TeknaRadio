# Leksjon 2 – Snakk sammen
### Kallenavn: _____________ &nbsp;&nbsp; Partner: _____________ &nbsp;&nbsp; Navn: _______________________________

**Kodefil:** `Leksjon2_Nodradio.ino` (brukes også i Leksjon 3)

---

## Del 1 – Montering

Alt fra Leksjon 1 blir stående. Legg til potmeteret nå, så slipper dere å koble mer senere.

- [ ] Midtben på potmeteret → **Pin A0**
- [ ] Høyre ytterben → **5V**
- [ ] Venstre ytterben → **GND**
- [ ] USB-kabel koblet mellom Arduino og PC

---

## Del 2 – Arduino IDE

- [ ] Åpnet filen `Leksjon2_Nodradio.ino` i Arduino IDE
- [ ] Skrevet de to kallenavnene fra Leksjon 1 øverst på dette arket
- [ ] Dette er en ny fil, så navnene må inn i koden en gang til. Funnet disse to linjene øverst:

    ```cpp
    #define KALLENAVN "ALFA"
    #define PARTNER   "BRAVO"
    ```

- [ ] Byttet `ALFA` med **vårt kallenavn**
- [ ] Byttet `BRAVO` med **partnerens kallenavn**
  - `ALFA` og `BRAVO` er bare eksempler. Ingen boks heter det.
  - La anførselstegnene stå. Store og små bokstaver spiller ingen rolle.
- [ ] Lastet opp koden til Arduino (**Upload**-knappen)

---

## Del 3 – Oppstart

- [ ] Matrisen viser `V` (radioen svarer), og så `K` (HUB hører dere)
- [ ] Navnet deres er ikke grått på tavla (grått betyr «ikke på nett»)
- [ ] Åpnet Serial Monitor (**Tools → Serial Monitor**, 9600 baud). Den viser `Kallenavn:` og `Partner:`.
      Trykk *reset* på Arduinoen om dere ikke ser noe.

| Ser du | Betyr | Gjør dette |
|---|---|---|
| `X` som blir stående | Radioen svarer ikke | Sjekk de sju kablene til radioen. Boksen prøver på nytt hvert sekund. |
| `?` | HUB hører dere ikke | Si fra til kursholderen. Alt annet virker likevel. |
| `N` | Navnet finnes ikke | Har dere byttet ut ALFA og BRAVO? Se i Serial Monitor: der står lista over gyldige navn. |

---

## Del 4 – Send meldinger

- [ ] Skriv en melding i Serial Monitor og trykk Enter, for eksempel `Hvordan går det?`
- [ ] Meldingen ruller over matrisen hos partneren, og står i deres Serial Monitor
- [ ] Meldingen står under «Samtaler» på tavla: `ALFA til BRAVO`, `Hvordan går det?`, med deres egne kallenavn
- [ ] Dere får et svar tilbake. Det ruller over matrisen deres.
- [ ] Rakk dere ikke å lese det? Trykk **Knapp B**. Da ruller siste melding en gang til.
- [ ] Skriv en melding som er lengre enn 28 tegn. Hva skjer?

**Prøv også:**

- [ ] Skriv `@alle Hei fra oss`. Hvem får meldingen nå?
- [ ] Skriv `@` og navnet på en annen boks enn partneren, og så en melding. Kom den fram?

---

## Refleksjonsspørsmål

> **Spørsmål 1:** Alle boksene i rommet sender på samme radiokanal. Hvorfor dukker ikke
> de andre parenes meldinger opp på matrisen deres?
>
> Svar: __________________________________________________________________________________________
>
> __________________________________________________________________________________________

> **Spørsmål 2:** Tavla viser alle meldingene i rommet. Hva sier det om hvem som kan
> lese det dere sender?
>
> Svar: __________________________________________________________________________________________
>
> __________________________________________________________________________________________

---

Når dere har fått til alt over:
**→ Gå videre til Leksjon 3**
