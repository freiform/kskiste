# Ks Kiste.

(K's Box)  
Arduino based, RFID-enabled jukebox.

## Features

A simple and portable jukebox that plays audio files stored on an SD card when the corresponding RFID tag is found. 

* Three buttons to control playback and volume
* Buttons light up on specific events
* Powered by a LiPo battery
* Built-in charger for the battery
* Drives a speaker directly (built-in)
  
## Usage
  
Known RFIDs are hardcoded and correspond to a specific directory. When a vaild RFID tag is found, the audio files in the corresponding folder are being played. Upon removal of the tag, playback is paused and continues when the same tag is found again. 
If another valid tag is found, playback starts for its corresponding folder. 

The buttons _pause/play_, _forward_ and _back_ do just what they are supposed to. Holding _play/pause_ while pressing _forward_ or _back_ in- or decreases the volume.

## Hardware

* Arduino Nano 
* [DFPLayer Mini](https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299)
* RC522 RFID shield
* Illuminated arcade buttons
* [Visaton FRS 8](http://www.visaton.de/en/products/fullrange-systems/frs-8-4-ohm) - 4 Ω full range driver
* WeMos battery shield
* LiPo battery (3500 mAh)
* A few resistors & odds and ends 

## Software

In addition to the standard Arduino libraries the sketch includes 

* [DFPlayer Mini mp3](https://github.com/Makuna/DFMiniMp3)
* [MFRC522](https://github.com/miguelbalboa/rfid)
* [EasyButton](https://github.com/evert-arias/EasyButton)