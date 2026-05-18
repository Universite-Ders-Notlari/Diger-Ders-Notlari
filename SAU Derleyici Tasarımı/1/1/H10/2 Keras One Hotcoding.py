# -*- coding: utf-8 -*-
"""
Keras - Tokenizer ile
One-hot word embedding örneği

"""

from keras.preprocessing.text import Tokenizer

samples = ['The cat sat on the mat.', 
           'The dog ate my homework.']

# We create a tokenizer, configured to only take
# into account the top-1000 most common words
tokenizer = Tokenizer(num_words=1000)

# This builds the word index
tokenizer.fit_on_texts(samples)

# This turns strings into lists of integer indices.
sequences = tokenizer.texts_to_sequences(samples)

# You could also directly get the one-hot binary representations.
# Note that other vectorization modes than one-hot encoding are supported!
one_hot_results = tokenizer.texts_to_matrix(samples, 
                                            mode='binary')

# Belirlenen kelime indekslerini al
word_index = tokenizer.word_index
print('Found %s unique tokens.' % len(word_index))
print(word_index)
