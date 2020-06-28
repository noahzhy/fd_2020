import glob, collections, re, random, os, argparse
import numpy as np
import pandas as pd
import tensorflow as tf
import matplotlib.pyplot as plt

from sklearn.metrics import *
from keras.layers import *
from keras.models import *
from tensorflow.keras.backend import *
from tensorflow.keras.callbacks import *
from tensorflow.keras import optimizers
from keras.utils import plot_model

SEED = 11
random.seed(a=SEED)

batch_size = 1000
epochs = 5


def load_CNN_LSTM():
    model = Sequential()
    model.add(Conv2D(32, (3, 3), padding='same', activation='relu'))
    model.add(MaxPooling2D(pool_size=(2, 2)))
    model.add(Conv2D(64, (3, 3), padding='same', activation='relu'))
    model.add(MaxPooling2D(pool_size=(2, 2)))
    model.add(Conv2D(128, (3, 3), padding='same', activation='relu'))
    model.add(MaxPooling2D(pool_size=(2, 2)))
    model.add(LSTM(50, return_sequences=True))
    model.add(Flatten())
    model.add(Dense(128, activation='relu'))
    model.add(Dense(6, activation='sigmoid'))
    model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
    
    plot_model(model, to_file='model.png')
    # model.fit(xtrain, ytrain, batch_size=batch_size, epochs=epochs)
    # model.save("fir32x24.h5")
    # pred = model.predict(xtest)
    # return pred


def spatial_stream():
    spatial_input = Input(shape=(None, 16, 16, 1), name='spatial_input')
    spatial_conv1 = TimeDistributed(Conv2D(16, (3, 3), padding='same', activation='relu', name='spatial_conv1'), name='spatial_timedistributed1')(spatial_input)
    spatial_bn_layer = TimeDistributed(BatchNormalization(name='spatial_bn_layer'), name='spatial_timedistributed2')(spatial_conv1)
    spatial_maxpool1 = TimeDistributed(MaxPooling2D(pool_size=(2, 2), strides=(2, 2), name='spatial_maxpool1'), name='spatial_timedistributed3')(spatial_bn_layer)
    spatial_conv2 = TimeDistributed(Conv2D(32, (3, 3), padding='same', activation='relu', name='spatial_conv2'), name='spatial_timedistributed4')(spatial_maxpool1)
    spatial_maxpool2 = TimeDistributed(MaxPooling2D(pool_size=(2, 2), strides=(2, 2), name='spatial_maxpool2'), name='spatial_timedistributed5')(spatial_conv2)
    spatial_conv3 = TimeDistributed(Conv2D(64, (3, 3), padding='same', activation='relu', name='spatial_conv3'), name='spatial_timedistributed6')(spatial_maxpool2)
    spatial_maxpool3 = TimeDistributed(MaxPooling2D(pool_size=(2, 2), strides=(2, 2), name='spatial_maxpool3'), name='spatial_timedistributed7')(spatial_conv3)
    spatial_conv4 = TimeDistributed(Conv2D(128, (3, 3), padding='same', activation='relu', name='spatial_conv4'), name='spatial_timedistributed8')(spatial_maxpool3)
    spatial_maxpool4 = TimeDistributed(MaxPooling2D(pool_size=(2, 2), strides=(2, 2), name='spatial_maxpool4'), name='spatial_timedistributed9')(spatial_conv4)
    spatial_flattened = TimeDistributed(Flatten(name='spatial_flattened'), name='spatial_timedistributed10')(spatial_maxpool4)
    spatial_dense1 = TimeDistributed(Dense(512, name='spatial_dense1'), name='spatial_timedistributed11')(spatial_flattened)
    spatial_dense2 = TimeDistributed(Dense(256, name='spatial_dense2'), name='spatial_timedistributed12')(spatial_dense1)
    spatial_LSTM = LSTM(100, return_sequences=True, name='spatial_LSTM')(spatial_dense2)
    spatial_LSTM2 = LSTM(100,  return_sequences=False, name='spatial_LSTM2')(spatial_LSTM)
    #handle numerical instability
    spatial_output = Lambda(lambda x: tensorflow.keras.backend.clip(x, KERAS_EPSILON, 1-KERAS_EPSILON))(spatial_LSTM2)
    return spatial_input, spatial_output


load_CNN_LSTM()