---
Title : NFC MULTI TAG

Github : https://github.com/dgm91-pro/Attiny85_NFC_MULTI_TAG

Description : A small NFC business keychain with 4 selectable profiles.

Total Time Spent : 13h
***

## The Idea 💡

I wanted to make something useful for my everyday life, especially for my business and professional activities.

The idea is to create a small NFC keychain that can share different links depending on the selected profile. For example, I could use it to share my website, portfolio, social media or professional contact information.

I wanted to make it compact, easy to use and battery powered.

Total time spent: 1h 30m

# Choosing Components ❓

I started looking for the components needed for this project.

Here is what I selected:

- ATtiny85V as the main microcontroller.
- ST25DV04KC for NFC communication.
- RGB LED to show the selected profile.
- Push button to control the device.
- CR2032 battery for power.

Total time spent: 1h

## Working on the Schematic and PCB 🗺️

I started designing the electronic circuit in KiCad.

I had to organise all the connections between the ATtiny85, NFC chip, RGB LED and button.

One of the main challenges was the nfc antenna , i never made one and it was pretty difficult to understand

I also started working on the PCB layout, keeping the final size in mind because I want this device to fit inside a small keychain.

Total time spent: 7h 30m

# Firmware Development 🖥️

I made the Arduino firmware ( with a bit help of AI , but i'll need to check everything once i get the parts ).
The main goal is to have four different NFC profiles that can be selected and overwrited using only one button and to be able to power on/off the device with this same button.
Each profile has its own LED colour:

* Blue
* Red
* Green
* Violet
* Maybe more 

Another important part is power management. Since the device uses a CR2032 battery, I included sleep mode and NFC power-saving features in the design.
Total time spent: 1h

## BOM and Budget 🪎

I prepared the Bill of Materials with the components needed for manufacturing. ( ToDo : add C4 if required to tune antenna )

I also researched component prices and estimated the PCB manufacturing costs.

The goal was to keep the project affordable while using components that fit the design.

Total time spent: 1h

## Reviewing the Project 🔍

I reviewed the different files and checked what still needs to be done.

The next step will be manufacturing the PCB and assembling the components. After that, I will need to program the ATtiny85 and test the NFC profiles with a smartphone.

My final goal is to have a small NFC tool that I can carry everywhere and use when sharing my business or professional information.

I also want to make a small case in 3D print , but i'll need to see what i can use to make the led see through , might be some reflective paper.
Total time spent: 1h

---

Total development time: 13h
