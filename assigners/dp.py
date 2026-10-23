import sys
import numpy as np
from multiprocessing import Pool

IN_FILE = open(sys.argv[1],"r")
OUT_FILE = open(sys.argv[2],"w+")
budget = int(sys.argv[3])

OUT_FILE.write("table_name,rowgroup,col,statistic\n")

sizes = {}
pruning = {}
partitions = set()
stats = set()

for i,line in enumerate(IN_FILE):
    if i==0:continue
    table_name,rowgroup,col,statistic,statistic_alias,size,pruning_power = line.split(",")
    rowgroup,col,statistic,size = map(int,[rowgroup,col,statistic,size])
    pruning_power = float(pruning_power)
    stats.add(statistic)
    sizes[(table_name,rowgroup,col,statistic)] = size
    pruning[(table_name,rowgroup,col,statistic)] = pruning_power
    partitions.add((table_name,rowgroup,col))

M = np.zeros((len(partitions)+1,budget+1),dtype=np.int32)

gi = 0
def f(j):
    for k in stats:
        size = sizes[(table_name,rowgroup,col,k)]
        pruning_power = pruning[(table_name,rowgroup,col,statistic)]
        if j-size < 0:
            continue
        M[gi,j] = max(M[gi,j],M[gi-1,j-size]+pruning_power)

for i,(table_name,rowgroup,col) in enumerate(partitions):
    gi=i+1
    print(i,"/",len(partitions))
    with Pool(12) as p:
        p.map(f, range(budget+1))
    