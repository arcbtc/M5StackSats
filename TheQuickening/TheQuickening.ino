#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <string.h>
#include "LNimg.h"
#include "config.h"

#include "PaymentInvoice.h"
#include "PaymentServer.h"
#include "PaymentServerLNPay.h"
#include "PaymentServerLND.h"



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

//calc 
float calc_sum;
float calc_1, calc_2;
String calc_fiat;
String calc_inputs ="";
bool calc_mode = false;


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

  if (PAYMENTSERVER=="LNPAY") {
    Serial.println((String)"Setting LNPAY as Payment Server");
    PaymentServerLNPay * lnpay = new PaymentServerLNPay();
    paymentserver = lnpay;
  } 
  else {
    Serial.println((String)"Setting LND as Payment PaymentServer");
    PaymentServerLND * lnd = new PaymentServerLND();
    lnd->init(LNDserver, LNDport, LNDreadMacaroonHex, LNDinvoiceMacaroonHex);
    paymentserver = lnd;
  }
 
  Serial.print((String)"Running as PaymentServer --> ");
  Serial.println(paymentserver->getServiceName());

  Serial.println((String)"Init PaymentServer Data ...");
  if (paymentserver->init()) {
    Serial.println((String)"OK");
  } 
  else {
    Serial.println((String)"FAIL");
  }

 // Wire.begin();
  WiFi.begin(wifiSSID, wifiPASS);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(i >= 5) {
    screen_wifi_check();
    }
    delay(1000);
    i++;
  }
  on_rates();
}

void loop() {
  screen_refresh();
  screen_page_input();
  
  bool cntr = false;

  while (cntr == false) {
    
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
      calc_inputs ="";
      calc_flag = false;
      
    } 

  #ifdef M5STACK
    else if (M5.BtnB.wasReleased()) {
      calc_inputs ="";
      screen_refresh();
      screen_page_input();
    }     
  #endif
        
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
      nosats = 0.0;
      calc_sign ="";
      calc_flag = false;
      calc_inputs ="";
      calc_sum = 0.0;
    }

  

    Serial.print(key_val);

    if(calc_flag == true) {
      if(calc_sign == "="){
    
        fiat = calc_sum;
        calc_inputs = calc_sum;
        calc_1 = calc_sum;
        inputs = calc_sum;
        satoshis = calc_sum/conversion;
        nosats = (int) round(satoshis*100000000.0);
        memo =  memoBase + " " + fiat + " " + currencyBase;  
        screen_input_sats(fiat, nosats);
        delay(100);
        key_val = "";
        calc_inputs ="";
      }

      else if(calc_sign == "+"  ) {
        calc_inputs += key_val;
        calc_2 = calc_inputs.toInt();
        calc_2 = calc_2 / 100;
        calc_fiat = calc_2;
        
        calc_sum = calc_1 + calc_2;
        screen_calc_sats(fiat, calc_sign, calc_fiat, String(calc_sum));
        delay(100);
        key_val = "";
      }
          
      // negative result will be set to zero
      else if(calc_sign == "-"  ) {   
        calc_inputs += key_val;
        calc_2 = calc_inputs.toInt();
        calc_2 = calc_2 / 100;
        calc_fiat = calc_2;
              
        calc_sum = calc_1 - calc_2;
        if ((calc_1 - calc_2) < 0){
          calc_sum = 0;
        }
        screen_calc_sats(fiat, calc_sign, calc_fiat, String(calc_sum));      
        delay(100);
        key_val = "";
      }
        
      // Multiply only by integer
      else if(calc_sign == "*"  ) 
      {
        calc_inputs += key_val;
        calc_2 = calc_inputs.toInt();
        calc_fiat = calc_2;
              
        calc_sum = calc_1 * calc_2;
        screen_calc_sats(fiat, calc_sign, calc_fiat, String(calc_sum));
        delay(100);
        key_val = "";
      }
      
      else {
        key_val = "";
      }
    }
    
    else {
      inputs += key_val;
      calc_1 = inputs.toInt();      
      calc_1 = calc_1 / 100;
      fiat = calc_1;
        
      satoshis = calc_1/conversion;
      nosats = (int) round(satoshis*100000000.0);
      memo =  memoBase + " " + fiat + " " + currencyBase;
          
      screen_input_sats(fiat, nosats);
    
      delay(100);
      key_val = "";

    }
  }
  screen_refresh();
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
        calc_inputs = "";
     }
   }
   screen_page_input();
}
