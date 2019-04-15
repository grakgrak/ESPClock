ESPClock - ESP32 based clock using a ST7789 240x240 display module.

#define ST7789_DRIVER      // Define additional parameters below for this display
#define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

#define TFT_MOSI 21
#define TFT_SCLK 22
#define TFT_DC   15  // Data Command control pin
#define TFT_RST  18  // Reset pin (could connect to RST pin)

#define SPI_FREQUENCY  40000000 // Maximum to use SPIFFS
