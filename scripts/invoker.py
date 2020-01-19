
import os
import sys
import argparse


log = open("log.txt", "w")
log.write("starting script\n")

parser = argparse.ArgumentParser()
parser.add_argument("--mode", nargs=1, type=str, default='release', help="running mode: degug, release, profile. default release")

args = parser.parse_args()

s = []

r = input()
N = int(r)
s.append(r)

for i in range(N):
    s.append(input())

os.system("mkdir ../temp/")
input = open("../temp/input.txt", "w")
input.write(" ".join(s))
input.close()

log.write("did input\n")
log.flush()

command = "../cmake-build-%s/CollageMakerApp -input ../temp/input.txt -output ../temp/output.txt" % args['mode']
print("run command: %s" % command)
os.system(command)

log.write("did run program\n")
log.flush()

output = open("../bin/output.txt")
s = output.read()
log.write(s)
for t in s.split(" "):
    print(t)
    

