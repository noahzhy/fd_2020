import time
import busio
import board
import adafruit_amg88xx
# import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from IPython import display


# plt.title("IR Image")
plt.ion()

# def ir_start():
i2c = busio.I2C(board.SCL, board.SDA)
amg = adafruit_amg88xx.AMG88XX(i2c, addr=0x68)

#     while True:
#         time.sleep(1)
#         for row in amg.pixels:
#             print(["{0:.1f}".format(temp) for temp in row])


def show_image(array_list):
    plt.imshow(array_list)
    plt.pause(0.1)
    plt.clf()
    # display.clear_output(wait=True)


if __name__ == "__main__":
    # ir_start()
    while True:
        # time.sleep(1)
        show_image(amg.pixels)
