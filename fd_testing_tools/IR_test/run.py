import busio
import adafruit_amg88xx
import board


i2c_bus = busio.I2C(board.SCL, board.SDA)
amg = adafruit_amg88xx.AMG88XX(i2c_bus)
amg = adafruit_amg88xx.AMG88XX(i2c_bus, addr=0x68)

print(amg.pixels)