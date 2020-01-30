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

// --- Internal variables
int keysdec;
int keyssdec;
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

void page_input()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(65, 20);
  M5.Lcd.println("Enter the amount");
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(20, 70);
  M5.Lcd.println(currencyFiat + ": ");
  M5.Lcd.setCursor(20, 100);
  M5.Lcd.println("Sats: ");
  M5.Lcd.println("");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(48, 160);
  M5.Lcd.println("Rate: " + String(conversion));
  M5.Lcd.println("");
  M5.Lcd.setCursor(37, 220);
  M5.Lcd.println("Reset");
  M5.Lcd.setCursor(214, 220);
  M5.Lcd.println("Proceed");
}

void page_processing()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(40, 80);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.println("PROCESSING");
}

void get_keypad()
{
  if (digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
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

void page_qrdisplay(String xxx)
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.qrcode(invoicePayReq, 45, 0, 240, 10);
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
    delay(1000);
    i++;
  }

  pinMode(KEYBOARD_INT, INPUT_PULLUP);

  get_exchange_rate();
}

void loop() {
  page_input();

  cntr = "1";

  while (cntr == "1") {
    M5.update();
    get_keypad();

    if (M5.BtnC.wasReleased()) {
      page_processing();

      generate_invoice(fiat);
      
      settled = false;

      page_qrdisplay(invoicePayReq);

      int counta = 0;
      int tempi = 0;

      while (tempi == 0) {
        check_payment(invoiceId);

        if (invoiceStatus != "complete") {
          counta ++;
          Serial.print(counta);
          if (counta == 50) {
            tempi = 1;
          }
          delay(2500);
        }

        else {
          tempi = 1;
          settled = true;

          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(60, 80);
          M5.Lcd.setTextSize(4);
          M5.Lcd.setTextColor(TFT_GREEN);
          M5.Lcd.println("COMPLETE");

          delay(2000);
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setTextColor(TFT_WHITE);

          get_exchange_rate();
          page_input();
        }

        int bee = 0;
        while ((bee < 120) && (tempi == 0)) {
          M5.update();

          if (M5.BtnA.wasReleased()) {
            tempi = -1;

            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(50, 80);
            M5.Lcd.setTextSize(4);
            M5.Lcd.setTextColor(TFT_RED);
            M5.Lcd.println("CANCELLED");

            delay(2000);

            get_exchange_rate();
            page_input();
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
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(TFT_WHITE);
      page_input();
      key_val = "";
      inputs = "";
      sats = "";
    }

    inputs += key_val;

    temp = inputs.toInt();
    temp = temp / 100;
    fiat = temp;
    satoshis = temp / conversion;

    int intsats = (int) round(satoshis * 100000000.0);

    sats = String(intsats);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(120, 70);
    M5.Lcd.println(fiat);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setCursor(120, 100);
    M5.Lcd.println(sats);

    delay(100);
    key_val = "";
  }
}

// --- HELPERS

String get_request_body(WiFiClientSecure &client)
{
  while (client.connected() || client.available()) {
    if (client.available() && client.readStringUntil('\n') == "\r") break;
  }
  
  String reqBody = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      String ln = client.readStringUntil('\n');
      if (ln.indexOf("{") >= 0 || ln.indexOf("}") >= 0) reqBody += ln;
      else if (ln == "\r") break;
    }
  }

  return reqBody;
}

// --- REQUESTS

void get_exchange_rate()
{
  WiFiClientSecure client;
  if (!client.connect(server, httpsPort)) return;

  // Send HTTP request
  client.print(String("GET ") + "/rates?currencyPairs=" + currencyPair + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "User-Agent: M5StackSatsBTCPAY/1.0\r\n" +
               "Content-Type: application/json\r\n" +
               "Accept: application/json\r\n" +
               "Authorization: Basic " + encodedApiKey + "\r\n" +
               "Connection: close\r\n\r\n");

  // Get and parse JSON body
  String json = get_request_body(client);
  client.stop();
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(5) + 80;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, json);

  // Get the conversion rate
  conversion = doc["data"][0]["rate"];
}

void generate_invoice(String value)
{
  WiFiClientSecure client;
  if (!client.connect(server, httpsPort)) return;

  String payload = "{\"price\":\"" + value + "\",\"currency\":\"" + currency  + "\",\"itemDesc\":\"" + description  + "\"}";

  client.print(String("POST ") + "/invoices/ HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "User-Agent: M5StackSatsBTCPAY/1.0\r\n" +
               "Content-Type: application/json\r\n" +
               "Accept: application/json\r\n" +
               "Authorization: Basic " + encodedApiKey + "\r\n" +
               "Connection: close\r\n" +
               "Content-Length: " + payload.length() + "\r\n" +
               "\r\n" +
               payload + "\n");
  
  // Get and parse JSON body
  String json = get_request_body(client);
  client.stop();
  const size_t capacity = 2*JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(2) + 9*JSON_OBJECT_SIZE(1) + 6*JSON_OBJECT_SIZE(2) + 5*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(9) + 2*JSON_OBJECT_SIZE(15) + JSON_OBJECT_SIZE(37) + 3140;
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

  Serial.println("URL: " + _invoiceUrl);
  Serial.println("ID: " + invoiceId);
  Serial.println("Status: " + invoiceStatus);
  Serial.println("Payment Request: " + invoicePayReq);
}

void check_payment(String invId)
{
  WiFiClientSecure client;
  if (!client.connect(server, httpsPort)) return;

  client.print(String("GET ") + "/invoices/" + invId + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "User-Agent: M5StackSatsBTCPAY/1.0\r\n" +
               "Content-Type: application/json\r\n" +
               "Accept: application/json\r\n" +
               "Authorization: Basic " + encodedApiKey + "\r\n" +
               "Connection: close\r\n\r\n");

  // Get and parse JSON body
  String json = get_request_body(client);
  client.stop();
  const size_t capacity = 2*JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(2) + 10*JSON_OBJECT_SIZE(1) + 5*JSON_OBJECT_SIZE(2) + 5*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(9) + 2*JSON_OBJECT_SIZE(15) + JSON_OBJECT_SIZE(37);
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, json);

  // Get the invoice status
  JsonObject data = doc["data"];
  String _invoiceStatus = data["status"];
  
  invoiceStatus = _invoiceStatus;
  
  Serial.println("Status: " + invoiceStatus);
}
