# NFC Profile Keychain

A compact, battery-powered NFC keychain built around an **ATtiny85V** and an **ST25DV04KC dynamic NFC tag**. A single button switches between four configurable URL profiles, each indicated by an RGB LED.

> **Project status:** Prototype / under development. The firmware and hardware have not yet been independently verified. Please review the design and test it before relying on it.

## Features

- Four selectable NFC URL profiles: blue, red, green, and violet
- One-button interface
- RGB LED feedback
- NFC is disabled when the device starts and when it is switched off
- Selected profile is saved in the ATtiny85 EEPROM
- Low-power sleep while switched off
- I²C error feedback: four yellow flashes

## How it works

| Action | Result |
|---|---|
| Power-on | The keychain starts with NFC disabled |
| Short press while off | Enables NFC and displays the saved profile |
| Short press while on | Switches to the next profile: blue → red → green → violet |
| Long press (at least 1 second) while on | Disables NFC |
| While on | Brief LED pulse approximately every 4 seconds |
| I²C / NFC error | Four yellow flashes |

The four URL values are currently examples and should be replaced with the intended destinations before testing:

- `URL_0`: blue profile
- `URL_1`: red profile
- `URL_2`: green profile
- `URL_3`: violet profile

The firmware supports `http://` and `https://` URLs and documents a maximum URL length of 116 characters.

## Hardware

The current bill of materials includes:

- **Microcontroller:** ATtiny85V-10SU, 8-pin SOIC
- **NFC tag IC:** ST25DV04KC
- **RGB LED:** Kingbright APTF1616LSEEZGKQBKC, common anode
- **Battery:** CR2032 (3 V)
- **Button:** momentary tactile switch
- Supporting resistors and capacitors

The KiCad project files and a DigiKey parts-list CSV are included in this repository.

## Pin mapping

The mapping below follows the firmware comments and refers to the ATtiny85 SOIC-8 package pin numbers.

| ATtiny85 pin | Signal | Connection |
|---|---|---|
| 1 / PB5 | RESET | ISP header; 10 kΩ pull-up |
| 2 / PB3 | LED green | RGB LED cathode through resistor |
| 3 / PB4 | LED blue | RGB LED cathode through resistor |
| 4 | GND | Ground |
| 5 / PB0 | SDA | ST25DV04KC SDA; 10 kΩ pull-up |
| 6 / PB1 | LED red | RGB LED cathode through resistor |
| 7 / PB2 | SCL / button | ST25DV04KC SCL, 10 kΩ pull-up, button to GND |
| 8 | VCC | 3 V supply |

**Important:** PB2 is shared by I²C SCL and the button. The firmware waits for the button to be released before starting I²C operations.

## Firmware setup

The firmware is in [`program/program.ino`](program/program.ino).

1. Install Arduino IDE 2.x.
2. Install **ATTinyCore** and select the ATtiny85 target.
3. Select the **ATtiny85 (no bootloader)** board option and the internal 1 MHz clock.
4. Select a compatible ISP programmer, such as USBasp. The original notes specify the programmer's **SLOW** setting for a 1 MHz target.
5. Burn the bootloader / initialization sequence once to configure the fuses, if required by your setup.
6. Replace the example URLs in `program.ino`.
7. Remove the CR2032 before programming. Power the board from a suitable programmer or regulated supply set to the correct voltage, and upload using **Sketch → Upload Using Programmer**.

Menu labels can vary with Arduino IDE and ATTinyCore versions; confirm the target, clock, and fuse settings for your exact setup.

## Safety and testing notes

- **Remove the CR2032 before connecting a programmer or external power supply.** Never connect a battery to an external supply at the same time.
- Keep all programming signals within the ATtiny85's and NFC IC's supported voltage limits. Do not apply 5 V logic to a circuit designed for a 3 V supply.
- Verify the schematic, PCB footprint assignments, power connections, pull-ups, and LED polarity before assembling the board.
- Test the NFC behavior with a compatible NFC phone or reader and confirm that each profile opens the expected URL.
- The current repository has not been confirmed to compile or to work on physical hardware. A review and hardware test are still needed.

## Repository layout

```text
.
├── Budget/
│   ├── HackClub-8pin.csv
│   └── OSH-Park.png
├── KiCad/
│   ├── tag.zip
│   └── IF LIB NOT WORKING.zip
├── program/
│   └── program.ino
├── README.md
└── HACKCLUB_REQUEST.md
```

## Help / review requested

I would appreciate a quick review of the KiCad design and firmware before ordering or assembling the boards. In particular, I would like confirmation that the PCB and component footprints look correct, the ATtiny85/ST25DV04KC wiring is plausible, and the firmware's NFC/I²C behavior is likely to work as intended.

See [`HACKCLUB_REQUEST.md`](HACKCLUB_REQUEST.md) for the message prepared for Hack Club.
