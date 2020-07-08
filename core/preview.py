import matplotlib.pyplot as plt
from pandas import read_csv
from time import sleep, time
from PIL import Image, ImageTk
from cv2 import NORM_MINMAX, CAP_PROP_FRAME_COUNT, CAP_PROP_POS_FRAMES, COLORMAP_JET, INTER_CUBIC, INTER_NEAREST, GaussianBlur, normalize, VideoCapture, applyColorMap, imread, imshow, merge, resize, split, waitKey
from numpy import array, cumsum, insert, mean, ndarray, reshape, sort, sqrt, where
from tkinter import Button, Checkbutton, Entry, HORIZONTAL, IntVar, Label, LabelFrame, Listbox, Menu, Radiobutton, Scale, StringVar, Tk, font, messagebox, ttk
from tkinter.filedialog import askopenfilename
import os
import glob
import threading as th
import sys


def print_ok(event):
    print('ok', event)


class App:
    def __init__(self):
        self.data = None
        self.cap = None
        self.quit_status = False
        self.play_status = False
        self.play_control = th.Thread(target=self.start_play)
        self.app = Tk()
        self.app.resizable(width=False, height=False)
        self.app.title("Video marking tool")
        # self.app.iconbitmap("icon.ico")

        img_rgb = imread(r'core/logo.jpg')
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
        operation_frame.pack(side='top', fill='both',
                             expand=True, padx=10, pady=5)

        open_btn = Button(
            operation_frame, text="Open [Ctrl+O]", command=self.select_path)
        open_btn.pack(side='left', fill='x', expand=True, padx=10, pady=10)

        play_btn = Button(operation_frame, width=10,
                          text="Play [Space]", command=self.start_pause)
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
        Radiobutton(label_frame, text='walk', variable=v, value=1).pack(side='left')
        Radiobutton(label_frame, text='stand', variable=v, value=2).pack(side='left')
        Radiobutton(label_frame, text='standup', variable=v, value=3).pack(side='left')
        Radiobutton(label_frame, text='sit', variable=v, value=4).pack(side='left')
        Radiobutton(label_frame, text='sitdown', variable=v, value=5).pack(side='left')
        Radiobutton(label_frame, text='falling', variable=v, value=6).pack(side='left')
        Radiobutton(label_frame, text='lying', variable=v, value=7).pack(side='left')
        Radiobutton(label_frame, text='sleeping', variable=v, value=8).pack(side='left')

        result_list = Listbox(self.app, width=100)
        result_list.pack(padx=10, pady=10)

        self.left_view = left_view
        self.right_view = right_view
        self.bar = bar
        self.open_btn = open_btn
        self.play_btn = play_btn

        self.app.bind('<Control-o>', self.select_path)
        self.app.bind('<space>', self.start_pause)
        self.app.bind('<Left>', self.test_key)
        self.app.bind('<Right>', self.test_key)
        self.app.bind('<Return>', self.test_key)

        self.app.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.app.mainloop()

    def get_fir_frame(self, pos):
        df = self.data[pos:pos+1]
        data = array(df).reshape((24, 32))
        frame = self.data_to_frame(data)
        frame = self.raw_img(frame)
        return self.openCV_to_PhotoImage(frame)

    def get_video_frame(self, pos):
        self.cap.set(CAP_PROP_POS_FRAMES, pos)
        _, img = self.cap.read()
        return self.openCV_to_PhotoImage(img)

    def update_win(self, pos=0):
        frame_num = int(
            pos * (self.cap.get(CAP_PROP_FRAME_COUNT)*0.95) / len(self.data)
        ) - 25

        img_left = self.get_fir_frame(self.bar.get())
        self.left_view.config(image=img_left)
        self.left_view.image = img_left

        img_right = self.get_video_frame(frame_num)
        self.right_view.config(image=img_right)
        self.right_view.image = img_right
        
        self.bar.set(pos)
        self.app.update()

    def start_pause(self, event=None):
        if not self.play_status:
            self.play_btn['text'] = 'Pause [Space]'
            self.play_status = True
        else:
            self.play_btn['text'] = 'Play [Space]'
            self.play_status = False

        print(event)

    def start_play(self):
        while True:
            if self.quit_status:
                print(self.quit_status)
                break
            try:
                if self.play_status:
                    self.update_win(self.bar.get()+1)
                else:
                    self.update_win(self.bar.get())
            except ValueError as e:
                self.play_btn['text'] = 'Play [Space]'
                self.play_status = False
            sleep(1/50)

    @ staticmethod
    def raw_img(frame):
        return resize(frame, (320, 240), interpolation=INTER_NEAREST)

    @ staticmethod
    def data_to_frame(data):
        out_data = None
        out_data = normalize(data, out_data, 0, 255, NORM_MINMAX)
        img_gray = (out_data).astype('uint8')
        heatmap_g = img_gray.astype('uint8')
        frame = applyColorMap(heatmap_g, COLORMAP_JET)
        return frame

    @ staticmethod
    def openCV_to_PhotoImage(src):
        b, g, r = split(src)
        img = merge((r, g, b))
        return ImageTk.PhotoImage(Image.fromarray(img))

    def select_path(self, event=None):
        path_ = askopenfilename()
        father_path = os.path.dirname(os.path.dirname(path_))
        date = os.path.basename(path_).split('.')[0].split('_')[0:2]
        data_path = glob.glob(
            '{}/data/{}_{}_*.csv'.format(father_path, date[0], date[1]))[0]
        video_path = glob.glob(
            '{}/video/{}_{}_*.mp4'.format(father_path, date[0], date[1]))[0]
        # print(data_path, video_path)
        self.data = read_csv(data_path, index_col=None).iloc[:, 2:]
        self.cap = VideoCapture(video_path)
        self.bar['to'] = len(self.data)
        self.bar['tickinterval'] = int(self.bar['to']/10)
        self.update_win(0)

        self.play_control.start()

        # self.start_pause()

    def test_key(self, event=None):
        print('test_key', event)

    def on_closing(self):
        if messagebox.askokcancel("Quit", "Do you want to quit?"):
            self.quit_status = True
            self.app.destroy()

if __name__ == "__main__":
    app = App()
