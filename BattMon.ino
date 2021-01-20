/*
   Author: Timo Birnschein (timo.birnschein@microforge.de)
   Date: 2021/01/19
   License: Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License

   Overview:
   5 channel battery monitor with calibration (magic numbes in the code, marked with comment).
   Displays all data on 2.4" display using my all time favorite library:
   https://github.com/Bodmer/TFT_ST7735

   Note: On the board, there is also an NRF wireless module but I'm not using it yet.
   It's also questionable if the RF24Network library fits into the remaining 2KB of flash.
   The tft library is darn large. However, I haven't tried cranking up the code optimization level.

*/

#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library
#include <TFT_ST7735.h> // Hardware-specific library
#include <SPI.h>

#define TFT_CS   10
#define TFT_RST  9
#define TFT_DC   8

#define TFT_SCLK 13
#define TFT_MOSI 11

// *************************************************************************************************** //
// ********************************* Battery Voltage Measurement Variables *************************** //
// *************************************************************************************************** //

#define battery1_Pin A0
#define battery2_Pin A1
#define battery3_Pin A2
#define battery4_Pin A3
#define battery5_Pin A4

#define VIN_MAX 18.0f
#define ADC_RANGE 3.3f
#define BIT_DEPTH 1024.0f
#define V_SEPARATION ADC_RANGE / BIT_DEPTH

#define R1_1 22000.0f
#define R2_1 4700.0f
#define RESISTOR_CONSTANT_1 (R2_1/(R1_1 + R2_1))
#define R1_2 22000.0f
#define R2_2 4700.0f
#define RESISTOR_CONSTANT_2 (R2_2/(R1_2 + R2_2))
#define R1_3 22000.0f
#define R2_3 4700.0f
#define RESISTOR_CONSTANT_3 (R2_3/(R1_3 + R2_3))
#define R1_4 22000.0f
#define R2_4 4700.0f
#define RESISTOR_CONSTANT_4 (R2_4/(R1_4 + R2_4))
#define R1_5 22000.0f
#define R2_5 4700.0f
#define RESISTOR_CONSTANT_5 (R2_5/(R1_5 + R2_5))

#define VBATT_GOOD 13.0f
#define VBATT_OK 12.5f
#define VBATT_WEAK 12.0f
#define VBATT_BAD 11.5f

#define GRAPHDURATION 5400 // 24h
//#define GRAPHDURATION 2700 // 12h
//#define GRAPHDURATION 1350 // 6h
//#define GRAPHDURATION 675 // 3h
//#define GRAPHDURATION 225 // 1h

float vBatt_1_House = 0.0f;
float vBatt_2_House = 0.0f;
float vBatt_3_Port = 0.0f;
float vBatt_4_Stb = 0.0f;
float vBatt_5_GenSet = 0.0f;

uint16_t displayUpdateTimer = 0;

/*
 * Number of lines stored for the console. Console is not scrollable, it will only auto-scroll down when lines are added
 */
#define NUM_CONSOLE_LINES 18 // How many lines are in the console

/*
 * Define the console storage for 19 lines of text
 */
String consoleBuffer[NUM_CONSOLE_LINES + 1];
int consoleBufferColor[NUM_CONSOLE_LINES + 1];
uint8_t consoleHead = 0;

/*
 * battery graphs with one head
 */
uint8_t graphBufferHead = 0;
#define GRAPHRANGE 3.0f
#define GRAPHOFFSET 11.5f

// *************************************************************************************************** //
// ********************************* Display Variables *********************************************** //
// *************************************************************************************************** //
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
TFT_ST7735 tft = TFT_ST7735();       // Invoke custom library - insanely fast alternative! https://github.com/Bodmer/TFT_ST7735

void setup(void) {
  //Serial.begin(9600);
  analogReference(EXTERNAL);

  tone(3, 440, 100);
  delay(100);
  tone(3, 600, 100);
  delay(100);
  tone(3, 800, 100);
  delay(100);
  tone(3, 1000, 100);
  delay(100);

  displayUpdateTimer = millis();
  
  displayInitScreen();
}
 
void loop(void) {

/*
 * We want to update the screen about ten times a second
 */
  if ((millis() % 16384) - displayUpdateTimer > 100)
  {
    vBatt_1_House = ((analogRead(battery1_Pin) * V_SEPARATION) / RESISTOR_CONSTANT_1) * 0.957540; // Magic Number is the calibration value which should be calculated using a define at the top
    vBatt_2_House = ((analogRead(battery2_Pin) * V_SEPARATION) / RESISTOR_CONSTANT_2) * 0.949164;
    vBatt_3_Port = ((analogRead(battery3_Pin) * V_SEPARATION) / RESISTOR_CONSTANT_3) * 0.952662;
    vBatt_4_Stb = ((analogRead(battery4_Pin) * V_SEPARATION) / RESISTOR_CONSTANT_4) * 0.947909;
    vBatt_5_GenSet = ((analogRead(battery5_Pin) * V_SEPARATION) / RESISTOR_CONSTANT_5) * 0.951923;
      
    String strBuffer = String(vBatt_1_House);
    display_PrintTextAdv(strBuffer, 50, 10, 1, 1, (vBatt_1_House > 12 ? ST7735_WHITE : ST7735_WHITE), (vBatt_1_House > 12 ? ST7735_BLACK : ST7735_RED));
    strBuffer = String(vBatt_2_House);
    display_PrintTextAdv(strBuffer, 50, 18, 1, 1, (vBatt_2_House > 12 ? ST7735_LIGHTGREY : ST7735_WHITE), (vBatt_2_House > 12 ? ST7735_BLACK : ST7735_RED));
    strBuffer = String(vBatt_3_Port);
    display_PrintTextAdv(strBuffer, 50, 27, 1, 1, (vBatt_3_Port > 12 ? ST7735_BLUE : ST7735_WHITE), (vBatt_3_Port > 12 ? ST7735_BLACK : ST7735_RED));
    strBuffer = String(vBatt_4_Stb);
    display_PrintTextAdv(strBuffer, 50, 36, 1, 1, (vBatt_4_Stb > 12 ? ST7735_GREEN : ST7735_WHITE), (vBatt_4_Stb > 12 ? ST7735_BLACK : ST7735_RED));
    strBuffer = String(vBatt_5_GenSet);
    display_PrintTextAdv(strBuffer, 50, 45, 1, 1, (vBatt_5_GenSet > 12 ? ST7735_YELLOW : ST7735_WHITE), (vBatt_5_GenSet > 12 ? ST7735_BLACK : ST7735_RED));
      
    addValue2Graph(vBatt_1_House, vBatt_2_House, vBatt_3_Port, vBatt_4_Stb, vBatt_5_GenSet);
    drawGraphBox();
    display_UpdateBatteryStatusBar( (vBatt_1_House > 12 ? true : false) && 
                                    (vBatt_2_House > 12 ? true : false) && 
                                    (vBatt_3_Port > 12 ? true : false) && 
                                    (vBatt_4_Stb > 12 ? true : false) && 
                                    (vBatt_5_GenSet > 12 ? true : false));
    
    displayUpdateTimer = millis() % 16384;
  }
}

void displayInitScreen(void)
{
  //tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);

  display_DrawTopStatusBar();
 
  String strBuffer = String("House 1");
  display_PrintText(strBuffer, 0, 10, 0, ST7735_WHITE);
  
  strBuffer = String("House 2");
  display_PrintText(strBuffer, 0, 18, 0, ST7735_LIGHTGREY);
  
  strBuffer = String("Port  E");
  display_PrintText(strBuffer, 0, 27, 0, ST7735_BLUE);
  
  strBuffer = String("Starb E");
  display_PrintText(strBuffer, 0, 36, 0, ST7735_GREEN);
  
  strBuffer = String("GenSet");
  display_PrintText(strBuffer, 0, 45, 0, ST7735_YELLOW);

  drawGraphBox();
}

void drawGraphBox(void)
{
  String maxGraphVoltage = String((uint8_t)GRAPHRANGE + (uint8_t)GRAPHOFFSET) + String(" Volts");
  display_PrintTextAdv(maxGraphVoltage, 0, 60, 1, 1, ST7735_WHITE, ST7735_BLACK);
  String minGraphVoltage = String((uint8_t)GRAPHOFFSET) + String(" Volts");
  display_PrintTextAdv(minGraphVoltage, 0, 109, 1, 1, ST7735_WHITE, ST7735_BLACK);
}

// Draws the bar at the top showing connection status and current time / date
void display_DrawTopStatusBar()
{
  tft.fillRect(0, 0, 159, 9, ST7735_BLUE);
  display_PrintText("MicroForge BatMon v0.2", 6*2, 1, 0, ST7735_WHITE);
  
  tft.fillRect(0, 118, 159, 127, ST7735_RED);
  display_PrintText("Batteries BAD", 6*7, 119, 0, ST7735_WHITE);

  tft.drawFastHLine(0, 56, 160, ST7735_WHITE);
}

// Draws the bar at the top showing connection status and current time / date
void display_UpdateBatteryStatusBar(bool batteryGood)
{  
  static bool lastBatteryState = false; // initially must be false to allow for the initial state transition to work.
  
  if (!batteryGood)
  {
    tft.fillRect(0, 118, 159, 127, ST7735_RED);
    display_PrintText("Batteries BAD", 6*7, 119, 0, ST7735_WHITE);
    tone(3, 1000, 200);
    tone(3, 500, 200);
  }
  else if (batteryGood == true && lastBatteryState == false)
  {
    tft.fillRect(0, 118, 159, 127, ST7735_GREEN);
    display_PrintText("Batteries GOOD", 6*6, 119, 0, ST7735_BLACK);
  }
  
  lastBatteryState = batteryGood;
}

void display_PrintText(String textBuffer, int x, int y, int textSize, int color)
{
  if (textSize == 0) textSize = 1;
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.setCursor(x, y);
  tft.print(textBuffer.c_str());
}

void display_PrintTextAdv(String textBuffer, int x, int y, int textSize, int font, int forgroundColor, int backgroundColor)
{
  if (textSize == 0) textSize = 1;
  tft.setTextColor(forgroundColor, backgroundColor);
  tft.setTextSize(textSize);
  tft.drawString(textBuffer.c_str(), x, y, font);
}

void addValue2Graph(float house1, float house2, float port, float stb, float genset)
{
  static uint16_t graphProgress = 0;
  uint8_t head = graphBufferHead;
  head %= 160;

  tft.drawFastVLine((head + 2) % 160, 118-60, 60, TFT_RED); // clearing the path forward
  tft.drawFastVLine((head + 1) % 160, 118-60, 60, TFT_BLACK); // clearing the path forward
  // Draw a graph
  float yPos = yPos = (60.0f / GRAPHRANGE) * (house1 - GRAPHOFFSET);
  tft.drawPixel(head, 119 - constrain(yPos, 0, 60), ST7735_WHITE);
  yPos = yPos = (60.0f / GRAPHRANGE) * (house2 - GRAPHOFFSET);
  tft.drawPixel(head, 119 - constrain(yPos, 0, 60), ST7735_LIGHTGREY);
  yPos = yPos = (60.0f / GRAPHRANGE) * (port - GRAPHOFFSET);
  tft.drawPixel(head, 119 - constrain(yPos, 0, 60), ST7735_BLUE);
  yPos = yPos = (60.0f / GRAPHRANGE) * (stb - GRAPHOFFSET);
  tft.drawPixel(head, 119 - constrain(yPos, 0, 60), ST7735_GREEN);
  yPos = yPos = (60.0f / GRAPHRANGE) * (genset - GRAPHOFFSET);
  tft.drawPixel(head, 119 - constrain(yPos, 0, 60), ST7735_YELLOW);
  graphProgress++;

  // Graph should run 24h per full width of the 160 pixel display
  if (graphProgress >= GRAPHDURATION)
  {
    head++;
    graphProgress = 0;
  }
  graphBufferHead = head;
}

/*
 * Draws a linux style console onto the screen whenever a new line is added to the buffer.
 */
void addLine2Console(String line, int color)
{
  uint8_t head = consoleHead;
  
  // This is clearly a hack to clear the memory. I'm not sure what happens here, to be honest.
  // I thought String() has a copy constructor but it doesn't appear to work.
  consoleBufferColor[consoleHead] = color;
  consoleBuffer[consoleHead++] = String(line) + String("                ");
  
  consoleHead %= NUM_CONSOLE_LINES;

  // the for-loop draws the entire console bottom to top every time a new line is added.
  // If there are less lines than a full screen, the console will scroll through bottom to top as well.
  // This is surprisingly fast and extremely useful for bootup sequences and debugging in general.
  for (uint8_t i = 0; i < NUM_CONSOLE_LINES; i++)
  {
    display_PrintTextAdv(consoleBuffer[head], 0, 152 - 8 * i, 1, 1, consoleBufferColor[head], ST7735_BLACK);
    head--;
    if (head == 255) // if the 8-bit value flips below 0 to 255...
      head = NUM_CONSOLE_LINES - 1; // set it back to 18 - 1
  }
}

void addLine2Console(String line)
{
  addLine2Console(line, ST7735_WHITE);
}

