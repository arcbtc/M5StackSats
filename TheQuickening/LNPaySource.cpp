/**
 * LNPay Payment Connector
 *
 * @author Tim Kijewski <bootstrapbandit7@gmail.com>
 *
 */
 
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "PaymentConnector.h"

  //API Setup
  String api_key = "3zB1DPk2bFwiocmjOasLJA4Tpfc2jUDB"; //  Public API key...Can be found here: https://lnpay.co/dashboard/integrations
  String wallet_key = "wi_ghDiaaRxecSHGcV8JrmITOtb"; // Invoice key...Can be found here: https://lnpay.co/dashboard/advanced-wallets
  

  //Endpoint Setup
  String api_endpoint = "https://lnpay.co/v1";
  String invoice_create_endpoint = "/wallet/" + wallet_key + "/invoice";
  String invoice_check_endpoint = "/lntx/{{tx_id}}?fields=settled"; //append LNTX ID to the end (e.g. /user/lntx/lntx_mfEKSse22)

  //HTTP client
  HTTPClient http;


  //Constructor
  PaymentConnector::PaymentConnector (String currency_pair) {
    _currency_pair = currency_pair;
  }


  /**
   * PaymentConnector::createRequest
   * 
   * @param String method - GET,POST
   * @param String path - REST path to be appended to api_endpoint
   * @param String data - string of data for POST requests
   * 
   * @return String - response data from request
   * 
   */
  String PaymentConnector::createRequest(String method,String path, String data)
  {
    String payload;
    int httpCode;

    Serial.println("PaymentConnector::createRequest BEGIN-------");
    Serial.println("METHOD:" + method);
    Serial.println("URL:" + api_endpoint + path);
    Serial.println("DATA:" + data);
    
    http.begin(api_endpoint + path); //Getting fancy to response size
    http.addHeader("Content-Type","application/json");
    http.addHeader("X-Api-Key",api_key);
    
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

  /**
   * PaymentConnector::createInvoice
   * 
   * @param int num_satoshis - number of satoshis for invoice
   * @param String memo - Memo for invoice
   * 
   * @return createInvoiceResponse - object containing both a payment_request AND an ID for lookup
   * 
   */
   
  createInvoiceResponse PaymentConnector::createInvoice(int num_satoshis, String memo) {
    String toPost = "{  \"num_satoshis\" : " + (String) num_satoshis +", \"memo\" :\""+ memo + String(random(1,1000)) + "\"}";
    
    String payload = PaymentConnector::createRequest((String)"POST",invoice_create_endpoint,toPost);
    
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

    createInvoiceResponse cir;

    cir.payment_request = payreq;
    cir.payment_id = lntx_id;

    return cir;
  }

  /**
   * PaymentConnector::checkIfPaymentIsSettled
   * 
   * @param String id - provided from the PaymentConnector::createInvoice call to check if settled
   * 
   * @return int - 1 or 0 depending on settled
   * 
   */
   
  int PaymentConnector::checkIfPaymentIsSettled(String id) {
    invoice_check_endpoint.replace("{{tx_id}}",id);
    String payloadd = PaymentConnector::createRequest((String)"GET",invoice_check_endpoint);
    delay(1500);
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payloadd);
    int Settled = doc["settled"];
    
    if (Settled == 1){
      return 1;
    } else{
      return 0;
    }
  }
