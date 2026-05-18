#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Predicting house prices: a regression example

*onceki koda normalizasyon eklendi
*model fonksiyon içerisinde tanımlandı
"""
from keras.datasets import boston_housing
from keras import models,layers
import matplotlib.pyplot as plt
#veri setini yukle---------------------------------------
(train_data, train_targets), (test_data, test_targets) =\
  boston_housing.load_data()
  
#min-max normalizasyon-----------------------------------
mean = train_data.mean(axis=0)
train_data -= mean
std = train_data.std(axis=0)
train_data /= std
test_data -= mean
test_data /= std
  
#modeli tanimla ----------------------------------------------
def build_model():
    model1=models.Sequential()
    model1.add(layers.Dense(64,activation='relu',input_shape=(13,)))#train_data.shape[1]
    model1.add(layers.Dense(64,activation='relu'))
    model1.add(layers.Dense(1,activation='linear'))
    
    #optimizasyon parametreleri
    model1.compile(optimizer='rmsprop',
                   loss='mse',
                   metrics=['mae'])
    return model1

model1=build_model()

#Egitim---------------------------------------------------------
h=model1.fit(train_data, 
           train_targets,
           epochs=10,
           batch_size=1,
           validation_data=(test_data, test_targets)
           )
#grafikler ------------------------------------------------
epochs=range(1,1+len(h.history['loss']))
plt.plot(epochs,h.history['loss'])
plt.plot(epochs,h.history['val_loss'])
plt.xlabel('epoch')
plt.ylabel('loss')

