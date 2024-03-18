#include "./FastPlace.h"
#include <chrono>
using namespace std;

namespace FastCGRA
{
    namespace FastPlace
    {
        std::unordered_map<std::string, std::unordered_set<std::string>> chisel_updateCompaTable(const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat,
                    const std::unordered_map<std::string, std::unordered_set<std::string>> &FUs)
        {
            unordered_map<string, unordered_set<string>> compatNew;
            unordered_map<string, unordered_set<string>> compatTEMP = compat;
            unordered_multimap<string, string> compatToDelete;
            for (const auto &vertex : DFG.vertices()) {
                compatNew[vertex.first] = unordered_set<string>();
                auto iter = compat.find(vertex.first);
                if(iter != compat.end()){
                    for(const auto &device : iter->second){
                        if (FUs.find(device) != FUs.end()) {
                            compatNew[vertex.first].insert(device);
                        }
                    }
                    if(compatNew[vertex.first].empty()){
                        if(compatTEMP[vertex.first].find("INPUT") != compatTEMP[vertex.first].end() || compatTEMP[vertex.first].find("OUTPUT") != compatTEMP[vertex.first].end() )
                            continue;
                        clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                } else {
                    string port = getPostfix(vertex.first);
                    string fu = getPrefix(vertex.first);
                    auto jter = compat.find(fu);
                    if(jter == compat.end()){   
                        clog << "FastPlacement: failed: unsupported device: " << fu << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                    for (const auto &device : jter->second) {
                        auto kter = FUs.find(device);
                        if(kter == FUs.end()){
                            continue;
                        }
                        if(kter->second.find(port) == kter->second.end()){
                            clog << "FastPlacement: undefined port found: " << fu << "." << port << "." << endl;
                            clog << "FastPlacement: Erase: " << fu << " -> " << device << endl;
                            compatToDelete.insert({fu, device});
                            continue;
                        }
                        compatNew[vertex.first].insert(device + "." + port);
                    }
                    if(compatNew[vertex.first].empty()){
                        if(compatTEMP[vertex.first].find("INPUT") != compatTEMP[vertex.first].end() || compatTEMP[vertex.first].find("OUTPUT") != compatTEMP[vertex.first].end() )
                            continue;
                        clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                }
            }
            for(const auto &toDelete: compatToDelete){
                const string &fu = toDelete.first;
                const string &device = toDelete.second;
                if(compatNew[fu].find(device) != compatNew[fu].end()){
                    compatNew[fu].erase(device);
                }
            }
            for(const auto &vertex: compatNew){
                if(vertex.second.empty()){
                    if(compatTEMP[vertex.first].find("INPUT") != compatTEMP[vertex.first].end() || compatTEMP[vertex.first].find("OUTPUT") != compatTEMP[vertex.first].end() )
                            continue;
                    clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                    return unordered_map<string, unordered_set<string>>();
                }
            }
            return compatNew;
        }

        std::unordered_map<std::string, std::unordered_set<std::string>> updateCompaTable(const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat,
                    const std::unordered_map<std::string, std::unordered_set<std::string>> &FUs)
        {
            unordered_map<string, unordered_set<string>> compatNew;
            unordered_multimap<string, string> compatToDelete;
            for (const auto &vertex : DFG.vertices()) {
                compatNew[vertex.first] = unordered_set<string>();
                auto iter = compat.find(vertex.first);
                if(iter != compat.end()){
                    for(const auto &device : iter->second){
                        if (FUs.find(device) != FUs.end()) {
                            compatNew[vertex.first].insert(device);
                        }
                    }
                    if(compatNew[vertex.first].empty()){
                        clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                } else {
                    string port = getPostfix(vertex.first);
                    string fu = getPrefix(vertex.first);
                    auto jter = compat.find(fu);
                    if(jter == compat.end()){   
                        clog << "FastPlacement: failed: unsupported device: " << fu << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                    for (const auto &device : jter->second) {
                        auto kter = FUs.find(device);
                        if(kter == FUs.end()){
                            continue;
                        }
                        if(kter->second.find(port) == kter->second.end()){
                            clog << "FastPlacement: undefined port found: " << fu << "." << port << "." << endl;
                            clog << "FastPlacement: Erase: " << fu << " -> " << device << endl;
                            compatToDelete.insert({fu, device});
                            continue;
                        }
                        compatNew[vertex.first].insert(device + "." + port);
                    }
                    if(compatNew[vertex.first].empty()){
                        clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                }
            }
            for(const auto &toDelete: compatToDelete){
                const string &fu = toDelete.first;
                const string &device = toDelete.second;
                if(compatNew[fu].find(device) != compatNew[fu].end()){
                    compatNew[fu].erase(device);
                }
            }
            for(const auto &vertex: compatNew){
                if(vertex.second.empty()){
                    clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                    return unordered_map<string, unordered_set<string>>();
                }
            }
            return compatNew;
        }


        void deleteForbiddened(std::unordered_map<std::string, std::unordered_set<std::string>> &compat, const std::unordered_multimap<std::string, std::string> &forbid)
        {
            for (const auto &tmp : forbid) {
                if (compat.find(tmp.first) != compat.end()) {
                    if (compat[tmp.first].find(tmp.second) != compat[tmp.first].end()) {
                        compat[tmp.first].erase(tmp.second);
                    }
                    if (compat[tmp.first].empty()) {
                        compat.erase(tmp.first);
                    }
                }
            }

        }

        std::vector<std::string> sortDFG(const Graph &dfg,const std::string &sortMode)
        {
            if(sortMode == "BFS"){
                return GraphSort::sortBFS(dfg);
            } else if(sortMode == "DFS"){
                return GraphSort::sortDFS(dfg);
            } else if(sortMode == "TVS"){
                return GraphSort::sortTVS(dfg);
            } else if(sortMode == "TOPO"){
                return GraphSort::sortTOPO(dfg);
            } else if(sortMode == "STB"){
                return GraphSort::sortSTB(dfg);
            }
            WARN << "FastPlacement::sortDFG: SortDFG Mode Not Found";
            return GraphSort::sortSTB(dfg);
        } 

        std::vector<std::string> sortDFG(const Graph &dfg,const std::string &sortMode, const std::string &seed)
        {
            if(sortMode == "BFS"){
                return GraphSort::sortBFS(dfg, seed);
            } else if(sortMode == "DFS"){
                return GraphSort::sortDFS(dfg, seed);
            } else if(sortMode == "TVS"){
                return GraphSort::sortTVS(dfg, seed);
            } else if(sortMode == "STB"){
                return GraphSort::sortSTB(dfg, seed);
            }
            WARN << "FastPlacement::sortDFG: SortDFG Mode Not Found";
            return GraphSort::sortSTB(dfg, seed);
        } 
        
 
        bool Place(const std::string &dfg, const std::string &compat, const std::string &rrg, const std::string &fus, const std::string &mapped, const std::string &routed, const std::string &sortMode, const std::string &seed)
        {
            Graph graphDFG(dfg);//Origin DFG
            clog << "Placement: DFG loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 
            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat);//Compat for graphDFGwithIO
            unordered_map<string, unordered_set<string>> FUs = NetworkAnalyzer::parse(fus); 

            Graph graphRRG(rrg);
            clog << "Placement: RRG loaded. " << "Vertices: " << graphRRG.nVertices() << "; " << "Edges: " << graphRRG.nEdges() << endl; 

            //Update Compatible
            unordered_map<string, unordered_set<string>> compatNew = updateCompaTable(graphDFG, compatFUs, FUs);
            if(compatNew.empty()){
                WARN << "FastPlacement::Placement: WARN: Found Uncompatible vertex.";
                return false;
            }

            //N-order validation
            NetworkAnalyzerLegacy analyzer(FUs, graphRRG); 
            Graph &RRGAnalyzed = analyzer.RRG();

            NOrderValidator validator(graphDFG, RRGAnalyzed, compatFUs);
            unordered_map<string, unordered_set<string>> device2vertexDFG;
            unordered_map<string, unordered_set<string>> compatibleVertexRRG;
            
            for (const auto &vertexDFG: compatFUs) {
                for (const auto &deviceDFG: vertexDFG.second) {
                    if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                        device2vertexDFG[deviceDFG] = unordered_set<string>();
                    }
                    device2vertexDFG[deviceDFG].insert(vertexDFG.first);
                }
            }
            for (const auto &vertexRRG: RRGAnalyzed.vertices()) {
                string device = vertexRRG.second.getAttr("device").getStr();
                if (device2vertexDFG.find(device) != device2vertexDFG.end()) {
                    for (const auto &vertexDFG : device2vertexDFG[device]) {
                        if (compatibleVertexRRG.find(vertexDFG) == compatibleVertexRRG.end()) {
                            compatibleVertexRRG[vertexDFG] = unordered_set<string>();
                        }
                        compatibleVertexRRG[vertexDFG].insert(vertexRRG.first);
                    }
                }
            }

            // size_t unplacibleCount = 0;
            // for (const auto &vertexDFG : graphDFG.vertices()) {
            //     if (!getPostfix(vertexDFG.first).empty()) {
            //         continue;
            //     }
            //     if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end()) {
            //         WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
            //         return false;
            //     }
            //     unordered_set<string> compatibles;
            //     for(const auto &vertexRRG: compatibleVertexRRG[vertexDFG.first]){
            //         clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
            //         // if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) { //NorderValidate
            //             compatibles.insert(vertexRRG);
            //         // }
            //     }
            //     clog << vertexDFG.first << ": " << compatibleVertexRRG[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
            //     compatibleVertexRRG[vertexDFG.first] = compatibles;
            // }
            // if(unplacibleCount > 0){
            //     clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
            //     return false;
            // }

            FastPlacer placer(graphRRG, analyzer);
            FastRouter router(graphRRG, FUs);

            unordered_map<std::string, std::string> vertexDFG2RRG;
            vector<string> order;
            if(seed.empty()){
                order = sortDFG(graphDFG, sortMode);
            } else {
                auto iter = graphDFG.vertices().find(seed);
                if(iter == graphDFG.vertices().end())
                {
                    WARN << "FastPlacement::Placement: Seed " << iter->first << " not Found";
                    return false;
                }
                order = sortDFG(graphDFG, sortMode, iter->first);
            }

            auto start = std::chrono::steady_clock::now();
            size_t count = 0; 
            while(count++ < 4)
            {
                
                vector<string> ordInit;
                if(sortMode == "TOPO_Forward"){
                    vector<string> ::reverse_iterator riter;
                    for(riter=order.rbegin(); riter!=order.rend();riter++){
                        if(getPostfix(*riter).empty()){
                            ordInit.push_back(*riter);
                    }
                }
                }
                else if(sortMode == "TOPO"){
                    for(const auto &node: order){
                    if(getPostfix(node).empty()){
                        ordInit.push_back(node);
                    }
                }}
                else{
                    for(const auto &node: order){
                        if(getPostfix(node).empty()){
                            ordInit.push_back(node);
                        }
                    }
                }
                pair<unordered_map<string, string>, FastRouter> result = placer.place(graphDFG, compatibleVertexRRG, router, vertexDFG2RRG, analyzer, validator, ordInit);
                size_t countFailed = 0;
                while(countFailed++ < 16 && result.first.empty())
                {
                    vector<string> failedVertices;
                    const unordered_map<string, size_t> &failures = placer.failedVertices();
                    for(const auto &vertex: failures)
                    {
                        failedVertices.push_back(vertex.first);
                    }
                    sort(failedVertices.begin(), failedVertices.end(), [&failures](const string &a, const string &b){
                        return failures.find(a)->second  > failures.find(b)->second; 
                    });
                    if(failedVertices.empty()){
                        continue;
                    }
                    if(countFailed < 8){
                        order = sortDFG(graphDFG, sortMode, failedVertices[0]);
                    } else if (countFailed < 12){
                        order = sortDFG(graphDFG, sortMode);
                    } else {
                        string seedInit = graphDFG.vertexNames()[rand() % graphDFG.vertexNames().size()];
                        order = sortDFG(graphDFG, sortMode, seedInit);
                    }
                    result = placer.place(graphDFG, compatibleVertexRRG, router, vertexDFG2RRG, analyzer, validator, order);
                }
                if(!result.first.empty()){

                    unordered_map<string, string> vertexDFG2RRG; 
                    for(const auto &vertex: result.first)
                    {
                        if(*compatFUs[getPrefix(vertex.first)].begin() != "__INPUT_FU__" && 
                        *compatFUs[getPrefix(vertex.first)].begin() != "__OUTPUT_FU__")
                        {
                            vertexDFG2RRG.insert(vertex); 
                        }
                    }

                    unordered_map<string, unordered_map<string, vector<string>>> paths; 
                    for(const auto &from: result.second.paths())
                    {
                        if(getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                        getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                        {
                            continue; 
                        }
                        if(paths.find(from.first) == paths.end())
                        {
                            paths[from.first] = unordered_map<string, vector<string>>(); 
                        }
                        for(const auto &to: from.second)
                        {
                            if(getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                            getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                            {
                                continue; 
                            }
                            assert(paths[from.first].find(to.first) == paths[from.first].end()); 
                            paths[from.first][to.first] = to.second; 
                        }
                    }
                    auto end = std::chrono::steady_clock::now();
                    std::chrono::duration<double> elapsed_secondas = end-start;
                    std::cout << "elapsed time: " << elapsed_secondas.count() << "s\n";
                    cout << dfg << " is success. " << endl;

                    std::ofstream outfile ("./hardpath.txt");
          

                    for(auto node: result.second.hardPath()){
                      outfile << (node.first) << " " << (node.second) << std::endl;
                    }
                    // outfile.close();
                    // string hardpath = "./hardpath.txt";
                    // Utils::writeMap(result.second.hardPath(), hardpath); 
                    
                    Utils::writeMap(vertexDFG2RRG, mapped); 
                    Utils::writePaths(paths, routed); 
                    return true; 
                }
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_secondas = end-start;
            std::cout << "elapsed time: " << elapsed_secondas.count() << "s\n";
            return false;
        }

    


        bool Chisel_Place(const std::string &dfg, const std::string &compat, const std::string &rrg, const std::string &fus, const std::string &mapped, const std::string &routed, const std::string &sortMode, const std::string &seed)
        {
            Graph graphDFG(dfg);//Origin DFG
            clog << "Placement: DFG loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 
            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat);//Compat for graphDFGwithIO
            unordered_map<string, unordered_set<string>> FUs = NetworkAnalyzer::parse(fus); 

            Graph graphRRG(rrg);
            clog << "Placement: RRG loaded. " << "Vertices: " << graphRRG.nVertices() << "; " << "Edges: " << graphRRG.nEdges() << endl; 

            //Update Compatible
            unordered_map<string, unordered_set<string>> compatNew = chisel_updateCompaTable(graphDFG, compatFUs, FUs);
            if(compatNew.empty()){
                WARN << "FastPlacement::Placement: WARN: Found Uncompatible vertex.";
                return false;
            }

            //N-order validation
            NetworkAnalyzerLegacy analyzer(FUs, graphRRG); 
            Graph &RRGAnalyzed = analyzer.RRG();

            NOrderValidator validator(graphDFG, RRGAnalyzed, compatFUs);
            unordered_map<string, unordered_set<string>> device2vertexDFG;
            unordered_map<string, unordered_set<string>> compatibleVertexRRG;
            
            for (const auto &vertexDFG: compatFUs) {
                for (const auto &deviceDFG: vertexDFG.second) {
                    if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                        device2vertexDFG[deviceDFG] = unordered_set<string>();
                    }
                    device2vertexDFG[deviceDFG].insert(vertexDFG.first);
                }
            }
            for (const auto &vertexRRG: RRGAnalyzed.vertices()) {
                string device = vertexRRG.second.getAttr("device").getStr();
                if (device2vertexDFG.find(device) != device2vertexDFG.end()) {
                    for (const auto &vertexDFG : device2vertexDFG[device]) {
                        if (compatibleVertexRRG.find(vertexDFG) == compatibleVertexRRG.end()) {
                            compatibleVertexRRG[vertexDFG] = unordered_set<string>();
                        }
                        compatibleVertexRRG[vertexDFG].insert(vertexRRG.first);
                    }
                }
            }

            // size_t unplacibleCount = 0;
            // for (const auto &vertexDFG : graphDFG.vertices()) {
            //     if (!getPostfix(vertexDFG.first).empty()) {
            //         continue;
            //     }
            //     if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end()) {
            //         WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
            //         return false;
            //     }
            //     unordered_set<string> compatibles;
            //     for(const auto &vertexRRG: compatibleVertexRRG[vertexDFG.first]){
            //         clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
            //         // if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) { //NorderValidate
            //             compatibles.insert(vertexRRG);
            //         // }
            //     }
            //     clog << vertexDFG.first << ": " << compatibleVertexRRG[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
            //     compatibleVertexRRG[vertexDFG.first] = compatibles;
            // }
            // if(unplacibleCount > 0){
            //     clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
            //     return false;
            // }

            FastPlacer placer(graphRRG, analyzer);
            FastRouter router(graphRRG, FUs);

            unordered_map<std::string, std::string> vertexDFG2RRG;
            auto start = std::chrono::steady_clock::now();
            size_t count = 0; 
            while(count++ < 4)
            {
                vector<string> order;
                if(seed.empty()){
                    order = sortDFG(graphDFG, sortMode);
                } else {
                    auto iter = graphDFG.vertices().find(seed);
                    if(iter == graphDFG.vertices().end())
                    {
                        WARN << "FastPlacement::Placement: Seed " << iter->first << " not Found";
                        return false;
                    }
                    order = sortDFG(graphDFG, sortMode, iter->first);
                }
                vector<string> ordInit;
                for(const auto &node: order){
                    if(getPostfix(node).empty()){
                        ordInit.emplace_back(node);
                    }
                }
                pair<unordered_map<string, string>, FastRouter> result = placer.place(graphDFG, compatibleVertexRRG, router, vertexDFG2RRG, analyzer, validator, ordInit);
                size_t countFailed = 0;
                while(countFailed++ < 16 && result.first.empty())
                {
                    vector<string> failedVertices;
                    const unordered_map<string, size_t> &failures = placer.failedVertices();
                    for(const auto &vertex: failures)
                    {
                        failedVertices.push_back(vertex.first);
                    }
                    sort(failedVertices.begin(), failedVertices.end(), [&failures](const string &a, const string &b){
                        return failures.find(a)->second  > failures.find(b)->second; 
                    });
                    if(failedVertices.empty()){
                        continue;
                    }
                    if(countFailed < 8){
                        order = sortDFG(graphDFG, sortMode, failedVertices[0]);
                    } else if (countFailed < 12){
                        order = sortDFG(graphDFG, sortMode);
                    } else {
                        string seedInit = graphDFG.vertexNames()[rand() % graphDFG.vertexNames().size()];
                        order = sortDFG(graphDFG, sortMode, seedInit);
                    }
                    result = placer.place(graphDFG, compatibleVertexRRG, router, vertexDFG2RRG, analyzer, validator, order);
                }
                if(!result.first.empty()){

                    unordered_map<string, string> vertexDFG2RRG; 
                    for(const auto &vertex: result.first)
                    {
                        if(*compatFUs[getPrefix(vertex.first)].begin() != "__INPUT_FU__" && 
                        *compatFUs[getPrefix(vertex.first)].begin() != "__OUTPUT_FU__")
                        {
                            vertexDFG2RRG.insert(vertex); 
                        }
                    }

                    unordered_map<string, unordered_map<string, vector<string>>> paths; 
                    for(const auto &from: result.second.paths())
                    {
                        if(getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                        getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                        {
                            continue; 
                        }
                        if(paths.find(from.first) == paths.end())
                        {
                            paths[from.first] = unordered_map<string, vector<string>>(); 
                        }
                        for(const auto &to: from.second)
                        {
                            if(getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                            getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                            {
                                continue; 
                            }
                            assert(paths[from.first].find(to.first) == paths[from.first].end()); 
                            paths[from.first][to.first] = to.second; 
                        }
                    }
                    auto end = std::chrono::steady_clock::now();
                    std::chrono::duration<double> elapsed_secondas = end-start;
                    std::cout << "elapsed time: " << elapsed_secondas.count() << "s\n";
                    cout << dfg << " is success. " << endl;

                    std::ofstream outfile ("./hardpath.txt");
          

                    for(auto node: result.second.hardPath()){
                    
                        
                      outfile << getPrefix(node.first) << " " << getPrefix(node.second) << std::endl;

                    }
                    // outfile.close();
                    // string hardpath = "./hardpath.txt";
                    // Utils::writeMap(result.second.hardPath(), hardpath); 
                    
                    Utils::writeMap(vertexDFG2RRG, mapped); 
                    Utils::writePaths(paths, routed); 
                    return true; 
                }
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_secondas = end-start;
            std::cout << "elapsed time: " << elapsed_secondas.count() << "s\n";
            return false;
        }

    
    }
}