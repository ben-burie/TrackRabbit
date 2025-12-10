#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFPlayerMini_Fast.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initializing the DFPlayerMini for audio feedback
SoftwareSerial mySerial(3, 2); 
DFPlayerMini_Fast audioFeedback;
const int VOLUME = 25;
const int AUDIO_ON_PACE = 21; // File for on pace
const int AUDIO_AHEAD_BASE = 1;      // Files 1-10 (ahead of pace)
const int AUDIO_BEHIND_BASE = 11;    // Files 11-20 (behind pace)
const int MAX_FEEDBACK_SECONDS = 10;

const int trigPin = 9;
const int echoPin = 10;
const int piezoPin = 5;

const unsigned long motionDebounce = 1000;

unsigned long goalTimeSeconds = 0;
unsigned long timerStartTime = 0;
unsigned long lastMotionTime = 0;
unsigned long passCount = 0;

bool timerRunning = false;
bool timerStarted = false;
bool inputMode = true;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(piezoPin, OUTPUT);

  // DFPlayer Mini initialization
  mySerial.begin(9600);
  delay(500);
  audioFeedback.begin(mySerial);
  delay(1000);  // Wait for DFPlayer to initialize
  audioFeedback.volume(VOLUME);
  
  Serial.println("System initialized");
  Serial.println("DFPlayer Mini initialized");

  // LCD initialization
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Enter Goal Time");
  lcd.setCursor(0, 1);
  lcd.print("In Seconds");

  // Prompt user to enter goal pace per lap
  Serial.println("Enter goal time (seconds per lap):");

  // Wait for user input
  while (inputMode) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();

      goalTimeSeconds = input.toInt();

      if (goalTimeSeconds > 0) {
        inputMode = false;
        timerStartTime = millis();
        timerRunning = true;
        timerStarted = false;
        passCount = 0;
        lastMotionTime = 0;

        Serial.print("Goal time set to: ");
        Serial.print(goalTimeSeconds);
        Serial.println(" seconds per lap");
        Serial.println("Waiting for first pass to start timer...");

        lcd.clear();
        delay(500);
      }
      else {
        Serial.println("Invalid input. Please enter a positive number.");
        lcd.setCursor(0,1);
        lcd.print("Invalid! Try again.");
      }
    }
  }
}

void loop() {
  if (!timerRunning) return;

  // Calculate elapsed time (only if timer has started)
  unsigned long elapsedTime = 0;
  if (timerStarted) {
    elapsedTime = (millis() - timerStartTime) / 1000;
    static bool firstTimerDisplay = true;
    if (firstTimerDisplay) {
      lcd.clear();
      firstTimerDisplay = false;
    }
    displayTimer(elapsedTime);
  }
  else {
    // Waiting for first pass
    lcd.setCursor(0, 0);
    lcd.print("Waiting for 1st ");
    lcd.setCursor(0, 1);
    lcd.print("pass to start...  ");
  }

  // Send trigger pulse to ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo time
  long duration = pulseIn(echoPin, HIGH, 30000);

  float distance = duration * 0.034 / 2;

  // Check if object closer than 60cm = motion detected
  if (distance < 60 && distance > 5) {

    // Oonly alert if enough time has passed since last motion
    if (millis() - lastMotionTime > motionDebounce) {
      lastMotionTime = millis();

      tone(piezoPin, 200, 100);
      delay(150);

      // Timer starts after first pass
      if (!timerStarted) {
        timerStartTime = millis();
        timerStarted = true;
        Serial.println("Timer started on first pass!");
        Serial.println("First pass will not be counted.");
      }
      else {
        // Counting passes after timer has already begun running
        passCount++;

        tone(piezoPin, 200);
        delay(100);
        noTone(piezoPin);

        unsigned long elapsedTime = (millis() - timerStartTime) / 1000;
        unsigned long expectedCumulativeTime = goalTimeSeconds * passCount;
        
        // Calculate difference in elapsed vs. expected time (negative = ahead, positive = behind)
        long diff = elapsedTime - expectedCumulativeTime;

        Serial.print("Pass ");
        Serial.print(passCount);
        Serial.print(" (");
        Serial.print(elapsedTime);
        Serial.print("s) - ");
        
        if (diff > 0) {
          // Behind pace
          Serial.print("You are ");
          Serial.print(diff);
          Serial.println(" seconds behind pace.");
          playAudioFeedback("behind", diff);
        }
        else if (diff < 0) {
          // Ahead of pace
          Serial.print("You are ");
          Serial.print(-diff);
          Serial.println(" seconds ahead of pace.");
          playAudioFeedback("ahead", diff);
        }
        else {
          // On pace
          Serial.println("You are right on pace.");
          playAudioFeedback("onpace", 0);
        }
        delay(100);
      }
    }
  }
  else {
    noTone(piezoPin);
  }
  delay(50); // 50 millisecond delay between readings
}

void displayTimer(unsigned long seconds) {
  unsigned long minutes = seconds / 60;
  unsigned long secs = seconds % 60;
  
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  
  if (secs < 10) lcd.print("0");
  lcd.print(secs);
  
  lcd.print(" L:");
  lcd.print(passCount);
}

void playAudioFeedback(String status, long secondsDifference) {
  int fileIndex = 0;

  if (status == "ahead") {
    int absoluteDiff = abs(secondsDifference);

    if (absoluteDiff > MAX_FEEDBACK_SECONDS) {
      //Play way ahead audio - File 22
      absoluteDiff = MAX_FEEDBACK_SECONDS;
      fileIndex = 22;
    }
    else {
      fileIndex = AUDIO_AHEAD_BASE + absoluteDiff - 1; // Files 1-10
    }
  }
  else if (status == "behind") {
    int absoluteDiff = abs(secondsDifference);

    if (absoluteDiff > MAX_FEEDBACK_SECONDS) {
      //Play way ahead audio - File 23
      absoluteDiff = MAX_FEEDBACK_SECONDS;
      fileIndex = 23;
    }
    else {
      fileIndex = AUDIO_BEHIND_BASE + absoluteDiff - 1; // Files 11-20
    }
  }
  else if (status == "onpace") {
    fileIndex = AUDIO_ON_PACE;
    Serial.println("Playing file.");
  }

  if (fileIndex > 0) {
    Serial.print("Playing audio file: ");
    Serial.println(fileIndex);

    audioFeedback.stop();
    delay(100);

    audioFeedback.volume(VOLUME);
    delay(50);
    audioFeedback.play(fileIndex);
    Serial.print("File ");
    Serial.print(fileIndex);
    Serial.println(" queued for playback");
    delay(2000);
  }
}