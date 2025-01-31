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
#include <qrencode.h>

#include <boards.h>
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansOblique12pt7b.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AceButton.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

#include "utilities.h"
#include "defConfig.h"
#include "create_qr.h"

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
//#include <GxGDE0213B72B/GxGDE0213B72B.h> // 2.13" b/w (blauwe tap) GxGDE0213B72 升级版本 默认LilyGO的出厂屏幕都是这种

#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP (zwarte tap)

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

typedef struct {
    char beername[64];
    char style[64];
    char abv[32];
    char ibu[32];
    char link[64];
} Beer_Info_t;

using namespace         ace_button;
AsyncWebServer          server(80);
GxIO_Class              io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class             display(io, EPD_RSET, EPD_BUSY);
Beer_Info_t             info;
AceButton               *btnPtr = NULL;

const uint8_t           btns[] = BUTTONS;
const uint8_t           handle_btn_nums = sizeof(btns) / sizeof(*btns);


extern void drawBitmap(GxEPD &display, const char *filename, int16_t x, int16_t y, bool with_color);
extern void createQrCode(GxEPD &display, String message);
void showBeerImage(void);
void showBeerInfo(void);
void saveBeerInfo(Beer_Info_t *info);
bool loadBeerInfo(Beer_Info_t *info);
void loadDefaultInfo(void);
void setupWiFi(bool apMode);

/****************************************************
  ____                   _____        __
 |  _ \                 |_   _|      / _|
 | |_) | ___  ___ _ __    | |  _ __ | |_ ___
 |  _ < / _ \/ _ \ '__|   | | | '_ \|  _/ _ \
 | |_) |  __/  __/ |     _| |_| | | | || (_) |
 |____/ \___|\___|_|    |_____|_| |_|_| \___/

****************************************************/
void saveBeerInfo(Beer_Info_t *info)
{
    File file = FILESYSTEM.open(BEER_CONFIG_FILE_NAME, FILE_WRITE);
    if (!file) {
        Serial.println(F("Failed to create file"));
        return;
    }
    cJSON *root =  cJSON_CreateObject();
    cJSON_AddStringToObject(root, "beername", info->beername);
    cJSON_AddStringToObject(root, "style", info->style);
    cJSON_AddStringToObject(root, "abv", info->abv);
    cJSON_AddStringToObject(root, "ibu", info->ibu);
    cJSON_AddStringToObject(root, "link", info->link);
    const char *str =  cJSON_Print(root);
    file.write((uint8_t *)str, strlen(str));
    file.close();
    cJSON_Delete(root);
}

bool loadBeerInfo(Beer_Info_t *info)
{
    if (!FILESYSTEM.exists(BEER_CONFIG_FILE_NAME)) {
        Serial.println("load configure fail");
        return false;
    }
    File file = FILESYSTEM.open(BEER_CONFIG_FILE_NAME);
    if (!file) {
        Serial.println("Open Fial -->");
        return false;
    }
    cJSON *root =  cJSON_Parse(file.readString().c_str());
    if (!root) {
        return false;
    }
    if (cJSON_GetObjectItem(root, "beername")->valuestring) {
        strlcpy(info->beername, cJSON_GetObjectItem(root, "beername")->valuestring, sizeof(info->beername));
    }
    if (cJSON_GetObjectItem(root, "style")->valuestring) {
        strlcpy(info->style, cJSON_GetObjectItem(root, "style")->valuestring, sizeof(info->style));
    }
    if (cJSON_GetObjectItem(root, "abv")->valuestring) {
        strlcpy(info->abv, cJSON_GetObjectItem(root, "abv")->valuestring, sizeof(info->abv));
    }
    if (cJSON_GetObjectItem(root, "ibu")->valuestring) {
        strlcpy(info->ibu, cJSON_GetObjectItem(root, "ibu")->valuestring, sizeof(info->ibu));
    }
    if (cJSON_GetObjectItem(root, "link")->valuestring) {
        strlcpy(info->link, cJSON_GetObjectItem(root, "link")->valuestring, sizeof(info->link));
    }

    file.close();
    cJSON_Delete(root);
    return true;
}


void loadDefaultInfo(void)
{
    Serial.println("Load default data");
    strlcpy(info.beername, "Beer name",  sizeof(info.beername));
    strlcpy(info.style,    "Beer style",      sizeof(info.style));
    strlcpy(info.abv,      "5",              sizeof(info.abv));
    strlcpy(info.ibu,      "25",             sizeof(info.ibu));
    strlcpy(info.link,     "https://untappd.com/qr/beer/[BEERID]", sizeof(info.link));
    saveBeerInfo(&info);
}

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
        sprintf(apName, "TapHandle-%02X:%02X", mac[4], mac[5]);
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

static void asyncWebServerDataPostCb(AsyncWebServerRequest *request)
{
    Serial.println("post data");
    
    for (int i = 0; i < request->params(); i++) {
        String name = request->getParam(i)->name();
        String params = request->getParam(i)->value();
        Serial.println(name + " : " + params);

        if (name == "beername") {
            strlcpy(info.beername, params.c_str(), sizeof(info.beername));
        } else if (name == "style") {
            strlcpy(info.style, params.c_str(), sizeof(info.style));
        } else if (name == "abv") {
            strlcpy(info.abv, params.c_str(), sizeof(info.abv));
        } else if (name == "ibu") {
            strlcpy(info.ibu, params.c_str(), sizeof(info.ibu));
        } else if (name == "link") {
            strlcpy(info.link, params.c_str(), sizeof(info.link));
        }
    }

    saveBeerInfo(&info);

    request->send(200, "text/plain", "");

    showBeerInfo();
}

static void asyncWebServerDataGetBeerInfoCb(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(1024);
    
    root["beername"] = info.beername;
    root["style"] = info.style;
    root["abv"] = round(String(info.abv).toFloat() * 10) / 10;
    root["ibu"] = String(info.ibu).toInt();
    root["link"] = info.link;
    serializeJson(root, *response);
    request->send(response);
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
    server.on("js/main.js", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(FILESYSTEM, "js/main.js", "application/javascript");
    });

    server.on("/data", HTTP_POST, asyncWebServerDataPostCb);
    server.on("/info", HTTP_GET, asyncWebServerDataGetBeerInfoCb);

    server.onNotFound(asyncWebServerNotFoundCb);

    MDNS.begin(MDNS_NAME);

    MDNS.addService("http", "tcp", 80);

    server.begin();
    Serial.println("WebServer started");
}


/*****************************************************************
  ____                   _____  _           _
 |  _ \                 |  __ \(_)         | |
 | |_) | ___  ___ _ __  | |  | |_ ___ _ __ | | __ _ _   _
 |  _ < / _ \/ _ \ '__| | |  | | / __| '_ \| |/ _` | | | |
 | |_) |  __/  __/ |    | |__| | \__ \ |_) | | (_| | |_| |
 |____/ \___|\___|_|    |_____/|_|___/ .__/|_|\__,_|\__, |
                                     | |             __/ |
                                     |_|            |___/
*****************************************************************/

static void displayText(const char *str, int16_t y, uint8_t align)
{
    int16_t x = 0;
    int16_t x1 = 0, y1 = 0;
    uint16_t w = 0, h = 0;
    display.setCursor(x, y);
    display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    if (w > 120 || h > 30) {
        display.setFont(&FreeSerif12pt7b);
        display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    }
    if (w > 120 || h > 30) {
        display.setFont(&FreeSerifBold12pt7b);
        display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    }
    if (w > 120 || h > 30) {
        display.setFont(&FreeSerif12pt7b);
        display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    }
    if (w > 120 || h > 30) {
        display.setFont(&FreeSerifBold9pt7b);
        display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    }
    if (w > 120 || h > 30) {
        display.setFont(&FreeSerif9pt7b);
        display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    }
    Serial.printf("w: %u h: %u\n", w, h);
    switch (align) {
    case GxEPD_ALIGN_RIGHT:
        display.setCursor(display.width() - w - x1, y);
        break;
    case GxEPD_ALIGN_LEFT:
        display.setCursor(0, y);
        break;
    case GxEPD_ALIGN_CENTER:
        display.setCursor(display.width() / 2 - ((w + x1) / 2), y);
        break;
    default:
        break;
    }
    display.println(str);
}


void showBeerInfo(void)
{
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeSerifBold18pt7b);
    displayText(info.beername, 30, GxEPD_ALIGN_CENTER);
    displayText(info.style, 65, GxEPD_ALIGN_CENTER);
    display.setFont(&FreeSerifBold12pt7b);
    char abv[30];
    sprintf(abv, "%.1f %%", String(info.abv).toFloat());
    displayText(abv, 95, GxEPD_ALIGN_CENTER);
    char ibu[30];
    sprintf(ibu, "%.0f IBU", String(info.ibu).toFloat());
    displayText(ibu, 120, GxEPD_ALIGN_CENTER);
    display.setFont(DEFAULT_FONT);
    createQrCode(display, info.link, 0, 135);
    display.update();
}

/****************************************************
                   ____        _   _
     /\           |  _ \      | | | |
    /  \   ___ ___| |_) |_   _| |_| |_ ___  _ __
   / /\ \ / __/ _ \  _ <| | | | __| __/ _ \| '_ \
  / ____ \ (_|  __/ |_) | |_| | |_| || (_) | | | |
 /_/    \_\___\___|____/ \__,_|\__|\__\___/|_| |_|
****************************************************/

static void socEnterSleepMode(void)
{
    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
    Serial.println("Going to sleep now");
    delay(2000);
    esp_deep_sleep_start();
}


static void singleButtonHandleCb(uint8_t event)
{
    if (event == 0) {
        showBeerInfo();
    } else if (event == AceButton::kEventLongPressed) {
        socEnterSleepMode();
    }
}


static void aceButtonHandleEventCb(AceButton *b, uint8_t event, uint8_t state)
{
    Serial.printf("Pin:%d event:%u state:%u\n", b->getPin(), event, state);
    singleButtonHandleCb(event);
}

static void setupButton(void)
{
    if (btnPtr) {
        return;
    }
    btnPtr = new  AceButton [handle_btn_nums];
    for (int i = 0; i < handle_btn_nums; ++i) {
        pinMode(btns[i], INPUT);
        btnPtr[i].init(btns[i]);
        ButtonConfig *buttonConfig = btnPtr[i].getButtonConfig();
        buttonConfig->setEventHandler(aceButtonHandleEventCb);
        buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
        buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
        buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    }
}

static void aceButtonLoop(void)
{
    if (!btnPtr) {
        return;
    }
    for (int i = 0; i < handle_btn_nums; ++i) {
        btnPtr[i].check();
    }
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
    setupButton();

    if (!loadBeerInfo(&info)) {
        loadDefaultInfo();
    }

    showBeerInfo();

    setupWiFi(START_WIFI_AP);

    setupWebServer();

}

void loop()
{
    // showBeerInfo()
    aceButtonLoop();
}


