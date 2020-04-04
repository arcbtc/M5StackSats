#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "PaymentInvoice.h"
#include "PaymentServer.h"
#include "PaymentServerLNPay.h"

String api_key;
String wallet_key;
String api_endpoint;
String invoice_create_endpoint;
String invoice_check_endpoint;

String PaymentServerLNPay::getServiceName() { return (String)"LNPAY"; }

bool PaymentServerLNPay::init() {

    //API Setup
    api_key = ""; //  Public API key...Can be found here: https://lnpay.co/dashboard/integrations
    wallet_key = ""; // Invoice key...Can be found here: https://lnpay.co/dashboard/advanced-wallets
  
    //Endpoint Setup
    api_endpoint = "https://lnpay.co/v1";
    invoice_create_endpoint = "/wallet/" + wallet_key + "/invoice";
    invoice_check_endpoint = "/lntx/{{tx_id}}?fields=settled"; //append LNTX ID to the end (e.g. /user/lntx/lntx_mfEKSse22)

    return true;
}

PaymentInvoice PaymentServerLNPay::getInvoice(int sats, String memo) { 

    String toPost = "{  \"num_satoshis\" : " + (String) sats +", \"memo\" :\""+ memo + String(random(1,1000)) + "\"}";

    String payload = createRequest((String)"POST",invoice_create_endpoint,toPost);

    const size_t capacity = JSON_OBJECT_SIZE(2) + 500;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, payload);
    Serial.println(payload);

    const char* payment_request = doc["payment_request"]; 
    const char* id = doc["id"]; 
    String payreq = (String) payment_request;
    String lntx_id = (String) id;
    Serial.println(payreq);
    Serial.println(lntx_id);

    PaymentInvoice invoice;
    invoice.paymentRequest = payreq;
    invoice.id = lntx_id;
    return invoice;
}

bool PaymentServerLNPay::isInvoicePaid(String id) { 

    invoice_check_endpoint.replace("{{tx_id}}",id);

    String payloadd = createRequest((String)"GET",invoice_check_endpoint,(String)"");
    delay(1500);
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payloadd);
    int Settled = doc["settled"];
    Serial.println(Settled);

    if (Settled == 1){
      return true;
    } else{
      return false;
    }

}

// PRIVATE METHODS

String PaymentServerLNPay::createRequest(String method, String path, String data)
  {
    String payload;
    int httpCode;
    HTTPClient http;

    Serial.println("PaymentConnector::createRequest BEGIN-------");
    Serial.println("METHOD:" + method);
    Serial.println("URL:" + api_endpoint + path);
    Serial.println("DATA:" + data);
    
    http.begin(PaymentServerLNPay::api_endpoint + path); //Getting fancy to response size
    http.addHeader("Content-Type","application/json");
    http.addHeader("X-Api-Key",PaymentServerLNPay::api_key);
    
    if (method.equals("POST"))
      httpCode = http.POST(data); //Make the request
    else if (method.equals("GET")) {
      httpCode = http.GET(); //Make the request
    } else {
      Serial.println("This HTTP method usage is not defined");
    }

    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println("RESPONSE:" + payload);
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end(); //Free the resources
    Serial.println("PaymentConnector::createRequest END--------");
   
    return payload;
  }
