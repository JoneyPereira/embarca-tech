#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "httpd.h" // Biblioteca para servidor HTTP básico

// Definição dos pinos
#define RELÉ_MOTOR 8
#define LED_LIGADO 4
#define LED_DESLIGADO 5

// Estado do motor
bool motor_ligado = false;

// Função para alternar o estado do motor
void alterar_estado_motor(bool ligar) {
    motor_ligado = ligar;
    gpio_put(RELÉ_MOTOR, ligar);
    gpio_put(LED_LIGADO, ligar);
    gpio_put(LED_DESLIGADO, !ligar);
}

// Manipulador para a rota "/motor" (POST)
void motor_handler(http_request_t *req, http_response_t *res) {
    if (req->method == HTTP_POST) {
        // Parse do corpo da requisição (espera um JSON: {"status": "on" ou "off"})
        if (strstr(req->body, "\"status\": \"on\"")) {
            alterar_estado_motor(true);
            http_response_set_body(res, "{\"status\": \"success\", \"message\": \"Motor ligado\"}");
        } else if (strstr(req->body, "\"status\": \"off\"")) {
            alterar_estado_motor(false);
            http_response_set_body(res, "{\"status\": \"success\", \"message\": \"Motor desligado\"}");
        } else {
            http_response_set_status(res, 400);
            http_response_set_body(res, "{\"error\": \"Parâmetro inválido\"}");
        }
    } else {
        http_response_set_status(res, 405);
        http_response_set_body(res, "{\"error\": \"Método não permitido\"}");
    }
}

// Manipulador para a rota "/status" (GET)
void status_handler(http_request_t *req, http_response_t *res) {
    char status[128];
    snprintf(status, sizeof(status), 
        "{\"motor\": \"%s\"}", motor_ligado ? "on" : "off");
    http_response_set_body(res, status);
}

// Configuração do servidor HTTP
void configurar_servidor_http() {
    httpd_init();
    httpd_add_route("/motor", HTTP_POST, motor_handler);
    httpd_add_route("/status", HTTP_GET, status_handler);
    printf("Servidor HTTP configurado.\n");
}

int main() {
    stdio_init_all();

    // Inicializar Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    // Conectar ao Wi-Fi
    const char *ssid = "SEU_SSID";
    const char *password = "SUA_SENHA";
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro ao conectar ao Wi-Fi\n");
        return -1;
    }
    printf("Conectado ao Wi-Fi\n");

    // Configuração dos pinos
    gpio_init(RELÉ_MOTOR);
    gpio_set_dir(RELÉ_MOTOR, GPIO_OUT);
    gpio_init(LED_LIGADO);
    gpio_set_dir(LED_LIGADO, GPIO_OUT);
    gpio_init(LED_DESLIGADO);
    gpio_set_dir(LED_DESLIGADO, GPIO_OUT);

    // Configuração do servidor HTTP
    configurar_servidor_http();

    // Loop principal
    while (1) {
        httpd_poll(); // Processa as requisições HTTP
        sleep_ms(10);
    }

    // Encerrar Wi-Fi
    cyw43_arch_deinit();
    return 0;
}