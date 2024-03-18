#include "./common/Common.h"
#include "./common/Logger.h"
#include "./common/HyperGraph.h"
#include "./common/HierGraph.h"
#include "./util/NetworkAnalyzer.h"
#include "./util/GraphSort.h"
#include "./util/Utils.h"
#include "./mapping/model/FastRouter.h"
#include "./mapping/model/FastPlacer.h"
#include "./mapping/FastPlace.h"

using namespace std; 
using namespace FastCGRA; 

int main(int argc, char **argv){

    string way        = string(argv[1]);
    srand(time(nullptr));
    bool result = false;
    unordered_map<string, unordered_map<string, vector<string>>> config = Utils::readConifgFile("./arch/arch.ini");
    string RRG = config["Global"]["TopRRG"][0];
    string FUs = config["Global"]["TopFUs"][0];
    string TopRRGAnalyzed = config["Global"]["TopRRGAnalyzed"][0];
    string TopLinksAnalyzed = config["Global"]["TopLinksAnalyzed"][0];
    Graph graphRRG(RRG);
    unordered_map<string, unordered_set<string>> fu = NetworkAnalyzer::parse(FUs); 
    
    NetworkAnalyzerLegacy analyzer(fu, graphRRG);
    analyzer.dumpAnalysis(TopRRGAnalyzed, TopLinksAnalyzed);
    if(way == "place"){
        assert(argc >= 4);
        string dfg        = string(argv[2]); 
        string compat     = string(argv[3]);
        string sortMode   = "TVS";//DFS TVS
        string seed       = "";
        if(argc >= 5){
            sortMode = string(argv[4]);
        } else if(argc >= 6){
            seed = string(argv[5]);
        } else if (argc >= 7) {
            // srand((unsigned)stoi(string(argv[7])));
        } else {
            // srand(time(nullptr));
        }
        NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);
        // string rrg = "./arch/Core_RRG.txt";
        // string fus = "./arch/Core_FUs.txt";
        
        string mapped = dfg.substr(0, dfg.size() - 3) + "placed.txt";
        string routed = dfg.substr(0, dfg.size() - 3) + "routed.txt";

        result = FastPlace::Place(dfg, compat, RRG, FUs, 
                                mapped, routed, sortMode, seed);
    }
    else if(way == "chisel_place"){
        assert(argc >= 4);
        string dfg        = string(argv[2]); 
        string compat     = string(argv[3]);
        string sortMode   = "TVS";//DFS TVS
        string seed       = "";
        if(argc >= 5){
            sortMode = string(argv[4]);
        } else if(argc >= 6){
            seed = string(argv[5]);
        } else if (argc >= 7) {
            // srand((unsigned)stoi(string(argv[7])));
        } else {
            // srand(time(nullptr));
        }
        NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);
        // string rrg = "./arch/Core_RRG.txt";
        // string fus = "./arch/Core_FUs.txt";
        
        string mapped = dfg.substr(0, dfg.size() - 3) + "placed.txt";
        string routed = dfg.substr(0, dfg.size() - 3) + "routed.txt";

        result = FastPlace::Chisel_Place(dfg, compat, RRG, FUs, 
                                mapped, routed, sortMode, seed);
    }
    else if(way == "set"){
        Graph graphRRG(RRG);
        unordered_map<string, unordered_set<string>> fu = NetworkAnalyzer::parse(FUs); 
        
        NetworkAnalyzerLegacy analyzer(fu, graphRRG);
        analyzer.dumpAnalysis(TopRRGAnalyzed, TopLinksAnalyzed);
    }
    else if(way == "validate")
    {
        string dfg        = string(argv[2]); 
        string compat     = string(argv[3]);
        NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);
        // string TopRRGAnalyzed = "./arch/Top_RRG_Analyzed.txt";
        // string TopLinksAnalyzed = "./arch/Top_Links_Analyzed.txt";
        NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);

        string mapped = dfg.substr(0, dfg.size() - 3) + "placed.txt";
        string routed = dfg.substr(0, dfg.size() - 3) + "routed.txt";

        result = Utils::validate(dfg, RRG, compat, mapped, routed);
    }
    
    return !result;
}