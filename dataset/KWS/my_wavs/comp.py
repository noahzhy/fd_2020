import os
import wave
import pandas
import sys

def compare_header_and_size(wav_filename):
    with wave.open(wav_filename, 'r') as fin:
        header_fsize = (fin.getnframes() * fin.getnchannels() * fin.getsampwidth()) + 44
    file_fsize = os.path.getsize(wav_filename)
    return header_fsize != file_fsize

df = pandas.read_csv(sys.argv[1])
print(df)
invalid = df.apply(lambda x: compare_header_and_size(x['file']), axis=1)
print('The following files are corrupted:')
print(df[invalid].values)