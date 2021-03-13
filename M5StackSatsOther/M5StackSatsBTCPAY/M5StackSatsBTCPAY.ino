#include "BTCPAYSplash.c"
#include <M5Stack.h>
#include <string.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <math.h>

#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

// --- Setup: Adapt to your needs
// Wifi
char wifiSSID[] = "YOUR-WIFI";
char wifiPASS[] = "YOUR-WIFI-PASSWORD";

// BTCPayServer
const char* server = "my.btcpay.com";
const int httpsPort = 443;
String encodedApiKey = "ENCODED-LEGACY-API-KEY"; // Encoded Legacy API Key: Generated on the "Manage store: Access Tokens" page
String currencyPair = "BTC_EUR";  // Currency pair
String currencyFiat = "Euro";  // Fiat display name
String description = "M5StackSats"; // Invoice description

// SSL Root CA – see this tutorial on how to retrieve yours:
// https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/
// This one is the DST Root CA X3, it works for Let's Encrypt certificates:
const char* root_ca= \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n" \
  "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
  "DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n" \
  "PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n" \
  "Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
  "AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n" \
  "rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n" \
  "OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n" \
  "xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n" \
  "7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n" \
  "aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n" \
  "HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n" \
  "SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n" \
  "ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n" \
  "AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n" \
  "R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n" \
  "JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n" \
  "Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n" \
  "-----END CERTIFICATE-----\n";

// --- Internal variables
int keysdec;
int keyssdec;
int invoiceCheckFrequency = 5; // interval in seconds for invoice check
int doneStateDuration = 3; // how long in seconds the complete/cancelled message is shown
float temp;
float satoshis;
float conversion;
String key_val;
String fiat = "";
String sats = "";
String cntr = "0";
String inputs = "";
String nofiat = "";
String postid = "";
String invoiceId = "";
String invoicePayReq = "";
String invoiceStatus = "";
String hash = "";
String currency = currencyPair.substring(4);
bool settled = false;
bool hasConversion;

void page_input(bool reset)
{
  if (reset) M5.Lcd.fillScreen(BLACK);
  
  if (hasConversion) {
    // Headline
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(65, 20);
    M5.Lcd.println("Enter the amount");

    // Fiat
    M5.Lcd.setCursor(20, 70);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println(currencyFiat + ": ");
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(120, 70);
    M5.Lcd.println(fiat);

    // Sats
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Sats: ");
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setCursor(120, 100);
    M5.Lcd.println(sats);

    // Rate
    M5.Lcd.setCursor(48, 160);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Rate: " + String(conversion) + " " + currencyFiat);

    // Buttons
    M5.Lcd.setCursor(37, 220);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Reset");
    if (sats.toInt() > 0) {
      M5.Lcd.setCursor(215, 220);
      M5.Lcd.println("Proceed");
    }
  } else {
    M5.Lcd.setCursor(23, 90);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.println("Conversion rate");
    M5.Lcd.setCursor(43, 120);
    M5.Lcd.println("not available");
    M5.Lcd.setCursor(128, 220);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("Retry");
  }
}

void page_processing()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(40, 80);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.println("PROCESSING");
}

void page_error(String message)
{
  Serial.println("ERR: " + message);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(40, 80);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.println(message);
  M5.Lcd.setCursor(214, 220);
  M5.Lcd.println("Retry");
}

void reset_input()
{
  key_val = "";
  inputs = "";
  sats = "";
  
  get_exchange_rate();
  page_input(true);
}

void get_keypad()
{
  if (digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1); // request 1 byte from keyboard
    while (Wire.available()) {
      uint8_t key = Wire.read(); // receive a byte as character
      key_val = key;

      if (key != 0) {
        if (key >= 0x20 && key < 0x7F) { // ASCII String
          if (isdigit((char)key)) {
            key_val = ((char)key);
          }
          else {
            key_val = "";
          }
        }
      }
    }
  }
}

void page_qrdisplay(String payReq)
{
  payReq.toUpperCase();
  Serial.println("QRCODE: " + payReq);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.qrcode(payReq, 45, 0, 240, 10);
  delay(100);
}

void setup()
{
  // Display splash screen
  M5.begin();
  M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)BTCPaySplash_map);
  Wire.begin();
  
  // Connect to wifi
  WiFi.begin(wifiSSID, wifiPASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (i >= 5) {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(55, 80);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(TFT_RED);
      M5.Lcd.println("WIFI NOT CONNECTED");
    }
    delay(5000);
    i++;
  }

  pinMode(KEYBOARD_INT, INPUT_PULLUP);
}

void loop() {
  reset_input();

  cntr = "1";

  while (cntr == "1") {
    M5.update();
    get_keypad();

    if (hasConversion) {
      if (sats.toInt() > 0 && M5.BtnC.wasReleased()) {
        page_processing();
  
        generate_invoice(fiat);
  
        settled = false;
  
        page_qrdisplay(invoicePayReq);
  
        int counta = 0;
        int tempi = 0;
  
        while (tempi == 0) {
          if (invoiceStatus != "complete") {
            counta ++;
            if (counta == 50) {
              tempi = 1;
            }
            delay(invoiceCheckFrequency * 1000);
            check_payment(invoiceId);
          } else {
            tempi = 1;
            settled = true;
  
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(40, 80);
            M5.Lcd.setTextSize(4);
            M5.Lcd.setTextColor(TFT_GREEN);
            M5.Lcd.println("THANKS! :)");
  
            delay(doneStateDuration * 1000);
  
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setTextColor(TFT_WHITE);
  
            reset_input();
          }
  
          int bee = 0;
          while ((bee < 120) && (tempi == 0)) {
            M5.update();
  
            if (M5.BtnA.wasReleased()) {
              tempi = -1;
  
              M5.Lcd.fillScreen(BLACK);
              M5.Lcd.setCursor(40, 80);
              M5.Lcd.setTextSize(4);
              M5.Lcd.setTextColor(TFT_RED);
              M5.Lcd.println("CANCELLED :(");
  
              delay(doneStateDuration * 1000);
  
              reset_input();
            }
  
            delay(10);
            bee++;
            key_val = "";
            inputs = "";
          }
        }
  
        key_val = "";
        inputs = "";
      }
  
      else if (M5.BtnA.wasReleased()) {
        reset_input();
      }

      inputs += key_val;

      temp = inputs.toInt();
      temp = temp / 100;
      fiat = temp;
      satoshis = temp / conversion;
  
      int intsats = (int) round(satoshis * 100000000.0);
  
      sats = String(intsats);

      page_input(false);

      delay(100);
      key_val = "";
    }
    
    else {
      if (M5.BtnB.wasReleased()) {
        reset_input();
      }
    }
  }
}

// --- HELPERS
String request_json(String action, String payload)
{
  WiFiClientSecure client;
  client.setCACert(root_ca);
  
  if (!client.connect(server, httpsPort)) {
    Serial.println("ERR: Connection error");
    char err_buf[100];
    if (client.lastError(err_buf, 100) < 0) {
        Serial.println(err_buf);
    }
    page_error("Connection failed.");
    return "";
  }

  Serial.println("REQ: " + action + " " + payload);
  
  // request
  client.print(action + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "User-Agent: M5StackSatsBTCPAY/1.0\r\n" +
               "Content-Type: application/json; charset=utf-8\r\n" +
               "Accept: application/json\r\n" +
               "Authorization: Basic " + encodedApiKey + "\r\n" +
               "Connection: close\r\n" + 
               "Content-Length: " + payload.length() + "\r\n\r\n" + payload + "\r\n");

  // skip response headers
  while (client.connected() || client.available()) {
    if (client.available() && client.readStringUntil('\n') == "\r") break;
  }

  // get and parse JSON body – this is still brittle!
  String json = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      String ln = client.readStringUntil('\n');
      Serial.println("---: " + ln);
      if (ln.indexOf("{") >= 0 || ln.indexOf("}") >= 0) {
        json += ln;
        break;
      } else if (ln == "\r") {
        break;
      }
    }
  }
  json.trim();
  
  client.stop();
  
  Serial.println("RES: " + json);

  return json;
}

// --- REQUESTS

void get_exchange_rate()
{
  String action = "GET /rates?currencyPairs=" + currencyPair;
  String json = request_json(action, "");
  if (json.length() == 0) return page_error("Exchange rate request failed.");

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(5) + 80;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, json);

  // Get the conversion rate
  conversion = doc["data"][0]["rate"];
  hasConversion = conversion > 0.0;
}

void generate_invoice(String value)
{
  String action = "POST /invoices/";
  String payload = "{\"price\":\"" + value + "\",\"currency\":\"" + currency  + "\",\"itemDesc\":\"" + description  + "\"}";
  String json = request_json(action, payload);
  if (json.length() == 0) return page_error("Generating invoice failed.");

  const size_t capacity = 2 * JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(2) + 9 * JSON_OBJECT_SIZE(1) + 6 * JSON_OBJECT_SIZE(2) + 5 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(9) + 2 * JSON_OBJECT_SIZE(15) + JSON_OBJECT_SIZE(37) + 3400;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, json);

  // Get the invoice details
  JsonObject data = doc["data"];
  String _invoiceUrl = data["url"];
  String _invoiceId = data["id"];
  String _invoiceStatus = data["status"];
  String _invoicePayReq = data["addresses"]["BTC_LightningLike"];

  invoiceId = _invoiceId;
  invoiceStatus = _invoiceStatus;
  invoicePayReq = _invoicePayReq;
  conversion = data["rate"];

  Serial.println("Invoice URL: " + _invoiceUrl);
  Serial.println("Invoice ID: " + invoiceId);
  Serial.println("Invoice Status: " + invoiceStatus);
  Serial.println("Payment Request: " + invoicePayReq);
}

void check_payment(String invId)
{
  String action = "GET /invoices/" + invId;
  String json = request_json(action, "");
  if (json.length() == 0) return page_error("Checking payment failed.");

  const size_t capacity = 2 * JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(2) + 10 * JSON_OBJECT_SIZE(1) + 5 * JSON_OBJECT_SIZE(2) + 5 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(9) + 2 * JSON_OBJECT_SIZE(15) + JSON_OBJECT_SIZE(37);
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, json);

  // Get the invoice status
  JsonObject data = doc["data"];
  String _invoiceStatus = data["status"];

  invoiceStatus = _invoiceStatus;

  Serial.println("Invoice status: " + invoiceStatus);
}
