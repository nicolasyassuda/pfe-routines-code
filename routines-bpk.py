import RPi.GPIO as GPIO
import time
import Adafruit_SSD1306
from PIL import Image, ImageDraw, ImageFont

# Definição dos pinos no Raspberry Pi
STEP_PIN = 27    # Pino de passo (GPIO 4)
DIR_PIN = 4   # Pino de direção (GPIO 17)
ENABLE_PIN = 22 # Pino de habilitação (GPIO 27)
CURSO_FINAL_PIN = 24 # Pino de final de curso (GPIO5)
RELAY = 6
BUTTON_PIN = 21

# Configuração do motor
Passos_Rotacao_motor = 200  # Passos por Rotação
Passo_Fuso = 3.0           # Passo do fuso (mm/rev)
Vel_desejada_mm_s = 0.3    # Velocidade desejada (mm/s)
Fator_de_erro_raspberry = 0.6
# Configuração do driver
Micro_passo = 8            # Configuração de microstepping do driver
Redutor_Planetario = 13.76 # Redução do motor

# Cálculo da frequência necessária
Passos_por_segundo = (Passos_Rotacao_motor * Micro_passo * Vel_desejada_mm_s) / (Passo_Fuso)
frequencia_HZ = Redutor_Planetario * Passos_por_segundo

# Calcule o RPM do motor
rpm = (Passos_por_segundo * 60) / (Passos_Rotacao_motor * Redutor_Planetario)
disp = Adafruit_SSD1306.SSD1306_128_64(rst=None, i2c_address=0x3C)
disp.begin()
disp.clear()
disp.display()

# Configurando o display OLED
width = disp.width
height = disp.height
image = Image.new('1', (width, height))

draw = ImageDraw.Draw(image)

draw.rectangle((0, 0, width, height), outline=0, fill=0)

font = ImageFont.load_default()

# Configuração do GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(STEP_PIN, GPIO.OUT)
GPIO.setup(DIR_PIN, GPIO.OUT)
GPIO.setup(ENABLE_PIN, GPIO.OUT)
GPIO.setup(CURSO_FINAL_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(button_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

GPIO.output(DIR_PIN, GPIO.LOW)  # Ajuste a direção conforme necessário
GPIO.output(ENABLE_PIN, GPIO.LOW) # Ativa o driver


def rotina_referenciamento( channel ):
    print("Rotina referenciamento iniciado")
    while GPIO.input(channel) == GPIO.HIGH:
         GPIO.output(STEP_PIN, GPIO.HIGH)
         time.sleep(0.05 / 2000)
         GPIO.output(STEP_PIN, GPIO.LOW)
         time.sleep(0.05 / 2000)
    GPIO.output(DIR_PIN, GPIO.HIGH)
    print("Rotina referenciamento finalizada")

def rotina_descida( intervalo_ms ):
    print("Rotina descida iniciada")
    contador_descida = 0
    limitador = (539000/3)
#    while contador_descida < :
#        contador_descida += 1
#        GPIO.output(STEP_PIN, GPIO.HIGH)
#        time.sleep(0.05 / 2000)
#        GPIO.output(STEP_PIN, GPIO.LOW)
#        time.sleep(0.05 / 2000)
    while contador_descida < limitador:
        contador_descida += 1
        GPIO.output(STEP_PIN, GPIO.HIGH)
        time.sleep(intervalo_ms/2000)
        GPIO.output(STEP_PIN, GPIO.LOW)
        time.sleep(intervalo_ms/2000)
    print("Rotina descida finalizada")
    time.sleep(3)

def rotina_subida( intervalo_ms , channel ):
    print("Rotina subida iniciada")
    GPIO.output(DIR_PIN, GPIO.LOW)
    while GPIO.input(channel) == GPIO.HIGH:
         GPIO.output(STEP_PIN, GPIO.HIGH)
         time.sleep( intervalo_ms / 2000)
         GPIO.output(STEP_PIN, GPIO.LOW)
         time.sleep( intervalo_ms / 2000)
    print("Rotina subida finalizada")

def motor_step():
    time.sleep(5)
    GPIO.output(RELAY, GPIO.HIGH)
    while GPIO.input(button_pin) == GPIO.LOW:
         print("Botão")
         draw.text((10, 20), "Esperando botão", font=font, fill=255)
         disp.image(image)
         disp.display()
    draw.text((10, 20), "            ", font=font, fill=255)
    disp.image(image)
    disp.display()
    rotina_referenciamento(CURSO_FINAL_PIN)
    intervalo_s = 1 / frequencia_HZ * Fator_de_erro_raspberry
    intervalo_ms = intervalo_s * 1000  # Converter para milissegundos
    contador = 0
    rotina_descida(intervalo_ms)
    rotina_subida(intervalo_ms, CURSO_FINAL_PIN)
    print("Experimento finalizado")
try:
    motor_step()
except KeyboardInterrupt:
    GPIO.cleanup()  # Limpa a configuração do GPIO
