import csv

# LEN = 10000
LEN = 1000000000
# LEN = 10000000
# OUTLIER_GAP = 100
OUTLIER_GAP = 10000
# FILE = 'test.csv'
FILE = 'sorted.csv'
# FILE = 'sorted-medium.csv'
NUM_OUTLIERS = LEN//OUTLIER_GAP
data = [None]*LEN
switched = False
idx = -1
with open(FILE, newline='') as csvfile:
    for row in csvfile:
        if (idx != -1):
            data[idx] = row
        if switched:
            idx += OUTLIER_GAP
        else:
            idx+=1
            if idx%OUTLIER_GAP == OUTLIER_GAP-1:
                print(idx)
                idx+=1
            if (idx >= LEN):
                switched = True
                idx = OUTLIER_GAP-1

f = open('outliers.csv','w+')
# f = open('outliers-medium.csv','w+')
f.write('time,data\n')
idx = 0
for row in data:
    idx += 1
    if idx % 10000 == 0:
        print(idx)
    f.write(row)

        