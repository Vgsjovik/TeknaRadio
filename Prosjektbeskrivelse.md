# TeknaRadio

**Trådløs radiokommunikasjon med Arduino. Workshop for elever på VGS og for voksne.**

Deltakerne bygger sin egen nødradio, og opplever hvordan trådløs kommunikasjon faktisk fungerer: adresser, kvitteringer og en kanal alle deler. Ikke som teori, men som knapper de trykker på og meldinger som ruller over en LED-skjerm de har koblet opp selv.

## Produktet

Hver gruppe får en 3D-printet boks med:

| Komponent | Funksjon |
|-----------|----------|
| Arduino UNO R4 WiFi | Hjernen, med innebygd 12×8 LED-matrise som skjerm |
| NRF24L01-radiomodul | Sender og mottar på 2,4 GHz (samme bånd som WiFi) |
| To trykknapper | Styr partnerens skjerm, skriv og send |
| Passiv summer | Lyd-feedback: pip, melodier og meldingsvarsler |
| Potensiometer | «Tastatur»: drei for å velge bokstaver |
| Breadboard og kabler | Deltakerne kobler alt selv |

Hver boks har et kallenavn, for eksempel HAWK, ORCA eller NOVA, og en partnerboks den snakker med. Det finnes tolv kallenavn, altså seks par.

Kursholderen har en egen hub som hører all radiotrafikk i rommet: hvem som er på nett, hvem som tenner skjermen til hvem, og hver melding med avsender og mottaker. Kursholderen kan også sende meldinger, til én boks eller til alle. En nettside på projektoren («tavla») viser dette i tre faner: Milepæler (hva hver boks har fått til), Samtaler (alle meldingene, alt eller ett par) og Signalvei (en fast forklaring av hvem som hører hva).

## Leksjonene

Hele opplegget tar omtrent to timer og bruker to kodefiler til boksene.

### Leksjon 1: Bygg og test
Deltakerne kobler radio, summer og knapper til Arduinoen og laster opp `Leksjon1_RadioTest.ino`. Boksen tester seg selv ved oppstart: `V` betyr at radioen svarer, `K` at huben har kvittert. Kvitteringen (ACK) er radioens egen måte å si «mottatt» på. Deretter styrer knappene partnerens skjerm: Knapp A tenner den, Knapp B slukker den.

### Leksjon 2: Snakk sammen
Potmeteret kobles til, og `Leksjon2_Nodradio.ino` lastes opp. Deltakerne skriver meldinger på tastaturet, og de ruller over skjermen hos partneren. Alle bokser sender på samme kanal og hører alt, men hver boks viser bare det som er adressert til den. Med `@navn` kan de sende til en annen boks, og med `@alle` til alle.

### Leksjon 3: Uten PC
Samme kode, men nå uten datamaskin. Deltakerne skriver tegn for tegn med potmeteret, legger til med Knapp A og sender med Knapp B. Vanlige bokstaver er lettest å treffe, slik de korteste morsetegnene er gitt til de vanligste bokstavene. Boksen kan gå på powerbank: en nødradio som virker uten nett og uten PC.

## Pedagogisk profil

- **Ingen kodeoppgaver.** All kode kjører ferdig. Deltakerne skriver inn to kallenavn øverst i filen og bruker tiden på å forstå og bruke radioen.
- **Umiddelbar feedback.** Hver handling gir lyd og lys. Feilkobling vises på skjermen, og boksen prøver på nytt til kablene sitter riktig.
- **Ekte teknologi.** Samme radiobrikke, frekvensbånd og begreper (adresser, kvittering, delt kanal) som profesjonell IoT bruker.

Opplegget er kjørt tre ganger for elever på VGS. Den gjeldende versjonen er lagt om for en workshop med voksne, og er ikke prøvd på ekte bokser ennå.

---

*Utviklet av Vegard Sjøvik for Tekna, 2026.
Kode og materiell: [github.com/Vgsjovik/TeknaRadio](https://github.com/Vgsjovik/TeknaRadio)*
