import sys
import random

f = open(sys.argv[1],"w+")
n = int(sys.argv[2])
idx = 0
percent = 0
f.write("time,data\n")
for i in range(n):
    f.write(f"{random.randint(1767225600000,1769817600000)},{random.randint(1,1000000)}\n")
    if idx % 10000000 == 0:
        print(f"{percent}% written")
        percent+=1
    idx += 1