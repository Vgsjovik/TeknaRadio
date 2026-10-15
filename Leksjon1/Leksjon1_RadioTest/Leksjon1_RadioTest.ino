/*
 * ╔══════════════════════════════════════════════════╗
 * ║   LEKSJON 1 – Bygg og test                       ║
 * ╚══════════════════════════════════════════════════╝
 *
 * Mål: Sjekk at radioen er riktig koblet, meld deg på nett
 *      hos HUB, og styr skjermen til partneren din.
 *
 * Koden kjører ferdig. Du trenger bare å sette de to navnene under.
 *
 * Oppkobling:
 *   VCC  ->5V       GND  ->GND
 *   CE   ->Pin 9    CSN  ->Pin 10
 *   SCK  ->Pin 13   MOSI ->Pin 11   MISO ->Pin 12
 *   Summer (+) ->Pin 3    Summer (-) ->GND
 *   Knapp A    ->Pin 2    (andre ben til GND)
 *   Knapp B    ->Pin 4    (andre ben til GND)
 *
 * Ved oppstart:
 *   V = radioen svarer        X = radioen svarer ikke (sjekk kablene)
 *   K = HUB hører deg         ? = ingen kontakt med HUB
 *
 * Hva gjør knappene?
 *   Knapp A = tenn skjermen til partneren
 *   Knapp B = slukk skjermen til partneren
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

// ── Pinner ────────────────────────────────────────────────
#define CE_PIN      9
#define CSN_PIN     10
#define BUZZER_PIN  3
#define KNAPP_A     2
#define KNAPP_B     4

// ── Objekter ──────────────────────────────────────────────
RF24 radio(CE_PIN, CSN_PIN);
ArduinoLEDMatrix matrix;

// ── Navnelista (må være lik i alle TeknaRadio-filer) ──────
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

// ── Adresser ──────────────────────────────────────────────
// Til huben: bare huben lytter her, og den kvitterer (ACK).
const byte hubAdresse[6]    = "HUB00";
// Felles: alle bokser sender og lytter her. Ingen kvitterer.
const byte fellesAdresse[6] = "RADIO";

// ── Pakketyper (første tegn i hver pakke) ─────────────────
//  Pakke = type + fra + til + eventuell tekst
const char TYPE_ONLINE  = 'O';   // "jeg har startet"
const char TYPE_PULS    = 'H';   // "jeg lever fortsatt"
const char TYPE_TENN    = 'T';
const char TYPE_SLUKK   = 'S';
const char TYPE_MELDING = 'M';

// ── Tilstand ──────────────────────────────────────────────
char minId, tilId;
bool hubKontakt = false;
bool skjermPaa  = false;
const unsigned long PULS_MS = 5000UL;
unsigned long nestePulsMs = 0;

// ── Skjermrammer ──────────────────────────────────────────
const uint32_t FULL[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
const uint32_t TOM[3]  = { 0x00000000, 0x00000000, 0x00000000 };

// ── Knapper ───────────────────────────────────────────────
unsigned long sistTrykkA = 0;
unsigned long sistTrykkB = 0;
bool varNedeA = false;
bool varNedeB = false;
const unsigned long DEBOUNCE_MS = 50;

// ─────────────────────────────────────────────────────────
// Navn
// ─────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────
// Hjelpefunksjoner
// ─────────────────────────────────────────────────────────

// Gir true én gang per trykk, også om knappen holdes inne
bool trykket(int pin, bool &varNede, unsigned long &sistTrykk) {
  bool nede = (digitalRead(pin) == LOW);
  bool nyttTrykk = nede && !varNede && (millis() - sistTrykk) > DEBOUNCE_MS;
  if (nyttTrykk) sistTrykk = millis();
  varNede = nede;
  return nyttTrykk;
}

void visKarakter(char tegn) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(3, 1, 0xFFFFFF);
  matrix.print(tegn);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

// Viser skjermen slik partneren sist satte den (tent eller slukket)
void visSkjerm() {
  matrix.loadFrame(skjermPaa ? FULL : TOM);
}

void pip(int antall) {
  for (int i = 0; i < antall; i++) {
    tone(BUZZER_PIN, 1200, 120);
    delay(220);
  }
}

void melodiOpp() {   // stigende
  tone(BUZZER_PIN, 262, 150); delay(200);  // C4
  tone(BUZZER_PIN, 330, 150); delay(200);  // E4
  tone(BUZZER_PIN, 392, 150); delay(200);  // G4
}

void melodiNed() {   // synkende
  tone(BUZZER_PIN, 392, 150); delay(200);  // G4
  tone(BUZZER_PIN, 330, 150); delay(200);  // E4
  tone(BUZZER_PIN, 262, 150); delay(200);  // C4
}

void rullTekst(const char* tekst) {
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(80);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(tekst);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

// ─────────────────────────────────────────────────────────
// Radio
// ─────────────────────────────────────────────────────────

// Starter radioen og setter alle innstillinger. Returnerer true hvis
// chipen faktisk svarer. (isChipConnected() er upålitelig via HW-200.)
bool startRadio() {
  bool ok = radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  // Kanal 80 = 2480 MHz. Øvre del av 2.4 GHz-båndet, unngår de vanligste WiFi-kanalene
  radio.setChannel(80);
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

// Sender på fellesadressen. Alle hører det, ingen kvitterer.
void sendTilAlle(char type, char til) {
  char pakke[4] = { type, minId, til, '\0' };
  radio.stopListening();
  radio.openWritingPipe(fellesAdresse);
  radio.setAutoAck(0, false);
  radio.write(pakke, sizeof(pakke));
  radio.startListening();
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
    visSkjerm();
  }
}

void sjekkRadio() {
  if (!radio.available()) return;

  char pakke[33] = { 0 };
  radio.read(pakke, 32);
  char type = pakke[0];
  char fra  = pakke[1];
  char til  = pakke[2];
  const char* tekst = pakke + 3;

  // Alle hører alt. Vi bryr oss bare om det som er til oss.
  if (til != minId && til != ID_ALLE) return;

  if (type == TYPE_TENN) {
    skjermPaa = true;
    visSkjerm();
    melodiOpp();
    Serial.print("[Mottatt] "); Serial.print(navnFor(fra));
    Serial.println(" tente skjermen din");
  }
  else if (type == TYPE_SLUKK) {
    skjermPaa = false;
    visSkjerm();
    melodiNed();
    Serial.print("[Mottatt] "); Serial.print(navnFor(fra));
    Serial.println(" slukket skjermen din");
  }
  else if (type == TYPE_MELDING) {
    Serial.print("[Melding fra "); Serial.print(navnFor(fra));
    Serial.print("] "); Serial.println(tekst);
    pip(1);
    rullTekst(tekst);
    visSkjerm();
  }
}

// ─────────────────────────────────────────────────────────
// Setup – kjører én gang ved oppstart
// ─────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(KNAPP_A, INPUT_PULLUP);
  pinMode(KNAPP_B, INPUT_PULLUP);
  matrix.begin();
  visSkjerm();

  // 1. Finnes navnene i lista?
  int jeg = finnNavn(KALLENAVN);
  int til = finnNavn(PARTNER);
  while (jeg < 0 || til < 0 || jeg == til) {
    Serial.println("[FEIL] Sjekk KALLENAVN og PARTNER øverst i koden.");
    Serial.println("       De må være to ulike navn fra denne lista:");
    for (int i = 0; i < ANTALL_NAVN; i++) {
      Serial.print("       "); Serial.println(NAVN[i]);
    }
    pip(2);
    rullTekst("NAVN?");
  }
  minId = 'A' + jeg;
  tilId = 'A' + til;

  Serial.println("================================");
  Serial.println("  TeknaRadio – Leksjon 1");
  Serial.print  ("  Kallenavn: ");    Serial.println(NAVN[jeg]);
  Serial.print  ("  Partner: ");   Serial.println(NAVN[til]);
  Serial.println("================================");
  Serial.println("  Knapp A : Tenn partnerens skjerm");
  Serial.println("  Knapp B : Slukk partnerens skjerm");
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
  visSkjerm();

  // Litt ulik takt per boks, så ikke alle melder seg samtidig
  nestePulsMs = millis() + PULS_MS + (minId - 'A') * 40;
}

// ─────────────────────────────────────────────────────────
// Loop – kjører på repeat
// ─────────────────────────────────────────────────────────
void loop() {
  sjekkRadio();
  sendPuls();

  if (trykket(KNAPP_A, varNedeA, sistTrykkA)) {
    tone(BUZZER_PIN, 1800, 40);   // kort klikk: "sendt"
    Serial.print("[Knapp A] Tenner skjermen til "); Serial.println(navnFor(tilId));
    sendTilAlle(TYPE_TENN, tilId);
  }

  if (trykket(KNAPP_B, varNedeB, sistTrykkB)) {
    tone(BUZZER_PIN, 1800, 40);
    Serial.print("[Knapp B] Slukker skjermen til "); Serial.println(navnFor(tilId));
    sendTilAlle(TYPE_SLUKK, tilId);
  }
}
