#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "FastLED.h"

#define BAUD_RATE 9600
#define TAG_LENGTH 7

#define NUM_TAGS   25

long interruptEndsAt = 0;
long playedHelloAt = 0;
long becomeAnnoyedAfter = 5000;
bool playedAnnoyed = false;
bool playedMusicalGoal = false;
bool playedFamilyGoal = false;
bool playedNavigationalGoal = false;
bool playedHistoryGoal = false;

struct tag {
  String tag;
  String name;
  int ledIndex;
  long audioLength;
  CRGB color;
  unsigned long scannedAt;
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
  tags[0] = (struct tag)  { "0x40x1410x1510x2340x2010x720x128", "foot", 0, 5000, {random(1, 256), 150, 255}, 0 }; // 0 chicken foot
  tags[1] = (struct tag)  { "0x40x1760x1510x2340x2010x720x128", "vial", 1, 5000, {random(1, 256), 150, 255}, 0 }; // 1 dirt vial  
  tags[2] = (struct tag)  { "0x40x1220x1510x2340x2010x720x128", "knuckle", 2, 5000, {random(1, 256), 150, 255}, 0 }; // 2 knuckle yellow tag
  tags[3] = (struct tag)  { "0x40x1420x1510x2340x2010x720x128", "egg", 3, 5000, {random(1, 256), 150, 255}, 0 }; // 3 fancy egg
  tags[4] = (struct tag)  { "0x40x1100x1510x2340x2010x720x129", "jaw", 0, 5000, {random(1, 256), 150, 255}, 0 }; // 4 jaw bone red tag
  tags[5] = (struct tag)  { "0x40x1210x1510x2340x2010x720x128", "pebble", 1, 5000, {random(1, 256), 150, 255}, 0 }; // 5 pebble vial yellow tag
  tags[6] = (struct tag)  { "0x40x1250x1510x2340x2010x720x129", "bell", 2, 5000, {random(1, 256), 150, 255}, 0 }; // 6 big globe bell red tag
  tags[7] = (struct tag)  { "0x40x750x1700x2340x2010x720x129", "block", 3, 5000, {random(1, 256), 150, 255}, 0 }; //7 weird tiny block box pattern yellow tag
  tags[8] = (struct tag)  { "0x40x1560x1510x2340x2010x720x128", "globe", 0, 5000, {random(1, 256), 150, 255}, 0 }; // 8 globe compass thing red tag
  tags[9] = (struct tag)  { "0x40x1350x1510x2340x2010x720x128", "bottle", 1, 5000, {random(1, 256), 150, 255}, 0 }; // 9 small green bottle yellow tag
  tags[10] = (struct tag) { "0x40x1110x1510x2340x2010x720x129", "compass", 2, 5000, {random(1, 256), 150, 255}, 0 }; // 10 compass red tag
  tags[11] = (struct tag) { "0x40x1300x1510x2340x2010x720x129", "tea", 3, 5000, {random(1, 256), 150, 255}, 0 }; //11 bottle of tea red tag
  tags[12] = (struct tag) { "0x40x1020x1510x2340x2010x720x128", "spine", 0, 5000, {random(1, 256), 150, 255}, 0 }; // 12 spine yellow tag
  tags[13] = (struct tag) { "0x40x1440x1510x2340x2010x720x129", "mousejaw", 1, 5000, {random(1, 256), 150, 255}, 0 }; //13 bottle of mouse jaws red tag
  tags[14] = (struct tag) { "0x40x1360x1510x2340x2010x720x128",  "birdes", 2, 5000, {random(1, 256), 150, 255}, 0 }; //14 bird bones yellow tag
  tags[15] = (struct tag) { "0x40x890x1700x2340x2010x720x129",  "box", 3, 5000, {random(1, 256), 150, 255}, 0 }; // 15 music box yellow tag
  tags[16] = (struct tag) { "0x40x1310x1510x2340x2010x720x129", "barth", 0, 5000, {random(1, 256), 150, 255}, 0 }; // 16 Bartholomew red tag
  tags[16] = (struct tag) { "0x40x30x1510x2340x2010x720x129",  "feathers", 1, 5000, {random(1, 256), 150, 255}, 0 }; // 17 feathers and beads yellow tag
  tags[18] = (struct tag) { "0x40x2450x1510x2340x2010x720x128", "scroll", 2, 5000, {random(1, 256), 150, 255}, 0 }; // 18 scroll silver tag
  tags[19] = (struct tag) { "0x40x530x1700x2340x2010x720x129", "cow", 3, 5000, {random(1, 256), 150, 255}, 0 }; // 19 cow tooth yellow tag
  tags[20] = (struct tag) { "0x40x1450x1510x2340x2010x720x129", "knife", 0, 5000, {random(1, 256), 150, 255}, 0 }; // 20 knife silver tag
  tags[21] = (struct tag) { "0x40x1110x1700x2340x2010x720x129", "whistle", 1, 5000, {random(1, 256), 150, 255}, 0 }; // 21 whistle yellow tag
  tags[22] = (struct tag) { "0x40x740x1700x2340x2010x720x129", "tbell", 2, 5000, {random(1, 256), 150, 255}, 0 }; // 22 tiny bell yellow tag
  tags[23] = (struct tag) { "0x40x1250x1500x2340x2010x720x129", "bluevial", 3, 5000, {random(1, 256), 150, 255}, 0 }; // 23 tiny blue vial inside book
  tags[24] = (struct tag) { "0x40x880x1700x2340x2010x720x129", "skull", 2, 5000, {random(1, 256), 150, 255}, 0 }; //24 3d printed skull attached to book

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

