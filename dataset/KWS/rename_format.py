import glob
import os


pre = None
n = None
count = 0
count_idx = 0

for idx, i in enumerate(glob.glob(r'dataset\KWS\*\*.wav')):
    count_idx += 1
    pre = i.split('\\')[2]
    
    if not pre == n:
        count += 1
        count_idx = 0
    os.rename(i, os.path.join(r'dataset\KWS', i.split('\\')[2], '{}_{}.wav'.format(i.split('\\')[2], count_idx)))
    n = pre
