# -*- coding: utf-8 -*-
"""
Cifar10 veri setinin konvolüsyon ağı ile eğitimi
"""
import keras
from keras.datasets import cifar10
from keras.models import Sequential
from keras.layers import Dense,Flatten
from keras.layers import Conv2D, MaxPooling2D,Activation
from keras import layers
import matplotlib.pyplot as plt
from keras.utils import to_categorical

# Eğitim ve test veri setleri
(x_train, y_train), (x_test, y_test) = cifar10.load_data()

#for i in range(0,16):
#    plt.subplot(4,4,i+1)
#    plt.imshow(x_train[i])
#
#normalizasyon
x_train=x_train.astype('float32')/255.0
x_test=x_test.astype('float32')/255


y_train=to_categorical(y_train,10)
y_test=to_categorical(y_test,10)
#model
model=Sequential()
model.add(layers.Conv2D(32,
                        (3,3),
                        activation='relu',
                        padding='same',
                        input_shape= (32, 32, 3)))
#model.add(Activation('relu'))
model.add(layers.Conv2D(32,
                        (3,3),
                        padding='same',
                        activation='relu'))
model.add(layers.MaxPool2D())

model.add(layers.Dropout(0.25))

model.add(layers.Conv2D(64,
                        (3,3),
                        padding='same',
                        activation='relu'))
model.add(layers.MaxPool2D())
model.add(layers.Dropout(0.25))
model.add(layers.Conv2D(64,
                        (3,3),
                        padding='same',
                        activation='relu'))
model.add(layers.MaxPool2D())

model.add(layers.Dropout(0.5))

model.add(layers.Flatten())
model.add(layers.Dense(512,activation='relu'))
model.add(layers.Dense(10,activation='softmax'))
model.summary()
#○modeli derle
from keras import optimizers
model.compile(loss='categorical_crossentropy',
              optimizer=optimizers.RMSprop(lr=1e-4),
              metrics=['acc'])

history=model.fit(x_train,
                  y_train,
                  epochs=100,
                  validation_data=(x_test,y_test))
#
#def benimfonk(img):
#    plt.imshow(im)
#    return img
#preprocessing_function=benimfonk(img),

#model.save('cifar10_model1.h5')
#model.save_weights('cifar10_weights1.h5')
model.load_weights('cifar10_weights1.h5')
#from keras import models
#models.load_model('cifar10_model1.h5')

import matplotlib.pyplot as plt

acc = history.history['acc']
val_acc = history.history['val_acc']
loss = history.history['loss']
val_loss = history.history['val_loss']

import numpy as np

#np.save('history1.npy',(acc,val_acc,loss,val_loss))
(acc,val_acc,loss,val_loss)=np.load('history_dropout.npy')

epochs = range(1,len(acc)+1)

plt.plot(epochs, acc, 'bo', label='Training acc')
plt.plot(epochs, val_acc, 'r', label='Validation acc')
plt.title('Training and validation accuracy')
plt.xlabel('epochs')
plt.ylabel('accuracy')
plt.legend()

plt.figure()

plt.plot(epochs, loss, 'bo', label='Training loss')
plt.plot(epochs, val_loss, 'r', label='Validation loss')
plt.title('Training and validation loss')
plt.xlabel('epochs')
plt.ylabel('loss')
plt.legend()

plt.show()






