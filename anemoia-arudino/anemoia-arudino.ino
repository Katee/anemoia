#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "FastLED.h"

#define BAUD_RATE 9600
#define TAG_LENGTH 7

#define NUM_TAGS   27

#define LIGHTING_UPDATE_TIME 100
#define ANNOYED_TIMEOUT 90000

unsigned long lastLightingUpdate = 0;
unsigned long interruptEndsAt = 0;
unsigned long playedHelloAt = 0;
bool playedAnnoyed = false;

unsigned int timesInterrupted = 0;
#define MAX_TIMES_INTERUPPTED 3

struct tag {
  String tag;
  String name;
  int ledIndex;
  long audioLength;
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
#define NUMPIXELS   27
CRGB leds[NUMPIXELS];

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

unsigned long loopTime = 0;

unsigned int knifeIndex = 23;
unsigned int scrollIndex = 24;
unsigned long inKnifeEndStateAt = 0;
unsigned long inScrollEndStateAt = 0;

void setup(void) {
  Serial.begin(BAUD_RATE);
  // search for "struct tag" to see what these values correspond to
  tags[0] =  { "0x40x1410x1510x2340x2010x720x128", "foot", 0, 60000, 0};
  tags[1] =  { "0x40x1220x1510x2340x2010x720x128", "knuckle", 1, 60000, 0};
  tags[2] =  { "0x40x1420x1510x2340x2010x720x128", "pear", 2, 60000, 0};
  tags[3] =  { "0x40x1100x1510x2340x2010x720x129", "jawbone", 3, 60000, 0};
  tags[4] =  { "0x40x1210x1510x2340x2010x720x128", "pebblevial", 4, 60000, 0};
  tags[5] =  { "0x40x1250x1510x2340x2010x720x129", "cowbell", 5, 60000, 0};
  tags[6] =  { "0x40x750x1700x2340x2010x720x129",  "scentedbox", 6, 60000, 0};
  tags[7] =  { "0x40x1560x1510x2340x2010x720x128", "globe", 7, 60000, 0};
  tags[8] =  { "0x40x1350x1510x2340x2010x720x128", "smallbottle", 8, 60000, 0};
  tags[9] =  { "0x40x1110x1510x2340x2010x720x129", "compass", 9, 60000, 0};
  tags[10] = { "0x40x1300x1510x2340x2010x720x129", "tea", 10, 60000, 0};
  tags[11] = { "0x40x1020x1510x2340x2010x720x128", "spine", 11, 60000, 0};
  tags[12] = { "0x40x1440x1510x2340x2010x720x129", "mousejaws", 12, 60000, 0};
  tags[13] = { "0x40x1360x1510x2340x2010x720x128", "birdbones", 13, 60000, 0};
  tags[14] = { "0x40x890x1700x2340x2010x720x129",  "musicbox", 14, 60000, 0};
  tags[15] = { "0x40x1310x1510x2340x2010x720x129", "barth", 15, 60000, 0};
  tags[16] = { "0x40x30x1510x2340x2010x720x129",   "feathervial", 16, 60000, 0};
  tags[17] = { "0x40x530x1700x2340x2010x720x129",  "cowtooth", 17, 60000, 0};
  tags[18] = { "0x40x1110x1700x2340x2010x720x129", "whistle", 18, 60000, 0};
  tags[19] = { "0x40x740x1700x2340x2010x720x129",  "smallbell", 19, 60000, 0};
  tags[20] = { "0x40x1250x1500x2340x2010x720x129", "crowvial", 20, 60000, 0};
  tags[21] = { "0x40x880x1700x2340x2010x720x129",  "dogskull", 21, 60000, 0};
  tags[22] = { "0x40x1760x1510x2340x2010x720x128",  "vial", 22, 60000, 0};
  tags[23] = { "0x40x1450x1510x2340x2010x720x129", "knife", 23, 60000, 0};
  tags[24] = { "0x40x2450x1510x2340x2010x720x128", "scroll", 24, 60000, 0};
  tags[25] = { "0x40x2440x1510x2340x2010x720x128", "book", 25, 60000, 0};
  tags[26] = { "0x40x1400x1500x2340x2010x720x129", "chickenbone", 26, 60000, 0};

  FastLED.addLeds<WS2812, PINPIXELS, GRB>(leds, NUMPIXELS);
  FastLED.setTemperature(Tungsten40W);

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Abort! No RFID reader found.");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found RFID reader"); Serial.println((versiondata >> 24) & 0xFF, HEX);

  // configure board to read RFID tags
  nfc.SAMConfig();
  
  // dont't wait forvever
  nfc.setPassiveActivationRetries(0x02);

  // wait until they trigger the pressure sensor
  while (playedHelloAt == 0) {
    loopTime = millis();
    updateCloudLights(loopTime);
    
    int sensorValue = analogRead(A0);
    Serial.println("pressure sensor: " + String(sensorValue));
    if (sensorValue > 100) {
      playAudio("hello0" + String(random(1, 6)));
      playedHelloAt = loopTime;
    }
  }
}

int lastScannedTagIndex = -1;

void loop(void) {
  loopTime = millis();

  updateCloudLights(loopTime);
  
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

    // handle a unique tag scan
    handleScannedTag(scannedTag);
  }

  if (!playedAnnoyed && !hasScannedTag()
      && ((loopTime - playedHelloAt) > ANNOYED_TIMEOUT)) {
    playAudio("annoyed0" + String(random(1, 2)));
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

void updateCloudLights(unsigned long currentTime) {
  if ((loopTime - lastLightingUpdate) > LIGHTING_UPDATE_TIME) {
    lastLightingUpdate = currentTime;
  } else {
    return;
  }
  
  // base candle like "cloud" pattern
  for(uint16_t i = 0; i < NUMPIXELS; i++) {
    CRGB color = CHSV(random(30, 41), random(150, 255), 100);
    leds[i] = color;
  }

  for (int i = 0; i < NUM_TAGS; i++) {
    // if this LED is the audio playing and we haven't been interu
    if (tags[i].scannedAt != 0) {
      leds[tags[i].ledIndex] = CHSV(random(30, 41), random(150, 255), 255);
    }
  }

  FastLED.show();
}

void handleScannedTag(String scannedTag) {
  // after the knife or scroll are scanned only play their "done" audio
  if (inKnifeEndStateAt != 0 && inKnifeEndStateAt < millis()) {
    playAudio("knife-done");
    return;
  } else if (inScrollEndStateAt != 0 && inScrollEndStateAt < millis()) {
    playAudio("scroll-done");
    return;
  }

  // play audio if another tag is scanned before current audio is done
  if (handleScanTooSoon()) {
    return;
  }

  for (int i = 0; i < NUM_TAGS; i++) {
    // ignore the ones that don't match
    if (scannedTag != tags[i].tag) {
      continue;
    }

    lastScannedTagIndex = i;

    if (lastScannedTagIndex == knifeIndex) {
      inKnifeEndStateAt = millis() + tags[i].audioLength;
    } else if (lastScannedTagIndex == scrollIndex) {
      inScrollEndStateAt = millis() + tags[i].audioLength;
    }

    // has never scanned this tag before
    if (tags[i].scannedAt == 0) {
      tags[i].scannedAt = loopTime;
      interruptEndsAt = tags[i].scannedAt + tags[i].audioLength;
  
      playAudio(tags[i].name);
    } else {
      playAudio("toldyou0" + String(random(1, 5)));
    }
  }
}

// play random interruption 
bool handleScanTooSoon() {
  if (loopTime < interruptEndsAt) {
    interruptEndsAt = 0;
    tags[lastScannedTagIndex].scannedAt = 0;

    if (timesInterrupted < MAX_TIMES_INTERUPPTED) {
      playAudio("interrupted0" + String(random(1, 3)));
    } else {
      playAudio("leavenow");
      enterEndState();
    }

    timesInterrupted++;

    // if we would have entered an end but were interrupted
    // don't enter that end state
    inKnifeEndStateAt = 0;
    inScrollEndStateAt = 0;
    
    return true;
  }

  return false;
}

void enterEndState() {
  while (1); // halt
}

void playAudio(String audioName) {
  Serial.println("play:" + audioName);
}

