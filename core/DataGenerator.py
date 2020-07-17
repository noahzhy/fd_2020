import os
import random
import numpy as np
from pasta import augment


class DataGenerator:
    '''
    FIR data generator
    ------------------
    [temperature, y] : list
        temperature : numpy array, shape:(32, 24)
        y : numpy array
    '''
    def __init__(self, data, batch_size:int, steps:int):
        self.data = data
        if (batch_size == -1):
            self.batch_size = len(data)
        else:
            self.batch_size = batch_size

    def __len__(self):
        return int(np.floor(len(self.data) / self.batch_size))

