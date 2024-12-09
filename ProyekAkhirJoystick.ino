#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
// Ganti dengan informasi WiFi Anda
#define WIFI_SSID "Pikahotarou"
#define WIFI_PASSWORD "Ehdiagendutloh123"

// Ganti dengan informasi Firebase Anda
#define FIREBASE_HOST "https://proyekakhiriot-default-rtdb.asia-southeast1.firebasedatabase.app/"  // URL Firebase Database
#define FIREBASE_AUTH "UwcukjWpmUEP3rMOpQjOSeSRlc2yf8bbRcgskM59"             // Secret Key Firebase

// Pin definitions
#define VRX_PIN     33    // Pin untuk sumbu X joystick
#define VRY_PIN     32    // Pin untuk sumbu Y joystick
#define BUTTON_PIN  25    // Pin untuk tombol joystick

// Firebase dan objek WiFi
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Global variables
int xValue = 0;       // Menyimpan nilai sumbu X
int yValue = 0;       // Menyimpan nilai sumbu Y
int buttonState = 1;  // Menyimpan status tombol

// Function prototypes
void TaskReadJoystick(void *pvParameters);
void TaskSendToFirebase(void *pvParameters);

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP); 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Menghubungkan ke WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung ke WiFi!");

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Terhubung ke Firebase!");

  xTaskCreatePinnedToCore(
    TaskReadJoystick,   
    "Read Joystick",    
    1024,              
    NULL,               
    1,                 
    NULL,               
    0
  );

  xTaskCreatePinnedToCore(
    TaskSendToFirebase, 
    "SendFirebase", 
    8192, 
    NULL, 
    1, 
    NULL, 
    1)
    ;
}

void loop() {
  vTaskDelay(portMAX_DELAY); 
}

void TaskReadJoystick(void *pvParameters) {
  (void) pvParameters; 
  while (1) {
    xValue = analogRead(VRX_PIN); 
    yValue = analogRead(VRY_PIN);
    buttonState = digitalRead(BUTTON_PIN); 

    Serial.print("X = ");
    Serial.print(xValue);
    Serial.print(", Y = ");
    Serial.println(yValue);

    Serial.print("Button = ");
    if (buttonState == LOW) {
      Serial.println("PRESSED");
    } else {
      Serial.println("NOT PRESSED");
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); 
  }
}

void TaskSendToFirebase(void *pvParameters) {
  while (1) {
    if (Firebase.ready()) {

      if (Firebase.setInt(firebaseData, "/joystick/X", xValue)) {
        Serial.println("X value dikirim ke Firebase.");
      } else {
        Serial.print("Gagal mengirim X: ");
        Serial.println(firebaseData.errorReason());
      }

      if (Firebase.setInt(firebaseData, "/joystick/Y", yValue)) {
        Serial.println("Y value dikirim ke Firebase.");
      } else {
        Serial.print("Gagal mengirim Y: ");
        Serial.println(firebaseData.errorReason());
      }

      String buttonStatus = (buttonState == LOW) ? "PRESSED" : "NOT PRESSED";
      if (Firebase.setString(firebaseData, "/joystick/Button", buttonStatus)) {
        Serial.println("Button state dikirim ke Firebase.");
      } else {
        Serial.print("Gagal mengirim Button: ");
        Serial.println(firebaseData.errorReason());
      }
    } else {
      Serial.println("Firebase belum siap!");
    }

    vTaskDelay(500/ portTICK_PERIOD_MS);  
  }
}
