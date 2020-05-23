import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from matplotlib.animation import FFMpegWriter


skip_frame = 0
plt.ion()
fig = plt.figure(figsize=(5, 5))
metadata = dict(title='Movie', artist='Matplotlib', comment='Movie support!')
writer = FFMpegWriter(fps=10, metadata=metadata)


def show_image(array_list, num=None):
    plt.xlabel('Frame: {}'.format(num))
    if num>15 and num<75: # 01
    # if num>55 and num<120: # 02
        plt.ylabel('Status: {}'.format('fall'), color='r')
    else:
        plt.ylabel('Status: {}'.format('normal'), color='g')
    plt.imshow(array_list)
    plt.pause(0.01)
    writer.grab_frame()
    plt.clf()


def load_data(path):
    df = pd.read_csv(path, usecols=range(1, 257))
    with writer.saving(fig, "sample_01.avi", 100):
        for d in df.index:
            data = df.iloc[d]
            data_list = np.array(data).reshape(16, 16).tolist()
            show_image(data_list, d)


if __name__ == "__main__":
    load_data(r"dataset\sample\sample_01.csv")
