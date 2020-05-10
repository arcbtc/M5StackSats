//Macaroons will need to be converted to hex strings, in terminal run "xxd -plain readonly.macaroon > readmac.txt"...in Linux Zap's readonly.macaroon can be found in `$XDG_CONFIG_HOME/Zap/lnd/bitcoin/mainnet/wallet-1` (or under`~/.config`)

#include "BLITZSplash.c"
#include <M5Stack.h> 
#include <string.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <math.h>

#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

//Wifi details
char wifiSSID[] = "YOUR-WIFI";
char wifiPASS[] = "YOUR-WIFI-PASSWORD";

//BLITZ DETAILS
const char*  server = "room77.raspiblitz.com"; 
const int httpsPort = 443;
const int lndport = 8080;
String pubkey;
String totcapacity;

bool certcheck = false;
String readmacaroon = "0201036c6e64028a01030a105dc5e03a7e5444d4ebbdbb513d1fc40f1201301a0f0a07616464726573731204726561641a0c0a04696e666f1204726561641a100a08696e766f696365731204726561641a0f0a076d6573736167651204726561641a100a086f6666636861696e1204726561641a0f0a076f6e636861696e1204726561641a0d0a05706565727312047265616400000620b964213f708bc349dc1de651b424817651858fcfffa38345f6c566053ee22cf5";
String invoicemacaroon = "0201036c6e640247030a105ec5e03a7e5444d4ebbdbb513d1fc40f1201301a160a0761646472657373120472656164120577726974651a170a08696e766f69636573120472656164120577726974650000062062630006bd8cc1f7ce6be81d90dd2c1b754ecf7d7d0d794fe2c36fec5680876e";
//#include "TLSCert.h" //Un-comment if you need to include a TLS Cert, also uncomment line 279, 303, 360, 403


String choice = "";

String on_currency = "BTCEUR"; //currency can be changed here ie BTCUSD BTCGBP etc
String on_sub_currency = on_currency.substring(3);
String memo = "Memo "; //memo suffix, followed by the price then a random number

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
  String postid = "";
  String data_id = "";
  String data_lightning_invoice_payreq = "";
  String data_status = "";
  bool settle = false;
  String payreq = "";
  String hash = "";

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

void setup() {
  M5.begin();
  M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)BLITZSplash_map);
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
  nodecheck();
}

void loop() {

 page_input();

  cntr = "1";
  while (cntr == "1"){
   M5.update();
   get_keypad(); 

    if (M5.BtnC.wasReleased()) {
   // if (key_val == "="){

     page_processing();
     
     reqinvoice(nosats);

     gethash(payreq);

     page_qrdisplay(payreq);

     int counta = 0;
     int tempi = 0;
     settle = false;

     while (tempi == 0){
      
      

     checkpayment(hash);

     if (settle == false){
        counta ++;
        Serial.print(counta);
        if (counta == 100){
         tempi = 1;
        }
 
     }
      
       else{
        tempi = 1;

     M5.Lcd.fillScreen(BLACK);
     M5.Lcd.setCursor(60, 80);
     M5.Lcd.setTextSize(4);
     M5.Lcd.setTextColor(TFT_GREEN);
     M5.Lcd.println("COMPLETE");

     delay(1000);
     nodecheck();;
     cntr = "2";
 
      }
      
     int bee = 0;
     while ((bee < 120) && (tempi==0)){

      M5.update();

     if (M5.BtnA.wasReleased()) {

        tempi = -1;
     
     M5.Lcd.fillScreen(BLACK);
     M5.Lcd.setCursor(50, 80);
     M5.Lcd.setTextSize(4);
     M5.Lcd.setTextColor(TFT_RED);
     M5.Lcd.println("CANCELLED");

     delay(1000);
     nodecheck();;
     cntr = "2";
     
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
  WiFiClientSecure client;
  if (!client.connect("api.opennode.co", httpsPort)) {
    return;
  }

  String url = "/v1/rates";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.opennode.co\r\n" +
               "User-Agent: ESP32\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
  Serial.println("poo");

      break;
    }
  }
  String line = client.readStringUntil('\n');
    const size_t capacity = 169*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(168) + 3800;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, line);
    conversion = doc["data"][on_currency][on_currency.substring(3)]; 

}

void nodecheck(){
  bool checker = false;
  while(!checker){
  WiFiClientSecure client;
   client.setCACert(test_root_ca);
  if (!client.connect(server, lndport)){

    M5.Lcd.fillScreen(BLACK);
     M5.Lcd.setCursor(20, 80);
     M5.Lcd.setTextSize(3);
     M5.Lcd.setTextColor(TFT_RED);
     M5.Lcd.println("NO NODE DETECTED");
     delay(1000);
  }
  else{
    checker = true;
  }
  }
  
}

void reqinvoice(String value){

   WiFiClientSecure client;

 // client.setCACert(test_root_ca);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, lndport)){
      return;   
  }

    
   String topost = "{\"value\": \""+ value +"\", \"memo\": \""+ memo + String(fiat) + on_sub_currency + " (" + String(random(1,1000)) + ")" +"\", \"expiry\": \"1000\"}";
  
       client.print(String("POST ")+ "https://" + server + ":" + String(lndport) + "/v1/invoices HTTP/1.1\r\n" +
                 "Host: "  + server +":"+ String(lndport) +"\r\n" +
                 "User-Agent: ESP322\r\n" +
                 "Grpc-Metadata-macaroon:" + invoicemacaroon + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: close\r\n" +
                 "Content-Length: " + topost.length() + "\r\n" +
                 "\r\n" + 
                 topost + "\n");

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
       
        break;
      }
    }
    
    String content = client.readStringUntil('\n');
  

    client.stop();
    
    const size_t capacity = JSON_OBJECT_SIZE(3) + 320;
    DynamicJsonDocument doc(capacity);

    deserializeJson(doc, content);

    const char* r_hash = doc["r_hash"];
    hash = r_hash;
    const char* payment_request = doc["payment_request"]; 
    payreq = payment_request;
 
}



void gethash(String xxx){
  
   WiFiClientSecure client;

 // client.setCACert(test_root_ca);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, lndport)){
       return;
  }
   

       client.println(String("GET ") + "https://" + server +":"+ String(lndport) + "/v1/payreq/"+ xxx +" HTTP/1.1\r\n" +
                 "Host: "  + server +":"+ String(lndport) +"\r\n" +
                 "Grpc-Metadata-macaroon:" + readmacaroon + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: close");
                 
       client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
       
        break;
      }
    }
    

    String content = client.readStringUntil('\n');

    client.stop();

    const size_t capacity = JSON_OBJECT_SIZE(7) + 270;
    DynamicJsonDocument doc(capacity);

    deserializeJson(doc, content);

    const char* payment_hash = doc["payment_hash"]; 
    hash = payment_hash;
}


void checkpayment(String xxx){
  
   WiFiClientSecure client;

 // client.setCACert(test_root_ca);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, lndport)){
       return;
  }

       client.println(String("GET ") + "https://" + server +":"+ String(lndport) + "/v1/invoice/"+ xxx +" HTTP/1.1\r\n" +
                 "Host: "  + server +":"+ String(lndport) +"\r\n" +
                 "Grpc-Metadata-macaroon:" + readmacaroon + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: close");
                 
       client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {

        break;
      }
    }
    

    String content = client.readStringUntil('\n');

    client.stop();
    
    const size_t capacity = JSON_OBJECT_SIZE(9) + 460;
    DynamicJsonDocument doc(capacity);

    deserializeJson(doc, content);

    settle = doc["settled"]; 
    
  
}



void page_qrdisplay(String xxx)
{  

  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.qrcode(payreq,45,0,240,10);
  delay(100);

}
