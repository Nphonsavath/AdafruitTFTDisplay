#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TouchScreen.h>

// Define pins for ILI9341
#define TFT_RESET 8
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_MISO 12
#define TFT_CLK 13

// Touchscreen pins for ILI9341
#define YP A1
#define XM A2
#define YM A0
#define XP A3

// Touchscreen calibration settings
#define TS_MINX 0
#define TS_MAXX 240
#define TS_MINY 0
#define TS_MAXY 320
#define TS_THRESHOLD 200

#define SCAN_NETWORK_X 35
#define SCAN_NETWORK_Y 50
#define BUTTON_W 250
#define BUTTON_H 50

int currentPage = 0;

// Create display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RESET, TFT_MISO);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, TS_THRESHOLD); 

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  if (currentPage == 0) {
    displayNetworkPage();
  }
}

void loop() {
  TSPoint touch = ts.getPoint();
  
  if (touch.z > TS_THRESHOLD) {
    int x = map(touch.x, TS_MINX, TS_MAXX, 0, tft.width());
    int y = map(touch.y, TS_MINY, TS_MAXY, 0, tft.height());
    if (isButtonPressed(x, y, SCAN_NETWORK_X, SCAN_NETWORK_Y, BUTTON_W, BUTTON_H)) {
      scanNearbyNetworks();
    }
    delay(500);
  }

  // Refresh network list every 30 seconds
  //delay(30000);
  //listNetworksToTFT();
} 

bool isButtonPressed(int touchX, int touchY, int buttonX, int buttonY, int buttonW, int buttonH) {
  if ((touchX >= buttonX && touchX <= buttonX + buttonW) && (touchY >= buttonY && touchY <= buttonY + buttonH)) {
    return true;
  }
}

void displayNetworkPage() {
  initializeTFT();
  drawNetworksPage();
  checkWifiModule();
  printMACAddress();
  listNetworksToTFT();
}

void initializeTFT() {
  tft.begin();
  tft.setRotation(1); // Portrait orientation
  tft.fillScreen(ILI9341_BLACK);
}

void drawTitle() {
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(25, 10);
  tft.println("Network Settings");
}

void checkWifiModule() {
  if (WiFi.status() == WL_NO_MODULE) {
    tft.setCursor(10, 50);
    tft.setTextColor(ILI9341_RED);
    tft.println("WiFi module failed!");
    while (true);
  }
}

void printMACAddress() {
  byte mac[6];
  WiFi.macAddress(mac);
  tft.setTextSize(1);
  tft.setCursor(25, 35);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("MAC Address: ");
  printMacAddressToTFT(mac);
}

void listNetworksToTFT() {
  // Clear previous network list
  //tft.fillRect(0, 120, 320, 200, ILI9341_BLACK);
  //tft.setCursor(10, 70);
  //tft.setTextColor(ILI9341_WHITE);
  //tft.setTextSize(3);

  // Scan for nearby networks
  int numSsid = scanNearbyNetworks();

  // Display networks
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    //tft.print(thisNet + 1);
    //tft.print(") ");
    tft.print(WiFi.SSID(thisNet));
    //tft.print(" (");
    // tft.print(WiFi.RSSI(thisNet)); //prints network signal strength
    // tft.print(" dBm) ");

    //tft.setTextColor(ILI9341_CYAN); //prints network encription
    //printEncryptionTypeToTFT(WiFi.encryptionType(thisNet));
  }
}

void drawNetworksPage() {
  drawTitle();

  tft.fillRect(SCAN_NETWORK_X, SCAN_NETWORK_Y, BUTTON_W, BUTTON_H, ILI9341_BLUE);
  tft.drawRect(SCAN_NETWORK_X, SCAN_NETWORK_Y, BUTTON_W, BUTTON_H, ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(45, 65);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Scan Networks");


}

int scanNearbyNetworks() {
  int numSsid = WiFi.scanNetworks();
  tft.setTextSize(2);
  tft.setCursor(55, 110);
  if (numSsid == -1) {
    tft.setTextColor(ILI9341_RED);
    tft.print("No networks found!");
    return 0;
  } else {
    tft.print("List of Networks");
  }
  return numSsid;
}

void printEncryptionTypeToTFT(int thisType) {
  switch (thisType) {
    case ENC_TYPE_WEP:
      tft.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      tft.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      tft.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      tft.println("Open");
      break;
    case ENC_TYPE_AUTO:
      tft.println("Auto");
      break;
    default:
      tft.println("Unknown");
      break;
  }
}

void printMacAddressToTFT(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      tft.print("0");
    }
    tft.print(mac[i], HEX);
    if (i > 0) {
      tft.print(":");
    }
  }
  tft.println();
}
