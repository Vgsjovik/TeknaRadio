// Hub.ino — Lærerens hub (alle leksjoner)
//
// Hører alt som skjer i rommet og skriver det til Serial, én linje per
// hendelse. Linjene leses av hub-nettsiden, men kan også leses direkte
// i Serial Monitor (9600 baud):
//
//     ONLINE|FOX|ION         FOX har startet, og har ION som partner
//     PULS|FOX|ION           FOX lever fortsatt (hvert 5. sekund)
//     TENN|FOX|ION           FOX tente skjermen til ION
//     SLUKK|FOX|ION          FOX slukket skjermen til ION
//     MSG|FOX|ION|tekst      melding fra FOX til ION
//     SENDT|ION|tekst        huben sendte selv en melding
//
// Sende fra læreren (skriv i Serial Monitor + Enter):
//
//     Hei alle sammen         sendes til alle
//     SEND|ion|Hei Ion        sendes bare til ION
//     PING                    huben svarer med READY og navnelista
//
// To adresser:
//   Pipe 0 = "HUB00"  bare huben lytter. Auto-ACK PÅ: kvitteringen er
//                     det som gir boksene «K».
//   Pipe 1 = "RADIO"  felles for alle bokser. Auto-ACK AV.
//
// Pinout: CE = 9, CSN = 10, SPI = 11/12/13

#include <ArduinoGraphics.h>      // MÅ komme FØR Arduino_LED_Matrix.h
#include "Arduino_LED_Matrix.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN     9
#define CSN_PIN   10

RF24 radio(CE_PIN, CSN_PIN);
ArduinoLEDMatrix matrix;

// --- Navnelista (må være lik i alle TeknaRadio-filer) ---
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

const byte hubAdresse[6]    = "HUB00";
const byte fellesAdresse[6] = "RADIO";

// Pakke = type + fra + til + eventuell tekst
const char TYPE_ONLINE  = 'O';
const char TYPE_PULS    = 'H';
const char TYPE_TENN    = 'T';
const char TYPE_SLUKK   = 'S';
const char TYPE_MELDING = 'M';
const int  SMS_MAKS     = 28;

// --- Kommando-buffer (innkommende fra Serial) ---
char cmdBuf[48];
int  cmdBufPos = 0;

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

// Viser de to første bokstavene i et navn. Én bokstav skiller ikke ICE fra ION.
void visToTegn(const char* navn) {
  matrix.beginDraw();
  matrix.clear();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_5x7);
  matrix.beginText(1, 1, 0xFFFFFF);    // to tegn à 5 piksler, midtstilt på 12
  matrix.print(navn[0]);
  if (navn[1] != '\0') matrix.print(navn[1]);
  matrix.endText(NO_SCROLL);
  matrix.endDraw();
}

// Forteller nettsiden at huben er oppe, og hvilke navn som finnes
void skrivReady() {
  Serial.println("READY|TeknaRadio Hub");
  Serial.print("NAVN|");
  for (int i = 0; i < ANTALL_NAVN; i++) {
    if (i > 0) Serial.print(",");
    Serial.print(NAVN[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);

  matrix.begin();
  matrix.beginDraw(); matrix.clear(); matrix.endDraw();

  if (!radio.begin()) {
    Serial.println("ERR|Radio ikke funnet");
    visKarakter('X');
    while (true);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(80);
  radio.setRetries(5, 3);

  radio.setAutoAck(false);
  radio.openReadingPipe(0, hubAdresse);
  radio.setAutoAck(0, true);              // boksene venter på kvittering her
  radio.openReadingPipe(1, fellesAdresse);
  radio.startListening();

  skrivReady();
  visKarakter('H');
}

// Sender en melding fra læreren. til = en boks, eller ID_ALLE.
void sendMelding(char til, const char* tekst) {
  char pakke[32];
  snprintf(pakke, sizeof(pakke), "%c%c%c%.*s", TYPE_MELDING, ID_HUB, til, SMS_MAKS, tekst);

  radio.stopListening();
  radio.setAutoAck(0, false);      // ingen kvitterer på fellesadressen
  radio.openWritingPipe(fellesAdresse);
  radio.write(pakke, strlen(pakke) + 1);
  radio.setAutoAck(0, true);       // gjenopprett kvittering på "HUB00"
  radio.startListening();          // RF24 setter pipe 0 tilbake til "HUB00" her

  Serial.print("SENDT|");
  Serial.print(navnFor(til));
  Serial.print("|");
  Serial.println(pakke + 3);

  visKarakter('>');
}

void handleSerialCommand(char* line) {
  if (strcasecmp(line, "PING") == 0) {
    skrivReady();
    return;
  }

  if (strncasecmp(line, "SEND|", 5) != 0) {
    sendMelding(ID_ALLE, line);    // vanlig tekst går til alle
    return;
  }

  char* navn = line + 5;
  char* sep  = strchr(navn, '|');
  if (!sep) return;
  *sep = '\0';

  int funnet = finnNavn(navn);
  if (strcasecmp(navn, "ALLE") == 0) {
    sendMelding(ID_ALLE, sep + 1);
  } else if (funnet >= 0) {
    sendMelding('A' + funnet, sep + 1);
  } else {
    Serial.print("ERR|Ukjent navn: ");
    Serial.println(navn);
  }
}

void loop() {
  if (radio.available()) {
    char pakke[33] = {0};
    radio.read(pakke, 32);
    char type = pakke[0];
    const char* fra = navnFor(pakke[1]);
    const char* til = navnFor(pakke[2]);

    if (type == TYPE_ONLINE || type == TYPE_PULS) {
      // "til" er her boksens partner, slik at nettsiden kan gruppere parene
      Serial.print(type == TYPE_ONLINE ? "ONLINE|" : "PULS|");
      Serial.print(fra); Serial.print("|"); Serial.println(til);
    } else if (type == TYPE_TENN || type == TYPE_SLUKK) {
      Serial.print(type == TYPE_TENN ? "TENN|" : "SLUKK|");
      Serial.print(fra); Serial.print("|"); Serial.println(til);
    } else if (type == TYPE_MELDING) {
      Serial.print("MSG|");
      Serial.print(fra); Serial.print("|");
      Serial.print(til); Serial.print("|");
      Serial.println(pakke + 3);
    }

    // Rask, ikke-blokkerende indikator: de to første bokstavene i avsenderens navn
    if (type != TYPE_PULS) visToTegn(fra);
  }

  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (cmdBufPos > 0) {
        cmdBuf[cmdBufPos] = '\0';
        handleSerialCommand(cmdBuf);
        cmdBufPos = 0;
      }
    } else if (cmdBufPos < (int)sizeof(cmdBuf) - 1) {
      cmdBuf[cmdBufPos++] = c;
    } else {
      cmdBufPos = 0;
      while (Serial.available()) Serial.read();
    }
  }
}
