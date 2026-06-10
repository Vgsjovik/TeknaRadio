# Leksjon 1 – Sjekkliste
### Gruppe: _______ &nbsp;&nbsp;&nbsp; Navn: _______________________________

## Del 1 – Montering

- [ ] Arduino UNO R4 WiFi er plassert i boksen og montert med T-plugger
- [ ] Breadboard er plassert i boksen
- [ ] NRF24L01 er koblet til  HW-200 adapter og plassert i antenneholder
    - **OBS:** Det kan være lettere å sette i kablene før du plasserer antennen i boksen

**Pinoppsett – NRF24L01-adapter til Arduino:**

<table>
<tr>
<td valign="top">

| Adapter | Arduino |
|---|---|
| VCC | 5V |
| GND | GND |
| CE | Pin 9 |
| CSN | Pin 10 |
| SCK | Pin 13 |
| MISO | Pin 12 |
| MOSI | Pin 11 |
| IRQ | Ikke i bruk |

</td>
<td valign="top">

<img src="assets/adapter.png" alt="Adapter module" width="350">

</td>
</tr>
</table>

> **Tilkoblinger:**  
- [ ] NRF24L01-adapter koblet til Arduino med 7 kabler
- [ ] Jord (GND) fra Arduino til øvre blå linje på breadboard
    - [ ] Tips: Snu breadboardet slik at den blå linja er nederst. Se bilde nedenfor.
- [ ] Buzzer er koblet til **Pin 3** (+) og **GND** (–)
- [ ] Knapp A er koblet til **Pin 2** og **GND**
- [ ] Knapp B er koblet til **Pin 4** og **GND**
- [ ] USB-kabel er koblet mellom Arduino og PC
<img src="assets/Skjermbilde%202026-06-09%20kl.%2017.34.50-1.png" width="550">



---

## Del 2 – Arduino IDE

- [ ] Gå til github.com/Vgsjovik/Teknaradio
- [ ] Lastet ned og åpnet filen `Leksjon1_RadioTest.ino` i Arduino IDE
- [ ] Valgt riktig kort: **Tools → Board → Arduino UNO R4 WiFi**
- [ ] Valgt riktig port: **Tools → Port** */dev/cu/USB-modem...*
- [ ] RF24-biblioteket er installerrt (Bruk library manager og søk på RF24)
- [ ] Endret `#define GRUPPE` til tallet dere har fått av læreren 
- [ ] Lastet opp koden til Arduino (**Upload**-knappen)



---

## Del 3 – Test

- [ ] Koden kompilerer uten feil (ingen røde meldinger i Arduino IDE)
- [ ] Serial Monitor er åpnet (**Tools → Serial Monitor**, 9600 baud)
- [ ] Serial Monitor viser oppstartsmeldingen med gruppenummeret
    - [ ] Trykk *reset* om dere ikke ser noen melding. På bildet er reset-knappen den røde knappen rett ved USB-kabelen. På deres enhet er knappen hvit og flat.

**Knappetester:**

- [ ] Trykket **Knapp A** → matrisen viser `V` + 1 pip
- [ ] Trykket **Knapp B** → matrisen viser `K` + 2 pip *(HUB bekreftet)*
- [ ] Hvis læreren sender en melding tilbake: Serial Monitor viser `[Mottatt fra hub]` + teksten, og matrisen ruller meldingen

---

## Refleksjonsspørsmål

> **Spørsmål:** Når dere trykker Knapp B sender dere en melding *og* får et svar
> tilbake fra huben (ACK). Hva er forskjellen på *å sende* og *å få bekreftelse på
> at meldingen kom frem? Hvorfor er det nyttig å vite at den kom fram?*
>
> Svar: _______________________________________________
>
> _______________________________________________

---

## Ferdig! ✓

Når alle boksene er krysset av og læreren har sett `K`-symbolet på skjermen:

**→ Gå videre til Leksjon 2**
