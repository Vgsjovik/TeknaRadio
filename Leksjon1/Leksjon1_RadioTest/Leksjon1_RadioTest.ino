/*
 * ╔══════════════════════════════════════════════════╗
 * ║   LEKSJON 1 – Er radioen koblet til?             ║
 * ╚══════════════════════════════════════════════════╝
 *
 * Mål: Sjekk at NRF24L01-modulen er riktig koblet,
 *      og send en ping til læreren.
 *
 * Koden kjører ferdig — du trenger bare å sette gruppenummeret ditt.
 *
 * Oppkobling:
 *   VCC  ->5V       GND  ->GND
 *   CE   ->Pin 9    CSN  ->Pin 10
 *   SCK  ->Pin 13   MOSI ->Pin 11   MISO ->Pin 12
 *   Summer (+) ->Pin 3    Summer (-) ->GND
 *   Knapp A    ->Pin 2    (andre ben til GND)
 *   Knapp B    ->Pin 4    (andre ben til GND)
 *
 * Hva gjør knappene?
 *   Knapp A = Test om radioen svarer
 *   Knapp B = Send ping til læreren
 */

// ════════════════════════════════════════════════════
//  Endre dette — skriv gruppenummeret ditt (1–9)
// ════════════════════════════════════════════════════
#define GRUPPE 1
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

// ── Adresse til lærers hub ────────────────────────────────
const byte hubAdresse[6] = "HUB00";

// ── Knapp-debounce ────────────────────────────────────────
unsigned long sistTrykkA = 0;
unsigned long sistTrykkB = 0;
const unsigned long DEBOUNCE_MS = 250;

// ─────────────────────────────────────────────────────────
// Hjelpefunksjoner
// ─────────────────────────────────────────────────────────

void visKarakter(char tegn) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(3, 1, 0xFFFFFF);
  matrix.print(tegn);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

void sluttMatrise() {
  matrix.beginDraw();
  matrix.clear();
  matrix.endDraw();
}

void pip(int antall) {
  for (int i = 0; i < antall; i++) {
    tone(BUZZER_PIN, 1200, 120);
    delay(220);
  }
}

void velkomstMelodi() {
  tone(BUZZER_PIN, 523, 120);   // C5
  delay(160);
  tone(BUZZER_PIN, 659, 120);   // E5
  delay(160);
  tone(BUZZER_PIN, 784, 200);   // G5
  delay(250);
}

void rullTekst(const char* tekst) {
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

// Starter radioen og setter alle innstillinger. Returnerer true hvis
// chipen faktisk svarer på SPI. radio.begin() er en pålitelig sjekk
// (mer enn isChipConnected(), som er upålitelig via HW-200-adapteren).
bool konfigurerRadio() {
  bool ok = radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  // Kanal 80 = 2480 MHz — øvre del av 2.4 GHz-båndet, unngår de vanligste WiFi-kanalene
  radio.setChannel(80);
  radio.setRetries(5, 3);
  radio.openReadingPipe(1, hubAdresse);   // så hub kan sende tilbake
  radio.startListening();
  return ok;
}

// ─────────────────────────────────────────────────────────
// Knapp A: Test om radioen er tilkoblet
// ─────────────────────────────────────────────────────────
void testRadio() {
  Serial.println("[Knapp A] Tester radioen...");

  if (konfigurerRadio()) {
    Serial.println("Radio funnet!");
    visKarakter('V');
    pip(1);
  } else {
    Serial.println("Fant ikke radioen. Sjekk kablingen.");
    visKarakter('X');
    pip(2);
  }
}

// ─────────────────────────────────────────────────────────
// Knapp B: Send ping til lærers hub
// ─────────────────────────────────────────────────────────
void sendPing() {
  char ping[8];
  snprintf(ping, sizeof(ping), "G%d", GRUPPE);

  Serial.println("[Knapp B] Sender ping til hub...");

  radio.stopListening();
  radio.openWritingPipe(hubAdresse);
  bool suksess = radio.write(ping, strlen(ping) + 1);
  radio.startListening();

  if (suksess) {
    Serial.println("Læreren svarte! (ACK mottatt)");
    visKarakter('K');
    pip(2);
  } else {
    Serial.println("Ingen svar fra hub. Er læreren klar?");
    visKarakter('?');
    pip(3);
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
  sluttMatrise();

  konfigurerRadio();

  Serial.println("================================");
  Serial.println("  TeknaRadio – Leksjon 1");
  Serial.print  ("  Gruppe: ");
  Serial.println(GRUPPE);
  Serial.println("================================");
  Serial.println("  Knapp A : Test om radio er koblet");
  Serial.println("  Knapp B : Send ping til laereren");
  Serial.println("================================");
  Serial.println();

  velkomstMelodi();
  visKarakter('R');
  delay(800);
  sluttMatrise();
}

// ─────────────────────────────────────────────────────────
// Loop – kjører på repeat
// ─────────────────────────────────────────────────────────
void loop() {
  unsigned long no = millis();

  // Sjekk om hub har sendt en melding
  if (radio.available()) {
    char melding[32] = {0};
    radio.read(&melding, sizeof(melding));
    Serial.print("[Mottatt fra hub] ");
    Serial.println(melding);
    rullTekst(melding);
    // scroll kjorer i bakgrunnen – sluttMatrise() kalles naar neste knapp trykkes
  }

  // Knapp A: test radio-chip
  if (digitalRead(KNAPP_A) == LOW && (no - sistTrykkA) > DEBOUNCE_MS) {
    sistTrykkA = no;
    testRadio();
    delay(2500);
    sluttMatrise();
  }

  // Knapp B: send ping til hub
  if (digitalRead(KNAPP_B) == LOW && (no - sistTrykkB) > DEBOUNCE_MS) {
    sistTrykkB = no;
    sendPing();
    delay(2500);
    sluttMatrise();
  }
}
