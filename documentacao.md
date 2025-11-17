### Sistema de Semáforos Inteligente com Sensor LDR e Modo Noturno


LINK das imagens e vídeo demonstrativo: [https://drive.google.com/drive/folders/1hZ-J_5z5H29-E_BpdBOBVze9a-kWM2x5?usp=sharing]  

#### Parte 1 — Sistema Físico com LDR e Modo Noturno


Este projeto consiste na criação de um sistema de controle de semáforos inteligente que utiliza sensores de luminosidade para adaptar seu comportamento às condições ambientais. Os principais objetivos incluem a montagem de dois semáforos físicos utilizando LEDs nas cores vermelho, amarelo e verde, a integração de um sensor LDR para detectar a presença de veículos através de variações bruscas de luz, a alteração automática do ciclo do semáforo com base na luminosidade ambiente, e a ativação automática do modo noturno quando o ambiente estiver escuro.

A montagem de cada semáforo é composta por um LED verde, um LED amarelo, um LED vermelho e resistores adequados para cada componente. Adicionalmente, um sensor LDR é conectado através de um divisor de tensão para realizar as leituras analógicas necessárias. Para facilitar a replicação do projeto, é recomendável incluir um diagrama Fritzing no repositório.

O funcionamento do LDR como detector de veículos baseia-se em um princípio simplificado onde, quando algo se aproxima e bloqueia parcialmente a luz incidente sobre o sensor, o valor analógico lido diminui, e quando a luz aumenta novamente, o valor sobe. Essa variação permite simular a passagem de veículos em frente ao sensor de forma eficaz.

O sistema interpreta os valores do LDR utilizando diferentes limiares de detecção. Quando o valor lido está acima de um threshold alto, o sistema considera que o ambiente está iluminado. Quando o valor cai abaixo de um threshold baixo, isso indica a presença de um objeto, como um carro. A média dos valores coletados é utilizada para determinar o estado geral da luminosidade ambiente, embora a lógica exata dependa da calibração específica do sensor.

O modo noturno é ativado ao detectar baixa luminosidade geral, não apenas durante a passagem de um carro. Quando esse modo está ativo, o comportamento do sistema se altera significativamente: os semáforos passam a piscar apenas a luz amarela, indicando atenção aos motoristas, e o ciclo normal de transição entre verde, amarelo e vermelho é interrompido. Essa implementação segue modelos reais de controle de tráfego utilizados em horários de baixo fluxo de veículos.

#### Parte 2 — Interface Online


A segunda parte do projeto consiste no desenvolvimento de uma interface online com diversos objetivos práticos. A interface permite visualizar em tempo real o valor lido pelo sensor LDR, possibilita ativar ou desativar manualmente o modo noturno, e oferece a capacidade de ajustar diversos parâmetros do semáforo, incluindo o tempo de duração de cada fase (verde, amarelo e vermelho) e os valores de threshold utilizados para detecção.
Para complementar a visualização dos dados, foi adicionado um display LCD físico ao sistema, que exibe em tempo real as informações mais relevantes diretamente no hardware, como o valor atual do sensor LDR, o estado operacional do semáforo e o modo ativo (diurno ou noturno). Esta adição proporciona monitoramento local imediato, eliminando a necessidade de acesso constante ao dashboard online para verificações rápidas do sistema.
As funcionalidades da interface incluem um dashboard de telemetria que exibe continuamente o valor lido pelo sensor LDR, proporcionando feedback visual imediato sobre as condições de luminosidade. Além disso, um sistema de controle manual permite ao usuário ativar o modo noturno a qualquer momento, forçar estados específicos dos semáforos ou pausar completamente o ciclo de operação, oferecendo flexibilidade total no gerenciamento do sistema.


