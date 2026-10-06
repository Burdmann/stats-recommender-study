import sys
from collections import defaultdict
import heapq

IN_FILE = open(sys.argv[1],"r")
OUT_FILE = open(sys.argv[2],"w+")
budget = int(sys.argv[3])

OUT_FILE.write("table_name,rowgroup,col,statistic\n")

mapping = {}
current_pruning_power = defaultdict(float)
current_size = defaultdict(int)

pq = []

for i,line in enumerate(IN_FILE):
    if i==0:continue
    table_name,rowgroup,col,statistic,statistic_alias,size,pruning_power = line.split(",")
    rowgroup,col,statistic,size = map(int,[rowgroup,col,statistic,size])
    pruning_power = float(pruning_power)
    pq.append((-pruning_power/size,pruning_power,table_name,rowgroup,col,statistic,size))

IN_FILE.close()

heapq.heapify(pq)

while pq:
    val,pruning_power,table_name,rowgroup,col,statistic,size = heapq.heappop(pq)
    if size > budget or pruning_power < current_pruning_power[(table_name,rowgroup,col)]:
        continue
    new_val = -(pruning_power-current_pruning_power[(table_name,rowgroup,col)])/(size-current_size[(table_name,rowgroup,col)])
    if val != new_val:
        heapq.heappush(pq,(new_val,pruning_power,table_name,rowgroup,col,statistic,size))
        continue
    budget -= size-current_size[(table_name,rowgroup,col)]
    mapping[(table_name,rowgroup,col)] = statistic
    current_pruning_power[(table_name,rowgroup,col)] = pruning_power
    current_size[(table_name,rowgroup,col)] = size

for table_name,rowgroup,col in mapping:
    statistic = mapping[(table_name,rowgroup,col)]
    OUT_FILE.write(f"{table_name},{rowgroup},{col},{statistic}\n")

OUT_FILE.close()