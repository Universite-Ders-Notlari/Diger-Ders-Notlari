# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as grafik
from matplotlib.pyplot import plot
import nnw 

# SGD (online training)
#VERI SETI ---------------------------------------------
#X=np.array([-2,-1.5,-1,-0.5, 0,0.5,1,1.5,2],dtype='f') 
X=np.linspace(-2,2,9)
#T=np.array([0, 0.075,0.292,0.617,1.0, 1.382,1.707,1.923,2])
T=1+np.sin(X*np.pi/4)
W1=np.random.rand(2,1)
b1=np.random.rand(2,1)
W2=np.random.rand(1,2)
b2=np.random.rand(1)

alfa=0.1#öğrenme oranı(learning rate)
epoch=20

hataMSE=np.empty(epoch)
for k in range(epoch):#Eğitim setinin kaç tur dolaşılacağını belirler
    
    for i in range(X.size):
        #print(i)
        #1. katman 
        y1=nnw.sigmoid( W1*X[i]+b1)
        #2. katman
        y2=np.matmul(W2,y1)+b2 #W2*y1, linear: f(n)=n
        #hata
        e=T[i]-y2        
        
        #GERI YAYILIMF2=[1];
        F2=1
        d2=-2*F2*e 
        
        F1=np.array([[ (1-y1[0])*y1[0] , 0],
                    [0 , (1-y1[1])*y1[1]] ]) 
                
        d1= np.matmul(F1.astype(float), W2.reshape(2,1))*d2   
        
        # 2. Katmandaki parametreler
        W2=W2-alfa*d2*y1.reshape(1,2) #y1'
        b2=b2-alfa*d2        
        #1. Katmandaki parametreler
        W1=W1-alfa*d1*X[i] #X(i)' 
        b1=b1-alfa*d1    
    #Doğruluk testi
    hata=0
    for i in range(len(X)):
        #1. katman
        Y1=nnw.sigmoid( W1*X[i]+b1)
        #2. katman
        Y2=np.matmul(W2,Y1)+b2 #linear
        hata= hata+(T[i]-Y2)**2  
    
    MSE=hata/len(X)
    print("MSE=",MSE)
    hataMSE[k]=MSE
    
grafik.figure(0)
grafik.plot(range(epoch),hataMSE,'ro')
grafik.xlabel("iterasyon")
grafik.ylabel("MSE")
print("W1=",W1,"\nW2",W2)
print("b1=",b1,"\nb2",b2)    

#Eğitilmiş ağın çıkışını diziye yaz
Y=np.empty(len(X))
for i in range(len(X)):
    #1. katman
    Y1=nnw.sigmoid( W1*X[i]+b1)
    #2. katman
    Y[i]=np.matmul(W2,Y1)+b2 #linear

#Karşılaştır
grafik.figure(1)
plot(X,Y,'ro')
grafik.plot(X,T,'b*')
grafik.xlabel("X")
grafik.ylabel("Y ve T")
