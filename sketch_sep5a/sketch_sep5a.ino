#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>


const char* ssid = "Chamudi's S25 Ultra";
const char* password = "Chamudi19056@";


String BOT_TOKEN = "8816428356:AAEOuOPqnK5tBkR71oxypr-YVmZdAH1s7jQ";
String CHAT_ID = "8939607037";


WiFiClientSecure clientTCP;


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


bool setupCamera() {
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

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

 
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  Serial.println("Initializing camera...");

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  Serial.println("Camera initialized successfully!");
  return true;
}


void sendTelegramMessage(String message) {
  message.replace(" ", "%20");
  message.replace("\n", "%0A");

  String url = "/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + message;

  if (clientTCP.connect("api.telegram.org", 443)) {
    clientTCP.println("GET " + url + " HTTP/1.1");
    clientTCP.println("Host: api.telegram.org");
    clientTCP.println("Connection: close");
    clientTCP.println();

    Serial.println("Telegram text message sent.");
  } else {
    Serial.println("Telegram text message failed.");
  }

  clientTCP.stop();
}


String sendPhotoTelegram() {
  const char* telegramServer = "api.telegram.org";

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }

  Serial.println("Photo captured");
  Serial.print("Photo size: ");
  Serial.println(fb->len);

  if (clientTCP.connect(telegramServer, 443)) {
    Serial.println("Connected to Telegram server");

    String head = "--EchoPawBoundary\r\n";
    head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
    head += CHAT_ID;
    head += "\r\n--EchoPawBoundary\r\n";
    head += "Content-Disposition: form-data; name=\"photo\"; filename=\"echopaw.jpg\"\r\n";
    head += "Content-Type: image/jpeg\r\n\r\n";

    String tail = "\r\n--EchoPawBoundary--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t totalLen = head.length() + imageLen + tail.length();

    clientTCP.println("POST /bot" + BOT_TOKEN + "/sendPhoto HTTP/1.1");
    clientTCP.println("Host: api.telegram.org");
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=EchoPawBoundary");
    clientTCP.println();

    clientTCP.print(head);

    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;

    for (size_t n = 0; n < fbLen; n += 1024) {
      if (n + 1024 < fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      } else {
        size_t remainder = fbLen - n;
        clientTCP.write(fbBuf, remainder);
      }
    }

    clientTCP.print(tail);

    esp_camera_fb_return(fb);

    Serial.println("Photo request sent to Telegram.");

    String response = "";
    long startTimer = millis();

    while ((millis() - startTimer) < 10000) {
      while (clientTCP.available()) {
        char c = clientTCP.read();
        response += c;
      }

      if (response.length() > 0) {
        break;
      }
    }

    Serial.println("Telegram response:");
    Serial.println(response);

    clientTCP.stop();

    return "Photo sent";
  } 
  else {
    esp_camera_fb_return(fb);
    Serial.println("Connection to Telegram failed");
    return "Telegram connection failed";
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting EchoPaw ESP32-CAM Photo Sender...");

  clientTCP.setInsecure();

  bool cameraReady = setupCamera();

  if (!cameraReady) {
    Serial.println("Camera failed. Check ribbon cable.");
    return;
  }

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("ESP32-CAM IP: ");
  Serial.println(WiFi.localIP());

  sendTelegramMessage("EchoPaw camera is online. Capturing photo now 📷🐾");

  delay(2000);

  sendPhotoTelegram();
}

void loop() {
  
}