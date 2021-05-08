# -*- coding: utf-8 -*-
"""
Random ağırlıklar
Forward propagation örneği
2-girişli  1-çıkışlı örnek yapar sinir ağı
1. katman 3 adet, 2. katman 1 adet yapay sinir içeriyor
"""

import numpy as np

def linear(n):
    return n

def sigmoid(x):
    y=1/(1+np.exp(-x))
    return y

# giriş vektörü
X=np.array([[5],[3]])
#1. katman --------------------
W1=np.random.rand(3,2)-0.5
b1=np.random.rand(3,1)  

# y1=f(W1*X+b1)= linear(W1*X+b1)=W1*X+b1
y1=linear(np.matmul(W1,X)+b1)

#2. katman
W2=np.random.rand(1,3)-0.5
b2=np.random.rand(1)

y2=sigmoid(np.matmul(W2,y1)+b2)

print("y=",y2)

