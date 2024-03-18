import sys 
sys.path.append(".")
import itertools
import xmltodict
import json
import random

class genpara: 
    def __init__(self, row, col, alunum, mulnum, addnum, subnum): 
        self._row = row
        self._col = col
        self._alunum = alunum
        self._mulnum = mulnum
        self._addnum = addnum
        self._subnum = subnum
        self._alus = ["ALU" + str(idx) for idx in range(alunum)]
        self._muls = ["MUL" + str(idx) for idx in range(mulnum)]
        self._adds = ["ADD" + str(idx) for idx in range(addnum)]
        self._subs = ["SUB" + str(idx) for idx in range(subnum)]
        self._alusets = ["ALU" + str(idx) + ".out0" for idx in range(alunum)]
        self._mulsets = ["MUL" + str(idx) + ".out0" for idx in range(mulnum)]
        self._addsets = ["ADD" + str(idx) + ".out0" for idx in range(addnum)]
        self._subsets = ["SUB" + str(idx) + ".out0" for idx in range(subnum)]
        self._coor = []
        self._SWinAll = []
        self._SWinAllnum = []
        self._SWoutAll = []
        self._info = {
            "row": row, 
            "col": col, 
            "alunum": alunum, 
            "mulnum": mulnum, 
            "addnum": addnum, 
            "subnum": subnum, 
            "SWinAll": [], 
            "SWinAllnum": [], 
            "SWoutAll": [], 
        }
    
    def _createmesh(self): # got self._coor in the mean time
        assert self._alunum % self._col == 0
        assert self._mulnum % self._col == 0
        assert self._addnum % self._col == 0
        assert self._subnum % self._col == 0
        aluRowNum = int(self._alunum / self._col)
        mulRowNum = int(self._mulnum / self._col)
        addRowNum = int(self._addnum / self._col)
        subRowNum = int(self._subnum / self._col)
        # self._SWoutAll & self._coor
        for i in range(aluRowNum):
            self._SWoutAll.append([])
            self._coor.append([])
            for j in range(self._col):
                self._SWoutAll[i].append(self._alus[i*self._col + j] + ".in0")
                self._coor[i].append(self._alus[i*self._col + j])
        for i in range(mulRowNum):
            self._SWoutAll.append([])
            self._coor.append([])
            for j in range(self._col):
                self._SWoutAll[aluRowNum + i].append(self._muls[i*self._col + j]+ ".in0")
                self._coor[aluRowNum + i].append(self._muls[i*self._col + j])
        for i in range(addRowNum):
            self._SWoutAll.append([])
            self._coor.append([])
            for j in range(self._col):
                self._SWoutAll[aluRowNum + mulRowNum + i].append(self._adds[i*self._col + j]+ ".in0")
                self._coor[aluRowNum + mulRowNum + i].append(self._adds[i*self._col + j])
        for i in range(subRowNum):
            self._SWoutAll.append([])
            self._coor.append([])
            for j in range(self._col):
                self._SWoutAll[aluRowNum + mulRowNum + addRowNum + i].append(self._subs[i*self._col + j]+ ".in0")
                self._coor[aluRowNum + mulRowNum + addRowNum + i].append(self._subs[i*self._col + j])
        # print(self._coor)
        # self._SWinAll & self._SWinAllnum
        for i in range(self._row):
            self._SWinAll.append([])
            self._SWinAllnum.append([])
            for j in range(self._col):
                self._SWinAll[i].append([])
                self._SWinAllnum[i].append([])
        ## self._SWinAll
        for i in range(self._row):
            for j in range(self._col):
                if i == 0 and j == 0:
                    self._SWinAll[i][j].append(self._coor[i][j + 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][self._col - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i + 1][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[self._row - 1][j] + ".out0")
                elif i == 0 and j != 0 and j != self._col - 1:
                    self._SWinAll[i][j].append(self._coor[i][j + 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][j - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i + 1][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[self._row - 1][j] + ".out0")
                elif i == 0 and j == self._col - 1:
                    self._SWinAll[i][j].append(self._coor[i][0] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][j - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i + 1][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[self._row - 1][j] + ".out0")
                elif i != 0 and i != self._row - 1 and j == 0:
                    self._SWinAll[i][j].append(self._coor[i][1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][self._col - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i + 1][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i - 1][j] + ".out0")
                elif i != 0 and i != self._row - 1 and j == self._col - 1:
                    self._SWinAll[i][j].append(self._coor[i][0] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][j - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i + 1][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i - 1][j] + ".out0")
                elif i == self._row - 1 and j == 0:
                    self._SWinAll[i][j].append(self._coor[i][1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][self._col - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[0][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i - 1][j] + ".out0")
                elif i == self._row - 1 and j != 0 and j != self._col - 1:
                    self._SWinAll[i][j].append(self._coor[i][j - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][j + 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[0][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i - 1][j] + ".out0")
                elif i == self._row - 1 and j == self._col - 1:
                    self._SWinAll[i][j].append(self._coor[i][j - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][0] + ".out0")
                    self._SWinAll[i][j].append(self._coor[0][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i - 1][j] + ".out0")
                else:
                    self._SWinAll[i][j].append(self._coor[i][j - 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i][j + 1] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i - 1][j] + ".out0")
                    self._SWinAll[i][j].append(self._coor[i + 1][j] + ".out0")
        ## self._SWinAllnum
        for i in range(self._row):
            for j in range(self._col):
                for k in range(4):
                    self._SWinAllnum[i][j].append(1)
        return None

    def _returninfo(self):
        return self._SWinAll, self._SWinAllnum, self._SWoutAll
    
    def _addSingleConnect(self, fr, to): # fr, to should be ALU0/MUL2/... without port
        # got index of to
        ito = 0
        jto = 0
        for i, item in enumerate(self._coor):
            for j, jtem in enumerate(item):
                if to == jtem:
                    ito = i
                    jto = j
        # add connect
        kfr = 0
        isin = False
        for k, ktem in enumerate(self._SWinAll[ito][jto]):
            if fr in ktem:
                kfr = k
                isin = True
        if isin == True:
            if self._SWinAllnum[ito][jto][kfr] == self._col:
                print("number of this path already reach max value, path is " + fr + " to " + to)
            if self._SWinAllnum[ito][jto][kfr] < self._col:
                self._SWinAllnum[ito][jto][kfr] = self._SWinAllnum[ito][jto][kfr] + 1
                print("add one path to this path, path is " + fr + " to " + to)
            if self._SWinAllnum[ito][jto][kfr] > self._col:
                print("err: number of this path is bigger than max value, path is " + fr + " to " + to)
        if isin == False:
            self._SWinAll[ito][jto].append(fr + ".out0")
            self._SWinAllnum[ito][jto].append(1)
        print("add single connect from " + fr + " to " + to + " done.")
        return None

    def _addSingleConnectAndOneRandom(self, fr, to): # fr, to should be ALU0/MUL2/... without port
        # got index of to
        ito = 0
        jto = 0
        for i, item in enumerate(self._coor):
            for j, jtem in enumerate(item):
                if to == jtem:
                    ito = i
                    jto = j
        # add connect
        kfr = 0
        isin = False
        for k, ktem in enumerate(self._SWinAll[ito][jto]):
            if fr in ktem:
                kfr = k
                isin = True
        if isin == True:
            if self._SWinAllnum[ito][jto][kfr] == self._col:
                print("number of this path already reach max value, path is " + fr + " to " + to)
            if self._SWinAllnum[ito][jto][kfr] < self._col:
                self._SWinAllnum[ito][jto][kfr] = self._SWinAllnum[ito][jto][kfr] + 1
                print("add one path to this path, path is " + fr + " to " + to)
            if self._SWinAllnum[ito][jto][kfr] > self._col:
                print("err: number of this path is bigger than max value, path is " + fr + " to " + to)
        if isin == False:
            self._SWinAll[ito][jto].append(fr + ".out0")
            self._SWinAllnum[ito][jto].append(1)
        print("add single connect from " + fr + " to " + to + " done.")
        lenlist = len(self._SWinAll[ito][jto])
        kfr2 = random.randint(0, lenlist - 1)
        if self._SWinAllnum[ito][jto][kfr2] == self._col:
            print("(another random)number of this path already reach max value, path is " + self._SWinAll[ito][jto][kfr2] + " to " + to)
        if self._SWinAllnum[ito][jto][kfr2] < self._col:
            self._SWinAllnum[ito][jto][kfr2] = self._SWinAllnum[ito][jto][kfr2] + 1
            print("(another random)add one path to this path, path is " + self._SWinAll[ito][jto][kfr2] + " to " + to)
        if self._SWinAllnum[ito][jto][kfr2] > self._col:
            print("(another random)err: number of this path is bigger than max value, path is " + self._SWinAll[ito][jto][kfr2] + " to " + to)
        return None

if __name__ == '__main__':
    assert(len(sys.argv) >= 1)
    row = 4
    col = 4
    alunum = 8
    mulnum = 0
    addnum = 4
    subnum = 4
    para = genpara(row, col, alunum, mulnum, addnum, subnum)
    para._createmesh()
    SWinAll, SWinAllnum, SWoutAll = para._returninfo()
    print(SWinAll)
    print(SWinAllnum)
    print(SWoutAll)
    para._addSingleConnect("ADD1", "ALU5")
    SWinAll2, SWinAllnum2, SWoutAll2 = para._returninfo()
    print(SWinAll2)
    print(SWinAllnum2)
    print(SWoutAll2)
    para._addSingleConnectAndOneRandom("ADD1", "ALU6")
    SWinAll3, SWinAllnum3, SWoutAll3 = para._returninfo()
    print(SWinAll3)
    print(SWinAllnum3)
    print(SWoutAll3)
