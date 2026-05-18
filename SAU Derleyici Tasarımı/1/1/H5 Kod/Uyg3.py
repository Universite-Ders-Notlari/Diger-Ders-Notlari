#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Predicting house prices: a regression example

*k-fold validation eklendi

"""
from keras.datasets import boston_housing
from keras import models,layers
import matplotlib.pyplot as plt
import numpy as np
#veri setini yukle
(train_data, train_targets), (test_data, test_targets) =\
  boston_housing.load_data()
  
#normalizasyon  
mean = train_data.mean(axis=0)
train_data -= mean
std = train_data.std(axis=0)
train_data /= std

test_data -= mean
test_data /= std
  
#modeli tanimla
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


k = 4# 4-fold val
num_val_samples = len(train_data) // k
num_epochs =5
all_scores = []
for i in range(k):
    print('processing fold #', i)
    # Prepare the validation data: data from partition # k
    val_data = train_data[i * num_val_samples: (i + 1) * num_val_samples]
    val_targets = train_targets[i * num_val_samples: (i + 1) * num_val_samples]

    # Prepare the training data: data from all other partitions
    partial_train_data = np.concatenate(
        [train_data[:i * num_val_samples],
         train_data[(i + 1) * num_val_samples:]],
        axis=0)
    partial_train_targets = np.concatenate(
        [train_targets[:i * num_val_samples],
         train_targets[(i + 1) * num_val_samples:]],
        axis=0)

    # Build the Keras model (already compiled)
    model = build_model()
    # Train the model (in silent mode, verbose=0)
    model.fit(partial_train_data, partial_train_targets,
              epochs=num_epochs, batch_size=1, verbose=0)
    # Evaluate the model on the validation data
    val_mse, val_mae = model.evaluate(val_data, val_targets, verbose=0)
    print('\tval_mse=', val_mse, 'val_mae=',val_mae)
    all_scores.append(val_mse)


print('ortalama_mse=',np.sum(all_scores)/k)
