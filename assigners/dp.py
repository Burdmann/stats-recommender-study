import sys
import numpy as np
from multiprocessing import Pool

IN_FILE = open(sys.argv[1],"r")
OUT_FILE = open(sys.argv[2],"w+")
budget = int(sys.argv[3])
# ASSIGNMENT=open(sys.argv[4],'r')

OUT_FILE.write("table_name,rowgroup,col,statistic\n")

# assignment = {}
# for i,line in enumerate(ASSIGNMENT):
#     if i==0:continue
#     # print(line)
#     table_name,rowgroup,col,statistic = line.split(',')
#     rowgroup,col,statistic = map(int,[rowgroup,col,statistic])
#     assignment[(table_name,rowgroup,col)] = statistic

sizes = {}
pruning = {}
partitions = set()
stats = set()
EMPTY_SIZE = 0

for i,line in enumerate(IN_FILE):
    if i==0:continue
    table_name,rowgroup,col,statistic,statistic_alias,size,pruning_power = line.split(",")
    rowgroup,col,statistic,size,pruning_power = map(int,[rowgroup,col,statistic,size,pruning_power])
    stats.add(statistic)
    if statistic == 0:
        EMPTY_SIZE = size
    # print(pruning_power)
    sizes[(table_name,rowgroup,col,statistic)] = size
    pruning[(table_name,rowgroup,col,statistic)] = pruning_power
    partitions.add((table_name,rowgroup,col))

for key in sizes:
    sizes[key] -= EMPTY_SIZE
budget -= len(partitions)*EMPTY_SIZE
partitions = list(partitions)
M = np.zeros((len(partitions)+1,budget+1),dtype=np.int32)

for i,(table_name,rowgroup,col) in enumerate(partitions):
    i+=1
    if i%100 == 0:
        print(i,"/",len(partitions))
    for k in stats:
    # for k in [assignment[(table_name,rowgroup,col)]]:
        pruning_power = pruning[(table_name,rowgroup,col,k)]
        size = sizes[(table_name,rowgroup,col,k)]
        # print(k,pruning_power,size)
        Mip = np.roll(M[i-1]+pruning_power,size)
        Mip[:size] = 0
        M[i] = np.maximum(M[i],Mip)
print(max(M[-1]))

assignment = {}
budget = np.argmax(M[-1])
pruned = max(M[-1])

for i,(table_name,rowgroup,col) in enumerate(partitions[::-1]):
    i = len(partitions)-i
    for k in stats:
        pruning_power = pruning[(table_name,rowgroup,col,k)]
        size = sizes[(table_name,rowgroup,col,k)]
        if size <= budget and M[i][budget] == M[i-1][budget-size]+pruning_power:
            budget-=size
            pruned-=pruning_power
            assignment[(table_name,rowgroup,col)] = k
            break

for table_name,rowgroup,col in assignment:
    statistic = assignment[(table_name,rowgroup,col)]
    OUT_FILE.write(f"{table_name},{rowgroup},{col},{statistic}\n")