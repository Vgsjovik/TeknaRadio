// LaererHub.ino — Lærers mottaker-enhet (alle tre leksjoner)
// Kobles til PC/projektor via USB — vis Serial Monitor til klassen.
//
// Lytter samtidig på:
//   Pipe 0      : "HUB00"  — Leksjon 1 (pings fra grupper, Auto-ACK PÅ)
//   Pipe 1–5    : "1SAMT".."5SAMT" — Leksjon 2/3 samtale-pipene (Auto-ACK AV)
//
// Slik kan hub-en se ALT som skjer: L1-pings OG all chat mellom gruppepar.
//
// Pinout:
//   CE = 9, CSN = 10, SPI = 11/12/13 (standard)
//   BUZZER = 3

#include <ArduinoGraphics.h>      // MÅ komme FØR Arduino_LED_Matrix.h
#include "Arduino_LED_Matrix.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// --- Pinner ---
#define CE_PIN     9
#define CSN_PIN   10
#define BUZZER     3

// --- Radio ---
RF24 radio(CE_PIN, CSN_PIN);
const byte hubAdresse[6] = "HUB00";   // Leksjon 1

// Samtale-adresser for pipe 1–5. Det varierende tegnet ligger FREMST
// ("1SAMT", "2SAMT", ...) fordi NRF24 lar pipe 2–5 kun variere den
// nederste adressebyten — resten arves fra pipe 1.
byte samtaleAdr[5][6];

// --- LED-matrise ---
ArduinoLEDMatrix matrix;

// --- Serial-input (lærer sender til alle) ---
char serialBuf[32];
int  serialBufPos = 0;

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);

  matrix.begin();
  matrix.beginDraw();
  matrix.clear();
  matrix.endDraw();   // prime-frame før første scroll

  // --- Start radio ---
  if (!radio.begin()) {
    Serial.println("FEIL: Kan ikke starte NRF24L01!");
    visKarakter('X');
    pip(3);
    while (true);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(80);
  radio.setRetries(5, 3);

  // Bygg samtale-adressene: "1SAMT" ... "5SAMT"
  for (int i = 0; i < 5; i++) {
    snprintf((char*)samtaleAdr[i], 6, "%dSAMT", i + 1);
  }

  // Auto-ACK av som standard (samtale-pipene har flere lyttere).
  radio.setAutoAck(false);

  // Pipe 0: Leksjon 1 — her MÅ Auto-ACK være på (elevene venter på ACK).
  radio.openReadingPipe(0, hubAdresse);
  radio.setAutoAck(0, true);

  // Pipe 1–5: Leksjon 2/3 samtale-pipene (Auto-ACK forblir av).
  for (int i = 0; i < 5; i++) {
    radio.openReadingPipe(i + 1, samtaleAdr[i]);
  }

  radio.startListening();

  Serial.println("================================");
  Serial.println("  LÆRER-HUB er klar");
  Serial.println("  Kanal 80 | 250 kbps | PA_HIGH");
  Serial.println("  Lytter: HUB00 + samtale 1–5");
  Serial.println("================================");
  Serial.println("  Venter på trafikk...");
  Serial.println("  Skriv melding + Enter = send til alle");
  Serial.println("================================");

  rullTekst("HUB");
}

// ─────────────────────────────────────────────────────────────────────────────

void sendBroadcast(const char* msg) {
  radio.stopListening();
  radio.setAutoAck(false);         // fire-and-forget for alle sendinger

  radio.openWritingPipe(hubAdresse);
  radio.write(msg, strlen(msg) + 1);
  for (int i = 0; i < 5; i++) {
    radio.openWritingPipe(samtaleAdr[i]);
    radio.write(msg, strlen(msg) + 1);
  }

  radio.setAutoAck(0, true);       // gjenopprett pipe 0 ACK for L1
  radio.startListening();

  Serial.print("[Broadcast] ");
  Serial.println(msg);
  visKarakter('>');
}

void loop() {
  uint8_t pipe = 0;
  if (radio.available(&pipe)) {
    char melding[32] = {0};
    radio.read(&melding, sizeof(melding));

    if (pipe == 0) {
      // Leksjon 1 — ping fra en gruppe
      Serial.print("[Ping] Gruppe ");
      Serial.print(melding + 1);   // hopp over 'G'-prefikset
      Serial.println(" er på nett!");
      pip(1);
      rullTekst(melding);          // celebrasjon (sjelden hendelse)
    } else {
      // Leksjon 2/3 — samtale på pipe 1–5
      Serial.print("[Samtale ");
      Serial.print(pipe);
      Serial.print("] ");
      Serial.println(melding);
      pip(1);
      visKarakter('0' + pipe);     // vis hvilken samtale (ikke-blokkerende)
    }
  }

  // Lærer skriver melding + Enter → sendes til alle elever
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBufPos > 0) {
        serialBuf[serialBufPos] = '\0';
        sendBroadcast(serialBuf);
        serialBufPos = 0;
      }
    } else if (serialBufPos < 31) {
      serialBuf[serialBufPos++] = c;
    } else {
      serialBufPos = 0;
      while (Serial.available()) Serial.read();
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Hjelpefunksjoner

// Pip — litt annen lyd enn elevenes summere
void pip(int antall) {
  for (int i = 0; i < antall; i++) {
    tone(BUZZER, 900, 100);
    delay(180);
  }
}

// Rull tekst på LED-matrisen: 3 sekunder statisk, deretter scroll til venstre
void rullTekst(const char* tekst) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(tekst);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
  delay(3000);

  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.textScrollSpeed(80);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(tekst);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

// Vis enkelt tegn på matrisen (ikke-blokkerende)
void visKarakter(char tegn) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(3, 1, 0xFFFFFF);
  matrix.print(tegn);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}
