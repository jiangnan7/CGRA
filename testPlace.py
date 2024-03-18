import os
import random
import sys 
import subprocess as sp
import datetime as dt
import time

def pack(subdfg, dfg, compat):
    print(dfg, ": Pack start. ")
    timestart = dt.datetime.now()
    testlog = dfg + ".pack.log"
    fo = open(testlog, 'wb')

    proPack = sp.Popen(['./build/pack', subdfg, dfg, compat], stdout=fo, stderr= sp.STDOUT)
    finished = False
    numQuited = 0
    while not finished and numQuited < 1:
        time.sleep(random.random() * 10)
        if not proPack.poll() is None:
            numQuited += 1
            if proPack.poll() == 0:
                finished = True
                timend = timestart = dt.datetime.now() - timestart
                print(dfg, ": Pack finished. Time:", str(timend.seconds + timend.microseconds / 1000000), 's')
                break
    fo.flush()
    fo.close()
    if not finished:
        print(dfg, ": Pack failed. ")
        return "failed"
    return 


def partition(dfg, compat, corenum = 1):
    print(dfg, ": Partition start. ")
    timestart = dt.datetime.now()
    testlog = dfg + ".partition.log"
    fo = open(testlog, 'wb')

    proPack = sp.Popen(['./build/partition', dfg, compat, str(corenum)], stdout=fo, stderr= sp.STDOUT)
    finished = False
    numQuited = 0
    while not finished and numQuited < 1:
        time.sleep(random.random() * 1)
        if not proPack.poll() is None:
            numQuited += 1
            if proPack.poll() == 0:
                finished = True
                timend = timestart = dt.datetime.now() - timestart
                print(dfg, ": Partition finished. Time:", str(timend.seconds + timend.microseconds / 1000000), 's')
                break
    fo.flush()
    fo.close()
    if not finished:
        print(dfg, ": Partition failed. ")
        sys.exit(1)


def placeCore(dfg, compat, partnum):
    corepart = []
    for _ in range(partnum):
        corepart.append(dfg[: len(dfg) -4] + "_part" + str(_) + ".txt")
    print(dfg, ": PlaceCore start. ")
    timestart = dt.datetime.now()
    testlog = dfg + ".placecore.log"
    fo = open(testlog, 'wb')
    parallel = False
    if parallel == True:
        ()
    else:
        for part in corepart:
            proPlace = sp.Popen(['./build/place','placeCore', part, dfg, compat], stdout=fo, stderr= sp.STDOUT)
            print(part, ": PlaceCore begin. ")
            finished = False
            numQuited = 0
            while not finished and numQuited < 1:
                time.sleep(random.random() * 1)
                if not proPlace.poll() is None:
                    numQuited += 1
                    if proPlace.poll() == 0:
                        finished = True
                        print(part, ": PlaceCore finished. ")
                        break
            if not finished:
                print(part, ": PlaceCore failed. ")
                sys.exit(1)
    timend = timestart = dt.datetime.now() - timestart
    print(dfg, ": PlaceCore finished. Time:", str(timend.seconds + timend.microseconds / 1000000), 's')
    fo.flush()
    fo.close()
    
def placeTop(dfg, compat):
    print(dfg, ": PlaceTop start. ")
    timestart = dt.datetime.now()
    testlog = dfg + ".placetop.log"
    fo = open(testlog, 'wb')

    proPlace = sp.Popen(['./build/place', 'placeTop', dfg, compat], stdout=fo, stderr= sp.STDOUT)
    finished = False
    numQuited = 0
    while not finished and numQuited < 1:
        time.sleep(random.random() * 1)
        if not proPlace.poll() is None:
            numQuited += 1
            if proPlace.poll() == 0:
                finished = True
                timend = timestart = dt.datetime.now() - timestart
                print(dfg, ": PlaceTop finished. Time:", str(timend.seconds + timend.microseconds / 1000000), 's')
                break
    fo.flush()
    fo.close()
    if not finished:
        print(dfg, ": PlaceTop failed. ")
        sys.exit(1)

def validate(dfg, compat):
    print(dfg, ": Validate start. ")
    timestart = dt.datetime.now()
    testlog = dfg + ".place.validate.log"
    fo = open(testlog, 'wb')

    proPlace = sp.Popen(['./build/place', 'validate', dfg, compat], stdout=fo, stderr= sp.STDOUT)
    finished = False
    numQuited = 0
    while not finished and numQuited < 1:
        time.sleep(random.random() * 1)
        if not proPlace.poll() is None:
            numQuited += 1
            if proPlace.poll() == 0:
                finished = True
                timend = timestart = dt.datetime.now() - timestart
                print(dfg, ": Validate finished. Time:", str(timend.seconds + timend.microseconds / 1000000), 's')
                break
    fo.flush()
    fo.close()
    if not finished:
        print(dfg, ": Validate failed. ")
        sys.exit(1)


if __name__ == "__main__":
    dfg = sys.argv[1]
    compat = sys.argv[2]

    timestart = dt.datetime.now()
    fail2excute = 3
    for _ in range(fail2excute):
        if pack(dfg, dfg, compat) != "failed":
            break
        if _ >= 1:
            print(dfg, ": The number of repack is ", _)

    partition(dfg, compat, corenum = 4)
    placeCore(dfg, compat, partnum = 4)
    placeTop(dfg, compat)
    validate(dfg, compat)
    timend = timestart = dt.datetime.now() - timestart
    print(dfg, ": ALL finished. Time:", str(timend.seconds + timend.microseconds / 1000000), 's')
