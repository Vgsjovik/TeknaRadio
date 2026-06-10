# Radiokanal og adresser — lærernotat

> **For læreren.** Dette dokumentet forklarer det tekniske fundamentet i TeknaRadio-systemet:
> hva kanal og adresse faktisk betyr, hvordan adressene er bygget opp, og hvordan topologien
> skifter mellom leksjonene. Ingen steg-for-steg-instruksjoner — bare forklaringer.

---

## 1. Hva er en radiokanal, og hvorfor kanal 80?

NRF24L01 opererer i **2.4 GHz-båndet**. Innenfor det båndet kan den velge mellom 128 kanaler
(0–127). Hver kanal er 1 MHz bred. Kanalens frekvens regnes ut slik:

```
Frekvens = 2400 + kanalnummer  (MHz)
Kanal 80 = 2400 + 80 = 2480 MHz
```

Tenk på det som FM-radio: alle sender og mottar på en avtalt frekvens. Hvis to radioer ikke er
på samme frekvens, hører de ikke hverandre, uansett hvor høyt de "roper".

### Hvorfor akkurat 80?

Standard WiFi bruker 2.4 GHz-båndet, men konsentrert i de tre "ikke-overlappende" kanalene:

```
WiFi kanal 1  → 2412 MHz
WiFi kanal 6  → 2437 MHz
WiFi kanal 11 → 2462 MHz
                         ← 2480 MHz (TeknaRadio, kanal 80)
```

Kanal 80 (2480 MHz) ligger i den øvre enden av båndet og overlapper minimalt med de vanligste
WiFi-kanalene. Skolen vil alltid ha WiFi-trafikk, og vi vil ikke at det skal forstyrre klassen.

Andre konfigurasjonsdetaljer som hjelper med dette:

- **RF24_250KBPS** — lav datahastighet gir bedre sensitivitet og rekkevidde, men også smalere
  kanalbruk. Tregere er faktisk bedre her.
- **RF24_PA_HIGH** — full sendestyrke, men fordi vi er i samme rom er dette overkill. Det gir
  likevel jevnere forbindelse.

---

## 2. Hva er en radioadresse?

Frekvensen (kanalen) bestemmer hvem som *kan* høre hverandre. Men på kanal 80 sender jo alle
klassens grupper. Adressen bestemmer hvem du *faktisk vil snakke med*.

Analogier:
- **Frekvens** = postnummer (du er i riktig by)
- **Adresse** = gatenavn og nummer (du frakter pakken til riktig dør)

Teknisk sett er en RF24-adresse **5 bytes**. I koden bruker vi lesbare ASCII-strenger:

```c
const byte hubAdresse[6] = "HUB00";
//                          H U B 0 0  <- 5 tegn = 5 bytes
//                          + 1 null-byte fra C-streng-konvensjon
```

Biblioteket bruker bare de første 5 bytene. Den sjette (null-tegnet) er der fordi C-strenger
alltid avsluttes med `\0` — RF24 ignorerer den.

### De to adressemønstrene i TeknaRadio

| Adresse | Brukt av | Eksempel |
|---------|----------|----------|
| `"HUB00"` | Lærers hub i Leksjon 1 | `"HUB00"` |
| `"{P}SAMT"` | Leksjon 2 og 3, felles samtale-pipe | `"1SAMT"` (pipe 1) |

`{P}` er **samtale-pipen** (1–5), satt av eleven i `#define PIPE`. Den avgjør hvilken samtale
enheten er med i. Gruppenummeret (`GRUPPE` i Leksjon 2, `NAVN` i Leksjon 3) brukes **ikke**
lenger i adressen — det er kun en etikett som settes foran meldingsteksten, slik at mottakeren
ser hvem som skriver.

> **Hvorfor ligger sifferet fremst i `"1SAMT"`?** Det er en NRF24-regel (se punkt 6): for at
> lærerhuben skal kunne lytte på alle fem samtalene samtidig, må adressene bare skille seg på
> den **nederste** byten. I C-arrayet er det byte nummer 0 — altså det første tegnet. Derfor
> `"1SAMT"`, `"2SAMT"`, ... og ikke `"SAMT1"`.

---

## 3. Hva er en pipe?

NRF24L01 kan lytte på **opptil 6 adresser samtidig** — disse lytteplassene kalles **pipes**
(rør). Tenk på det som seks postkasser på samme vegg. Du kan ha brevet adressert til hvilken
som helst av de seks og plukke det opp.

```
Pipe 0  – Full 5-byte-adresse. Brukes også internt til Auto-ACK ved sending.
Pipe 1  – Full 5-byte-adresse. Vår primære lyttepipe på elev-enhetene.
Pipe 2–5 – Deler de fire øverste bytene med pipe 1; bare nederste byte er fri.
```

På en **elev-enhet** (Leksjon 2 og 3) ser det slik ut — samme adresse brukes til både lytting
og sending, fordi begge gruppene deler én felles pipe:

```c
radio.openReadingPipe(1, samtaleAdresse);  // lytt på f.eks. "1SAMT"
radio.openWritingPipe(samtaleAdresse);     // skriv til samme adresse
```

Når du kaller `startListening()`, er enheten i mottakermodus. Når du kaller `stopListening()`,
bytter den til sendemodus (se punkt 5 om half-duplex).

På **lærerhuben** brukes flere pipes samtidig — se punkt 6.

---

## 4. Innstillingene: hvordan PIPE og GRUPPE/NAVN mappes

Eleven setter to konstanter øverst i koden.

**Leksjon 2:**
```c
#define GRUPPE 3      // identitet (0–9), vises foran meldingene
#define PIPE   1      // felles samtale-pipe (1–5)
```

**Leksjon 3:**
```c
#define NAVN "ALPHA"  // identitet (kallenavn), vises foran meldingene
#define PIPE  1       // felles samtale-pipe (1–5)
```

Koden bygger samtale-adressen automatisk med `snprintf`:

```c
snprintf((char*)samtaleAdresse, 6, "%dSAMT", PIPE);  // PIPE 1 → "1SAMT"
```

### Slik kobles to grupper sammen

To grupper havner i **samme samtale ved å sette samme PIPE**. Gruppenummeret/​navnet kan (og
skal) være forskjellig — det er bare en etikett. Med 10 grupper og 5 pipes blir det fem par:

```
PIPE 1:  Gruppe 0  ◄──►  Gruppe 1
PIPE 2:  Gruppe 2  ◄──►  Gruppe 3
PIPE 3:  Gruppe 4  ◄──►  Gruppe 5
PIPE 4:  Gruppe 6  ◄──►  Gruppe 7
PIPE 5:  Gruppe 8  ◄──►  Gruppe 9
```

Du kan parre hvilke grupper du vil — det er PIPE-tallet som styrer, ikke gruppenummeret.
Vil du sette gruppe 3 sammen med gruppe 8, gir du dem bare samme PIPE.

> **Mer enn to grupper på én pipe?** Det går fint. Alle som deler en pipe hører alle andre på
> samme pipe (en liten "chatkanal"). Hver enhet i en gruppe på 2 elever har sin egen Arduino,
> så et par på to grupper er teknisk sett opptil fire enheter på samme pipe. Etiketten foran
> meldingen viser hvem som skriver.

---

## 5. Half-duplex og hvorfor Auto-ACK er AV i Leksjon 2/3

### Half-duplex

NRF24L01 kan **ikke sende og motta samtidig**. Den er alltid i én av to tilstander:

```
[Lyttermodus]  ──────────────── radio.startListening()
[Sendemodus]   ──────────────── radio.stopListening() → radio.write() → radio.startListening()
```

Sekvensen for å sende en melding er alltid:
1. `stopListening()` — skrur av mottakeren
2. `write(data, lengde)` — sender pakken
3. `startListening()` — skrur mottakeren på igjen

Uten det siste steget vil enheten aldri høre svar.

### Auto-ACK — på i Leksjon 1, av i Leksjon 2/3

Auto-ACK er en funksjon der mottakeren automatisk sender en liten kvitteringspakke (**ACK**)
tilbake til avsenderen, helt uten at programmet gjør noe. `radio.write()` returnerer da `true`
hvis ACK kom innen timeout.

**Leksjon 1 (Auto-ACK PÅ):** Hver gruppe sender til `"HUB00"`, og det er bare én mottaker
(huben). Huben kvitterer automatisk, og elevens `write()` returnerer `true` — det er nettopp
slik Leksjon 1 sjekker "er huben på nett?".

**Leksjon 2 og 3 (Auto-ACK AV):** Her deler flere enheter **samme adresse** på én pipe. Hvis
Auto-ACK var på, ville *alle* lytterne prøve å kvittere på samme melding samtidig — og
ACK-pakkene ville kollidere. Derfor slår vi den av:

```c
radio.setAutoAck(false);
```

Konsekvensen: `write()` returnerer ikke lenger en pålitelig "kom den frem?"-bekreftelse. Det
er greit her — kommunikasjonen er en åpen kringkasting på pipen, ikke en punkt-til-punkt-link.

`setRetries(5, 3)` står fortsatt i koden, men har ingen praktisk effekt når Auto-ACK er av
(det er ingen ACK å vente på). Vi lar den stå for konsistens.

---

## 6. Topologi per leksjon

### Leksjon 1 — Stjernetopologi (uendret)

Alle elev-enheter sender til hub. Hub lytter på `"HUB00"` og kvitterer automatisk via Auto-ACK.

```
  Gruppe 0 ──────────────────────────────►
  Gruppe 1 ──────────────────────────────►   HUB00
  Gruppe 2 ──────────────────────────────►  (lærerhub)
  Gruppe 3 ──────────────────────────────►
               alle sender til "HUB00"
               hub sender ACK automatisk
```

---

### Leksjon 2 og 3 — Delt pipe per par

To grupper deler én pipe og snakker begge veier. **Identisk kode på alle enheter**, ingen
rollevalg. Den eneste forskjellen mellom L2 og L3 er hvordan meldingen lages (knapp/serial i
L2, potmeter i L3) — radiooppsettet er helt likt.

```
        PIPE 1 = "1SAMT"
  ┌───────────────────────────────────┐
  │  Gruppe 0  ◄────────►  Gruppe 1   │   alle lytter OG sender på "1SAMT"
  └───────────────────────────────────┘
        PIPE 2 = "2SAMT"
  ┌───────────────────────────────────┐
  │  Gruppe 2  ◄────────►  Gruppe 3   │
  └───────────────────────────────────┘
        ... osv. opp til PIPE 5
```

### Lærerhuben hører ALT samtidig

Huben utnytter at NRF24 kan lytte på 6 pipes på én gang:

```
  Pipe 0:  "HUB00"   (Leksjon 1-pings,  Auto-ACK PÅ)
  Pipe 1:  "1SAMT"   (samtale 1,        Auto-ACK AV)
  Pipe 2:  "2SAMT"   (samtale 2,        Auto-ACK AV)
  Pipe 3:  "3SAMT"   (samtale 3,        Auto-ACK AV)
  Pipe 4:  "4SAMT"   (samtale 4,        Auto-ACK AV)
  Pipe 5:  "5SAMT"   (samtale 5,        Auto-ACK AV)
```

Auto-ACK settes **per pipe**: på for pipe 0 (så Leksjon 1 fungerer som før), av for pipe 1–5.
Slik kan én og samme hub-sketch betjene alle tre leksjoner, og under L2/L3 viser Serial
Monitor all chat på storskjerm — merket med hvilken samtale (pipe) den kom fra.

> **NRF24-regelen bak adressevalget:** Pipe 0 og pipe 1 kan ha vilkårlige 5-byte-adresser.
> Pipe 2–5 arver de fire øverste bytene fra pipe 1 og kan bare variere den nederste byten.
> Derfor er samtale-adressene `"1SAMT"`, `"2SAMT"`, … der bare det første tegnet (nederste
> byte) skiller dem. Pipe 1 setter mønsteret `"?SAMT"`, og pipe 2–5 fyller inn sitt siffer.

---

## 7. Potensielle konflikter

### To par med samme PIPE

Dette er den nye "klassiske feilen". Hvis to ulike par begge får `PIPE 1`, havner alle fire
gruppene i samme samtale, og meldingene blandes. Løsning: gi hvert par sin egen PIPE (1–5).
Skriv gjerne opp på tavla hvilket par som har hvilken pipe.

### Gruppenummer/navn er bare etiketter

To grupper kan i prinsippet ha samme `GRUPPE`-nummer eller `NAVN` uten at radioen bryter
sammen — det blir bare forvirrende å lese hvem som skrev hva. Be elevene velge unike
etiketter, men det er **PIPE** som er kritisk for hvem som hører hvem.

### Blande leksjoner i samme rom

Leksjon 1 bruker `"HUB00"`. Leksjon 2/3 bruker `"{P}SAMT"`. Disse er forskjellige adresser,
så det er trygt å kjøre blandet — så lenge to L2/L3-par ikke deler PIPE ved et uhell.

---

## 8. Rask oppslagstabell

| Hva | Verdi |
|-----|-------|
| Kanal | 80 (= 2480 MHz) |
| Datahastighet | RF24_250KBPS |
| Sendestyrke | RF24_PA_HIGH |
| Auto-ACK, Leksjon 1 | PÅ (hub pipe 0 / `"HUB00"`) |
| Auto-ACK, Leksjon 2/3 | AV (delt pipe, flere lyttere) |
| Hub-adresse (L1) | `"HUB00"` |
| Samtale-adresse (L2/L3) | `"{PIPE}SAMT"`, PIPE = 1–5 |
| Elev-pipe (L2/L3) | Pipe 1 (lytter + sender på samme adresse) |
| Hub-pipes | Pipe 0 = HUB00, pipe 1–5 = samtale 1–5 |
| Adresselengde | 5 bytes (ASCII-tegn) |
| Maks samtaler samtidig | 5 (pipe 1–5) → 10 grupper i par |

---

## 9. Kodeknagger — der adressen faktisk settes

**LaererHub (alle leksjoner):**
```c
const byte hubAdresse[6] = "HUB00";
radio.setAutoAck(false);                 // standard av
radio.openReadingPipe(0, hubAdresse);    // Leksjon 1
radio.setAutoAck(0, true);               // ...men L1 trenger ACK
for (int i = 0; i < 5; i++) {            // Leksjon 2/3: "1SAMT".."5SAMT"
  snprintf((char*)samtaleAdr[i], 6, "%dSAMT", i + 1);
  radio.openReadingPipe(i + 1, samtaleAdr[i]);
}
```

**Leksjon 2 og 3 (elev-enhet):**
```c
snprintf((char*)samtaleAdresse, 6, "%dSAMT", PIPE);  // f.eks. "1SAMT"
radio.setAutoAck(false);
radio.openReadingPipe(1, samtaleAdresse);
radio.openWritingPipe(samtaleAdresse);
```
