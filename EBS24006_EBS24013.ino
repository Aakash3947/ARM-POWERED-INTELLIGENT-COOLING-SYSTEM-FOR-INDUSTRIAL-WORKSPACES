#include <DHT.h>
// Pin Definitions
#define DHTPIN 2             // DHT11 Data pin connected to STM32 pin 2
#define PIR_PIN 3            // PIR sensor pin connected to STM32 pin 3
#define TRIG_PIN 5           // Ultrasonic sensor TRIG pin connected to STM32 pin 5
#define ECHO_PIN 6           // Ultrasonic sensor ECHO pin connected to STM32 pin 6
#define MOTOR_IN1 7          // Motor driver IN1 pin connected to STM32 pin 7
#define MOTOR_IN2 8          // Motor driver IN2 pin connected to STM32 pin 8
#define MQ7_PIN A0           // MQ-7 CO sensor analog pin connected to STM32 A0
#define BUZZER_PIN 9         // Buzzer connected to STM32 pin 9
// DHT11 Setup
#define DHTTYPE DHT11        // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);    // DHT object for temperature and humidity
// Variables
int peopleCount = 0;         // Stores number of people detected
float temperature = 0;       // Stores temperature value
long distance = 0;           // Stores distance value from ultrasonic sensor
unsigned long lastDetectionTime = 0; // Stores last time a person was detected
const unsigned long detectionTimeout = 0.2 * 60 * 1000; // 10 minutes in milliseconds
int coLevel = 0;             // Stores CO level
const int coThreshold = 350; // Threshold for CO level to trigger the alarm
// Function to get distance from Ultrasonic sensor
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Set timeout to avoid infinite wait
  if (duration == 0) {
    return -1; // Indicates out of range or no object detected
  }
  long distance = duration * 0.034 / 2;
  return distance;
}
// Function to control the fan based on temperature and people count
void controlFan() {
  // Read temperature from DHT11
  temperature = dht.readTemperature();
  // Check distance from Ultrasonic Sensor
  distance = getDistance();
  if (distance != -1 && distance < 30) { // Only stop motor if distance is valid and less than 30 cm
    digitalWrite(MOTOR_IN1, LOW); // Turn OFF the fan
    digitalWrite(MOTOR_IN2, LOW);
    return; // Exit the function early
  }
  // Check if peopleCount is zero; if so, turn off the fan regardless of temperature
  if (peopleCount == 0) {
    digitalWrite(MOTOR_IN1, LOW); // Turn OFF the fan
    digitalWrite(MOTOR_IN2, LOW);
    return;
  }
  // Turn the fan on if temperature is greater than 25°C and people are detected
  if (temperature > 25 && peopleCount > 0) {
    digitalWrite(MOTOR_IN1, HIGH); // Turn ON the fan
    digitalWrite(MOTOR_IN2, LOW);
  } else {
    digitalWrite(MOTOR_IN1, LOW); // Turn OFF the fan
    digitalWrite(MOTOR_IN2, LOW);
  }
}
// Function to update the people count based on PIR sensor
void updatePeopleCount() {
  int pirState = digitalRead(PIR_PIN);
  // Update peopleCount and reset timer if person detected
  if (pirState == HIGH) {
    peopleCount = 1;
    lastDetectionTime = millis(); // Update last detection time
  } else if (millis() - lastDetectionTime > detectionTimeout) {
    peopleCount = 0; // No person detected within 10 minutes, set peopleCount to 0
  }
}
// Function to check CO levels and activate buzzer if level is high
void checkCOLevel() {
  coLevel = analogRead(MQ7_PIN); // Read CO level from MQ-7
  if (coLevel > coThreshold) {
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
  }
}
void setup() {
  // Initialize pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  // Start DHT11
  dht.begin();
  // Initialize Motor (set direction)
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  // Initialize Serial Monitor
  Serial.begin(9600);
}
void loop() {
  // Update people count
  updatePeopleCount();
  // Control the fan based on conditions
  controlFan();
  // Check CO level and activate buzzer if needed
  checkCOLevel();
  // Print sensor data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("C, People Count: ");
  Serial.print(peopleCount);
  Serial.print(", Distance: ");
  if (distance == -1) {
    Serial.print("Out of range or no object detected");
  } else {
    Serial.print(distance);
    Serial.print(" cm");
  }
  Serial.print(", CO Level: ");
  Serial.print(coLevel);
  Serial.println();
  // Small delay to stabilize sensor readings
  delay(500);
}
