// Photon 2 Garage Parking Assistant
#include "Particle.h"
#include "neopixel.h"


// Pin definitions
#define SERVO_PIN D1
#define TRIG_PIN D2
#define ECHO_PIN D3
#define PIXEL_PIN SPI  


// Distance thresholds (in cm)
#define DISTANCE_MEDIUM 100   // Yellow zone
#define DISTANCE_STOP 20      // Red zone


// Servo configuration
#define SERVO_REST 0
#define SERVO_PRESS 53
#define PRESS_DURATION 500


// NeoPixel configuration
int PIXEL_COUNT = 1;
int PIXEL_TYPE = WS2812;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);


// Timing
#define MEASURE_INTERVAL 200
#define TIMER_WAIT 30000       // 30 seconds


// State variables
unsigned long lastMeasurement = 0;
unsigned long colorChangeTime = 0;
bool timerActive = false;
double currentDistance = 0;
String currentColor = "green";
String lastColor = "green";
bool carPresent = false;


// Create servo object
Servo doorServo;


// Function prototypes
float measureDistance();
void updateLED(float distance);
void pressGarageDoorButton();
int manualCloseDoor(String command);


void setup() {
    Serial.begin(9600);
   
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
   
    doorServo.attach(SERVO_PIN);
    doorServo.write(SERVO_REST);
   
    strip.begin();
    strip.setBrightness(50);
    strip.show();
   
    Particle.variable("distance", currentDistance);
    Particle.variable("ledColor", currentColor);
    Particle.variable("carPresent", carPresent);
    Particle.function("closeDoor", manualCloseDoor);
   
    Serial.println("=== Garage Assistant Ready ===");
   
    // Startup flash
    for(int i = 0; i < 3; i++) {
        strip.setPixelColor(0, strip.Color(255, 255, 255));
        strip.show();
        delay(200);
        strip.setPixelColor(0, strip.Color(0, 0, 0));
        strip.show();
        delay(200);
    }
}


void loop() {
    unsigned long now = millis();
   
    if (now - lastMeasurement >= MEASURE_INTERVAL) {
        lastMeasurement = now;
        currentDistance = measureDistance();
       
        carPresent = (currentDistance > 0 && currentDistance < 300);
       
        updateLED(currentDistance);
       
        // Print status
        Serial.print("Distance: ");
        Serial.print(currentDistance);
        Serial.print(" cm | Color: ");
        Serial.print(currentColor);
        Serial.print(" | Last: ");
        Serial.println(lastColor);
       
        // Check for color changes
        if (currentColor != lastColor) {
            Serial.print(">>> COLOR CHANGED: ");
            Serial.print(lastColor);
            Serial.print(" -> ");
            Serial.println(currentColor);
           
            // APPROACHING: Green->Yellow OR Yellow->Red
            if ((lastColor == "green" && currentColor == "yellow") ||
                (lastColor == "yellow" && currentColor == "red")) {
                Serial.println("*** APPROACHING - STARTING 30 SEC TIMER ***");
                colorChangeTime = now;
                timerActive = true;
            }
           
            // DEPARTING: Red->Yellow OR Yellow->Green
            else if ((lastColor == "red" && currentColor == "yellow") ||
                     (lastColor == "yellow" && currentColor == "green")) {
                Serial.println("*** DEPARTING - STARTING 30 SEC TIMER ***");
                colorChangeTime = now;
                timerActive = true;
            }
           
            lastColor = currentColor;
        }
       
        // Check if timer expired
        if (timerActive && (now - colorChangeTime >= TIMER_WAIT)) {
            Serial.println("*** TIMER EXPIRED - PRESSING BUTTON ***");
            Particle.publish("garage/status", "Timer expired - closing", PRIVATE);
            pressGarageDoorButton();
            timerActive = false;
        }
    }
}


float measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
   
    unsigned long duration = pulseIn(ECHO_PIN, HIGH);
   
    // Timeout check
    if (duration == 0 || duration > 30000) return 0;
   
    float distance = (duration / 2.0) * 0.0343;
    return distance;
}


void updateLED(float distance) {
    uint32_t color;
   
    if (distance == 0 || distance > DISTANCE_MEDIUM) {
        // Out of range or far - Green
        color = strip.Color(255, 0, 0); // GRB format
        currentColor = "green";
    }
    else if (distance > DISTANCE_STOP) {
        // Medium - Yellow
        color = strip.Color(255, 255, 0);
        currentColor = "yellow";
    }
    else {
        // Close - Red
        color = strip.Color(0, 255, 0);
        currentColor = "red";
    }
   
    strip.setPixelColor(0, color);
    strip.show();
}


void pressGarageDoorButton() {
    Serial.println(">>> PRESSING BUTTON NOW <<<");
    Serial.print("Moving servo from ");
    Serial.print(SERVO_REST);
    Serial.print(" to ");
    Serial.println(SERVO_PRESS);
   
    doorServo.write(SERVO_PRESS);
    delay(PRESS_DURATION);
   
    Serial.print("Returning servo to ");
    Serial.println(SERVO_REST);
    doorServo.write(SERVO_REST);
    delay(300);
   
    Serial.println(">>> BUTTON PRESS COMPLETE <<<");
}


int manualCloseDoor(String command) {
    Serial.println("*** MANUAL CLOSE REQUESTED ***");
    pressGarageDoorButton();
    Particle.publish("garage/status", "Manual close", PRIVATE);
    return 1;
}
