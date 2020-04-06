from tkinter import *
from PIL import ImageTk, Image
import os
import cv2
import datetime


class App:  
    def __init__(self):
        self.RECORDING_STATUS = False
        self.path = 'video'
        self.check_dir()

        self.root = Tk()
        self.root.title("Fall detection recording and testing tool")
        # Create a frame
        self.app = Frame(self.root)
        self.app.grid()
        # Create a label in the frame
        self.cam_area = Label(self.app)
        self.cam_area.grid(row=0, column=0, padx=10, pady=10)

        self.ir_area = Label(self.app)
        self.ir_area.grid(row=0, column=1, padx=10, pady=10)

        self.btn_record = Button(self.app, text="Start Recording", command=self.recording)
        self.btn_record.grid(row=1, columnspan=2, sticky=W+E, padx=10, pady=10)

        self.statusbar = Label(self.app, text="[INFO] Previewing", bd=1, relief=SUNKEN, anchor=W)
        self.statusbar.grid(row=2, columnspan=2, sticky=W+S+E)

        # Capture from camera
        self.cam = cv2.VideoCapture(1)
        self.ir = cv2.VideoCapture(0)

        self.fourcc = cv2.VideoWriter_fourcc(*'XVID')
        self.output_cam = cv2.VideoWriter()
        self.output_ir = cv2.VideoWriter()

    def check_dir(self):
        if not os.path.exists(self.path):
            os.mkdir(self.path)

    def recording(self):
        # recording, so stop it
        if self.RECORDING_STATUS:
            self.RECORDING_STATUS = False
            self.btn_record['text'] = "Start Recording"
            self.statusbar['text'] = "[INFO] Previewing"

        # not recording, so start recording
        else:
            self.RECORDING_STATUS = True
            self.btn_record['text'] = "Stop Recording"
            self.statusbar['text'] = "[...] Recording"

            dt = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
            self.output_cam = cv2.VideoWriter(os.path.join(self.path, 'cam_{}.avi'.format(dt)), self.fourcc, 25, (320, 240))
            self.output_ir = cv2.VideoWriter(os.path.join(self.path, 'ir_{}.avi'.format(dt)), self.fourcc, 25, (240, 240))

    # function for video streaming
    def video_stream(self):
        _, cam_frame = self.cam.read()
        _, ir_frame = self.ir.read()
        cam_frame = cv2.resize(cam_frame, (320, 240))
        ir_frame = cv2.resize(ir_frame, (240, 240))

        if self.RECORDING_STATUS:
            self.output_cam.write(cam_frame)
            self.output_ir.write(ir_frame)

        cam_img = Image.fromarray(cv2.cvtColor(cam_frame, cv2.COLOR_BGR2RGBA))
        ir_img = Image.fromarray(cv2.cvtColor(ir_frame, cv2.COLOR_BGR2RGBA))

        cam_imgtk = ImageTk.PhotoImage(image=cam_img)
        ir_imgtk = ImageTk.PhotoImage(image=ir_img)

        self.cam_area.imgtk = cam_imgtk
        self.ir_area.imgtk = ir_imgtk

        self.cam_area.configure(image=cam_imgtk)
        self.ir_area.configure(image=ir_imgtk)

        cv2.waitKey(1)
        self.app.after(1, self.video_stream)


if __name__ == "__main__":
    app = App()
    app.video_stream()
    app.root.mainloop()
