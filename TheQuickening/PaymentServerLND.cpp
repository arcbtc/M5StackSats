#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#include "PaymentInvoice.h"
#include "PaymentServer.h"
#include "PaymentServerLND.h"

String lndServer;
int lndRestApiPort;
String readmacaroon;
String invoicemacaroon;
String tlscert;

String PaymentServerLND::getServiceName() { return (String)"LND"; }

bool PaymentServerLND::init(String server, int restPort, String readMacaroonHex, String invoiceMacaroonHex) {

    // Endpoint Setup
    lndServer = server; // domain or IP
    lndRestApiPort = restPort; // default is 8080

    // Credentials Setup (the HEX Strings from RaspiBlitz EXPORT)
    readmacaroon = readMacaroonHex;
    invoicemacaroon = invoiceMacaroonHex;
    tlscert= ""; // ignore for now

    return true;
}

PaymentInvoice PaymentServerLND::getInvoice(int sats, String memo) { 

  PaymentInvoice invoice;

   WiFiClientSecure client;
  // client.setCACert(test_root_ca);

  // server string to char pointer
  char server[100];
  lndServer.toCharArray(server,100);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, lndRestApiPort)){
      invoice.id = (String)"error";
      return invoice;
  }

   String topost = "{\"value\": \""+ (String)sats +"\", \"memo\": \""+ memo +"\", \"expiry\": \"1000\"}";
  
       client.print(String("POST ")+ "https://" + lndServer + ":" + String(lndRestApiPort) + "/v1/invoices HTTP/1.1\r\n" +
                 "Host: "  + lndServer +":"+ String(lndRestApiPort) +"\r\n" +
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
    
    const size_t capacity = JSON_OBJECT_SIZE(3) + 512;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, content);
    const char* r_hash = doc["r_hash"];
    const char* payment_request = doc["payment_request"]; 
    invoice.paymentRequest = payment_request;

    Serial.println("\nStarting connection to server...");
    if (!client.connect(server, lndRestApiPort)){
      invoice.id = (String)"error";
      return invoice;
    }

    client.println(String("GET ") + "https://" + server +":"+ String(lndRestApiPort) + "/v1/payreq/"+ invoice.paymentRequest +" HTTP/1.1\r\n" +
                 "Host: "  + server +":"+ String(lndRestApiPort) +"\r\n" +
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
    content = client.readStringUntil('\n');
    client.stop();

    const size_t capacity2 = JSON_OBJECT_SIZE(7) + 270;
    DynamicJsonDocument doc2(capacity2);
    deserializeJson(doc2, content);

    const char* payment_hash = doc2["payment_hash"]; 
    invoice.id = payment_hash;
    Serial.println("invoice.id:" + invoice.id);
    Serial.println("invoice.paymentRequest:" + invoice.paymentRequest);
    return invoice;
}

bool PaymentServerLND::isInvoicePaid(String id) {

   WiFiClientSecure client;
  // client.setCACert(test_root_ca);

  // server string to char pointer
  char server[100];
  lndServer.toCharArray(server,100);    
  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, lndRestApiPort)){
       Serial.println("\nServer connect fail");
       return false;
  }
  client.println(String("GET ") + "https://" + lndServer +":"+ String(lndRestApiPort) + "/v1/invoice/"+ id +" HTTP/1.1\r\n" +
                 "Host: "  + lndServer +":"+ String(lndRestApiPort) +"\r\n" +
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
  //Serial.println(content);
  client.stop();
  const size_t capacity = JSON_OBJECT_SIZE(9) + 460;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, content);
  return doc["settled"]; 

}
