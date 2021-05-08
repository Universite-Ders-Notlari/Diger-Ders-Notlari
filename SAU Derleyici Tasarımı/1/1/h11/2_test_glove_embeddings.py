# -*- coding: utf-8 -*-
"""
"""
from keras.preprocessing.text import Tokenizer
from keras.preprocessing.sequence import pad_sequences
import numpy as np
import os
from keras import models

def loadImdbTest():
    imdb_dir = 'aclImdb'
    maxlen = 100 
    max_words = 10000  
    tokenizer = Tokenizer(num_words=max_words)
    test_dir = os.path.join(imdb_dir, 'test')
    labels = []
    texts = []
    for label_type in ['neg', 'pos']:
        dir_name = os.path.join(test_dir, label_type)
        for fname in sorted(os.listdir(dir_name)):
            #print('fname=',fname,' ',label_type)
            if fname[-4:] == '.txt':                
                f = open(os.path.join(dir_name, fname),
                         encoding="utf8")
                texts.append(f.read())
                #print('texts=',texts)
                f.close()
                if label_type == 'neg':
                    labels.append(0)
                else:
                    labels.append(1)
                    
    tokenizer.fit_on_texts(texts)
    sequences = tokenizer.texts_to_sequences(texts)
    x_test = pad_sequences(sequences, maxlen=maxlen)
    y_test = np.asarray(labels)
    return (x_test,y_test)

(x_test, y_test)=loadImdbTest()
model=models.load_model('pre_trained_glove_model.h5')
[loss,acc]=model.evaluate(x_test, y_test)
print("loss=",loss,"acc=",acc)
