#include <Arduino.h>

// Definición de pines
const int BUTTON_INCREASE_PIN = 25; // Botón de aumentar (pull-up)
const int BUTTON_DECREASE_PIN = 26; // Botón de disminuir (pull-up)
const int BUTTON_MODE_PIN = 27;     // Botón de modo (pull-down)
const int LED1_PIN = 2;
const int LED2_PIN = 4;
const int LED3_PIN = 16;
const int LED4_PIN = 17;

// Variables globales
volatile int mode = 0;  // 0: Binario, 1: Decimal
volatile int count = 0;

// Variables para debounce y estado de botones
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool lastButtonModeState = LOW;
bool buttonModeStateChanged = false;

// Funciones de manejo de botones
void handleButtonIncrease();
void handleButtonDecrease();
void handleButtonMode();

void setup() {
    // Iniciar el monitor serial
    Serial.begin(115200);

    // Configurar pines de los botones como entradas
    pinMode(BUTTON_INCREASE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DECREASE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_MODE_PIN, INPUT_PULLDOWN);

    // Configurar pines de los LEDs como salidas
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(LED3_PIN, OUTPUT);
    pinMode(LED4_PIN, OUTPUT);

    // Asignar interrupciones a los botones
    attachInterrupt(digitalPinToInterrupt(BUTTON_INCREASE_PIN), handleButtonIncrease, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUTTON_DECREASE_PIN), handleButtonDecrease, FALLING);
    attachInterrupt(digitalPinToInterrupt(BUTTON_MODE_PIN), handleButtonMode, CHANGE);
}

void loop() {
    // Verificar si el estado del botón de modo ha cambiado
    if (buttonModeStateChanged) {
        buttonModeStateChanged = false;
        mode = !mode; // Cambiar modo
        count = 0;    // Resetear el contador al cambiar de modo
        Serial.print("Modo cambiado a: ");
        Serial.println(mode == 0 ? "Binario" : "Decimal");
    }

    // Actualizar LEDs según el modo y el contador
    if (mode == 0) {
        // Modo binario
        digitalWrite(LED1_PIN, (count & 0x01) ? HIGH : LOW);
        digitalWrite(LED2_PIN, (count & 0x02) ? HIGH : LOW);
        digitalWrite(LED3_PIN, (count & 0x04) ? HIGH : LOW);
        digitalWrite(LED4_PIN, (count & 0x08) ? HIGH : LOW);
    } else {
        // Modo decimal
        digitalWrite(LED1_PIN, (count >= 1) ? HIGH : LOW);
        digitalWrite(LED2_PIN, (count >= 2) ? HIGH : LOW);
        digitalWrite(LED3_PIN, (count >= 3) ? HIGH : LOW);
        digitalWrite(LED4_PIN, (count >= 4) ? HIGH : LOW);
    }

    // Imprimir el valor del contador en el monitor serial
    Serial.print("Modo: ");
    Serial.print(mode == 0 ? "Binario" : "Decimal");
    Serial.print(", Valor: ");
    Serial.println(count);

    // Delay para evitar desbordar el monitor serial
    delay(100);
}

void handleButtonIncrease() {
    if ((millis() - lastDebounceTime) > debounceDelay) {
        lastDebounceTime = millis();
        if (mode == 0) {
            if (count < 15) count++;
        } else {
            if (count < 4) count++;
        }
    }
}

void handleButtonDecrease() {
    if ((millis() - lastDebounceTime) > debounceDelay) {
        lastDebounceTime = millis();
        if (count > 0) count--;
    }
}

void handleButtonMode() {
    bool currentButtonModeState = digitalRead(BUTTON_MODE_PIN);
    if (currentButtonModeState != lastButtonModeState) {
        lastButtonModeState = currentButtonModeState;
        if (currentButtonModeState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
            lastDebounceTime = millis();
            buttonModeStateChanged = true;
        }
    }
}
