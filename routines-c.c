#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pigpio.h>
#include <string.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

// Definições de pinos (BCM GPIO numbers)
#define STEP_PIN 27      // STEP
#define DIR_PIN 4        // DIRECTION
#define ENABLE_PIN 22    // ENABLE
#define CURSO_FINAL_PIN 24  // Final de curso
#define RELAY_PIN 6      // Relé
#define BUTTON_PIN 21    // Botão

// Frequência desejada para o motor: 12kHz
#define FREQUENCIA_KHZ 14

// Endereço I2C do display OLED (SSD1306)
#define OLED_ADDR 0x3C
int sleep_time; // Declaração global

void clear_oled(int fd) {
    char init[] = {
        0x00, 0xAE, // Display off
        0x00, 0x20, 0x00, // Horizontal addressing
        0x00, 0xB0, // Set page start address
        0x00, 0x02, // Set low column address
        0x00, 0x10, // Set high column address
        0x00, 0xC8, // Set com scan inc
        0x00, 0xDA, 0x12, // Set COM pins
        0x00, 0x81, 0xCF, // Set contrast
        0x00, 0xA1, // Set segment re-map
        0x00, 0xA6, // Normal display
        0x00, 0xA8, 0x3F, // Multiplex ratio
        0x00, 0xD3, 0x00, // Display offset
        0x00, 0xD5, 0x80, // Display clock divide
        0x00, 0xD9, 0xF1, // Precharge
        0x00, 0xDB, 0x40, // VCOM detect
        0x00, 0x8D, 0x14, // Charge pump
        0x00, 0xAF  // Display on
    };

    write(fd, init, sizeof(init));
}

void define_text_in_oled(int fd, const char *lines[], int count) {
    clear_oled(fd);
    // Placeholder visual — implementação real requer biblioteca gráfica
    char data[128];
    for (int i = 0; i < count; i++) {
        memset(data, 0xFF, 128);
        write(fd, data, 128);
    }
}

void rotina_referenciamento(int fd) {
    printf("Rotina referenciamento iniciado\n");
    const char *lines[] = {"Rotina referenciamento", "iniciado"};
    define_text_in_oled(fd, lines, 2);
    gpioWrite(DIR_PIN, 0);  // Direção referenciamento
    gpioSetPWMfrequency(STEP_PIN, FREQUENCIA_KHZ*1000);
    gpioPWM(STEP_PIN, 500);

    while (gpioRead(CURSO_FINAL_PIN)) {
      
    }

    gpioWrite(DIR_PIN, 1);  // Muda direção
    printf("Rotina referenciamento finalizada\n");
    const char *lines2[] = {"Rotina referenciamento", "finalizada"};
    define_text_in_oled(fd, lines2, 2);
}

void rotina_descida(int fd) {
    printf("Rotina descida iniciada\n");
    const char *lines[] = {"Rotina descida", "iniciado"};
    define_text_in_oled(fd, lines, 2);
    gpioSetPWMfrequency(STEP_PIN, FREQUENCIA_KHZ*1000);
    gpioPWM(STEP_PIN, 500);

    while (gpioRead(CURSO_FINAL_PIN)) {
    }

    gpioPWM(STEP_PIN, 0); // Para o motor

    gpioWrite(DIR_PIN, 0);  // Muda direção
    printf("Rotina descida finalizada\n");
    const char *lines2[] = {"Rotina descida", "finalizada"};
    define_text_in_oled(fd, lines2, 2);
    usleep(3000000); // 3 segundos
}

void rotina_subida(int fd) {
    printf("Rotina subida iniciada\n");
    const char *lines[] = {"Rotina subida", "iniciada"};
    define_text_in_oled(fd, lines, 2);

    gpioSetPWMfrequency(STEP_PIN, FREQUENCIA_KHZ*1000);
    gpioPWM(STEP_PIN, 500);

    while (gpioRead(CURSO_FINAL_PIN)) {
    }

    gpioPWM(STEP_PIN, 0); // Para o motor

    printf("Rotina subida finalizada\n");
    const char *lines2[] = {"Rotina subida", "finalizada"};
    define_text_in_oled(fd, lines2, 2);
}

int main(void) {
    double meio_periodo_us = (1.0 / (FREQUENCIA_KHZ * 1000.0)) / 2.0 * 1000000.0;
    printf("Meio período: %.4f µs\n", meio_periodo_us);


    sleep_time = (int)(meio_periodo_us + 0.5);
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Erro ao inicializar pigpio\n");
        return 1;
    }

    // Configuração dos pinos
    gpioSetMode(STEP_PIN, PI_OUTPUT);
    gpioSetMode(DIR_PIN, PI_OUTPUT);
    gpioSetMode(ENABLE_PIN, PI_OUTPUT);
    gpioSetMode(RELAY_PIN, PI_OUTPUT);
    gpioSetMode(CURSO_FINAL_PIN, PI_INPUT);
    gpioSetPullUpDown(CURSO_FINAL_PIN, PI_PUD_UP);
    gpioSetMode(BUTTON_PIN, PI_INPUT);
    gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);

    gpioWrite(DIR_PIN, 0);       // Direção padrão
    gpioWrite(ENABLE_PIN, 0);    // Habilita driver
    gpioWrite(RELAY_PIN, 1);     // Inicialmente desligado
    gpioSetPWMrange(STEP_PIN, 1000);
    // Inicialização do display OLED
    int oled_fd = open("/dev/i2c-1", O_RDWR);
    if (oled_fd < 0) {
        perror("Erro ao abrir barramento I2C");
        gpioTerminate();
        return 1;
    }

    if (ioctl(oled_fd, I2C_SLAVE, OLED_ADDR) < 0) {
        perror("Erro ao selecionar dispositivo OLED");
        close(oled_fd);
        gpioTerminate();
        return 1;
    }

    clear_oled(oled_fd);
    printf("Aguardando pressionar o botão...\n");
    const char *wait_lines[] = {"Esperando botao", "para iniciar"};
    define_text_in_oled(oled_fd, wait_lines, 2);

    while (gpioRead(BUTTON_PIN)) {
        usleep(100000);  // 100ms
    }

    gpioWrite(RELAY_PIN, 0); // Liga relé

    rotina_referenciamento(oled_fd);
    sleep(3);
    rotina_descida(oled_fd);
    rotina_subida(oled_fd);

    printf("Experimento finalizado\n");
    const char *done_lines[] = {"Programa finalizado"};
    define_text_in_oled(oled_fd, done_lines, 1);

    gpioWrite(ENABLE_PIN, 1);   // Desabilita driver
    gpioWrite(RELAY_PIN, 1);    // Desliga relé

    close(oled_fd);
    gpioTerminate();

    return 0;
}
