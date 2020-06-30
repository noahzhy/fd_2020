from collections import deque
from time import sleep, time
from tkinter import (Button, Checkbutton, Entry, Label, LabelFrame, Menu,
                     StringVar, Tk, font, messagebox, ttk)

import pandas as pd
from cv2 import (COLORMAP_JET, INTER_CUBIC, INTER_NEAREST, GaussianBlur,
                 applyColorMap, imread, imshow, merge, resize, split, waitKey)
from numpy import (array, cumsum, insert, mean, ndarray, reshape, sort, sqrt,
                   where)
from PIL import Image, ImageTk
import matplotlib.pyplot as plt
import cv2


class App:
    def __init__(self):
        pass

    def load_data(self, data_path, video_path):
        df = pd.read_csv(data_path, index_col=None)
        cap = cv2.VideoCapture(video_path)
        print(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        p768 = df.iloc[:, 2:]

        # if cap.isOpened():
        #     while True:
        #         ret, frame = cap.read()
        #         if ret == True:
        #             print(cap.get(cv2.CAP_PROP_POS_FRAMES)/25*7.8)
        #             cv2.imshow('video', frame)
        #         else:
        #             break

        #         if cv2.waitKey(20)==27:
        #             break

        for i in p768.itertuples(index=False):
            data = array(i).reshape((24, 32))
            # plt.imshow(data)
            # plt.pause(0.0001)
            frame = self.data_to_frame(data)
            frame = self.raw_img(frame)
            imshow('preview', frame)

            sleep(0.0125) # x10
            waitKey(1)

    @staticmethod
    def normalization(data):
        _range = data.max() - data.min()
        return (data - data.min()) / _range

    @staticmethod
    def raw_img(frame):
        return resize(frame, (320, 240), interpolation=INTER_NEAREST)

    def data_to_frame(self, data):
        data = self.normalization(data)
        img_gray = (data*255).astype('uint8')
        heatmap_g = img_gray.astype('uint8')
        frame = applyColorMap(heatmap_g, COLORMAP_JET)
        return frame


if __name__ == "__main__":
    data_path = r'data\20200629_163753_mlx90640_02_light_none.csv'
    video_path = r'video\20200628_183032_mlx90640_01_light_natural.mp4'

    app = App()
    app.load_data(data_path, video_path)
