# 🚀 Deploy do Semáforo Inteligente

## GitHub Pages (Interface Web)

### Acesso
A interface está disponível em: **https://pietroalkmin.github.io/Ponderada-Semaforo-Inteligente/**

### Como foi feito

1. **Configuração automática via GitHub Actions**
   - O arquivo `.github/workflows/deploy.yml` faz deploy automático
   - Cada push na branch `main` atualiza o site

2. **Broker MQTT Público**
   - Usando HiveMQ Cloud (broker gratuito)
   - Conexão segura via WebSocket (wss://)
   - Acessível de qualquer lugar

### Configuração Manual (caso necessário)

Se o deploy automático não funcionar:

1. Vá em **Settings → Pages**
2. Em **Source**, selecione: **GitHub Actions**
3. Aguarde o workflow executar

## ESP32 (Hardware)

### Passo a passo

1. **Instalar bibliotecas no Arduino IDE:**
   - WiFi (já inclusa no ESP32)
   - PubSubClient
   - Wire (já inclusa)
   - LiquidCrystal_I2C

2. **Configurar WiFi:**
   ```cpp
   const char* ssid = "SEU_WIFI";
   const char* password = "SUA_SENHA";
   ```

3. **Upload do código:**
   - Abra `esp32/semaforo_inteligente_completo.ino`
   - Selecione a porta COM do ESP32
   - Faça upload

4. **Verificar funcionamento:**
   - Abra o Serial Monitor (115200 baud)
   - Verifique se conectou ao WiFi e MQTT
   - Acesse a interface web

## Testando a Integração

1. Acesse: https://pietroalkmin.github.io/Ponderada-Semaforo-Inteligente/
2. Aguarde conectar ao broker MQTT
3. O ESP32 deve aparecer publicando dados
4. Teste os controles:
   - Ativar modo noturno
   - Enviar mensagens para o LCD

## Troubleshooting

### Interface não conecta ao MQTT
- Verifique se o ESP32 está ligado e conectado
- Abra o console do navegador (F12) para ver erros
- Certifique-se que o ESP32 usa o mesmo broker (broker.hivemq.com)

### ESP32 não conecta ao WiFi
- Verifique credenciais do WiFi
- Certifique-se que o WiFi é 2.4GHz (ESP32 não suporta 5GHz)
- Veja mensagens no Serial Monitor

### Dados não aparecem na interface
- Confirme que os tópicos MQTT estão corretos
- Verifique logs no console do navegador
- Reinicie o ESP32

## Alternativa: Broker Local

Para usar broker local (rede Inteli):

1. **No ESP32:**
   ```cpp
   const char* mqtt_server = "10.128.0.140";
   const int mqtt_port = 1883;
   ```

2. **No script.js:**
   ```javascript
   const client = mqtt.connect('ws://10.128.0.140:9001');
   ```

**Nota:** Com broker local, a interface só funciona na mesma rede!
