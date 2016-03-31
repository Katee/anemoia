
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>  
#include <Adafruit_PN532.h>
#define BAUD_RATE 9600
String scannedTag;
typedef struct{
  String ID;
  String name;
  float audioLength;
  long scannedAt;
  bool hasScanned;
} tag ;
tag tagList[25];
//
//String tags[25][2] = { {"0x40x1410x1510x2340x2010x720x128", "foot"}, // 0 chicken foot
//  { "0x40x1760x1510x2340x2010x720x128", "vial"}, // 1 dirt vial
//  { "0x40x1220x1510x2340x2010x720x128", "knuckle"},  // 2 knuckle yellow tag
//  { "0x40x1420x1510x2340x2010x720x128", "egg"}, // 3 fancy egg
//  { "0x40x1100x1510x2340x2010x720x129", "jaw"},// 4 jaw bone red tag
//  { "0x40x1210x1510x2340x2010x720x128", "pebble"},// 5 pebble vial yellow tag
//  { "0x40x1250x1510x2340x2010x720x129", "bell"},// 6 big globe bell red tag
//  { "0x40x750x1700x2340x2010x720x129", "block"},//7 weird tiny block box pattern yellow tag
//  { "0x40x1560x1510x2340x2010x720x128", "globe"},// 8 globe compass thing red tag
//  { "0x40x1350x1510x2340x2010x720x128", "bottle"},// 9 small green bottle yellow tag
//  { "0x40x1110x1510x2340x2010x720x129", "compass"},// 10 compass red tag
//  { "0x40x1300x1510x2340x2010x720x129", "tea"},//11 bottle of tea red tag
//  { "0x40x1020x1510x2340x2010x720x128", "spine"},// 12 spine yellow tag
//  { "0x40x1440x1510x2340x2010x720x129", "mousejaw"},//13 bottle of mouse jaws red tag
//  { "0x40x1360x1510x2340x2010x720x128",  "birdes"},// 14 bird bones yellow tag
//  { "0x40x890x1700x2340x2010x720x129",  "box"},// 15 music box yellow tag
//  { "0x40x1310x1510x2340x2010x720x129", "barth"},// 16 Bartholomew red tag
//  { "0x40x30x1510x2340x2010x720x129",  "feathers"},// 17 feathers and beads yellow tag
//  { "0x40x2450x1510x2340x2010x720x128", "scroll"},// 18 scroll silver tag
//  { "0x40x530x1700x2340x2010x720x129", "cow"},// 19 cow tooth yellow tag
//  { "0x40x1450x1510x2340x2010x720x129", "knife"},// 20 knife silver tag
//  { "0x40x1110x1700x2340x2010x720x129", "whistle"},// 21 whistle yellow tag
//  { "0x40x740x1700x2340x2010x720x129", "tbell"},// 22 tiny bell yellow tag
//  { "0x40x1250x1500x2340x2010x720x129", "bluevial"},// 23 tiny blue vial inside book
//  { "0x40x880x1700x2340x2010x720x129", "skull"}//24 3d printed skull attached to book
//};
//
//bool tagState[25] = { false, false, false, false, false, false,
//                      false, false, false, false, false, false,
//                      false, false, false, false, false, false,
//                      false, false, false, false, false, false,
//                      false
//                    };

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)
#define PIN            9
#define NUMPIXELS      16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
void setup(void) {
  tagList[0] = { "0x40x1410x1510x2340x2010x720x128", "foot", 60000, 0, false};

  Serial.begin(BAUD_RATE);
  Serial.println("Hello!");
  nfc.begin();
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.Color(0, 0, 0); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    //Serial.println("Found an ISO14443A card");
    Serial.flush();
    if (uidLength == 7) {
      for (uint8_t i = 0; i < uidLength; i++)
      {
        scannedTag = scannedTag + "0x" + uid[i];
      }
      for (int i = 0; i < sizeof(tagList); i++)  {
        check(scannedTag, i);
      }
      scannedTag = "";
    }
    else
    {
      //Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }
    Serial.flush();
  }
}
void check(String scan, int check) {
  if (scan == tagList[check].ID) {
    if (tagList[check].hasScanned != true) {
      tagList[check].hasScanned = true;
      Serial.println(tagList[check].name);
    }
  } else {
    //  Serial.println("NOO");
  }
}
