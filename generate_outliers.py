import sys
import numpy as np

in_file = sys.argv[1]
out_file = sys.argv[2]
LEN = sys.argv[3]
OUTLIER_GAP = sys.argv[4]
NUM_OUTLIERS = LEN//OUTLIER_GAP
# data = [None]*LEN
data = np.ndarray((LEN,2),dtype=np.uint64)
switched = False
idx = -1
percent = 0
with open(in_file, newline='') as csvfile:
    for row in csvfile:
        if (idx != -1):
            a,b = map(int,row.split(","))
            data[idx][0] = a
            data[idx][1] = b
        if switched:
            idx += OUTLIER_GAP
        else:
            idx+=1
            if idx%OUTLIER_GAP == OUTLIER_GAP-1:
                idx+=1
            if (idx >= LEN):
                switched = True
                idx = OUTLIER_GAP-1
            if idx % 10000000 == 0:
                print(f"{percent}% read")
                percent+=1

# f = open('outliers.csv','w+')
np.savetxt(out_file, data, fmt='%i', delimiter=",",header="time,data",comments='')
# f.write('time,data\n')
# idx = 0
# percent = 0
# for row in data:
#     idx += 1
#     f.write(row)
#     if idx % 10000000 == 0:
#         print(f"{percent}% written")
#         percent+=1

        