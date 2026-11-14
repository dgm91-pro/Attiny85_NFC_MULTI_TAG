---
title  | NFC MULTI TAG

github | https://github.com/dgm91-pro/Attiny85_NFC_MULTI_TAG

description | A small NFC business keychain with 4 selectable profiles.

total_time | 13h
--- 
# The Idea

I wanted to make something useful for my everyday life, especially for my business and professional activities.

The idea is to create a small NFC keychain that can share different links depending on the selected profile. For example, I could use it to share my website, portfolio, social media or professional contact information.

I wanted to make it compact, easy to use and battery powered.

Total time spent: 1h 30m

# Choosing Components

I started looking for the components needed for this project.

Here is what I selected:

* ATtiny85V as the main microcontroller.
* ST25DV04KC for NFC communication.
* RGB LED to show the selected profile.
* Push button to control the device.
* CR2032 battery for power.

I chose the ATtiny85 because it is small and has only 8 pins, which makes the project more challenging.

Total time spent: 1h

# Working on the Schematic and PCB

I started designing the electronic circuit in KiCad.

I had to organise all the connections between the ATtiny85, NFC chip, RGB LED and button.

One of the main challenges was the limited number of pins available on the microcontroller. I had to think carefully about how to use them.

I also started working on the PCB layout, keeping the final size in mind because I want this device to fit inside a small keychain.

Total time spent: 3h 30m

# Firmware Development

I worked on the Arduino firmware.

The main goal is to have four different NFC profiles that can be selected using only one button.

Each profile has its own LED colour:

* Blue
* Red
* Green
* Violet

I also worked on the button behaviour, including switching profiles and turning the device on and off.

Another important part is power management. Since the device uses a CR2032 battery, I included sleep mode and NFC power-saving features in the design.

Total time spent: 3h

# BOM and Budget

I prepared the Bill of Materials with the components needed for manufacturing.

I also researched component prices and estimated the PCB manufacturing costs.

The goal was to keep the project affordable while using components that fit the design.

Total time spent: 1h

# GitHub Documentation

I organised the GitHub repository and added the project files.

I included the KiCad design files, firmware and component list.

I also worked on the README to explain the project, its features, the hardware and how the device is supposed to work.

I wanted the repository to be clear enough for someone else to understand the project.

Total time spent: 2h

# Reviewing the Project

I reviewed the different files and checked what still needs to be done.

The next step will be manufacturing the PCB and assembling the components. After that, I will need to program the ATtiny85 and test the NFC profiles with a smartphone.

I haven't confirmed the design on physical hardware yet, so there is still work to do.

My final goal is to have a small NFC tool that I can carry everywhere and use when sharing my business or professional information.

Total time spent: 1h

---

Total development time: 13h
