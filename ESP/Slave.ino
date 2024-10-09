#include <WiFi.h>
#include <esp_now.h>

// Define the control pins
#define IN1 23
#define IN2 22
#define IN3 21
#define IN4 19

// Replace with the new Master ESP32's MAC address
uint8_t masterAddress[] = {0x08, 0xa6, 0xf7, 0xa0, 0x5a, 0x70};  // Updated MAC address

// Structure to hold the message
typedef struct struct_message {
  char message[32];
} struct_message;

// Create a message instance
struct_message myData;

String task;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Fail");
  }
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  char receivedMessage[len + 1];
  memcpy(receivedMessage, incomingData, len);
  receivedMessage[len] = '\0';  // Null-terminate the string

  Serial.print("Received message: ");
  Serial.println(receivedMessage);
  
  if (strcmp(receivedMessage, "Forward") == 0) {
    task = "Forward";
  }
  else if (strcmp(receivedMessage, "Backward") == 0) {
    task = "Backward";
  }
  else {
    task = "Stop";
  }
}

void SendMessage(char message[32]){

  // Prepare the message
  strcpy(myData.message, message);

  // Send the message once
  Serial.println("Attempting to send message to Slave...");
  esp_now_send(masterAddress, (uint8_t *) &myData, sizeof(myData));
  Serial.println("Message sent!");
}

// Set desired RPM
float desiredRPM = 58; // Change this to your desired RPM

void setup() {

  // Set the control pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);  // Wait for Serial Monitor to open
  Serial.println("Slave ESP32 Started.");

  // Initialize WiFi in Station mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized.");

  // Register the send callback
  esp_now_register_send_cb(OnDataSent);

  // Register the receive callback function
  esp_now_register_recv_cb(OnDataRecv);

  // Define the peer (Master)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, masterAddress, 6);  // Copy the new Master's MAC address
  peerInfo.channel = 0;  // Use current channel
  peerInfo.encrypt = false;  // No encryption

  // Add the peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Peer (Master) added successfully.");

  SendMessage("Hi slave!");



  // // Prepare the message
  // strcpy(myData.message, "Hi Master");

  // // Send the message once
  // Serial.println("Attempting to send message to Master...");
  // esp_now_send(masterAddress, (uint8_t *) &myData, sizeof(myData));
  // Serial.println("Message sent!");

  // Serial.println("Slave ESP32 is ready to receive messages.");
}

void loop() {
  // No loop is needed since we send the message only once
  // Determine motor direction based on joystick position

  if (task == "Forward") { // Joystick pushed right
    rotateMotor(1); // Rotate clockwise
  } 
  else if (task == "Backward") { // Joystick pushed left
    rotateMotor(-1); // Rotate counterclockwise
  }
  else {
    // If joystick is in neutral position, stop the motor
    stopMotor(); // Stop the motor
  }

  // Small delay to avoid rapid motor steps
  delay(50); // Adjust as needed

}

// Function to rotate the motor
void rotateMotor(int direction) {
  for (int i = 0; i < 512; i++) { // Adjust number of steps as needed
    if (direction == 1) {
      stepMotor(i % 4); // Clockwise
    } else {
      stepMotor(3 - (i % 4)); // Counterclockwise
    }
    delay(calculateDelay(desiredRPM)); // Delay based on RPM
  }
}

// Function to stop the motor
void stopMotor() {
  // Set all control pins to LOW to stop the motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// Function to control the stepper motor
void stepMotor(int step) {
  switch (step) {
    case 0:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 1:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      break;
    case 2:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      break;
    case 3:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      break;
  }
}

// Function to calculate the delay based on desired RPM
int calculateDelay(float rpm) {
  // Steps per revolution for the 28BYJ-48 (2048 steps for full rotation)
  int stepsPerRevolution = 512; 
  float secondsPerRevolution = 60.0 / rpm; // Time for one revolution in seconds
  float delayPerStep = (secondsPerRevolution / stepsPerRevolution) * 1000; // Convert to milliseconds
  return (int)delayPerStep; // Return as integer
}
