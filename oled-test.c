#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

#define OLED_ADDR 0x3C
#define OLED_CMD_MODE 0x00
#define OLED_DATA_MODE 0x40

int i2c_fd;

void oled_send_byte(int fd, uint8_t byte, uint8_t mode) {
    char data[2] = {mode, byte};
    if (write(fd, data, 2) != 2) {
        perror("Erro ao escrever no OLED");
    }
}

void oled_init(int fd) {
    // Sequência de inicialização do SSD1306
    oled_send_byte(fd, 0xAE, OLED_CMD_MODE); // Display off
    oled_send_byte(fd, 0xD5, OLED_CMD_MODE); // Set display clock divide ratio/osc freq
    oled_send_byte(fd, 0x80, OLED_CMD_MODE); // Default
    oled_send_byte(fd, 0xA8, OLED_CMD_MODE); // Set multiplex ratio
    oled_send_byte(fd, 0x3F, OLED_CMD_MODE); // 64MUX
    oled_send_byte(fd, 0xD3, OLED_CMD_MODE); // Set display offset
    oled_send_byte(fd, 0x00, OLED_CMD_MODE); // No offset
    oled_send_byte(fd, 0x40, OLED_CMD_MODE); // Set start line
    oled_send_byte(fd, 0x8D, OLED_CMD_MODE); // Charge pump
    oled_send_byte(fd, 0x14, OLED_CMD_MODE); // Enable charge pump
    oled_send_byte(fd, 0x20, OLED_CMD_MODE); // Memory addressing mode
    oled_send_byte(fd, 0x00, OLED_CMD_MODE); // Horizontal addressing mode
    oled_send_byte(fd, 0xA1, OLED_CMD_MODE); // Segment remap
    oled_send_byte(fd, 0xC8, OLED_CMD_MODE); // COM scan direction
    oled_send_byte(fd, 0xDA, OLED_CMD_MODE); // Set COM pins hardware configuration
    oled_send_byte(fd, 0x12, OLED_CMD_MODE);
    oled_send_byte(fd, 0x81, OLED_CMD_MODE); // Set contrast
    oled_send_byte(fd, 0xCF, OLED_CMD_MODE);
    oled_send_byte(fd, 0xD9, OLED_CMD_MODE); // Precharge period
    oled_send_byte(fd, 0xF1, OLED_CMD_MODE);
    oled_send_byte(fd, 0xDB, OLED_CMD_MODE); // VCOM detect
    oled_send_byte(fd, 0x40, OLED_CMD_MODE);
    oled_send_byte(fd, 0xA4, OLED_CMD_MODE); // Output follows RAM content
    oled_send_byte(fd, 0xA6, OLED_CMD_MODE); // Set normal display
    oled_send_byte(fd, 0xAF, OLED_CMD_MODE); // Display on
}

void oled_clear(int fd) {
    for (int page = 0; page < 8; page++) {
        oled_send_byte(fd, 0xB0 + page, OLED_CMD_MODE); // Page number
        oled_send_byte(fd, 0x00, OLED_CMD_MODE);         // Lower column
        oled_send_byte(fd, 0x10, OLED_CMD_MODE);         // Higher column
        for (int i = 0; i < 128; i++) {
            oled_send_byte(fd, 0x00, OLED_DATA_MODE);    // Clear all pixels
        }
    }
}

int main() {
    char *filename = "/dev/i2c-1"; // Para Raspberry Pi versão 2 ou superior
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

    printf("OLED inicializado.\n");

    close(i2c_fd);
    return 0;
}