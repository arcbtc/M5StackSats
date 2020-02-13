#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <string.h>
#include <ArduinoJson.h>
#include "LNimg.h"

#include "PaymentInvoice.h"
#include "PaymentServer.h"
#include "PaymentServerLNPay.h"
#include "PaymentServerLND.h"

String PAYMENTSERVER = "LND"; 

//Payment Setup 
String memo = "PoS "; //memo suffix, followed by a random number
String on_currency = "BTCUSD"; //currency can be changed here ie BTCUSD BTCGBP etc

//Declare whether you're using the DIY or M5stackSats flavour
#include "M5Stack.h"

//WIFI Setup
char wifiSSID[] = "raspiblitz";
char wifiPASS[] = "raspiblitz";
  
//Variables
String inputs = "";
String fiat;
float satoshis;
int nosats;
float temp; 
float conversion;
bool settled;

PaymentServer *paymentserver;

void setup() {

 //M5-centric
 M5.begin();
 Wire.begin();
 pinMode(KEYBOARD_INT, INPUT_PULLUP);
 
 screen_splash();
 Serial.begin(115200);

 if (PAYMENTSERVER=="LNPAY")
 {
   Serial.println((String)"Setting LND as Payment Server");
   PaymentServerLND * lnd = new PaymentServerLND();
   paymentserver = lnd;
 } else
 {
   Serial.println((String)"Setting LNPay as Payment PaymentServer");
   PaymentServerLNPay * lnpay = new PaymentServerLNPay();
   paymentserver = lnpay;
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

    if (M5.BtnC.wasReleased()) {
      
      screen_page_processing();
      
      PaymentInvoice resp = paymentserver->getInvoice(nosats,memo);
      
      screen_qrdisplay(resp.paymentRequest);

      settled = paymentserver->isInvoicePaid(resp.id);
      checkpaid(resp);
       
      key_val = "";
      inputs = "";
      
    } else if (M5.BtnB.wasReleased()) {
      
      screen_page_processing();
      
      nosats = 0;
      
      PaymentInvoice resp = paymentserver->getInvoice(nosats,memo);
      
      screen_qrdisplay(resp.paymentRequest);
      
      settled = paymentserver->isInvoicePaid(resp.id);
      checkpaid(resp);
      
      key_val = "";
      inputs = "";
      
    } else if (M5.BtnA.wasReleased()) {
      
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
          }
       } else {
        screen_complete();
        tempi = true;
        delay(1000);
      }
     int bee = 0;
     while ((bee < 120) && (tempi==0)) {
        M5.update();
        if (M5.BtnA.wasReleased()) {
          tempi = true;
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
