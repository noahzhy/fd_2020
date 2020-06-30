import glob, collections, re, random, os, argparse
import numpy as np
import pandas as pd
import tensorflow as tf
import matplotlib.pyplot as plt

from sklearn.metrics import *
from keras.layers import *
from keras.models import *
from keras.utils import plot_model
from keras import optimizers, callbacks, backend

SEED = 11
random.seed(a=SEED)

batch_size = 1000
epochs = 5


def load_CNN_LSTM():
    model = Sequential()
    model.add(TimeDistributed(
        Conv2D(32, (3, 3), padding='same', activation='relu')))
    model.add(TimeDistributed(BatchNormalization()))
    model.add(TimeDistributed(MaxPooling2D(pool_size=(2, 2))))
    model.add(TimeDistributed(
        Conv2D(64, (3, 3), padding='same', activation='relu')))
    model.add(TimeDistributed(MaxPooling2D(pool_size=(2, 2))))
    # model.add(TimeDistributed(
    #     Conv2D(128, (3, 3), padding='same', activation='relu')))
    # model.add(TimeDistributed(MaxPooling2D(pool_size=(2, 2))))
    model.add(TimeDistributed(Flatten()))
    model.add(LSTM(256, return_sequences=True))
    model.add(LSTM(128, return_sequences=False))
    model.add(Dense(64, activation='relu'))
    model.add(Dense(8, activation='softmax'))
    model.compile(loss='categorical_crossentropy',
                  optimizer='adam', metrics=['accuracy'])

    plot_model(model, to_file='model.png', rankdir='LR')
    # model.build()
    # model.summary()
    # model.fit(xtrain, ytrain, batch_size=batch_size, epochs=epochs)
    # model.save("fir32x24.h5")
    # pred = model.predict(xtest)
    # return pred


load_CNN_LSTM()
