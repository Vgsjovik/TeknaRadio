/*
 * ╔══════════════════════════════════════════════════╗
 * ║   LEKSJON 3 – SMS-RADIO MED POTMETER             ║
 * ╚══════════════════════════════════════════════════╝
 *
 * Samme radio-oppsett som Leksjon 2: de to gruppene i paret
 * deler EN felles pipe og snakker begge veier. Her skriver du
 * meldinger tegn for tegn med potmeteret.
 *
 *   Potmeter = velg tegn
 *   Knapp A  = legg til tegn
 *   Knapp B  = send melding (vises hos alle paa samme pipe)
 *   Serial   = skriv melding + Enter (valgfritt, krever PC)
 *
 * To innstillinger oeverst:
 *   NAVN = gruppenavnet ditt, vises foran meldingene dine
 *   PIPE = felles samtale-pipe (1–5). BEGGE gruppene i paret
 *          setter SAMME PIPE.
 *
 * Du trenger IKKE PC for aa bruke radioen — potmeter og knapper
 * gjoer alt. Serial Monitor er bare en bonus hvis du har laptop.
 */

// ════════════════════════════════════════════════════
//  Endre dette — gruppenavnet ditt
//  Maks 8 tegn, store bokstaver/tall, ingen mellomrom
//  Eksempel: "ALPHA", "BRAVO", "G1TEAM"
// ════════════════════════════════════════════════════
#define NAVN "ALPHA"

// ════════════════════════════════════════════════════
//  Endre dette — felles samtale-pipe (1–5)
//  Begge gruppene i paret MAA sette samme PIPE.
// ════════════════════════════════════════════════════
#define PIPE 1
const int SMS_MAKS = 30 - (int)sizeof(NAVN);   // maks tegn, tilpasset navnelengde

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

// Felles samtale-adresse — bygges i setup() fra PIPE ("1SAMT" ...)
byte samtaleAdresse[6];

// ── SMS-melding ──────────────────────────────────────
char smsMelding[32] = "";   // buffer (SMS_MAKS + null + margin)
int  smsLen         = 0;
char aktueltTegn    = 'A';
unsigned long sisteBevegelseMs = 0;

// ── Debounce ─────────────────────────────────────────
unsigned long sistTrykkA = 0;
unsigned long sistTrykkB = 0;
const unsigned long DEBOUNCE_MS = 250;

// ── Idle-scroll ───────────────────────────────────────
const unsigned long IDLE_SCROLL = 5000UL;

// ── Serial-input ─────────────────────────────────────
String serialBuffer  = "";
bool serialKuttet    = false;

// ── Innkommende melding ───────────────────────────────
char innkommende[32] = "";

// ── Tegnsett med frekvensbaserte vekter ──────────────
// Mellomrom er lagt til rundt bokstaver og sifre for lettere navigering
const uint8_t N_TEGN = 38;

const char tegnsett[N_TEGN] = {
  ' ',
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

uint16_t grenser[N_TEGN];

// ── Bygg potmeter-grenseverdier for tegnsett ─────────
void byggGrenseverdier() {
  uint32_t total = 0;
  for (int i = 0; i < N_TEGN; i++) total += vekter[i];
  uint32_t akk = 0;
  for (int i = 0; i < N_TEGN; i++) {
    akk += vekter[i];
    grenser[i] = (uint16_t)((akk * 1023UL) / total);
  }
}

// ── Les tegn fra potmeter (gjennomsnitt av 3 avlesninger) ──
char lesAvPotmeter() {
  int v = (analogRead(POT_PIN) + analogRead(POT_PIN) + analogRead(POT_PIN)) / 3;
  for (int i = 0; i < N_TEGN; i++) {
    if (v <= grenser[i]) return tegnsett[i];
  }
  return tegnsett[N_TEGN - 1];
}

// ── Vis enkelt tegn på matrise (5x7-font) ────────────
void visKarakter(char tegn) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(3, 1, 0xFFFFFF);
  matrix.print(tegn);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

// ── Vis tre tegn på matrise (4x6-font, SMS-statusvisning) ──
// d1 = nest siste tegn, d2 = siste tegn, d3 = aktuelt potmeter-tegn
void visStatusSMS(char d1, char d2, char d3) {
  if (d1 == ' ') d1 = '_';
  if (d2 == ' ') d2 = '_';
  if (d3 == ' ') d3 = '_';
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(d1);
  matrix.print(d2);
  matrix.print(d3);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

// ── Tøm matrise ───────────────────────────────────────
void sluttMatrise() {
  matrix.beginDraw();
  matrix.clear();
  matrix.endDraw();
}

// ── Rull tekst over matrise ──────────────────────────
void visRullendeTekst(const char* tekst) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(tekst);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
  delay(1000);
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(80);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(tekst);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

// ── Pip N ganger ─────────────────────────────────────
void pip(int antall) {
  for (int i = 0; i < antall; i++) {
    tone(BUZZER_PIN, 1200, 120);
    delay(220);
  }
}

// ── Gjenopprett SMS-status på matrise ────────────────
void gjenopprettSMSDisplay() {
  char d1 = (smsLen >= 2) ? smsMelding[smsLen - 2] : ' ';
  char d2 = (smsLen >= 1) ? smsMelding[smsLen - 1] : ' ';
  visStatusSMS(d1, d2, aktueltTegn);
}

// ── Bygg og send melding med NAVN-prefiks ────────────
void sendSMS(const char* tekst) {
  char utgaende[32];
  int prefixLen = strlen(NAVN) + 2;       // "NAVN: "
  int meldingMax = 31 - prefixLen;
  char kopi[28];
  strncpy(kopi, tekst, sizeof(kopi));
  kopi[sizeof(kopi) - 1] = '\0';
  if ((int)strlen(kopi) > meldingMax) kopi[meldingMax] = '\0';
  snprintf(utgaende, sizeof(utgaende), "%s: %s", NAVN, kopi);

  radio.stopListening();
  radio.write(utgaende, strlen(utgaende) + 1);
  radio.startListening();

  Serial.print("[Sendt] ");
  Serial.println(utgaende);
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(KNAPP_A, INPUT_PULLUP);
  pinMode(KNAPP_B, INPUT_PULLUP);
  matrix.begin();

  // Render én blank frame slik at scroll-API er initialisert
  sluttMatrise();

  byggGrenseverdier();

  // Bygg den felles samtale-adressen fra PIPE: "1SAMT" ... "5SAMT"
  snprintf((char*)samtaleAdresse, 6, "%dSAMT", PIPE);

  // Start radio — radio.begin() er paalitelig oppstartsjekk (HW-200)
  if (!radio.begin()) {
    Serial.println("[FEIL] Radio ikke funnet! Sjekk kabling og start om.");
    visKarakter('X');
    pip(2);
    while (true) {}
  }
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(80);        // 2480 MHz — unngår vanligste WiFi-kanaler
  radio.setRetries(5, 3);

  // Felles pipe baade for lytting og sending. Auto-ACK av:
  // flere enheter deler adressen og kan ikke alle kvittere.
  radio.setAutoAck(false);
  radio.openReadingPipe(1, samtaleAdresse);
  radio.openWritingPipe(samtaleAdresse);
  radio.startListening();

  Serial.println("================================");
  Serial.println("  TeknaRadio – Leksjon 3");
  Serial.print  ("  Navn: ");  Serial.print(NAVN);
  Serial.print  ("  |  Pipe: "); Serial.println(PIPE);
  Serial.println("================================");
  Serial.println("  Potmeter = velg tegn");
  Serial.println("  Knapp A  = legg til tegn");
  Serial.println("  Knapp B  = send melding");
  Serial.println("  (Serial Monitor + Enter sender ogsaa)");
  Serial.println("================================");
  Serial.println();

  // Start rett i skrivemodus
  aktueltTegn = lesAvPotmeter();
  sisteBevegelseMs = millis();
  gjenopprettSMSDisplay();
}

void loop() {
  unsigned long no = millis();

  // ── Mottak ───────────────────────────────────────
  if (radio.available()) {
    radio.read(innkommende, sizeof(innkommende));
    Serial.print("[Mottatt] ");
    Serial.println(innkommende);
    pip(1);
    visRullendeTekst(innkommende);
    sisteBevegelseMs = millis();
    // scroll kjorer i bakgrunnen – gjenopprettSMSDisplay() trigges naar potmeter beveges
  }

  // ── Potmeter — oppdater aktuelt tegn ──────────────
  char nyTegn = lesAvPotmeter();
  if (nyTegn != aktueltTegn) {
    aktueltTegn = nyTegn;
    sisteBevegelseMs = no;
    gjenopprettSMSDisplay();
  }

  // ── Idle-scroll (5 sek uten aktivitet) ───────────
  if (smsLen > 0 && (no - sisteBevegelseMs) > IDLE_SCROLL) {
    visRullendeTekst(smsMelding);
    sisteBevegelseMs = millis();
  }

  // ── Knapp A: legg til tegn ────────────────────────
  if (digitalRead(KNAPP_A) == LOW && (no - sistTrykkA) > DEBOUNCE_MS) {
    sistTrykkA = no;
    if (smsLen < SMS_MAKS) {
      smsMelding[smsLen++] = aktueltTegn;
      smsMelding[smsLen]   = '\0';
      pip(1);
      sisteBevegelseMs = millis();
      Serial.print("[Melding] ");
      Serial.println(smsMelding);
      gjenopprettSMSDisplay();
    } else {
      Serial.println("(Melding full – send med Knapp B)");
      pip(2);
    }
  }

  // ── Knapp B: send melding ─────────────────────────
  if (digitalRead(KNAPP_B) == LOW && (no - sistTrykkB) > DEBOUNCE_MS) {
    sistTrykkB = no;
    if (smsLen > 0) {
      sendSMS(smsMelding);
      pip(2);
      visKarakter('>');
      delay(600);
      memset(smsMelding, 0, sizeof(smsMelding));
      smsLen = 0;
      sisteBevegelseMs = millis();
      gjenopprettSMSDisplay();
    } else {
      Serial.println("(Ingen melding. Legg til tegn med Knapp A foerst.)");
      pip(2);
      visKarakter('?');
      delay(600);
      gjenopprettSMSDisplay();
    }
  }

  // ── Serial Monitor-input (valgfritt – krever PC) ──
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      serialBuffer.trim();
      if (serialBuffer.length() > 0) {
        if (serialBuffer.length() > 27) {
          serialBuffer = serialBuffer.substring(0, 27);
          Serial.println("(Serial-melding ble trunkert til 27 tegn)");
        }
        char serBuf[28];
        serialBuffer.toCharArray(serBuf, sizeof(serBuf));
        sendSMS(serBuf);
        pip(1);
        sisteBevegelseMs = millis();
      }
      serialBuffer = "";
      serialKuttet = false;
    } else {
      if ((int)serialBuffer.length() < 27) {
        serialBuffer += c;
      } else if (!serialKuttet) {
        serialKuttet = true;
      }
    }
  }
}
