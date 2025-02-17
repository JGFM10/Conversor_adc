#include <stdlib.h>
#include <stdio.h> //biblioteca padrão da linguagem C
#include <stdint.h>
#include "pico/stdlib.h" //subconjunto central de bibliotecas do SDK Pico
#include "pico/time.h" //biblioteca para gerenciamento de tempo
#include "hardware/irq.h" //biblioteca para gerenciamento de interrupções
#include "hardware/pwm.h" //biblioteca para controlar o hardware de PWM
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/adc.h"     

// Definição Pinos I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Definição dos Botões e RGB
#define BUTTON_A 5  // Pino do botão A
#define BUTTON_B 6  // Pino do botão B
#define LED_VERMELHO 13    // Pino do LED Vermelho
#define LED_VERDE 11    // Pino do LED Verde
#define LED_AZUL 12    // Pino do LED Azul
#define JOYSTICK_Y_PIN 26  // GPIO para eixo X
#define JOYSTICK_X_PIN 27  // GPIO para eixo Y
#define JOYSTICK_PB 22 // GPIO para botão do Joystick

//Configuração Frequência do PWM
const uint16_t WRAP_PERIOD = 25000; //valor máximo do contador - WRAP
const float PWM_DIVISER = 100.0; //divisor do clock para o PWM

// Variáveis globais
static volatile uint a = 1;
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)
volatile bool led_verde_state = false;  // Estado do LED Verde
volatile bool pwm_state = true;   // Estado do LED Azul

// Variáveis para limitar o valor de x e de y
uint16_t t = 0;
uint16_t R1 = 1;
uint16_t R2 = 60;
uint16_t R3 = 1;
uint16_t R4 = 124;
uint16_t RECT_Y_MIN = 4;
uint16_t RECT_Y_MAX = 52;
uint16_t RECT_X_MIN = 4;
uint16_t RECT_X_MAX = 116;

// Variável para controlar a espessura das bordas
uint8_t border_thickness = 0; // Espessura inicial das bordas
bool increasing = true; // Direção do aumento/redução da espessura

// Declaração de funções
void inoutput_init();
static void gpio_irq_handler(uint gpio, uint32_t events);
void pwm_setup();
void ssd1306_thick_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, uint8_t thickness);

int main()
{
    // Inicializa comunicação USB CDC para monitor serial
    stdio_init_all(); 
    // Inicializa entradas e saídas.
    inoutput_init();
    // Inicializa o PWM 
    pwm_setup();

    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  
    // Variável de Leitura do adc
    uint16_t adc_value_y;
    uint16_t adc_value_x;  

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
   
    //Variáveis Joystick
    bool cor = true;
    char letra = '\0';
    uint32_t last_print_time = 0; 
    uint16_t last_y = 40;
    uint16_t last_x = 40;
    uint16_t new_y = last_y;
    uint16_t new_x = last_x;
    bool Controle_Borda = led_verde_state;


    //Variáveis PWM
    int pwm_x=0;
    int pwm_y=0;

    while (true)
    {
        cor = !cor;
        // Atualiza o conteúdo do display com animações
        adc_select_input(0); // Seleciona o ADC para eixo y. O pino 26 como entrada analógica
        adc_value_y = 4095 - adc_read();
        adc_select_input(1); // Seleciona o ADC para eixo x. O pino 27 como entrada analógica
        adc_value_x = adc_read();

        // Calcula as novas coordenadas
        int new_y = adc_value_y / 64; // Ajuste a escala conforme necessário
        int new_x = adc_value_x / 32; // Ajuste a escala conforme necessário

        // Aplica os limites para que o caractere não encoste no retângulo
        new_y = (new_y < RECT_Y_MIN) ? RECT_Y_MIN : (new_y > RECT_Y_MAX) ? RECT_Y_MAX : new_y;
        new_x = (new_x < RECT_X_MIN) ? RECT_X_MIN : (new_x > RECT_X_MAX) ? RECT_X_MAX : new_x;

        // Apaga o caractere anterior
        ssd1306_draw_char(&ssd, ' ', last_x, last_y);

        // Desenha o caractere na nova posição
        ssd1306_draw_char(&ssd, '~', new_x, new_y);

        // Atualiza a posição anterior
        last_y = new_y;
        last_x = new_x;

        // Atualiza o display
        ssd1306_send_data(&ssd);

        if (Controle_Borda!=led_verde_state)
        {
        // Desenha o retângulo com bordas grossas
        ssd1306_fill(&ssd, false); // Limpa o display
        ssd1306_thick_rect(&ssd, R1, R3, R4 - R3, R2 - R1, true, border_thickness);
        ssd1306_send_data(&ssd); // Atualiza o display

        Controle_Borda = led_verde_state;
        }

        // PWM A PARTIR DAQUI
         pwm_x = abs((int16_t)(adc_value_x - 2090));
         pwm_y = abs((int16_t)(adc_value_y - 2090));

         if (pwm_state==true)
         {
         if(pwm_x>20 && pwm_y>20)
        {
        pwm_set_gpio_level(LED_VERMELHO, ((pwm_x*25000)/2090)); //define o nível atual do PWM (duty cycle)
        pwm_set_gpio_level(LED_AZUL, ((pwm_y*25000)/2090)); //definir o cico de trabalho (duty cycle) do pwm
        }
        else
        {
          pwm_set_gpio_level(LED_VERMELHO, 0); //define o nível atual do PWM (duty cycle)
          pwm_set_gpio_level(LED_AZUL, 0); //definir o cico de trabalho (duty cycle) do pwm
        }
      }
      else
      {
        pwm_set_gpio_level(LED_VERMELHO, 0); //define o nível atual do PWM (duty cycle)
        pwm_set_gpio_level(LED_AZUL, 0); //definir o cico de trabalho (duty cycle) do pwm
      }
    }
}

// Inicializa os leds e botões
void inoutput_init()
{
    // Input
    gpio_init(LED_VERMELHO);
    gpio_init(LED_AZUL);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERMELHO, false);
    gpio_put(LED_AZUL, false);
    gpio_put(LED_VERDE, false);

    // Configura os pinos dos botões como entrada
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);  // Ativa o resistor pull-up interno para o botão A

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);  // Ativa o resistor pull-up interno para o botão B

    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB); // Ativa o resistor pull-up interno para o botão do JOYSTICK

    // Inicializa o ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);  
}

// Função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 200000) // 200 ms de debounce
    {
        last_time = current_time; // Atualiza o tempo do último evento

        if (gpio == JOYSTICK_PB) { 
            led_verde_state = !led_verde_state;  // Alterna estado do LED Verde
            gpio_put(LED_VERDE, led_verde_state);
            printf("Botão do Joystick pressionado: LED_VERDE = %d\n", led_verde_state);

                     // Aumenta ou reduz a espessura das bordas
                     if (increasing) {
                      border_thickness++;
                      if (border_thickness >= 5) { // Limite máximo de espessura
                          increasing = false;
                      }
                  } else {
                      border_thickness--;
                      if (border_thickness == 0) { // Limite mínimo de espessura
                          increasing = true;
                      }
                  }
            // Atualiza as dimensões do retângulo
            R1 = border_thickness;
            R2 = 64 - border_thickness;
            R3 = border_thickness;
            R4 = 128 - border_thickness;
            // Atualiza os limites para o caractere
            RECT_Y_MIN = R1 + 3;
            RECT_Y_MAX = 52 - border_thickness;
            RECT_X_MIN = R3 + 3;
            RECT_X_MAX = 116-border_thickness;
        }

        if (gpio == BUTTON_A) { 
          pwm_state = !pwm_state;  // Alterna estado do LED Verde
          printf("Botão A pressionado: PWM = %d\n", pwm_state);
        }
    }
}

// Função para configurar o módulo PWM
void pwm_setup()
{
    gpio_set_function(LED_AZUL, GPIO_FUNC_PWM); // Configura o pino do servomotor para função PWM
    gpio_set_function(LED_VERMELHO, GPIO_FUNC_PWM); // Configura o pino do LED RGB para função PWM

    uint slice_num_azul = pwm_gpio_to_slice_num(LED_AZUL); // Obtém o número do slice PWM do pino do servomotor
    uint slice_num_vermelho = pwm_gpio_to_slice_num(LED_VERMELHO); // Obtém o número do slice PWM do pino do servomotor

    pwm_set_clkdiv(slice_num_azul, PWM_DIVISER); //define o divisor de clock do PWM
    pwm_set_clkdiv(slice_num_vermelho, PWM_DIVISER); //define o divisor de clock do PWM

    pwm_set_wrap(slice_num_azul, WRAP_PERIOD); //definir o valor de wrap
    pwm_set_wrap(slice_num_vermelho, WRAP_PERIOD); //definir o valor de wrap

    pwm_set_gpio_level(LED_AZUL, 0); //definir o cico de trabalho (duty cycle) do pwm
    pwm_set_gpio_level(LED_VERMELHO, 0); //definir o cico de trabalho (duty cycle) do pwm

    pwm_set_enabled(slice_num_azul, true); //habilita o pwm no slice correspondente
    pwm_set_enabled(slice_num_vermelho, true); //habilita o pwm no slice correspondente
}

// Função para desenhar um retângulo com bordas grossas
void ssd1306_thick_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, uint8_t thickness) {
    for (uint8_t t = 0; t < thickness; ++t) {
        // Desenha o retângulo com bordas cada vez mais grossas
        ssd1306_rect(ssd, top - t, left - t, width + 2 * t, height + 2 * t, value, false);
    }
}