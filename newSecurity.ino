#include <Time.h>
#include <TimeLib.h>
#include <stdint.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>

TouchScreen ts = TouchScreen(XP, YP, XM, YM); //init TouchScreen port pins
#define isWithin(x,a,b)((x>=a) && (x<=b))

char* doorName[30] = {"", "Front East", "Front Porch", "Garage"};
char* doorStatus[30] = {"Unlocked", "Locked"};
unsigned int screenH = 320;
unsigned int screenW = 240;
unsigned int blockMargin = 20;
unsigned int blockH = 280;
unsigned int blockW = 200;
unsigned int dStatusColors[2] = {RED, GREEN};
unsigned int hStatusColors[2] = {YELLOW, GREEN};
unsigned int dStatusTextColors[2] = {WHITE, BLACK};
unsigned int dStatusClockColors[2] = {WHITE, BLACK};
unsigned int digitPos[] = {0, 176, 156, 120, 100, 64, 44};
unsigned int heightStatus = 70;
unsigned int textLineSpacing = 22;
unsigned int doorState[] = {0, 1, 1, 1};
unsigned int doorXPos = 24;
unsigned int doorWidth = 192;
unsigned int doorYPos[] = {0, 24, 24 + heightStatus + 4, 24 + 2 * heightStatus + 8};
unsigned int doorT = 2;
unsigned int houseStatus = doorState[1] && doorState[2] && doorState[3];
unsigned int hours = 12;
unsigned int minutes = 30;
unsigned int seconds = 0;
unsigned int months = 0;
unsigned int days = 0;
unsigned int years = 0;
String hs;
String ms;
String ss;
unsigned int doorNameSize[] = {0, String(doorName[1]).length()*doorT * 6, String(doorName[2]).length()*doorT * 6, String(doorName[3]).length()*doorT * 6};
unsigned int doorStatusSize[] = {String(doorStatus[0]).length()*doorT * 6, String(doorStatus[1]).length()*doorT * 6};
unsigned int doorClockSize = String("00:00:00").length() * doorT * 6;
unsigned int hmsY = 250;
unsigned int doorX[] = {0, 60, 53, 74};
unsigned int doorY[] = {0, 30, 104, 178};
boolean initializeClock = true;
boolean runClock = true;
boolean setMyClock = false;
boolean pressed = false;
boolean secMsd = false;
boolean flagHstatusUpdate = false;
boolean lsd = false;
int temptime = 0;
int value;
int toggle;
int doorHours[] = {12, 12, 12, 12};
int doorMinutes[] = {0, 0, 0, 0};
int doorSeconds[] = {0, 0, 0, 0};
int active =  -1;
String timeString;
String dateString;
char test[50];

// Pin thing
int checkLine1 = 2;
int checkLine2 = 3;
int checkLine3 = 8;

int frontEast = 5;
int frontCenter = 5;
int garageDoor = 5;

// Block for touchpad
typedef struct {
  int left;
  int top;
  int width;
  int height;
} rectangle ;

// Rotations 0,2 = portrait  : 0->USB=right,upper : 2->USB=left,lower
// Rotations 1,3 = landscape : 1->USB=left,upper  : 3->USB=right,lower

// These don't need to be ints as they will never go beyond 255 or below 0
const byte rotation = 2; //(0->3)
const byte row = 3;
const byte col = 3;

//rectangle rect[col * row];

rectangle rect[col * row + 6];
// Strings waste memory, use char* instead also static is not needed.
char *btnTitle[35] = {"", "1", "2", "3", "4", "5", "6", "7", "8", "9", "HOUR", "MINUTE", "DAY", "MONTH", "YEAR"};
char *timeNames[] = {"HOUR", "MINUTE", "DAY", "MONTH", "", "YEAR"};
int aSetTime[6]; //hr, min, sec, day, mnth, yr



// Block for touchpad


void setup()
{


  Tft.TFTinit();  //init TFT library
  Serial.begin(115200);
  mainScreenSetup();

  pinMode(checkLine1, INPUT);
  pinMode(checkLine2, INPUT);
  pinMode(checkLine3, INPUT);

  setTime(06, 04, 00, 21, 02, 2019);

}

void loop()
{
  time_t t = now(); // store the current time in time variable t
  hour(t);          // returns the hour for the given time t
  minute(t);        // returns the minute for the given time t
  second(t);        // returns the second for the given time t
  day(t);           // the day for the given time t
  weekday(t);       // day of the week for the given time t
  month(t);         // the month for the given time t
  year(t);          // the year for the given time t

  if (!frontEast == digitalRead(checkLine1) && runClock) {
    frontEast = digitalRead(checkLine1);
    doorState[1] = frontEast;
    doorHours[1] = hour();
    doorMinutes[1] = minute();
    doorSeconds[1] = second();
    flagHstatusUpdate = true;
    displayDoorStatus(1);
  }
  if (!frontCenter == digitalRead(checkLine2) && runClock) {
    frontCenter = digitalRead(checkLine2);
    doorState[2] = frontCenter;
    doorHours[2] = hour();
    doorMinutes[2] = minute();
    doorSeconds[2] = second();
    flagHstatusUpdate = true;
    displayDoorStatus(2);
  }
  if (!garageDoor == digitalRead(checkLine3) && runClock) {
    garageDoor = digitalRead(checkLine3);
    doorState[3] = garageDoor;
    doorHours[3] = hour();
    doorMinutes[3] = minute();
    doorSeconds[3] = second();
    flagHstatusUpdate = true;
    displayDoorStatus(3);
  }

  if (flagHstatusUpdate && runClock) {
    houseStatus = doorState[1] && doorState[2] && doorState[3];
    setHouseState(houseStatus);
  }

    //Serial.print(" [1] " + String(digitalRead(checkLine1)));
    //Serial.print(" [2] " + String(digitalRead(checkLine2)));
    //Serial.print(" [3] " + String(digitalRead(checkLine3)) + "\n");

  Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

  if (runClock) {

    if (initializeClock) {
      secMsd = true;
      //initializeClockFunction();
      initializeClock = false;
    }

    updateHmsDigits();
    updateDmyDigits();

    if (p.z > __PRESURE) {
      runClock = false;
      setMyClock = true;
      timeSetup();
      p.y = 360;
      initializeClock = true;
    }

    delay(100);
  }

  if (setMyClock) {
    delay(100);

    if (initializeClock) {
      showGrid(t);
      initializeClock = false;
    }

    // Get input
    while (pressed = false) {
      Point p = ts.getPoint();
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);
      if (p.z > __PRESURE) pressed = true;
    }
    int x;
    int y;

    if (rotation == 0)
    {
      x = 240 - p.x;
      y = 320 - p.y;
    }
    else if (rotation == 1)
    {
      //  p.y reversed
      x = 320 - p.y;
      y = p.x;
    }
    else if (rotation == 2)
    {
      x = p.x;
      y = p.y;
    }
    else if (rotation == 3)
    {
      //  p.x, p.y reversed
      x = p.y;
      y = 240 - p.x;
    }

    for (byte count = 0; count < (col * row) + 6; count++)
    {
      if ( isWithin(x, rect[count].left, (rect[count].left + rect[count].width)) && isWithin(y, rect[count].top, (rect[count].top + rect[count].height)) )
      {
        showTouchData( x, y, count, t );
      }
    }

    if (p.y < 50) {
      runClock = true;
      setMyClock = false;
      mainScreenSetup();
      initializeClock = true;
      displayDoorStatus(1);
      displayDoorStatus(2);
      displayDoorStatus(3);
      pressed = false;
      seconds = 00;
      secMsd = true;
      processUpdate();
    }
    delay(300); //End of Set block
  }
}

int showTouchData(int xCoord, int yCoord, int id, time_t t) {
  if (!lsd)Serial.print("lsd false \n");

  if (aSetTime[0] == 0)aSetTime[0] = hour(t);
  if (aSetTime[1] == 0)aSetTime[1] = minute(t);
  if (aSetTime[2] == 0)aSetTime[2] = second(t);
  if (aSetTime[3] == 0)aSetTime[3] = day(t);
  if (aSetTime[4] == 0)aSetTime[4] = month(t);
  if (aSetTime[5] == 0)aSetTime[5] = year(t);

  if (id > 9) {
    lsd = false;
    if (active == id) {
      Tft.fillRectangle(rect[active].left + 3, rect[active].top + 3, rect[active].width - 6, rect[active].height - 10, BLUE);
      if (active == 14) {
        changeDigit(rect[14].left + 2, rect[14].top + 8, 2, 2, BLUE, WHITE);
        changeDigit(rect[14].left + 2 + 3 * 6, rect[14].top + 8, 2, 0, BLUE, WHITE);
      }
      active = -1;
    } else if (active < 0) {
      active = id;
      Tft.fillRectangle(rect[active].left + 3, rect[active].top + 3, rect[active].width - 6, rect[active].height - 10, RED);
      if (active == 14) {
        changeDigit(rect[active].left + 2, rect[active].top + 8, 2, 2, RED, WHITE);
        changeDigit(rect[active].left + 2 + 3 * 6, rect[active].top + 8, 2, 0, RED, WHITE);
      }
    }
  } else {
    if (!lsd && active > 0) {
      value = (id) * 10;
      if (active == 14) {
        changeDigit(rect[14].left + 2 + 6 * 6, rect[14].top + 8, 2, value / 10, RED, WHITE);
      } else {
        changeDigit(rect[active].left + 24, rect[active].top + 8, 3, value / 10, RED, WHITE);
      }
      lsd = true;
    } else if(active > 0){
      boolean bGood = checkEntry(active, value + id);
      if (bGood) {
        value += id;
        Tft.fillRectangle(rect[active].left + 3, rect[active].top + 3, rect[active].width - 6, rect[active].height - 10, BLUE);
        if (active == 14) {
          changeDigit(rect[active].left + 2, rect[active].top + 8, 2, 2, BLUE, WHITE);
          changeDigit(rect[active].left + 2 + 3 * 6, rect[active].top + 8, 2, 0, BLUE, WHITE);
          changeDigit(rect[active].left + 2 + 6 * 6, rect[active].top + 8, 2, value / 10, BLUE, WHITE);
          changeDigit(rect[active].left + 2 + 9 * 6, rect[active].top + 8, 2, id, BLUE, WHITE);
        } else {
          changeDigit(rect[active].left + 6, rect[active].top + 8, 3, value / 10, BLUE, WHITE);
          changeDigit(rect[active].left + 24, rect[active].top + 8, 3, id, BLUE, WHITE);
        }
        if (active == 12) {
          aSetTime[3] = value;
        } else if (active == 13) {
          aSetTime[4] = value;
        } else {
          aSetTime[active - 10] = value;
        }
        aSetTime[2] = 0;
        active = -1;
        lsd = false;
      } else {
        for (int i = 0; i < 5; i++) {
          Tft.fillRectangle(rect[active].left + 3, rect[active].top + 3, rect[active].width - 6, rect[active].height - 10, BLUE);
          delay(50);
          Tft.fillRectangle(rect[active].left + 3, rect[active].top + 3, rect[active].width - 6, rect[active].height - 10, RED);
          delay(50);
          value = 0;
          lsd = false;
        }
      }
    }
  }
}

boolean checkEntry(int idx, int valuex) {
  boolean testx = true;
  switch (idx) {
    case 10:
      if (valuex > 23) {
        testx = false;
      }
      break;
    case 11:
      if (valuex > 59) {
        testx = false;
      }
      break;
    case 12:
      if (valuex > 31) {
        testx = false;
      }
      break;
    case 13:
      if (valuex > 12) {
        testx = false;
      }
      break;
    default:

      break;
  }
  return  testx;
}

void processUpdate() {
  setTime(aSetTime[0], aSetTime[1], aSetTime[2], aSetTime[3], aSetTime[4], aSetTime[5]);
}

void showGrid(time_t t)
{
  // some of these don't need to be ints
  int left, top;
  int l = 10;
  int tp = 180;
  int w = 70;
  int h = 40;
  int hgap = 10;
  int vgap = 10;
  int id = 1;
  int tempid;
  boolean runOnce = true;

  for (byte j = 0; j < row; j++)
  {
    for (byte i = 0; i < col; i++)
    {
      left = l + i * (w + vgap);
      top = tp + j * (h + hgap);
      rect[id].left = left;
      rect[id].top = top;
      rect[id].width = w;
      rect[id].height = h;
      Tft.drawRectangle( left, top, w, h, WHITE);
      Tft.drawString(btnTitle[id], left + 5, top + 5, 2, WHITE);
      id++;
    }
  }

  for (byte j = 0; j < row - 1; j++)
  {
    for (byte i = 0; i < col; i++)
    {
      left = l + i * (w + vgap);
      top = 68 + j * (h + 25);
      tempid = id;
      if (id == 14 && runOnce) {
        tempid--;
        id = 0;
        runOnce = false;
      }
      rect[id].left = left;
      rect[id].top = top;
      rect[id].width = w;
      rect[id].height = h;
      Tft.drawRectangle( left, top, w, h, WHITE);
      Tft.drawString(btnTitle[id], left + 5, top - vgap, 1, WHITE);
      if (j == 1 && i == 1) {
        Tft.drawString("0", left + 5, top + 5, 2, WHITE);
      }
      id = tempid;
      id++;
    }
  }



  value = hour(t) / 10;
  changeDigit(rect[10].left + 17, rect[10].top + 8, 3, value, BLUE, WHITE);
  value = hour(t) % 10;
  changeDigit(rect[10].left + 17 + 3 * 6, rect[10].top + 8, 3, value, BLUE, WHITE);
  value = minute(t) / 10;
  changeDigit(rect[11].left + 17, rect[11].top + 8, 3, value, BLUE, WHITE);
  value = minute(t) % 10;
  changeDigit(rect[11].left + 17 + 3 * 6, rect[11].top + 8, 3, value, BLUE, WHITE);
  value = day(t) / 10;
  changeDigit(rect[12].left + 17, rect[12].top + 8, 3, value, BLUE, WHITE);
  value = day(t) % 10;
  changeDigit(rect[12].left + 17 + 3 * 6, rect[12].top + 8, 3, value, BLUE, WHITE);

  value = month(t) / 10;
  changeDigit(rect[13].left + 17, rect[13].top + 8, 3, value, BLUE, WHITE);
  value = month(t) % 10;
  changeDigit(rect[13].left + 17 + 3 * 6, rect[13].top + 8, 3, value, BLUE, WHITE);
  value = year(t) / 1000;
  changeDigit(rect[14].left + 2, rect[14].top + 8, 2, value, BLUE, WHITE);
  value = year(t) / 100 % 10;
  changeDigit(rect[14].left + 2 + 3 * 6, rect[14].top + 8, 2, value, BLUE, WHITE);
  value = year(t) / 10 % 10;
  changeDigit(rect[14].left + 2 + 6 * 6, rect[14].top + 8, 2, value, BLUE, WHITE);
  value = year(t) % 10;
  changeDigit(rect[14].left + 2 + 9 * 6, rect[14].top + 8, 2, value, BLUE, WHITE);

}

/****************************************************/
/* Function to Print a single number on the screen  */
/* x: Horizontal Position on screen                 */
/* y: Vertical Position on screen                   */
/* value: Single Digit Integer                      */
/****************************************************/
void changeDigit(int x, int y, int iSize, int value, int colorb, int colorf) {
  Tft.fillRectangle(x, y, 6 * iSize, 8 * iSize, colorb); // Erase old digit
  Tft.drawNumber(value, x, y,  iSize, colorf);  // Paint new digit
}

void displayDoorStatus(int door) {
  Tft.fillRectangle(doorXPos, doorYPos[door], doorWidth, heightStatus, dStatusColors[doorState[door]]);  //Paint Door 1 block
  Tft.drawString(doorName[door], screenW / 2 - doorNameSize[door] / 2, doorY[door], 2, dStatusTextColors[doorState[door]]); //Print Door1 Name
  Tft.drawString(doorStatus[doorState[door]], screenW / 2 - doorStatusSize[doorState[door]] / 2, doorY[door] + textLineSpacing, 2, dStatusTextColors[doorState[door]]); //Print Door1 Status
  Tft.drawString("  :  :", screenW / 2 - doorClockSize / 2, doorY[door] + 2 * textLineSpacing, 2, dStatusTextColors[doorState[door]]); // Print Action Time
  changeDigit(70, doorY[door] + 2 * textLineSpacing, 2, doorHours[door] / 10, dStatusColors[doorState[door]], dStatusTextColors[doorState[door]]);
  changeDigit(85, doorY[door] + 2 * textLineSpacing, 2, doorHours[door] % 10, dStatusColors[doorState[door]], dStatusTextColors[doorState[door]]);
  changeDigit(107, doorY[door] + 2 * textLineSpacing, 2, doorMinutes[door] / 10, dStatusColors[doorState[door]], dStatusTextColors[doorState[door]]);
  changeDigit(122, doorY[door] + 2 * textLineSpacing, 2, doorMinutes[door] % 10, dStatusColors[doorState[door]], dStatusTextColors[doorState[door]]);
  changeDigit(140, doorY[door] + 2 * textLineSpacing, 2, doorSeconds[door] / 10, dStatusColors[doorState[door]], dStatusTextColors[doorState[door]]);
  changeDigit(155, doorY[door] + 2 * textLineSpacing, 2, doorSeconds[door] % 10, dStatusColors[doorState[door]], dStatusTextColors[doorState[door]]);
}

void setHouseState(int state) {
  int j = 0;
  int k = 0;
  for (int i = 0; i < 20; i++) {
    Tft.drawRectangle(j + i, k + i, screenW - i * 2, screenH - i * 2, hStatusColors[state]);
  }
}

void initializeClockFunction() {
  value = second() % 10;
  changeDigit(digitPos[1], 260, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
  value = second() / 10;
  changeDigit(digitPos[2], 260, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
  value = minute() % 10;
  changeDigit(digitPos[3], 260, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
  value = minute() / 10;
  changeDigit(digitPos[4], 260, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
  value = hour() % 10;
  changeDigit(digitPos[5], 260, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
  value = hour() / 10;
  changeDigit(digitPos[6], 260, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
}

void updateHmsDigits() {

  if (seconds != second() || secMsd) {
    if (second() % 10 == 0 || secMsd) {
      value = second() / 10;
      changeDigit(digitPos[2], hmsY, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
    }
    value = second() % 10;
    changeDigit(digitPos[1], hmsY, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
    seconds = second();
  }

  // Display minutes digits
  if (minutes != minute() || secMsd) {
    if (minute() % 10 == 0 || secMsd) {
      value = minute() / 10;
      changeDigit(digitPos[4], hmsY, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
    }
    value = minute() % 10;
    changeDigit(digitPos[3], hmsY, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
    minutes = minute();
  }

  // Display hours digits
  if (hours != hour() || secMsd) {
    if (hour() % 10 == 0 || secMsd) {
      value = hour() / 10;
      changeDigit(digitPos[6], hmsY, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
    }
    value = hour() % 10;
    changeDigit(digitPos[5], hmsY, 3, value, dStatusClockColors[1], dStatusClockColors[0]);
    hours = hour();
  }
}

void updateDmyDigits() {
  // Display months digits
  if (months != month() || secMsd) {
    if (month() % 10 == 0 || secMsd) {
      value = month() / 10;
      changeDigit(digitPos[6] - 12, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    }
    value = month() % 10;
    changeDigit(digitPos[5] - 12, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    months = month();
  }
  // Display days digits
  if (days != day() || secMsd) {
    if (day() % 10 == 0 || secMsd) {
      value = day() / 10;
      changeDigit(digitPos[4] - 12, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    }
    value = day() % 10;
    changeDigit(digitPos[3] - 12, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    days = day();
  }
  // Display years digits
  if (years != year() || secMsd) {
    value = year() / 1000;
    changeDigit(digitPos[2] - 12, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    value = (year() / 100) % 10;
    changeDigit(digitPos[1] - 12, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    value = (year() / 10) % 10;
    changeDigit(digitPos[1] + 6, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    value = year() % 10;
    changeDigit(digitPos[1] + 20, hmsY + 32, 2, value, dStatusClockColors[1], dStatusClockColors[0]);
    years = year();
  }
  secMsd = false;
}

void mainScreenSetup() {
  Tft.fillScreen(0, screenW, 0, screenH, hStatusColors[houseStatus]);  //Paint screen green or yellow
  Tft.fillScreen(blockMargin, blockW + blockMargin, blockMargin, blockH + blockMargin, BLACK); //Paint a smaller black rectangle

  Tft.fillRectangle(doorXPos, doorYPos[1], doorWidth, heightStatus, dStatusColors[doorState[1]]);  //Paint Door 1 block
  Tft.fillRectangle(doorXPos, doorYPos[2], doorWidth, heightStatus, dStatusColors[doorState[2]]);  //Paint Door 2 block
  Tft.fillRectangle(doorXPos, doorYPos[3], doorWidth, heightStatus, dStatusColors[doorState[3]]);  //Paint Door 3 block

  Tft.drawString(doorName[1], screenW / 2 - doorNameSize[1] / 2, doorY[1], 2, dStatusTextColors[doorState[1]]); //Print Door1 Name
  Tft.drawString(doorStatus[doorState[1]], screenW / 2 - doorStatusSize[doorState[1]] / 2, doorY[1] + textLineSpacing, 2, dStatusTextColors[doorState[1]]); //Print Door1 Status
  Tft.drawString("00:00:00",  screenW / 2 - doorClockSize / 2, doorY[1] + 2 * textLineSpacing, 2, dStatusTextColors[doorState[1]]); // Print Action Time

  Tft.drawString(doorName[2], screenW / 2 - doorNameSize[2] / 2, doorY[2], 2, dStatusTextColors[doorState[2]]); //Print Door2 Name
  Tft.drawString(doorStatus[doorState[2]], screenW / 2 - doorStatusSize[doorState[2]] / 2, doorY[2] + textLineSpacing, 2, dStatusTextColors[doorState[2]]); //Print Door2 Status
  Tft.drawString("00:00:00",   screenW / 2 - doorClockSize / 2, doorY[2] + 2 * textLineSpacing, 2, dStatusTextColors[doorState[2]]); // Print Action Time

  Tft.drawString(doorName[3], screenW / 2 - doorNameSize[3] / 2, doorY[3], 2, dStatusTextColors[doorState[3]]); //Print Door3 Name
  Tft.drawString(doorStatus[doorState[3]], screenW / 2 - doorStatusSize[doorState[3]] / 2, doorY[3] + textLineSpacing, 2, dStatusTextColors[doorState[3]]); //Print Door3 Status
  Tft.drawString("00:00:00",   screenW / 2 - doorClockSize / 2, doorY[3] + 2 * textLineSpacing, 2, dStatusTextColors[doorState[3]]); // Print Action Time
}

void timeSetup() {
  Tft.fillScreen(0, screenW, 0, screenH, BLUE);  //Paint screen green or yellow
  Tft.fillRectangle(0, 0, screenW, 50, GREEN);  //Paint Enter Button
  Tft.drawString("ENTER",   screenW / 2 - (5 * 24) / 2, 10, 4, BLACK); //Print ENTER

}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
