#include <stdio.h>
#include "pico/stdlib.h"
#include "ws2818b.pio.h"
#include "includes/funcoesMatriz.h"
#include "includes/convertePixels.h"
#include "animacoes/contagem.h"

#define BTN_A 5               // Definindo o pino do botão A
#define BTN_B 6               // Definindo o pino do botão B
#define LED_RED 13            // Definindo o pino do LED vermelho
#define MATRIX_ROWS 5         // Número de linhas da matriz de LEDs
#define MATRIX_COLS 5         // Número de colunas da matriz de LEDs
#define MATRIX_DEPTH 3        // Profundidade da matriz de LEDs (RGB)
#define DEBOUNCE_TIME_MS 200  // Tempo de debounce em milissegundos (evita múltiplos registros de pressionamento do botão)

volatile uint8_t btn_counter = 0; // Contador para a matriz, que será modificado pelos botões
absolute_time_t last_time_a = 0;  // Variável para armazenar o último tempo do botão A
absolute_time_t last_time_b = 0;  // Variável para armazenar o último tempo do botão B

// Função de interrupção para lidar com o pressionamento dos botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    absolute_time_t current_time = get_absolute_time();  // Obtém o tempo atual

    // Verifica se o botão A foi pressionado e se o debounce foi respeitado
    if (gpio == BTN_A && absolute_time_diff_us(last_time_a, current_time) > DEBOUNCE_TIME_MS * 1000) {
        if (btn_counter < 9) {  // Limita o contador a no máximo 9
            btn_counter++;  // Incrementa o contador
            printf("Botão A pressionado: contador = %d\n", btn_counter);  // Exibe o valor do contador
        }
        last_time_a = current_time;  // Atualiza o último tempo de pressionamento do botão A
    }

    // Verifica se o botão B foi pressionado e se o debounce foi respeitado
    if (gpio == BTN_B && absolute_time_diff_us(last_time_b, current_time) > DEBOUNCE_TIME_MS * 1000) {
        if (btn_counter > 0) {  // Limita o contador a no mínimo 0
            btn_counter--;  // Decrementa o contador
            printf("Botão B pressionado: contador = %d\n", btn_counter);  // Exibe o valor do contador
        }
        last_time_b = current_time;  // Atualiza o último tempo de pressionamento do botão B
    }
}

// Função para atualizar a matriz de LEDs com base no contador
void mudarValor(uint8_t btn_counter, npLED_t leds[], int rgb_matrix[MATRIX_ROWS][MATRIX_COLS][MATRIX_DEPTH]) {
    convertARGBtoMatriz(contagem_regressiva[btn_counter], rgb_matrix);  // Converte o valor do contador para a matriz RGB
    spriteWirite(rgb_matrix, leds);  // Escreve o valor da matriz na estrutura de LEDs
    matrizWrite(leds);  // Atualiza a matriz de LEDs fisicamente
}

int main() {
    stdio_init_all();  // Inicializa a entrada e saída padrão

    // Inicializa a matriz de LEDs
    npLED_t leds[LED_COUNT];
    int rgb_matrix[MATRIX_ROWS][MATRIX_COLS][MATRIX_DEPTH];
    matrizInit(LED_PIN, leds);  // Inicializa os LEDs

    // Inicializa o LED vermelho
    gpio_init(LED_RED);  // Inicializa o pino do LED vermelho
    gpio_set_dir(LED_RED, GPIO_OUT);  // Configura o pino como saída
    gpio_put(LED_RED, 0);  // Desliga o LED vermelho inicialmente

    // Configuração do botão A
    gpio_init(BTN_A);  // Inicializa o pino do botão A
    gpio_set_dir(BTN_A, GPIO_IN);  // Configura o pino como entrada
    gpio_pull_up(BTN_A);  // Ativa o resistor pull-up para o botão
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);  // Configura a interrupção para o botão A

    // Configuração do botão B
    gpio_init(BTN_B);  // Inicializa o pino do botão B
    gpio_set_dir(BTN_B, GPIO_IN);  // Configura o pino como entrada
    gpio_pull_up(BTN_B);  // Ativa o resistor pull-up para o botão
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);  // Configura a interrupção para o botão B

    // Exibe o número inicial na matriz de LEDs
    mudarValor(btn_counter, leds, rgb_matrix);

    // Loop principal
    while (true) {
        // Pisca o LED vermelho continuamente
        gpio_put(LED_RED, 1);  // Acende o LED vermelho
        sleep_ms(100);  // Espera por 100ms
        gpio_put(LED_RED, 0);  // Apaga o LED vermelho
        sleep_ms(100);  // Espera por 100ms

        // Atualiza a matriz de LEDs com base no valor do contador
        mudarValor(btn_counter, leds, rgb_matrix);
    }
}