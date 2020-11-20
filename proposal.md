## Project title
PayPi

## Team members
Kai Bartolone, Tristan Sinclair, Vincent Xia

## Project description

Our goal is to create an NFC payment system with the Pi and the PN532 NFC HAT which communicates over SPI. We’ll start by sending data to the HAT from the PI and make sure that the Pi is correctly sending/receiving data using the Logic Analyzer. Then we’ll move towards generating NFC-compatible bit patterns from the Pi. After we ensure that the 2-way data transmission correctly works, we will wire up PN532 HAT to the Pi, and using the NFC Tool app, we will confirm reading/writing to NFC tags. If we are able to achieve it, this wireless NFC data reception module will be a major milestone for the project and the most viable target end-goal for the project. Time permitting, we’ll investigate persistent memory storage of payment information on the Pi.



## Hardware, budget


-- Waveshare PN532 NFC HAT -- $20

## Major tasks, member responsibilities
Major task breakdown and who is responsible for each task.

## Schedule, midpoint milestones

Demonstrate reading a message over SPI bus (simulate sending an SPI message from our laptops)
Demonstrate sending a message over the SPI bus and getting the correct message
Send the SPI message over to our laptops
Demonstrate Waveshare library sends stuff over the SPI bus
Make sure Waveshare library is bug free when working with staff SPI module
Read an NFC frame of bytes
Write an NFC frame of bytes
Demonstrate reading and displaying NFC tag
Demonstrate writing an NFC tag
Add in payment functionality and integration with shell



## Resources needed, issues

Staff SPI protocol implementation
PN532 NFC HAT
Waveshare wiki
Wakeshare demo's

