#include "../../util/NetworkAnalyzer.h"
#include "../../common/HyperGraph.h"
#include "../../pruning/inc/pruning.h"
#include "../../pruning/inc/pruAnalyse.h"

using namespace std;
using namespace FastCGRA;

int main(int argc, char** argv)
{
    string fuPruningFile = argv[1];
    string initialRRG = "./arch6/Top_RRG.txt";
    string benchmark_routed = "./benchmark_routed/benchmark_routed.txt";
    string pathPruningFile = "./benchmark_routed/pathPruning.txt";
    PRUANALYSE::PruAnalyser pruAnalyse(initialRRG, benchmark_routed);

    // unordered_map<string, unordered_map<string, vector<string>>> linkTable = pruAnalyse.getLinkTable();
    // unordered_map<string, vector<string>> pathTable = pruAnalyse.getPathTable();
    // unordered_map<string, unordered_map<string, vector<vector<string>>>> routesTable = pruAnalyse.getRoutesTable();
    // unordered_map<string, unordered_map<string, unordered_set<string>> > routesIdx = pruAnalyse.getRoutesIdx();
    // clog << "---------------------------------------------------linkTable-----------------" << endl;
    // for (const auto &a: linkTable){
    //     clog << a.first << ": " << endl;
    //     for (const auto &b: a.second){
    //         clog << "  " << b.first << ": " << endl;
    //         for (const auto &c: b.second){
    //             clog << "    " << c << endl;
    //         }
    //     }
    // }
    // clog << "-----------------------------------------------------pathTable-----------------" << endl;
    // for (const auto &a: pathTable){
    //     clog << a.first << ": " << endl;
    //     for (const auto &b: a.second){
    //         clog << "  " << b << endl;
    //     }
    // }
    // clog << "-------------------------------------------------------routesTable------------" << endl;
    // for (const auto &a: routesTable){
    //     clog << a.first << ": " << endl;
    //     for (const auto &b: a.second){
    //         clog << "  " << b.first << ": " << endl;
    //         for (const auto &c: b.second){
    //             for (const auto &d : c){
    //                 clog << "    " << d << endl;
    //             }
    //             clog << "    ------------------------" << endl;
    //         }
    //     }
    // }
    // clog << "---------------------------------------------------------routesIdx------------" << endl;
    // for (const auto &a: routesIdx){
    //     clog << a.first << ": " << endl;
    //     for (const auto &b: a.second){
    //         clog << "  " << b.first << ": " << endl;
    //         for (const auto &c: b.second){
    //             clog << "    " << c << endl;
    //         }
    //     }
    // }

    // auto tmpTopPaths = pruAnalyse.DeletePathAnnealingVersion0();
    // auto topResult = pruAnalyse.DeleteVerticesEdgesIn(tmpTopPaths);
    // pruAnalyse.dumpVerticesToDelete(pathPruningFile, "TOP", topResult);

    PRUNING::Pruning pruning(pathPruningFile, fuPruningFile);
    pruning._pruningFu();
    pruning._pruningSwitch();
    // pruning.pruningRRG();
    
    string archPath = "./arch6pruning/Top_RRG.txt";
    string coreParh = "./arch6pruning/Core_RRG.txt";
    string ariParh = "./arch6pruning/BlockARI_RRG.txt";
    pruning.dumpRRG(archPath, coreParh, ariParh);

    pruning.evaluation();

    return 0;
}