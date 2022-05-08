#include <qrencode.h>

int offsetsX = 20;
int offsetsY = 20;
int screenwidth = 120;
int screenheight = 120;

int multiply = 1;

void initSettings(int startX, int startY) {
  int min = screenwidth;
  if (screenheight<screenwidth)
    min = screenheight;
  multiply = min/WD;
  offsetsX = (screenwidth-(WD*multiply))/2;
  offsetsY = startY;
}

void renderPixel(GxEPD &display, int x, int y, int color){
  multiply = 2;
  x=(x*multiply)+offsetsX;
  y=(y*multiply)+offsetsY;
  if(color==1) {
    display.drawPixel(x,y,GxEPD_BLACK);
    if (multiply>1) {
      display.drawPixel(x+1,y,GxEPD_BLACK);
      display.drawPixel(x+1,y+1,GxEPD_BLACK);
      display.drawPixel(x,y+1,GxEPD_BLACK);
    }
  } else {
    display.drawPixel(x,y,GxEPD_WHITE);
    if (multiply>1) {
      display.drawPixel(x+1,y,GxEPD_WHITE);
      display.drawPixel(x+1,y+1,GxEPD_WHITE);
      display.drawPixel(x,y+1,GxEPD_WHITE);
    }
  }
}


void createQrCode(GxEPD &display, String message, int startX, int startY) {

  initSettings(startX, startY);
  // create QR code
  message.toCharArray((char *)strinbuf,260);
  qrencode();

  // print QR Code
  for (byte x = 0; x < WD; x+=2) {
    for (byte y = 0; y < WD; y++) {
      if ( QRBIT(x,y) &&  QRBIT((x+1),y)) {
        // black square on top of black square
        renderPixel(display, x, y, 1);
        renderPixel(display, (x+1), y, 1);
      }
      if (!QRBIT(x,y) &&  QRBIT((x+1),y)) {
        // white square on top of black square
        renderPixel(display, x, y, 0);
        renderPixel(display, (x+1), y, 1);
      }
      if ( QRBIT(x,y) && !QRBIT((x+1),y)) {
        // black square on top of white square
        renderPixel(display, x, y, 1);
        renderPixel(display, (x+1), y, 0);
      }
      if (!QRBIT(x,y) && !QRBIT((x+1),y)) {
        // white square on top of white square
        renderPixel(display, x, y, 0);
        renderPixel(display, (x+1), y, 0);
      }
    }
  }
}