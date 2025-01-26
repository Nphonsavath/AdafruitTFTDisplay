#include <string.h>
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
#define TS_MINX 151
#define TS_MINY 146
#define TS_MAXX 869
#define TS_MAXY 881
#define TS_THRESHOLD 300

#define BUTTON_X 35
#define BUTTON_Y 50
#define BUTTON_W 250
#define BUTTON_H 50
#define MAX_SSID_LENGTH 12

bool arrowsDrawn = false;
int arduinoState = 0;
int networkPage = 1;
int numSsid = 0;
bool isNetworkPage1Drawn = false;
bool isNetworkPage2Drawn = false;
bool isNetworkPage3Drawn = false;

// Create display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RESET, TFT_MISO);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, TS_THRESHOLD); 

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  tft.begin();
  tft.setRotation(1); // Portrait orientation
  tft.fillScreen(ILI9341_BLACK);

  if (arduinoState == 0) {
    initializeNetworkPage();
  }
}

void loop() {
  TSPoint touch = ts.getPoint();
  
  if (touch.z > TS_THRESHOLD) {
    int screen_width = tft.width();  // Example: 240 (for ILI9341)
    int screen_height = tft.height(); // Example: 320 (for ILI9341)
    
    int y = map(touch.x, TS_MINY, TS_MAXY, 240, 0); // Y-axis maps to display X
    int x = map(touch.y, TS_MINX, TS_MAXX, 0, 320); // X-axis maps to display Y

    if (y < 0) y = 0;
    if (y > 240) y = 240;
    if (x < 0) x = 0;
    if (x > 320) x = 320;

    Serial.println(networkPage);
    
    if (networkPage == 1) {
      if (((x >= 300) && (x <= (300 + 20))) && ((y >= 220) && (y <= (220 + 20))) && arrowsDrawn) {
        networkPage = 2;
        tft.fillScreen(ILI9341_BLACK);
        isNetworkPage1Drawn = false;
      } else if (((x >= BUTTON_X) && (x <= (BUTTON_X + BUTTON_W))) && ((y >= BUTTON_Y) && (y <= (BUTTON_Y + BUTTON_H)))) {
        numSsid = scanNearbyNetworks();
        drawNetworksPage1(numSsid);
      // Serial.print(" BUTTON_X ");
      // Serial.print(BUTTON_X);
      // Serial.print(" BUTTON_Y ");
      // Serial.print(BUTTON_Y);
      // Serial.print(" BUTTON_W ");
      // Serial.print(BUTTON_W);
      // Serial.print("BUTTON_H ");
      // Serial.print(BUTTON_H);
      // Serial.println();
      }
    }

    if (networkPage == 2) {
      if (!isNetworkPage2Drawn) {
        drawNetworksPage2(numSsid);
        isNetworkPage2Drawn = true;
      } else if (((x >= 300) && (x <= (300 + 20))) && ((y >= 220) && (y <= (220 + 20))) && arrowsDrawn) {
        networkPage = 3;
        tft.fillScreen(ILI9341_BLACK);
        isNetworkPage2Drawn = false;
      } else if (((x >= 0) && (x <= 20)) && ((y >= 220) && (y <= (220 + 20))) && arrowsDrawn) {
        networkPage = 1;
        tft.fillScreen(ILI9341_BLACK);
        initializeNetworkPage();
        drawNetworksPage1(numSsid);
        isNetworkPage1Drawn = true;
        isNetworkPage2Drawn = false;
      }
    }

    if (networkPage == 3) {
      if (!isNetworkPage3Drawn) {
        drawNetworksPage3(numSsid);
        isNetworkPage3Drawn = true;
      } else if (((x >= 0) && (x <= 20)) && ((y >= 220) && (y <= (220 + 20))) && arrowsDrawn) {
        networkPage = 2;
        tft.fillScreen(ILI9341_BLACK);
        drawNetworksPage2(numSsid);
        isNetworkPage2Drawn = true;
        isNetworkPage3Drawn = false;
      }
    }
    
    Serial.print("x: ");
    Serial.print(x);
    Serial.print(", y: ");
    Serial.print(y);
    // Serial.print("touch.x: ");
    // Serial.print(touch.x);
    // Serial.print("touch.y: ");
    // Serial.print(touch.y);
    // Serial.println();
    delay(500);
  }

  // Refresh network list every 30 seconds
  //delay(30000);
  //listNetworksToTFT();
} 

bool isButtonPressed(int touchX, int touchY, int buttonX, int buttonY, int buttonW, int buttonH) {
  if (((touchX >= buttonX) && (touchX <= (buttonX + buttonW))) && ((touchY >= buttonY) && (touchY <= (buttonY + buttonH)))) {
    Serial.println("*** TouchX: ");
    Serial.print(touchX);
    Serial.print(" buttonX: ");
    Serial.print(buttonX);
    Serial.print(" buttonW: ");
    Serial.print(buttonW);
    Serial.print(" touchY: ");
    Serial.print(touchY);
    Serial.print(" buttonH: ");
    Serial.print(buttonH);
    return true;
  }
}

void initializeNetworkPage() {
  checkWifiModule();
  drawTitle();
  printMACAddress();
  drawButton(45, 55, BUTTON_X, BUTTON_Y-5, BUTTON_W, BUTTON_H-10, ILI9341_BLUE, 3, "Scan Networks");
  isNetworkPage1Drawn = true;
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

void drawRightArrow(int x, int y, int size) {
  tft.drawLine(x, y, x + size, y, ILI9341_WHITE); // Horizontal line
  tft.drawLine(x + size, y, x + size - size / 2, y - size / 2, ILI9341_WHITE); // Top diagonal line
  tft.drawLine(x + size, y, x + size - size / 2, y + size / 2, ILI9341_WHITE); // Bottom diagonal line
}

void drawLeftArrow(int x, int y, int size) {
  tft.drawLine(x, y, x - size, y, ILI9341_WHITE); // Horizontal line
  tft.drawLine(x - size, y, x - size + size / 2, y - size / 2, ILI9341_WHITE); // Top diagonal line
  tft.drawLine(x - size, y, x - size + size / 2, y + size / 2, ILI9341_WHITE); // Bottom diagonal line
}

void drawNetworksPage1(int numSsid) {
  //drawButton(45, 55, BUTTON_X, BUTTON_Y-5, BUTTON_W, BUTTON_H-10, ILI9341_BLUE, 3, "Scan Networks");
  tft.setTextSize(1);
  tft.setCursor(100, 90);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("List of Networks");
  for (int thisNet = 0; thisNet < min(3, numSsid); thisNet++) {
    int offset = ((thisNet+1)*45);
    String wifiName = WiFi.SSID(thisNet);
    if (wifiName.length() > MAX_SSID_LENGTH) {
      wifiName = wifiName.substring(0, MAX_SSID_LENGTH) + "...";
    }
    drawButton(45, 75 + offset, BUTTON_X, 60 + offset, BUTTON_W, BUTTON_H-10, ILI9341_DARKGREY, 2, wifiName);
    tft.setTextColor(ILI9341_CYAN); //prints network encription
    tft.setTextSize(1);
    tft.setCursor(233, 70 + offset);
    tft.print(WiFi.RSSI(thisNet)); //prints network signal strength
    tft.print(" dBm");
    tft.setCursor(233, 85 + offset);
    printEncryptionTypeToTFT(WiFi.encryptionType(thisNet));
  }
  arrowsDrawn = true;
  drawRightArrow(300, 230, 20);
}

void drawNetworksPage2(int numSsid) {
  drawTitle();
  printMACAddress();
  for (int thisNet = 3; thisNet < min(7, numSsid); thisNet++) {
    int offset = ((thisNet-3)*45);
    String wifiName = WiFi.SSID(thisNet);
    if (wifiName.length() > MAX_SSID_LENGTH) {
      wifiName = wifiName.substring(0, MAX_SSID_LENGTH) + "...";
    }
    drawButton(45, 65 + offset, BUTTON_X, 50 + offset, BUTTON_W, BUTTON_H-10, ILI9341_DARKGREY, 2, wifiName);
    tft.setTextColor(ILI9341_CYAN); //prints network encription
    tft.setTextSize(1);
    tft.setCursor(233, 60 + offset);
    tft.print(WiFi.RSSI(thisNet)); //prints network signal strength
    tft.print(" dBm");
    tft.setCursor(233, 75 + offset);
    printEncryptionTypeToTFT(WiFi.encryptionType(thisNet));
  }
  drawRightArrow(300, 230, 20);
  drawLeftArrow(20, 230, 20);
}

void drawNetworksPage3(int numSsid) {
  drawTitle();
  printMACAddress();
  for (int thisNet = 7; thisNet < min(10, numSsid); thisNet++) {
    Serial.println(thisNet);
    int offset = ((thisNet-7)*45);
    String wifiName = WiFi.SSID(thisNet);
    if (wifiName.length() > MAX_SSID_LENGTH) {
      wifiName = wifiName.substring(0, MAX_SSID_LENGTH) + "...";
    }
    drawButton(45, 65 + offset, BUTTON_X, 50 + offset, BUTTON_W, BUTTON_H-10, ILI9341_DARKGREY, 2, wifiName);
    tft.setTextColor(ILI9341_CYAN); //prints network encription
    tft.setTextSize(1);
    tft.setCursor(233, 60 + offset);
    tft.print(WiFi.RSSI(thisNet)); //prints network signal strength
    tft.print(" dBm");
    tft.setCursor(233, 75 + offset);
    printEncryptionTypeToTFT(WiFi.encryptionType(thisNet));
  }
  drawLeftArrow(20, 230, 20);
}

void drawButton(int x, int y, int buttonX, int buttonY, int buttonW, int buttonH, int textColour, int textSize, String text) {
  tft.fillRect(buttonX, buttonY, buttonW, buttonH, textColour);
  tft.drawRect(buttonX, buttonY, buttonW, buttonH, ILI9341_WHITE);
  tft.setTextSize(textSize);
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(text);
  return;
}

int scanNearbyNetworks() {
  int numSsid = WiFi.scanNetworks();
  for (int j = 0; j < numSsid; j++) {
    Serial.println(WiFi.SSID(j));
  }
  tft.setTextSize(1);
  tft.setCursor(100, 90);
  if (numSsid == -1) {
    tft.setTextColor(ILI9341_RED);
    tft.print("No networks found!");
    return 0;
  }
  return numSsid;
}

void printEncryptionTypeToTFT(int thisType) {
  switch (thisType) {
    case ENC_TYPE_WEP:
      tft.print("WEP");
      break;
    case ENC_TYPE_TKIP:
      tft.print("WPA");
      break;
    case ENC_TYPE_CCMP:
      tft.print("WPA2");
      break;
    case ENC_TYPE_NONE:
      tft.print("Open");
      break;
    case ENC_TYPE_AUTO:
      tft.print("Auto");
      break;
    default:
      tft.print("Unknown");
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
