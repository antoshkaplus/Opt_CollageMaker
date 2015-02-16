
from __future__ import print_function
import os

s = []

r = raw_input()
N = int(r)
s.append(r)

for i in range(N):
    s.append(raw_input())

input = open("../bin/input.txt", "w")
input.write(" ".join(s))
input.close()

os.system("../bin/CollageMakerApp")

output = open("../bin/output.txt")
s = output.read()
for t in s.split(" "):
    print(t)

    

