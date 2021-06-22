/*
    LilyGo Ink Screen Series
        - Created by Lewis he , Last updated 3/16/2021
*/

// According to the board, cancel the corresponding macro definition
#define LILYGO_T5_V213
// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28

#include <SD.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <cJSON.h>

#include <boards.h>
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeSans9pt7b.h>


#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AceButton.h>

#include "utilities.h"
#include "defConfig.h"


//Select the header file corresponding to the screen.

// #include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
// #include <GxGDEW0154Z17/GxGDEW0154Z17.h>  // 1.54" b/w/r 152x152
// #include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w

// #include <GxDEPG0150BN/GxDEPG0150BN.h>    // 1.51" b/w   form DKE GROUP

// #include <GxGDEW027C44/GxGDEW027C44.h>    // 2.7" b/w/r
// #include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w

// #include <GxGDEW0213Z16/GxGDEW0213Z16.h>  // 2.13" b/w/r

// old panel
// #include <GxGDEH0213B72/GxGDEH0213B72.h>  // 2.13" b/w old panel
// #include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w old panel
// #include <GxGDE0213B72B/GxGDE0213B72B.h> // 2.13" b/w           GxGDE0213B72 升级版本 默认LilyGO的出厂屏幕都是这种

#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP

// #include <GxGDEM0213B74/GxGDEM0213B74.h>  // 2.13" b/w  form GoodDisplay 4-color

// #include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w  form DKE GROUP

// #include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w
// #include <GxQYEG0290BN/GxQYEG0290BN.h>    // 2.9" b/w new panel
// #include <GxDEPG0290B/GxDEPG0290B.h>      // 2.9" b/w    form DKE GROUP

// #include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r
// #include <GxDEPG0290R/GxDEPG0290R.h>      // 2.9" b/w/r  form DKE GROUP

// #include <GxDEPG0750BN/GxDEPG0750BN.h>    // 7.5" b/w    form DKE GROUP

enum {
    GxEPD_ALIGN_RIGHT,
    GxEPD_ALIGN_LEFT,
    GxEPD_ALIGN_CENTER,
};

using namespace         ace_button;
AsyncWebServer          server(80);
GxIO_Class              io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class             display(io, EPD_RSET, EPD_BUSY);

const uint8_t           btns[] = BUTTONS;
const uint8_t           handle_btn_nums = sizeof(btns) / sizeof(*btns);



extern void drawBitmap(GxEPD &display, const char *filename, int16_t x, int16_t y, bool with_color);
void showBeerImage(void);
/****************************************************
 __          ___ ______ _
 \ \        / (_)  ____(_)
  \ \  /\  / / _| |__   _
   \ \/  \/ / | |  __| | |
    \  /\  /  | | |    | |
     \/  \/   |_|_|    |_|

****************************************************/

void setupWiFi(bool apMode)
{
    if (apMode) {
        uint8_t mac[6];
        char    apName[64];
        WiFi.mode(WIFI_AP);
        WiFi.macAddress(mac);
        sprintf(apName, "TTGO-Badge-%02X:%02X", mac[4], mac[5]);
        WiFi.softAP(apName);
    } else {
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
            Serial.print(".");
            esp_restart();
        }
        Serial.println("WiFi connected");
        Serial.println("");
        Serial.println(WiFi.localIP());
    }
}

/****************************************************************
 __          __  _     _____
 \ \        / / | |   / ____|
  \ \  /\  / /__| |__| (___   ___ _ ____   _____ _ __
   \ \/  \/ / _ \ '_ \\___ \ / _ \ '__\ \ / / _ \ '__|
    \  /\  /  __/ |_) |___) |  __/ |   \ V /  __/ |
     \/  \/ \___|_.__/_____/ \___|_|    \_/ \___|_|

****************************************************************/
static void asyncWebServerFileUploadCb(AsyncWebServerRequest *request, const String &filename,
                                       size_t index, uint8_t *data, size_t len, bool final)
{
    static File file;
    if (!index) {
        Serial.printf("UploadStart: %s\n", filename.c_str());
        file = FILESYSTEM.open(DEFAULT_IMAGE_BMP, FILE_WRITE);
        if (!file) {
            Serial.println("Open FAIL");
            request->send(500, "text/plain", "hander error");
            return;
        }
    }
    if (file.write(data, len) != len) {
        Serial.println("Write fail");
        request->send(500, "text/plain", "hander error");
        file.close();
        return;
    }

    if (final) {
        Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
        file.close();
        request->send(200, "text/plain", "");
        showBeerImage();
    }
}

static void asyncWebServerNotFoundCb(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}


static void setupWebServer(void)
{
    server.serveStatic("/", FILESYSTEM, "/").setDefaultFile("index.html");

    server.on("css/main.css", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "css/main.css", "text/css");
    });
    server.on("js/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "js/jquery.min.js", "application/javascript");
    });
    server.on("js/tbdValidate.js", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "js/tbdValidate.js", "application/javascript");
    });

    server.onFileUpload(asyncWebServerFileUploadCb);

    server.onNotFound(asyncWebServerNotFoundCb);

    MDNS.begin(MDNS_NAME);

    MDNS.addService("http", "tcp", 80);

    server.begin();
    Serial.println("WebServer started");
}

/****************************************************************
  ____            _              _____  _           _
 |  _ \          | |            |  __ \(_)         | |
 | |_) | __ _  __| | __ _  ___  | |  | |_ ___ _ __ | | __ _ _   _
 |  _ < / _` |/ _` |/ _` |/ _ \ | |  | | / __| '_ \| |/ _` | | | |
 | |_) | (_| | (_| | (_| |  __/ | |__| | \__ \ |_) | | (_| | |_| |
 |____/ \__,_|\__,_|\__, |\___| |_____/|_|___/ .__/|_|\__,_|\__, |
                     __/ |                   | |             __/ |
                    |___/                    |_|            |___/
****************************************************************/

void showBeerImage(void)
{
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    drawBitmap(display, DEFAULT_IMAGE_BMP, 0, 0, false);
    display.update();
}


/****************************************************************
 __  __       _
|  \/  |     (_)
| \  / | __ _ _ _ __
| |\/| |/ _` | | '_ \
| |  | | (_| | | | | |
|_|  |_|\__,_|_|_| |_|
****************************************************************/
void setup()
{
    Serial.begin(115200);

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();
    display.setRotation(1);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(DEFAULT_FONT);

    if (!FILESYSTEM.begin()) {
        Serial.println("FILESYSTEM initialization error.");
        Serial.println("FILESYSTEM formart ...");

        display.setCursor(0, 16);
        display.println("FILESYSTEM initialization error.");
        display.println("FILESYSTEM formart ...");
        display.update();
        FILESYSTEM.begin(true);
    }

    showBeerImage();

    setupWiFi(START_WIFI_AP);

    setupWebServer();

}

void loop()
{
    // aceButtonLoop();
}


