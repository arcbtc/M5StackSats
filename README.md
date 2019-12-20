![M5StackSats](https://i.imgur.com/zGZpYhn.png)

# M5StackSats

A bitcoin Lightning-Network Point of Sale terminal using the ESP32 based M5Stack Faces Development Kit. Included in the kit is a keypad and charging dock, making it an ideal candidate for an cheap($55)/easy DIY bitcoin point of sale terminal.
https://m5stack.com/products/face

[![bitcoinpos](https://i.imgur.com/sHp1AXw.png)](https://www.youtube.com/watch?v=KPI54s1xXsc)

There are three implementations of M5StackSats, one that communicates directly with an LND node, and another that communicates with opennode (make an account here https://opennode.co/join/f774f2a0-1377-45e2-b719-6b821f24900d), and another that sits between the two projects using a neutrino LND node via Zap desktop wallet(https://docs.zaphq.io/docs-desktop-neutrino-connect).

Converting the project to communicate with c-lightning nodes/other custodial services should be easy.

## Tutorial

[![bitcoinpos](https://i.imgur.com/uwLAbot.png)](https://www.youtube.com/watch?v=o4jqUbmypRQ)

## Installing Arduino + libraries

Install the
[Arduino IDE](https://www.arduino.cc/en/Main/Software) and
[ESP32 hardware](https://github.com/espressif/arduino-esp32#installation-instructions).

From "Manage Libraries" install,

- LittlevGL
- ArduinoJson
- M5Stack

![a](https://i.imgur.com/mCfnhZN.png)

## Notes

To create a splash image, you can use the
[Online Image to C Array Converter](https://littlevgl.com/image-to-c-array).

Here you can find some more information on the
[client server communication](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/client-examples.html).

For parsing JSON responses and calculating the capacity you can use the
[ArduinoJson Assistant](https://arduinojson.org/v6/assistant/) tool.

## Tip me

If you like this project and might even use or extend it, why not
[send some tip love](https://paywall.link/to/f4e4e)!
