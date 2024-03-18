
import sys
import os


def parseRRG():
    vertexLines = []
    edgeLines = []
    with open("./arch7/Top_RRG.txt", "r") as fin: 
        lines = fin.readlines()
    inputs = {"in" + str(i) for i in range(23)}
    outputs = {"out" + str(i) for i in range(1)}
    
    pass
def parseChoose(dfg, chiesl): 
    dfg_txt = dfg[:-4]+ "_DFG.txt"
    compat  = dfg[:-4]+ "_compat.txt"
    os.system('python3 ./benchmarks/convertDFG.py %s ' %(dfg))
    if chiesl == "normel":
        os.system('./build/place place %s %s ' %(dfg_txt, compat))
        # os.system('./build/place place %s %s TOPO' %(dfg_txt, compat))
    else:
        # os.system('python3 ./benchmarks/convertDFG.py %s chisel' %(dfg))
        parseRRG()
        exit(0)
        os.system('./build/place chisel_place %s %s ' %(dfg_txt, compat))


#python3 ./1_place.py ./benchmarks/express/arf/arf.dot
#python3 ./1_place.py ./benchmarks/express/arf/arf.dot aa
if __name__ == "__main__":
    dfg = sys.argv[1]
    form = "normel"
    # os.system('rm ./hardpath.txt')
    if len(sys.argv) > 2:
        form  = sys.argv[2]

    parseChoose(dfg, form)

