#ifndef display_h
#define display_h

#include <Arduino.h>
#include <SPI.h>
SPIClass hspi(HSPI);

//some functions were replaced/copied from epd7in3f.cpp from waveshare's demo code

#define EPD_7IN3F_BLACK 0x0   /// 000
#define EPD_7IN3F_WHITE 0x1   ///	001
#define EPD_7IN3F_GREEN 0x2   ///	010
#define EPD_7IN3F_BLUE 0x3    ///	011
#define EPD_7IN3F_RED 0x4     ///	100
#define EPD_7IN3F_YELLOW 0x5  ///	101
#define EPD_7IN3F_ORANGE 0x6  ///	110
#define EPD_7IN3F_CLEAN 0x7   ///	111   unavailable  Afterimage

// SPI pins. Adapt to your wiring
#define PIN_SPI_SCK 12
#define PIN_SPI_DIN 11
#define PIN_SPI_CS 15
#define PIN_SPI_BUSY 14
#define PIN_SPI_RST 17
#define PIN_SPI_DC 16

// Wakes up the display from sleep.
void resetDisplay() {
  digitalWrite(PIN_SPI_RST, HIGH);
  delay(20);
  digitalWrite(PIN_SPI_RST, LOW);  //module reset
  delay(1);
  digitalWrite(PIN_SPI_RST, HIGH);
  delay(20);
}

// Sends one byte via SPI.
void sendSpi(byte data) {
  digitalWrite(PIN_SPI_CS, LOW);
  hspi.transfer(data);
  digitalWrite(PIN_SPI_CS, HIGH);
}

// Sends one byte as a command.
void sendCommand(byte command) {
  digitalWrite(PIN_SPI_DC, LOW);
  hspi.transfer(command);
}

// Sends one byte as data.
void sendData(byte data) {
  digitalWrite(PIN_SPI_DC, HIGH);
  hspi.transfer(data);
}

// Waits until the display is ready.
void waitForIdle() {
  while (digitalRead(PIN_SPI_BUSY) == LOW /* busy */) {
    Serial.print(".");
    delay(100);
  }
}

// Returns whether the display is busy
bool isDisplayBusy() {
  return (digitalRead(PIN_SPI_BUSY) == LOW);
}

void EPD_7IN3F_BusyHigh()  // If BUSYN=0 then waiting
{
  while (!digitalRead(PIN_SPI_BUSY)) {
    //yield();
    delay(1);
  }
}

void TurnOnDisplay() {  //runs on another core to avoid watchdog timer error
  Serial.println("TurnOnDisplay called");
  Serial.println("power on");
  sendCommand(0x04);  // POWER_ON
  EPD_7IN3F_BusyHigh();
  //yield();
  Serial.println("refresh");
  sendCommand(0x12);  // DISPLAY_REFRESH
  sendData(0x00);
  EPD_7IN3F_BusyHigh();
  //yield();
  Serial.println("power off");
  sendCommand(0x02);  // POWER_OFF
  sendData(0x00);
  EPD_7IN3F_BusyHigh();
  //yield();
}

int IfInit(void) {
  // Initialize SPI.
  pinMode(PIN_SPI_CS, OUTPUT);
  pinMode(PIN_SPI_RST, OUTPUT);
  pinMode(PIN_SPI_DC, OUTPUT);
  pinMode(PIN_SPI_BUSY, INPUT);
  hspi.begin(PIN_SPI_SCK, -1, PIN_SPI_DIN);
  hspi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

  return 0;
}

// Initializes the display.
int initDisplay(void) {
  Serial.println("Initializing display");
  if (IfInit() != 0) {
    return -1;
  }

  // Initialize the display.
  resetDisplay();
  delay(20);
  EPD_7IN3F_BusyHigh();

  sendCommand(0xAA);  // CMDH
  sendData(0x49);
  sendData(0x55);
  sendData(0x20);
  sendData(0x08);
  sendData(0x09);
  sendData(0x18);

  sendCommand(0x01);
  sendData(0x3F);
  sendData(0x00);
  sendData(0x32);
  sendData(0x2A);
  sendData(0x0E);
  sendData(0x2A);

  sendCommand(0x00);
  sendData(0x5F);
  sendData(0x69);

  sendCommand(0x03);
  sendData(0x00);
  sendData(0x54);
  sendData(0x00);
  sendData(0x44);

  sendCommand(0x05);
  sendData(0x40);
  sendData(0x1F);
  sendData(0x1F);
  sendData(0x2C);

  sendCommand(0x06);
  sendData(0x6F);
  sendData(0x1F);
  sendData(0x1F);
  sendData(0x22);

  sendCommand(0x08);
  sendData(0x6F);
  sendData(0x1F);
  sendData(0x1F);
  sendData(0x22);

  sendCommand(0x13);  // IPC
  sendData(0x00);
  sendData(0x04);

  sendCommand(0x30);
  sendData(0x3C);

  sendCommand(0x41);  // TSE
  sendData(0x00);

  sendCommand(0x50);
  sendData(0x3F);

  sendCommand(0x60);
  sendData(0x02);
  sendData(0x00);

  sendCommand(0x61);
  sendData(0x03);
  sendData(0x20);
  sendData(0x01);
  sendData(0xE0);

  sendCommand(0x82);
  sendData(0x1E);

  sendCommand(0x84);
  sendData(0x00);

  sendCommand(0x86);  // AGID
  sendData(0x00);

  sendCommand(0xE3);
  sendData(0x2F);

  sendCommand(0xE0);  // CCSET
  sendData(0x00);

  sendCommand(0xE6);  // TSSET
  sendData(0x00);

  return 0;
}

void Sleep(void) {
  sendCommand(0x07);
  sendData(0xA5);
  delay(10);
  digitalWrite(PIN_SPI_RST, 0);  // Reset
}

// Converts one pixel from input encoding (2 bits) to output encoding (4 bits).
byte convertPixel(char input, byte mask, int shift) {
  const byte value = (input & mask) >> shift;
  switch (value) {
    case 0x0:
      // Black: 000 -> 0000
      return 0x0;
    case 0x1:
      // White: 001 -> 0011
      return 0x3;
    case 0x2:
      // Green: 010 -> 0101
      return 0x5;
    case 0x3:
      // Blue: 011 -> 0110
      return 0x6;
    case 0x4:
      // Red: 100 -> 0100
      return 0x4;
    case 0x5:
      // Yellow: 101 -> 0111
      return 0x7;
    case 0x6:
      // Orange: 110 -> 1000
      return 0x8;
    default:
      Serial.printf("Unknown pixel value: 0x%02X\n", value);
      return 0x0;
  }
}

void EPD_7IN3F_DisplaySegment(const char* image, unsigned long startRow, unsigned long endRow) {
  for (unsigned long i = startRow; i < endRow; i++) {
    for (unsigned long j = 0; j < 800 / 2; j++) {
      sendData(image[j + 800 / 2 * i]);
    }
    // Yield to the FreeRTOS scheduler to reset the watchdog
    delay(1);
  }
}

void EPD_7IN3F_Display(const char* image) {
  const unsigned long rowsPerChunk = 10;  // Number of rows to process per segment

  for (unsigned long startRow = 0; startRow < 480; startRow += rowsPerChunk) {
    unsigned long endRow = startRow + rowsPerChunk;
    if (endRow > 480) endRow = 480;  // Ensure we don't exceed the total rows
    EPD_7IN3F_DisplaySegment(image, startRow, endRow);
  }
}

// Loads partial image data onto the display.
void loadImage(const char* image_data, size_t length) {
  Serial.printf("Loading image data: %d bytes\n", length);
  EPD_7IN3F_Display(image_data);
}

#endif  // display_h