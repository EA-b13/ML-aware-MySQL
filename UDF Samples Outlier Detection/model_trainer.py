import numpy as np
from sklearn.linear_model import LogisticRegression

X = np.array([[10], [12], [13], [15], [14], [13], [12], [11], [13], [100]])
y = np.array([0, 0, 0, 0, 0, 0, 0, 0, 0, 1])

clf = LogisticRegression()
clf.fit(X, y)

w = clf.coef_[0][0]
b = clf.intercept_[0]

print(f"Model parameters:\nw = {w}\nb = {b}")