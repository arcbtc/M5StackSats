///USER SETUP///

//HARDWARE Uncomment for hardware used//
#define M5STACK //Based on M5Stack Faces Kit
//#define DIY //Based on ESP32/1.8TFT/Keypad Matrix

String PAYMENTSERVER = "LND"; 

//WIFI Setup
char wifiSSID[] = <SSID>;
char wifiPASS[] = <PASSWORD>;

//Payment Setup 
String memoBase = "PoS "; //memo suffix, followed by a random number
String memo="";
String currencyBase="EUR";
String on_currency = "BTC"+currencyBase; //currency can be changed here ie BTCUSD BTCGBP etc

//LND Setup
String LNDinvoiceMacaroonHex = <MACAROON_INVOICE>;
String LNDreadMacaroonHex = <MACAROON_READ>;
int    LNDport = 8080;
String LNDserver = <IP LND SERVER>;


////END OF USER SETUP////
