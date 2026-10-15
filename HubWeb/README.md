# TeknaRadio – hub-nettsiden

Viser på projektor hva huben hører på radioen. Tre faner:

- **Signalvei:** en fast forklaring med to eksempelpar (FOX–ION og ICE–WOLF). Velg en
  hendelse og se hvem som hører den, hvem som viser den, og hva som havner på tavla.
  Den bruker ikke det huben hører.

- **Milepæler:** én rad per boks, én kolonne per ting boksen skal ha fått til.
- **Samtaler:** alle boksene i en ring rundt HUB, og alt som er hørt, ved siden av.
  Klikk et navn eller en parknapp for å se bare det paret.

```
Arduino (Hub/Hub.ino)  ──USB──►  server.py  ──SSE──►  index.html i nettleseren
```

Serveren sender linjene fra huben uendret videre. Nettsiden tolker dem selv.
Formatet står øverst i `Hub/Hub.ino`.

## Start og stopp

Dobbeltklikk **`Start TeknaRadio Hub.app`**. Den starter serveren og åpner
**http://localhost:8765**. Stopp med **`Stopp TeknaRadio Hub.command`**.

`Start TeknaRadio Hub.command` gjør det samme i et Terminal-vindu med logg
(Ctrl-C stopper). Første gang lastes `pyserial` ned, og det krever internett.

## Uten maskinvare

Åpne «Med i dag» oppe til høyre og trykk **Start demo**. Da lager serveren
oppdiktet trafikk i samme format som huben. ORCA kommer på nett etter ca.
10 sekunder, og WOLF mister kontakten etter ca. 45. **Stopp demo** tømmer tavla.

Fra terminal: `./.venv/bin/python server.py --demo`

## Hva tavla viser

| På tavla | Kommer av |
|---|---|
| på nett | `ONLINE` eller `PULS` fra boksen de siste 15 sekundene |
| ikke på nett (grått navn) | hørt tidligere, men ingenting de siste 15 sekundene. Haken i «HUB hører deg» blir stående |
| skjerm tent | `TENN` til boksen, og ikke `SLUKK` eller omstart etterpå |
| Godkjent kallenavn | begge boksene er hørt, og hver av dem har den andre som `PARTNER` i koden |
| Tente partneren / Sendte melding | boksen har sendt `TENN` / `MSG` |
| Fikk melding | en `MSG` adressert til boksen (meldinger til alle teller ikke) |
| utenfor paret | en `MSG` til en annen enn partneren |

«Med i dag» velger hvilke kallenavn som vises. Valget huskes i nettleseren.

Meldinger fra sendelinja nederst går til én boks eller alle, maks 28 tegn.
