#include <WiFi.h>
#include <esp_now.h>
#include <HTTPClient.h> // To make HTTP requests

// Replace with your network credentials
const char* ssid = "Belong0712D6";
const char* password = "3gvvajdcj3nk62bn";

// Define the server URL (replace with your server's IP and port)
const char* serverURL = "http://10.0.0.20:3000/esp-status"; // Update with your server's IP

uint8_t slaveAddress[] = {0xd0, 0xef, 0x76, 0x46, 0xb4, 0xf4};

// Structure to hold the message
typedef struct struct_message {
  char message[32];
} struct_message;

// Create a message instance
struct_message myData;

// Declare timer for periodic status update
unsigned long timer;

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  char receivedMessage[len + 1];
  memcpy(receivedMessage, incomingData, len);
  receivedMessage[len] = '\0';  // Null-terminate the string

  Serial.print("Received message: ");
  Serial.println(receivedMessage);
}

void sendPeriodicStatus() {
    Serial.println("Sending periodic status update...");
    updateESPStatus(true); // Sending connected status periodically
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Fail");
  }
}

// Function to send HTTP POST request to update ESP status
void updateESPStatus(bool connected) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot update ESP status.");
    return;
  }

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  String jsonPayload = "{\"connected\":";
  jsonPayload += (connected) ? "true" : "false";
  jsonPayload += "}";

  // Send POST request
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// Function to send movement commands via ESP-NOW
void sendMoveCommand(const char* direction, int steps) {
  struct_message cmd;
  strcpy(cmd.message, direction);

  // Send the direction first
  esp_now_send(slaveAddress, (uint8_t *) &cmd, sizeof(cmd));

  delay(100); // Small delay to ensure messages are sent in order

  // Send the steps as a separate message
  String stepsStr = String(steps);
  strcpy(cmd.message, stepsStr.c_str());
  esp_now_send(slaveAddress, (uint8_t *) &cmd, sizeof(cmd));

  Serial.printf("Sent Move Command: %s, Steps: %d\n", direction, steps);
}

// Define Joystick Pins
#define JOY_X 34 // Connect the X-axis of the joystick to pin 34 (GPIO34)

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open
  Serial.println("Master ESP32 Started.");

  // Initialize Joystick Pin
  pinMode(JOY_X, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Update ESP status as connected
  updateESPStatus(true);

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW Initialized");

  // Register send and receive callbacks
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Add Slave as Peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add Slave as Peer");
    return;
  }
  Serial.println("Slave Added as Peer Successfully");

  // Send initial message to Slave
  sendMoveCommand("Hi Slave!", 0);

  // Initialize the timer
  timer = millis();
}

void loop() {
  // Periodically send status every 5 seconds
  if (millis() - timer >= 5000) {
    Serial.println("5 seconds elapsed. Sending periodic status...");
    sendPeriodicStatus();
    timer = millis(); // Reset the timer
  }

  int joystickValue = analogRead(JOY_X); // Read the joystick X value

  // Define thresholds for movement (calibrate based on joystick)
  const int threshold = 1000; // Adjust based on joystick sensitivity

  if (joystickValue > (4095 - threshold)) { // Joystick pushed to the right (forward)
    Serial.println("Joystick moved Forward");
    sendMoveCommand("Forward", 10); // Adjust steps as needed
    delay(500); // Debounce delay
  }
  if (joystickValue < threshold) { // Joystick pushed to the left (reverse)
    Serial.println("Joystick moved Reverse");
    sendMoveCommand("Backward", 10); // Adjust steps as needed
    delay(500); // Debounce delay
  }

  // Small delay to avoid rapid motor steps
  delay(50); // Adjust as needed
}

