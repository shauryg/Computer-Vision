import numpy as np
import matplotlib.pyplot as plt
plt.switch_backend('agg')
from sklearn.metrics import roc_curve, auc
from sklearn.metrics import roc_auc_score

tp = []
fp = list()
threshhold = list()

f = open("ROC.txt",'r')
data = f.read()
x = data.split('\n')
for trio in x:
  if trio:
    y = trio.split(' ')
    if y:
      threshhold.append(y[0])
      tp.append(y[1])
      fp.append(y[2])
  
lw = 2
plt.plot(fp, tp, color='darkorange', lw=lw, label='ROC curve')
plt.plot([0, 1], [0, 1], color='navy', lw=lw, linestyle='--')
plt.xlim([0.0, 1.0])
plt.ylim([0.0, 1.05])
plt.xlabel('False Positive Rate')
plt.ylabel('True Positive Rate')
plt.title('Receiver operating characteristic')
plt.legend(loc="lower right")
plt.savefig('ROC_Graph.png')
plt.close()
