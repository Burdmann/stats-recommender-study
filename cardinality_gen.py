import sys
import random

in_file = open(sys.argv[1],"r")
out_file = open(sys.argv[2],"w+")
granularity = int(sys.argv[3])
# granularity = int(sys.argv[1])

random.seed(42)

ids = []
for i in range(granularity):
    ids.append(random.randint(0,18446744073709551615))

# for i in range(1000):
#     print(ids[random.randint(0,granularity-1)])

idx = 0
for row in in_file:
    if idx == 0:
        out_file.write(row)
    else:
        time,_ = map(int,row.split(","))
        out_file.write(f"{time},{ids[random.randint(0,granularity-1)]}\n")
    if (idx % 10000 == 0):
        print(idx)
    idx += 1