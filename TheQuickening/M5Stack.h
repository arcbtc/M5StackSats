

#include <M5Stack.h> 
#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

//keypad stuff
String key_val;

void screen_splash()
{  
M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)Splash_map);
}

void screen_input_sats(String fiat, int nosats)
{
M5.Lcd.setTextSize(3);
M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
M5.Lcd.setCursor(70, 88);
M5.Lcd.println(fiat);
M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
M5.Lcd.setCursor(87, 136);
M5.Lcd.println(String(nosats));
}

void screen_qrdisplay(String xxx)
{  
M5.Lcd.fillScreen(BLACK); 
M5.Lcd.qrcode(xxx,45,0,240,14);
delay(100);
}

void screen_wifi_check()
{
M5.Lcd.fillScreen(BLACK);
M5.Lcd.setCursor(55, 80);
M5.Lcd.setTextSize(2);
M5.Lcd.setTextColor(TFT_RED);
M5.Lcd.println("WIFI NOT CONNECTED");
}

void screen_refresh()
{
M5.Lcd.fillScreen(BLACK);
M5.Lcd.setCursor(0, 0);
M5.Lcd.setTextColor(TFT_WHITE);
}

void screen_page_input()
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

void screen_page_processing()
{ 
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(40, 80);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.println("PROCESSING");
}

void screen_complete()
{
M5.Lcd.fillScreen(BLACK);
M5.Lcd.setCursor(60, 80);
M5.Lcd.setTextSize(4);
M5.Lcd.setTextColor(TFT_GREEN);
M5.Lcd.println("COMPLETE");
delay(1000);
}

void screen_cancel()
{
M5.Lcd.fillScreen(BLACK);
M5.Lcd.setCursor(50, 80);
M5.Lcd.setTextSize(4);
M5.Lcd.setTextColor(TFT_RED);
M5.Lcd.println("CANCELLED");
delay(1000);
}

void get_keypad(){
  M5.update();
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
