#include <wiringPi.h>
#include <stdio.h>
#include "oled-test.c"

#define STEP_PIN        27   // STEP - GPIO 27
#define DIR_PIN         4    // DIR - GPIO 4
#define ENABLE_PIN      22   // ENABLE - GPIO 22
#define RELAY_PIN       6    // RELÉ - GPIO 6
#define CURSO_FINAL_PIN 24   // Final de curso - GPIO 23
#define BUTTON_PIN 21

#define PASSOS_ROTACAO_MOTOR    200     // Passos por rotação
#define PASSO_FUSO              3.0     // Passo do fuso (mm/rev)
#define MICRO_PASSO             16      // Microstepping configurado no driver
#define REDUTOR_PLANETARIO      13.76   // Redução do redutor planetário

#define FREQUENCIA_DESEJADA_HZ 12000
#define INTERVALO_US (1000000 / FREQUENCIA_DESEJADA_HZ)  // Tempo por ciclo em microssegundos

void rotina_referenciamento();
void rotina_descida(int passos);
void rotina_subida();
void motor_step();

int main() {
    char *filename = "/dev/i2c-1";
    if ((i2c_fd = open(filename, O_RDWR)) < 0) {
        perror("Falha ao abrir o barramento I2C");
        exit(1);
    }

    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Falha ao acessar o escravo I2C");
        close(i2c_fd);
        exit(1);
    }

    oled_init(i2c_fd);
    oled_clear(i2c_fd);
    motor_step();
    close(i2c_fd);
    return 0;
}

void rotina_referenciamento() {
    printf("Rotina de referenciamento iniciada\n");
    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Referenciamento");
    oled_draw_string(i2c_fd, 2, 0, "Iniciado");
    digitalWrite(DIR_PIN, LOW);  // Direção para encontrar o final de curso

    while (digitalRead(CURSO_FINAL_PIN) == HIGH) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(INTERVALO_US*0.375);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(INTERVALO_US*0.375);
    }
    printf("Rotina de referenciamento finalizada\n");
    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Referenciamento");
    oled_draw_string(i2c_fd, 2, 0, "Finalizado");
    delay(3000);  // Pausa de 1 segundo
    digitalWrite(DIR_PIN, HIGH);  // Inverte direção após referenciar
}

void rotina_descida(int passos) {
    printf("Rotina de descida iniciada\n");
    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Descida");
    oled_draw_string(i2c_fd, 2, 0, "Iniciada");
    for (int i = 0; i < passos; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(INTERVALO_US / 2);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(INTERVALO_US / 2);
    }

    printf("Rotina de descida finalizada\n");
    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Descida");
    oled_draw_string(i2c_fd, 2, 0, "Finalizada");
}

void rotina_subida() {
    printf("Rotina de subida iniciada\n");
    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Subida");
    oled_draw_string(i2c_fd, 2, 0, "Iniciada");
    digitalWrite(DIR_PIN,LOW);
    while (digitalRead(CURSO_FINAL_PIN) == HIGH) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(INTERVALO_US / 2);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(INTERVALO_US / 2);
    }

    printf("Rotina de subida finalizada\n");
    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Subida");
    oled_draw_string(i2c_fd, 2, 0, "Finalizada");
    delay(3000);  // Pausa de 3 segundos
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
    pinMode(BUTTON_PIN, INPUT);
    digitalWrite(RELAY_PIN,HIGH);
    pullUpDnControl(BUTTON_PIN, PUD_UP);
    pullUpDnControl(CURSO_FINAL_PIN, PUD_UP);  // Pull-up interno
    printf("Aguardando o botão ser pressionado...\n");
    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Esperando o switch");
    oled_draw_string(i2c_fd, 2, 0, "verde de inicio");
    oled_draw_string(i2c_fd, 4, 0, "ser pressionado...");
    while(digitalRead(BUTTON_PIN) == HIGH){
    }
    digitalWrite(RELAY_PIN,LOW);
    // Chama as rotinas
    rotina_referenciamento();
    delay(1000);
    int passos_descida = 2500000;  // Ajuste conforme necessário
    rotina_descida(passos_descida);
    delay(20000);
    rotina_subida();

    printf("Experimento finalizado\n");

    oled_clear(i2c_fd);
    oled_draw_string(i2c_fd, 0, 0, "Experiment");
    oled_draw_string(i2c_fd, 2, 0, "completed.");
    delay(3000);  // Pausa de 3 segundos
    // Desativa o driver do motor
    digitalWrite(RELAY_PIN, HIGH);  // Desliga o relé
    // Limpa os pinos
    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);
    digitalWrite(ENABLE_PIN, HIGH);  // Desativa o driver
    digitalWrite(RELAY_PIN, LOW);    // Garante que o relé esteja desligado
}
