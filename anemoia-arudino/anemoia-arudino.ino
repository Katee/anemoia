#include <Wire.h>
#include <SPI.h>
#include "FastLED.h"

#define BAUD_RATE 9600
#define TAG_LENGTH 7

#define NUM_TAGS   7

long interruptEndsAt = 0;
long playedHelloAt = 0;
long becomeAnnoyedAfter = 5000;
bool playedAnnoyed = false;
bool playedMusicalGoal = false;
bool playedFamilyGoal = false;
bool playedNavigationalGoal = false;
bool playedHistoryGoal = false;

struct tag {
  String name;
  String tag;
  int ledIndex;
  long audioLength;
  CRGB color;
  long scannedAt;
};

struct tag tags[NUM_TAGS];

String lastScannedTag;

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

#define PINPIXELS   9
#define NUMPIXELS   6
CRGB leds[NUMPIXELS];

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void setup(void) {
  Serial.begin(BAUD_RATE);
  // search for "struct tag" to see what these values correspond to
  tags[0] = (struct tag) { "paper1", "0x40x1760x1510x2340x2010x720x128", 0, 5000, CHSV(random8(64), 150, 255), 0 };
  tags[1] = (struct tag) { "paper2", "0x40x1560x1510x2340x2010x720x128", 1, 5000, CHSV(random8(64), 150, 255), 0 };
  tags[2] = (struct tag) { "paper3", "0x40x1420x1510x2340x2010x720x128", 2, 5000, CHSV(random8(64), 150, 255), 0 };
  tags[3] = (struct tag) { "paper4", "0x40x1410x1510x2340x2010x720x128", 3, 5000, CHSV(random8(64), 150, 255), 0 };

  FastLED.addLeds<WS2812, PINPIXELS>(leds, NUMPIXELS);
  updateCloudLights();

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();
  
  // dont't wait forvever
  nfc.setPassiveActivationRetries(0x02);

  Serial.println("Waiting to scan object ...");
}

int lastScannedTagIndex = -1;

void loop(void) {
  updateCloudLights();
  
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    if (uidLength != TAG_LENGTH) {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
      return;
    }
    
    Serial.print(".");
    String scannedTag = tagToString(uid);

    // it is the same as our last scanned tag, ignore
    if (scannedTag == lastScannedTag) {
      return;
    } else {
      lastScannedTag = scannedTag;
    }

    Serial.println("\nscanned tag: " + scannedTag);

    // play random interruption 
    if (millis() < interruptEndsAt) {
      interruptEndsAt = 0;
      Serial.println("interrupted0" + String(random(1, 3)));

      tags[lastScannedTagIndex].scannedAt = 0;
      
      return;
    }

    for (int i = 0; i < NUM_TAGS; i++) {
      // ignore the ones that don't match
      if (scannedTag != tags[i].tag) {
        continue;
      }

      lastScannedTagIndex = i;
      
      tags[i].scannedAt = millis();
      interruptEndsAt = tags[i].scannedAt + tags[i].audioLength;

      Serial.println(tags[i].name);
    }

    if (tags[0].scannedAt != 0 && tags[1].scannedAt != 0 && !playedMusicalGoal) {
      playedMusicalGoal = true;
      Serial.println("musicgoal");
      leds[0] = CRGB(255, 255, 0);
      delay(3000);
    }
    if (tags[2].scannedAt != 0 && tags[3].scannedAt != 0 && !playedNavigationalGoal) {
      playedNavigationalGoal = true;
      Serial.println("navigationalgoal");
      leds[1] = CRGB(65, 150, 0);
      delay(3000);
    }
    if (tags[4].scannedAt != 0 && tags[5].scannedAt != 0 && !playedFamilyGoal) {
      playedFamilyGoal = true;
      Serial.println("familygoal");
      leds[2] = CRGB(0, 0, 250);
      delay(3000);
    }
    if (tags[0].scannedAt != 0 && tags[1].scannedAt != 0 && tags[6].scannedAt != 0 && !playedHistoryGoal) {
      playedHistoryGoal = true;
      Serial.println("historygoal");
      leds[7] = CRGB(90, 0, 250);
      delay(3000);
    }
  }

  if (!hasPlayedHello()) {
    int sensorValue = analogRead(A0);
    if (sensorValue > 100) {
      Serial.println("\nHello0" + String(random(1, 6)));
      playedHelloAt = millis();
    }
  }
  
  if (!playedAnnoyed && !hasScannedTag() && hasPlayedHello() && ((millis() - playedHelloAt) > becomeAnnoyedAfter)) {
    Serial.println("\nannoyed0" + String(random(1, 2)));
    playedAnnoyed = true;
  }
}

String tagToString(uint8_t chartag[]) {
  String stringTag = "";
  
  for (uint8_t i = 0; i < TAG_LENGTH; i++) {
    stringTag = stringTag + "0x" + chartag[i];
  }
  
  return stringTag;
}

bool hasScannedTag() {
  for (int i = 0; i < NUM_TAGS; i++) {
    if (tags[i].scannedAt != 0) {
      return true;
    }
  }
  return false;
}

bool hasPlayedHello() {
  return playedHelloAt != 0;
}

void updateCloudLights() {
  // base "cloud" pattern
  for(uint16_t i = 0; i < NUMPIXELS; i++) {
    CRGB color = CHSV(random8(64), 150, 100);
    leds[i] = color;
  }

  for (int i = 0; i < NUM_TAGS; i++) {
    // if this LED is the audio playing and we haven't been interu
    if (tags[i].scannedAt != 0) {
      leds[tags[i].ledIndex] = tags[i].color;
    }
  }

  FastLED.show();
}

