import time
import busio
import board
import adafruit_amg88xx
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg


# plt.title("IR Image")
plt.ion()

def ir_start():
    i2c = busio.I2C(board.SCL, board.SDA)
    amg = adafruit_amg88xx.AMG88XX(i2c, addr=0x68)

    while True:
        time.sleep(1)
        for row in amg.pixels:
            print(["{0:.1f}".format(temp) for temp in row])


def show_image(array_list):
    # plt.show()
    plt.imshow(array_list)
    plt.pause(0.1)
    plt.clf()


if __name__ == "__main__":
    ir_start()
    array_list = [
        [25.0, 25.75, 25.5, 26.5, 25.5, 25.5, 26.25, 25.0], 
        [25.0, 25.0, 24.75, 25.5, 24.75, 24.25, 25.25, 25.75], 
        [24.0, 25.5, 26.5, 24.75, 25.25, 24.75, 25.25, 26.25], 
        [25.75, 27.25, 26.75, 25.5, 24.75, 25.75, 24.75, 26.25], 
        [26.25, 27.75, 26.5, 25.75, 26.25, 24.75, 25.5, 26.0], 
        [26.5, 26.5, 27.0, 26.0, 25.0, 25.5, 25.5, 27.0], 
        [28.5, 26.25, 25.5, 25.5, 26.5, 25.5, 29.5, 27.5], 
        [27.75, 27.0, 26.25, 25.75, 26.25, 29.0, 32.25, 32.25]
    ]
    show_image(array_list)

    # array_list = [
    #     [25.0, 25.75, 25.5, 26.5, 25.5, 25.5, 26.25, 25.0], 
    #     [25.0, 25.0, 24.75, 25.5, 24.75, 24.25, 25.25, 25.75], 
    #     [24.0, 25.5, 26.5, 24.75, 25.25, 24.75, 25.25, 26.25], 
    #     [25.75, 27.25, 26.75, 25.5, 24.75, 25.75, 24.75, 26.25], 
    #     [26.25, 27.75, 26.5, 25.75, 26.25, 24.75, 25.5, 26.0], 
    #     [27.75, 27.0, 26.25, 25.75, 26.25, 29.0, 32.25, 32.25],
    #     [28.5, 26.25, 25.5, 25.5, 26.5, 25.5, 29.5, 27.5], 
    #     [27.75, 27.0, 26.25, 25.75, 26.25, 29.0, 32.25, 32.25]
    # ]
    # show_image(array_list)