# -*- coding: utf-8 -*-
"""

"""

from keras.datasets import imdb
from keras import preprocessing
from keras.models import Sequential
from keras.layers import Flatten, Dense, Embedding, Dropout

# Özellik olarak düşnülecek kelime sayısı
max_features = 10000
#100 kelimeden sonra metni kes
maxlen =100 #20 

#veriyi tamsayılar listesi olarak yükle
(x_train1, y_train), (x_test1, y_test) = \
imdb.load_data(num_words=max_features)

# veriyi 2D tensor olarak yükle (samples, maxlen)
x_train = preprocessing.sequence.pad_sequences(
        x_train1, maxlen=maxlen)
x_test = preprocessing.sequence.pad_sequences(
        x_test1, maxlen=maxlen)


#Bir embedding katmanı ve sınıflandırıcı kullan


model = Sequential()

model.add(Embedding(10000, 
                    8, 
                    input_length=maxlen))

model.add(Flatten())
#model.add(Dropout(0.3))
model.add(Dense(1, 
                activation='sigmoid'))

model.summary()
from keras import optimizers
model.compile(optimizer=optimizers.RMSprop(),
              loss='binary_crossentropy',
              metrics=['acc'])


history = model.fit(x_train, y_train,
                    epochs=5,
                    batch_size=32,
                    validation_split=0.2)

model.save('embedding1.h5')