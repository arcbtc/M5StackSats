
#include "LNPAYSplash.c"
#include <M5Stack.h> 
#include <string.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <math.h>

#include <HTTPClient.h>


#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

//WIFI Setup
char wifiSSID[] = "<your_wifi_ssid>";
char wifiPASS[] = "<your_wifi_pass>";

//API Setup
String api_key = "<api_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/integrations
String wallet_key = "<wi_XXXXX_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/advanced-wallets

//Payment Setup
String memo = "M5 "; //memo suffix, followed by a random number
String on_currency = "BTCUSD"; //currency can be changed here ie BTCUSD BTCGBP etc

//Endpoint setup
const char* api_endpoint = "https://lnpay.co/v1";
String invoice_create_endpoint = "/user/wallet/" + wallet_key + "/invoice";
String invoice_check_endpoint = "/user/lntx/"; //append LNTX ID to the end (e.g. /user/lntx/lntx_mfEKSse22)


String lntx_id;
String choice = "";
String on_sub_currency = on_currency.substring(3);

String key_val;
String cntr = "0";
String inputs = "";
int keysdec;
int keyssdec;
float temp;  
String fiat;
float satoshis;
String nosats = "";
float conversion;
bool settle = false;
String payreq = "";


void setup() {
  M5.begin();
  M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)PAYWSplash_map);
  Wire.begin();

  //connect to local wifi            
  WiFi.begin(wifiSSID, wifiPASS);   
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(i >= 5){
     screen_wifi_check()
    }
    delay(1000);
    i++;
  }
 
  pinMode(KEYBOARD_INT, INPUT_PULLUP);
  on_rates();
 
}

void loop() {
  page_input();
  cntr = "1";
  while (cntr == "1"){
    M5.update();
    get_keypad(); 

    if (M5.BtnC.wasReleased()) {
      screen_page_processing();
      reqinvoice(nosats);
      screen_page_qrdisplay(payreq);
      checkpaid();
      key_val = "";
      inputs = "";
    } else if (M5.BtnB.wasReleased()) {
      screen_page_processing();
      nosats = "0";
      reqinvoice(nosats);
      screen_page_qrdisplay(payreq);
      checkpaid();
      key_val = "";
      inputs = "";
    } else if (M5.BtnA.wasReleased()) {
      screen_refresh()
      page_input();
      key_val = "";
      inputs = "";  
      nosats = "";
    }
    
    Serial.print(key_val);
    inputs += key_val;
    temp = inputs.toInt();
    temp = temp / 100;
    fiat = temp;
    satoshis = temp/conversion;
    int intsats = (int) round(satoshis*100000000.0);

    nosats = String(intsats);
    screen_input_sats(fiat, nosats)


    delay(100);
    key_val = "";
  }
}
//OPENNODE GET PRICE
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




