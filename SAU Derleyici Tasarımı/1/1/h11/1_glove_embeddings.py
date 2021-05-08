# -*- coding: utf-8 -*-
"""
https://github.com/fchollet/deep-learning-with-python-notebooks/blob/master/6.1-using-word-embeddings.ipynb
GloVe word embeddings
"""
import os
#Tokenize the data -------------------------------------------------------
from keras.preprocessing.text import Tokenizer
from keras.preprocessing.sequence import pad_sequences
import numpy as np

imdb_dir = 'aclImdb'
train_dir = os.path.join(imdb_dir, 'train')

labels = []
texts = []

for label_type in ['neg', 'pos']:
    dir_name = os.path.join(train_dir, label_type)
    for fname in os.listdir(dir_name):
        #print(label_type,fname)
        if fname[-4:] == '.txt':
            f = open(os.path.join(dir_name, fname),
                     encoding="utf8")
            texts.append(f.read())
            f.close()
            if label_type == 'neg':
                labels.append(0)
            else:
                labels.append(1) #'pos'

maxlen = 100  # yorumları 100 kelime ile sınırlandır
training_samples = 5000  
validation_samples = 10000  
max_words = 10000  
tokenizer = Tokenizer(num_words=max_words)
tokenizer.fit_on_texts(texts)
sequences = tokenizer.texts_to_sequences(texts)

word_index = tokenizer.word_index
print('Found %s unique tokens.' % len(word_index))

data = pad_sequences(sequences, maxlen=maxlen)

labels = np.asarray(labels)
print('Shape of data tensor:', data.shape)
print('Shape of label tensor:', labels.shape)

# Split the data into a training set and a validation set
# But first, shuffle the data, since we started from data
# where sample are ordered (all negative first, then all positive).
indices = np.arange(data.shape[0])
np.random.shuffle(indices)
data = data[indices]
labels = labels[indices]

x_train = data[:training_samples]
y_train = labels[:training_samples]
x_val = data[training_samples: training_samples + validation_samples]
y_val = labels[training_samples: training_samples + validation_samples]

#--------------------------------------------------------------------------
#Pre-process the embeddings Let's parse the un-zipped file (it's a txt file) 
#to build an index mapping words (as strings) to their vector 
#representation (as number vectors).

glove_dir = 'glove.6B'
embeddings_index = {}
f = open(os.path.join(glove_dir, 'glove.6B.100d.txt'),encoding="utf8")
for line in f:
    values = line.split()
    word = values[0]
    coefs = np.asarray(values[1:], dtype='float32')
    embeddings_index[word] = coefs
f.close()
print('Found %s word vectors.' % len(embeddings_index))


embedding_dim = 100
sayac=0
embedding_matrix = np.zeros((max_words, embedding_dim))
for word, i in word_index.items():
    embedding_vector = embeddings_index.get(word)
    #print(word,embedding_vector)
    if i < max_words:
        if embedding_vector is not None:
            # Words not found in embedding index will be all-zeros.
            embedding_matrix[i] = embedding_vector
        else:
            sayac=sayac+1
            #print(word) #?

print('Bulunamayan kelime sayisi=',sayac)





#Define a model
from keras.models import Sequential
from keras.layers import Embedding, Flatten, Dense
model = Sequential()
model.add(Embedding(10000, 100, input_length=100))
model.add(Flatten())
model.add(Dense(32, activation='relu'))
model.add(Dense(1, activation='sigmoid'))

model.summary()
#Load the GloVe embeddings in the model ******************
model.layers[0].set_weights([embedding_matrix])
model.layers[0].trainable = False 
#**********************************************************
model.summary()


#Train and evaluate
model.compile(optimizer='rmsprop',
              loss='binary_crossentropy',
              metrics=['acc','mae','mse'])
history = model.fit(x_train, y_train,
                    epochs=10,
                    batch_size=32,
                    validation_data=(x_val, y_val))
model.save('pre_trained_glove_model.h5')
#model.save_weights('weights.h5')
#model.load_weights('weights.h5')


