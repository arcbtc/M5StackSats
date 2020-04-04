/**
 *  M5StackSatsLNPAY
 */

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
  on_rates();
 
}

void loop() {
  page_input();
  cntr = "1";
  while (cntr == "1"){
    M5.update();
    get_keypad(); 

    if (M5.BtnC.wasReleased()) {
      page_processing();
      reqinvoice(nosats);
      page_qrdisplay(payreq);
      checkpaid();
      key_val = "";
      inputs = "";
    } else if (M5.BtnB.wasReleased()) {
      page_processing();
      nosats = "0";
      reqinvoice(nosats);
      page_qrdisplay(payreq);
      checkpaid();
      key_val = "";
      inputs = "";
    } else if (M5.BtnA.wasReleased()) {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextColor(TFT_WHITE);
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
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(70, 88);
    M5.Lcd.println(fiat);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setCursor(87, 136);
    M5.Lcd.println(nosats);

    delay(100);
    key_val = "";
  }
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

/**
 * Request an invoice
 */
void reqinvoice(String value){

  String payload;
  HTTPClient http;
  http.begin(api_endpoint + invoice_create_endpoint + "?fields=payment_request,id"); //Getting fancy to response size
  http.addHeader("Content-Type","application/json");
  http.addHeader("X-Api-Key",api_key);
  String toPost = "{  \"num_satoshis\" : " + nosats +", \"memo\" :\""+ memo + String(random(1,1000)) + "\"}";
  int httpCode = http.POST(toPost); //Make the request
  
  if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println(payload);
    }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources

  Serial.println(payload);

  
  const size_t capacity = JSON_OBJECT_SIZE(2) + 500;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload);
  
  const char* payment_request = doc["payment_request"]; 
  const char* id = doc["id"]; 
  payreq = (String) payment_request;
  lntx_id = (String) id;
  Serial.println(payreq);
  Serial.println(lntx_id);
}


void checkpaid(){

     int counta = 0;
     int tempi = 0;
     settle = false;

     while (tempi == 0){
       checkpayment();
       if (settle == false){
          counta ++;
          Serial.print(counta);
          if (counta == 100) {
           tempi = 1;
          }
       } else {
        tempi = 1;
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(60, 80);
        M5.Lcd.setTextSize(4);
        M5.Lcd.setTextColor(TFT_GREEN);
        M5.Lcd.println("COMPLETE");
        delay(1000);
        
        cntr = "2";
      }
      
     int bee = 0;
     while ((bee < 120) && (tempi==0)) {
        M5.update();
        if (M5.BtnA.wasReleased()) {
          tempi = -1;
        
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(50, 80);
          M5.Lcd.setTextSize(4);
          M5.Lcd.setTextColor(TFT_RED);
          M5.Lcd.println("CANCELLED");
          delay(1000);
          cntr = "2";
        }
        
        delay(10);
        bee++;
        key_val = "";
        inputs = "";
     }
   }
}


void checkpayment(){
  String payload;
  HTTPClient http;
  http.begin(api_endpoint + invoice_check_endpoint + lntx_id + "?fields=settled"); //Getting fancy to response size
  http.addHeader("Content-Type","application/json");
  http.addHeader("X-Api-Key",api_key);
  int httpCode = http.GET(); //Make the request
  
  if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println(payload);
    }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources
  
  const size_t capacity = JSON_OBJECT_SIZE(2) + 500;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload);
  
  int settled = doc["settled"]; 
  Serial.println(settled);
  if (settled == 1){
    settle = true;
  }
  else{
    settle = false;
  }
}

void page_qrdisplay(String xxx)
{  
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.qrcode(payreq,45,0,240,14);
  delay(100);
}

void page_input()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.println("Amount then C");
  M5.Lcd.println("");
  M5.Lcd.println(on_currency.substring(3) + ": ");
  M5.Lcd.println("");
  M5.Lcd.println("SATS: ");
  M5.Lcd.println("");
  M5.Lcd.println("");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(50, 200);
  M5.Lcd.println("TO RESET PRESS A");
}

void page_processing()
{ 
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(40, 80);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.println("PROCESSING");
}

void get_keypad(){
   if(digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
    while (Wire.available()) { 
       uint8_t key = Wire.read();                  // receive a byte as character
       key_val = key;

       if(key != 0) {
        if(key >= 0x20 && key < 0x7F) { // ASCII String
          if (isdigit((char)key)){
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
