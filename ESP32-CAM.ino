#include <WiFi.h>
#include <WebServer.h>
#include <esp_camera.h>

const char* ssid = "INEA-4519";
const char* password = "F8S14S1H7";

WebServer server(80);

String temperaturaBez = "0";
String dsTemperatura = "0";
String pulsCzu = "0";
String saturacjaVal = "0";
String przewodnictwo = "0";
float srednicaZrenicy = 0.0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Łączenie z WiFi...");
  }
  Serial.println("Połączono z WiFi");

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  if (!startCamera()) {
    Serial.println("Błąd podczas inicjalizacji kamery");
  }
}

void loop() {
  while (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');
    Serial.println("Otrzymane dane: " + receivedData);

    if (receivedData.startsWith("MLX temperatura:")) {
      temperaturaBez = receivedData.substring(receivedData.lastIndexOf(" ") + 1);
    } else if (receivedData.startsWith("DS18 temperatura:")) {
      dsTemperatura = receivedData.substring(receivedData.lastIndexOf(" ") + 1);
    } else if (receivedData.startsWith("Puls:")) {
      pulsCzu = receivedData.substring(receivedData.lastIndexOf(" ") + 1);
    } else if (receivedData.startsWith("SaturacjaKrwi:")) {
      saturacjaVal = receivedData.substring(receivedData.lastIndexOf(" ") + 1);
    } else if (receivedData.startsWith("PrzewodnictwoSkory:")) {
      przewodnictwo = receivedData.substring(receivedData.lastIndexOf(" ") + 1);
    }
  }

  srednicaZrenicy = funkcjaSrednicyOka();
  server.handleClient();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Wyniki czujników</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "<script>";
  html += "let labels = [], tempBezData = [], dsTempData = [], pulsData = [], saturacjaData = [], gsrData = [], pupilData = [];";
  html += "function fetchData() {";
  html += "  fetch('/data').then(response => response.json()).then(data => {";
  html += "    labels.push(new Date().toLocaleTimeString());";
  html += "    tempBezData.push(data.tempBez);";
  html += "    dsTempData.push(data.dsTemp);";
  html += "    pulsData.push(data.puls);";
  html += "    saturacjaData.push(data.saturacja);";
  html += "    gsrData.push(data.gsr);";
  html += "    pupilData.push(data.pupil);";
  html += "    if (labels.length > 20) {";
  html += "      labels.shift(); tempBezData.shift(); dsTempData.shift(); pulsData.shift(); saturacjaData.shift(); gsrData.shift(); pupilData.shift();";
  html += "    }";
  html += "    tempBezChart.update(); dsTempChart.update(); pulsChart.update(); saturacjaChart.update(); gsrChart.update(); pupilChart.update();";
  html += "  });";
  html += "}";
  html += "setInterval(fetchData, 2000);";
  html += "</script></head><body>";
  html += "<h1>Odczyty czujników i średnicy źrenicy</h1>";
  html += "<canvas id='tempBezChart' width='400' height='200'></canvas>";
  html += "<canvas id='dsTempChart' width='400' height='200'></canvas>";
  html += "<canvas id='pulsChart' width='400' height='200'></canvas>";
  html += "<canvas id='saturacjaChart' width='400' height='200'></canvas>";
  html += "<canvas id='gsrChart' width='400' height='200'></canvas>";
  html += "<canvas id='pupilChart' width='400' height='200'></canvas>";
  html += "<script>";
  html += "const ctx1 = document.getElementById('tempBezChart').getContext('2d');";
  html += "const ctx2 = document.getElementById('dsTempChart').getContext('2d');";
  html += "const ctx3 = document.getElementById('pulsChart').getContext('2d');";
  html += "const ctx4 = document.getElementById('saturacjaChart').getContext('2d');";
  html += "const ctx5 = document.getElementById('gsrChart').getContext('2d');";
  html += "const ctx6 = document.getElementById('pupilChart').getContext('2d');";
  html += "const tempBezChart = new Chart(ctx1, { type: 'line', data: { labels: labels, datasets: [{ label: 'MLX Temperatura', data: tempBezData, borderColor: 'green', fill: false }] }, options: { scales: { x: { display: true }, y: { beginAtZero: true }}}});";
  html += "const dsTempChart = new Chart(ctx2, { type: 'line', data: { labels: labels, datasets: [{ label: 'DS18 Temperatura', data: dsTempData, borderColor: 'blue', fill: false }] }, options: { scales: { x: { display: true }, y: { beginAtZero: true }}}});";
  html += "const pulsChart = new Chart(ctx3, { type: 'line', data: { labels: labels, datasets: [{ label: 'Puls', data: pulsData, borderColor: 'red', fill: false }] }, options: { scales: { x: { display: true }, y: { beginAtZero: true }}}});";
  html += "const saturacjaChart = new Chart(ctx4, { type: 'line', data: { labels: labels, datasets: [{ label: 'Saturacja', data: saturacjaData, borderColor: 'orange', fill: false }] }, options: { scales: { x: { display: true }, y: { beginAtZero: true }}}});";
  html += "const gsrChart = new Chart(ctx5, { type: 'line', data: { labels: labels, datasets: [{ label: 'Przewodnictwo skóry', data: gsrData, borderColor: 'purple', fill: false }] }, options: { scales: { x: { display: true }, y: { beginAtZero: true }}}});";
  html += "const pupilChart = new Chart(ctx6, { type: 'line', data: { labels: labels, datasets: [{ label: 'Średnica źrenicy (mm)', data: pupilData, borderColor: 'black', fill: false }] }, options: { scales: { x: { display: true }, y: { beginAtZero: true }}}});";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"tempBez\": " + temperaturaBez + ",";
  json += "\"dsTemp\": " + dsTemperatura + ",";
  json += "\"puls\": " + pulsCzu + ",";
  json += "\"saturacja\": " + saturacjaVal + ",";
  json += "\"gsr\": " + przewodnictwo + ",";
  json += "\"pupil\": " + String(srednicaZrenicy);
  json += "}";
  server.send(200, "application/json", json);
}

float funkcjaSrednicyOka(){
camera_fb_t* zdjecie = esp_camera_fb_get();
  if(!zdjecie){
    Serial.println("Błąd: Nie udało się pobrać obrazu z kamery");
    return -1;
  }

  uint8_t* grayscale = (uint8_t*)malloc(zdjecie->width * zdjecie->height);
  if(!grayscale){
    esp_camera_fb_return(zdjecie);
    return -1;
  }

  for(int i = 0; i < zdjecie->len; i += 3){
    uint8_t r = zdjecie->buf[i];
    uint8_t g = zdjecie->buf[i + 1];
    uint8_t b = zdjecie->buf[i + 2];
    grayscale[i / 3] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
  }

  int thresholdValue = 50;
  for(int i = 0; i < zdjecie->width * zdjecie->height; i++){
    if(grayscale[i] < thresholdValue){
      grayscale[i] = 0;
    }else{
      grayscale[i] = 255;
    }
  }

  int left = zdjecie->width, right = 0, top = zdjecie->height, bottom = 0;
  for(int y = 0; y < zdjecie->height; y++){
    for(int x = 0; x < zdjecie->width; x++){
      if (grayscale[y * zdjecie->width + x] == 0){
        if(x < left){
          left = x;
        } 
        if(x > right){
          right = x;
        }
        if(y < top){
          top = y;
        }
        if(y > bottom){
          bottom = y;
        }
      }
    }
  }

  int width = right - left;
  int height = bottom - top;
  float diameter = (width + height) / 2.0;
  esp_camera_fb_return(zdjecie);
  free(grayscale);
  return (diameter * 0.1);
}

bool startCamera() {
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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  return esp_camera_init(&config) == ESP_OK;
}
