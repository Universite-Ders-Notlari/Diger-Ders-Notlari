# -*- coding: utf-8 -*-
"""
Forward propagation örneği
2-girişli  1-çıkışlı örnek yapar sinir ağı
1. katman 3 adet, 2. katman 1 adet yapay sinir içeriyor

"""
import numpy as np

def linear(n):
    return n

X=np.array([[5],
            [3]])
#1. katman
#eğitilecek parametreler
W1=np.array([[0.5, 0.5],
             [0.5, 0.5],
             [0.5, 0.5]])
    
b1=np.array([[0.5],
             [0.5],
             [0.5]])

# y1=f(W1*X+b1)= linear(W1*X+b1)=W1*X+b1
y1=linear(np.matmul(W1,X)+b1)

#2. katman
W2=np.array([0.5, 0.5, 0.5])

b2=np.array([0.5])

y2=linear(np.matmul(W2,y1)+b2)

print("y=",y2)

