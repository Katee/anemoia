#include <Wire.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>  
#include <Adafruit_PN532.h>
String scannedTag;
int interuptCount = 0;
typedef struct{
  String ID;
  String name;
  float audioLength;
  long scannedAt;
  bool hasScanned;
} tag ;
tag tagList[25];
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
  tagList[1] = { "0x40x1220x1510x2340x2010x720x128", "knuckle", 60000, 0, false};
  tagList[2] = { "0x40x1420x1510x2340x2010x720x128", "pear", 60000, 0, false};
  tagList[3] = { "0x40x1100x1510x2340x2010x720x129", "jawbone", 60000, 0, false};
  tagList[4] = { "0x40x1210x1510x2340x2010x720x128", "pebblevial", 60000, 0, false};
  tagList[5] = { "0x40x1250x1510x2340x2010x720x129", "cowbell", 60000, 0, false};
  tagList[6] = { "0x40x750x1700x2340x2010x720x129", "scentedbox", 60000, 0, false};
  tagList[7] = { "0x40x1560x1510x2340x2010x720x128", "globe", 60000, 0, false};
  tagList[8] = { "0x40x1350x1510x2340x2010x720x128", "smallbottle", 60000, 0, false};
  tagList[9] = { "0x40x1110x1510x2340x2010x720x129", "compass", 60000, 0, false};
  tagList[10] = { "0x40x1300x1510x2340x2010x720x129", "tea", 60000, 0, false};
  tagList[11] = { "0x40x1020x1510x2340x2010x720x128", "spine", 60000, 0, false};
  tagList[12] = { "0x40x1440x1510x2340x2010x720x129", "mousejaws", 60000, 0, false};
  tagList[13] = { "0x40x1360x1510x2340x2010x720x128", "birdbones", 60000, 0, false};
  tagList[14] = { "0x40x890x1700x2340x2010x720x129", "musicbox", 60000, 0, false};
  tagList[15] = { "0x40x1310x1510x2340x2010x720x129", "barth", 60000, 0, false};  
  tagList[16] = { "0x40x30x1510x2340x2010x720x129", "feathervial", 60000, 0, false};
  tagList[17] = { "0x40x530x1700x2340x2010x720x129", "cowtooth", 60000, 0, false};
  tagList[18] = { "0x40x1110x1700x2340x2010x720x129", "whistle", 60000, 0, false};
  tagList[19] = { "0x40x740x1700x2340x2010x720x129", "smallbell", 60000, 0, false};
  tagList[20] = { "0x40x1250x1500x2340x2010x720x129", "crowvial", 60000, 0, false};
  tagList[21] = { "0x40x880x1700x2340x2010x720x129", "dogskull", 60000, 0, false};
  tagList[22] = { "0x40x1450x1510x2340x2010x720x129", "knife", 60000, 0, false};
  tagList[23] = { "0x40x2450x1510x2340x2010x720x128", "scroll", 60000, 0, false};
  Serial.begin(9600);
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
        if(interuptCount > 2){
         check(scannedTag, i);
        }
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
      tagList[check].scannedAt = (millis());
      checkInterupt(tagList[check].scannedAt);
      Serial.println(tagList[check].name);
    }
  } else {
    //  Serial.println("NOO");
  }
}
void checkInterupt(long startTime){
  if(millis() < (startTime + 60000)){ 
    if(interuptCount < 3){
     Serial.println("reginterupt");
     interuptCount++;
    } else{ 
      Serial.println("interuptoverlaod");
    }
  }
}

