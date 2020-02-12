#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <string.h>
#include <ArduinoJson.h>
#include "PaymentConnector.h"
#include "LNimg.h"

PaymentConnector paymentConnector("BTCUSD");

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
int settled;

void setup() {

 //M5-centric
 M5.begin();
 Wire.begin();
 pinMode(KEYBOARD_INT, INPUT_PULLUP);
 
 screen_splash();
 Serial.begin(115200);
 
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
      
      createInvoiceResponse resp = paymentConnector.createInvoice(nosats,memo);
      
      screen_qrdisplay(resp.payment_request);

      settled = paymentConnector.checkIfPaymentIsSettled(resp.payment_id);
      checkpaid(resp);
       
      key_val = "";
      inputs = "";
      
    } else if (M5.BtnB.wasReleased()) {
      
      screen_page_processing();
      
      nosats = 0;
      
      createInvoiceResponse resp = paymentConnector.createInvoice(nosats,memo);
      
      screen_qrdisplay(resp.payment_request);
      
      settled = paymentConnector.checkIfPaymentIsSettled(resp.payment_id);
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

void checkpaid(createInvoiceResponse resp){
     int counta = 0;
     bool tempi = false;
     while (tempi == false){
       settled = paymentConnector.checkIfPaymentIsSettled(resp.payment_id);
       if (settled == 0){
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
