#include <SPI.h>
#include <TFT_eSPI.h>

// NeoPixel Values
#define PIXELPIN   5
#define NUMPIXELS  5
#define pixlux    20  //saturation level for NeoPixels colors

#define BUTTON_1        35
#define BUTTON_2        0

TFT_eSPI tft = TFT_eSPI(135, 240);

//============================= game variables =================================
unsigned long offsetM = 0;
unsigned long offsetT = 0;
unsigned long offsetF = 0;
unsigned long offsetB = 0;
unsigned long offsetA = 0;
unsigned long offsetAF = 0;
unsigned long offsetAB = 0;
unsigned long offsetS = 0;

int threshold = 40;

boolean startPrinted = false;
boolean beginGame = false;
boolean beginGame2 = true;
boolean play = false;
int score = 0;
int scoreInc = 10;
int level = 1;

//---------------------Player-----------------------------------------------
int shipX = 50;
int shipY = 200;
int oldShipX = 0;
int oldShipY = 0;
int changeShipX = 0;
int changeShipY = 0;
int shipSpeed = 50;
boolean doSplode = false;
boolean fire = true;

int fFireX[5] = {0, 0, 0, 0, 0};
int fFireY[5] = {0, 0, 0, 0, 0};
int fFireAge[5] = {0, 0, 0, 0, 0};


//--------------------------Aliens-----------------------------------------------
const int numberOfalien = 9;
boolean alienLive[numberOfalien];
int alienLiveCount = numberOfalien;
int alienX = 7;
int alienY = 25;
int oldAlienX = 7;
int oldAlienY = 25;
int changeAlienX = 6;
int changeAlienY = 0;
int alienSpeed = 200;
int oldAlienSpeed;

int aFireX[5];
int aFireY[5];
boolean aFireAge[5];
int chanceOfFire = 2;

//================================ bitmaps ===================================
//your starship
const int shipImgW = 14;
const int shipImgH = 16;
char shipImg[] = "ZZZZZZWWZZZZZZZZZZZYWWYZZZZZZZZZZWWWWZZZZZZZZZZWWWWZZZZZZZZZWWWWWWZZZZZZZZWWWWWWZZZZZYZZWWWWWWZZYZZYZZWWWWWWZZYZWWZZWWBBWWZZWWWWZZWBBBBWZZWWWWZWWBBBBWWZWWWWZWWWWWWWWZWWWWWWWWWWWWWWWWWRWWWWWWWWWWRWZZWWWWWWWWWWZZZZZWRRWWRRWZZZ";

//flames
const int flamesImgW = 12;
const int flamesImgH = 6;
char flamesImg[] = "RZZZZZZZZZZRRZRYYRRYYRZRRZRYYRRYYRZRZZRYRZZRYRZZZZZRZZZZRZZZZZZRZZZZRZZZ";

//alien
const int alienImgW = 14;
const int alienImgH = 11;
char alienImg[] = "GGGZZZZZZZZGGGZZZGZZZZZZGZZZZZGGGGGGGGGGZZZGGGGGGGGGGGGZGGGZGGGGGGZGGGGGGZZGGGGZZGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGZZZGGZZGGZZZGZGGZGGZZGGZGGZZZZZZGZZGZZZZZ";

//ship 'sploded
const int splodedImgW = 14;
const int splodedImgH = 16;
char splodedImg[] = "ZZZZZZWWZZZZZZZZZZRYWWYRZZZYZZZRRWWRRRRZRWYZRRRRRYYRRRZWYZRYRYYRYYRRRZWWRYYYRYYYYYRZWWRYYRYRYYYYRRWWRYYYRWRYBRRZRRRYRRWWWRYRWZZRYYRRBBWRYRWWZZRYYBBBRRYBWWRZZRYYYRRYYZZWZRRWRYYRBYRZZWZZRYBRYYYYYRRZZRWWYYYWWRRRZZZZWRRWWRRRWZZZ";

//=============================== setup and loop =============================
void setup() {
  memset(alienLive, true, numberOfalien);
  memset(aFireX, 0, 5);
  memset(aFireY, 0, 5);
  memset(aFireAge, 0, 5);
  
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(0x5E85);
  tft.setTextSize(3);

  randomSeed(analogRead(6));

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_1),right, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2),left, FALLING);
}

void loop() {
  //-------------Start Screen--------------
  if (millis() - offsetS >= 900 and !beginGame) {
    if (!startPrinted) {
      tft.setCursor(5, 120);
      tft.print(">START<");
      startPrinted = true;
      offsetS = millis();
    }
    else {
      tft.fillRect(5, 120, 200, 32, TFT_BLACK);
      startPrinted = false;
      offsetS = millis();
    }
  }
  if (beginGame and beginGame2) {
    tft.fillRect(5, 120, 200, 32, TFT_BLACK);
    beginGame2 = false;
    play = true;
  }
  //-------------Player--------------------
  if (millis() - offsetM >= shipSpeed and play) {
    moveShip();
    offsetM = millis();
  }
  if (oldShipX != shipX or oldShipY != shipY) {
    tft.fillRect(oldShipX, oldShipY, 28, 44, TFT_BLACK);
    oldShipX = shipX;
    oldShipY = shipY;
    drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
  }
  
  if (fire and play) {
    fireDaLazer();
  }
  if (millis() - offsetB >= 50) {
    for (int i = 0; i < 5; i++) {
      if (fFireAge[i] < 20 and fFireAge[i] > 0) {
        keepFirinDaLazer(i);
      }
      if (fFireAge[i] == 20) {
        stopFirinDaLazer(i);
      }
    }
    offsetB = millis();
  }
  
  if (millis() - offsetT > 50) {
    changeShipX = 0;
    changeShipY = 0;
  }

  //---------------Aliens-----------------------------------
  if (millis() - offsetA >= alienSpeed and play) {
    moveAliens();
    offsetA = millis();
  }
  if (findAlienX(5) >= 105) {
    changeAlienX = -3;
    changeAlienY = 3;
  }
  if (alienX <= 6) {
    changeAlienX = 3;
    changeAlienY = 3;
  }

  alienLiveCount = 0;
  for (int i = 0; i < numberOfalien; i++) {
    if (alienLive[i]) {
      alienLiveCount += 1;
      if (alienShot(i)) {
        tft.fillRect(findOldAlienX(i), findOldAlienY(i), 28, 22, TFT_BLACK);
        alienLiveCount -= 1;
        alienLive[i] = false;
        score += scoreInc;
      }
      if (onPlayer(i) or exceedBoundary(i)) {
        gameOver();
      }
    }
  }
  if (alienLiveCount == 1) {
    oldAlienSpeed = alienSpeed;
    if (alienSpeed > 50) {
      alienSpeed -= 10;
    }
    else {
      alienSpeed = 20;
    }
  }
  if (alienLiveCount == 0) {
    levelUp();
  }
}

//================================ functions ==================================================================
void gameOver() {
  play = false;
  if (doSplode) {
    drawBitmap(splodedImg, splodedImgW, splodedImgH, shipX, shipY, 2);
  }
  tft.fillScreen(TFT_BLACK);
  drawScore(false);
  delay(1000);
  tft.setCursor(0, 180);
  tft.setTextSize(2);
  tft.print("(Reset device to replay)");
  while (true) {}
}

void drawScore(boolean win) {
  tft.setCursor(0, 20);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  if (win) {
    tft.print("LEVEL UP!");
  }
  else {
    tft.print("GAME OVER");
  }
  for (;millis() - offsetM <= 1000;)
  tft.setCursor(0, 89);
  tft.setTextSize(2);
  tft.print("Score: ");
  tft.print(score);
  offsetM = millis();

  for (;millis() - offsetM <= 1000;) {
  }
  tft.setCursor(0, 128);
  tft.print("Level: ");
  tft.print(level);
}

void levelUp() {
  play = false;
  
  memset(alienLive, true, numberOfalien);
  memset(aFireX, 0, 5);
  memset(aFireY, 0, 5);
  memset(aFireAge, 0, 5);
  
  alienX = 7;
  alienY = 25;
  oldAlienX = 7;
  oldAlienY = 25;
  alienSpeed = oldAlienSpeed;
  if (alienSpeed > 100) {
    alienSpeed -= 10;
    chanceOfFire -= 10;
  }
  else if (alienSpeed > 50) {
    alienSpeed -= 10;
    chanceOfFire -=5;
  }
  else if (alienSpeed > 25) {
    alienSpeed -= 5;
    chanceOfFire -=1;
  }
  
  score += 50;
  scoreInc += 5;

  changeShipX = 0;
  changeShipY = 0;
  
  for (unsigned long i = millis(); millis() - i <= 1600;) {
    if (millis() - offsetM >= 20) {
      tft.fillRect(oldShipX, oldShipY, 28, 44, TFT_BLACK);
      drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
      drawBitmap(flamesImg, flamesImgW, flamesImgH, shipX + 1, shipY + 32, 2);
      oldShipX = shipX;
      oldShipY = shipY;
      shipY -= 6;
      offsetM = millis();
    }
  }

  drawScore(true);
  level += 1;
  shipX = 50;
  shipY = 200;
  for (; millis() - offsetM <= 4000;) {
  }
  tft.fillScreen(TFT_BLACK);
  offsetM = millis();
  play = true;
}


boolean alienShot(int num) {
  for (int i; i < 5; i++) {
    if (fFireAge[i] < 20 and fFireAge[i] > 0) {
      if (fFireX[i] > findAlienX(num) - 4 and fFireX[i] < findAlienX(num) + 28 and fFireY[i] < findAlienY(num) + 22 and fFireY[i] > findAlienY(num) + 4) {
        fFireAge[i] = 20;
        return true;
      }
    }
  }
  return false;
}

boolean onPlayer(int num) {
  if (findAlienX(num) - shipX < 24 and findAlienX(num) - shipX > -28 and findAlienY(num) - shipY < 32 and findAlienY(num) - shipY > -22) {
    doSplode = true;
    return true;
  }
  else {
    return false;
  }
}

boolean exceedBoundary(int num) {
  if (findAlienY(num) > 218) {
    return true;
  }
  else {
    return false;
  }
}

void moveAliens() {
  for (int i = 0; i < numberOfalien; i++) {
    if (alienLive[i]) {
      tft.fillRect(findOldAlienX(i), findOldAlienY(i), 28, 22, TFT_BLACK);
      drawBitmap(alienImg, alienImgW, alienImgH, findAlienX(i), findAlienY(i), 2);
    }
  }
  oldAlienX = alienX;
  oldAlienY = alienY;
  alienX += changeAlienX;
  alienY += changeAlienY;
  if (changeAlienY != 0) {
    changeAlienY = 0; 
  }
}

int findAlienX(int num) {
  return alienX + 42*(num % 3);
}

int findAlienY(int num) {
  return alienY + 33*(num / 3);
}

int findOldAlienX(int num) {
  return oldAlienX + 42*(num % 3);
}

int findOldAlienY(int num) {
  return oldAlienY + 33*(num / 3);
}

//---------------------------Player----------------------------------------
void fireDaLazer() {
  int bulletNo = -1;
  for (int i = 0; i < 4; i++) {
    if (fFireAge[i] == 0) {
      bulletNo = i;
    }
  }
  if (bulletNo != -1) {
    fFireAge[bulletNo] = 1;
    fFireX[bulletNo] = shipX + 13;
    fFireY[bulletNo] = shipY - 4;
    tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 3, TFT_MAGENTA);
  }
  //fire = false;
}

void keepFirinDaLazer(int bulletNo) {
  tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, TFT_BLACK);
  fFireY[bulletNo] -= 8;
  tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, TFT_MAGENTA);
  fFireAge[bulletNo] += 1;
}

void stopFirinDaLazer(int bulletNo) {
  tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, TFT_BLACK);
  fFireAge[bulletNo] = 0;
}

void moveShip() {
  if (shipX + changeShipX < 288 and shipX + changeShipX > 6 and changeShipX != 0) {
    shipX += changeShipX;
  }
  if (shipY + changeShipY > 24 and shipY + changeShipY < 192 and changeShipY != 0) {
    shipY += changeShipY;
  }
  if (oldShipX != shipX or oldShipY != shipY) {
    tft.fillRect(oldShipX, oldShipY, 28, 44, TFT_BLACK);
    oldShipX = shipX;
    oldShipY = shipY;
    drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
  }
}

void drawBitmap(char img[], int imgW, int imgH, int x, int y, int scale) {
  uint16_t cellColor;
  char curPix;
  for (int i = 0; i < imgW*imgH; i++) {
    curPix = img[i];
    if (curPix == 'W') {
      cellColor = TFT_WHITE;
    }
    else if (curPix == 'Y') {
      cellColor = TFT_YELLOW;
    }
    else if (curPix == 'B') {
      cellColor = TFT_BLUE;
    }
    else if (curPix == 'R') {
      cellColor = TFT_RED;
    }
    else if (curPix == 'G') {
      cellColor = 0x5E85;
    }
    if (curPix != 'Z' and scale == 1) {
      tft.drawPixel(x + i % imgW, y + i / imgW, cellColor);
    }
    else if (curPix != 'Z' and scale > 1) {
      tft.fillRect(x + scale*(i%imgW), y + scale*(i/imgW), scale, scale, cellColor);
    }
  }
}

//=========================== button functions ===================================
void left() {
  if (beginGame) {
    if (millis() - offsetT >= 50 and play) {
      changeShipX = -6;
      changeShipY = 0;
      offsetT = millis();
    }
  }
  else if (!beginGame)
  {
    beginGame = true;
  }
}

void right() {
  if (millis() - offsetT >= 50 and play) {
    changeShipX = 6;
    changeShipY = 0;
    offsetT = millis();
  }
}

void select() {
  if (millis() - offsetF >= 500 and play) {
    fire = true;
    offsetF = millis();
  }
  if (!beginGame) {
    beginGame = true;
  }
}
