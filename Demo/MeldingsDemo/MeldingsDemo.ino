#include <ArduinoGraphics.h>
#include "Arduino_LED_Matrix.h"

#define KNAPP_PIN   6
#define BUZZER_PIN  11

const unsigned long FORSINKELSE_MS   = 1000;   // ms fra trykk til lyd
const int           BLINK_ANTALL     = 2;       // antall blink før tekst
const int           SCROLL_HASTIGHET = 120;     // ms per rute (lavere = raskere)

ArduinoLEDMatrix matrix;
bool     venter    = false;
unsigned long trykk_tid = 0;

void meldingslyd() {
    tone(BUZZER_PIN, 131, 240); delay(290);
    tone(BUZZER_PIN, 196, 240); delay(290);
    tone(BUZZER_PIN, 262, 240); delay(300);
}

void blink_matrix(int n) {
    for (int i = 0; i < n; i++) {
        matrix.beginDraw();
        matrix.fill(0xFFFFFFFF);
        matrix.rect(0, 0, 12, 8);
        matrix.endDraw();
        delay(250);
        matrix.beginDraw();
        matrix.clear();
        matrix.endDraw();
        delay(250);
    }
}

void scroll_tekst(const Font& font, int y) {
    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textFont(font);
    matrix.beginText(0, y, 0xFFFFFF);
    matrix.print("KOM HJEM!");
    matrix.endText(NO_SCROLL);
    matrix.endDraw();
    delay(1000);

    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textScrollSpeed(SCROLL_HASTIGHET);
    matrix.textFont(font);
    matrix.beginText(0, y, 0xFFFFFF);
    matrix.print("KOM HJEM!");
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
}

void setup() {
    pinMode(KNAPP_PIN, INPUT_PULLUP);
    matrix.begin();

    matrix.beginDraw();
    matrix.fill(0xFFFFFFFF);
    matrix.rect(0, 0, 12, 8);
    matrix.endDraw();
    delay(1500);
    matrix.beginDraw();
    matrix.clear();
    matrix.endDraw();

    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textScrollSpeed(SCROLL_HASTIGHET);
    matrix.textFont(Font_5x7);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.print("UNO rev 4");
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
}

void loop() {
    if (digitalRead(KNAPP_PIN) == LOW && !venter) {
        venter    = true;
        trykk_tid = millis();
    }

    if (venter && millis() - trykk_tid >= FORSINKELSE_MS) {
        venter = false;
        meldingslyd();
        blink_matrix(BLINK_ANTALL);
        scroll_tekst(Font_5x7, 1);
        scroll_tekst(Font_5x7, 1);
        matrix.beginDraw();
        matrix.clear();
        matrix.endDraw();
    }
}
