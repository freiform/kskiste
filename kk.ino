/** 
 * Karlis Kiste
 * 
 * Arduino based, RFID-enabled jukebox.
 * 
 * TODO:
 * - Sometimes a track is repeated (after button press?)
 * - Prevent fwd/back on pause
 * - Stop and sleep after pausing for a time (2 min?)
 * 
 * 
 * BSD 3-Clause License
 * Copyright (c) 2019, Sven Kissner
 * 
 * / 

#include "Arduino.h"
#include "SoftwareSerial.h"
#include <DFMiniMp3.h>
#include <EasyButton.h>
#include <MFRC522.h>
#include <SPI.h>


/** 
 * Pinout custom board (top - bottom)
 * 1 - A0 - Button Play - red
 * 2 - A1 - Button Fwd  - or
 * 3 - A2 - Button Back - br
 * 4 - D4 - LED Back    - ye
 * 5 - D3 - LED Fwd     - bl
 * 6 - D2 - LED Play    - gn
 * 7 - GND
 * 8 - GND
 **/

/** 
 * DFPlayer, communication via SoftwareSerial
 */
#define PIN_DFP_RX 7
#define PIN_DFP_TX 8
#define PIN_DFP_BUSY 6

/** 
 * RFID-RC522
 */
#define PIN_SDA 10
#define PIN_RST 9

/**
 * Buttons incl. LED
 */
#define BUTTON_PLAY A0
#define BUTTON_FORWARD A1
#define BUTTON_BACK A2
#define LED_PLAY 2
#define LED_FORWARD 3
#define LED_BACK 4

/*
 * Global variables to keep track of Tags
 */
int oldTag = -1, newTag;
boolean rfidState;
boolean cardPresent;
boolean oldState;
boolean isSetVolume;

boolean playing;
uint16_t currentTrack;
uint16_t nTracks;

static void continuePlayback(uint16_t track);

/*
 * Notifications for DFMiniMp3
 */
class Mp3Notify {
  public:
    static void OnError(uint16_t errorCode) {
      // see DfMp3_Error for code meaning
      Serial.println();
      Serial.print("Com Error ");
      Serial.println(errorCode);
    }
  
    static void OnPlayFinished(uint16_t globalTrack) {
      Serial.println();
      Serial.print("Play finished for #");
      Serial.println(globalTrack);
      continuePlayback(globalTrack);
    }
  
    static void OnCardOnline(uint16_t code) {
      Serial.println();
      Serial.print("Card online ");
      Serial.println(code);     
    }
  
    static void OnCardInserted(uint16_t code) {
      Serial.println();
      Serial.print("Card inserted ");
      Serial.println(code); 
    }
  
    static void OnCardRemoved(uint16_t code) {
      Serial.println();
      Serial.print("Card removed ");
      Serial.println(code);  
    }
};


MFRC522 rfidReader(PIN_SDA, PIN_RST);
SoftwareSerial softSerial(PIN_DFP_RX, PIN_DFP_TX);
DFMiniMp3<SoftwareSerial, Mp3Notify> player(softSerial);

/* 
 * All valid UIDs, one for each folder. 
 */
byte validUids[][4] = {
  //{0xB6, 0xC8, 0x10, 0x1A}, // Tag 0 (Card)
  {0x04, 0x4E, 0x9D, 0x1A}, // Tag 0 (Sticker)
  {0x04, 0x2F, 0x9D, 0x1A}, // Tag 1 (Sticker)
  //{0xC6, 0xEA, 0x3B, 0x1E}, // Tag 1 (Card)
  {0x3C, 0x04, 0x4C, 0xD3}, // Tag 2
  {0x6B, 0x51, 0x4C, 0xD3}, // Tag 3
};

/*
 * Total number of tags/valid UIDs.
 */
int nFolders = sizeof(validUids)/sizeof(byte);

/*
 * Buttons and their callbacks
 */
EasyButton buttonPlay(BUTTON_PLAY);
EasyButton buttonForward(BUTTON_FORWARD);
EasyButton buttonBack(BUTTON_BACK);

uint16_t longpress = 2500;

void onButtonPressPlay() {
  // ignore if volume is beeing set
  if (isSetVolume) {
    isSetVolume = false;
    return;
  }
  if (digitalRead(PIN_DFP_BUSY)) {
    player.start();
    playing = true;
  } else {
    player.pause();
    playing = false;
  }
}

void onButtonPressBack() {
  if (buttonPlay.isPressed()) {
    isSetVolume = true;
    player.decreaseVolume();
    Serial.print("Vol-- | Vol: ");
    Serial.println(player.getVolume());
  } else {
    if (currentTrack == 1) {
      return;
    }
    player.playFolderTrack(oldTag, --currentTrack);
   }
}

void onButtonPressForward() {
  if (buttonPlay.isPressed()) {
    isSetVolume = true;
    if (player.getVolume() > 16) {
      Serial.print("Max. Vol: ");
      Serial.println(player.getVolume());
      return;
    }
    player.increaseVolume();
    Serial.print("Vol++ | Vol: ");
    Serial.println(player.getVolume());
  } else {
  if (currentTrack == nTracks) {
    return;
  }
  continuePlayback(0);
  }
}
 
void onButtonPressedForPlay() {}

void onButtonPressedForForward() {}

void onButtonPressedForBack() {}


/**
 * We need to manually start the next track on onPlayFinished().
 * The callback fires twice for some reason, catch duplicates.
 */
uint16_t lastTrack;
static void continuePlayback(uint16_t track) {
  if ((track == lastTrack) && track != 0) {
    return;
  }
  lastTrack = track;
  player.playFolderTrack(oldTag, ++currentTrack);
}


void setup() {

  buttonPlay.begin();
  buttonForward.begin();
  buttonBack.begin();
  
  buttonPlay.onPressed(onButtonPressPlay);
  buttonForward.onPressed(onButtonPressForward);
  buttonBack.onPressed(onButtonPressBack);

  // buttonPlay.onPressedFor(longpress, onButtonPressedForPlay);
  // buttonForward.onPressedFor(longpress, onButtonPressedForForward);
  // buttonBack.onPressedFor(longpress, onButtonPressedForBack);

  pinMode(LED_PLAY, OUTPUT);
  pinMode(LED_FORWARD, OUTPUT);
  pinMode(LED_BACK, OUTPUT);
  
  pinMode(PIN_DFP_BUSY, INPUT); 

  softSerial.begin(9600);

  SPI.begin();
  rfidReader.PCD_Init();

  Serial.begin(9600);
  Serial.println("Initializing...");

  player.begin();

  player.setVolume(8);  //Set volume value. From 0 to 30
}


void loop() {

  // light when pressing buttons
  digitalWrite(LED_PLAY, !digitalRead(BUTTON_PLAY));
  digitalWrite(LED_FORWARD, !digitalRead(BUTTON_FORWARD));
  digitalWrite(LED_BACK, !digitalRead(BUTTON_BACK));

  player.loop();

  buttonPlay.read();
  buttonForward.read();
  buttonBack.read();

  cardPresent = rfidReader.PICC_IsNewCardPresent();
  rfidState = rfidReader.PICC_ReadCardSerial();

  /* 
   * PICC_IsNewCardPresent() and PICC_ReadCardSerial() return false every
   * 2nd iteration even if a tag is present. oldState keeps track of two 
   * consecutive "false" which should indicate that a tag has indeed been 
   * removed. Great stuff. 
   */
  if (!cardPresent && !rfidState && oldState) {
    oldState = false; 
  } else if (!cardPresent && !rfidState && !oldState) {
    oldState = true;
    player.pause();
  } else if (cardPresent && rfidState) {
    oldState = true;
    newTag = checkUID(rfidReader.uid.uidByte);
    // new card detected
    if ((newTag > -1) && (newTag != oldTag)) {
      blinkTagFound();
      Serial.print("Detected new valid tag #");
      Serial.println(newTag);
      player.pause();
      currentTrack = 1;
      nTracks = player.getFolderTrackCount(newTag);
      Serial.println(nTracks);
      player.playFolderTrack(newTag, currentTrack);
      oldTag = newTag;
      playing = true;
    // same card as before
    } else if (newTag == oldTag){
      if (playing) {
        player.start();
      }
    } else {
      Serial.println("Unknown or invalid Tag.");
    }
  } 
       
  player.loop();
}


void blinkTagFound() {
  digitalWrite(LED_BACK, HIGH);
  delay(150);
  digitalWrite(LED_PLAY, HIGH);
  delay(150);
  digitalWrite(LED_FORWARD, HIGH);
  delay(150);
  digitalWrite(LED_BACK, LOW);
  delay(150);
  digitalWrite(LED_PLAY, LOW);
  delay(150);
  digitalWrite(LED_FORWARD, LOW);
}


int checkUID(byte uid[8]) {

  for (int i = 0; i < nFolders; i++) {
    if ((uid[0] == validUids[i][0]) &&
        (uid[1] == validUids[i][1]) &&
        (uid[2] == validUids[i][2]) &&
        (uid[3] == validUids[i][3])) {
          return i+1;
    }
  }
  return -1;
}
