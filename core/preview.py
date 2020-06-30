from collections import deque
from time import sleep, time
from tkinter import Button, Checkbutton, Entry, HORIZONTAL, IntVar, Label, LabelFrame, Listbox, Menu, Radiobutton, Scale, StringVar, Tk, font, messagebox, ttk

import pandas as pd
from cv2 import (COLORMAP_JET, INTER_CUBIC, INTER_NEAREST, GaussianBlur,
                 applyColorMap, imread, imshow, merge, resize, split, waitKey)
from numpy import (array, cumsum, insert, mean, ndarray, reshape, sort, sqrt,
                   where)
from PIL import Image, ImageTk
import matplotlib.pyplot as plt
import cv2
import numpy as np


class App:
    def __init__(self):
        self.data = None
        self.cap = None
        self.app = Tk()
        self.app.resizable(width=False, height=False)
        self.app.title("Video marking tool")
        # self.app.iconbitmap("icon.ico")

        img_rgb = imread(r'logo.jpg')
        img_rgb = resize(img_rgb, (320, 240), interpolation=INTER_NEAREST)

        imgs = LabelFrame(self.app, text="Videos")
        imgs.pack(side='top', expand=True, padx=10, pady=5)

        im1 = self.openCV_to_PhotoImage(img_rgb)
        left_view = Label(imgs, image=im1)
        left_view.pack(side='left', padx=10, pady=10)

        im2 = self.openCV_to_PhotoImage(img_rgb)
        right_view = Label(imgs, image=im2)
        right_view.pack(side='right', padx=10, pady=10)

        bar = Scale(self.app, from_=0, to=2000, tickinterval=100,
                    orient=HORIZONTAL, length=690)
        bar.pack()

        operation_frame = LabelFrame(self.app, text="Operation")
        operation_frame.pack(side='top', fill='both', expand=True, padx=10, pady=5)

        play_btn = Button(operation_frame, text="Play [Space]")
        play_btn.pack(side='left', fill='x', expand=True, padx=10, pady=10)

        start_btn = Button(operation_frame, text="Start [<]")
        start_btn.pack(side='left', fill='x', expand=True, padx=10, pady=10)

        end_btn = Button(operation_frame, text="End [>]")
        end_btn.pack(side='left', fill='x', expand=True, padx=10, pady=10)

        complete_btn = Button(operation_frame, text="Complete [Enter]")
        complete_btn.pack(side='left', fill='x', expand=True, padx=10, pady=10)

        label_frame = LabelFrame(self.app, text="Labels")
        label_frame.pack(side='top', fill='both', expand=True, padx=10, pady=5)

        v = IntVar()
        Radiobutton(label_frame, text='One', variable=v, value=1).pack(side='left')
        Radiobutton(label_frame, text='Two', variable=v, value=2).pack(side='left')
        Radiobutton(label_frame, text='Three', variable=v, value=3).pack(side='left')
        Radiobutton(label_frame, text='One', variable=v, value=4).pack(side='left')
        Radiobutton(label_frame, text='Two', variable=v, value=5).pack(side='left')
        Radiobutton(label_frame, text='Three', variable=v, value=6).pack(side='left')

        result_list = Listbox(self.app, width=100)
        result_list.pack(padx=10, pady=10)

        self.left_view = left_view
        self.right_view = right_view

        self.app.mainloop()

    def load_data(self, data_path, video_path):
        self.data = pd.read_csv(data_path, index_col=None).iloc[:, 2:]
        self.cap = cv2.VideoCapture(video_path)

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

    def get_fir_frame(self, frame_num):
        data = np.array(self.data[frame_num:frame_num+1]).reshape((24, 32))
        frame = self.data_to_frame(data)
        frame = self.raw_img(frame)
        return frame
        # imshow('preview', frame)
        # waitKey(0)

        # for i in p768.itertuples(index=False):
        #     data = array(i).reshape((24, 32))
        #     # plt.imshow(data)
        #     # plt.pause(0.0001)
        #     frame = self.data_to_frame(data)
        #     frame = self.raw_img(frame)
        #     imshow('preview', frame)

        #     sleep(0.0125) # x10
        #     waitKey(1)

    @staticmethod
    def normalization(data):
        np.array(data)
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

    @staticmethod
    def openCV_to_PhotoImage(src):
        b, g, r = split(src)
        img = merge((r, g, b))
        return ImageTk.PhotoImage(Image.fromarray(img))


if __name__ == "__main__":
    data_path = r'data\20200628_183032_mlx90640_01_light_natural.csv'
    video_path = r'video\20200628_183032_mlx90640_01_light_natural.mp4'

    app = App()
    data = app.load_data(data_path, video_path)
    app.get_fir_frame(79)
