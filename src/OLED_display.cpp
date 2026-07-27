#include <OLED_display.h>

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RST_PIN);

void init_OLED() {
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println("Failed to initialize OLED display");
    while (true); // Stop execution if OLED initialization fails
  }
  display.clearDisplay();
  display.display();
}

int frame_loader = 0;
int frame_cool = 0;
int frame_danger = 0;
void loader_display() {
  display.clearDisplay();
  display.drawBitmap(32, 0, frames_loader[frame_loader], FRAME_WIDTH, FRAME_HEIGHT, 1);
  display.display();
  frame_loader = (frame_loader + 1) % FRAME_COUNT;
  delay(FRAME_DELAY);
}
void cool_display() {
  display.clearDisplay();
  display.drawBitmap(32, 0, frames_cool[frame_cool], FRAME_WIDTH, FRAME_HEIGHT, 1);
  display.display();
  frame_cool = (frame_cool + 1) % FRAME_COUNT;
  delay(FRAME_DELAY);
}

void danger_display() {
  display.clearDisplay();
  display.drawBitmap(32, 0, frames_danger[frame_danger], FRAME_WIDTH, FRAME_HEIGHT, 1);
  display.display();
  frame_danger = (frame_danger + 1) % FRAME_COUNT;
  delay(FRAME_DELAY);
}