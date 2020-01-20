
import os
import sys
import argparse


log = open("log.txt", "w")
log.write("starting script\n")

try:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", nargs=1, type=str, default=['release'], help="running mode: degug, release, profile. default release")

    args = parser.parse_args()
    MODE = args.mode[0]

    s = []

    r = input()
    N = int(r)
    s.append(r)

    for i in range(N):
        s.append(input())

    os.system("mkdir -p ../temp/")
    input = open("../temp/input.txt", "w")
    input.write(" ".join(map(str, s)))
    input.close()

    log.write("did input\n")
    log.write("hello")
    log.write(repr(args))
    log.flush()

    if MODE == 'valgrind':
        command = "valgrind --tool=callgrind ../cmake-build-release/CollageMakerApp -input ../temp/input.txt -output ../temp/output.txt"
        log.write("run command: %s" % command)
        os.system(command)
    elif MODE == 'gperf':
        #os.system()
        command = "export CPUPROFILE=../temp/perf.out; ../cmake-build-%s/CollageMakerApp -input ../temp/input.txt -output ../temp/output.txt" % MODE
        log.write("run command: %s" % command)
        os.system(command)
    elif MODE == 'perf':
        command = "perf record -g ../cmake-build-perf/CollageMakerApp -input ../temp/input.txt -output ../temp/output.txt > /dev/null"
        log.write("run command: %s" % command)
        log.flush()
        os.system(command)
    else:
        command = "../cmake-build-%s/CollageMakerApp -input ../temp/input.txt -output ../temp/output.txt" % MODE
        log.write("run command: %s" % command)
        os.system(command)

    log.write("did run program\n")
    log.flush()

    output = open("../temp/output.txt")
    s = output.read()
    log.write(s)
    for t in s.split(" "):
        print(t)
except Exception as ex:
    log.write(repr(ex))
    log.flush()


