import glob, collections, re, random, os
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


def load_data():
    pass


def define_CNN_LSTM():
    model = Sequential()
    # Total params: 995,210
    model.add(TimeDistributed(Conv2D(32, (3, 3), padding='same', activation='relu'), input_shape=(None, 32, 24, 1)))
    model.add(TimeDistributed(MaxPooling2D((2, 2))))
    model.add(TimeDistributed(Conv2D(64, (3, 3), padding='same', activation='relu')))
    model.add(TimeDistributed(MaxPooling2D((2, 2))))
    model.add(TimeDistributed(Conv2D(128, (3, 3), padding='same', activation='relu')))
    model.add(TimeDistributed(MaxPooling2D((2, 2))))
    model.add(TimeDistributed(Flatten()))
    model.add(LSTM(128, return_sequences=True))
    model.add(LSTM(64, return_sequences=False))
    # model.add(Dense(64, activation='relu'))
    model.add(Dense(10, activation='softmax'))
    model.compile(loss='categorical_crossentropy', optimizer='rmsprop', metrics=['acc'])

    model.summary()
    plot_model(model, to_file='model.png', rankdir='TB')

    return model

    # pred = model.predict(xtest)
    # return pred

def fit_model(model):
    history = model.fit(xtrain, ytrain, batch_size=batch_size, epochs=epochs)
    
    model.save("fir_32x24.h5")
    return history


def show_acc_loss(history):
    acc = history.history['acc']
    val_acc = history.history['val_acc']
    loss = history.history['loss']
    val_loss = history.history['val_loss']

    epochs = range(1, len(acc)+1)

    plt.plot(epochs, acc, 'bo', label='Training acc')
    plt.plot(epochs, val_acc, 'b', label='Validation acc')
    plt.title('Training and validation accuracy')
    plt.legend()

    plt.figure()

    plt.plot(epochs, loss, 'bo', label='Training loss')
    plt.plot(epochs, val_loss, 'b', label='Validation loss')
    plt.title('Training and validation loss')
    plt.legend()
    
    plt.show()


if __name__ == "__main__":
    xtrain, ytrain = load_data()
    model = define_CNN_LSTM(xtrain, ytrain)
    history = fit_model(model)
    show_acc_loss(history)
