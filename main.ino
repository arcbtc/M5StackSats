#include <M5Stack.h>
#include <WiFiClientSecure.h>
#include "PaymentConnector.h"

PaymentConnector lnpay("BTCUSD");

void setup() {
  Serial.begin(115200);
  M5.begin();
  Wire.begin();
  WiFi.begin("SSID", "PASSWORD");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(i >= 5){
     M5.Lcd.fillScreen(BLACK);
     M5.Lcd.setCursor(55, 80);
     M5.Lcd.setTextSize(2);
     M5.Lcd.setTextColor(TFT_RED);
     M5.Lcd.println("WIFI NOT CONNECTED");
    }
    i++;
  }
  
  
    

}

void loop() {
  // put your main code here, to run repeatedly:
  
  
  //createInvoiceResponse resp = lnpay.createInvoice(20,"Memo!");
  
  int settled = 0;
  while (!settled) {
    settled = lnpay.checkIfPaymentIsSettled(resp.payment_id);
    Serial.println("SETTLED:" + settled);
    delay(1000);
  }
  
}
