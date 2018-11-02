import sys
import numpy as np
import random

KEY_MAX = 500
NONKEY_MAX = 100000

if len(sys.argv) != 4:
    print("Usage : %s <output_path> <num_row> <R/S>" % (sys.argv[0]))
    exit(-1)

out_path = sys.argv[1]
num_row = int(sys.argv[2])
relname = sys.argv[3]

if relname not in ["R", "S"]:
    print("Usage : %s <output_path> <num_row> <R/S>" % (sys.argv[0]))
    exit(-1)


with open(out_path, "w") as f:
    for i in range(num_row):
        key = np.random.randint(KEY_MAX)
        nonkey = np.random.randint(NONKEY_MAX)

        if relname == "R":
            f.write("%d %d\n" % (nonkey, key))
        else:
            f.write("%d %d\n" % (key, nonkey))
