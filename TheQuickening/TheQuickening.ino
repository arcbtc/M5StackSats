#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <string.h>
#include <ArduinoJson.h>
#include "LNimg.h"

#include "PaymentInvoice.h"
#include "PaymentServer.h"
#include "PaymentServerLNPay.h"
#include "PaymentServerLND.h"

//HARDWARE Uncomment for hardware used//
#define M5STACK //Based on M5Stack Faces Kit
//#define DIY //Based on ESP32/1.8TFT/Keypad Matrix

String PAYMENTSERVER = "LND"; 

//WIFI Setup
char wifiSSID[] = "ROOM77";
char wifiPASS[] = "allyourcoin";

//Payment Setup 
String memoBase = "PoS "; //memo suffix, followed by a random number
String memo="";
String currencyBase="EUR";
String on_currency = "BTC"+currencyBase; //currency can be changed here ie BTCUSD BTCGBP etc


////END OF USER SETUP///

#ifdef DIY
  #include "DIYv.h"
#endif

#ifdef M5STACK
  #include "M5Stack.h"
#endif

//Variables
String inputs = "";
String fiat;
float satoshis;
int nosats;
float temp; 
float conversion;
bool settled;
bool cntr = false;

PaymentServer *paymentserver;

void setup() {
  
  #ifdef M5STACK
   M5.begin();
   pinMode(KEYBOARD_INT, INPUT_PULLUP);
   Wire.begin();
  #endif

  #ifdef DIY
   tft.begin();
   tft.fillScreen(TFT_BLACK);
   tft.setRotation(3);
  #endif
  
 screen_splash();
 Serial.begin(115200);

 if (PAYMENTSERVER=="LNPAY")
 {
   Serial.println((String)"Setting LNPAY as Payment Server");
   PaymentServerLNPay * lnpay = new PaymentServerLNPay();
   paymentserver = lnpay;
 } else
 {
   Serial.println((String)"Setting LND as Payment PaymentServer");
   PaymentServerLND * lnd = new PaymentServerLND();
   lnd->init("room77.raspiblitz.com",8077,"0201036C6E64028A01030A10CCC987A6CE26FC4CF42676A6D1EE1D1C1201301A0F0A07616464726573731204726561641A0C0A04696E666F1204726561641A100A08696E766F696365731204726561641A0F0A076D6573736167651204726561641A100A086F6666636861696E1204726561641A0F0A076F6E636861696E1204726561641A0D0A0570656572731204726561640000062074D6B7847B83039162230EC94BD4CFB1E0FBC1FFF7B6E7BF001A3523F5289C9A", "0201036C6E640247030A10CFC987A6CE26FC4CF42676A6D1EE1D1C1201301A160A0761646472657373120472656164120577726974651A170A08696E766F69636573120472656164120577726974650000062013765C97050424F33D0FE4508D7A1A09D1772F7365FB95E2AECCAA4E1F35C782");
   paymentserver = lnd;
 }
 
 Serial.print((String)"Running as PaymentServer --> ");
 Serial.println(paymentserver->getServiceName());

 Serial.println((String)"Init PaymentServer Data ...");
 if (paymentserver->init()) {
   Serial.println((String)"OK");
 } else {
   Serial.println((String)"FAIL");
 }

 // Wire.begin();
  WiFi.begin(wifiSSID, wifiPASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(i >= 5){
    screen_wifi_check();
    }
    delay(1000);
    i++;
  }
  
  on_rates();
 
}

void loop() {
  screen_page_input();
  bool cntr = false;

  while (cntr == false){
    
    get_keypad(); 

  #ifdef M5STACK
   if (M5.BtnC.wasReleased()) {
  #endif
  
  #ifdef DIY
   if (key_val == "#") {
  #endif
      
      
      screen_page_processing();
      
      PaymentInvoice resp = paymentserver->getInvoice(nosats,memo);
      
      screen_qrdisplay(resp.paymentRequest);

      settled = paymentserver->isInvoicePaid(resp.id);
      checkpaid(resp);
       
      key_val = "";
      inputs = "";
      
    } 
      
  #ifdef M5STACK
   else if (M5.BtnA.wasReleased()) {
  #endif
  
  #ifdef DIY
   else if (key_val == "*") {
  #endif
      
      screen_refresh();
      
      screen_page_input();

      key_val = "";
      inputs = "";  
      nosats = 0;
    }
    
    Serial.print(key_val);
    inputs += key_val;
    temp = inputs.toInt();
    temp = temp / 100;
    fiat = temp;
    satoshis = temp/conversion;
    nosats = (int) round(satoshis*100000000.0);
    memo =  memoBase + " " + fiat + " " + currencyBase;

    screen_input_sats(fiat, nosats);

    delay(100);
    key_val = "";
  }

  screen_page_input();
  
}

//OPENNODE REQUESTS
void on_rates(){
    String payload;
    HTTPClient http;
    http.begin("https://api.opennode.co/v1/rates"); //Specify the URL
    int httpCode = http.GET(); //Make the request
    
    if (httpCode > 0) { //Check for the returning code
        payload = http.getString();
        Serial.println(payload);
      }
    else {
      Serial.println("Error on HTTP request");
    }
    http.end(); //Free the resources
    const size_t capacity = 169*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(168) + 3800;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, payload);
    conversion = doc["data"][on_currency][on_currency.substring(3)]; 
    Serial.println(conversion);
}

void checkpaid(PaymentInvoice resp){
     int counta = 0;
     bool tempi = false;
     
     while (tempi == false){
       settled = paymentserver->isInvoicePaid(resp.id);
       if (!settled){
          counta ++;
          if (counta == 100) {
            
           tempi = true;
           cntr = true;
          }
       } else {
        screen_complete();
        tempi = true;
        cntr = true;
        delay(1000);
      }
     int bee = 0;
     while ((bee < 120) && (tempi==0)) {
      
         get_keypad(); 
         
          #ifdef M5STACK
           if (M5.BtnA.wasReleased()) {
          #endif
          
          #ifdef DIY
           if (key_val == "*") {
          #endif

      
          tempi = true;
          cntr = true;
          screen_cancel();
          delay(1000);
          
        }
        delay(10);
        bee++;
        key_val = "";
        inputs = "";
     }
   }
   screen_page_input();
}
