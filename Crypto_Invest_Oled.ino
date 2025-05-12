#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// API endpoint
const char* binanceApiDEEP = "https://fapi.binance.com/fapi/v1/ticker/24hr?symbol=DEEPUSDT";
const char* binanceApiBTC  = "https://fapi.binance.com/fapi/v1/ticker/24hr?symbol=BTCUSDT";

// ⚙️ Cấu hình số lượng token và vốn đầu tư ban đầu (USD)
const int MAX_PURCHASES = 9;

float deepAmounts[MAX_PURCHASES] = {11870, 1770, 21655, 28670, 14769, 1173, 3092, 2286, 2286};
float deepPrices[MAX_PURCHASES]  = {0.045, 0.045, 0.045, 0.045, 0.09, 0.055, 0.045, 0.045, 0.045};
int deepCount = 8; // số đợt mua thực tế

float btcAmounts[MAX_PURCHASES] = {};
float btcPrices[MAX_PURCHASES]  = {};
int btcCount = 0;

float getTotalAmount(float amounts[], int count) {
  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += amounts[i];
  }
  return sum;
}

float getTotalCost(float amounts[], float prices[], int count) {
  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += amounts[i] * prices[i];
  }
  return sum;
}


// ⚙️ Hiển thị thông báo một dòng
void showMessage(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(msg);
  display.display();
}

// ⚙️ Hiển thị 2 dòng
void showTwoLines(String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 10);
  display.println(line2);
  display.display();
}

// ⚙️ Vẽ biểu tượng Wi-Fi
void drawWiFiIcon(bool connected) {
  if (connected) {
    display.drawCircle(3, 3, 3, SSD1306_WHITE);
    display.drawCircle(3, 3, 2, SSD1306_WHITE);
    display.fillCircle(3, 3, 1, SSD1306_WHITE);
  } else {
    display.drawCircle(3, 3, 3, SSD1306_WHITE);
    display.drawLine(0, 0, 6, 6, SSD1306_WHITE);
  }
}


// ⚙️ Hiển thị DEEP
void showDEEPBOOK(float price, bool wifiOK) {
  float totalAmount = getTotalAmount(deepAmounts, deepCount);
  float totalCost   = getTotalCost(deepAmounts, deepPrices, deepCount);
  float totalValue  = price * totalAmount;
  float profitLoss  = totalValue - totalCost;

  display.clearDisplay();
  drawWiFiIcon(wifiOK);
  display.setTextSize(1);
  display.setCursor(12, 0);
  display.print("DEEPUSDT - Binance");

  display.setTextSize(2);
  display.setCursor(20, 8);
  display.print("$");
  display.print(price, 4);

  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("$");
  display.print(totalValue, 2);

  display.setCursor(80, 25);
  display.print((profitLoss >= 0) ? "+" : "-");
  display.print(abs(profitLoss), 2);

  display.display();
}

// ⚙️ Hiển thị BTC
void showBTC(float price, bool wifiOK) {
  float totalAmount = getTotalAmount(btcAmounts, btcCount);
  float totalCost   = getTotalCost(btcAmounts, btcPrices, btcCount);
  float totalValue  = price * totalAmount;
  float profitLoss  = totalValue - totalCost;

  display.clearDisplay();
  drawWiFiIcon(wifiOK);
  display.setTextSize(1);
  display.setCursor(12, 0);
  display.print("BTCUSDT - Binance");

  display.setTextSize(2);
  display.setCursor(20, 8);
  display.print("$");
  display.print(price, 0);

  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("$");
  display.print(totalValue, 2);

  display.setCursor(80, 25);
  display.print((profitLoss >= 0) ? "+" : "-");
  display.print(abs(profitLoss), 2);

  display.display();
}


// ⚙️ Fetch DEEP
void fetchDEEPBOOK() {
  if (WiFi.status() != WL_CONNECTED) {
    showDEEPBOOK(0, false);
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (https.begin(client, binanceApiDEEP)) {
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();
      DynamicJsonDocument doc(1024);
      if (deserializeJson(doc, payload)) {
        showMessage("Loi JSON DEEP");
        return;
      }
      float lastPrice = doc["lastPrice"].as<float>();
      showDEEPBOOK(lastPrice, true);
    } else {
      showMessage("Loi API DEEP");
    }
    https.end();
  } else {
    showMessage("Loi ket noi DEEP");
  }
}

// ⚙️ Fetch BTC
void fetchBTC() {
  if (WiFi.status() != WL_CONNECTED) {
    showBTC(0, false);
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (https.begin(client, binanceApiBTC)) {
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();
      DynamicJsonDocument doc(1024);
      if (deserializeJson(doc, payload)) {
        showMessage("Loi JSON BTC");
        return;
      }
      float lastPrice = doc["lastPrice"].as<float>();
      showBTC(lastPrice, true);
    } else {
      showMessage("Loi API BTC");
    }
    https.end();
  } else {
    showMessage("Loi ket noi BTC");
  }
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED khong khoi dong duoc"));
    while (true);
  }
  display.clearDisplay();
  display.display();

  showTwoLines("Dang ket noi WiFi...", "WanR-Crypto");
  delay(2000);

  WiFiManager wm;
  bool res = wm.autoConnect("WanR-Crypto", "12345678");
  if (!res) {
    showMessage("WiFi fail. Reset");
    delay(2000);
    ESP.restart();
  }

  showMessage("WiFi OK. Ket noi API...");
  delay(1000);
}

int counter = 0;

void loop() {
  if (counter % 2 == 0) {
    fetchDEEPBOOK();
  } else {
    fetchBTC();
  }
  counter++;
  delay(30000); // cập nhật mỗi 30 giây
}
