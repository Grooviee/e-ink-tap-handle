
 <h1 align = "center">Beer handle e-ink taphandle</h1>

# Use PlatformIO
1. Install the correct serial port driver [CP210X Driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
2. a. For the first use, you need to select the model you are using at the top of `main.cpp`, and select the corresponding screen header file
   b. Optional you can change some settings in the `src/defConfig.h`
3. Open directly to change your serial port in `platformio.ini`, click compile
4. Terminal run `pio run --target uploadfs` to upload the FS file to ESP32
5. Terminal run `pio run --target upload` to upload the code to the badge

## Library link used :
- [Fork branch/lewisxhe/GxEPD](https://github.com/lewisxhe/GxEPD)
- [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AceButton](https://github.com/bxparks/AceButton)


## Configure Badge Website

- **Configure the badge by entering http://ttgo.local or ip address in your browser.**

- **The display will be refreshed after submission**

## CAUTION:
- Currently only supports 120*250px BMP format as image file for uploading
