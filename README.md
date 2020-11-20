# **PayPi**

### Kai Bartolone - Vincent Xia - Tristan Sinclair

## **Project Description**
This app uses the pn532 nfc sensor to simulate a payment system with nfc tags. Using the console, a user can charge, pay, set, and check the balance that is on an nfc tag. Users can also get a printout of block information on the nfc tag.
___
## **Member contribution**
### Kai Bartolone
### Vincent Xia
### Tristan Sinclair

___
## **References**
[Waveshare Wiki PN532 NFC HAT](https://www.waveshare.com/wiki/PN532_NFC_HAT)

___
## **Self-evaluation**
We were able to execute the proposal very well for the time span that we had. We are particularly proud of the debugging process that went into getting the pn532 sensor to work. Initially, we could not get spi communication to work with the pn532, but using the logic analyzer we were able to make connections between the byte level communications and the protocol. These situations that needed the logic analyzer happened often, and we were able to overcome faulty wirings, faulty cables, and confusing protocols. In addition to the Mifare tag protocol, something that was interesting to learn was the redundancy that was embedded into the communication systems. For example, the spi communication protocol checks the status of the pn532 multiple times in every data transaction in addition to sending an acknowledgement of the pn532 receiving a command.

___
## **Photos**
You are encouraged to submit photos/videos of your project in action. Add the files and commit to your project repository to include along with your submission.

**We should use [RecordIt](https://recordit.co/) (Makes cool gifs for GitHub)**
___