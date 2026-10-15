/*
 * ╔══════════════════════════════════════════════════╗
 * ║   LEKSJON 2 OG 3 – NØDRADIO                      ║
 * ╚══════════════════════════════════════════════════╝
 *
 * Send tekstmeldinger til partneren din. Du kan skrive på to måter,
 * og begge virker hele tiden:
 *
 *   Leksjon 2: Serial Monitor. Skriv tekst + Enter.
 *   Leksjon 3: Potmeter og knapper. Ingen PC nødvendig.
 *
 *   Potmeter = velg tegn
 *   Knapp A  = legg til tegnet
 *              (velg < helt til venstre for å slette siste tegn)
 *   Knapp B  = send meldingen
 *              (uten egen melding: vis siste mottatte en gang til)
 *
 * Alle bokser hører alle meldinger. Boksen din viser bare dem
 * som er til deg. HUB hører alt og viser det på tavla.
 *
 * I Serial Monitor kan du også sende til en annen enn partneren:
 *   @navn Hei på deg       sender til boksen med det kallenavnet
 *   @alle Hei alle         sender til alle
 *
 * Oppkobling: som Leksjon 1, pluss potmeter
 *   Midtben ->A0    Ytterben ->5V og GND
 */

// ════════════════════════════════════════════════════
//  Endre dette: navnet på din boks, og navnet på partneren.
//  Store og små bokstaver spiller ingen rolle.
//  ALFA og BRAVO er bare eksempler og finnes ikke i navnelista.
//  Bytt dem med kallenavnene dere har fått.
// ════════════════════════════════════════════════════
#define KALLENAVN "ALFA"
#define PARTNER   "BRAVO"
// ════════════════════════════════════════════════════

#include <ArduinoGraphics.h>   // MÅ komme FØR Arduino_LED_Matrix.h
#include "Arduino_LED_Matrix.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN     9
#define CSN_PIN    10
#define BUZZER_PIN 3
#define KNAPP_A    2
#define KNAPP_B    4
#define POT_PIN    A0

RF24 radio(CE_PIN, CSN_PIN);
ArduinoLEDMatrix matrix;

// ── Navnelista (må være lik i alle TeknaRadio-filer) ──
const char* const NAVN[] = {
  "HAWK", "JET",     // parene står ved siden av hverandre
  "ICE",  "WOLF",
  "ACE",  "PUMA",
  "BOLT", "ORCA",
  "ION",  "FOX",
  "LYNX", "NOVA"
};
const int ANTALL_NAVN = sizeof(NAVN) / sizeof(NAVN[0]);
const char ID_ALLE = '*';
const char ID_HUB  = '#';

// ── Adresser ─────────────────────────────────────────
// Til huben: bare huben lytter her, og den kvitterer (ACK).
const byte hubAdresse[6]    = "HUB00";
// Felles: alle bokser sender og lytter her. Ingen kvitterer.
const byte fellesAdresse[6] = "RADIO";

// ── Pakketyper (første tegn i hver pakke) ────────────
//  Pakke = type + fra + til + tekst. En pakke er maks 32 tegn.
const char TYPE_ONLINE  = 'O';   // "jeg har startet"
const char TYPE_PULS    = 'H';   // "jeg lever fortsatt"
const char TYPE_MELDING = 'M';
const int  SMS_MAKS     = 28;    // 32 minus type, fra, til og sluttmerke

// ── Tilstand ─────────────────────────────────────────
char minId, tilId;
bool hubKontakt = false;
const unsigned long PULS_MS = 5000UL;
unsigned long nestePulsMs = 0;

// ── Meldingen du skriver ─────────────────────────────
char smsMelding[SMS_MAKS + 1] = "";
int  smsLen         = 0;
char aktueltTegn    = 'A';

// ── Knapper ──────────────────────────────────────────
unsigned long sistTrykkA = 0;
unsigned long sistTrykkB = 0;
bool varNedeA = false;
bool varNedeB = false;
const unsigned long DEBOUNCE_MS = 50;

// ── Rulletekst ───────────────────────────────────────
// Rulles ett steg om gangen fra loop(), slik at knapper, potmeter
// og radio virker mens teksten går.
const unsigned long RULL_MS     = 80;       // tid per steg
const unsigned long RULL_PAUSE  = 1000UL;   // teksten står stille først
const unsigned long IDLE_SCROLL = 5000UL;   // så lenge uten aktivitet før egen melding rulles
char rullTekst[40] = "";
int  rullX         = 0;
bool ruller        = false;
unsigned long nesteRullMs = 0;
unsigned long nesteIdleMs = 0;

// ── Serial-input ─────────────────────────────────────
const int SERIAL_MAKS = SMS_MAKS + 8;       // plass til "@navn " foran teksten
String serialBuffer  = "";

// ── Siste mottatte melding ("AVSENDER: tekst") ───────
char innkommende[40] = "";

// ── Tegnsett med frekvensbaserte vekter ──────────────
// '<' helt til venstre er ikke et tegn i meldingen: det sletter siste tegn.
const uint8_t N_TEGN = 38;
const char SLETT_TEGN = '<';

const char tegnsett[N_TEGN] = {
  SLETT_TEGN,
  'A','B','C','D','E','F','G','H','I','J','K','L','M',
  'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  ' ',
  '1','2','3','4','5','6','7','8','9',
  ' '
};

// Vekter basert på norsk bokstavfrekvens (kilde: typisk norsk tekstkorpus)
const uint16_t vekter[N_TEGN] = {
  20,
  53,14,4,37,116,15,30,20,50,8,27,45,27,68,39,10,1,71,57,63,15,21,2,1,7,1,
  20,
  18,18,18,18,18,18,18,18,18,
  20
};

// Sjeldne bokstaver (Q, X, Z ...) får minst like mye plass som et tall,
// ellers blir de umulige å treffe med potmeteret.
const uint16_t MIN_VEKT = 18;

// Så mye må potmeteret forbi en grense før tegnet bytter.
// Hindrer flimring når det står midt mellom to tegn.
const int POT_SLINGRING = 3;

uint16_t grenser[N_TEGN];
int aktuellIdx = 0;

// ════════════════════════════════════════════════════
//  Navn
// ════════════════════════════════════════════════════

// Gir plassen i navnelista, eller -1 hvis navnet ikke finnes
int finnNavn(const char* navn) {
  for (int i = 0; i < ANTALL_NAVN; i++) {
    if (strcasecmp(navn, NAVN[i]) == 0) return i;
  }
  return -1;
}

const char* navnFor(char id) {
  if (id == ID_HUB)  return "HUB";
  if (id == ID_ALLE) return "ALLE";
  int i = id - 'A';
  return (i >= 0 && i < ANTALL_NAVN) ? NAVN[i] : "?";
}

void skrivNavneliste() {
  for (int i = 0; i < ANTALL_NAVN; i++) {
    Serial.print("       "); Serial.println(NAVN[i]);
  }
}

// ════════════════════════════════════════════════════
//  Potmeter
// ════════════════════════════════════════════════════
uint16_t vektFor(int i) {
  return (vekter[i] < MIN_VEKT) ? MIN_VEKT : vekter[i];
}

void byggGrenseverdier() {
  uint32_t total = 0;
  for (int i = 0; i < N_TEGN; i++) total += vektFor(i);
  uint32_t akk = 0;
  for (int i = 0; i < N_TEGN; i++) {
    akk += vektFor(i);
    grenser[i] = (uint16_t)((akk * 1023UL) / total);
  }
}

// Les tegn fra potmeter (gjennomsnitt av 3 avlesninger)
char lesAvPotmeter() {
  int v = (analogRead(POT_PIN) + analogRead(POT_PIN) + analogRead(POT_PIN)) / 3;

  // Bli på samme tegn så lenge vi er innenfor sonen pluss litt slingring
  int nedre = (aktuellIdx == 0) ? 0 : grenser[aktuellIdx - 1] + 1;
  int ovre  = grenser[aktuellIdx];
  if (v >= nedre - POT_SLINGRING && v <= ovre + POT_SLINGRING) {
    return tegnsett[aktuellIdx];
  }

  aktuellIdx = N_TEGN - 1;
  for (int i = 0; i < N_TEGN; i++) {
    if (v <= grenser[i]) { aktuellIdx = i; break; }
  }
  return tegnsett[aktuellIdx];
}

// ════════════════════════════════════════════════════
//  Matrise og lyd
// ════════════════════════════════════════════════════

// Vis enkelt tegn (5x7-font)
void visKarakter(char tegn) {
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(3, 1, 0xFFFFFF);
  matrix.print(tegn);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

// Vis tre tegn (4x6-font): de to siste i meldingen + tegnet potmeteret står på
void visStatusSMS(char d1, char d2, char d3) {
  if (d1 == ' ') d1 = '_';
  if (d2 == ' ') d2 = '_';
  if (d3 == ' ') d3 = '_';
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(d1);
  matrix.print(d2);
  matrix.print(d3);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

void tegnRull() {
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(rullX, 1, 0xFFFFFF);
  matrix.print(rullTekst);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

void startRull(const char* tekst) {
  strncpy(rullTekst, tekst, sizeof(rullTekst) - 1);
  rullTekst[sizeof(rullTekst) - 1] = '\0';
  rullX = 0;
  ruller = true;
  tegnRull();
  nesteRullMs = millis() + RULL_PAUSE;
}

// Kalles fra loop(): flytter teksten ett steg når det er tid for det
void oppdaterRull() {
  if (!ruller || (long)(millis() - nesteRullMs) < 0) return;
  rullX--;
  if (rullX < -(int)strlen(rullTekst) * 5) {      // hele teksten er ute av bildet
    gjenopprettSMSDisplay();
    return;
  }
  tegnRull();
  nesteRullMs = millis() + RULL_MS;
}

void pip(int antall) {
  for (int i = 0; i < antall; i++) {
    tone(BUZZER_PIN, 1200, 120);
    delay(220);
  }
}

// Gir true én gang per trykk, også om knappen holdes inne
bool trykket(int pin, bool &varNede, unsigned long &sistTrykk) {
  bool nede = (digitalRead(pin) == LOW);
  bool nyttTrykk = nede && !varNede && (millis() - sistTrykk) > DEBOUNCE_MS;
  if (nyttTrykk) sistTrykk = millis();
  varNede = nede;
  return nyttTrykk;
}

// Tilbake til skrivevisningen. Stopper eventuell rulletekst.
void gjenopprettSMSDisplay() {
  ruller = false;
  nesteIdleMs = millis() + IDLE_SCROLL;
  char d1 = (smsLen >= 2) ? smsMelding[smsLen - 2] : ' ';
  char d2 = (smsLen >= 1) ? smsMelding[smsLen - 1] : ' ';
  visStatusSMS(d1, d2, aktueltTegn);
}

// ════════════════════════════════════════════════════
//  Radio
// ════════════════════════════════════════════════════

// Starter radioen og setter alle innstillinger. Returnerer true hvis
// chipen faktisk svarer. (isChipConnected() er upålitelig via HW-200.)
bool startRadio() {
  bool ok = radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(80);        // 2480 MHz, unngår vanligste WiFi-kanaler
  radio.setRetries(5, 3);
  radio.openReadingPipe(1, fellesAdresse);
  radio.setAutoAck(1, false);             // mange lytter her, ingen kvitterer
  radio.startListening();
  return ok;
}

// Sender til huben. Huben kvitterer, så vi får vite om den hørte oss.
// Vi sier også hvem partneren er, så tavla vet hvilke bokser som hører sammen.
bool sendTilHub(char type) {
  char pakke[4] = { type, minId, tilId, '\0' };
  radio.stopListening();
  radio.openWritingPipe(hubAdresse);
  radio.setAutoAck(0, true);
  bool hort = radio.write(pakke, sizeof(pakke));
  radio.startListening();
  return hort;
}

// Sender en melding på fellesadressen. Alle hører den, ingen kvitterer.
void sendSMS(char til, const char* tekst) {
  char pakke[32];
  snprintf(pakke, sizeof(pakke), "%c%c%c%s", TYPE_MELDING, minId, til, tekst);
  radio.stopListening();
  radio.openWritingPipe(fellesAdresse);
  radio.setAutoAck(0, false);
  radio.write(pakke, strlen(pakke) + 1);
  radio.startListening();

  Serial.print("[Sendt til "); Serial.print(navnFor(til));
  Serial.print("] "); Serial.println(pakke + 3);
}

void visHubKontakt() {
  if (hubKontakt) {
    Serial.println("HUB hører deg! (ACK mottatt)");
    visKarakter('K');
    pip(2);
  } else {
    Serial.println("Ingen kontakt med HUB.");
    visKarakter('?');
    pip(3);
  }
}

// Forteller huben at vi lever, hvert femte sekund
void sendPuls() {
  if ((long)(millis() - nestePulsMs) < 0) return;
  nestePulsMs = millis() + PULS_MS;

  bool hort = sendTilHub(TYPE_PULS);
  if (hort != hubKontakt) {      // si bare fra når noe endrer seg
    hubKontakt = hort;
    visHubKontakt();
    delay(1000);
    gjenopprettSMSDisplay();
  }
}

void sjekkRadio() {
  if (!radio.available()) return;

  char pakke[33] = { 0 };
  radio.read(pakke, 32);
  char type = pakke[0];
  char fra  = pakke[1];
  char til  = pakke[2];

  // Alle hører alt. Vi viser bare meldinger som er til oss.
  if (type != TYPE_MELDING) return;
  if (til != minId && til != ID_ALLE) return;

  snprintf(innkommende, sizeof(innkommende), "%s: %s", navnFor(fra), pakke + 3);
  Serial.print("[Mottatt] ");
  Serial.println(innkommende);
  pip(1);
  startRull(innkommende);
}

// ════════════════════════════════════════════════════
//  Serial Monitor (valgfritt, krever PC)
// ════════════════════════════════════════════════════

// Én ferdig linje fra Serial Monitor: "tekst" eller "@navn tekst"
void sendFraSerial(String linje) {
  char til = tilId;

  if (linje.startsWith("@")) {
    int mellomrom = linje.indexOf(' ');
    String navn = (mellomrom < 0) ? linje.substring(1) : linje.substring(1, mellomrom);
    linje = (mellomrom < 0) ? "" : linje.substring(mellomrom + 1);
    linje.trim();

    int funnet = finnNavn(navn.c_str());
    if (navn.equalsIgnoreCase("alle")) {
      til = ID_ALLE;
    } else if (funnet >= 0) {
      til = 'A' + funnet;
    } else {
      Serial.print("(Kjenner ikke navnet "); Serial.print(navn);
      Serial.println(". Velg ett av disse, eller alle:)");
      skrivNavneliste();
      return;
    }
  }

  if (linje.length() == 0) return;
  if ((int)linje.length() > SMS_MAKS) {
    linje = linje.substring(0, SMS_MAKS);
    Serial.println("(Meldingen var for lang og ble kuttet)");
  }
  sendSMS(til, linje.c_str());
  pip(1);
}

void sjekkSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      serialBuffer.trim();
      if (serialBuffer.length() > 0) sendFraSerial(serialBuffer);
      serialBuffer = "";
    } else if ((int)serialBuffer.length() < SERIAL_MAKS) {
      serialBuffer += c;
    }
  }
}

// ════════════════════════════════════════════════════
//  setup()
// ════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(KNAPP_A, INPUT_PULLUP);
  pinMode(KNAPP_B, INPUT_PULLUP);
  matrix.begin();
  matrix.beginDraw(); matrix.clear(); matrix.endDraw();

  byggGrenseverdier();

  // 1. Finnes navnene i lista?
  int jeg = finnNavn(KALLENAVN);
  int til = finnNavn(PARTNER);
  while (jeg < 0 || til < 0 || jeg == til) {
    Serial.println("[FEIL] Sjekk KALLENAVN og PARTNER øverst i koden.");
    Serial.println("       De må være to ulike navn fra denne lista:");
    skrivNavneliste();
    visKarakter('N');
    pip(2);
    delay(3000);
  }
  minId = 'A' + jeg;
  tilId = 'A' + til;

  Serial.println("================================");
  Serial.println("  TeknaRadio – Nødradio");
  Serial.print  ("  Kallenavn: ");    Serial.println(NAVN[jeg]);
  Serial.print  ("  Partner: ");   Serial.println(NAVN[til]);
  Serial.println("================================");
  Serial.println("  Skriv tekst + Enter for å sende");
  Serial.println("  Potmeter = velg tegn (< sletter)");
  Serial.println("  Knapp A  = legg til tegn");
  Serial.println("  Knapp B  = send / vis siste melding");
  Serial.println("================================");
  Serial.println();

  // 2. Svarer radioen? Prøver på nytt til kablene er riktige.
  bool forsteForsok = true;
  while (!startRadio()) {
    if (forsteForsok) {
      Serial.println("Fant ikke radioen. Sjekk kablingen. Prøver igjen hvert sekund...");
      pip(2);
      forsteForsok = false;
    }
    visKarakter('X');
    delay(1000);
  }
  Serial.println("Radio funnet!");
  visKarakter('V');
  pip(1);
  delay(800);

  // 3. Hører HUB oss?
  hubKontakt = sendTilHub(TYPE_ONLINE);
  visHubKontakt();
  delay(1200);

  // Litt ulik takt per boks, så ikke alle melder seg samtidig
  nestePulsMs = millis() + PULS_MS + (minId - 'A') * 40;

  // Start rett i skrivemodus
  aktueltTegn = lesAvPotmeter();
  gjenopprettSMSDisplay();
}

// ════════════════════════════════════════════════════
//  loop()
// ════════════════════════════════════════════════════
void loop() {
  sjekkRadio();
  sendPuls();
  sjekkSerial();

  // ── Potmeter: oppdater aktuelt tegn ───────────────
  char nyTegn = lesAvPotmeter();
  if (nyTegn != aktueltTegn) {
    aktueltTegn = nyTegn;
    gjenopprettSMSDisplay();
  }

  // ── Rulletekst ───────────────────────────────────
  oppdaterRull();
  if (smsLen > 0 && !ruller && (long)(millis() - nesteIdleMs) >= 0) {
    startRull(smsMelding);       // vis hele meldingen du holder på med
  }

  // ── Knapp A: legg til tegn (eller slett med <) ────
  if (trykket(KNAPP_A, varNedeA, sistTrykkA)) {
    if (aktueltTegn == SLETT_TEGN) {
      if (smsLen > 0) smsMelding[--smsLen] = '\0';
      tone(BUZZER_PIN, 600, 120);
      Serial.print("[Melding] ");
      Serial.println(smsMelding);
    } else if (smsLen < SMS_MAKS) {
      smsMelding[smsLen++] = aktueltTegn;
      smsMelding[smsLen]   = '\0';
      pip(1);
      Serial.print("[Melding] ");
      Serial.println(smsMelding);
    } else {
      Serial.println("(Melding full – send med Knapp B)");
      pip(2);
    }
    gjenopprettSMSDisplay();
  }

  // ── Knapp B: send, eller vis siste mottatte ───────
  if (trykket(KNAPP_B, varNedeB, sistTrykkB)) {
    if (smsLen > 0) {
      sendSMS(tilId, smsMelding);
      pip(2);
      visKarakter('>');
      delay(600);
      smsMelding[0] = '\0';
      smsLen = 0;
      gjenopprettSMSDisplay();
    } else if (innkommende[0] != '\0') {
      startRull(innkommende);
    } else {
      Serial.println("(Ingen melding ennå. Legg til tegn med Knapp A først.)");
      pip(2);
      visKarakter('?');
      delay(600);
      gjenopprettSMSDisplay();
    }
  }
}
