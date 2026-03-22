/*
 * SSD1683 Partial Update Example
 * 
 * This example demonstrates how to use the optimized SSD1683 driver
 * with partial update support for GDEY042Z98 4.2inch e-paper display.
 * 
 * Based on: https://blog.csdn.net/weixin_43550576/article/details/137375815
 */

#include <GxEPD2_3C.h>
#include <GxEPD2_420c_GDEY042Z98_opt.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// Pin definitions for ESP32
#define SPI_CS    5
#define SPI_DC    17
#define SPI_RST   16
#define SPI_BUSY  4

// Create display instance with optimized driver
GxEPD2_420c_GDEY042Z98_opt display(SPI_CS, SPI_DC, SPI_RST, SPI_BUSY);

// Screen dimensions
#define WIDTH 400
#define HEIGHT 300

// Buffer for partial updates
uint8_t* partialBuffer = nullptr;

void setup()
{
  Serial.begin(115200);
  Serial.println("SSD1683 Partial Update Example");
  
  // Initialize display
  display.init(115200);
  
  // Allocate buffer for partial updates
  partialBuffer = (uint8_t*)malloc(WIDTH * HEIGHT / 8);
  if (!partialBuffer) {
    Serial.println("Failed to allocate buffer!");
    while(1);
  }
  
  // Clear screen to white
  display.setRotation(0);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  
  // Full refresh first time
  display.clearScreen(GxEPD_WHITE);
  
  // Draw initial content
  drawStaticContent();
  
  delay(2000);
  
  // Now switch to partial update mode for faster refreshes
  display.initPartialUpdate();
  
  Serial.println("Ready for partial updates!");
}

void loop()
{
  static uint32_t counter = 0;
  static uint32_t lastUpdate = 0;
  
  // Update every second
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    
    // Update only the dynamic part (time display)
    updateTimeDisplay(counter);
    
    counter++;
  }
  
  delay(100);
}

void drawStaticContent()
{
  // Draw static content that doesn't change often
  display.setFullWindow();
  display.firstPage();
  do {
    // Title
    display.setCursor(10, 30);
    display.setTextColor(GxEPD_RED);
    display.println("SSD1683 Partial Update Demo");
    
    // Static info
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 60);
    display.println("Static Content:");
    display.setCursor(10, 80);
    display.println("- Device: GDEY042Z98");
    display.setCursor(10, 100);
    display.println("- Size: 400x300");
    display.setCursor(10, 120);
    display.println("- Controller: SSD1683");
    
    // Draw a box around dynamic area
    display.drawRect(250, 40, 140, 100, GxEPD_BLACK);
    display.setCursor(255, 65);
    display.println("Dynamic Area");
    
  } while (display.nextPage());
}

void updateTimeDisplay(uint32_t counter)
{
  char buffer[32];
  
  // Clear the dynamic area buffer
  memset(partialBuffer, 0xFF, WIDTH * HEIGHT / 8); // 0xFF = white
  
  // Create a virtual display for the partial area
  GxEPD2_3C::paint16bpp paint(partialBuffer, 140, 100); // width, height of partial area
  
  // Set paint properties
  paint.setRotation(0);
  paint.setFont(&FreeMonoBold9pt7b);
  paint.setTextColor(GxEPD_BLACK);
  
  // Draw dynamic content to buffer
  paint.drawRect(0, 0, 140, 100, GxEPD_BLACK);
  
  paint.setCursor(10, 30);
  snprintf(buffer, sizeof(buffer), "Count: %lu", counter);
  paint.print(buffer);
  
  paint.setCursor(10, 55);
  snprintf(buffer, sizeof(buffer), "Time: %lu s", counter);
  paint.print(buffer);
  
  paint.setCursor(10, 80);
  paint.print("Partial Update");
  
  // Write buffer to display (partial update)
  display.writeImagePart(partialBuffer, 0, 0, 140, 100, 250, 40, 140, 100);
  
  // Refresh only the partial area
  display.refresh(250, 40, 140, 100);
}

// Additional example: Update status indicator
void updateStatusIndicator(int x, int y, bool status)
{
  uint8_t buffer[16 * 16 / 8]; // Small 16x16 buffer
  
  GxEPD2_3C::paint16bpp paint(buffer, 16, 16);
  paint.setRotation(0);
  
  // Clear
  paint.fillScreen(GxEPD_WHITE);
  
  // Draw status indicator
  paint.fillCircle(8, 8, 6, status ? GxEPD_RED : GxEPD_BLACK);
  paint.drawCircle(8, 8, 8, GxEPD_BLACK);
  
  // Update display
  display.writeImage(buffer, x, y, 16, 16);
  display.refresh(x, y, 16, 16);
}

// Example of voltage tuning for different batches
void tuneVoltageForBatch(uint8_t batch)
{
  switch (batch) {
    case 1: // Batch 1: default settings
      display.setGateVoltage(0x15);
      display.setSourceVoltage(0x41, 0xA8, 0x32);
      display.setVCOM(0x36);
      break;
      
    case 2: // Batch 2: slightly higher voltage for better contrast
      display.setGateVoltage(0x17);
      display.setSourceVoltage(0x45, 0xA8, 0x32);
      display.setVCOM(0x38);
      break;
      
    case 3: // Batch 3: lower voltage for power saving
      display.setGateVoltage(0x13);
      display.setSourceVoltage(0x3F, 0xA8, 0x32);
      display.setVCOM(0x34);
      break;
  }
}
