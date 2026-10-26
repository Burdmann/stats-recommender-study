import random
import sys
from datetime import datetime,timedelta
from collections import defaultdict

template = sys.argv[1]
count = int(sys.argv[2])
ranges = sys.argv[3:]
types,mins,maxs,ids = ranges[::4],ranges[1::4],ranges[2::4],ranges[3::4]
ids = list(map(int,ids))

templatep = []
for x in template.split("?"):
    templatep.append(x)
    templatep.append("")
templatep.pop()
template = templatep


def gen(tp,mn,mx):
    seed = random.random()
    match tp:
        case "string":
            return
        case "time":
            fmt = "%Y-%m-%d %H:%M:%S"
            start = datetime.strptime(mn,fmt)
            end = datetime.strptime(mx,fmt)
            delta = end - start
            return f"'{datetime.strftime(start + timedelta(seconds=random.uniform(0, delta.total_seconds())),fmt)}'"
        case "int":
            return int(int(mn)+seed*(int(mx)-int(mn))+0.5)
        case "float":
            return float(mn)+seed*(float(mx)-float(mn))
    return
# print(types,mins,maxs)

for _ in range(count):
    encounters = defaultdict(int)
    data = defaultdict(list)
    for i in range(len(types)):
        template[1+2*i] = (ids[i],encounters[ids[i]])
        data[ids[i]].append(str(gen(types[i],mins[i],maxs[i])))
        encounters[ids[i]] += 1
    for key in data:
        data[key].sort()
    for i in range(len(types)):
        j,k = template[1+2*i]
        template[1+2*i] = data[j][k]
    print("".join(template))