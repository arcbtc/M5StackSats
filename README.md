![M5StackSats](https://i.imgur.com/arHP2MD.png)
# M5StackSats
A bitcoin Lightning-Network Point of Sale terminal using the ESP32 based M5Stack Faces Development Kit. Included in the kit is a keypad and charging dock, making it an ideal candidate for an cheap($55)/easy DIY bitcoin point of sale terminal.
https://m5stack.com/products/face

[![bitcoinpos](https://i.imgur.com/sHp1AXw.png)](https://www.youtube.com/watch?v=KPI54s1xXsc)

There are two implementations of M5StackSats, one that communicates directly with an LND node, and another that communicates with opennode (make an account here https://opennode.co/join/f774f2a0-1377-45e2-b719-6b821f24900d). 

Converting the project to communicate with c-lightning nodes/other custodial services should be easy.


# Installing arduino + libraries

Install the Arduino IDE,<br>
https://www.arduino.cc/en/Main/Software

Install the ESP32 hardware,<br>
https://github.com/espressif/arduino-esp32#installation-instructions

From "Manage Libraries" install,<br>
- LittlevGL
- ArduinoJson
- M5Stack

![a](https://i.imgur.com/mCfnhZN.png)




