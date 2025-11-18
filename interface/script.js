// Conexão MQTT com broker público HiveMQ (gratuito)
// Para usar seu próprio broker, substitua a URL abaixo
const client = mqtt.connect('wss://broker.hivemq.com:8884/mqtt');

client.on('connect', () => {
    console.log('Conectado ao Mosquitto!');
    addLog('Conectado ao broker MQTT.');

    // Subscrever tópicos para dados em tempo real
    client.subscribe('/semaforo/1/ldr');
    client.subscribe('/semaforo/2/ldr');
    client.subscribe('/semaforo/1/estado');
    client.subscribe('/semaforo/2/estado');
});

client.on('message', (topic, message) => {
    const data = message.toString();
    addLog(`Recebido: ${topic} -> ${data}`);

    if (topic === '/semaforo/1/ldr') {
        document.getElementById('ldr1').textContent = data;
    } else if (topic === '/semaforo/2/ldr') {
        document.getElementById('ldr2').textContent = data;
    } else if (topic === '/semaforo/1/estado') {
        document.getElementById('estado1').textContent = data;
    } else if (topic === '/semaforo/2/estado') {
        document.getElementById('estado2').textContent = data;
    }
});

// Função para adicionar logs
function addLog(message) {
    const logContainer = document.getElementById('logContainer');
    const logEntry = document.createElement('p');
    logEntry.textContent = new Date().toLocaleTimeString() + ' - ' + message;
    logContainer.appendChild(logEntry);
    logContainer.scrollTop = logContainer.scrollHeight; // Auto-scroll
}

// Controles
document.getElementById('modoNoturnoBtn').addEventListener('click', () => {
    client.publish('/semaforo/modo', 'noturno');
    addLog('Modo noturno ativado.');
});

document.getElementById('modoNormalBtn').addEventListener('click', () => {
    client.publish('/semaforo/modo', 'normal');
    addLog('Modo normal ativado.');
});

document.getElementById('enviarMensagemBtn').addEventListener('click', () => {
    const mensagem = document.getElementById('mensagemLCD').value;
    if (mensagem) {
        client.publish('/semaforo/lcd', mensagem);
        addLog(`Mensagem enviada para LCD: ${mensagem}`);
        document.getElementById('mensagemLCD').value = ''; // Limpar campo
    } else {
        alert('Digite uma mensagem primeiro!');
    }
});