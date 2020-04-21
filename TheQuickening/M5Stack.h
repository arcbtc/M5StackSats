#include <M5Stack.h> 
#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

//keypad stuff
String key_val,key_val2;
String calc_sign ="";
bool calc_flag = false;


void screen_splash()
{  
  M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)SplashM5_map);
}

void screen_input_sats(String fiat, int nosats)
{
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
  M5.Lcd.setCursor(30, 70);
  M5.Lcd.println(fiat+" "+on_currency.substring(3)+"       ");
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Lcd.setCursor(30, 136);
  M5.Lcd.println(String(nosats)+" "+"sats"+"       ");
}

void screen_calc_sats(String fiat, String sign, String calc, String sum)
{
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.setCursor(30, 70);
  M5.Lcd.println(fiat+" "+sign+" "+calc+" "+on_currency.substring(3)+"       ");
  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
  M5.Lcd.setCursor(30, 136);
  M5.Lcd.println(sum+" "+on_currency.substring(3)+"       ");
}

void screen_qrdisplay(String xxx)
{  
  Serial.println(xxx);
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
  M5.Lcd.setCursor(30, 20);
  M5.Lcd.println("OK for Invoice");
  M5.Lcd.println("");
  M5.Lcd.println("");
  M5.Lcd.println("");
  M5.Lcd.println("");
  M5.Lcd.println("");
  M5.Lcd.println("");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(30, 200);
  M5.Lcd.println("Cancel            OK");
  
}

  void screen_page_calc()
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.println("M5StackSats");
    M5.Lcd.println("");
    M5.Lcd.println("");
    M5.Lcd.println("");
    M5.Lcd.println("");
    M5.Lcd.println("");
    M5.Lcd.println("");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(30, 200);
    M5.Lcd.println("Cancel    <      OK");
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
  M5.Speaker.tone(500);
  delay(500);
  M5.Speaker.tone(1000);
  delay(500);
  M5.Speaker.mute();
  delay(2000);
  M5.Speaker.tone(500);
  delay(500);
  M5.Speaker.tone(1000);
  delay(500);
  M5.Speaker.mute();
  delay(2000);
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
            Serial.print((String)"\nDIGIT "+key_val);
          }
          else if(key == 42 || key == 43 || key ==45 || key == 61){ //check if arithmetic sign
            Serial.print((String)"\nOPERATOR hex "+key);
            key_val = ((char)key);
            calc_sign= key_val;
            Serial.print((String)"\nOPERATOR "+key_val);
            key_val = "";
            calc_flag = true;
            
          } 
          else {
            Serial.print((String)"\nVOID "+key_val);
            key_val = "";
          } 
        }
      }
    }
  }
}
