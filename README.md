# Comunicação via protocolos UART e I2C
O algoritmo implementa a comunicação do microcontrolador rp 2040 com alguns periféricos (display SSD1306 e Serial Monitor) Através dos protocolos de comunicação UART e I2C.
O algoritmo é compatível com a plataforma Educacional BitDogLab v6.3, Microcontrolador RP2040 e periféricos, e com o simulador online WOKWI.

## Funcionalidades

- Leitura de caractéres de comandos através do Serial Monitor.
- Quando um número entre 0 e 9 for digitado, um símbolo correspondente ao número será exibido na matriz 5x5 WS2812.
- Ao apertar o botão A, o LED RGB Verde muda de estado e uma mensagem informando o estado é exibida
  tanto no serial monitor quanto no display 128 x 64.
- Ao apertar o botão B, o LED RGB Azul muda de estado e uma mensagem informando o estado é exibida
  tanto no serial monitor quanto no display 128 x 64.


## Hardware Necessário

- 1 Raspberry Pi Pico W (rp2040).
- 2 Botões.
- Matriz de Leds 5x5 WS2812B.
- Display 128 x 64 SSD1306.
- 3 LEDs (Vermelho, Azul, Verde) ou LED RGB.
- Resistores apropriados para os LEDs.
- Fios de conexão.

## Pinagem

- Botão A: Pino GPIO 5
- Botão B: Pino GPIO 6
- Matriz de LEDs WS2812B: Pino GPIO 7
- Display SSD1306: GPIO 14 e 15
- LED Vermelho: Pino GPIO 13
- LED Azul: Pino GPIO 12
- LED Verde: Pino GPIO 11

## Configuração do Ambiente

Antes de começar, certifique-se de que você tenha o ambiente de desenvolvimento do **Raspberry Pi Pico** configurado corretamente. Siga as instruções do [Raspberry Pi Pico SDK](https://www.raspberrypi.org/documentation/rp2040/getting-started/) para configurar o SDK e as ferramentas de compilação. Para visualição do projeto no simulador também é importante configurar o simuldor WOKWI no VS code, conforme as instruções no link [WOKWI para VS code](https://docs.wokwi.com/pt-BR/vscode/getting-started).

## Compilação e Upload

1. Compile o código usando o ambiente de desenvolvimento configurado.
2. Após a compilação, faça o upload do código para o seu **Raspberry Pi Pico**.

## Tecnologias Utilizadas

- **C/C++**
- **Raspberry Pi Pico SDK**
- **Simulador Online WOKWI**
- **Protocolo de Comunicação I2C**
- **Protocolo de Comunicação UART**

## Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para abrir um problema ou enviar um pull request.
