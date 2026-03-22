import sys
import numpy as np

in_file = sys.argv[1]
out_file = sys.argv[2]

header = ""
with open(in_file,"r") as f:
    for line in f:
        header = line
        break
header = header.strip()


data = np.loadtxt(in_file, delimiter=',',skiprows=1,dtype=np.uint64)
data.view('u8,u8').sort(order=['f0'], axis=0)
np.savetxt(out_file, data, fmt='%i', delimiter=",",header=header,comments='')