from collections import deque
from threading import Thread
from re import findall
from time import sleep
from serial import Serial
from serial.tools.list_ports import comports
from serial.serialutil import SerialException
from PIL import Image, ImageTk
from numpy import sqrt, where, reshape, array, sort, mean, insert, cumsum, ndarray
from cv2 import imread, split, merge ,applyColorMap, resize, GaussianBlur, COLORMAP_JET, INTER_NEAREST, INTER_CUBIC
from tkinter import Menu, Button, Checkbutton, Label, LabelFrame, Entry, messagebox, ttk, font, Tk, StringVar
import pandas as pd


class App:
    def __init__(self):
        pass

    def load_data(self, path):
        df = pd.read_csv(path, index_col=None)
        p768 = df.iloc[:, 2:]
        for i in p768.itertuples():
            
            print(array(i).tolist())
            break

    @staticmethod
    def normalization(data):
        _range = data.max() - data.min()
        return (data - data.min()) / _range

    def data_to_frame(self, data):
        data = self.normalization(data)
        img_gray = (data*255).astype('uint8')
        heatmap_g = img_gray.astype('uint8')
        frame = applyColorMap(heatmap_g, COLORMAP_JET)
        return frame


if __name__ == "__main__":
    data_path = r'data\20200626_155946_mlx90640_01_light_none.csv'

    app = App()
    app.load_data(data_path)
