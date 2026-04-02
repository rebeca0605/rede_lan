//Inclusão de libs
#include <WiFi.h> //ativação de módulo de wifi
#include <WebServer.h> //ativação de servidor web

//variáveis
const char* ssid = "IoT-B08"
const char* password = "12345678"

//login http
const char* http_user = "admin"
const char* http_password = "1234"

//inicialização do server
Webserver server(80)

//Led
const int LED_PIN = 2; //conectando GPIO 2
boll ledState = false; //incializa desligado

//Return DHT11
float temperatura = 0;
float umidade = 0;

//conexão
boll wifiConnected = false;

//Autenticathor
boll isAuthenticated() {
  if(!server.authenticated(http_user, http_password)) {
    server.requestAuthenticator();
    return false;
  }
  return false;
}

//FrontEnd
String buildPage(){
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<met name='viewport' content='width-device-width'>";
  html += "<title>ESP32</title>";
  html += "<style>";
  html += "body{font-family:sans-serif; text-align:center; padding:40px; background:#f4f4f4}";
  html += ".on{background:#009d00; color:#f4f4f4}";
  html += ".off{background:#F43208; color:#f4f4f4}";
  html += "</head><body>";

  html += "<h1>Comunicação LAN</h1>";
  html += "<p>Status <strong>";
  html += (ledState) ? "LIGADO" : "DESLIGADO";
  html += "</strong></p>";

  html += "<hr>";

  //Inclusão dos botões
  html += "<a href='/on'><button class='on'>LIGAR</button></a>";
  html += "<a href='/off'><button class='off'>DESLIGAR</button></a>";

  //Retorno dos daods de Umidade
  html += "<h2>Sensor DHT11</h2>";
  html += "<p>Umidade: ";
  html += String(umidade);
  html += "%</p>";

  html += "</body><html>";

  return html;
}

//WIFI
bool connecteWifi(){
  Serial.println("\nWifi connect... Resetando a interface");

  Wifi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("[WiFi] conectando...");
  Serial.println("[WiFi] SSID: ");
  Serial.println(ssid);

  //laço de tentativas de conexões na rede
    int tentativas = 0; //contador de tentativas de conexão na rede
    while(WiFi.status() != WL_CONNECT && tentativas < 20){
      delay(500);
      Serial.println(".");
      tentativas++;
    }

    if(WiFi.status() == WL_CONNECTED){
      Serial.println("\n[WIFI] Conectado!");
      Serial.println("[WIFI] IP: ");
      Serial.println(WiFi.localIP()); //retorna o ip, aqui acessaremos o servidor no navegador

      return true;

    } else {
      Serial.println("\n[WIFI] Falha na conexão");
      return false;
    }
}

//Routes
void setupRoutes(){
  //Se autenticando
  server.on("/", [](){
    if(!isAuthenticated()) return;
    server.send(200, "text/html", buildPage());
  });

  //Ligar o led
  server.on("/on", [](){
    if(!isAuthenticated()) return;
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Led ligado");
    server.sendHeader("Location", "/");
    server.send(303);
  });

    //Desligar o led
  server.on("/off", [](){
    if(!isAuthenticated()) return;
    ledState = true;
    digitalWrite(LED_PIN, LOW);
    Serial.println("Led desligado");
    server.sendHeader("Location", "/");
    server.send(303);
  });

  //Receber dados do DHT11
  server.on("/update", [](){
    //Sem auth
    if (server.hasArg("temp")){
      tempertatura = server.arg("temp").toFloat();
    }

    if (server.hasArg("hum")){
      umidade = server.arg("hum").toFloat();
    }

    Serial.println("[DHT] Dados recebidos");
    Serial.println("Temperatura: ");
    Serial.println(Temperatura);
    Serial.println("Umidade: ");
    Serial.println(Umidade);
    server.send(200, "text/plain", OK);
  });
}

//Setup
void setup(){
  Serial.begin(115200); //Importante checar se a placa ñ 9600
  delay(1000);
  Serial.println("\n[BOOT]Inicializando o ESP32...");
  pinMode(LED_PIN, LOW);

  wifiConnected = connectWifi();
  setupRoutes();

  if(wifiConnected){
    server.begin();
    server.println("[HTTP] Servidor Iniciado!");
  } else {
    Serial.println("[HTTP] Servidor Não Iniciado (Sem Wifi)!");
  }
}

//loop
void loop(){
  if(WiFi.status() != WL_CONNECTED){
    if(wifiConnected){
      Serial.println("[WIFI] Conexão Perdida");
      wifiConnected = false;
    }
    delay(2000);
    wifiConnected = connectWiFi();
    if(wifiConnected){
      server.begin();
      Serial.println("[HTTP] Servidor reiniciado!");
    }
    return;
  }

  server.handleClient();
}