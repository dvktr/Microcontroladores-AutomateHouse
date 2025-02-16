#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <Update.h>

// Configurações do Sensor de Chuva
#define RAIN_SENSOR_PIN 33

// Configurações do Sensor de Luz
#define LIGHT_SENSOR_PIN 32
#define LIGHT_THRESHOLD 1000  // Ajuste conforme necessário
#define LED_PIN 4

// Configurações do Servo Motor
#define SERVO_PIN 13
Servo windowServo;

// Configurações do WiFiManager
AsyncWebServer server(80);
const char* host = "esp32";
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
const char* MQTT_WINDOW_CONTROL_TOPIC = "esp32/window/control";

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
    Serial.print("IP obtido: ");
    Serial.println(WiFi.localIP());
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

const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 - identifique-se</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Login:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Senha:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Identificar'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Login ou senha inválidos')"
    "}"
    "}"
"</script>";
  
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>Progresso: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('Progresso: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('Sucesso!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

 void setupOTA() {
  if (!MDNS.begin(host)) { 
    Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
    delay(1000);
    ESP.restart();        
  }
  
  Serial.println("mDNS configurado e inicializado;");
  
  // Configura as páginas de login e upload de firmware OTA
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", loginIndex);
  });
  
  server.on("/serverIndex", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", serverIndex);
  });
  
  // Define tratamentos do update de firmware OTA
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      // Início do upload de firmware OTA
      Serial.printf("Update: %s\n", filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    }
    
    // Escrevendo firmware enviado na flash do ESP32
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }
    
    if (final) {
      // Final de upload
      if (Update.end(true)) {
        Serial.printf("Sucesso no update de firmware: %u\nReiniciando ESP32...\n", index + len);
      } else {
        Update.printError(Serial);
      }
    }
  });
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
      mqttClient.subscribe(MQTT_WINDOW_CONTROL_TOPIC);
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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.trim();

  if (String(topic) == MQTT_WINDOW_CONTROL_TOPIC) {
    Serial.println("Mensagem recebida no tópico: " + String(topic));
    Serial.println("Conteúdo da mensagem: " + message);

    if (message.equals("{\"state\":\"closed\"}")) {
      Serial.println("Estado recebido: closed");
      windowServo.write(15);
    } else if (message.equals("{\"state\":\"open\"}")) {
      Serial.println("Estado recebido: open");
      windowServo.write(115);
    } else {
      Serial.println("Mensagem desconhecida: " + message);
    }
  } else {
    Serial.println("Tópico desconhecido: " + String(topic));
  }
}

// Verifica se está chovendo
bool isRaining() {
  int rainValue = digitalRead(RAIN_SENSOR_PIN);
  return rainValue;  // Ajuste o valor conforme necessário
}

// Verifica a luminosidade
bool isDark() {
  int lightValue = digitalRead(LIGHT_SENSOR_PIN);
  return lightValue;
}

// Faz uma requisição GET para obter o estado inicial da janela
void getInitialWindowState() {
  HTTPClient http;
  http.begin("https://automate-house-production.up.railway.app/window/state");
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    payload.trim();

    // Comparação robusta usando equals()
    if (payload.equals("{\"state\":\"closed\"}")) {
      Serial.println("Peguei o estado na primeira ligação: closed");
      windowServo.write(15);  // Fecha a janela
    } else if (payload.equals("{\"state\":\"open\"}")) {
      Serial.println("Peguei o estado na primeira ligação: open");
      windowServo.write(115);  // Abre a janela
    } else {
      Serial.println("Estado desconhecido: " + payload);
    }
  } else {
    Serial.println("Falha ao obter o estado inicial da janela. Código HTTP: " + String(httpCode));
  }
  http.end();
}

String getWindowStateFromAPI() {
  HTTPClient http;
  http.begin("https://automate-house-production.up.railway.app/window/state");
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();
    return payload;
  } else {
    Serial.println("Falha ao obter o estado da janela da API");
    http.end();
    return "";
  }
}

void setup() {
  Serial.begin(115200);
  initLittleFS();

  // Inicializa o servo motor
  windowServo.attach(SERVO_PIN);

  // Inicializa o LED
  pinMode(LED_PIN, OUTPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);

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

    // Conecta ao MQTT
    mqttClient.setCallback(mqttCallback);
    connectToMQTT();

    // Obtém o estado inicial da janela
    getInitialWindowState();

    setupOTA();
  }
}

void loop() {
  // Verifica se está chovendo
  if (isRaining() == LOW) {  
    String windowState = getWindowStateFromAPI();
  
    windowState.replace(" ", "");
  
    if (windowState.equals("{\"state\":\"open\"}")) {
      mqttClient.publish(MQTT_WINDOW_CONTROL_TOPIC, "{\"state\": \"closed\"}");
      Serial.println("Janela estava aberta. Enviando comando para fechar.");
      windowServo.write(15);  // Fecha a janela
    } else {
      Serial.println("Janela já está fechada. Nenhum comando enviado.");
    }
  }

  // Verifica a luminosidade
  static unsigned long darkStartTime = 0;
  if (isDark()) {
    if (darkStartTime == 0) {
      darkStartTime = millis();
    } else if (millis() - darkStartTime >= 5000) {
      digitalWrite(LED_PIN, HIGH);  // Acende o LED
    }
  } else {
    darkStartTime = 0;
    digitalWrite(LED_PIN, LOW);  // Apaga o LED
  }

  // Mantém a conexão MQTT ativa
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
  delay(500);
}