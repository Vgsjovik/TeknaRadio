/*
 * ╔══════════════════════════════════════════════════╗
 * ║   LEKSJON 2 – To grupper snakker sammen          ║
 * ╚══════════════════════════════════════════════════╝
 *
 * Alle enheter kjoerer IDENTISK kode. Ingen rollevalg —
 * begge gruppene i et par sender og lytter paa SAMME pipe.
 *
 *   Knapp A = tenn skjermen til de andre + melodi opp  (Bee-Bipp)
 *   Knapp B = slukk skjermen til de andre + melodi ned (Bee-Boop)
 *   Serial  = skriv tekst + Enter for aa chatte (vises hos alle)
 *
 * To innstillinger oeverst:
 *   GRUPPE = ditt gruppenummer (0–9), vises i meldingene dine
 *   PIPE   = felles samtale-pipe (1–5). BEGGE gruppene i paret
 *            setter SAMME PIPE — det er det som kobler dere sammen.
 *
 * Oppkobling (uendret fra Leksjon 1):
 *   VCC  ->5V       GND  ->GND
 *   CE   ->Pin 9    CSN  ->Pin 10
 *   SCK  ->Pin 13   MOSI ->Pin 11   MISO ->Pin 12
 *   Summer (+) ->Pin 3    Summer (-) ->GND
 *   Knapp A    ->Pin 2    (andre ben til GND)
 *   Knapp B    ->Pin 4    (andre ben til GND)
 */

// ════════════════════════════════════════════════════
//  Endre dette — ditt gruppenummer (0–9)
// ════════════════════════════════════════════════════
#define GRUPPE 1

// ════════════════════════════════════════════════════
//  Endre dette — felles samtale-pipe (1–5)
//  Begge gruppene i paret MAA sette samme PIPE.
// ════════════════════════════════════════════════════
#define PIPE 1

#include <ArduinoGraphics.h>   // MÅ komme FØR Arduino_LED_Matrix.h
#include "Arduino_LED_Matrix.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ─── Pinner ────────────────────────────────────────
#define CE_PIN       9
#define CSN_PIN      10
#define BUZZER_PIN   3
#define KNAPP_A      2
#define KNAPP_B      4

// ─── Objekter ──────────────────────────────────────
ArduinoLEDMatrix matrix;
RF24 radio(CE_PIN, CSN_PIN);

// ─── Felles samtale-adresse (bygges i setup) ───────
//  Formen er "{PIPE}SAMT", f.eks. "1SAMT". Det varierende
//  tegnet ligger fremst slik at laererhuben kan lytte paa
//  alle 5 samtalene samtidig (NRF24 multiceiver-regel).
byte samtaleAdresse[6];

// ─── Tilstand ──────────────────────────────────────
bool skjermPaa = false;

// ─── Skjermrammer ──────────────────────────────────
const uint32_t FULL[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
const uint32_t TOM[3]  = { 0x00000000, 0x00000000, 0x00000000 };

// ─── Debounce ──────────────────────────────────────
unsigned long sistTrykkA = 0;
unsigned long sistTrykkB = 0;
const unsigned long DEBOUNCE_MS = 250;

// ─── Serial-buffer ─────────────────────────────────
char serialBuffer[32];
int  serialPos = 0;

// ════════════════════════════════════════════════════
//  Hjelpefunksjoner
// ════════════════════════════════════════════════════

void pip(int antall) {
  for (int i = 0; i < antall; i++) {
    tone(BUZZER_PIN, 1200, 120);
    delay(220);
  }
}

void melodiOpp() {   // Bee-Bipp (stigende)
  tone(BUZZER_PIN, 262, 150); delay(200);  // C4
  tone(BUZZER_PIN, 330, 150); delay(200);  // E4
  tone(BUZZER_PIN, 392, 150); delay(200);  // G4
}

void melodiNed() {   // Bee-Boop (synkende)
  tone(BUZZER_PIN, 392, 150); delay(200);  // G4
  tone(BUZZER_PIN, 330, 150); delay(200);  // E4
  tone(BUZZER_PIN, 262, 150); delay(200);  // C4
}

void visKarakter(char c) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(3, 1, 0xFFFFFF);
  matrix.print(c);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

void sendMelding(const char* tekst) {
  radio.stopListening();
  radio.write(tekst, strlen(tekst) + 1);
  radio.startListening();
  Serial.print("[Sendt] "); Serial.println(tekst);
}

// ════════════════════════════════════════════════════
//  setup()
// ════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);

  pinMode(KNAPP_A, INPUT_PULLUP);
  pinMode(KNAPP_B, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // Bygg den felles samtale-adressen fra PIPE: "1SAMT" ... "5SAMT"
  snprintf((char*)samtaleAdresse, 6, "%dSAMT", PIPE);

  // Start LED-matrise
  matrix.begin();
  matrix.beginDraw(); matrix.clear(); matrix.endDraw();  // prime-frame

  // Start radio — radio.begin() er paalitelig oppstartsjekk (HW-200)
  if (!radio.begin()) {
    Serial.println("[FEIL] Radio ikke funnet! Sjekk kabling og start om.");
    visKarakter('X');
    pip(2);
    while (true) {}
  }
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(80);
  radio.setRetries(5, 3);

  // Felles pipe brukt baade til lytting og sending.
  // Auto-ACK MAA av: flere enheter deler samme adresse, og de kan
  // ikke alle kvittere paa en gang (det ville kollidere).
  radio.setAutoAck(false);
  radio.openReadingPipe(1, samtaleAdresse);
  radio.openWritingPipe(samtaleAdresse);
  radio.startListening();

  Serial.println("================================");
  Serial.println("  TeknaRadio – Leksjon 2");
  Serial.print  ("  Gruppe: ");   Serial.print(GRUPPE);
  Serial.print  ("  |  Pipe: ");  Serial.println(PIPE);
  Serial.println("================================");
  Serial.println("  Knapp A = tenn de andres skjerm");
  Serial.println("  Knapp B = slukk de andres skjerm");
  Serial.println("  Skriv tekst + Enter for aa chatte");
  Serial.println("================================");
  Serial.println();

  // Vis gruppenummeret kort ved oppstart
  visKarakter('0' + GRUPPE);
  delay(800);
  matrix.beginDraw(); matrix.clear(); matrix.endDraw();
  pip(1);
}

// ════════════════════════════════════════════════════
//  Innkommende radio-meldinger
// ════════════════════════════════════════════════════
void sjekkRadio() {
  if (radio.available()) {
    char melding[32] = { 0 };
    radio.read(&melding, sizeof(melding));

    if (strcmp(melding, "PA") == 0) {
      skjermPaa = true;
      matrix.loadFrame(FULL);
      melodiOpp();
      Serial.println("[Mottatt] Skjerm tennes!");
    }
    else if (strcmp(melding, "AV") == 0) {
      skjermPaa = false;
      matrix.loadFrame(TOM);
      melodiNed();
      Serial.println("[Mottatt] Skjerm slukket.");
    }
    else {
      Serial.print("[Mottatt] ");
      Serial.println(melding);
      pip(1);
    }
  }
}

// ════════════════════════════════════════════════════
//  Serial-input fra brukeren
// ════════════════════════════════════════════════════
void sjekkSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();

    if (c == '\n' || c == '\r') {
      if (serialPos > 0) {
        serialBuffer[serialPos] = '\0';

        String s = String(serialBuffer);
        s.trim();
        s.toUpperCase();

        if (s == "PA" || s == "P\xC3\x85") {       // "PA" eller "PÅ"
          sendMelding("PA");
        }
        else if (s == "AV") {
          sendMelding("AV");
        }
        else {
          // Fritekst — sett gruppenummer foran saa mottakeren ser hvem
          char utgaende[32];
          snprintf(utgaende, sizeof(utgaende), "G%d: %s", GRUPPE, serialBuffer);
          sendMelding(utgaende);
        }

        serialPos = 0;
      }
    }
    else {
      if (serialPos < 24) {        // plass til "G9: " + tekst innen 32 bytes
        serialBuffer[serialPos++] = c;
      } else {
        serialBuffer[24] = '\0';
        Serial.println("[Advarsel] Melding for lang, avskjaert.");
        char utgaende[32];
        snprintf(utgaende, sizeof(utgaende), "G%d: %s", GRUPPE, serialBuffer);
        sendMelding(utgaende);
        serialPos = 0;
        while (Serial.available()) Serial.read();
      }
    }
  }
}

// ════════════════════════════════════════════════════
//  Knappehåndtering
// ════════════════════════════════════════════════════
void sjekkKnapper() {
  unsigned long na = millis();
  if (digitalRead(KNAPP_A) == LOW && (na - sistTrykkA) > DEBOUNCE_MS) {
    sistTrykkA = na;
    Serial.println("[Knapp A] Tenner de andres skjerm...");
    sendMelding("PA");
  }
  else if (digitalRead(KNAPP_B) == LOW && (na - sistTrykkB) > DEBOUNCE_MS) {
    sistTrykkB = na;
    Serial.println("[Knapp B] Slukker de andres skjerm...");
    sendMelding("AV");
  }
}

// ════════════════════════════════════════════════════
//  loop()
// ════════════════════════════════════════════════════
void loop() {
  sjekkKnapper();
  sjekkRadio();
  sjekkSerial();
}
