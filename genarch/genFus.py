import os

def getInitialFus(filename):
    fus = {} 
    with open(filename, 'r') as fp:
        for line in fp.readlines():
            lineSplit = line.split()
            if lineSplit[0] not in fus.keys():
                fus[lineSplit[0]] = []
                for i in range(1,len(lineSplit)):
                    fus[lineSplit[0]].append(lineSplit[i])
    
    fp.close()
    return fus

def getDeleteFus(filename):
    deleteFus = [] 
    with open(filename, 'r') as fp:
        for line in fp.readlines():
            lineSplit = line.split()
            for item in lineSplit:
                deleteFus.append(item)
    return deleteFus

def getDeleteFus2(filename):
    deleteFus = [] 
    with open(filename, 'r') as fp:
        RorD = 0
        for line in fp.readlines():
            if "Replace" in line:
                RorD = 1
                continue
            if "Delete" in line:
                RorD = 2
                continue
            if RorD == 1:
                lineSplit = line.split()
                deleteFus.append(lineSplit[1])
            if RorD == 2:
                lineSplit = line.split()
                for item in lineSplit:
                    deleteFus.append(item)
    return deleteFus

def dumpFus(filename, fus, deleteFus): 
    _del = [] 
    for item in deleteFus:
        for _fu in fus.keys():
            if "." in _fu:
                pure_fu = _fu.split('.')[1]
                if item == pure_fu:
                    _del.append(_fu)
    with open(filename, 'w') as fp:
        for _fu in fus.keys():
            if _fu in _del:
                print('deleteFu: ' + _fu + '\n')
                continue
            fp.write(_fu + " ")
            for _ in fus[_fu]:
                fp.write(_ + " ")
            fp.write('\n')
    fp.close()

    return None

if __name__ == '__main__':
    fus = getInitialFus("./data/fus.original.txt")
    deleteFus = getDeleteFus("./routes/delFu.version1.txt")
    dumpFus("./routes/pruning/fus.version1.new.txt", fus, deleteFus)
