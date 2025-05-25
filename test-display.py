import board
import busio
from PIL import Image, ImageDraw, ImageFont
import adafruit_ssd1306

# Create the I2C interface
i2c = busio.I2C(board.SCL, board.SDA)

# Create the SSD1306 OLED object
disp = adafruit_ssd1306.SSD1306_I2C(128, 64, i2c, addr=0x3C)

# Clear display
disp.fill(0)
disp.show()

# Create blank image for drawing
width = disp.width
height = disp.height
image = Image.new('1', (width, height))

# Get drawing object
draw = ImageDraw.Draw(image)

# Draw some text
font = ImageFont.load_default()
draw.text((0, 0), "Hello, World!", font=font, fill=255)

# Display image
disp.image(image)
disp.show()
