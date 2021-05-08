# -*- coding: utf-8 -*-
"""
İkili sınıflandırma (Binary classification)
*örnekte imdb veri setindeki yorumlar
  olumlu ve olumsuz olmak üzere iki sınıfa ayrılmıştır.
  
* yorumlardaki kelimeler en sık 10000 kelime ile sınırlandırılmıştır.

* Ağın 10000 girişi bulunmaktadır her bir giriş bir kelimeyi temsil eder

* Bir yorumda kullanılan kelimeler vektör üzerindeki işaretlenir 
   ve bu şekilde ağa uygulanır. Bu amaçla vectorize_sequences fonksiyonu 
   sıfırlardan oluşan 10000 elemanlı vektörü oluşturur ve ilgili yorumdaki 
   kelimeleri işaretler.

*ikili sınıflandırma yapıldığı için son katman tek çıkışa sahiptir ve 
    simoid fonksiyonu kullanılmıştır.
    
*Bu uygulama daha sonra RNN ve Embedding vektörler ile 
 daha etkili gerçekleştirilecektir.

"""
import numpy as np
from keras import models
from keras import layers
from keras import optimizers
from keras import metrics
from keras import losses
from keras.datasets import imdb
import matplotlib.pyplot as plt

from keras.preprocessing.text import Tokenizer

def vectorize_sequences(sequences, dimension=10000):
    # Sıfırlardan oluşan, (len(sequences), dimension) boyutlu matris
    results = np.zeros((len(sequences), dimension))
    for i, sequence in enumerate(sequences):
        results[i, sequence] = 1. 
    return results
    
(train_data, train_labels),(test_data, test_labels)=imdb.load_data(num_words=10000)

# Eğitim ve test verilerini vektöre dönüştür
x_train = vectorize_sequences(train_data)
x_test = vectorize_sequences(test_data)


# vektöre dönştürme Tokenizer ile yapılabilir
#tokenizer = Tokenizer(num_words=10000)
#x_train = tokenizer.sequences_to_matrix(train_data, mode='binary')
#x_test = tokenizer.sequences_to_matrix(test_data, mode='binary')
#
#

# Etiketleri vektöre dönüştür
#y_train = np.asarray(train_labels).astype('float32')
#y_test = np.asarray(test_labels).astype('float32')
y_train = train_labels
y_test = test_labels


#modeli tanımla----------------------------------------
model=models.Sequential()
model.add(layers.Dense(16,
                       activation='relu',
                       input_shape=(10000,)))
model.add(layers.Dense(16,
                       activation='relu'))
model.add(layers.Dense(1,
                       activation='sigmoid'))

# Optimizasyon parametreleri----------------------------
model.compile(optimizer=optimizers.RMSprop(lr=0.01),
              loss=losses.binary_crossentropy, 
              metrics=["accuracy"])

# Eğitim ---------------------------------------------------------
x_val = x_train[:10000] 
partial_x_train = x_train[10000:]

y_val = y_train[:10000]
partial_y_train = y_train[10000:]

history = model.fit(partial_x_train,
                    partial_y_train,
                    epochs=10,
                    batch_size=512,
                    validation_data=(x_val, y_val))
                    

#history = model.fit(x_train,
#                    y_train,
#                    epochs=5,
#                    batch_size=512,
#                    validation_split=0.1)# ! önceki örnekten farklı satır
#         



# Grafikler ------------------------------------------------
history_dict = history.history
loss_values = history_dict['loss']
val_loss_values = history_dict['val_loss']

epochs = range(1,
               len(loss_values ) + 1)

plt.figure(1)
plt.plot(epochs, loss_values, 'ro', label='Training loss')
plt.plot(epochs, val_loss_values, 'r', label='Validation loss')
plt.title('Training and validation loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')

plt.legend()
plt.show()

plt.figure(2)
acc = history.history['acc']
val_acc = history.history['val_acc']

plt.plot(epochs, acc, 'ro', label='Training acc')
plt.plot(epochs, val_acc, 'r', label='Validation acc')
plt.title('Training and validation accuracy')
plt.xlabel('Epochs')
plt.ylabel('acc')
plt.legend()

plt.show()
