import glob
import time
import pandas as pd
import numpy as np
import os
import csv


def load_data(path=''):
    dir_list = glob.glob('*/raw/*.csv')
    print('len:', len(dir_list))
    for i in dir_list:
        print(i)
        df = pd.read_csv(i, header=1)
        # df = pd.read_csv(i, header=0, skiprows=range(1, 15))
        df = df.round(2)
        df.to_csv(i, index=False)
        # df.to_csv('data.csv', index=False)
        # print(df)
        # break


if __name__ == "__main__":
    load_data()
