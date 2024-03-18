import sys 
import os
import time
import json
sys.path.append(".")

if __name__ == "__main__": 
    # read file
    benchmarks = []
    path = r'./config_genarch.txt'
    f = open(path)
    lines = f.readlines()
    for line in lines:
        vec = line.strip().split()
        if(vec[0] == "benchmark"):
            benchmark = []
            for i in range(1, len(vec)):
                benchmark.append(vec[i])
            benchmarks.append(benchmark)
    routed_path = []
    for item in benchmarks:
        dfgPath = item[1]
        length = len(dfgPath) - 4
        newPath = dfgPath[:length] + "_DFG.routed.txt"
        routed_path.append(newPath)
    all_paths = {}
    for item in routed_path:
        all_paths[item] = []
        with open(item, 'r') as f:
            lines = f.readlines()
            for line in lines:
                vec = line.strip().split()
                if len(vec) > 2 and "block0" in vec[0] and "block0" in vec[1]:
                    new_list = vec[2: -2]
                    all_paths[item].append(new_list)
    with open("./benchmark_routed/benchmark_routed.txt", 'w') as f:
        for key, value in all_paths.items():
            f.write(key + '\n')
            for item in value:
                content = ' '.join(map(str, item))
                f.write(content + '\n')
        