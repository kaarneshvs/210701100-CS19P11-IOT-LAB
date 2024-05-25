#include <SPI.h>
#include <MFRC522.h>

#define PIR_SENSOR_PIN 2
#define MICROPHONE_PIN A0  // Analog pin for microphone sensor
#define RFID_SS_PIN 10     // Slave select pin for RFID reader
#define RFID_RST_PIN 9     // Reset pin for RFID reader
#define RED_LIGHT_PIN 5
#define YELLOW_LIGHT_PIN 6
#define GREEN_LIGHT_PIN 7

// Create an instance of the MFRC522 class
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

// Predefined authorized emergency vehicle RFID tags
const byte authorizedTags[][4] = {
  {0x73, 0x83, 0x1C, 0x0E},  // Tag: 73 83 1C 0E
  {0x43, 0xF7, 0x45, 0x25}   // Card: 43 F7 45 25
};

// Variables for RFID tag storage
byte readCard[4];
bool sirenDetected = false;
bool emergencyActive = false;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize PIR sensor pin as input
  pinMode(PIR_SENSOR_PIN, INPUT);

  // Initialize RFID reader
  SPI.begin();
  mfrc522.PCD_Init();

  // Initialize LED pins as outputs for traffic light control
  pinMode(RED_LIGHT_PIN, OUTPUT);
  pinMode(YELLOW_LIGHT_PIN, OUTPUT);
  pinMode(GREEN_LIGHT_PIN, OUTPUT);

  // Print setup initialization message
  Serial.println("Setup initialized.");
}

void loop() {
  // Check for pedestrian presence (PIR sensor)
  int pirReading = digitalRead(PIR_SENSOR_PIN);
  if (pirReading == HIGH) {
    if(emergencyActive==false){
        Serial.println("PIR sensor triggered!");
        pedestrianCrossing();
    }
    else{
      Serial.println("Emergency Vehicle detected. Please Wait!");
    }
    
  }

  // Check for loud noise (microphone sensor)
  int micReading = analogRead(MICROPHONE_PIN);
  if (micReading > 512) {  // Threshold for detecting loud noise, adjust as needed
    Serial.println("Siren sound detected by microphone!");
    sirenDetected = true;
  }

  // Check for RFID tag detection if siren was detected
  if (sirenDetected && isTagDetected()) {
    Serial.println("RFID tag detected!");
    readRFIDTag(readCard);
    if (isAuthorizedTag(readCard)) {
      Serial.println("Authorized RFID tag detected! Activating emergency vehicle passage.");
      emergencyActive = true;
      emergencyVehiclePassage();
      sirenDetected = false;  // Reset siren detection
    } else {
      Serial.println("Unauthorized RFID tag detected!");
    }
  }

  // Default traffic light sequence
  defaultTrafficLightSequence();
}

void pedestrianCrossing() {
  // Turn on green light for pedestrians
  digitalWrite(RED_LIGHT_PIN, LOW);
  digitalWrite(YELLOW_LIGHT_PIN, LOW);
  digitalWrite(GREEN_LIGHT_PIN, HIGH);
  delay(9000); // Adjust pedestrian crossing duration

  // Turn on yellow light for transition
  digitalWrite(GREEN_LIGHT_PIN, LOW);
  digitalWrite(YELLOW_LIGHT_PIN, HIGH);
  delay(3000); // Adjust yellow light duration

  // Turn on red light for vehicles
  digitalWrite(YELLOW_LIGHT_PIN, LOW);
  digitalWrite(RED_LIGHT_PIN, HIGH);
}

void emergencyVehiclePassage() {
  // Turn on yellow light for all other directions
  digitalWrite(RED_LIGHT_PIN, LOW);
  digitalWrite(GREEN_LIGHT_PIN, LOW);
  digitalWrite(YELLOW_LIGHT_PIN, HIGH);
  delay(7000); // Adjust emergency vehicle passage duration

  // Turn on red light for all other directions
  digitalWrite(YELLOW_LIGHT_PIN, LOW);
  digitalWrite(RED_LIGHT_PIN, HIGH);
}

void defaultTrafficLightSequence() {
  // Green light for vehicles
  digitalWrite(RED_LIGHT_PIN, LOW);
  digitalWrite(YELLOW_LIGHT_PIN, LOW);
  digitalWrite(GREEN_LIGHT_PIN, HIGH);
  delay(5000); // Adjust green light duration

  // Yellow light for transition
  digitalWrite(GREEN_LIGHT_PIN, LOW);
  digitalWrite(YELLOW_LIGHT_PIN, HIGH);
  delay(5000); // Adjust yellow light duration

  // Red light for vehicles
  digitalWrite(YELLOW_LIGHT_PIN, LOW);
  digitalWrite(RED_LIGHT_PIN, HIGH);
  delay(5000); // Adjust red light duration
}

bool isTagDetected() {
  // Check for new card
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  return true;
}

void readRFIDTag(byte *tagData) {
  // Copy the UID data to the tagData array
  for (byte i = 0; i < 4; i++) {
    tagData[i] = mfrc522.uid.uidByte[i];
  }

  // Debugging: Print the tag data
  Serial.print("Tag: ");
  for (byte i = 0; i < 4; i++) {
    Serial.print(tagData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();
}

bool isAuthorizedTag(byte *checkTag) {
  // Compare the detected tag ID with authorized tags list
  for (byte i = 0; i < sizeof(authorizedTags) / sizeof(authorizedTags[0]); i++) {
    bool match = true;
    for (byte j = 0; j < 4; j++) {
      if (checkTag[j] != authorizedTags[i][j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return true;
    }
  }
  return false;
}
