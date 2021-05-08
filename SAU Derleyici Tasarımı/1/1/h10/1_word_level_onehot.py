# -*- coding: utf-8 -*-
"""
One-hot word embedding örneği
Programda verilen cümledeki farklı kelimeler bulunup,
her birine farklı bir indis atanıyor.
"""

import numpy as np

samples = ['Bromwell High is a cartoon comedy. It ran at the same time as some other programs about school life, such as "Teachers". My 35 years in the teaching profession lead me to believe that Bromwell High's satire is much closer to reality than is "Teachers". The scramble to survive financially, the insightful students who can see right through their pathetic teachers' pomp, the pettiness of the whole situation, all remind me of the schools I knew and their students. When I saw the episode in which a student repeatedly tried to burn down the school, I immediately recalled ......... at .......... High. A classic line: INSPECTOR: Im here to sack one of your teachers. STUDENT: Welcome to Bromwell High. I expect that many adults of my age think that Bromwell High is far fetched. What a pity that it isnt!']

token_index = {} #boş dictionary

print('Kelimelere atanan indisler:')
for sample in samples:
    print('sample=',sample)
    for word in sample.split():# sıradaki kelimeyi al
        print('word=',word)
        if word not in token_index:# token indeks verilmemişse            
            token_index[word] = len(token_index) + 1 # sıraki tamsayıyı ata
            print('\t',word,'=',token_index[word])
            
max_length = 10
results = np.zeros(shape=(len(samples),
                          max_length,
                          max(token_index.values()) + 1))

print('Kelimelere vektör ata:')
for i, sample in enumerate(samples):
    for j, word in list(enumerate(sample.split()))[:max_length]:
        index = token_index.get(word)
        results[i, j, index] = 1.

           

#print(i,j,sample,word)