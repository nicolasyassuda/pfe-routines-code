#include <wiringPi.h>
#include <stdio.h>

// Definição dos pinos (usando BCM)
#define STEP_PIN        27   // STEP - GPIO 27
#define DIR_PIN         4    // DIR - GPIO 4
#define ENABLE_PIN      22   // ENABLE - GPIO 22
#define RELAY_PIN       6    // RELÉ - GPIO 6
#define CURSO_FINAL_PIN 23   // Final de curso - GPIO 23

// Configurações do motor
#define PASSOS_ROTACAO_MOTOR    200     // Passos por rotação
#define PASSO_FUSO              3.0     // Passo do fuso (mm/rev)
#define MICRO_PASSO             16      // Microstepping configurado no driver
#define REDUTOR_PLANETARIO      13.76   // Redução do redutor planetário

// Frequência desejada: 16kHz
#define FREQUENCIA_DESEJADA_HZ 16000
#define INTERVALO_US (1000000 / FREQUENCIA_DESEJADA_HZ)  // Tempo por ciclo em microssegundos

void rotina_referenciamento();
void rotina_descida(int passos);
void rotina_subida();
void motor_step();

int main() {
    motor_step();
    return 0;
}

void rotina_referenciamento() {
    printf("Rotina de referenciamento iniciada\n");
    digitalWrite(DIR_PIN, LOW);  // Direção para encontrar o final de curso

    while (digitalRead(CURSO_FINAL_PIN) == HIGH) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(INTERVALO_US / 2);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(INTERVALO_US / 2);
    }

    printf("Rotina de referenciamento finalizada\n");
    digitalWrite(DIR_PIN, HIGH);  // Inverte direção após referenciar
}

void rotina_descida(int passos) {
    printf("Rotina de descida iniciada\n");

    // Liga o relé antes de iniciar a descida
    digitalWrite(RELAY_PIN, HIGH);
    printf("Relé ligado\n");

    for (int i = 0; i < passos; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(INTERVALO_US / 2);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(INTERVALO_US / 2);
    }

    // Desliga o relé após a descida
    digitalWrite(RELAY_PIN, LOW);
    printf("Relé desligado\n");

    printf("Rotina de descida finalizada\n");
    delay(3000);  // Pausa de 3 segundos
}

void rotina_subida() {
    printf("Rotina de subida iniciada\n");

    while (digitalRead(CURSO_FINAL_PIN) == HIGH) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(INTERVALO_US / 2);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(INTERVALO_US / 2);
    }

    printf("Rotina de subida finalizada\n");
}

void motor_step() {
    // Inicialização do WiringPi
    if (wiringPiSetupGpio() == -1) {  // Usa numeração BCM
        printf("Erro ao inicializar WiringPi!\n");
        return;
    }

    // Configuração dos pinos
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(CURSO_FINAL_PIN, INPUT);

    pullUpDnControl(CURSO_FINAL_PIN, PUD_UP);  // Pull-up interno
    digitalWrite(ENABLE_PIN, LOW);  // Ativa o driver (ativo em LOW)

    // Chama as rotinas
    rotina_referenciamento();
    int passos_descida = 100000;  // Ajuste conforme necessário
    rotina_descida(passos_descida);
    rotina_subida();

    printf("Experimento finalizado\n");

    // Limpa os pinos
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);
    digitalWrite(ENABLE_PIN, HIGH);  // Desativa o driver
    digitalWrite(RELAY_PIN, LOW);    // Garante que o relé esteja desligado
}
