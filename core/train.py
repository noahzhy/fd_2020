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

from core.DataGenerator import DataGenerator
from keras.preprocessing.image import ImageDataGenerator


# SEED = 11
# random.seed(a=SEED)

batch_size = 128
epochs = 5


def load_data():
    return (xtrain, ytrain), (xtest, ytest)


def define_CNN_LSTM():
    model = Sequential()
    # Total params: 995,210
    model.add(TimeDistributed(Conv2D(32, (3, 3), padding='same', activation='relu'), input_shape=(None, 32, 24, 1)))
    # BatchNormalization maybe doesn't work on small batch size
    model.add(TimeDistributed(BatchNormalization()))
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
    model.compile(
        optimizer=optimizers.RMSprop(),
        loss='categorical_crossentropy',
        metrics=['acc']
    )

    model.summary()
    plot_model(model, to_file='model.png', rankdir='TB')

    return model

    # pred = model.predict(xtest)
    # return pred

def fit_model(model, xtest, ytest):
    history = model.fit_generator(
        xtrain,
        ytrain,
        batch_size=batch_size,
        epochs=epochs,
        validation_data=(xtest, ytest)
    )

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
    parser = argparse.ArgumentParser()

    # (xtrain, ytrain), (xtest, ytest) = load_data()
    model = define_CNN_LSTM()
    # history = fit_model(model, xtrain, ytrain)
    # show_acc_loss(history)
