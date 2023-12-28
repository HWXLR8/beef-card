# 牛肉卡
This project uses the PN532 RFID/NFC transceiver module to read FeliCa based e-amusement cards to card-in to Bemani 
rhythm games using the SpiceTools API. If your spice deployment uses an API password, make sure to change the following 
line in the sketch before flashing.
```
#define SPICEAPI_PASSWORD ""
```
All libraries are included in the source tree for your convenience. Connect the PN532 to your MCU's I2C bus and you're good to go.
