import sys
import random

f = open(sys.argv[1],"w+")
n = 10000000

f.write("time,data\n")
for i in range(n):
    f.write(f"{random.randint(1767225600000,1769817600000)},{random.randint(1,1000000)}\n")