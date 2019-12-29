# M5StackSats [LNPay.co](https://lnpay.co) Edition

### Step 0: Login / Create account on [LNPay.co](https://lnpay.co)

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
String wallet_key = "<wi_XXXXX_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/advanced-wallets
```
![screencast 2019-12-28 18-45-58](https://user-images.githubusercontent.com/3043754/71550595-83267d80-29a2-11ea-931c-fa2dc9c6bc2a.gif)


### Step 3: Payment / Invoice Details

```
//Payment Setup
String memo = "M5 "; //memo suffix, followed by a random number
String on_currency = "BTCUSD"; //currency can be changed here ie BTCUSD BTCGBP etc
```
