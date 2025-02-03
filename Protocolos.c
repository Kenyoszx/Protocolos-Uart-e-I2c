// Inclusão das Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

//Definição dos Pinos
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

int main()
{
    
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
