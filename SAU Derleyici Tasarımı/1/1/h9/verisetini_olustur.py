# -*- coding: utf-8 -*-
"""
Cifar0 verisetini klasörlere dagit8
"""

import matplotlib.pyplot as plt
import numpy as np
import os
from keras.datasets import cifar10
(x_train, y_train), (x_test, y_test) = cifar10.load_data()

os.mkdir('dataset')
os.mkdir('dataset\\train')
os.mkdir('dataset\\test')

for i in range(10):
    path=os.path.join('dataset\\train',str(i))
    os.mkdir(path)
    path=os.path.join('dataset\\test',str(i))
    os.mkdir(path)

import matplotlib.pyplot as plt
for i in range(50000):
    path='dataset/train/'+str(int(y_train[i]))+'/'+str(i)+'.png'  
    plt.imsave(path,x_train[i])
    
for i in range(10000):
    path='dataset/test/'+str(int(y_test[i]))+'/'+str(i)+'.png'  
    plt.imsave(path,x_test[i])

    