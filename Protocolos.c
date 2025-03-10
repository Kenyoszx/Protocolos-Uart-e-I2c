// Inclusão das Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

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
#define I2C_PORT i2c1
#define ENDERECO 0x3C

// Variáveis globais
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)
static volatile bool ledgreen_active = false, ledblue_active = false, cor = true;
static volatile char caracter;
ssd1306_t ssd;
struct pixel_t
{
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel RGB.
};
typedef struct pixel_t npLED_t; // Definindo a struct pixel_t como um tipo "npLED_t" para abstrair complexidade
npLED_t leds[LED_COUNT];        // buffer de pixels que formam a matriz.
PIO np_pio;                     // Variáveis para uso da máquina PIO.
uint sm;

// Protótipos das Funções
void init();
void npInit(uint);
void npSetLED(const uint, const uint8_t, const uint8_t, const uint8_t);
void npClear();
void npWrite();
void display_state();
void showNumber();
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
    stdio_init_all(); // inicializa a entrada e saída padrão
    init();           // inicializa os pinos

    while (true)
    {
        if (stdio_usb_connected()) // verifica se o microcontrolador está conectado
        {
            // recebe um caracter via serial monitor
            if (scanf("%c", &caracter) == 1)
            {
                ssd1306_fill(&ssd,!cor);
                ssd1306_draw_string(&ssd,"Recebido: ",16,30);
                ssd1306_draw_char(&ssd,caracter,96,30);
                ssd1306_send_data(&ssd);
                printf("Recebido: '%c'\n", caracter); // Informa o Caracter recebido no serial monitor
                showNumber();                         // caso seja um número representa na matriz de leds 5x5
            }
        }
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
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa a comunicação Uart
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}
void npInit(uint pin)
{
    // Inicializa a máquina PIO para controle da matriz de LEDs.

    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;

    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0)
    {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }

    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    // Atribui uma cor RGB a um LED.

    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}
void npClear()
{
    // Limpa o buffer de pixels.

    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}
void npWrite()
{
    // Escreve os dados do buffer nos LEDs.

    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    // Configura a ação ao apertar o botão e implementa o Debouce

    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 200000) // 200 ms de debouncing
    {
        last_time = current_time; // Atualiza o tempo do último evento
        // Código Função:
        if (gpio == BUTTON_A_PIN)
        {
            // Troca o estado do LED e informa no Serial Monitor que o Botão A foi pressionado
            ledgreen_active = !ledgreen_active;
            gpio_put(LED_PIN_GREEN, ledgreen_active);
            printf("Button A foi pressionado: ");

            // Atualiza o conteúdo do display e do Serial monitor com mensagem informando estado do led
            display_state();
        }
        else if (gpio == BUTTON_B_PIN)
        {
            // Troca o estado do LED e informa no Serial Monitor que o Botão B foi pressionado
            ledblue_active = !ledblue_active;
            gpio_put(LED_PIN_BLUE, ledblue_active);
            printf("Button B foi pressionado: ");

            // Atualiza o conteúdo do displaye do Serial monitor com mensagem informando estado do led
            display_state();
        }
    }
}
void NUMBER_0()
{
    // Representa o Número 0 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(6, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(16, 49, 49, 0);
    npSetLED(18, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_1()
{
    // Representa o Número 1 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(7, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(16, 49, 49, 0);
    npSetLED(17, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npWrite();
}
void NUMBER_2()
{
    // Representa o Número 2 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(6, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(18, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_3()
{
    // Representa o Número 3 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(18, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_4()
{
    // Representa o Número 4 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(16, 49, 49, 0);
    npSetLED(18, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_5()
{
    // Representa o Número 5 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(16, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_6()
{
    // Representa o Número 6 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(6, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(16, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_7()
{
    // Representa o Número 7 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(18, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_8()
{
    // Representa o Número 8 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(6, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(16, 49, 49, 0);
    npSetLED(18, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void NUMBER_9()
{
    // Representa o Número 9 Na Matriz 5x5

    npClear();
    npSetLED(1, 49, 49, 0);
    npSetLED(2, 49, 49, 0);
    npSetLED(3, 49, 49, 0);
    npSetLED(8, 49, 49, 0);
    npSetLED(11, 49, 49, 0);
    npSetLED(12, 49, 49, 0);
    npSetLED(13, 49, 49, 0);
    npSetLED(16, 49, 49, 0);
    npSetLED(18, 49, 49, 0);
    npSetLED(21, 49, 49, 0);
    npSetLED(22, 49, 49, 0);
    npSetLED(23, 49, 49, 0);
    npWrite();
}
void display_state()
{
    // Atualiza o Display e a UART de Acordo com o estado do(s) LED(s)

    ssd1306_fill(&ssd, !cor);                     // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_send_data(&ssd);                      // Atualiza o display
    if (ledblue_active && ledgreen_active)
    {
        printf("Ambos os leds Ligados\n");
        ssd1306_fill(&ssd, !cor);                           // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);       // Desenha um retângulo
        ssd1306_draw_string(&ssd, "Ambos os LEDs", 15, 10); // Desenha uma string
        ssd1306_draw_string(&ssd, "Agora Estao:", 20, 30);  // Desenha uma string
        ssd1306_draw_string(&ssd, "Ligados", 35, 48);       // Desenha uma string
        ssd1306_send_data(&ssd);                            // Atualiza o display
        return;
    }
    else if (ledblue_active)
    {
        printf("Led azul ligado\n");
        ssd1306_draw_string(&ssd, "LED Azul", 30, 10);    // Desenha uma string
        ssd1306_draw_string(&ssd, "Agora Esta:", 20, 30); // Desenha uma string
        ssd1306_draw_string(&ssd, "Ligado", 35, 48);      // Desenha uma string
        ssd1306_send_data(&ssd);                          // Atualiza o display
    }
    else if (ledgreen_active)
    {
        printf("Led verde ligado\n");
        ssd1306_draw_string(&ssd, "LED Verde", 30, 10);   // Desenha uma string
        ssd1306_draw_string(&ssd, "Agora Esta:", 20, 30); // Desenha uma string
        ssd1306_draw_string(&ssd, "Ligado", 35, 48);      // Desenha uma string
        ssd1306_send_data(&ssd);                          // Atualiza o display
    }
    else
    {
        printf("Ambos os leds Apagados\n");
        ssd1306_fill(&ssd, !cor);                           // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);       // Desenha um retângulo
        ssd1306_draw_string(&ssd, "Ambos os LEDs", 15, 10); // Desenha uma string
        ssd1306_draw_string(&ssd, "Agora Estao:", 20, 30);  // Desenha uma string
        ssd1306_draw_string(&ssd, "Apagados", 35, 48);      // Desenha uma string
        ssd1306_send_data(&ssd);                            // Atualiza o display
        return;
    }
}
void showNumber()
{
    // Caso Seja recebido um número pela UART representa ele na matriz de leds
    // Do contrário apaga a Matriz de LEDs

    switch (caracter)
    {
    case '0':
        NUMBER_0();
        break;
    case '1':
        NUMBER_1();
        break;
    case '2':
        NUMBER_2();
        break;
    case '3':
        NUMBER_3();
        break;
    case '4':
        NUMBER_4();
        break;
    case '5':
        NUMBER_5();
        break;
    case '6':
        NUMBER_6();
        break;
    case '7':
        NUMBER_7();
        break;
    case '8':
        NUMBER_8();
        break;
    case '9':
        NUMBER_9();
        break;
    default:
        npClear();
        npWrite();
        break;
    }
}