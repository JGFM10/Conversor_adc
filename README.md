# Explicação do Código

Este código é destinado ao uso com a placa Raspberry Pi Pico e implementa uma série de funcionalidades interativas utilizando LEDs, botões, um joystick e um display OLED SSD1306. Abaixo está uma breve explicação das principais funcionalidades:

Obs Link para o Vídeo Demonstrativo:https://youtu.be/M3Qta8NiVwQ

## Funcionalidades Principais

1. **Inicialização e Configuração:**
   - O código inicializa os pinos GPIO para LEDs, botões e o joystick.
   - Configura o ADC para leitura dos eixos X e Y do joystick.
   - Inicializa o display OLED SSD1306 via I2C.

2. **Controle de LEDs:**
   - O LED verde é controlado pelo botão do joystick, alternando seu estado a cada pressionamento.
   - Os LEDs vermelho e azul são controlados via PWM, com a intensidade variando de acordo com a posição do joystick.

3. **Interrupções:**
   - As interrupções são configuradas para os botões A, B e o botão do joystick.
   - O botão do joystick alterna o estado do LED verde e ajusta a espessura das bordas de um retângulo exibido no display.
   - O botão A alterna o estado do PWM, ligando ou desligando os LEDs vermelho e azul.

4. **Display OLED:**
   - O display exibe um retângulo com bordas de espessura variável, controlada pelo botão do joystick.
   - Um caractere '~' é movido pelo display de acordo com a posição do joystick, mantendo-se dentro dos limites do retângulo.

5. **PWM:**
   - O PWM é utilizado para controlar a intensidade dos LEDs vermelho e azul.
   - A intensidade do LED vermelho é controlada pelo eixo X do joystick, e a do LED azul pelo eixo Y.

## Estrutura do Código

- **inoutput_init():** Inicializa os pinos GPIO e o ADC.
- **gpio_irq_handler():** Manipula as interrupções dos botões, alterando estados dos LEDs e ajustando a espessura das bordas do retângulo.
- **pwm_setup():** Configura o hardware PWM para os LEDs vermelho e azul.
- **ssd1306_thick_rect():** Desenha um retângulo com bordas de espessura variável no display OLED.

## Como Funciona

- O joystick controla a posição do caractere '~' no display, esse caractere foi convertido em um quadrado 8x8 como pedido na atividade,  e a intensidade dos LEDs vermelho e azul.
- O botão do joystick alterna o estado do LED verde e ajusta a espessura das bordas do retângulo.
- O botão A liga ou desliga o PWM, controlando os LEDs vermelho e azul.

Este código é um exemplo de como integrar múltiplos periféricos em um projeto com a Raspberry Pi Pico, demonstrando o uso de GPIO, ADC, PWM, I2C e interrupções.
