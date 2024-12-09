#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <FirebaseESP32.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <WiFiManager.h>

#define FIREBASE_HOST ""  // URL Firebase Database
#define FIREBASE_AUTH ""             // Secret Key Firebase

// Initialize Telegram BOT
String BOTtoken = "";
String CHAT_ID = "";

bool sendPhoto = false;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
#define SERVO_1      14
#define SERVO_2      15
bool flashState = LOW;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

TaskHandle_t TaskCamHandle = NULL;
TaskHandle_t TaskFirebaseHandle = NULL;

WebServer server(80);

Servo servo1;
Servo servo2;

int servo1Pos = 90;
int servo2Pos = 90;

// Firebase dan objek WiFi
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}

void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome , " + from_name + "\n";
      welcome += "Use the following commands to interact with the ESP32-CAM \n";
      welcome += "/photo : takes a new photo\n";
      welcome += "/flash : toggles flash LED \n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      Serial.println("Change flash LED state");
    }
    if (text == "/photo") {
      sendPhoto = true;
            Serial.println("New photo request");
    }
  }
}

String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String head = "--electroniclinic\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--electroniclinic\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--electroniclinic--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=electroniclinic");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
   
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
  
}

const char HEADER[] = 
    "HTTP/1.1 200 OK\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";

void handleNotFound() {
    String message = "Server is running!\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    server.send(200, "text/plain", message);
}

void handle_jpg_stream() {
    char buf[32];
    int s;

    WiFiClient client = server.client();
    client.write(HEADER, strlen(HEADER));
    client.write(BOUNDARY, strlen(BOUNDARY));

    while (client.connected()) {  
        if (!client.connected()) break;

        camera_fb_t *fb = esp_camera_fb_get(); 
        if (!fb) {
            Serial.println("Camera capture failed");
            break;
        }

        s = fb->len;
        client.write(CTNTTYPE, strlen(CTNTTYPE));
        sprintf(buf, "%d\r\n\r\n", s);
        client.write(buf, strlen(buf));
        client.write((char *)fb->buf, s);
        client.write(BOUNDARY, strlen(BOUNDARY));
        esp_camera_fb_return(fb); 
    }
}

const char JHEADER[] = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Disposition: inline; filename=capture.jpg\r\n"
    "Content-Type: image/jpeg\r\n\r\n";

void handle_jpg() {
    WiFiClient client = server.client();
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    client.write(JHEADER, strlen(JHEADER));
    client.write((char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
}

void updateServoPositions(int x, int y) {
  const int THRESHOLD_HIGH = 3072;
  const int THRESHOLD_LOW = 1024;
  const int STEP = 10;

  // Servo 2 (Horizontal Movement)
  if (x > THRESHOLD_HIGH && y > THRESHOLD_HIGH) {
    servo2Pos = constrain(servo2Pos + STEP, 0, 180);
  } else if (x < THRESHOLD_LOW && y < THRESHOLD_LOW) {
    servo2Pos = constrain(servo2Pos - STEP, 0, 180);
  }

  // Servo 1 (Vertical Movement)
  if (x < THRESHOLD_LOW && y > THRESHOLD_HIGH) {
    servo1Pos = constrain(servo1Pos + STEP, 0, 180);
  } else if (x > THRESHOLD_HIGH && y < THRESHOLD_LOW) {
    servo1Pos = constrain(servo1Pos - STEP, 0, 180);
  }

  servo1.write(servo1Pos);
  servo2.write(servo2Pos);
}

void TaskCam(void *pvParameters){
  while(1){
    if (sendPhoto) {
    Serial.println("Preparing photo");
    sendPhotoTelegram(); 
    sendPhoto = false; 
    
  }
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
   vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void TaskFirebase(void *pvParameters) {
  int reconnectAttempts = 0;
  const int MAX_RECONNECT_ATTEMPTS = 5;

  while(1) {
    if (!Firebase.ready()) {
      Serial.println("Firebase not ready. Attempting to reconnect...");
      Firebase.reconnectWiFi(true);
      
      reconnectAttempts++;
      if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        Serial.println("Failed to reconnect to Firebase. Restarting...");
        ESP.restart();
      }
      
      delay(2000); 
      continue;
    }

    reconnectAttempts = 0; 

    int xValue = 0, yValue = 0, button = 0; 
    
    if (!Firebase.getInt(firebaseData, "/joystick/X")) {
      Serial.println("Firebase X read error: " + firebaseData.errorReason());
    } else {
      xValue = firebaseData.intData();
    }

    if (!Firebase.getInt(firebaseData, "/joystick/Y")) {
      Serial.println("Firebase Y read error: " + firebaseData.errorReason());
    } else {
      yValue = firebaseData.intData();
    }

    if (!Firebase.getInt(firebaseData, "/joystick/Button")) {
      Serial.println("Firebase Button read error: " + firebaseData.errorReason());
    } else {
      button = firebaseData.intData();
      if (button == 1) {
        delay(200);
        sendPhoto = true;
      }
    }
    
    updateServoPositions(xValue, yValue);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup(){
  WiFiManager wm;
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  if (!psramFound()) {
  Serial.println("PSRAM not found. Ensure it is enabled in your board configuration.");
  while (true); 
  }

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  configInitCamera();

  if (!wm.autoConnect("ESP-CAM")) {
        Serial.println("Gagal menghubungkan ke WiFi, restart ESP...");
        ESP.restart();
    }
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP()); 

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  
  servo1.attach(SERVO_1, 1000, 2000);
  servo2.attach(SERVO_2, 1000, 2000);

  servo1.write(servo1Pos);
  servo2.write(servo2Pos);

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Terhubung ke Firebase!");

  server.on("/mjpeg/1", HTTP_GET, handle_jpg_stream);
  server.on("/jpg", HTTP_GET, handle_jpg);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  xTaskCreatePinnedToCore(
    TaskCam,           
    "CamTask",         
    8192,              
    NULL,              
    1,                 
    &TaskCamHandle,    
    0                  
  );

    xTaskCreatePinnedToCore(
    TaskFirebase,           
    "FirebaseTask",         
    8192,              
    NULL,              
    1,                 
    &TaskFirebaseHandle,    
    1                  
  );
}

void loop() {
  server.handleClient();
}