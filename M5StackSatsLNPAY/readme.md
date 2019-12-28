#LNPay M5StackSats

### Step 1: WiFi Configuration

```
//WIFI Setup
char wifiSSID[] = "<your_wifi_ssid>";
char wifiPASS[] = "<your_wifi_pass>";
```


### Step 2: API / Wallet Keys

```
//API Setup
String api_key = "<api_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/integrations
String wallet_key = "<wallet_invoice_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/advanced-wallets
```



### Step 3: Payment / Invoice Details

```
//Payment Setup
String memo = "M5 "; //memo suffix, followed by a random number
String on_currency = "BTCUSD"; //currency can be changed here ie BTCUSD BTCGBP etc
```