import sys
from collections import defaultdict
import heapq

IN_FILE = open(sys.argv[1],"r")
OUT_FILE = open(sys.argv[2],"w+")
budget = int(sys.argv[3])

OUT_FILE.write("table_name,rowgroup,col,statistic\n")

mapping = {}

options = []

for i,line in enumerate(IN_FILE):
    if i==0:continue
    table_name,rowgroup,col,statistic,statistic_alias,size,pruning_power = line.split(",")
    rowgroup,col,statistic,size = map(int,[rowgroup,col,statistic,size])
    pruning_power = float(pruning_power)
    options.append((-pruning_power,size,table_name,rowgroup,col,statistic))

IN_FILE.close()
options.sort()

for _,size,table_name,rowgroup,col,statistic in options:
    if not (table_name,rowgroup,col) in mapping and size <= budget:
        budget-=size
        mapping[(table_name,rowgroup,col)] = statistic

for table_name,rowgroup,col in mapping:
    statistic = mapping[(table_name,rowgroup,col)]
    OUT_FILE.write(f"{table_name},{rowgroup},{col},{statistic}\n")

OUT_FILE.close()