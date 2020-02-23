#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "PaymentInvoice.h"
#include "PaymentServer.h"
#include "PaymentServerLND.h"

String lndServer;
int lndRestApiPort;
String readmacaroon;
String invoicemacaroon;
String tlscert;
HTTPClient http;

String PaymentServerLND::getServiceName() { return (String)"LND"; }

bool PaymentServerLND::init() {

    // Credentials Setup
    readmacaroon = "0201036c6e64028a01030a105dc5e03a7e5444d4ebbdbb513d1fc40f1201301a0f0a07616464726573731204726561641a0c0a04696e666f1204726561641a100a08696e766f696365731204726561641a0f0a076d6573736167651204726561641a100a086f6666636861696e1204726561641a0f0a076f6e636861696e1204726561641a0d0a05706565727312047265616400000620b964213f708bc349dc1de651b424817651858fcfffa38345f6c566053ee22cf5";
    invoicemacaroon = "0201036c6e640247030a105ec5e03a7e5444d4ebbdbb513d1fc40f1201301a160a0761646472657373120472656164120577726974651a170a08696e766f69636573120472656164120577726974650000062062630006bd8cc1f7ce6be81d90dd2c1b754ecf7d7d0d794fe2c36fec5680876e";
    tlscert= "";

    // Endpoint Setup
    lndServer = "room77.raspiblitz.com";
    lndRestApiPort = 8080;

    return true;
}

PaymentInvoice PaymentServerLND::getInvoice(int sats, String memo) { 

    String data = "{\"value\": \""+ (String)sats +"\", \"memo\": \""+ memo +"\", \"expiry\": \"1000\"}";
    String content = createRequest((String)"POST",(String)"/v1/invoices",(String)data,(String)invoicemacaroon);
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(3) + 320);
    deserializeJson(doc, content);
    //const char* r_hash = doc["r_hash"];
    //hash = r_hash;
    const char* payment_request = doc["payment_request"]; 
    payment_request;
 
    /* DELETE IF ABOVE IS WORKING
    if (!client.connect(server, lndport)){
      return;   
    }
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
 
    */

    content = createRequest((String)"GET",(String)"/v1/payreq/"+payment_request,(String)"",readmacaroon);
    DynamicJsonDocument doc1(JSON_OBJECT_SIZE(7) + 270);
    deserializeJson(doc1, content);
    const char* payment_hash = doc1["payment_hash"]; 

    /* DELETE IF ABOVE IS WORKING
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
    */

    // TODO

    PaymentInvoice invoice;
    invoice.paymentRequest = (String)payment_request;
    invoice.id = (String)payment_hash;
    return invoice;
}

bool PaymentServerLND::isInvoicePaid(String id) {
    
    String content = createRequest((String)"GET",(String)"/v1/invoice/"+id, (String)"", readmacaroon);
    const size_t capacity = JSON_OBJECT_SIZE(9) + 460;
    DynamicJsonDocument doc2(capacity);
    deserializeJson(doc2, content);
    int settle = doc2["settled"];
    if (settle == 1){
      return true;
    } else{
      return false;
    }

  /* DELETE IF ABOVE IS WORKING
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
    */

}

// PRIVATE METHODS

String PaymentServerLND::createRequest(String method, String path, String data, String macaroon)
  {

    String url = lndServer + (String)lndRestApiPort + path;
    Serial.println("createRequest BEGIN-------");
    Serial.println("METHOD:" + method);
    Serial.println("URL:" + url);
    Serial.println("DATA:" + data);
    
    http.begin(url);
    http.addHeader("Content-Type","application/json");
    http.addHeader("Grpc-Metadata-macaroon:",macaroon);
    
    int httpCode;
    if (method.equals("POST"))
      httpCode = http.POST(data);
    else if (method.equals("GET")) {
      httpCode = http.GET();
    } else {
      Serial.println("This HTTP method usage is not defined");
    }

    String payload;
    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println("RESPONSE:" + payload);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
    Serial.println("createRequest END--------");
   
    return payload;
  }