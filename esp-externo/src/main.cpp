#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Configurações do DHT
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configurações do WiFiManager
AsyncWebServer server(80);
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

String ssid;
String pass;
String ip;
String gateway;

const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

unsigned long previousMillis = 0;
const long interval = 10000;

String ledState;

const char* MQTT_BROKER_URL = "431338ce3124410fbd4b0a2d7700c20f.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USERNAME = "mendes";
const char* MQTT_PASSWORD = "Mendes123";
const char* MQTT_TOPIC = "esp32/sensor";

const char* MQTT_CAFILE = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// Inicializa o LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("Erro ao montar o LittleFS");
  }
  Serial.println("LittleFS montado com sucesso");
}

// Lê arquivo do LittleFS
String readFile(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Escreve arquivo no LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    return;
  }
  file.print(message);
}

// Tenta conectar à rede WiFi pré-definida
bool connectToPredefinedWiFi() {
  const char* predefinedSSID = "Galaxy A716D96";
  const char* predefinedPassword = "blxf2209";

  WiFi.begin(predefinedSSID, predefinedPassword);
  Serial.println("Tentando conectar à rede WiFi pré-definida...");

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) { // Timeout de 10 segundos
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado à rede WiFi pré-definida!");
    return true;
  } else {
    Serial.println("\nFalha ao conectar à rede WiFi pré-definida.");
    return false;
  }
}

// Inicializa o WiFi Manager
void startWiFiManager() {
  WiFi.softAP("ESP-WIFI-MANAGER", NULL);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/wifimanager.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);
      if (p->isPost()) {
        if (p->name() == PARAM_INPUT_1) {
          ssid = p->value().c_str();
          writeFile(LittleFS, ssidPath, ssid.c_str());
        }
        if (p->name() == PARAM_INPUT_2) {
          pass = p->value().c_str();
          writeFile(LittleFS, passPath, pass.c_str());
        }
        if (p->name() == PARAM_INPUT_3) {
          ip = p->value().c_str();
          writeFile(LittleFS, ipPath, ip.c_str());
        }
        if (p->name() == PARAM_INPUT_4) {
          gateway = p->value().c_str();
          writeFile(LittleFS, gatewayPath, gateway.c_str());
        }
      }
    }
    request->send(200, "text/plain", "Configuração salva. Reiniciando...");
    delay(3000);
    ESP.restart();
  });
  server.begin();
}

// Conecta ao broker MQTT
void connectToMQTT() {
  mqttClient.setServer(MQTT_BROKER_URL, MQTT_PORT);
  wifiClient.setCACert(MQTT_CAFILE);

  Serial.println("Conectando ao broker MQTT...");
  int tentativas = 0;
  const int maxTentativas = 10;

  while (!mqttClient.connected() && tentativas < maxTentativas) {
    if (mqttClient.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("Conectado ao broker MQTT");
    } else {
      Serial.print("Falha ao conectar ao broker MQTT, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
      tentativas++;
    }
  }

  if (tentativas >= maxTentativas) {
    Serial.println("Número máximo de tentativas atingido. Reiniciando...");
    ESP.restart();
  }
}

// Envia dados do sensor via MQTT
void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Falha ao ler o sensor DHT!");
    float temperature = 0.0;
    float humidity = 0.0;
  }

  String payload = "{\"temperatura\":" + String(temperature) + ",\"umidade\":" + String(humidity) + "}";
  if (mqttClient.publish(MQTT_TOPIC, payload.c_str())) {
    Serial.println("Dados enviados via MQTT");
  } else {
    Serial.println("Falha ao enviar dados via MQTT");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  initLittleFS();

  // Tenta conectar à rede WiFi pré-definida
  if (!connectToPredefinedWiFi()) {
    // Se não conseguir, inicia o WiFi Manager
    startWiFiManager();
  } else {
    // Se conectar, prossegue com a lógica normal
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html");
    });
    server.serveStatic("/", LittleFS, "/");
    server.begin();

    // Conecta ao MQTT e envia dados
    connectToMQTT();
    sendSensorData();

    // Entra em modo deep sleep por 10 minutos
    Serial.println("Conectado ao WiFi. Entrando em modo deep sleep por 10 minutos...");
    esp_sleep_enable_timer_wakeup(10 * 60 * 1000000); // 10 minutos em microssegundos
    esp_deep_sleep_start();
  }
}

void loop() {
  // O código não chega aqui devido ao deep sleep ou ao WiFi Manager
}