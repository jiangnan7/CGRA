import sys 
import os
import subprocess as sp
import time
import json
sys.path.append(".")

from audioop import mul
import json
from sqlite3 import connect
from typing import Tuple
from genarch.utils.genarch import ModuleGen
from genarch.utils.arch import Arch
from genarch.utils.protocols import Function
from genarch.utils.protocols import Unit
from genarch.utils.protocols import Module
from genarch.utils.genpara import genpara


if __name__ == "__main__": 
    # read file
    rownum = 0
    colnum = 0
    alunum = 0
    mulnum = 0
    addnum = 0
    subnum = 0
    benchmarks = []
    path = r'./config_genarch.txt'
    f = open(path)
    lines = f.readlines()
    for line in lines:
        vec = line.strip().split()
        if(vec[0] == "rownum"):
            rownum = int(vec[1])
        if(vec[0] == "colnum"):
            colnum = int(vec[1])
        if(vec[0] == "alunum"):
            alunum = int(vec[1])
        if(vec[0] == "mulnum"):
            mulnum = int(vec[1])
        if(vec[0] == "addnum"):
            addnum = int(vec[1])
        if(vec[0] == "subnum"):
            subnum = int(vec[1])
        if(vec[0] == "benchmark"):
            benchmark = []
            for i in range(1, len(vec)):
                benchmark.append(vec[i])
            benchmarks.append(benchmark)
    benchmarkCount = 0

    # write coordination information
    with open(f"./arch6/RRG_coor.txt", "w") as fout: 
        alus = ["block0.ALU" + str(idx) for idx in range(alunum)]
        muls = ["block0.MUL" + str(idx) for idx in range(mulnum)]
        adds = ["block0.ADD" + str(idx) for idx in range(addnum)]
        subs = ["block0.SUB" + str(idx) for idx in range(subnum)]
        alusetnum = alunum // colnum
        mulsetnum = mulnum // colnum
        addsetnum = addnum // colnum
        subsetnum = subnum // colnum
        x_coor = -1
        for i, item in enumerate(alus):
            if i % colnum == 0:
                x_coor = x_coor + 1
                fout.write(str(item) + " " + str(x_coor) + " " + "0" + "\n")
            else:
                fout.write(str(item) + " " + str(x_coor) + " " + str(i % colnum) + "\n")
        for i, item in enumerate(muls):
            if i % colnum == 0:
                x_coor = x_coor + 1
                fout.write(str(item) + " " + str(x_coor) + " " + "0" + "\n")
            else:
                fout.write(str(item) + " " + str(x_coor) + " " + str(i % colnum) + "\n")
        for i, item in enumerate(adds):
            if i % colnum == 0:
                x_coor = x_coor + 1
                fout.write(str(item) + " " + str(x_coor) + " " + "0" + "\n")
            else:
                fout.write(str(item) + " " + str(x_coor) + " " + str(i % colnum) + "\n")
        for i, item in enumerate(subs):
            if i % colnum == 0:
                x_coor = x_coor + 1
                fout.write(str(item) + " " + str(x_coor) + " " + "0" + "\n")
            else:
                fout.write(str(item) + " " + str(x_coor) + " " + str(i % colnum) + "\n")

    import genarch.utils.units as units
    para = genpara(rownum, colnum, alunum, mulnum, addnum, subnum)
    para._createmesh()
    SWinAll, SWinAllnum, SWoutAll = para._returninfo()

    #### write original mesh para to file
    # get coordination
    coor = {}
    rowcount = 0
    for i in range(alunum):
        coor["ALU" + str(i)] = [rowcount, i%colnum]
        if(i%colnum == colnum - 1):
            rowcount += 1
    for i in range(mulnum):
        coor["MUL" + str(i)] = [rowcount, i%colnum]
        if(i%colnum == colnum - 1):
            rowcount += 1
    for i in range(addnum):
        coor["ADD" + str(i)] = [rowcount, i%colnum]
        if(i%colnum == colnum - 1):
            rowcount += 1
    for i in range(subnum):
        coor["SUB" + str(i)] = [rowcount, i%colnum]
        if(i%colnum == colnum - 1):
            rowcount += 1
    assert rowcount == rownum
    # get matrixIn and map
    matrixIn2 = {}
    matrixInMap2 = {}
    for i in range(rownum):
        matrixIn2[i] = []
        matrixInMap2[i] = {} # matrixInMap[i][unit] = j, it represents the j-th inport of i-th row matrix
    for i, SWin in enumerate(SWinAll):
        incount = 0
        for item in SWin:
            for item2 in item:
                unit = item2.split('.')[0]
                co = coor[unit]
                if unit not in matrixInMap2[i]:
                    matrixIn2[i].append((co[0], co[1]))
                    matrixInMap2[i][unit] = incount
                    incount += 1
    # get matrixInner
    matrixInner2 = {}
    for i in range(rownum):
        matrixInner2[i] = {}
        for j in range(colnum):
            matrixInner2[i][j] = []
    for i, SWin in enumerate(SWinAll):
        for j, item in enumerate(SWin):
            for k, item2 in enumerate(item):
                temp = []
                unit = item2.split('.')[0]
                temp.append(matrixInMap2[i][unit])
                temp.append(SWinAllnum[i][j][k])
                matrixInner2[i][j].append(temp)
    ### write to the file
    ### txt
    with open("./para_hardware/matrixIn_ori.txt", "w") as f:
        # f.write("matrixIn: \n")
        f.write("List(\n")
        for key, value in matrixIn2.items():
            if key != len(matrixIn2) - 1:
                f.write("\tList(\n")
                for i in range(len(value)):
                    if i != len(value) - 1:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + "), \n")
                    else:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + ") \n")
                f.write("\t), \n")
            else:
                f.write("\tList(\n")
                for i in range(len(value)):
                    if i != len(value) - 1:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + "), \n")
                    else:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + ") \n")
                f.write("\t)\n")
        f.write("),\n")
    with open("./para_hardware/matrixInner_ori.txt", "w") as f:
        # f.write("matrixInner: \n")
        f.write("List(\n")
        for key, value in matrixInner2.items():
            if key != len(matrixInner2) - 1:
                f.write("\tscala.collection.mutable.Map(\n")
                for key2, value2 in value.items():
                    if key2 != len(value) - 1:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write("), \n")
                    else:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write(") \n")
                f.write("\t), \n")
            else:
                f.write("\tscala.collection.mutable.Map(\n")
                for key2, value2 in value.items():
                    if key2 != len(value) - 1:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write("), \n")
                    else:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write(") \n")
                f.write("\t) \n")
        f.write("), \n")


    succNum = 0
    while benchmarkCount < len(benchmarks):
    # while os.path.getsize('./hardpath.txt') != 0:
        f = open('./hardpath.txt', 'r')
        for line in f.readlines():
            line = line.strip()
            frto = line.split()
            fr = frto[0].split('.')[1]
            to = frto[1].split('.')[1]
            para._addSingleConnect(fr, to)
        f.close()
        SWinAll, SWinAllnum, SWoutAll = para._returninfo()

        print("SWinAll = ")
        print(SWinAll)
        print("SWinAllnum = ")
        print(SWinAllnum)
        print("SWoutAll = ")
        print(SWoutAll)

        infoFuncs = units.functions()
        infoUnits = units.units()
        infoSwitches = {}
        infoModules = {}
        funcStructs = {}
        unitStructs = {}
        moduleStructs = {}
        for name, info in infoFuncs.items(): 
            funcStructs[name] = Function(name, info)
        for name, info in infoUnits.items(): 
            unitStructs[name] = Unit(name, info)

        blockARI = ModuleGen(["in" + str(idx) for idx in range(32)], 
                            ["out" + str(idx) for idx in range(32)], 
                            funcStructs, unitStructs, [])
        alus = ["ALU" + str(idx) for idx in range(alunum)]
        muls = ["MUL" + str(idx) for idx in range(mulnum)]
        adds = ["ADD" + str(idx) for idx in range(addnum)]
        subs = ["SUB" + str(idx) for idx in range(subnum)]
        
        alusets = ["ALU" + str(idx) + ".out0" for idx in range(alunum)]
        mulsets = ["MUL" + str(idx) + ".out0" for idx in range(mulnum)]
        addsets = ["ADD" + str(idx) + ".out0" for idx in range(addnum)]
        subsets = ["SUB" + str(idx) + ".out0" for idx in range(subnum)]
        blockARI.addUnits(alus, ["ALU" for _ in range(len(alus))])
        blockARI.addUnits(muls, ["MUL" for _ in range(len(muls))])
        blockARI.addUnits(adds, ["ADD" for _ in range(len(adds))])
        blockARI.addUnits(subs, ["SUB" for _ in range(len(subs))])
        blockARI.addGroupAny("AAA", alus + muls + adds + subs, row=rownum, col=colnum, SWinAll = SWinAll, SWinAllnum = SWinAllnum, SWoutAll = SWoutAll)
        blockARI.connectInportsAll(alus + muls + adds + subs)
        blockARI.connectOutportsAll(alus + muls + adds + subs)
        blockARI.dealWithFanin()

        for key, value in blockARI._switches.items(): 
            infoSwitches[key] = value
        infoModules["BlockARI"] = blockARI._info
        for name, info in infoModules.items(): 
            moduleStructs[name] = Module(name, info)

        top = ModuleGen([], [], 
                        funcStructs, unitStructs, moduleStructs)
        inputs = ["IN" + str(idx) for idx in range(32)]
        top.addUnits(inputs, ["INPUT" for _ in range(len(inputs))])
        outputs = ["OUT" + str(idx) for idx in range(32)]
        top.addUnits(outputs, ["OUTPUT" for _ in range(len(outputs))])
        cores = ["block0"]
        types = ["BlockARI"]
        top.addModules(cores, types)
        for idx in range(32):
            for core in cores:
                top.connect("IN" + str(idx) + ".out0",core + ".in" + str(idx))
                # top.connect("IN" + str(idx+16) + ".out0",core + ".in" + str(idx))
                # top.connect("IN" + str(idx+32) + ".out0",core + ".in" + str(idx))
                # top.connect("IN" + str(idx+48) + ".out0",core + ".in" + str(idx))
        for idx in range(32):
            for core in cores:
                top.connect(core + ".out" + str(idx), "OUT" + str(idx) + ".in0")
                # top.connect(core + ".out" + str(idx), "OUT" + str(idx*2+1) + ".in0")
        top.dealWithFanin()

        for key, value in top._switches.items(): 
            infoSwitches[key] = value
        infoModules["Top"] = top._info

        names = list(infoModules.keys())
        for name in names: 
            top = {
                "Function": infoFuncs, 
                "Unit": infoUnits, 
                "Switch": infoSwitches, 
                "Module": infoModules
            }
            top["Module"]["TOP"] = infoModules[name]
            with open(f"./arch6/{name}.json", "w") as fout: 
                fout.write(json.dumps(top, indent=4))
            arch = Arch(f"./arch6/{name}.json")
            with open(f"./arch6/{name}_RRG.txt", "w") as fout: 
                fout.write(arch.rrg())
            with open(f"./arch6/{name}_FUs.txt", "w") as fout: 
                fout.write(arch.fus())

        # test benchmarks
        for i in range(len(benchmarks)):
            if i == benchmarkCount:
                dotfile = benchmarks[i][1]
                sp.run(["python3", "./1_place.py", dotfile])
                print("one place iteration completed.")

                if os.path.getsize('./hardpath.txt') == 0:
                    succNum += 1
                    if succNum < 300:
                        pass
                    else:
                        succNum = 0
                        print("benchmark" + str(i) + " has mapped successfully.")
                        benchmarkCount += 1

    #### write para to file
    print("SWinAll = ")
    print(SWinAll)
    print("SWinAllnum = ")
    print(SWinAllnum)
    print("SWoutAll = ")
    print(SWoutAll)
    #### write para to file
    # # get coordination
    # coor = {}
    # rowcount = 0
    # for i in range(alunum):
    #     coor["ALU" + str(i)] = [rowcount, i%colnum]
    #     if(i%colnum == colnum - 1):
    #         rowcount += 1
    # for i in range(mulnum):
    #     coor["MUL" + str(i)] = [rowcount, i%colnum]
    #     if(i%colnum == colnum - 1):
    #         rowcount += 1
    # for i in range(addnum):
    #     coor["ADD" + str(i)] = [rowcount, i%colnum]
    #     if(i%colnum == colnum - 1):
    #         rowcount += 1
    # for i in range(subnum):
    #     coor["SUB" + str(i)] = [rowcount, i%colnum]
    #     if(i%colnum == colnum - 1):
    #         rowcount += 1
    # assert rowcount == rownum
    # get matrixIn and map
    matrixIn = {}
    matrixInMap = {}
    for i in range(rownum):
        matrixIn[i] = []
        matrixInMap[i] = {} # matrixInMap[i][unit] = j, it represents the j-th inport of i-th row matrix
    for i, SWin in enumerate(SWinAll):
        incount = 0
        for item in SWin:
            for item2 in item:
                unit = item2.split('.')[0]
                co = coor[unit]
                if unit not in matrixInMap[i]:
                    matrixIn[i].append((co[0], co[1]))
                    matrixInMap[i][unit] = incount
                    incount += 1
    # get matrixInner
    matrixInner = {}
    for i in range(rownum):
        matrixInner[i] = {}
        for j in range(colnum):
            matrixInner[i][j] = []
    for i, SWin in enumerate(SWinAll):
        for j, item in enumerate(SWin):
            for k, item2 in enumerate(item):
                temp = []
                unit = item2.split('.')[0]
                temp.append(matrixInMap[i][unit])
                temp.append(SWinAllnum[i][j][k])
                matrixInner[i][j].append(temp)
    ### write to the file
    ### json
    json_str1 = json.dumps(matrixIn, indent=4)
    with open('multi_matrixIn.json', 'w') as json_file1:
        json_file1.write(json_str1)
    json_str2 = json.dumps(matrixInner, indent=4)
    with open('multi_matrixInner.json', 'w') as json_file2:
        json_file2.write(json_str2)
    ### txt
    with open("./para_hardware/matrixIn.txt", "w") as f:
        # f.write("matrixIn: \n")
        f.write("List(\n")
        for key, value in matrixIn.items():
            if key != len(matrixIn) - 1:
                f.write("\tList(\n")
                for i in range(len(value)):
                    if i != len(value) - 1:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + "), \n")
                    else:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + ") \n")
                f.write("\t), \n")
            else:
                f.write("\tList(\n")
                for i in range(len(value)):
                    if i != len(value) - 1:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + "), \n")
                    else:
                        f.write("\t\t(" + str(value[i][0]) + ', ' + str(value[i][1]) + ") \n")
                f.write("\t)\n")
        f.write("),\n")
    with open("./para_hardware/matrixInner.txt", "w") as f:
        # f.write("matrixInner: \n")
        f.write("List(\n")
        for key, value in matrixInner.items():
            if key != len(matrixInner) - 1:
                f.write("\tscala.collection.mutable.Map(\n")
                for key2, value2 in value.items():
                    if key2 != len(value) - 1:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write("), \n")
                    else:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write(") \n")
                f.write("\t), \n")
            else:
                f.write("\tscala.collection.mutable.Map(\n")
                for key2, value2 in value.items():
                    if key2 != len(value) - 1:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write("), \n")
                    else:
                        f.write("\t\tList(8, " + str(key2) + ") ->List(")
                        for i in range(len(value2)):
                            if i != len(value2) - 1:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + "), ")
                            else:
                                f.write("List(8, " + str(value2[i][0]) + ", " + str(value2[i][1]) + ")")
                        f.write(") \n")
                f.write("\t) \n")
        f.write("), \n")

    #### In & Out
    routed_path = []
    for item in benchmarks:
        dfgPath = item[1]
        length = len(dfgPath) - 4
        newPath = dfgPath[:length] + "_DFG.routed.txt"
        routed_path.append(newPath)
    ### IN
    ALLinports = []
    for item in coor:
        pos1 = (coor[item][0], coor[item][1], 0)
        pos2 = (coor[item][0], coor[item][1], 1)
        ALLinports.append(pos1)
        ALLinports.append(pos2)
    # for item in routed_path:
    #     with open(item, 'r') as f:
    #         lines = f.readlines()
    #         for line in lines:
    #             vec = line.strip().split()
    #             if "IN" in vec[0] and "out0" in vec[0]:
    #                 inport = vec[1].split(".")
    #                 unit = inport[1]
    #                 portnum = int(inport[2][-1])
    #                 pos = (coor[unit][0], coor[unit][1], portnum)
    #                 isin = False
    #                 for t in ALLinports:
    #                     if t == pos:
    #                         isin = True
    #                         break
    #                 if not isin:
    #                     ALLinports.append(pos)
    ## to file
    with open("./para_hardware/inList.txt", "w") as f:
        f.write("List(\n")
        for i, item in enumerate(ALLinports):
            if i != len(ALLinports) - 1:
                f.write("\t(" + str(item[0]) + ", " + str(item[1]) + ", " + str(item[2]) + "), \n")
            else:
                f.write("\t(" + str(item[0]) + ", " + str(item[1]) + ", " + str(item[2]) + ") \n")
        f.write("),")
    ### OUT
    ALLoutports = []
    # for item in routed_path:
    #     with open(item, 'r') as f:
    #         lines = f.readlines()
    #         for line in lines:
    #             vec = line.strip().split()
    #             if "OUT" in vec[1] and "in0" in vec[1]:
    #                 outport = vec[0].split(".")
    #                 unit = outport[1]
    #                 pos = (coor[unit][0], coor[unit][1])
    #                 isin = False
    #                 for t in ALLoutports:
    #                     if t == pos:
    #                         isin = True
    #                         break
    #                 if not isin:
    #                     ALLoutports.append(pos)
    for item in coor:
        pos = (coor[item][0], coor[item][1])
        ALLoutports.append(pos)
    ## to file
    with open("./para_hardware/outList.txt", "w") as f:
        f.write("List(\n")
        for i, item in enumerate(ALLoutports):
            if i != len(ALLoutports) - 1:
                f.write("\t(" + str(item[0]) + ", " + str(item[1]) + "), \n")
            else:
                f.write("\t(" + str(item[0]) + ", " + str(item[1]) + ") \n")
        f.write("),")

