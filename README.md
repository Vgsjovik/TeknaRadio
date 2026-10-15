# TeknaRadio

Workshop om trådløs radiokommunikasjon, utviklet for Tekna.

Deltakerne bruker Arduino UNO R4 WiFi og NRF24L01-radiomoduler til å bygge sin egen nødradio: fra koblingstest til en frittstående tekstradio med potmeter.

---

## Hardware per boks

| Komponent | Tilkobling |
|-----------|------------|
| Arduino UNO R4 WiFi | |
| NRF24L01 + HW-200-adapter | CE→9, CSN→10, SCK→13, MOSI→11, MISO→12 |
| Passiv summer | Pin 3 (+), GND (–) |
| Trykknapp A | Pin 2, GND |
| Trykknapp B | Pin 4, GND |
| Potensiometer WH148 | A0 (midtben), 5V, GND |
| Breadboard 400 hull | |

> **Viktig:** Bruk HW-200-adapteren mellom NRF24L01 og Arduino. Adapteren tåler 5V, den løse modulen gjør det ikke.
> Pin 13 er i bruk av radioen (SCK). Bruk aldri `LED_BUILTIN`, bruk LED-matrisen.

Kursholderen trenger én ekstra Arduino + NRF24L01 + HW-200 som hub.

---

## Kom i gang

Installer i Arduino IDE:

- Kortpakken **Arduino UNO R4 Boards** (Tools → Board → Boards Manager)
- Biblioteket **RF24** av TMRh20 (Tools → Manage Libraries)
- Biblioteket **ArduinoGraphics** (følger IKKE med kortpakken, må installeres)

Velg kort: **Tools → Board → Arduino UNO R4 WiFi**

---

## Leksjoner

Hver boks har et kallenavn fra en fast liste med seks par: HAWK og JET, ICE og WOLF, ACE og PUMA, BOLT og ORCA, ION og FOX, LYNX og NOVA. Øverst i begge boksfilene settes to linjer: hvem boksen er, og hvem den snakker med.

```cpp
#define KALLENAVN "ALFA"
#define PARTNER   "BRAVO"
```

ALFA og BRAVO er eksempler og finnes ikke i lista. Lastes koden opp uten at de er byttet, viser boksen at navnet er ukjent.

| Leksjon | Fil | Hva skjer |
|---|---|---|
| **1. Bygg og test** | `Leksjon1/Leksjon1_RadioTest/Leksjon1_RadioTest.ino` | Boksen tester radioen ved oppstart (`V`/`X`) og melder seg hos HUB (`K`/`?`). Knapp A tenner partnerens skjerm, Knapp B slukker den. |
| **2. Snakk sammen** | `Leksjon2/Leksjon2_Nodradio/Leksjon2_Nodradio.ino` | Potmeteret kobles til. Tekst + Enter i Serial Monitor sendes til partneren og ruller over matrisen der. |
| **3. Uten PC** | Samme fil som Leksjon 2 | Potmeteret velger tegn, Knapp A legger til (`<` sletter), Knapp B sender. Knapp B uten tekst viser siste mottatte melding på nytt. |

Hver leksjon har et ark: `Leksjon1/Leksjon1_Sjekkliste.md`, `Leksjon2/Leksjon2_Sjekkliste.md`, `Leksjon3/Leksjon3_Sjekkliste.md`.

### Kursholderen

Last opp `Hub/Hub.ino` til kursholderens Arduino og åpne Serial Monitor (9600 baud). Huben hører alt i rommet og skriver én linje per hendelse:

```
ONLINE|FOX|ION
TENN|FOX|ION
MSG|FOX|ION|Hvordan går det?
```

Skriv tekst + Enter for å sende til alle bokser, eller `SEND|ion|tekst` for å sende til én.

### Tavla

`HubWeb/` er en nettside for projektor som leser de samme linjene fra huben: Milepæler (hva hver boks har fått til), Samtaler (alle meldingene, alt eller ett par) og Signalvei (en fast forklaring av hvem som hører hva). Den kjøres lokalt på kursholderens Mac med `server.py`, og har en demomodus som virker uten bokser. Se `HubWeb/README.md`.

---

## Tekniske detaljer

- **Kanal:** 80 (2480 MHz, øvre del av 2.4 GHz-båndet)
- **Hastighet:** 250 kbps (bedre rekkevidde)
- **Effekt:** PA_HIGH
- **To adresser:** `HUB00` (boks til hub, med kvittering) og `RADIO` (felles for alle, uten kvittering)
- **Alle hører alt.** Hver pakke sier hvem den er fra og til, og boksen viser bare det som er til den selv.
- **Livstegn:** boksene melder seg til huben hvert femte sekund.
- **Navnelista** står øverst i alle tre `.ino`-filene og må være lik i alle.

---

Utviklet av Lektor Sjøvik for Tekna.
