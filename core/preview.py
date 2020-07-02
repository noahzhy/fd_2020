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
from tkinter.filedialog import askopenfilename
import os
import glob


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

        bar = Scale(self.app, from_=0, to=2000, tickinterval=200,
                    orient=HORIZONTAL, length=690)
        bar.pack()

        operation_frame = LabelFrame(self.app, text="Operation")
        operation_frame.pack(side='top', fill='both', expand=True, padx=10, pady=5)

        open_btn = Button(operation_frame, text="Open [Ctrl+O]", command=self.select_path)
        open_btn.pack(side='left', fill='x', expand=True, padx=10, pady=10)

        play_btn = Button(operation_frame, text="Play [Space]", command=self.update_win)
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
        self.bar = bar

        self.app.mainloop()

    def get_fir_frame(self, pos):
        df = self.data[pos:pos+1]
        data = np.array(df).reshape((24, 32))
        frame = self.data_to_frame(data)
        frame = self.raw_img(frame)
        return self.openCV_to_PhotoImage(frame)

    def get_video_frame(self, pos):
        self.cap.set(cv2.CAP_PROP_POS_FRAMES, pos)
        _, img = self.cap.read()
        return self.openCV_to_PhotoImage(img)

    def update_win(self, pos=0):
        frame_num = int(self.bar.get()/25*7.8)
        if frame_num >= len(self.data):
            frame_num = len(self.data)-1

        img_left = self.get_fir_frame(frame_num)
        self.left_view.config(image=img_left)
        self.left_view.image = img_left

        img_right = self.get_video_frame(self.bar.get()-25)
        self.right_view.config(image=img_right)
        self.right_view.image = img_right
        self.app.update()

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

    def select_path(self):
        path_ = askopenfilename()
        father_path = os.path.dirname(os.path.dirname(path_))
        date = os.path.basename(path_).split('.')[0].split('_')[0:2]
        print(date)
        data_path = glob.glob('{}/data/{}_{}_*.csv'.format(father_path, date[0], date[1]))[0]
        video_path = glob.glob('{}/video/{}_{}_*.mp4'.format(father_path, date[0], date[1]))[0]
        print(data_path, video_path)
        self.data = pd.read_csv(data_path, index_col=None).iloc[:, 2:]
        self.cap = cv2.VideoCapture(video_path)
        self.bar['to'] = self.cap.get(cv2.CAP_PROP_FRAME_COUNT)
        self.bar['tickinterval'] = int(self.bar['to']/10)
        self.update_win(0)


if __name__ == "__main__":
    data_path = r'data\20200628_183032_mlx90640_01_light_natural.csv'
    video_path = r'video\20200628_183032_mlx90640_01_light_natural.mp4'

    app = App()
    # data = app.load_data(data_path, video_path)
    # app.update_win(78)
