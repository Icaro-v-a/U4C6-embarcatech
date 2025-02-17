#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2818b.pio.h"
#include <string.h>
#include "includes/funcoesMatriz.h"
#include "includes/convertePixels.h"
#include "animacoes/contagem.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definições para comunicação I2C e hardware
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Definições de pinos para botões e LEDs
#define BTN_A 5 // Definindo o pino do botão A
#define BTN_B 6 // Definindo o pino do botão B
#define LED_GREEN 11 //Definindo o pino do LED verde
#define LED_BLUE 12 //Definindo o pino do LED azul

// Definições para a matriz de LEDs
#define MATRIX_ROWS 5        // Número de linhas da matriz de LEDs
#define MATRIX_COLS 5        // Número de colunas da matriz de LEDs
#define MATRIX_DEPTH 3       // Profundidade da matriz de LEDs (RGB)

#define DEBOUNCE_TIME_MS 200 // Tempo de debounce em milissegundos (evita múltiplos registros de pressionamento do botão)

// Variáveis globais
volatile uint8_t btn_counter = 10; // Contador para a matriz de LEDs
absolute_time_t last_time_a = 0;   // Variável para armazenar o último tempo do botão A
absolute_time_t last_time_b = 0;   // Variável para armazenar o último tempo do botão B
ssd1306_t ssd; // Estrutura para o display OLED     
bool cor = true; // Controle da cor do display

// Função de interrupção para lidar com o pressionamento dos botões
void gpio_irq_handler(uint gpio, uint32_t events)
{
    absolute_time_t current_time = get_absolute_time(); // Obtém o tempo atual

    // Verifica se o botão A foi pressionado e se o debounce foi respeitado
    if (gpio == BTN_A && absolute_time_diff_us(last_time_a, current_time) > DEBOUNCE_TIME_MS * 1000)
    {
        if(gpio_get(LED_GREEN) == 0){
            gpio_put(LED_GREEN, 1);
            ssd1306_fill(&ssd, !cor); 
            printf("LED verde ligado!\n");
            ssd1306_draw_string(&ssd, "LED verde on", 8, 10); // Desenha uma string  
        } else {
            gpio_put(LED_GREEN, 0);
            ssd1306_fill(&ssd, !cor); 
            printf("LED verde desligado!\n");
            ssd1306_draw_string(&ssd, "LED verde off", 8, 10); // Desenha uma string 
        }
        ssd1306_send_data(&ssd); // Atualiza o display
        last_time_a = current_time; // Atualiza o último tempo de pressionamento do botão A
    }

    // Verifica se o botão B foi pressionado e se o debounce foi respeitado
    if (gpio == BTN_B && absolute_time_diff_us(last_time_b, current_time) > DEBOUNCE_TIME_MS * 1000)
    {
        if(gpio_get(LED_BLUE) == 0){
            gpio_put(LED_BLUE, 1);
            ssd1306_fill(&ssd, !cor);
            printf("LED azul ligado!\n");
            ssd1306_draw_string(&ssd, "LED azul on", 8, 10); // Desenha uma string
        } else {
            gpio_put(LED_BLUE, 0);
            ssd1306_fill(&ssd, !cor);
            printf("LED azul desligado!\n");
            ssd1306_draw_string(&ssd, "LED azul off", 8, 10); // Desenha uma string
        }
        ssd1306_send_data(&ssd); // Atualiza o display
        last_time_b = current_time; // Atualiza o último tempo de pressionamento do botão B
    }
}

// Função para atualizar a matriz de LEDs com base no contador
void mudarValor(uint8_t btn_counter, npLED_t leds[], int rgb_matrix[MATRIX_ROWS][MATRIX_COLS][MATRIX_DEPTH])
{
    convertARGBtoMatriz(contagem_regressiva[btn_counter], rgb_matrix); // Converte o valor do contador para a matriz RGB
    spriteWirite(rgb_matrix, leds);                                    // Escreve o valor da matriz na estrutura de LEDs
    matrizWrite(leds);                                                 // Atualiza a matriz de LEDs fisicamente
}

int main()
{
    stdio_init_all(); // Inicializa a entrada e saída padrão

    // Inicialização do barramento I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line

    // Inicialização do display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    ssd1306_fill(&ssd, false); // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_send_data(&ssd); // Atualiza o display

    // Inicializa a matriz de LEDs
    npLED_t leds[LED_COUNT];
    int rgb_matrix[MATRIX_ROWS][MATRIX_COLS][MATRIX_DEPTH];
    matrizInit(LED_PIN, leds); // Inicializa os LEDs

     // Configuração dos LEDs
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, 0);

    // Configuração do botão A
    gpio_init(BTN_A);                                                                       // Inicializa o pino do botão A
    gpio_set_dir(BTN_A, GPIO_IN);                                                           // Configura o pino como entrada
    gpio_pull_up(BTN_A);                                                                    // Ativa o resistor pull-up para o botão
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Configura a interrupção para o botão A

    // Configuração do botão B
    gpio_init(BTN_B);                                                                       // Inicializa o pino do botão B
    gpio_set_dir(BTN_B, GPIO_IN);                                                           // Configura o pino como entrada
    gpio_pull_up(BTN_B);                                                                    // Ativa o resistor pull-up para o botão
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Configura a interrupção para o botão B

    
    // Loop principal
    while (true)
    {
        if (stdio_usb_connected()) // Certifica-se de que o USB está conectado
        { 
            char c;
            if (scanf("%c", &c) == 1) // Lê caractere da entrada padrão
            { 
                ssd1306_fill(&ssd, !cor);             // Limpa o display
                ssd1306_draw_string(&ssd, &c, 60, 30); // Desenha um o caractere digitado
                ssd1306_send_data(&ssd);              // Atualiza o display
                switch (c)
                {
                case '0':
                    btn_counter = 0;
                    break;
                case '1':
                    btn_counter = 1;
                    break;
                case '2':
                    btn_counter = 2;
                    break;
                case '3':
                    btn_counter = 3;
                    break;
                case '4':
                    btn_counter = 4;
                    break;
                case '5':
                    btn_counter = 5;
                    break;
                case '6':
                    btn_counter = 6;
                    break;
                case '7':
                    btn_counter = 7;
                    break;
                case '8':
                    btn_counter = 8;
                    break;
                case '9':
                    btn_counter = 9;
                    break;
                default:
                    btn_counter = 10;
                }
            }
        }
        sleep_ms(40);

        // Atualiza a matriz de LEDs com base no valor do contador
        mudarValor(btn_counter, leds, rgb_matrix);
    }
}