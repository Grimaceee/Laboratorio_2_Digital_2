#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

// Pines de botones y LEDs
#define BUTTON_INCREASE_PIN GPIO_NUM_25
#define BUTTON_DECREASE_PIN GPIO_NUM_26
#define BUTTON_MODE_PIN GPIO_NUM_27
#define LED1_PIN GPIO_NUM_2
#define LED2_PIN GPIO_NUM_4
#define LED3_PIN GPIO_NUM_16
#define LED4_PIN GPIO_NUM_17

// Variables globales
volatile int mode = 0;  // 0: Binario, 1: Decimal
volatile int count = 0;

// Variables para debounce y estado de botones
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
bool lastButtonModeState = false;
bool buttonModeStateChanged = false;

// Función para configurar los pines GPIO
void init_gpio() {
    gpio_config_t io_conf;

    // Configurar pines de LEDs como salidas
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED1_PIN) | (1ULL << LED2_PIN) | (1ULL << LED3_PIN) | (1ULL << LED4_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // Configurar pines de botones como entradas
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_INCREASE_PIN) | (1ULL << BUTTON_DECREASE_PIN) | (1ULL << BUTTON_MODE_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Configurar botón de modo con pull-down
    gpio_set_pull_mode(BUTTON_MODE_PIN, GPIO_PULLDOWN_ONLY);

    // Configurar interrupciones para los botones
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_INCREASE_PIN, handleButtonIncrease, (void *)BUTTON_INCREASE_PIN);
    gpio_isr_handler_add(BUTTON_DECREASE_PIN, handleButtonDecrease, (void *)BUTTON_DECREASE_PIN);
    gpio_isr_handler_add(BUTTON_MODE_PIN, handleButtonMode, (void *)BUTTON_MODE_PIN);
}

// Función de ISR para el botón de aumentar
static void IRAM_ATTR handleButtonIncrease(void *arg) {
    if ((xTaskGetTickCount() - lastDebounceTime) > debounceDelay) {
        lastDebounceTime = xTaskGetTickCount();
        if (mode == 0) {
            if (count < 15) count++;
        } else {
            if (count < 4) count++;
        }
    }
}

// Función de ISR para el botón de disminuir
static void IRAM_ATTR handleButtonDecrease(void *arg) {
    if ((xTaskGetTickCount() - lastDebounceTime) > debounceDelay) {
        lastDebounceTime = xTaskGetTickCount();
        if (count > 0) count--;
    }
}

// Función de ISR para el botón de modo
static void IRAM_ATTR handleButtonMode(void *arg) {
    bool currentButtonModeState = gpio_get_level(BUTTON_MODE_PIN);
    if (currentButtonModeState != lastButtonModeState) {
        lastButtonModeState = currentButtonModeState;
        if (currentButtonModeState == true && (xTaskGetTickCount() - lastDebounceTime) > debounceDelay) {
            lastDebounceTime = xTaskGetTickCount();
            buttonModeStateChanged = true;
        }
    }
}

void app_main(void) {
    // Inicializar GPIO
    init_gpio();

    // Inicializar comunicación serial
    ESP_LOGI("app_main", "Iniciando...");

    while (true) {
        // Verificar si el estado del botón de modo ha cambiado
        if (buttonModeStateChanged) {
            buttonModeStateChanged = false;
            mode = !mode; // Cambiar modo
            count = 0;    // Resetear el contador al cambiar de modo
            ESP_LOGI("app_main", "Modo cambiado a: %s", mode == 0 ? "Binario" : "Decimal");
        }

        // Actualizar LEDs según el modo y el contador
        if (mode == 0) {
            // Modo binario
            gpio_set_level(LED1_PIN, (count & 0x01) ? 1 : 0);
            gpio_set_level(LED2_PIN, (count & 0x02) ? 1 : 0);
            gpio_set_level(LED3_PIN, (count & 0x04) ? 1 : 0);
            gpio_set_level(LED4_PIN, (count & 0x08) ? 1 : 0);
        } else {
            // Modo decimal
            gpio_set_level(LED1_PIN, (count >= 1) ? 1 : 0);
            gpio_set_level(LED2_PIN, (count >= 2) ? 1 : 0);
            gpio_set_level(LED3_PIN, (count >= 3) ? 1 : 0);
            gpio_set_level(LED4_PIN, (count >= 4) ? 1 : 0);
        }

        // Imprimir el valor del contador en el monitor serial
        ESP_LOGI("app_main", "Modo: %s, Valor: %d", mode == 0 ? "Binario" : "Decimal", count);

        // Delay para evitar desbordar el monitor serial
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}