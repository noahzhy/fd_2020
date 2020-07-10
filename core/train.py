import glob
import collections
import re
import random
import os
import argparse
import numpy as np
import pandas as pd
import tensorflow as tf
import matplotlib.pyplot as plt

from sklearn.metrics import *
from keras.layers import *
from keras.models import *
from keras.utils import plot_model
from keras import optimizers, callbacks, backend

# set the seed to random fixly
# SEED = 11
# random.seed(a=SEED)

batch_size = 1000
epochs = 5


def init_CNN_LSTM():
    model = Sequential()
    # Total params: 281,738
    model.add(TimeDistributed(Conv2D(32, (3, 3), padding='same', activation='relu'), input_shape=(None, 32, 24, 1)))
    model.add(TimeDistributed(MaxPooling2D((2, 2))))
    model.add(TimeDistributed(Conv2D(64, (3, 3), padding='same', activation='relu')))
    model.add(TimeDistributed(MaxPooling2D((2, 2))))
    model.add(TimeDistributed(Conv2D(64, (3, 3), padding='same', activation='relu')))
    model.add(TimeDistributed(MaxPooling2D((2, 2))))
    model.add(TimeDistributed(Flatten()))
    model.add(LSTM(64, return_sequences=True))
    model.add(LSTM(32, return_sequences=False))
    # model.add(Dense(64, activation='relu'))
    model.add(Dense(10, activation='softmax'))
    model.compile(loss='categorical_crossentropy', optimizer='rmsprop', metrics=['accuracy'])

    plot_model(model, to_file='model.png', rankdir='TB')
    # model.build()
    model.summary()
    # model.fit(xtrain, ytrain, batch_size=batch_size, epochs=epochs)
    # model.save("fir_32x24.h5")
    # pred = model.predict(xtest)
    # return pred


init_CNN_LSTM()
