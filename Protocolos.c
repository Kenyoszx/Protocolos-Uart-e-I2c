// Inclusão das Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

// Definição dos Pinos
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define I2C_SDA 14
#define I2C_SCL 15
#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12
#define LED_PIN_RED 13
#define MATRIZ_LEDS 7
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Definição das Constantes
#define LED_COUNT 25
#define UART_ID uart0
#define BAUD_RATE 115200
#define I2C_PORT i2c0

// Protótipos das Funções
void init();
void npInit(uint);
void npSetLED(const uint, const uint8_t, const uint8_t, const uint8_t);
void npClear();
void npWrite();
void show_number();
void blink();
void NUMBER_0();
void NUMBER_1();
void NUMBER_2();
void NUMBER_3();
void NUMBER_4();
void NUMBER_5();
void NUMBER_6();
void NUMBER_7();
void NUMBER_8();
void NUMBER_9();
static void gpio_irq_handler(uint gpio, uint32_t events);

int main()
{
    stdio_init_all();
    init();
    while (true)
    {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}

void init()
{

    // inicialização dos pinos
    npInit(MATRIZ_LEDS);
    npClear();
    npWrite();

    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_put(LED_PIN_GREEN, 0);

    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_put(LED_PIN_BLUE, 0);

    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_put(LED_PIN_RED, 0);

    // inicialização dos botões com resistor interno pull-up
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Rotina de Interrupção

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Rotina de Interrupção
    
    // inicializa a comunicação I2C
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa a comunicação Uart
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}
