# An End-to-End Agile Design Framework to Improve Energy Efficiency on CGRAs

## Framework
Our framework is a comprehensive reconfigurable architecture modeling framework that significantly advances the flexibility and optimization capabilities of CGRAs. It includes CGRA modeling, application mapping, architecture optimization, evaluation, and Verilog generation based on Chisel toolchains.




## How to install
### Compiler
If you already have `g++`, you can compile directly. 
```sh
mkdir build && cd build 
mkdir object
cd ..
make -j8
```

If you get an error ` g++: error: unrecognized command line option ‘-std=c++14’ `, you can update ` g++ ` or change it to ` c++11 `.

Then you can install the python packages we need. 
```python
pip3 install -r requirement.txt
```

### Dependencies for Verilog Generation
- JDK 8 or newer 

- SBT 

## How to run

### Generate Architecture
```python
python3 ./genarch/genarch21_common.py 
``` 
Note that you need to configure the `./config_genarch.txt` before running, which includes the scale of the architecture, the functionality of processing elements (PEs), and the applications that need to be mapped.

### Mapping
```python
python3 ./1_place.py dfg_dot
```
where `dfg_dot` is the dot file of application, such as `./benchmarks/express/cosine2/cosine2.dot`. 

The `./1_place.py` script will invoke the mapping subroutine `./build/place` for efficient mapping.

### Verilog Generation
You can generate the Verilog using sbt command

```
sbt "runMain  CGRA.yzArch.yazhouGen"
```

The function of `TopYZGen` is used to generate the architecture, so you can change the parameters of this function to get a new architecture.

### Architecture Pruning
First, you need to run the following script, which can automatically obtain the mapping information required for the pruning program.
```python
python3 ./pruning_route_results.py
```

After that, you can run the pruning program.
```
./build/testPruning ./pruning/FUpruning.txt
```
Note that the pruned architecture will be exported to `./arch6pruning/`. 

To test the mapping performance on the new architecture, you need to modify the configuration information in `./arch/arch.ini`, and then you can run the mapping script mentioned above.


### Physical Implemantation
We conduct logic synthesis using Synopsys Design Compiler (DC) and physical implementation using IC Compiler with the netlist produced by DC and constraints as inputs.

- 4X6 CGRA

![4X6 CGRA](./figure/4X6CGRA.png)