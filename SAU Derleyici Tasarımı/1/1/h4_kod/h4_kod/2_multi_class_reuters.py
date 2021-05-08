# -*- coding: utf-8 -*-
"""
Çok sınıflı sınıflandırma  (Multiclass classification)
* Haber metinleri 46 farklı kategoriye ayrılmıştır
* imdb örneğine benzer şekilde kelimeler 10000 ile sınırlandırılmıştır
* vectorize_sequences fonksiyonuna alterntif olarak Tokenizer kutuphanesinden
   sequences_to_matrix fonksiyonu kullanılmıştır 
* Ağ 46 çıkışlı olarak tanımlanmış ve softmax fonksiyonu kullanılmıştır.
* train_label veya test_label 0-45 arası tamsayı değerler sakladığı için
   keras.utils.to_categorical fonksiyonu ile herbir etiket 46 elemanlı
   çıkış vektörüne dönüştürülmüştür. Yani haber 10 numaralı kategorideyse
   vektörde bu konum 1 diğerleri 0 olarak tanımlanır. 
"""

import keras
from keras.datasets import reuters
from keras.models import Sequential
from keras.layers import Dense, Activation
from keras.preprocessing.text import Tokenizer

#(x_train, y_train), (x_test, y_test) =\
# reuters.load_data(num_words=None, test_split=0.2)
max_words = 10000
(train_data, train_labels), (test_data, test_labels) =\
reuters.load_data(num_words=max_words)
 
 
word_index = reuters.get_word_index()

print('# of Training Samples: {}'.format(len(train_data)))
print('# of Test Samples: {}'.format(len(test_data)))
      
word_index = reuters.get_word_index()

    
num_classes = max(train_labels) + 1

print('# of Classes: {}'.format(num_classes))
      
index_to_word = {}
for key, value in word_index.items():
    index_to_word[value] = key

print(' '.join([index_to_word[x] for x in train_data[0]]))
print(train_labels[0])


#10000 kelime için 10000 elemanlı binary vektör oluştur
#ilgili yorum içerisinde kullanılan kelimeler 1 diğerleri 0 olarak işaretlenir
tokenizer = Tokenizer(num_words=max_words)
x_train = tokenizer.sequences_to_matrix(train_data, mode='binary')
x_test = tokenizer.sequences_to_matrix(test_data, mode='binary')

#one-hot coding
#46 elemanlı (her sınıf için) binary vektör
y_train = keras.utils.to_categorical(train_labels, num_classes)
y_test= keras.utils.to_categorical(test_labels, num_classes)

#model
model = Sequential()
model.add(Dense(32,activation='relu', input_shape=(10000,)))
#model.add(Activation('relu'))
model.add(Dense(32))
model.add(Activation('relu'))
model.add(Dense(46,activation='softmax'))
#model.add(Activation('softmax'))


model.compile(loss='categorical_crossentropy', 
              optimizer='rmsprop', 
              metrics=['accuracy'])

print(model.metrics_names)
batch_size = 512
epochs = 10

history = model.fit(x_train, 
                    y_train, 
                    batch_size=batch_size, 
                    epochs=epochs, #verbose=1, 
                    validation_split=0.1)#!!


score = model.evaluate(x_test,
                       y_test,
                       batch_size=batch_size)


print('Test loss:', score[0])
print('Test accuracy:', score[1])


import matplotlib.pyplot as plt

loss = history.history['loss']
val_loss = history.history['val_loss']

epochs = range(1, len(loss) + 1)

plt.figure(1)
plt.plot(epochs, loss, 'bo', label='Training loss')
plt.plot(epochs, val_loss, 'b', label='Validation loss')
plt.title('Training and validation loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.show()

plt.figure(2)
acc = history.history['acc']
val_acc = history.history['val_acc']
acc = history.history['acc']
val_acc = history.history['val_acc']
plt.plot(epochs, acc, 'ro', label='Training acc')
plt.plot(epochs, val_acc, 'r', label='Validation acc')
plt.title('Training and validation accuracy')
plt.xlabel('Epochs')
plt.ylabel('acc')
plt.legend()

plt.show()
