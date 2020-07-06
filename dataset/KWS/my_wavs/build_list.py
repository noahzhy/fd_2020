import glob
import os
import random


_list = glob.glob('*help/*.wav')
length = len(_list)
validation_list = random.sample(_list, int(length*.8))
testing_list = list(set(_list) - set(validation_list))

with open('validation_list.txt', 'w') as f:
    for i in validation_list:
        f.write(i+'\n')
    
    f.close()


with open('testing_list.txt', 'w') as f:
    for i in testing_list:
        f.write(i+'\n')
    
    f.close()
