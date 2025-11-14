#include <WiFi.h>
#include <PubSubClient.h>

// Configurações WiFi
const char* ssid = "Inteli.Iot";  // Rede WiFi
const char* password = "%(Yk(sxGMtvFEs.3";  // Senha WiFi

// Configurações MQTT
const char* mqtt_server = "10.128.0.140";  // IP do computador com Mosquitto
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

// Pinos dos LDRs
const int LDR1_PIN = 34;  // LDR para semáforo 1
const int LDR2_PIN = 35;  // LDR para semáforo 2

// Pinos dos Semáforos
// Semáforo 1
const int SEM1_VERMELHO = 12;
const int SEM1_AMARELO = 13;
const int SEM1_VERDE = 14;
// Semáforo 2
const int SEM2_VERMELHO = 25;
const int SEM2_AMARELO = 26;
const int SEM2_VERDE = 27;

// Configurações
const int LIMIAR_LUZ = 1000;  // Valor alto = luz presente, baixo = falta de luz (veículo)
const int TEMPO_VERDE = 4000;   // 4s
const int TEMPO_AMARELO = 2000; // 2s
const int TEMPO_VERMELHO = 6000; // 6s
const int INTERVALO_PISCA = 500; // Modo noturno

// Estados
bool modoNoturno = false;
int estadoSem1 = 0;  // 0=vermelho, 1=verde, 2=amarelo
int estadoSem2 = 0;
unsigned long tempoInicioSem1 = 0;
unsigned long tempoInicioSem2 = 0;
unsigned long ultimoPisca = 0;
bool estadoPisca = false;
String mensagemLCD = "Bem-vindo!";

void setup() {
  Serial.begin(115200);

  // Configurar pinos
  pinMode(LDR1_PIN, INPUT);
  pinMode(LDR2_PIN, INPUT);
  pinMode(SEM1_VERMELHO, OUTPUT);
  pinMode(SEM1_AMARELO, OUTPUT);
  pinMode(SEM1_VERDE, OUTPUT);
  pinMode(SEM2_VERMELHO, OUTPUT);
  pinMode(SEM2_AMARELO, OUTPUT);
  pinMode(SEM2_VERDE, OUTPUT);

  // Desligar todos
  apagarSemaforos();

  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  // Conectar MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  Serial.println("Sistema iniciado!");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Ler LDRs e publicar
  int ldr1 = analogRead(LDR1_PIN);
  int ldr2 = analogRead(LDR2_PIN);
  client.publish("/semaforo/1/ldr", String(ldr1).c_str());
  client.publish("/semaforo/2/ldr", String(ldr2).c_str());

  // Lógica de alternância baseada em LDR
  if (!modoNoturno) {
    // Detectar veículo no LDR1 (falta de luz)
    if (ldr1 < LIMIAR_LUZ && estadoSem1 == 0) {  // Sem1 em vermelho
      iniciarCicloSem1();
    }
    // Detectar veículo no LDR2
    if (ldr2 < LIMIAR_LUZ && estadoSem2 == 0) {
      iniciarCicloSem2();
    }

    // Atualizar ciclos
    atualizarCicloSem1();
    atualizarCicloSem2();
  } else {
    // Modo noturno: piscar amarelo em ambos
    if (millis() - ultimoPisca >= INTERVALO_PISCA) {
      estadoPisca = !estadoPisca;
      digitalWrite(SEM1_AMARELO, estadoPisca);
      digitalWrite(SEM2_AMARELO, estadoPisca);
      digitalWrite(SEM1_VERMELHO, LOW);
      digitalWrite(SEM1_VERDE, LOW);
      digitalWrite(SEM2_VERMELHO, LOW);
      digitalWrite(SEM2_VERDE, LOW);
      ultimoPisca = millis();
    }
  }

  delay(100);  // Pequeno delay para estabilidade
}

void iniciarCicloSem1() {
  estadoSem1 = 1;  // Verde
  tempoInicioSem1 = millis();
  digitalWrite(SEM1_VERDE, HIGH);
  digitalWrite(SEM1_AMARELO, LOW);
  digitalWrite(SEM1_VERMELHO, LOW);
  // Sem2 para vermelho
  estadoSem2 = 0;
  digitalWrite(SEM2_VERMELHO, HIGH);
  digitalWrite(SEM2_AMARELO, LOW);
  digitalWrite(SEM2_VERDE, LOW);
  client.publish("/semaforo/1/estado", "verde");
  client.publish("/semaforo/2/estado", "vermelho");
}

void iniciarCicloSem2() {
  estadoSem2 = 1;  // Verde
  tempoInicioSem2 = millis();
  digitalWrite(SEM2_VERDE, HIGH);
  digitalWrite(SEM2_AMARELO, LOW);
  digitalWrite(SEM2_VERMELHO, LOW);
  // Sem1 para vermelho
  estadoSem1 = 0;
  digitalWrite(SEM1_VERMELHO, HIGH);
  digitalWrite(SEM1_AMARELO, LOW);
  digitalWrite(SEM1_VERDE, LOW);
  client.publish("/semaforo/2/estado", "verde");
  client.publish("/semaforo/1/estado", "vermelho");
}

void atualizarCicloSem1() {
  if (estadoSem1 == 1 && millis() - tempoInicioSem1 >= TEMPO_VERDE) {
    estadoSem1 = 2;  // Amarelo
    tempoInicioSem1 = millis();
    digitalWrite(SEM1_VERDE, LOW);
    digitalWrite(SEM1_AMARELO, HIGH);
    client.publish("/semaforo/1/estado", "amarelo");
  } else if (estadoSem1 == 2 && millis() - tempoInicioSem1 >= TEMPO_AMARELO) {
    estadoSem1 = 0;  // Vermelho
    tempoInicioSem1 = 0;
    digitalWrite(SEM1_AMARELO, LOW);
    digitalWrite(SEM1_VERMELHO, HIGH);
    client.publish("/semaforo/1/estado", "vermelho");
  }
}

void atualizarCicloSem2() {
  if (estadoSem2 == 1 && millis() - tempoInicioSem2 >= TEMPO_VERDE) {
    estadoSem2 = 2;  // Amarelo
    tempoInicioSem2 = millis();
    digitalWrite(SEM2_VERDE, LOW);
    digitalWrite(SEM2_AMARELO, HIGH);
    client.publish("/semaforo/2/estado", "amarelo");
  } else if (estadoSem2 == 2 && millis() - tempoInicioSem2 >= TEMPO_AMARELO) {
    estadoSem2 = 0;  // Vermelho
    tempoInicioSem2 = 0;
    digitalWrite(SEM2_AMARELO, LOW);
    digitalWrite(SEM2_VERMELHO, HIGH);
    client.publish("/semaforo/2/estado", "vermelho");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "/semaforo/modo") {
    if (message == "noturno") {
      modoNoturno = true;
      apagarSemaforos();
      Serial.println("Modo noturno ativado via MQTT");
    } else if (message == "normal") {
      modoNoturno = false;
      apagarSemaforos();
      Serial.println("Modo normal ativado via MQTT");
    }
  } else if (String(topic) == "/semaforo/lcd/mensagem") {
    mensagemLCD = message;
    Serial.print("Mensagem LCD: ");
    Serial.println(mensagemLCD);
    // Aqui você pode integrar com a biblioteca do LCD, ex.: lcd.print(mensagemLCD);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("conectado!");
      client.subscribe("/semaforo/modo");
      client.subscribe("/semaforo/lcd/mensagem");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
    }
  }
}

void apagarSemaforos() {
  digitalWrite(SEM1_VERMELHO, LOW);
  digitalWrite(SEM1_AMARELO, LOW);
  digitalWrite(SEM1_VERDE, LOW);
  digitalWrite(SEM2_VERMELHO, LOW);
  digitalWrite(SEM2_AMARELO, LOW);
  digitalWrite(SEM2_VERDE, LOW);
}