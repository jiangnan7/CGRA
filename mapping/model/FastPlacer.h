#ifndef __FASTPLACER__
#define __FASTPLACER__

#include "./common/Common.h"
#include "./common/HyperGraph.h"
#include "./util/NOrderValidator.h"
#include "./util/NetworkAnalyzer.h"
#include "./util/GraphSort.h"
#include "./FastRouter.h"
#include "util/FastMatcher.h"
#include "./util/Utils.h"
namespace FastCGRA
{

class FastPlacer
{
private:
    Graph _RRG;
    Graph _RRGAnalyzed;
    std::unordered_map<std::string, std::unordered_set<std::string>> _FUs; 

    std::unordered_map<std::string, size_t> _failedVertices; 
public:
    FastPlacer() = delete;
    FastPlacer(Graph &rrg, std::unordered_map<std::string, std::unordered_set<std::string>> &FUs): _RRG(rrg), _FUs(FUs){ NetworkAnalyzerLegacy analyzer(_FUs, _RRG); _RRGAnalyzed = analyzer.RRG(); }
    FastPlacer(Graph &rrg, const NetworkAnalyzerLegacy &analyzer): _RRG(rrg), _RRGAnalyzed(analyzer.RRG()), _FUs(analyzer.fus()){}
    FastPlacer(Graph &rrg, std::unordered_map<std::string, std::unordered_set<std::string>> &FUs, Graph &rrgContracted): _RRG(rrg), _RRGAnalyzed(rrgContracted), _FUs(FUs){}
    FastPlacer(const FastPlacer &placer): _RRG(placer._RRG), _RRGAnalyzed(placer._RRGAnalyzed), _FUs(placer._FUs), _failedVertices(placer._failedVertices) {}
    FastPlacer(FastPlacer &&placer): _RRG(placer._RRG), _RRGAnalyzed(placer._RRGAnalyzed), _FUs(placer._FUs), _failedVertices(placer._failedVertices) {}

    const std::unordered_map<std::string, size_t> &failedVertices() const {return _failedVertices; } 

    std::pair<std::unordered_map<std::string, std::string>, FastRouter> place(const Graph &DFG, const std::unordered_map<std::string, 
                                                    std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                    const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial, const NetworkAnalyzerLegacy &analyzerInitial, const NOrderValidator &validatorInitial,
                                                 const std::vector<std::string> &order); 

    std::pair<std::unordered_map<std::string, std::string>, FastRouter> chisel_place(const Graph &DFG, const std::unordered_map<std::string, 
                                                    std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                    const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial, const NetworkAnalyzerLegacy &analyzerInitial, const NOrderValidator &validatorInitial,
                                                 const std::vector<std::string> &order); 

    std::pair<std::unordered_map<std::string, std::string>, FastRouter> placeGivenCompat(const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat); 

    //annealing
    std::pair<std::unordered_map<std::string, std::string>, FastRouter> place_annealing(const Graph &graphDFG, const Graph &globalDFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compatible, 
                                                        const FastRouter &routerInit, const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial,  
                                                        const NetworkAnalyzerLegacy &analyzerInitial, const NOrderValidator &validatorInitial, const std::vector<std::string> &order);
};

}


#endif