from sklearn.datasets import load_boston
from sklearn.model_selection import train_test_split

import numpy

X,y=load_boston(return_X_y=True)
trainx,testx,trainy,testy=train_test_split(X,y,test_size=0.33,random_state=42)

def linear_regression(X_train,y_train):
    X_one = numpy.hstack((X_train, numpy.ones((X_train.shape[0], 1))))
    return numpy.linalg.inv(X_one.T.dot(X_one)).dot(X_one.T).dot(y_train)

def MSE(X_train, y_train, X_test, y_test):
    w = linear_regression(X_train, y_train)
    X_one = numpy.hstack((X_test, numpy.ones((X_test.shape[0], 1))))
    py = X_one.dot(w)
    mse = ((y_test - py) ** 2).mean()
    return mse

linear_regression_MSE = MSE(trainx, trainy, testx, testy)
