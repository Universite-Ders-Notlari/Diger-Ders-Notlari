#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Predicting house prices: a regression example

"""
from keras.datasets import boston_housing
from keras import models,layers
import matplotlib.pyplot as plt
#veri setini yukle
(train_data, train_targets), (test_data, test_targets) =\
  boston_housing.load_data()
  
#modeli tanimla
model1=models.Sequential()
model1.add(layers.Dense(64,activation='relu',input_shape=(13,)))#train_data.shape[1]
model1.add(layers.Dense(64,activation='relu'))
model1.add(layers.Dense(1,activation='linear'))

#optimizasyon parametreleri
model1.compile(optimizer='adam',
               loss='mse',
               metrics=['mae'])

#Egitim
h=model1.fit(train_data, 
           train_targets,
           epochs=10,
           batch_size=1,
           verbose=1,
           )
#grafikler
epochs=range(1,1+len(h.history['loss']))
plt.plot(epochs,h.history['loss'])
plt.xlabel('epoch')
plt.ylabel('loss')
