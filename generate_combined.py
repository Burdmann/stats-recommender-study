import sys
import numpy as np
import random

random.seed(5)

start = sys.argv[1:-2]
in_files,probabilities = start[::2],[float(p) for p in start[1::2]]
out_file = sys.argv[-2]
PARTITION_SIZE = int(sys.argv[-1])

datas = [np.loadtxt(in_file, delimiter=',',skiprows=1,dtype=np.uint64) for in_file in in_files]


# all input files should have the same length
assert len(set([len(data) for data in datas])) == 1
LEN = len(datas[0])
data = np.ndarray((LEN,2),dtype=np.uint64)

NUM_PARTITIONS = (LEN+PARTITION_SIZE-1)//PARTITION_SIZE

start = 0
end=PARTITION_SIZE
for partition in range(NUM_PARTITIONS):
    draw = random.random()
    for idx in range(len(in_files)):
        if draw < probabilities[idx]:
            break
        draw -= probabilities[idx]
    data[start:end,0:2] = datas[idx][start:end,0:2]
    start+=PARTITION_SIZE
    end+=PARTITION_SIZE
    print(idx)

np.savetxt(out_file, data, fmt='%i', delimiter=",",header="time,data",comments='')