import sys
import os

if len(sys.argv)!= 3:
    print("Usage : %s <R_path> <S_path>" % sys.argv[0])
    exit(-1)

R_dict = {}
with open(sys.argv[1], "r") as f:
    for line in f:
        row = line.strip().split()
        if row[1] not in R_dict:
            R_dict[row[1]] = []
        R_dict[row[1]].append(row)


S_dict = {}
with open(sys.argv[2], "r") as f:
    for line in f:
        row = line.strip().split()
        if row[0] not in S_dict:
            S_dict[row[0]] = []
        S_dict[row[0]].append(row)

final = []
for rl, sl in zip(R_dict.keys(), S_dict.keys()):
    rl = R_dict[rl]
    sl = S_dict[sl]

    for r in rl:
        for s in sl:
            final.append(" ".join(map(str,r[:1] + s)))

final = sorted(final)

for d in final:
    print(d)
