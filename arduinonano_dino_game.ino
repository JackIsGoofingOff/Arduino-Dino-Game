#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTTON_PIN 6

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Custom character definitions
byte dino_l[8] = {
  B00000111,
  B00000101,
  B00000111,
  B00010110,
  B00011111,
  B00011110,
  B00001110,
  B00000100
};

byte dino_r[8] = {
  B00000111,
  B00000101,
  B00000111,
  B00010110,
  B00011111,
  B00011110,
  B00001110,
  B00000010
};

byte cactus_small[8] = {
  B00000100,
  B00000101,
  B00010101,
  B00010101,
  B00010111,
  B00011100,
  B00000100,
  B00000000
};

byte cactus_big[8] = {
  B00000000,
  B00000100,
  B00000101,
  B00010101,
  B00010110,
  B00001100,
  B00000100,
  B00000100
};

// Game constants
const int GROUND_ROW = 3;
const int DINO_COL = 1;
const int DINO_JUMP_ROW = 1;
const int MAX_OBSTACLES = 3;
const int JUMP_DURATION = 500;
const int OBSTACLE_SPAWN_INTERVAL = 1500;

// Game variables
int dinoPos = GROUND_ROW;
bool isJumping = false;
unsigned long jumpStartTime = 0;
int obstaclePos[MAX_OBSTACLES] = { -1, -1, -1 };
int obstacleType[MAX_OBSTACLES] = { 0, 0, 0 };
unsigned long lastObstacleTime = 0;
int score = 0;
int lastScore = -1;
int gameSpeed = 500;
bool gameOver = false;
int prevDinoPos = GROUND_ROW;
int prevObstaclePos[MAX_OBSTACLES] = { -1, -1, -1 };


void resetGame() {
  dinoPos = GROUND_ROW;
  prevDinoPos = GROUND_ROW;
  isJumping = false;
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstaclePos[i] = -1;
    prevObstaclePos[i] = -1;
    obstacleType[i] = 0;
  }
  score = 0;
  lastScore = -1;
  gameSpeed = 500;
  gameOver = false;

  // Clear only dynamic areas
  lcd.setCursor(0, 0);
  lcd.print("Score:          ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 2);
  lcd.print("                ");
  drawGround();
}

void drawGround() {
  // Draw ground only once
  lcd.setCursor(0, GROUND_ROW);
  for (int i = 0; i < 20; i++) {
    lcd.print('_');
  }
}

void updateDino() {
  if (isJumping) {
    unsigned long jumpTime = millis() - jumpStartTime;
    if (jumpTime < JUMP_DURATION / 2) {
      dinoPos = DINO_JUMP_ROW;
    } else if (jumpTime < JUMP_DURATION) {
      dinoPos = GROUND_ROW - 1;
    } else {
      dinoPos = GROUND_ROW;
      isJumping = false;
    }
  }
}

void spawnObstacle() {
  if (millis() - lastObstacleTime > OBSTACLE_SPAWN_INTERVAL) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      if (obstaclePos[i] < 0) {
        obstaclePos[i] = 19;
        obstacleType[i] = random(1, 3);
        lastObstacleTime = millis();
        break;
      }
    }
  }
}

void updateObstacles() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstaclePos[i] >= 0) {
      obstaclePos[i]--;

      if (obstaclePos[i] < 0) {
        obstaclePos[i] = -1;
        obstacleType[i] = 0;
        score++;
        if (score % 5 == 0 && gameSpeed > 200) {
          gameSpeed -= 50;
        }
      }

      if (obstaclePos[i] == DINO_COL && dinoPos >= GROUND_ROW - 1) {
        gameOver = true;
      }
    }
  }
}

void drawDino() {
  // Only redraw if position changed
  if (dinoPos != prevDinoPos) {
    // Clear previous position
    lcd.setCursor(DINO_COL, prevDinoPos);
    lcd.print(' ');
    prevDinoPos = dinoPos;
  }

  // Draw dino
  lcd.setCursor(DINO_COL, dinoPos);
  if (isJumping) {
    lcd.write(1);  // Jumping pose
  } else {
    // Walking animation
    lcd.write((millis() / 200) % 2 == 0 ? 0 : 1);
  }
}

void drawObstacles() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    // Only update if position changed
    if (obstaclePos[i] != prevObstaclePos[i]) {
      // Clear previous obstacle position
      if (prevObstaclePos[i] >= 0 && prevObstaclePos[i] < 20) {
        lcd.setCursor(prevObstaclePos[i], GROUND_ROW);
        lcd.print('_');
      }

      // Draw new obstacle position
      if (obstaclePos[i] >= 0 && obstaclePos[i] < 20) {
        lcd.setCursor(obstaclePos[i], GROUND_ROW);
        lcd.write(obstacleType[i] + 1);
      }

      prevObstaclePos[i] = obstaclePos[i];
    }
  }
}

void drawScore() {
  // Only update when score changes
  if (score != lastScore) {
    lcd.setCursor(7, 0);
    lcd.print("    ");
    lcd.setCursor(7, 0);
    lcd.print(score);
    lastScore = score;
  }
}

void drawGame() {
  drawScore();
  drawDino();
  drawObstacles();
}

void drawGameOver() {
  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("GAME OVER");
  lcd.setCursor(3, 2);
  lcd.print("Score: ");
  lcd.print(score);
  lcd.setCursor(1, 3);
  lcd.print("PRESS TO RESTART");
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, dino_l);
  lcd.createChar(1, dino_r);
  lcd.createChar(2, cactus_small);
  lcd.createChar(3, cactus_big);
  lcd.setCursor(0, 0);
  lcd.print("Score: 0");
  randomSeed(analogRead(0)); // create random seed
  drawGround();
  // reset game
  resetGame();
}

void loop() {
  // game loop
  if (!gameOver) {
    if (digitalRead(BUTTON_PIN) == LOW && !isJumping && dinoPos == GROUND_ROW) {
      isJumping = true;
      jumpStartTime = millis();
      delay(50);
    }
    updateDino();
    spawnObstacle();
    updateObstacles();
    drawGame();
    delay(gameSpeed / 2);
  } else {
    drawGameOver();
    while (digitalRead(BUTTON_PIN) == HIGH) {
      ;
    }
    resetGame();
  }
}