#include <stdio.h>
#include "pico/stdlib.h"

// Definição dos pinos
#define CHAVE_MODOS 9  // Chave de três posições conectada a este pino
#define BOTAO_LIGAR 2
#define BOTAO_DESLIGAR 3
#define LED_LIGADO 4
#define LED_DESLIGADO 5
#define SENSOR_CAIXA_CHEIA 6
#define SENSOR_CAIXA_VAZIA 7
#define RELÉ_MOTOR 8
#define SENSOR_TERMICO 28

// Estados do sistema
typedef enum {
    DESLIGADO,
    MODO_MANUAL,
    MODO_AUTOMATICO
} modo_t;

modo_t modo_atual = DESLIGADO;

// Função para inicializar os pinos
void inicializar_pinos() {
    gpio_init(CHAVE_MODOS);
    gpio_set_dir(CHAVE_MODOS, GPIO_IN);
    gpio_pull_up(CHAVE_MODOS);

    gpio_init(BOTAO_LIGAR);
    gpio_set_dir(BOTAO_LIGAR, GPIO_IN);
    gpio_pull_up(BOTAO_LIGAR);

    gpio_init(BOTAO_DESLIGAR);
    gpio_set_dir(BOTAO_DESLIGAR, GPIO_IN);
    gpio_pull_up(BOTAO_DESLIGAR);

    gpio_init(LED_LIGADO);
    gpio_set_dir(LED_LIGADO, GPIO_OUT);

    gpio_init(LED_DESLIGADO);
    gpio_set_dir(LED_DESLIGADO, GPIO_OUT);

    gpio_init(SENSOR_CAIXA_CHEIA);
    gpio_set_dir(SENSOR_CAIXA_CHEIA, GPIO_IN);
    gpio_pull_up(SENSOR_CAIXA_CHEIA);

    gpio_init(SENSOR_CAIXA_VAZIA);
    gpio_set_dir(SENSOR_CAIXA_VAZIA, GPIO_IN);
    gpio_pull_up(SENSOR_CAIXA_VAZIA);

    gpio_init(RELÉ_MOTOR);
    gpio_set_dir(RELÉ_MOTOR, GPIO_OUT);

    gpio_init(SENSOR_TERMICO);
    gpio_set_dir(SENSOR_TERMICO, GPIO_IN);
}

// Função para verificar o estado da chave de modos
void atualizar_modo() {
    uint8_t estado_chave = gpio_get(CHAVE_MODOS);

    // Configuração da chave de três posições:
    // - Posição 0: DESLIGADO
    // - Posição 1: MODO_MANUAL
    // - Posição 2: MODO_AUTOMATICO
    switch (estado_chave) {
        case 0:
            modo_atual = DESLIGADO;
            break;
        case 1:
            modo_atual = MODO_MANUAL;
            break;
        case 2:
            modo_atual = MODO_AUTOMATICO;
            break;
        default:
            modo_atual = DESLIGADO;
    }
}

// Função para verificar o estado dos botões no modo manual
void verificar_modo_manual() {
    if (gpio_get(BOTAO_LIGAR) == 0) {
        gpio_put(RELÉ_MOTOR, 1);
        gpio_put(LED_LIGADO, 1);
        gpio_put(LED_DESLIGADO, 0);
    } else if (gpio_get(BOTAO_DESLIGAR) == 0) {
        gpio_put(RELÉ_MOTOR, 0);
        gpio_put(LED_LIGADO, 0);
        gpio_put(LED_DESLIGADO, 1);
    }
}

// Função para verificar o estado dos sensores no modo automático
void verificar_modo_automatico() {
    if (gpio_get(SENSOR_CAIXA_VAZIA) == 0) {
        gpio_put(RELÉ_MOTOR, 1);
        gpio_put(LED_LIGADO, 1);
        gpio_put(LED_DESLIGADO, 0);
    } else if (gpio_get(SENSOR_CAIXA_CHEIA) == 0) {
        gpio_put(RELÉ_MOTOR, 0);
        gpio_put(LED_LIGADO, 0);
        gpio_put(LED_DESLIGADO, 1);
    }
}

// Função para monitorar a temperatura do motor
void verificar_temperatura_motor() {
    if (gpio_get(SENSOR_TERMICO) == 1) {
        gpio_put(RELÉ_MOTOR, 0);
        gpio_put(LED_LIGADO, 0);
        gpio_put(LED_DESLIGADO, 1);
        printf("Alerta: Motor superaquecido! Motor desligado.\n");
    }
}

int main() {
    stdio_init_all();
    inicializar_pinos();

    printf("Sistema iniciado.\n");

    while (1) {
        atualizar_modo();

        switch (modo_atual) {
            case DESLIGADO:
                gpio_put(RELÉ_MOTOR, 0);
                gpio_put(LED_LIGADO, 0);
                gpio_put(LED_DESLIGADO, 1);
                break;

            case MODO_MANUAL:
                verificar_modo_manual();
                break;

            case MODO_AUTOMATICO:
                verificar_modo_automatico();
                break;
        }

        verificar_temperatura_motor();
        sleep_ms(100);
    }

    return 0;
}