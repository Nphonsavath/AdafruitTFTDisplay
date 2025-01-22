#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Define pins for ILI9341
#define TFT_RESET 8
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_MISO 12
#define TFT_CLK 13

// Create display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RESET, TFT_MISO);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  // Initialize the TFT display
  tft.begin();
  tft.setRotation(2); // Landscape orientation
  tft.fillScreen(ILI9341_BLACK);

  // Display title
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Network Settings");

  // Check for WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    tft.setCursor(10, 50);
    tft.setTextColor(ILI9341_RED);
    tft.println("WiFi module failed!");
    while (true);
  }

  // Check firmware version
  String firmwareVersion = WiFi.firmwareVersion();
  if (firmwareVersion < WIFI_FIRMWARE_LATEST_VERSION) {
    tft.setCursor(10, 50);
    tft.setTextColor(ILI9341_YELLOW);
    tft.println("Upgrade firmware!");
  }

  // Print MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  tft.setTextSize(1);
  tft.setCursor(10, 80);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("MAC Address: ");
  printMacAddressToTFT(mac);

  // Display network scan info
  listNetworksToTFT();
}

void loop() {
  // Refresh network list every 10 seconds
  delay(10000);
  listNetworksToTFT();
}

void listNetworksToTFT() {
  // Clear previous network list
  tft.fillRect(0, 120, 320, 200, ILI9341_BLACK);
  tft.setCursor(10, 120);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);

  // Scan for nearby networks
  tft.println("Scanning for networks...");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    tft.setTextColor(ILI9341_RED);
    tft.println("No networks found!");
    return;
  }

  // Display networks
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    tft.setTextColor(ILI9341_WHITE);
    tft.print(thisNet + 1);
    tft.print(") ");
    tft.print(WiFi.SSID(thisNet));
    tft.print(" (");
    tft.print(WiFi.RSSI(thisNet));
    tft.print(" dBm) ");

    tft.setTextColor(ILI9341_CYAN);
    printEncryptionTypeToTFT(WiFi.encryptionType(thisNet));
  }
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
