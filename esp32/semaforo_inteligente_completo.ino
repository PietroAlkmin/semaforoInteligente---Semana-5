#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== CONFIGURAÇÕES LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Endereço 0x27, 16 colunas, 2 linhas
// Se não funcionar com 0x27, tente 0x3F

// ===== CONFIGURAÇÕES WIFI =====
const char* ssid = "Inteli.Iot";
const char* password = "%(Yk(sxGMtvFEs.3";

// ===== CONFIGURAÇÕES MQTT =====
const char* mqtt_server = "10.128.0.140";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

// ===== PINOS DOS LDRs =====
const int LDR_PIN = 34;  // GPIO34 - ÚNICO LDR FUNCIONANDO

// ===== PINOS DOS SEMÁFOROS =====
// Semáforo 1 e 2 compartilham os mesmos pinos (em paralelo)
const int SEM_VERMELHO = 13;  // GPIO13
const int SEM_AMARELO = 12;   // GPIO12
const int SEM_VERDE = 14;     // GPIO14

// ===== CONFIGURAÇÕES =====
const int LIMIAR_LUZ = 2000;  // Ajuste conforme seu LDR (menor = escuro/veículo presente)
const int TEMPO_VERDE = 4000;     // 4 segundos
const int TEMPO_AMARELO = 2000;   // 2 segundos
const int TEMPO_VERMELHO = 6000;  // 6 segundos
const int INTERVALO_PISCA = 500;  // Piscar amarelo no modo noturno

// ===== VARIÁVEIS DE ESTADO =====
bool modoNoturno = false;
bool veiculoDetectado = false;  // true = veículo detectado no LDR
int estadoAtual = 0;  // 0=verde, 1=amarelo, 2=vermelho
unsigned long tempoMudancaEstado = 0;
unsigned long ultimoPisca = 0;
bool estadoPisca = false;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n=== SISTEMA SEMÁFORO INTELIGENTE ===");

  // ===== INICIALIZAR LCD =====
  Wire.begin(21, 22);  // SDA=GPIO21, SCL=GPIO22
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SEMAFORO IoT");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);

  // ===== CONFIGURAR PINOS =====
  pinMode(LDR_PIN, INPUT);
  pinMode(SEM_VERMELHO, OUTPUT);
  pinMode(SEM_AMARELO, OUTPUT);
  pinMode(SEM_VERDE, OUTPUT);

  // Inicializar todos desligados
  apagarTodosLeds();

  // ===== CONECTAR WIFI =====
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");
  
  Serial.print("Conectando WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi OK!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);

  // ===== CONECTAR MQTT =====
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  // ===== ESTADO INICIAL =====
  estadoAtual = 0;
  tempoMudancaEstado = millis();
  atualizarSemaforos();
  
  // Limpar LCD e aguardar mensagens
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aguardando msg");
  lcd.setCursor(0, 1);
  lcd.print("via MQTT...");
  
  Serial.println("=== Sistema Pronto! ===\n");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // ===== LER LDR (GPIO34) =====
  int ldrValue = analogRead(LDR_PIN);
  
  // Detectar veículo (LDR escuro = veículo presente)
  veiculoDetectado = (ldrValue < LIMIAR_LUZ);

  // Publicar valores LDR via MQTT
  static unsigned long ultimaPublicacao = 0;
  if (millis() - ultimaPublicacao > 1000) {  // Publicar a cada 1s
    client.publish("/semaforo/1/ldr", String(ldrValue).c_str());
    client.publish("/semaforo/2/ldr", String(ldrValue).c_str());
    
    Serial.print("LDR (GPIO34): ");
    Serial.print(ldrValue);
    Serial.println(veiculoDetectado ? " [VEÍCULO DETECTADO]" : " [LIVRE]");
    
    ultimaPublicacao = millis();
  }

  // ===== MODO NOTURNO =====
  if (modoNoturno) {
    modoNoturnoLoop();
    return;
  }

  // ===== LÓGICA DOS SEMÁFOROS =====
  // Só conta o tempo se houver veículo detectado
  if (veiculoDetectado) {
    unsigned long tempoDecorrido = millis() - tempoMudancaEstado;

    switch (estadoAtual) {
      case 0:  // VERDE
        if (tempoDecorrido >= TEMPO_VERDE) {
          estadoAtual = 1;  // Próximo: AMARELO
          tempoMudancaEstado = millis();
          atualizarSemaforos();
        }
        break;

      case 1:  // AMARELO
        if (tempoDecorrido >= TEMPO_AMARELO) {
          estadoAtual = 2;  // Próximo: VERMELHO
          tempoMudancaEstado = millis();
          atualizarSemaforos();
        }
        break;

      case 2:  // VERMELHO
        if (tempoDecorrido >= TEMPO_VERMELHO) {
          estadoAtual = 0;  // Volta: VERDE
          tempoMudancaEstado = millis();
          atualizarSemaforos();
        }
        break;
    }
  } else {
    // Sem veículo: reseta o contador de tempo para "pausar" o ciclo
    tempoMudancaEstado = millis();
  }

  delay(100);  // Pequeno delay para não sobrecarregar
}

// ===== FUNÇÕES AUXILIARES =====

void apagarTodosLeds() {
  digitalWrite(SEM_VERMELHO, LOW);
  digitalWrite(SEM_AMARELO, LOW);
  digitalWrite(SEM_VERDE, LOW);
}

void atualizarSemaforos() {
  apagarTodosLeds();
  
  switch (estadoAtual) {
    case 0:  // VERDE (ambos semáforos)
      digitalWrite(SEM_VERDE, HIGH);
      client.publish("/semaforo/1/estado", "verde");
      client.publish("/semaforo/2/estado", "verde");
      Serial.println("Estado: VERDE");
      break;

    case 1:  // AMARELO (ambos semáforos)
      digitalWrite(SEM_AMARELO, HIGH);
      client.publish("/semaforo/1/estado", "amarelo");
      client.publish("/semaforo/2/estado", "amarelo");
      Serial.println("Estado: AMARELO");
      break;

    case 2:  // VERMELHO (ambos semáforos)
      digitalWrite(SEM_VERMELHO, HIGH);
      client.publish("/semaforo/1/estado", "vermelho");
      client.publish("/semaforo/2/estado", "vermelho");
      Serial.println("Estado: VERMELHO");
      break;
  }
}

void modoNoturnoLoop() {
  // Piscar amarelo a cada 500ms
  if (millis() - ultimoPisca >= INTERVALO_PISCA) {
    estadoPisca = !estadoPisca;
    
    apagarTodosLeds();
    if (estadoPisca) {
      digitalWrite(SEM_AMARELO, HIGH);
    }
    
    // Publicar estado apenas quando mudar
    static unsigned long ultimaPublicacaoNoturno = 0;
    if (millis() - ultimaPublicacaoNoturno > 5000) {  // A cada 5s
      client.publish("/semaforo/1/estado", "noturno");
      client.publish("/semaforo/2/estado", "noturno");
      ultimaPublicacaoNoturno = millis();
    }
    
    ultimoPisca = millis();
  }
}

// ===== CALLBACK MQTT =====
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("MQTT recebido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  Serial.print("Length: ");
  Serial.println(length);

  if (String(topic) == "/semaforo/modo") {
    if (message == "noturno") {
      modoNoturno = true;
      apagarTodosLeds();
      ultimoPisca = millis();
      Serial.println(">>> MODO NOTURNO ATIVADO <<<");
    } else if (message == "normal") {
      modoNoturno = false;
      estadoAtual = 0;
      tempoMudancaEstado = millis();
      atualizarSemaforos();
      Serial.println(">>> MODO NORMAL ATIVADO <<<");
    }
  }
  
  // Mensagem para LCD (formato: "Linha1|Linha2" ou apenas "Linha1")
  if (String(topic) == "/semaforo/lcd") {
    Serial.println(">>> TÓPICO /semaforo/lcd DETECTADO <<<");
    Serial.print("Mensagem recebida: [");
    Serial.print(message);
    Serial.println("]");
    
    int separador = message.indexOf('|');
    
    lcd.clear();
    lcd.setCursor(0, 0);
    
    if (separador > 0) {
      // Duas linhas
      String linha1 = message.substring(0, separador);
      String linha2 = message.substring(separador + 1);
      
      Serial.print("Linha 1: [");
      Serial.print(linha1);
      Serial.println("]");
      Serial.print("Linha 2: [");
      Serial.print(linha2);
      Serial.println("]");
      
      lcd.print(linha1);
      lcd.setCursor(0, 1);
      lcd.print(linha2);
    } else {
      // Uma linha apenas
      Serial.print("Linha única: [");
      Serial.print(message);
      Serial.println("]");
      
      lcd.print(message);
    }
    
    Serial.println(">>> MENSAGEM LCD ATUALIZADA NO DISPLAY <<<");
  }
}

// ===== RECONEXÃO MQTT =====
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32_Semaforo")) {
      Serial.println(" conectado!");
      client.subscribe("/semaforo/modo");
      client.subscribe("/semaforo/lcd");
      Serial.println("Inscrito nos tópicos MQTT");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(client.state());
      Serial.println(" - tentando novamente em 5s");
      delay(5000);
    }
  }
}
