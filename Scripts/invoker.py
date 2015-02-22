
from __future__ import print_function
import os
import sys

log = open("log.txt", "w")


s = []

r = raw_input()
N = int(r)
s.append(r)

for i in range(N):
    s.append(raw_input())

input = open("../bin/input.txt", "w")
input.write(" ".join(s))
input.close()

log.write("did input\n")
log.flush()

os.system("../bin/CollageMakerApp -input ../bin/input.txt -output ../bin/output.txt")

log.write("did run program\n")
log.flush()

output = open("../bin/output.txt")
s = output.read()
log.write(s)
for t in s.split(" "):
    print(t)
    

