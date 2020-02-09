#include <M5Stack.h> 
#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5

void screen_input_sats(String fiat, String nosats)
{
tft.setTextSize(3);
tft.setTextColor(TFT_RED, TFT_BLACK);
tft.setCursor(70, 88);
tft.println(fiat);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setCursor(87, 136);
tft.println(nosats);
}

void screen_page_qrdisplay(String xxx)
{  
tft.fillScreen(BLACK); 
tft.qrcode(payreq,45,0,240,14);
delay(100);
}

void screen_wifi_check()
{
tft.fillScreen(BLACK);
tft.setCursor(55, 80);
tft.setTextSize(2);
tft.setTextColor(TFT_RED);
tft.println("WIFI NOT CONNECTED");
}

void screen_refresh()
{
tft.fillScreen(BLACK);
tft.setCursor(0, 0);
tft.setTextColor(TFT_WHITE);
}

void screen_page_input()
{
  tft.fillScreen(BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 40);
  tft.println("Amount then C");
  tft.println("");
  tft.println(on_currency.substring(3) + ": ");
  tft.println("");
  tft.println("SATS: ");
  tft.println("");
  tft.println("");
  tft.setTextSize(2);
  tft.setCursor(50, 200);
  tft.println("TO RESET PRESS A");
}

void screen_page_processing()
{ 
  tft.fillScreen(BLACK);
  tft.setCursor(40, 80);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.println("PROCESSING");
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
