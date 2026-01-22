#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <ESP32Servo.h>

// ---------------- HARDWARE PIN CONFIG ----------------
#define DHTPIN 16
#define DHTTYPE DHT11
#define SERVO_PIN 17
#define LED_RED 33
#define LED_YELLOW 32
#define LED_GREEN 25
#define BTN_UP 14
#define BTN_DOWN 27
#define BTN_ENTER 26
#define LDR_PIN 34
#define TRIG_PIN 4
#define ECHO_PIN 19
#define BUZZER 13
#define MQ7_PIN 35
#define SOIL_PIN 36
#define DIST_THRESHOLD 10     
#define LDR_HIGH 2800
#define LDR_MED 1500

// OLED CONFIG
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SENSORS & ACTUATORS
DHT dht(DHTPIN, DHTTYPE);
Servo myservo;


// ---------------- MENU CONFIGURATION ----------------
const int NUM_PROJECTS = 34;
const char* menuItems[NUM_PROJECTS] = {
  "1. Temperature Alarm",
  "2. Collision Alarm",
  "3. Smart Sound",
  "4. Smart OLED",
  "5. Smart Servo",
  "6. Intensity Recognition",
  "7. Smart Soil",
  "8. Gas Alert",
  "9. Traffic Lights",
  "10. Smart LED",
  "11. OLED Game",
  "12. Nokia Snake Game",
  "13. Countdown Timer",
  "14. Music Player",
  "15. Dancing LEDs",
  "16. Pixel Control",
  "17. Matrix Shapes",
  "18. Scrolling Text",
  "19. Animation Loop",
  "20. Temp Monitor",
  "21. Distance Ruler",
  "22. Smart Light",
  "23. Intruder Alarm",
  "24. Servo Sweep",
  "25. Solar Tracker",
  "26. Soil Doctor",
  "27. Gas Safety",
  "28. Parking Sensor",
  "29. Sonic Theremin",
  "30. Reaction Game",
  "31. Stopwatch",
  "32. Electronic Dice",
  "33. Plant Sitter"
};

int currentSelection = 0;
int topVisible = 0;
const int ITEMS_PER_PAGE = 5;

// Button Timing
unsigned long lastBtnTime = 0;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  // Pin Modes
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ENTER, INPUT_PULLUP);

  // Initialize Hardware
  myservo.attach(SERVO_PIN);
  dht.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;)
      ;  // Halt if display fails
  }

  // Welcome Screen
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(11, 20);
  display.println(" SAMARTH DEV BOARD ");
  display.setCursor(15, 35);
  display.println(" Loading Projects... ");
  display.display();
  delay(2000);
}

// ---------------- INPUT HANDLER ----------------
// Returns: 0=None, 1=Up, 2=Down, 3=Enter, 4=Exit(UP+ENTER 1s Hold)
int readButtons() {
  // 1. CHECK EXIT COMBO (UP + ENTER)
  if (digitalRead(BTN_UP) == LOW && digitalRead(BTN_ENTER) == LOW) {
    unsigned long holdStart = millis();
    // Wait while both are held
    while (digitalRead(BTN_UP) == LOW && digitalRead(BTN_ENTER) == LOW) {
      if (millis() - holdStart > 1000) {  // 1 Second Hold required

        // VISUAL FEEDBACK
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(20, 25);
        display.println("Going to Menu...");
        display.display();

        // WAIT FOR RELEASE (Prevent accidental clicks in menu)
        while (digitalRead(BTN_UP) == LOW || digitalRead(BTN_ENTER) == LOW) {
          delay(10);
        }
        delay(200);  // Extra safety debounce

        return 4;  // EXIT COMMAND
      }
    }
  }

  if (millis() - lastBtnTime < 200) return 0;  // Debounce

  if (digitalRead(BTN_UP) == LOW) {
    lastBtnTime = millis();
    return 1;
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    lastBtnTime = millis();
    return 2;
  }
  if (digitalRead(BTN_ENTER) == LOW) {
    lastBtnTime = millis();
    return 3;
  }
  return 0;
}

// ---------------- MENU UI ----------------
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setCursor(0, 0);
  display.println("--- PROJECT MENU ---");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // List Items
  for (int i = 0; i < ITEMS_PER_PAGE; i++) {
    int itemIndex = topVisible + i;
    if (itemIndex >= NUM_PROJECTS) break;

    int yPos = 12 + (i * 10);

    if (itemIndex == currentSelection) {
      // Highlight Selected Item
      display.fillRect(0, yPos - 1, 128, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(2, yPos);
    display.println(menuItems[itemIndex]);
  }
  display.display();
}

// ---------------- HELPER: Footer for Projects ----------------
void drawFooter() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  // Black box to clear previous footer text
  display.fillRect(0, 56, 128, 8, SSD1306_BLACK);

  display.setCursor(0, 56);
  display.print("Hold: UP+ENTER = EXIT");
  display.display();
}

// ==================================================
//               PROJECT IMPLEMENTATIONS
// ==================================================

// 1. Temperature Alarm with 3 Levels
void p_TemperatureAlarm() {
  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(BUZZER, LOW);
      return;
    }

    // Read DHT11
    float temp = dht.readTemperature();   // Celsius
    if (isnan(temp)) temp = 0;            // Fallback

    // Clear LEDs & buzzer
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER, LOW);

    // Determine alert level
    if (temp < 20) {
      digitalWrite(LED_GREEN, HIGH);           // Safe: Green
    } 
    else if (temp >= 20 && temp <= 30) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, HIGH);          // Warning: Green + Yellow
    } 
    else if (temp > 30 && temp <= 35) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_RED, HIGH);             // High: All 3 LEDs
    } 
    else if (temp > 35) {
      digitalWrite(LED_RED, HIGH);             // Critical: Red + buzzer
      digitalWrite(BUZZER, HIGH);
    }

    // Display on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("TEMP ALERT SYSTEM");
    display.setTextSize(2);
    display.setCursor(20, 25);
    display.print(temp, 1);
    display.println(" C");

    display.setTextSize(1);
    display.setCursor(0, 50);
    if (temp < 20) display.println("Status: Safe");
    else if (temp <= 30) display.println("Status: Warning");
    else if (temp <= 35) display.println("Status: High");
    else display.println("Status: Critical");

    drawFooter();
    delay(500);  // Update every 0.5s
  }
}


// 2. Collision Alarm
void p_CollisionAlarm() {
  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(BUZZER, LOW);
      return;
    }

    // Trigger Ultrasonic Sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * 0.034 / 2;  // cm

    // Clear LEDs & buzzer
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER, LOW);

    // Determine alert level based on distance
    if (distance >= 15) {
      digitalWrite(LED_GREEN, HIGH);              // Safe: Green
    } 
    else if (distance >= 10 && distance < 15) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, HIGH);            // Warning: Green + Yellow
    } 
    else if (distance >= 5 && distance < 10) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_RED, HIGH);               // High: All LEDs
    } 
    else if (distance < 5) {
      digitalWrite(LED_RED, HIGH);               // Critical: Red + Buzzer
      digitalWrite(BUZZER, HIGH);
    }

    // Display on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("COLLISION ALARM");
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.print(distance, 1);
    display.println(" cm");

    display.setTextSize(1);
    display.setCursor(0, 50);
    if (distance >= 15) display.println("Status: Safe");
    else if (distance >= 10) display.println("Status: Warning");
    else if (distance >= 5) display.println("Status: High");
    else display.println("Status: Critical");

    drawFooter();
    delay(100);  // Refresh rate
  }
}

// 3. Smart Sound (Buzzer volume based on light)
// 3. Smart Sound (Buzzer louder as light fades)
// 3. Smart Sound (Buzzer louder as light fades) – analogWrite version
void p_SmartSound() {
  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      analogWrite(BUZZER, 0);  // Turn off buzzer
      return;
    }

    // Read LDR value
    int ldrVal = analogRead(LDR_PIN);  // LDR value: 600 (bright) → 3500 (dark)
    int constrainedVal = constrain(ldrVal, 600, 3500);

    // Map darkness to buzzer intensity (0 = off, 255 = max)
    int buzzerLevel = map(constrainedVal, 600, 3500, 0, 255);

    // Set buzzer intensity
    analogWrite(BUZZER, buzzerLevel);

    // OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("3. SMART SOUND");

    display.setCursor(0, 20);
    display.print("Light: ");
    display.println(ldrVal);

    display.setCursor(0, 40);
    display.print("Buzzer Intensity: ");
    display.println(buzzerLevel);

    drawFooter();
    delay(100);  // Smooth update every 0.1s
  }
}

// 4. Smart OLED Power Control using Ultrasonic Sensor
void p_SmartOLED() {
  bool oledOn = true;

  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      display.ssd1306_command(SSD1306_DISPLAYON);
      return;
    }

    // Ultrasonic trigger
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    float distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;

    // OLED control logic
    if (distance < 5 && oledOn) {
      display.clearDisplay();
      display.display();
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      oledOn = false;
    }
    else if (distance >= 5 && !oledOn) {
      display.ssd1306_command(SSD1306_DISPLAYON);
      oledOn = true;
    }

    if (oledOn) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("9. SMART OLED");

      display.setCursor(0, 20);
      display.print("Distance: ");
      display.print(distance, 1);
      display.println(" cm");

      display.setCursor(0, 40);
      display.println("OLED: ON");

      drawFooter();
    }

    delay(150);
  }
}

// 5. Smart Servo (moves based on object distance)
void p_SmartServo() {
  int pos = 90;       // Start at middle position
  int step = 10;       // Servo step size

  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      myservo.write(90);  // Reset servo to neutral
      return;
    }

    // Trigger ultrasonic sensor
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    float distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2; // cm

    // Determine servo direction
    if (distance < 10) {
      pos += step;  // Move forward
      if (pos > 180) pos = 180;  // Limit max angle
    } else {
      pos -= step;  // Move backward
      if (pos < 0) pos = 0;      // Limit min angle
    }

    myservo.write(pos);

    // OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("5. SMART SERVO");

    display.setCursor(0, 20);
    display.print("Distance: ");
    display.print(distance, 1);
    display.println(" cm");

    display.setCursor(0, 40);
    display.print("Servo Angle: ");
    display.println(pos);

    drawFooter();
    delay(50);  // Smooth movement
  }
}


// 6. Smart Colour Recognition using LDR
void p_SmartColor() {
  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      return;
    }

    int ldrVal = analogRead(LDR_PIN);

    // Turn OFF all LEDs first
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);

    String intensityDetected = "";

    // --- Threshold-based color detection ---
    if (ldrVal < 1200) {
      digitalWrite(LED_RED, HIGH);
      intensityDetected = "LOW LUMINOSITY";
    }
    else if (ldrVal >= 1200 && ldrVal < 2500) {
      digitalWrite(LED_YELLOW, HIGH);
      intensityDetected = "MEDIUM LUMINOSITY";
    }
    else {
      digitalWrite(LED_GREEN, HIGH);
      intensityDetected = "HIGH LUMINOSITY";
    }

    // OLED Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("6. COLOR SENSOR");

    display.setCursor(0, 20);
    display.print("LDR Value: ");
    display.println(ldrVal);

    display.setTextSize(2);
    display.setCursor(10, 40);
    display.println(intensityDetected);

    drawFooter();
    delay(150);
  }
}


// 7. Smart Soil Moisture Servo System
void p_SmartSoil() {
  const int threshold = 2000;   // Adjust based on calibration
  int servoPos = 90;            // Neutral position

  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      myservo.write(90);  // Reset servo
      return;
    }

    int moisture = analogRead(SOIL_PIN);

    // Servo control logic
    if (moisture < threshold) {
      servoPos = 160;   // Dry soil → Water ON
    } else {
      servoPos = 20;    // Wet soil → Water OFF
    }

    myservo.write(servoPos);

    // OLED Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("7. SMART SOIL");

    display.setCursor(0, 20);
    display.print("Moisture: ");
    display.println(moisture);

    display.setCursor(0, 40);
    if (moisture < threshold)
      display.println("Status: DRY");
    else
      display.println("Status: WET");

    drawFooter();
    delay(300);
  }
}

// 8. Smart Gas Alert System
void p_GasAlarm() {
  while (true) {
    int btn = readButtons();
    if (btn == 4) {  // Exit
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);
      digitalWrite(BUZZER, LOW);
      return;
    }

    int gasValue = analogRead(MQ7_PIN);

    // Turn OFF everything first
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(BUZZER, LOW);

    String status = "";

    // --- Gas level logic ---
    if (gasValue < 750) {
      digitalWrite(LED_GREEN, HIGH);
      status = "SAFE";
    }
    else if (gasValue >= 750 && gasValue < 850) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, HIGH);
      status = "WARNING";
    }
    else if (gasValue >= 850 && gasValue < 1000) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_RED, HIGH);
      status = "HIGH";
    }
    else {
      digitalWrite(LED_RED, HIGH);
      digitalWrite(BUZZER, HIGH);
      status = "DANGER!";
    }

    // OLED Display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("8. GAS ALERT");

    display.setCursor(0, 20);
    display.print("Gas Value: ");
    display.println(gasValue);

    display.setTextSize(2);
    display.setCursor(0, 40);
    display.println(status);

    drawFooter();
    delay(200);
  }
}


// 9. Traffic Lights
void p_Traffic() {
  int state = 0;  // 0=Red, 1=Yel, 2=Grn
  unsigned long timer = millis();

  while (true) {
    if (readButtons() == 4) {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("9. TRAFFIC SIM");
    display.setTextSize(2);
    display.setCursor(20, 25);

    if (state == 0) {
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      display.println("STOP");
      if (millis() - timer > 3000) {
        state = 1;
        timer = millis();
      }
    } else if (state == 1) {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_GREEN, LOW);
      display.println("READY");
      if (millis() - timer > 1000) {
        state = 2;
        timer = millis();
      }
    } else if (state == 2) {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, HIGH);
      display.println("GO!");
      if (millis() - timer > 3000) {
        state = 0;
        timer = millis();
      }
    }

    drawFooter();
    delay(50);
  }
}

// 10. Smart LED Pattern System (Ultrasonic + LDR)
void p_SmartLedPattern() {

  while (true) {
    int btn = readButtons();
    if (btn == 4) {   // Exit
      analogWrite(LED_RED, 0);
      analogWrite(LED_YELLOW, 0);
      analogWrite(LED_GREEN, 0);
      return;
    }

    // ---------- Ultrasonic ----------
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    float distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;

    // ---------- LDR ----------
    int ldrVal = analogRead(LDR_PIN);

    // ---------- Case 1: No object nearby ----------
    if (distance >= DIST_THRESHOLD) {
      analogWrite(LED_RED, 60);
      analogWrite(LED_YELLOW, 60);
      analogWrite(LED_GREEN, 60);
    }

    // ---------- Object detected ----------
    else {
      // HIGH light → LEDs OFF
      if (ldrVal > LDR_HIGH) {
        analogWrite(LED_RED, 0);
        analogWrite(LED_YELLOW, 0);
        analogWrite(LED_GREEN, 0);
      }

      // MEDIUM light → Pattern
      else if (ldrVal > LDR_MED) {
        int pattern[][3] = {
          {255, 0, 0},   // Red
          {0, 255, 0},   // Green
          {0, 0, 255},   // Yellow
          {0, 255, 0},
          {0, 0, 255},
          {0, 255, 0},
          {0, 0, 255},
          {255, 0, 0},
          {0, 0, 255},
          {0, 255, 0}
        };

        for (int i = 0; i < 10; i++) {
          analogWrite(LED_RED, pattern[i][0]);
          analogWrite(LED_GREEN, pattern[i][1]);
          analogWrite(LED_YELLOW, pattern[i][2]);
          delay(200);

          if (readButtons() == 4) return;
        }
      }

      // VERY LOW light → Fade IN / OUT
      else {
        for (int b = 0; b <= 255; b += 5) {
          analogWrite(LED_RED, b);
          analogWrite(LED_YELLOW, b);
          analogWrite(LED_GREEN, b);
          delay(20);
          if (readButtons() == 4) return;
        }
        for (int b = 255; b >= 0; b -= 5) {
          analogWrite(LED_RED, b);
          analogWrite(LED_YELLOW, b);
          analogWrite(LED_GREEN, b);
          delay(20);
          if (readButtons() == 4) return;
        }
      }
    }

    // ---------- OLED ----------
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("10. SMART LED");

    display.setCursor(0, 18);
    display.print("Dist: ");
    display.print(distance, 1);
    display.println(" cm");

    display.setCursor(0, 30);
    display.print("LDR: ");
    display.println(ldrVal);

    display.setCursor(0, 45);
    if (distance >= DIST_THRESHOLD)
      display.println("MODE: AMBIENT");
    else if (ldrVal > LDR_HIGH)
      display.println("MODE: OFF");
    else if (ldrVal > LDR_MED)
      display.println("MODE: PATTERN");
    else
      display.println("MODE: FADE");

    drawFooter();
    delay(100);
  }
}


// 11. OLED Game using UP, DOWN, SELECT buttons
void p_Game() {

  int playerY = 25;
  int blockX = 120;
  int blockY = random(0, 56);
  int score = 0;
  bool gameRunning = false;

  while (true) {
    int btn = readButtons();

    // EXIT to menu
    if (btn == 4) {
      display.clearDisplay();
      display.display();
      return;
    }

    // START / RESTART
    if (btn == 3 && !gameRunning) {
      score = 0;
      playerY = 25;
      blockX = 120;
      blockY = random(0, 56);
      gameRunning = true;
    }

    if (gameRunning) {
      // Player movement
      if (btn == 1 && playerY > 0) playerY -= 3;      // UP
      if (btn == 2 && playerY < 52) playerY += 3;     // DOWN

      // Move obstacle
      blockX -= 3;

      // Collision check
      if (blockX <= 6) {
        if (blockY >= playerY && blockY <= playerY + 12) {
          score++;
          blockX = 120;
          blockY = random(0, 56);
        } else {
          gameRunning = false;  // Game Over
        }
      }
    }

    // ---------- OLED DRAW ----------
    display.clearDisplay();
    display.setTextSize(1);

    if (!gameRunning) {
      display.setCursor(18, 15);
      display.println("OLED GAME");

      display.setCursor(8, 30);
      display.println("Press SELECT");

      display.setCursor(30, 45);
      display.print("Score: ");
      display.println(score);
    } 
    else {
      // Player
      display.fillRect(0, playerY, 4, 12, SSD1306_WHITE);

      // Falling block
      display.fillRect(blockX, blockY, 6, 6, SSD1306_WHITE);

      // Score
      display.setCursor(90, 0);
      display.print("S:");
      display.println(score);
    }

    display.display();
    delay(60);
  }
}


// 12. Nokia Snake Game (OLED)
void p_Snake() {

  const int GRID_W = 16;   // 128 / 8
  const int GRID_H = 8;    // 64 / 8
  int snakeX[50], snakeY[50];
  int length = 3;

  int dir = 0; // 0=RIGHT, 1=UP, 2=DOWN , 3= LEFT
  int foodX, foodY;
  bool running = false;

  auto spawnFood = [&]() {
    foodX = random(1, GRID_W);
    foodY = random(1, GRID_H);
  };

  spawnFood();

  while (true) {
    int btn = readButtons();

    // EXIT
    if (btn == 4) {
      display.clearDisplay();
      display.display();
      return;
    }

    // START / RESET
    if (btn == 3 && !running) {
      length = 3;
      dir = 0;
      for (int i = 0; i < length; i++) {
        snakeX[i] = 5 - i;
        snakeY[i] = 4;
      }
      spawnFood();
      running = true;
    }

    if (running) {
      // Controls
      if (btn == 1) dir = 1; // UP
      if (btn == 2) dir = 2; // DOWN
      if (btn == 3) dir = 0; // RIGHT
      if (btn == 2) dir = 3; // LEFT


      // Move body
      for (int i = length - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
      }

      // Move head
      if (dir == 0) snakeX[0]++;
      if (dir == 1) snakeY[0]--;
      if (dir == 2) snakeY[0]++;
      if (dir == 3) snakeY[0]--;
      // Wall collision
      if (snakeX[0] <= 0 || snakeX[0] >= GRID_W ||
          snakeY[0] <= 0 || snakeY[0] >= GRID_H) {
        running = true;
      }

      // Food collision
      if (snakeX[0] == foodX && snakeY[0] == foodY) {
        length++;
        spawnFood();
      }

      // Self collision
      for (int i = 1; i < length; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
          running = false;
        }
      }
    }

    // ---------- DRAW ----------
    display.clearDisplay();
    display.setTextSize(1);

    if (!running) {
      display.setCursor(30, 10);
      display.println("SNAKE GAME");

      display.setCursor(12, 30);
      display.println("Press SELECT");

      display.setCursor(20, 45);
      display.print("Score: ");
      display.println(length - 3);
    } else {
      // Draw snake
      for (int i = 0; i < length; i++) {
        display.fillRect(snakeX[i] * 8, snakeY[i] * 8, 8, 8, SSD1306_WHITE);
      }

      // Draw food
      display.fillRect(foodX * 8, foodY * 8, 8, 8, SSD1306_WHITE);

      // Score
      display.setCursor(0, 0);
      display.print("S:");
      display.println(length - 3);
    }

    display.display();
    delay(180);
  }
}


//13. Countdown
void p_Countdown() {

  int setTime = 30;          // Default time in seconds
  int remainingTime = setTime;
  bool running = false;
  unsigned long lastTick = 0;

  while (true) {
    int btn = readButtons();

    // EXIT
    if (btn == 4) {
      digitalWrite(BUZZER, LOW);
      return;
    }

    // INCREASE TIME
    if (btn == 1 && !running) {
      setTime += 10;
      if (setTime > 300) setTime = 300; // Max 5 minutes
      remainingTime = setTime;
    }

    // DECREASE TIME
    if (btn == 2 && !running) {
      setTime -= 10;
      if (setTime < 10) setTime = 10;
      remainingTime = setTime;
    }

    // START / STOP
    if (btn == 3) {
      running = !running;
      lastTick = millis();
    }

    // TIMER COUNTDOWN
    if (running && millis() - lastTick >= 1000) {
      lastTick = millis();
      remainingTime--;

      // Time Over
      if (remainingTime <= 0) {
        running = false;
        remainingTime = 0;

        // BUZZER ALERT
        digitalWrite(BUZZER, HIGH);
        delay(1500);
        digitalWrite(BUZZER, LOW);
      }
    }

    // ---------- OLED DISPLAY ----------
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("13. TIMER");

    display.setTextSize(2);
    display.setCursor(20, 25);

    int mins = remainingTime / 60;
    int secs = remainingTime % 60;

    if (mins < 10) display.print("0");
    display.print(mins);
    display.print(":");
    if (secs < 10) display.print("0");
    display.println(secs);

    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println("UP:+10 DOWN:-10");
    display.setCursor(0, 58);
    display.println("SEL=Start/Stop");

    display.display();
    delay(100);
  }
}


// 14. Music Player
void p_music() {

  // ---------- MELODIES ----------
  int melody1[] = {262, 294, 330, 349, 392};          // Scale Up
  int melody2[] = {392, 349, 330, 294, 262};          // Scale Down
  int melody3[] = {262, 262, 392, 392, 440, 440, 392}; // Twinkle start
  int melody4[] = {330, 330, 349, 392, 392, 349, 330}; // Simple tune
  int melody5[] = {523, 494, 440, 392, 349};          // Alarm tone

  int* melodies[] = {melody1, melody2, melody3, melody4, melody5};
  int melodyLength[] = {5, 5, 7, 7, 5};

  const char* melodyNames[] = {
    "Scale Up",
    "Scale Down",
    "Twinkle",
    "Simple Tune",
    "Alarm"
  };

  int index = 0;

  while (true) {
    int btn = readButtons();

    // EXIT
    if (btn == 4) {
      noTone(BUZZER);
      return;
    }

    // NEXT MELODY
    if (btn == 1) {
      index++;
      if (index > 4) index = 0;
    }

    // PREVIOUS MELODY
    if (btn == 2) {
      index--;
      if (index < 0) index = 4;
    }

    // PLAY MELODY
    if (btn == 3) {
      for (int i = 0; i < melodyLength[index]; i++) {
        tone(BUZZER, melodies[index][i]);
        delay(250);
        noTone(BUZZER);
        delay(50);

        // Allow exit during playback
        if (readButtons() == 4) {
          noTone(BUZZER);
          return;
        }
      }
    }

    // ---------- OLED DISPLAY ----------
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("14. MELODY PLAYER");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setCursor(0, 25);
    display.print("Melody: ");
    display.println(index + 1);

    display.setCursor(0, 40);
    display.println(melodyNames[index]);

    display.setCursor(0, 55);
    display.println("UP/DN Select  SEL Play");

    display.display();
    delay(150);
  }
}


// 15. Dancing LEDs
void p_Dancing(){

  // Melody (~2 minutes looped)
  int melody[] = {
    262, 330, 392, 330, 262, 330, 392, 523,
    392, 330, 262, 330, 392, 330, 262, 196,
    262, 330, 392, 523, 659, 523, 392, 330
  };
  int melodyLength = sizeof(melody) / sizeof(melody[0]);

  unsigned long startTime = millis();
  int pattern = 0;

  while (true) {
    // EXIT
    if (readButtons() == 4) {
      noTone(BUZZER);
      analogWrite(LED_RED, 0);
      analogWrite(LED_YELLOW, 0);
      analogWrite(LED_GREEN, 0);
      return;
    }

    // Change pattern every 10 seconds
    if ((millis() - startTime) / 10000 != pattern) {
      pattern = (pattern + 1) % 4;
    }

    for (int i = 0; i < melodyLength; i++) {

      int note = melody[i];
      tone(BUZZER, note);

      // -------- LED PATTERNS --------
      if (pattern == 0) {  
        // Beat Blink
        digitalWrite(LED_RED, note > 400);
        digitalWrite(LED_YELLOW, note > 300 && note <= 400);
        digitalWrite(LED_GREEN, note <= 300);
      }

      if (pattern == 1) {  
        // Fade All
        for (int f = 0; f <= 255; f += 25) {
          analogWrite(LED_RED, f);
          analogWrite(LED_YELLOW, f);
          analogWrite(LED_GREEN, f);
          delay(5);
        }
      }

      if (pattern == 2) {  
        // Alternating Chase
        digitalWrite(LED_RED, HIGH);
        delay(50);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        delay(50);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, HIGH);
        delay(50);
        digitalWrite(LED_GREEN, LOW);
      }

      if (pattern == 3) {  
        // Pulse by pitch
        analogWrite(LED_RED, map(note, 200, 700, 50, 255));
        analogWrite(LED_GREEN, map(note, 200, 700, 255, 50));
      }

      delay(180);
      noTone(BUZZER);

      // Stop after ~2 minutes
      if (millis() - startTime > 120000) {
        noTone(BUZZER);
        analogWrite(LED_RED, 0);
        analogWrite(LED_YELLOW, 0);
        analogWrite(LED_GREEN, 0);
        startTime = millis();  // restart show
      }

      // Allow exit during play
      if (readButtons() == 4) {
        noTone(BUZZER);
        analogWrite(LED_RED, 0);
        analogWrite(LED_YELLOW, 0);
        analogWrite(LED_GREEN, 0);
        return;
      }
    }

    // OLED Status
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.println("MUSIC LED SHOW");
    display.setCursor(18, 40);
    display.println("Enjoy the Beat!");
    display.display();
  }
}

//16. OLED Single Pixel Control Demo
void p_SinglePixel() {

  int x = 64;   // Start center
  int y = 32;

  while (true) {
    int btn = readButtons();

    // EXIT
    if (btn == 4) {
      display.clearDisplay();
      display.display();
      return;
    }

    // MOVE CONTROLS
    if (btn == 1 && y > 0) y--;        // UP
    if (btn == 2 && y < 63) y++;       // DOWN
    if (btn == 3 && x < 127) x++;      // SELECT → RIGHT

    // WRAP AROUND
    if (x >= 128) x = 0;

    // DRAW PIXEL
    display.clearDisplay();
    display.drawPixel(x, y, SSD1306_WHITE);

    // INFO TEXT
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("X:");
    display.print(x);
    display.print(" Y:");
    display.print(y);

    display.setCursor(0, 54);
    display.println("UP/DN Move  SEL->");

    display.display();
    delay(60);
  }
}

//17. Shapes on Matrix Display using OLED
// Shapes on Matrix Display using OLED
void p_MatrixShapes() {

  int shape = 0;   // 0=square,1=rect,2=circle,3=triangle
  int x = 32;
  int y = 16;
  bool animate = false;

  while (true) {
    int btn = readButtons();

    // EXIT
    if (btn == 4) {
      display.clearDisplay();
      display.display();
      return;
    }

    // SHAPE SELECT
    if (btn == 1) shape = (shape + 1) % 4;   // UP
    if (btn == 2) shape = (shape + 3) % 4;   // DOWN

    // TOGGLE ANIMATION
    if (btn == 3) animate = !animate;

    // SIMPLE ANIMATION
    if (animate) {
      x += 2;
      if (x > 96) x = 0;
    }

    // ---------- DRAW ----------
    display.clearDisplay();

    // Draw virtual matrix grid (8x8 look)
    for (int gx = 0; gx < 128; gx += 16) {
      for (int gy = 0; gy < 64; gy += 16) {
        display.drawRect(gx, gy, 16, 16, SSD1306_WHITE);
      }
    }

    // Draw selected shape
    switch (shape) {

      case 0: // Square
        display.fillRect(x, y, 16, 16, SSD1306_WHITE);
        break;

      case 1: // Rectangle
        display.fillRect(x, y, 32, 16, SSD1306_WHITE);
        break;

      case 2: // Circle
        display.fillCircle(x + 8, y + 8, 8, SSD1306_WHITE);
        break;

      case 3: // Triangle (pixel style)
        display.fillTriangle(
          x + 8, y,
          x, y + 16,
          x + 16, y + 16,
          SSD1306_WHITE
        );
        break;
    }

    // INFO TEXT
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Shape: ");
    if (shape == 0) display.println("Square");
    if (shape == 1) display.println("Rect");
    if (shape == 2) display.println("Circle");
    if (shape == 3) display.println("Triangle");

    display.setCursor(0, 56);
    display.println("UP/DN Sel  SEL Move");

    display.display();
    delay(80);
  }
}


    //18. Scrolling Text Demo
    void p_ScrollingText() {
  
  String message = "HELLO WORLD! WELCOME TO SAMARTH DEV BOARD DEMO ";
  int scrollX = 128;   // Start off-screen right
  int speed = 2;       // Pixels per frame
  bool paused = false;

  while (true) {
    int btn = readButtons();

    // EXIT
    if (btn == 4) {
      display.clearDisplay();
      display.display();
      return;
    }

    // SPEED CONTROL
    if (btn == 1 && speed < 10) speed++;      // UP → Faster
    if (btn == 2 && speed > 1) speed--;       // DOWN → Slower
    if (btn == 3) paused = !paused;           // SELECT → Pause/Resume

    // ---------- DRAW ----------
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(scrollX, 25);
    display.println(message);

    display.setCursor(0, 0);
    display.print("Speed: ");
    display.println(speed);
    display.setCursor(0, 55);
    display.println("UP/DN Speed  SEL Pause");

    display.display();

    if (!paused) {
      scrollX -= speed;
      if (scrollX < -((int)message.length() * 6)) { // Reset after fully off-screen
        scrollX = 128;
      }
    }

    delay(50);
  }
}

// Animation Loop Demo
void p_AnimationLoop(){

  int speed = 2;       // Pixels per frame
  bool paused = false;

  // Shape positions and directions
  int x1 = 0, y1 = 10, dx1 = 1, dy1 = 1;   // Square
  int x2 = 64, y2 = 20, dx2 = 2, dy2 = 1;  // Circle
  int x3 = 32, y3 = 40, dx3 = 1, dy3 = 2;  // Pixel

  while (true) {
    int btn = readButtons();

    // EXIT
    if (btn == 4) {
      display.clearDisplay();
      display.display();
      return;
    }

    // SPEED CONTROL
    if (btn == 1 && speed < 5) speed++;    // UP → Faster
    if (btn == 2 && speed > 1) speed--;    // DOWN → Slower
    if (btn == 3) paused = !paused;        // SELECT → Pause/Resume

    // ---------- ANIMATION ----------
    if (!paused) {
      // Move square
      x1 += dx1 * speed;
      y1 += dy1 * speed;
      if (x1 <= 0 || x1 >= 112) dx1 *= -1;  // Bounce edges (128-16)
      if (y1 <= 10 || y1 >= 48) dy1 *= -1;

      // Move circle
      x2 += dx2 * speed;
      y2 += dy2 * speed;
      if (x2 <= 0 || x2 >= 120) dx2 *= -1;  // 128-8*2 radius
      if (y2 <= 10 || y2 >= 56) dy2 *= -1;

      // Move pixel
      x3 += dx3 * speed;
      y3 += dy3 * speed;
      if (x3 < 0 || x3 > 127) dx3 *= -1;
      if (y3 < 10 || y3 > 63) dy3 *= -1;
    }

    // ---------- DRAW ----------
    display.clearDisplay();

    // Draw square
    display.fillRect(x1, y1, 16, 16, SSD1306_WHITE);

    // Draw circle
    display.fillCircle(x2, y2, 8, SSD1306_WHITE);

    // Draw pixel
    display.drawPixel(x3, y3, SSD1306_WHITE);

    // HUD
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Speed: ");
    display.println(speed);
    display.setCursor(0, 55);
    display.println("UP/DN Speed  SEL Pause");

    display.display();
    delay(50);
  }
}


// 1. Temp Monitor
void p_TempMonitor() {
  while (true) {
    if (readButtons() == 4) return;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("1. WEATHER STATION");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 20);
    if (isnan(t)) display.println("Sens Err");
    else {
      display.print(t, 1);
      display.println(" C");
    }

    display.setCursor(0, 40);
    if (isnan(h)) display.println("");
    else {
      display.print(h, 0);
      display.println(" % RH");
    }

    drawFooter();
    delay(200);
  }
}

// 2. Ultrasonic Ruler
void p_Distance() {
  while (true) {
    if (readButtons() == 4) return;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    int cm = duration * 0.034 / 2;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("2. DIGITAL RULER");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setTextSize(3);
    display.setCursor(10, 25);
    display.print(cm);
    display.setTextSize(2);
    display.println(" cm");

    drawFooter();
    delay(100);
  }
}

// 3. Smart Light (UPDATED: Progress Bars & Proper Turn Off)
void p_SmartLight() {
  while (true) {
    // Check Exit
    if (readButtons() == 4) {
      // FORCE TURN OFF
      analogWrite(LED_GREEN, 0);
      analogWrite(LED_RED, 0);
      return;
    }

    int val = analogRead(LDR_PIN);
    // Calibration: 600 (Bright) to 3500 (Dark)
    int constrainedVal = constrain(val, 600, 3500);

    // GREEN: Follows Darkness (Darker = Brighter Green)
    int greenPWM = map(constrainedVal, 600, 3500, 0, 255);

    // RED: Opposite (Brighter = Brighter Red)
    int redPWM = map(constrainedVal, 600, 3500, 255, 0);

    analogWrite(LED_GREEN, greenPWM);
    analogWrite(LED_RED, redPWM);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("3. SMART LIGHT");

    display.setCursor(0, 20);
    display.print("LDR Value: ");
    display.println(val);

    // --- DRAW PROGRESS BARS ---

    // Green Bar (Night Mode)
    display.setCursor(0, 32);
    display.print("Green(N):");
    display.drawRect(55, 32, 70, 8, SSD1306_WHITE);  // Frame
    int gBar = map(greenPWM, 0, 255, 0, 70);
    display.fillRect(55, 32, gBar, 8, SSD1306_WHITE);

    // Red Bar (Day Mode)
    display.setCursor(0, 44);
    display.print("Red(D)  :");
    display.drawRect(55, 44, 70, 8, SSD1306_WHITE);  // Frame
    int rBar = map(redPWM, 0, 255, 0, 70);
    display.fillRect(55, 44, rBar, 8, SSD1306_WHITE);

    drawFooter();
    delay(50);
  }
}

// 4. Intruder Alarm
void p_Intruder() {
  bool alarmActive = false;
  while (true) {
    int btn = readButtons();
    if (btn == 4) {
      digitalWrite(BUZZER, LOW);
      return;
    }

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    int cm = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;

    if (cm > 0 && cm < 15) alarmActive = true;
    else alarmActive = false;

    if (alarmActive) {
      digitalWrite(BUZZER, HIGH);
      digitalWrite(LED_RED, HIGH);
    } else {
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED_RED, LOW);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("4. SECURITY SYS");

    display.setTextSize(2);
    display.setCursor(10, 25);
    if (alarmActive) display.println("BREACH!");
    else display.println("SAFE");

    drawFooter();
    delay(100);
  }
}

// 5. Servo Sweep
void p_ServoSweep() {
  int pos = 0;
  int dir = 1;  // 1 = up, -1 = down
  while (true) {
    if (readButtons() == 4) return;

    myservo.write(pos);
    pos += (5 * dir);
    if (pos >= 180 || pos <= 0) dir *= -1;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("5. SERVO DEMO");
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.print("Angle:");
    display.println(pos);

    drawFooter();
    delay(50);
  }
}

// 6. Solar Tracker (Smoothed)
void p_SolarTracker() {
  float smoothedVal = 0;
  int lastAngle = -1;

  while (true) {
    if (readButtons() == 4) return;

    int val = analogRead(LDR_PIN);
    smoothedVal = (smoothedVal * 0.90) + (val * 0.10);

    // Using calibrated range for smoother solar tracking
    int targetAngle = map(constrain((int)smoothedVal, 600, 3500), 600, 3500, 0, 180);

    if (abs(targetAngle - lastAngle) > 2) {
      myservo.write(targetAngle);
      lastAngle = targetAngle;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("6. SOLAR TRACKER");

    display.setCursor(0, 20);
    display.print("Light: ");
    display.println((int)smoothedVal);
    display.setTextSize(2);
    display.setCursor(0, 35);
    display.print("Ang: ");
    display.println(lastAngle);

    drawFooter();
    delay(50);
  }
}

// 7. Soil Doctor
void p_SoilDoctor() {
  while (true) {
    if (readButtons() == 4) return;

    int val = analogRead(SOIL_PIN);
    // Assuming 4095 = Dry, 0 = Wet
    int moisture = map(val, 4095, 0, 0, 100);
    if (moisture < 0) moisture = 0;
    if (moisture > 100) moisture = 100;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("7. SOIL DOCTOR");

    display.setTextSize(2);
    display.setCursor(0, 25);
    display.print(moisture);
    display.println("% Wet");

    display.setTextSize(1);
    display.setCursor(0, 45);
    if (moisture < 20) display.println("Status: THIRSTY!");
    else if (moisture > 80) display.println("Status: OVERWATER");
    else display.println("Status: PERFECT");

    drawFooter();
    delay(200);
  }
}

// 8. Gas Safety
void p_GasSafety() {
  while (true) {
    if (readButtons() == 4) {
      digitalWrite(BUZZER, LOW);
      return;
    }

    int val = analogRead(MQ7_PIN);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("8. GAS DETECTOR");

    display.setTextSize(2);
    display.setCursor(10, 25);
    display.println(val);

    display.setTextSize(1);
    display.setCursor(0, 45);
    if (val > 2000) {  // Threshold
      display.println("Status: DANGER!");
      digitalWrite(BUZZER, HIGH);
      digitalWrite(LED_RED, HIGH);
    } else {
      display.println("Status: CLEAN AIR");
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED_RED, LOW);
    }

    drawFooter();
    delay(200);
  }
}



// 10. Parking Sensor
void p_Parking() {
  while (true) {
    if (readButtons() == 4) {
      digitalWrite(BUZZER, LOW);
      return;
    }

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    int cm = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("10. PARKING");
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.print(cm);
    display.println(" cm");
    drawFooter();

    // Beep logic
    if (cm < 5) {
      digitalWrite(BUZZER, HIGH);  // Constant tone
    } else if (cm < 15) {
      digitalWrite(BUZZER, HIGH);
      delay(50);
      digitalWrite(BUZZER, LOW);
      delay(50);
    } else if (cm < 30) {
      digitalWrite(BUZZER, HIGH);
      delay(100);
      digitalWrite(BUZZER, LOW);
      delay(200);
    } else {
      digitalWrite(BUZZER, LOW);
    }
  }
}

// 11. Sonic Theremin (Music by Distance)
void p_Theremin() {
  while (true) {
    if (readButtons() == 4) {
      noTone(BUZZER);
      return;
    }

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    int cm = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("11. THEREMIN");
    display.setCursor(0, 25);
    display.println("Move hand to play!");
    drawFooter();

    if (cm > 2 && cm < 40) {
      int pitch = map(cm, 2, 40, 1000, 200);
      tone(BUZZER, pitch);
    } else {
      noTone(BUZZER);
    }
    delay(50);
  }
}

// 12. Reaction Game (FIXED: Yellow LED & Release Check)
void p_Reaction() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("12. REACTION");
  display.setCursor(0, 20);
  display.println("Wait for LED...");
  display.setCursor(0, 30);
  display.println("Press UP fast!");
  display.display();
  delay(2000);

  while (true) {
    // 1. Ensure Buttons are RELEASED (Anti-Cheat / Logic Fix)
    while (digitalRead(BTN_UP) == LOW || digitalRead(BTN_ENTER) == LOW) {
      delay(10);
    }
    digitalWrite(LED_YELLOW, LOW);  // Ensure OFF

    // 2. Random Wait
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Wait...");
    drawFooter();

    // Wait random time (Check exit during wait)
    long waitTime = random(2000, 5000);
    long waitStart = millis();
    while (millis() - waitStart < waitTime) {
      if (readButtons() == 4) return;  // Exit command check
    }

    // 3. LED ON -> Measure Reaction
    digitalWrite(LED_YELLOW, HIGH);
    unsigned long start = millis();

    // Wait for button press
    while (digitalRead(BTN_UP) == HIGH) {
      // Allow exit during wait
      if (readButtons() == 4) {
        digitalWrite(LED_YELLOW, LOW);
        return;
      }
    }

    unsigned long result = millis() - start;
    digitalWrite(LED_YELLOW, LOW);

    // 4. Show Result
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print(result);
    display.println(" ms");
    display.setTextSize(1);
    display.setCursor(0, 45);
    display.println("Press DOWN to Retry");
    drawFooter();

    // Wait for retry
    while (digitalRead(BTN_DOWN) == HIGH) {
      if (readButtons() == 4) return;
    }
    delay(500);
  }
}

// 13. Stopwatch
void p_Stopwatch() {
  unsigned long startTime = 0;
  bool running = false;
  float elapsed = 0;

  while (true) {
    int btn = readButtons();
    if (btn == 4) return;

    if (btn == 3) {  // Enter to toggle
      if (running) {
        running = false;
      } else {
        startTime = millis() - (unsigned long)(elapsed * 1000);
        running = true;
      }
    }
    if (btn == 1) {  // UP to Reset
      running = false;
      elapsed = 0;
    }

    if (running) elapsed = (millis() - startTime) / 1000.0;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("13. STOPWATCH");
    display.setTextSize(2);
    display.setCursor(15, 25);
    display.print(elapsed, 1);
    display.println(" s");

    display.setTextSize(1);
    display.setCursor(0, 45);
    display.println("ENTER=St/Stop UP=Rst");
    drawFooter();
  }
}

// 14. Electronic Dice
void p_Dice() {
  while (true) {
    if (readButtons() == 4) return;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("14. E-DICE");
    display.setCursor(0, 30);
    display.println("Press UP to Roll");
    drawFooter();

    if (digitalRead(BTN_UP) == LOW) {
      // Rolling animation
      for (int i = 0; i < 10; i++) {
        display.clearDisplay();
        display.setTextSize(4);
        display.setCursor(50, 15);
        display.println(random(1, 7));
        display.display();
        delay(50 + (i * 20));
      }
      // Final Number
      int dice = random(1, 7);
      display.clearDisplay();
      display.setTextSize(4);
      display.setCursor(50, 15);
      display.println(dice);
      display.setTextSize(1);
      display.setCursor(0, 55);
      display.println("Result!");
      display.display();
      delay(2000);
    }
  }
}

// 15. Plant Sitter
void p_PlantSitter() {
  while (true) {
    if (readButtons() == 4) return;
    int val = analogRead(SOIL_PIN);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("15. PLANT SITTER");

    // Check if dry (high value)
    if (val > 3500) {
      display.setCursor(0, 25);
      display.println("I need WATER!");
      // Chirp alarm
      digitalWrite(BUZZER, HIGH);
      delay(100);
      digitalWrite(BUZZER, LOW);
    } else {
      display.setCursor(0, 25);
      display.println("I am happy :)");
    }

    drawFooter();
    delay(500);
  }
}




// ---------------- MAIN LOOP ----------------
void loop() {
  drawMenu();

  int input = readButtons();

  // Navigation Logic
  if (input == 1) {  // UP
    currentSelection--;
    if (currentSelection < 0) {
      currentSelection = NUM_PROJECTS - 1;  // Wrap to bottom
      topVisible = NUM_PROJECTS - ITEMS_PER_PAGE;
    } else if (currentSelection < topVisible) {
      topVisible--;  // Scroll Up
    }
  }

  if (input == 2) {  // DOWN
    currentSelection++;
    if (currentSelection >= NUM_PROJECTS) {
      currentSelection = 0;  // Wrap to top
      topVisible = 0;
    } else if (currentSelection >= topVisible + ITEMS_PER_PAGE) {
      topVisible++;  // Scroll Down
    }
  }

  if (input == 3) {  // ENTER (Click)
    display.clearDisplay();
    display.setCursor(20, 20);
    display.println("Opening...");
    display.display();
    delay(300);

    // Launch Project
    switch (currentSelection) {
      case 0: p_TemperatureAlarm(); break;
      case 1: p_CollisionAlarm(); break;
      case 2: p_SmartSound(); break;
      case 3: p_SmartOLED(); break;
      case 4: p_SmartServo(); break;
      case 5: p_SmartColor(); break;
      case 6: p_SmartSoil(); break;
      case 7: p_GasAlarm(); break;
      case 8: p_Traffic(); break;
      case 9: p_SmartLedPattern(); break;
      case 10: p_Game(); break;
      case 11: p_Snake(); break;
      case 12: p_Countdown(); break;
      case 13: p_music(); break;
      case 14: p_Dancing(); break;
      case 15: p_SinglePixel();break;
      case 16: p_MatrixShapes(); break;
      case 17: p_ScrollingText(); break;
      case 18:p_AnimationLoop(); break;
      case 19: p_TempMonitor(); break;
      case 20: p_Distance(); break;
      case 21: p_SmartLight(); break;
      case 22: p_Intruder(); break;
      case 23: p_ServoSweep(); break;
      case 24: p_SolarTracker(); break;
      case 25: p_SoilDoctor(); break;
      case 26: p_GasSafety(); break;
      case 28: p_Parking(); break;
      case 29: p_Theremin(); break;
      case 30: p_Reaction(); break;
      case 31: p_Stopwatch(); break;
      case 32: p_Dice(); break;
      case 33: p_PlantSitter(); break;
 
    }
  }
}