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
import matplotlib.pyplot as plt
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
model.add(layers.Conv2D(64,
                        (3,3),
                        padding='same',
                        activation='relu'))
model.add(layers.MaxPool2D())
model.add(layers.Conv2D(64,
                        (3,3),
                        padding='same',
                        activation='relu'))
model.add(layers.MaxPool2D())
model.add(layers.Flatten())
model.add(layers.Dense(512,activation='relu'))
model.add(layers.Dense(10,activation='softmax'))
model.summary()
#○modeli derle
from keras import optimizers
model.compile(loss='categorical_crossentropy',
              optimizer=optimizers.RMSprop(lr=1e-4),
              metrics=['acc'])


#data augmentation ***********************************
def fonk(img):
    plt.imshow(img)
    plt.show()
    return img
from keras.preprocessing.image import ImageDataGenerator
train_datagen = ImageDataGenerator( 
      preprocessing_function=fonk,
      rotation_range=40,
      width_shift_range=0.2,
      height_shift_range=0.2,
      shear_range=0.2,
      zoom_range=0.2,
      horizontal_flip=True,
      vertical_flip=True,
      fill_mode='nearest')

train_generator = train_datagen.flow(
        x_train,
        y_train,
        batch_size=20)

test_datagen = ImageDataGenerator()
validation_generator = test_datagen.flow(
        x_test,
        y_test,
        batch_size=20)
        #class_mode='categorical')


#history=model.fit(x_train,
#                  y_train,
#                  epochs=100,
#                  validation_data=(x_test,y_test))

history = model.fit_generator(
      train_generator,
      steps_per_epoch=100,
      epochs=30,
      validation_data=validation_generator,
      validation_steps=100)

#model.save('cifar10_model1.h5')
#model.save_weights('cifar10_weights1.h5')
model.load_weights('cifar10_weights1.h5')
#from keras import models
#models.load_model('cifar10_model1.h5')



acc = history.history['acc']
val_acc = history.history['val_acc']
loss = history.history['loss']
val_loss = history.history['val_loss']

import numpy as np

#np.save('history1.npy',(acc,val_acc,loss,val_loss))
(acc,val_acc,loss,val_loss)=np.load('history.npy')

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






