

#include <TFT_eSPI.h>
#include "qrcode.h" 
#include <Keypad.h>

//keypad stuff
String key_val;
TFT_eSPI tft = TFT_eSPI();

void screen_splash()
{  
tft.drawBitmap(0, 0, (uint8_t *)SplashDIY_map, 160, 128, TFT_WHITE);

}

void screen_input_sats(String fiat, int nosats)
{
tft.setTextSize(1);
tft.setTextColor(TFT_RED, TFT_BLACK);
tft.setCursor(25, 40);
tft.println(fiat);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setCursor(30, 57);
tft.println(String(nosats));
}

void screen_qrdisplay(String XXX)
{  
 tft.fillScreen(TFT_WHITE);
  XXX.toUpperCase();
 const char* addr = XXX.c_str();
 Serial.println(addr);
  int qrSize = 12;
  int sizes[17] = { 14, 26, 42, 62, 84, 106, 122, 152, 180, 213, 251, 287, 331, 362, 412, 480, 504 };
  int len = String(addr).length();
  for(int i=0; i<17; i++){
    if(sizes[i] > len){
      qrSize = i+1;
      break;
    }
  }
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(qrSize)];
  qrcode_initText(&qrcode, qrcodeData, qrSize-1, ECC_LOW, addr);
  Serial.println(qrSize -1);

  float scale = 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if(qrcode_getModule(&qrcode, x, y)){
        tft.drawRect(15+2+scale*x, 2+scale*y, scale, scale, TFT_BLACK);
      }
      else{
        tft.drawRect(15+2+scale*x, 2+scale*y, scale, scale, TFT_WHITE);
      }
    }
  }
}

void screen_wifi_check()
{
tft.fillScreen(TFT_BLACK);
tft.setCursor(25, 40);
tft.setTextSize(1);
tft.setTextColor(TFT_RED);
tft.println("WIFI NOT CONNECTED");
}

void screen_refresh()
{
tft.fillScreen(TFT_BLACK);
tft.setCursor(0, 0);
tft.setTextColor(TFT_WHITE);
}

void screen_page_input()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0, 17);
  tft.println("AMOUNT THEN #");
  tft.println("");
  tft.println("");
  tft.println(on_currency.substring(3) + ": ");
  tft.println("");
  tft.println("SATS: ");
  tft.println("");
  tft.println("");
  tft.setTextSize(1);
  tft.setCursor(30, 110);
  tft.println("TO RESET PRESS *");
}

void screen_page_processing()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 40);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.println("PROCESSING");
}

void screen_complete()
{
tft.fillScreen(TFT_BLACK);
tft.setCursor(30, 40);
tft.setTextSize(2);
tft.setTextColor(TFT_GREEN);
tft.println("COMPLETE");
delay(1000);
}

void screen_cancel()
{
tft.fillScreen(TFT_BLACK);
tft.setCursor(23, 40);
tft.setTextSize(2);
tft.setTextColor(TFT_RED);
tft.println("CANCELLED");
delay(1000);
}


//Set keypad
const byte rows = 4; //four rows
const byte cols = 3; //three columns
char keys[rows][cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[rows] = {12, 14, 27, 26}; //connect to the row pinouts of the keypad
byte colPins[cols] = {25, 33, 32}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );


void get_keypad(){

       key_val = keypad.getKey();
       uint8_t key = 1;
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
