# Message to Hack Club

Hi Hack Club team,

I'm working on a small NFC keychain project and would really appreciate your help reviewing it.

The design uses an ATtiny85V microcontroller, an ST25DV04KC NFC tag IC, and an RGB LED. A button switches between four URL profiles, and the firmware is intended to disable NFC when the device is off to reduce power consumption.

I've included the KiCad files, firmware, and a DigiKey bill of materials in this repository:
[Insert your GitHub repository link here]

## Parts budget question

The current estimate is approximately **$5.80 for one set of components** and **$16.80 for three PCBs on OSH wich is the minimum quantity**. With a $35 budget, that would leave $18.20 after the PCB cost.

Would it be possible to request **three sets of the components** so I can assemble all three boards? Based on those estimates, the total would be about $34.20 ($16.80 + 3 × $5.80), before any shipping, taxes, or price changes. If three sets are not possible, would **two sets of components** be an option?

I may be misunderstanding how the parts allowance is calculated, so please let me know what quantity is eligible under the budget.

## Quick technical review

If someone has time, could you also take a quick look at the project before I order or assemble it? I'd especially appreciate a sanity check of the KiCad schematic/PCB, the component footprints and wiring, and whether the firmware looks likely to work with the ATtiny85 and ST25DV04KC.

I understand that a full review or hardware test may not be possible, but even a brief review or advice on the most important things to verify would be very helpful. I haven't independently confirmed that the design works yet, so I don't want to imply that it has been tested.

Thank you for your time and help!
