#include <WiFi.h>
#include <esp_now.h>

uint8_t slaveAddress[] = {0xd0, 0xef, 0x76, 0x46, 0xb4, 0xf4};

// Structure to hold the message
typedef struct struct_message {
  char message[32];
} struct_message;

// Create a message instance
struct_message myData;

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  char receivedMessage[len + 1];
  memcpy(receivedMessage, incomingData, len);
  receivedMessage[len] = '\0';  // Null-terminate the string

  Serial.print("Received message: ");
  Serial.println(receivedMessage);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Fail");
  }
}

void SendMessage(char message[32]){

  // Prepare the message
  strcpy(myData.message, message);

  // Send the message once
  Serial.println("Attempting to send message to Slave...");
  esp_now_send(slaveAddress, (uint8_t *) &myData, sizeof(myData));
  Serial.println("Message sent!");
}

#define JOY_X 34 // Connect the X-axis of the joystick to pin 34 (GPIO34)

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open
  Serial.println("Master ESP32 Started.");

  // Set joystick pin as input
  pinMode(JOY_X, INPUT);

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
  memcpy(peerInfo.peer_addr, slaveAddress, 6);  // Copy the new Master's MAC address
  peerInfo.channel = 0;  // Use current channel
  peerInfo.encrypt = false;  // No encryption

  // Add the peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Peer (Slave) added successfully.");

  // Prepare the message
  //strcpy(myData.message, "Hi Slave");

  SendMessage("Hi slave!");

  // Send the message once
  // Serial.println("Attempting to send message to Slave...");
  // esp_now_send(slaveAddress, (uint8_t *) &myData, sizeof(myData));
  // Serial.println("Message sent!");

  // Serial.println("Master ESP32 is ready to receive messages.");
}

void loop() {
  int joystickValue = analogRead(JOY_X); // Read the joystick X value

  // Determine motor direction based on joystick position
  if (joystickValue > 4090) { // Joystick pushed right
    SendMessage("Forward"); // Rotate clockwise
  } 
  else if (joystickValue < 10) { // Joystick pushed left
    SendMessage("Backward"); // Rotate counterclockwise
  }
  else {
    // If joystick is in neutral position, stop the motor
    SendMessage("Stop"); // Stop the motor
  }

  // Small delay to avoid rapid motor steps
  delay(50); // Adjust as needed
}