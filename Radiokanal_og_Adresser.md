# Radiokanal og adresser — notat for kursholderen

> Forklarer det tekniske fundamentet i TeknaRadio: hva kanal, adresse og pipe betyr, hvorfor
> det finnes nøyaktig to adresser, og hvordan én boks vet hva som er til den. Ingen
> steg-for-steg-instruksjoner, bare forklaringer. Gjelder opplegget fra 06.10.2026
> (`Leksjon1_RadioTest.ino`, `Leksjon2_Nodradio.ino`, `Hub/Hub.ino`). Den forrige versjonen,
> med én pipe per par, ligger i `gammelt/` og i git-historikken.

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
WiFi-kanalene. Det vil alltid være WiFi-trafikk i rommet, og den skal ikke forstyrre boksene.

Andre innstillinger som hjelper:

- **RF24_250KBPS** — lav datahastighet gir bedre sensitivitet og rekkevidde. Tregere er
  faktisk bedre her.
- **RF24_PA_HIGH** — høy sendestyrke. I ett rom er det mer enn nok, men det gir jevnere
  forbindelse.

---

## 2. Hva er en radioadresse?

Kanalen bestemmer hvem som *kan* høre hverandre. Adressen bestemmer hvilke pakker en radio
faktisk tar imot.

Analogier:
- **Kanal** = postnummer (du er i riktig by)
- **Adresse** = gatenavn og nummer (pakken havner i riktig postkasse)

Teknisk sett er en RF24-adresse **5 bytes**. I koden bruker vi lesbare ASCII-strenger:

```c
const byte hubAdresse[6] = "HUB00";
//                          H U B 0 0  <- 5 tegn = 5 bytes
//                          + 1 null-byte fra C-streng-konvensjon
```

Biblioteket bruker bare de første 5 bytene. Den sjette (null-tegnet) er der fordi C-strenger
alltid avsluttes med `\0`.

### De to adressene i TeknaRadio

| Adresse | Hvem sender | Hvem lytter | Kvittering (ACK) | Brukes til |
|---------|-------------|-------------|------------------|------------|
| `"HUB00"` | Boksene | Bare huben | Ja | Oppstart (`O`) og livstegn (`H`). Kvitteringen gir `K`. |
| `"RADIO"` | Alle, også huben | Alle | Nei | Tenn (`T`), slukk (`S`) og meldinger (`M`) |

Det finnes ingen adresse per par. Alle bokser lytter på `"RADIO"` og hører alt som sendes der.
Hvem en pakke er til, står i selve pakken (punkt 4).

---

## 3. Hva er en pipe?

NRF24L01 kan lytte på **opptil 6 adresser samtidig**. Disse lytteplassene kalles **pipes**
(rør). Tenk på det som seks postkasser på samme vegg.

```
Pipe 0   – Full 5-byte-adresse. Brukes også internt til å ta imot ACK ved sending.
Pipe 1   – Full 5-byte-adresse.
Pipe 2–5 – Deler de fire øverste bytene med pipe 1; bare nederste byte er fri.
```

TeknaRadio bruker bare to:

| | Pipe 0 | Pipe 1 |
|---|---|---|
| **Boks** | (bare mens den venter på ACK fra huben) | `"RADIO"`, Auto-ACK av |
| **Hub** | `"HUB00"`, Auto-ACK på | `"RADIO"`, Auto-ACK av |

Siden ingen trenger én pipe per par, er antall bokser ikke begrenset av radioens seks
lytteplasser. Grensen er navnelista (punkt 4).

---

## 4. Pakken, og hvordan en boks vet hva som er til den

Hver pakke ser slik ut:

```
┌──────┬─────┬─────┬──────────────────┬───────┐
│ type │ fra │ til │ tekst (0–28 tegn) │ slutt │
└──────┴─────┴─────┴──────────────────┴───────┘
  1 tegn 1 tegn 1 tegn                  1 tegn   = maks 32 bytes (radioens grense)
```

| Type | Betyr | Adresse | «til»-feltet |
|------|-------|---------|--------------|
| `O` | Jeg har startet | `HUB00` | Boksens partner |
| `H` | Jeg lever fortsatt (hvert 5. sekund) | `HUB00` | Boksens partner |
| `T` | Tenn skjermen | `RADIO` | Mottaker |
| `S` | Slukk skjermen | `RADIO` | Mottaker |
| `M` | Tekstmelding | `RADIO` | Mottaker, eller `*` for alle |

**«fra» og «til» er én bokstav:** `'A'` + plassen i navnelista. HAWK er `A`, JET er `B`, og så
videre. `*` betyr alle, `#` betyr huben.

**Navnelista** står øverst i alle tre `.ino`-filene og må være lik i alle:

```c
const char* const NAVN[] = {
  "HAWK", "JET",     // parene står ved siden av hverandre
  "ICE",  "WOLF",
  "ACE",  "PUMA",
  "BOLT", "ORCA",
  "ION",  "FOX",
  "LYNX", "NOVA"
};
```

Deltakerne setter `#define KALLENAVN` og `#define PARTNER` øverst i boksfilen. Koden slår opp
begge navnene i lista ved oppstart (store og små bokstaver spiller ingen rolle). Finnes ett av
dem ikke, eller er de like, stopper boksen og viser `NAVN?` (Leksjon 1) eller `N` (Leksjon 2/3),
og skriver lista i Serial Monitor. Standardverdiene `"ALFA"` og `"BRAVO"` finnes bevisst ikke i
lista, så en boks som ikke er satt opp, sier fra i stedet for å bli en kopi av en annen.

**Filteret i boksen** er én linje:

```c
if (til != minId && til != ID_ALLE) return;   // ikke til meg: ignorer
```

Alle bokser hører alle pakker på `"RADIO"`. De viser bare dem som er til seg selv eller til alle.
Det er dette refleksjonsspørsmål 1 i Leksjon 2 handler om. Og fordi huben hører alt, viser
tavla alle meldingene i rommet: det er refleksjonsspørsmål 2.

**Partneren i livstegnet.** I `O`- og `H`-pakkene er «til» boksens partner, ikke en mottaker.
Slik får huben (og tavla) vite hvilke bokser som hører sammen, uten at kursholderen setter opp
parene noe sted.

---

## 5. Half-duplex og Auto-ACK

### Half-duplex

NRF24L01 kan **ikke sende og motta samtidig**. Sekvensen for å sende er alltid:

1. `stopListening()` — skrur av mottakeren
2. `write(data, lengde)` — sender pakken
3. `startListening()` — skrur mottakeren på igjen

Uten det siste steget hører boksen aldri noe mer.

### Auto-ACK: på til huben, av på fellesadressen

Auto-ACK er en funksjon der mottakeren automatisk sender en liten kvitteringspakke (**ACK**)
tilbake, helt uten at programmet gjør noe. `radio.write()` returnerer da `true` hvis ACK kom
innen tidsfristen.

**Til `"HUB00"` (Auto-ACK PÅ):** Bare huben lytter der, så bare én kan kvittere.
`write()` gir `true` når huben hørte boksen, og det er det som gir `K`. Livstegnet hvert femte
sekund bruker samme mekanisme: blir kvitteringen borte, viser boksen `?`, og når den kommer
tilbake, `K` igjen.

Boksene må **aldri** lytte på `"HUB00"`. Gjorde de det, ville de kvittert for hverandres
livstegn, og `K` ville stått selv med huben slått av.

**Til `"RADIO"` (Auto-ACK AV):** Her lytter alle. Hadde Auto-ACK vært på, ville alle prøvd å
kvittere samtidig, og kvitteringene ville kollidert. Derfor:

```c
radio.setAutoAck(1, false);   // lytting på RADIO: ingen kvitterer
radio.setAutoAck(0, false);   // før sending til RADIO
radio.setAutoAck(0, true);    // før sending til HUB00
```

Konsekvensen: Når en boks tenner partnerens skjerm eller sender en melding, vet den ikke om
pakken kom fram. Det er spørsmålet i refleksjonen til Leksjon 1, og grunnen til at `>` på
skjermen betyr «sendt», ikke «mottatt».

`setRetries(5, 3)` gjelder bare når Auto-ACK er på, altså sendingene til huben.

---

## 6. Topologi

```
                      "RADIO"  (alle hører alt, ingen kvitterer)
   ┌──────────────────────────────────────────────────────────────┐
   │   FOX ◄──► ION        ICE ◄──► WOLF        HAWK ◄──► JET  …  │
   └──────────────────────────────────────────────────────────────┘
        │  │                 │  │                  │  │
        ▼  ▼                 ▼  ▼                  ▼  ▼
                      "HUB00"  (bare huben lytter, kvitterer)
                               ┌──────┐
                               │ HUB  │──USB──► server.py ──► tavla
                               └──────┘
```

- Alle bokser sender livstegn til `"HUB00"` og får kvittering.
- Alt annet går på `"RADIO"`. Huben lytter der også, og hører derfor alle meldinger.
- Huben sender selv på `"RADIO"`, med `#` som avsender. Vanlig tekst i Serial Monitor går til
  alle (`*`), `SEND|navn|tekst` til én boks.

Begge boksfilene bruker nøyaktig samme radiooppsett. Forskjellen mellom leksjonene er bare hva
boksen gjør med knappene og hvilke pakketyper den bryr seg om.

---

## 7. Potensielle konflikter

### To bokser med samme kallenavn

Får to bokser samme `KALLENAVN`, får de samme bokstav i «fra»-feltet. Begge viser meldinger til
det navnet, og tavla slår dem sammen til én rad. Radioen bryter ikke sammen, men det blir umulig
å se hvem som er hvem. Kallenavnlappene er det som hindrer dette.

### PARTNER peker på feil navn

Boksen fungerer, men tenn, slukk og meldinger havner hos en annen boks (eller ingen). På tavla
mangler haken under «Godkjent kallenavn» for begge. Se linja `Partner:` i Serial Monitor på
begge boksene.

### Navnelista ulik i filene

Bokstaven `'A' + plass` betyr bare det samme hvis lista er lik overalt. Endres lista i én fil,
må den endres i alle tre. Tavla henter lista fra hubens `NAVN|…`-linje og har ingen egen kopi.

### Blande leksjoner i samme rom

Trygt. Begge boksfilene bruker samme adresser og samme pakke. En Leksjon 1-boks kan ikke sende
meldinger, men viser dem den får som rulletekst. En Leksjon 2-boks ignorerer tenn og slukk.

---

## 8. Rask oppslagstabell

| Hva | Verdi |
|-----|-------|
| Kanal | 80 (= 2480 MHz) |
| Datahastighet | RF24_250KBPS |
| Sendestyrke | RF24_PA_HIGH |
| Adresser | `"HUB00"` (til huben, ACK på) og `"RADIO"` (felles, ACK av) |
| Boks lytter på | Pipe 1 = `"RADIO"` |
| Hub lytter på | Pipe 0 = `"HUB00"`, pipe 1 = `"RADIO"` |
| Pakke | type + fra + til + tekst, maks 32 bytes |
| Tekst per melding | Maks 28 tegn |
| Livstegn | Hvert 5. sekund. Tavla viser «ikke på nett» etter 15 s uten |
| Antall bokser | Så mange som det er navn i lista (12 per i dag) |

---

## 9. Der adressene faktisk settes

**Boksene (`startRadio()`, likt i begge filene):**
```c
radio.openReadingPipe(1, fellesAdresse);   // "RADIO"
radio.setAutoAck(1, false);
radio.startListening();
```

**Boksene, ved sending:**
```c
// til huben (sendTilHub)
radio.openWritingPipe(hubAdresse);         // "HUB00"
radio.setAutoAck(0, true);
bool hort = radio.write(pakke, sizeof(pakke));

// til alle (sendTilAlle / sendSMS)
radio.openWritingPipe(fellesAdresse);      // "RADIO"
radio.setAutoAck(0, false);
radio.write(pakke, ...);
```

**Huben (`setup()`):**
```c
radio.setAutoAck(false);
radio.openReadingPipe(0, hubAdresse);      // "HUB00"
radio.setAutoAck(0, true);                 // boksene venter på kvittering her
radio.openReadingPipe(1, fellesAdresse);   // "RADIO"
radio.startListening();
```
