import sys
import subprocess as sp
import time
import genFus as gf

if __name__ == '__main__':
    assert(len(sys.argv) >= 1)
    fuPruningFile = sys.argv[1] 

    fus = gf.getInitialFus("./arch6/Top_FUs.txt")
    deleteFus = gf.getDeleteFus2(fuPruningFile)
    gf.dumpFus("./arch6pruning/Top_FUs.txt", fus, deleteFus)

    # logFileName = './dse.log'
    # fo = open(logFileName, 'wb')
    # print("start")
    # res = sp.Popen(["./build/testPruning", fuPruningFile], stdout = fo, stderr = fo) 

    # while res.poll() is None:
    #     time.sleep(0.1)
    # print("generating RRG finished")

