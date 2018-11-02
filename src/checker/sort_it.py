import sys
import os

if len(sys.argv)!= 2:
    print("Usage : %s <path>" % sys.argv[0])
    exit(-1)

final = []

with open(sys.argv[1], "r") as f:
    for x in f:
        final.append(x.strip())

final = sorted(final)
for d in final:
    print(d)
