#include "./FastPlacer.h"

using namespace std;

namespace FastCGRA
{


std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::place(const Graph &DFG, const std::unordered_map<std::string, 
                                                    std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                    const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial, const NetworkAnalyzerLegacy &analyzerInitial, const NOrderValidator &validatorInitial,
                                                 const std::vector<std::string> &order)
{
    const size_t Max_Failed_Times = 64;

    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);
    vector<string> Goorder = order;
    unordered_map<string, string> vertexDFG2RRG = usedDFG2RRGInitial;
    unordered_map<string, string> vertexRRG2DFG;
    unordered_map<string, bool>   travesalVertexDFG;

    unordered_map<string, string> coarse2RRG;
    unordered_map<string, string> RRG2coarse;
    unordered_map<string, unordered_set<string>> compat = compatible;
    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;
  
    string arch = "arch6";
    unordered_map<string, unordered_map<string, vector<string>>> config = Utils::readConifgFile("./arch/arch.ini");
    string RRG = config["Global"]["TopRRG"][0];
    if(RRG.find("arch6") == string::npos){
        arch = "arch7";
    }
    // for(const auto &vertex: vertexDFG2RRG)
    // {
    //     travesalVertexDFG[vertex.first] = true;
    //     vertexRRG2DFG[vertex.second] = vertex.first;
    // }
    vector<string> contractVertex;
    unordered_map<string, unordered_set<string>> contractCompat;
    unordered_map<string, unordered_set<string>> contractPorts;
    for(const auto &item: compat){
        if (getPostfix(item.first).empty()) {
                contractVertex.emplace_back(item.first);
                contractCompat.emplace(item);
        }
    }

    for(const auto &vertexDFG: DFG.vertices())
    {
        travesalVertexDFG[vertexDFG.first] = false;
        if(!getPostfix(vertexDFG.first).empty()){
            string prefix = getPrefix(vertexDFG.first);
            if (contractPorts.find(prefix) == contractPorts.end()) {
                contractPorts[prefix] = {};
            }
            contractPorts[prefix].emplace(getPostfix(vertexDFG.first));
        }
    }

    //N-order validation
    NetworkAnalyzerLegacy analyzer(analyzerInitial);
    Graph &RRGAnalyzed = analyzer.RRG();
    NOrderValidator validator(validatorInitial);
    // size_t unplacibleCount = 0;
    // for (const auto &vertexDFG : coarseDFG.vertices()) {
    //     if (!getPostfix(vertexDFG.first).empty() || vertexDFG.first.find("block") != string::npos) {
    //         continue;
    //     }
    //     if (compat.find(vertexDFG.first) == compat.end()) {
    //         WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
    //         return {unordered_map<string, string>(), router};
    //     }
    //     unordered_set<string> compatibles;
    //     for(const auto &vertexRRG: compat[vertexDFG.first]){
    //         clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
    //         if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) { //NorderValidate
    //             compatibles.insert(vertexRRG);
    //         }
    //     }
    //     clog << vertexDFG.first << ": " << compat[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
    //     compat[vertexDFG.first] = compatibles;
    // }
    // if(unplacibleCount > 0){
    //     clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
    //     return {unordered_map<string, string>(), router};
    // }

    size_t furthest = 0;
    size_t coarseIter = 0;
    size_t failureCount = 0;
    cout << endl << "FastPlacer: Begin placing. " << endl;
    vector<string> temp;
    // for(const auto &vertex: order)
    // {
    //     if(vertex.find("inport") != string::npos)
    //     {
    //         temp.push_back(vertex);
    //     }
    // }
    // for(const auto &vertex: order)
    // {
    //     if(vertex.find("insert") == string::npos)
    //     {
    //         temp.push_back(vertex);
    //     }
    // }
    // for(const auto &vertex: order)
    // {
    //     if(vertex.find("insert") != string::npos && vertex.find("inport") == string::npos)
    //     {
    //         temp.push_back(vertex);
    //     }
    // }
    // Goorder = temp;
    // cout << Goorder << endl;
    // cout << compatible << endl;
    // exit(0);
    string toMap = Goorder[0];
    cout << toMap << endl;
    unordered_set<string> verticesToTry = contractCompat.find(toMap)->second;
    while(coarseIter < Goorder.size()){
        furthest = max(furthest, coarseIter);
        clog << "FastPlacer: New iteration, size of stack: " << vertexDFG2RRG.size() << " / " << travesalVertexDFG.size() << "; furthest: " << furthest << endl;
        bool failed = false;
        string toTry = "";
        unordered_set<string> toDelete;
        // Prepare to delete used RRG vertices
        for(const auto &vertexToTry: verticesToTry){
            if(vertexRRG2DFG.find(vertexToTry) != vertexRRG2DFG.end()){
                toDelete.insert(vertexToTry);
            }
        }
        // Prepare to unconnectable RRG vertice
        for(const auto &vertexToTry: verticesToTry){
            if(toDelete.find(vertexToTry) != toDelete.end()){
                continue;
            }
            // bool available = true;
            // unordered_multimap<string, string> linksToValidate;
            // for(const auto &edge: DFG.edgesIn(toMap)){
            //     if(vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end()){
            //         continue;
            //     }
            //     const string &fromRRG = vertexDFG2RRG[edge.from()];
            //     linksToValidate.insert({fromRRG, vertexToTry});

            // }
            // for(const auto &edge: DFG.edgesOut(toMap)){
            //     if(vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()){
            //         continue;
            //     }
            //     const string &toRRG = vertexDFG2RRG[edge.to()];
            //     linksToValidate.insert({vertexToTry, toRRG});
            // }
            // for(const auto &link: linksToValidate){
            //     bool found = false;
            //     for(const auto &edge: _RRGAnalyzed.edgesOut(link.first)){
            //         if(edge.to() == link.second){
            //             found = true;
            //             break;
            //         }
            //     }
            //     if(!found){
            //         available = false;
            //         break;
            //     }
            // }
            // if(!available){
            //     toDelete.insert(vertexToTry);
            // }
        }
        //Delete
        for (const auto &vertex : toDelete) {
            verticesToTry.erase(vertex);
        }
        
        clog << "FastPlacer: -> toMap: " << toMap << "; Candidates After Purge: " << verticesToTry.size() << endl;
        vector<string> verticesToTryRanked(verticesToTry.begin(), verticesToTry.end());
        vector<pair<string, string>> edgesToMap;
        vector<string> edgesSignal;
        unordered_map<string, string> port2rrg;
        if (verticesToTry.empty()) {
            failed = true;
            clog << "FastPlacer: Failed. Nothing to try for " << toMap << endl;
        } else {
            //  sort the candidates by sharedNet
            // unordered_map<string, size_t> sharedNet;
            // unordered_set<string> inPortRRG;
            // unordered_set<string> outPortRRG;
            // for(const auto &vertexToTry: verticesToTry){
            //     for(const auto &port: contractPorts[toMap]){
            //         for(const auto &edge: DFG.edgesIn(toMap + "." + port)){//(out)->in
            //             if(vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end()){
            //                 continue;
            //             }
            //             inPortRRG.emplace(vertexDFG2RRG[edge.from()]);
            //             // const string &fromRRG = vertexDFG2RRG[edge.from()];
            //             // const string &toRRG = vertexToTry + "." + port;
            //             // linksToValidate.insert({fromRRG, toRRG});
            //         }
            //         for(const auto &edge: DFG.edgesOut(toMap + "." + port)){//out->(in)
            //             if(vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()){
            //                 continue;
            //             }
            //             outPortRRG.insert(vertexDFG2RRG[edge.to()]);
            //             // const string &toRRG = vertexDFG2RRG[edge.to()];
            //             // const string &fromRRG = vertexToTry + "." + port;
            //             // linksToValidate.insert({fromRRG, toRRG});
            //         }
            //     }
            // }
            // verticesToTryRanked.insert(verticesToTryRanked.begin(), coreVertex.begin(),coreVertex.end());
            //     unordered_set<string> inPortRRG;
            //     unordered_set<string> outPortRRG;
            //     for(const auto &edge: DFG.edgesIn(toMap)){
            //         inPortRRG.insert(vertexToTry);
            //     }
            //     for(const auto &edge: DFG.edgesOut(toMap)){

            //         outPortRRG.insert(vertexToTry);
            //         // if(toMapType == "Coin"){
            //         //     const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
            //         //     outPortRRG.insert(fromRRG);
            //         // } else {
            //         //     const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
            //         //     inPortRRG.insert(fromRRG);
            //         // }
            //     }
            //     for(const auto &inport: inPortRRG){
            //         for(const auto &edge: _RRGAnalyzed.edgesIn(inport)){
            //             if(vertexRRG2DFG.find(edge.from()) != vertexRRG2DFG.end()){
            //                 sharedNet[vertexToTry]++;
            //             }
            //         }
            //     }
            //     for(const auto &outport: outPortRRG){
            //         for(const auto &edge: _RRGAnalyzed.edgesOut(outport)){
            //             if(vertexRRG2DFG.find(edge.to()) != vertexRRG2DFG.end()){
            //                 sharedNet[vertexToTry]++;
            //             }
            //         }
            //     }
            // }
            // random_shuffle(verticesToTryRanked.end(), verticesToTryRanked.end());
            unordered_map<string, int> pos_dis;
            // sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return sharedNet[a] > sharedNet[b];});
            auto sortPE = [&](auto &toMap, auto &verticesToTry, auto &verticesToTryRanked){
                //  sort the candidates by sharedNet
                unordered_map<string, size_t> distancedNet;
                unordered_set<string> inPortRRG;
                unordered_set<string> outPortRRG;
                unordered_map<std::string, pair<int,int>> pos;

                if(arch.find("arch6") != string::npos){
                    pos = readPos("./arch6/RRG_coor.txt");
                }
                else{
                    pos = readPos("./arch7/RRG_coor.txt");
                }

                for(const auto &vertexToTry: verticesToTry){
                    inPortRRG.clear();
                    outPortRRG.clear();
                    for(const auto &port: contractPorts[toMap]){
                        int dis = 0;
                        for(const auto &edge: DFG.edgesIn(toMap + "." + port)){
                            if(vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end()){
                                continue;
                            }
                       
                            // string placed_cycle = split(vertexDFG2RRG[edge.from()],".")[0];
                            // string placed_pos = split(vertexDFG2RRG[edge.from()],".")[1];
                            int placed_x = pos[getPrefix(vertexDFG2RRG[edge.from()])].first;
                            int placed_y = pos[getPrefix(vertexDFG2RRG[edge.from()])].second;

                            // string try_cycle = split(vertexToTry,".")[0];
                            // string try_pos = split(vertexToTry,".")[1];
                            int try_x = pos[vertexToTry].first;
                            int try_y = pos[vertexToTry].second;
                            dis = manhattan_distance(placed_x, placed_y, 
                                                     try_x, try_y);
                            pos_dis[vertexToTry] += dis;
           
                            inPortRRG.emplace(vertexDFG2RRG[edge.from()]);
                        }
                        for(const auto &edge: DFG.edgesOut(toMap + "." + port)){//out->(in)
                            if(vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()){
                                continue;
                            }

                            int placed_x = pos[getPrefix(vertexDFG2RRG[edge.to()])].first;
                            int placed_y = pos[getPrefix(vertexDFG2RRG[edge.to()])].second;
                            
                            int try_x = pos[vertexToTry].first;
                            int try_y = pos[vertexToTry].second;

                            dis = manhattan_distance(placed_x, placed_y, 
                                                     try_x, try_y);
                            pos_dis[vertexToTry] += dis;
                  
                            outPortRRG.insert(vertexDFG2RRG[edge.to()]);
                        }
                    }
                }
            };
            cout << pos_dis << endl;
            sortPE(toMap, verticesToTry, verticesToTryRanked);
            sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return pos_dis[a] < pos_dis[b];});

            // try map vertex
            bool isSuccess = false;
            size_t IterVertextoTry = 0;
            while(!isSuccess && IterVertextoTry < verticesToTryRanked.size()){
                toTry = verticesToTryRanked[IterVertextoTry++];
                clog << "FastPlacer: try to map " << toMap << " to " << toTry << endl;
                // -> Find edges that need to be mapped
                edgesToMap.clear();
                edgesSignal.clear();
                unordered_map<string, unordered_map<string, vector<string>>> pathsGiven;
                if (!isSuccess) {
                    vector<string> portVertex = {};
                    port2rrg.clear();
                    for(const auto &port: contractPorts[toMap]){
                        port2rrg[toMap + "." + port] = toTry + "." + port;
                        if(port.find("in") != string::npos){
                            edgesToMap.emplace_back(pair<string, string>(toTry + "." + port, toTry));
                            edgesSignal.emplace_back(toMap + "." + port);
                        }
                        else{
                            edgesToMap.emplace_back(pair<string, string>(toTry, toTry + "." + port));
                            edgesSignal.emplace_back(toMap);
                        }
                    }
                    // travesalVertexDFG[toMap] = true;
                    // vertexDFG2RRG[toMap] = toTry;
                    for(const auto &vertex: port2rrg){
                        for (const auto &edge : DFG.edgesIn(vertex.first)) {
                            if (travesalVertexDFG[edge.from()] ) {
                                edgesToMap.emplace_back(pair<string, string>(vertexDFG2RRG[edge.from()], vertex.second));
                                edgesSignal.emplace_back(edge.from());
                            }
                        }
                        for (const auto &edge : DFG.edgesOut(vertex.first)) {
                            if (travesalVertexDFG[edge.to()]) {
                                edgesToMap.emplace_back(pair<string, string>(vertex.second, vertexDFG2RRG[edge.to()]));
                                edgesSignal.emplace_back(vertex.first);
                            }
                        }
                    }
                    // travesalVertexDFG[toMap] = false;
                    // vertexDFG2RRG.erase(toMap);
                    clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
                }
                if(edgesToMap.size() > 0){
                    isSuccess = router.norm_route(edgesToMap, edgesSignal);
                    // if(!isSuccess){
                    //     clog << "FastPlacer: Choose a loose route." << endl;
                    //     isSuccess = router.route(edgesToMap, edgesSignal);
                    // }
                } else {
                    isSuccess = true;
                }
            }
            if(!isSuccess){
                failed = true;
            }
        }
    
        if(!failed){
            vertexDFG2RRG[toMap] = toTry;
            vertexRRG2DFG[toTry] = toMap;
            travesalVertexDFG[toMap]=true;
            for(const auto &port: port2rrg){
                vertexDFG2RRG[port.first] = port.second;
                vertexRRG2DFG[port.second] = port.first;
                travesalVertexDFG[port.first] = true;
            }
            stackVerticesToTry.push(verticesToTry);
            stackEdgesToMap.push(edgesToMap);

            if(++coarseIter < Goorder.size()){
                toMap = Goorder[coarseIter];
                verticesToTry = compat.find(toMap)->second;
            } else {
                break;
            }
        } else {
            std::ofstream outfile ("./hardpath.txt");
            for(auto node: router.hardPath()){
                outfile << (node.first) << " " << (node.second) << std::endl;
            }
            outfile.close();
            failureCount++;
            if (_failedVertices.find(toMap) == _failedVertices.end()) {
                _failedVertices[toMap] = 0;
            }
            _failedVertices[toMap]++;
            if( _failedVertices[toMap] > Max_Failed_Times || failureCount > 32 * Max_Failed_Times){
                clog << "FastPlacer: FAILED. Too many failure. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            if(coarseIter == 0){
                clog << "FastPlacer: ALL FAILED. Nothing to try. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            toMap = Goorder[--coarseIter];
            verticesToTry = stackVerticesToTry.top();
            stackVerticesToTry.pop();
            clog << "FastPlacer: Roll back to: " << toMap << "; Candidates: " << verticesToTry.size() << endl;
            vector<pair<string, string>> edgesToDelte = stackEdgesToMap.top();
            stackEdgesToMap.pop();
            router.unroute(edgesToDelte);
            vertexRRG2DFG.erase(vertexDFG2RRG[toMap]);
            vertexDFG2RRG.erase(toMap);
            for(const auto &port: port2rrg){
                vertexDFG2RRG.erase(port.first);
                vertexRRG2DFG.erase(port.second);
                travesalVertexDFG[port.first] = false;
            }
            travesalVertexDFG[toMap]=false;
        }
        clog << endl << endl;
    }

    clog << "FastPlacer: finished placing the DFG. Failure count: " << failureCount << "." << endl
         << endl;

    return{vertexDFG2RRG, router};

}

std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::place_annealing(const Graph &graphDFG, const Graph &globalDFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                        const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial,  const NetworkAnalyzerLegacy &analyzerInitial, const NOrderValidator &validatorInitial,
                                                        const std::vector<std::string> &order)
{


    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);
    vector<string> Goorder = order;
    unordered_map<string, string> vertexDFG2RRG = usedDFG2RRGInitial;
    FastRouter routeInitialezed = routerInit;
    unordered_map<string, string> vertexRRG2DFG;
    unordered_map<string, bool>   travesalVertexDFG;

    unordered_map<string, unordered_set<string>> compat = compatible;
    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;

    for(const auto &vertexDFG: graphDFG.vertices())
    {
        travesalVertexDFG[vertexDFG.first] = false;
    }
    for(const auto &vertex: vertexDFG2RRG)
    {
        travesalVertexDFG[vertex.first] = true;
        vertexRRG2DFG[vertex.second] = vertex.first;
    }

    //route the different block edges
    for(const auto &vertex: graphDFG.vertices())
    {
        if(travesalVertexDFG[vertex.first])
        {
            for(const auto &edge: graphDFG.edgesOut(vertex.first))
            {
                if(travesalVertexDFG[edge.to()])
                {
                    assert(vertexDFG2RRG.find(edge.from()) != vertexDFG2RRG.end() && vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end());
                    const string &fromrrg = vertexDFG2RRG[edge.from()];
                    const string &torrg   = vertexDFG2RRG[edge.to()];
                    if (router.paths().find(fromrrg) == router.paths().end() ||
                        router.paths().find(fromrrg)->second.find(torrg) == router.paths().find(fromrrg)->second.end())
                        {
                            bool routed = router.norm_route({
                                                            {fromrrg, torrg},
                                                       },
                                                       {
                                                            edge.from(),
                                                       });
                            if(!routed)
                            {
                                clog << "FastRouter: FAILED, the path " << fromrrg << " -> " << torrg << " cannot route. " << endl;
                                return {unordered_map<string, string>(), router};
                            }
                        }
                }
            }
        }
    }
    //if only one part, it is finished.
    if(vertexDFG2RRG.size() == graphDFG.nVertices())
    {
        return {vertexDFG2RRG, router};
    }

    // N-order validation
    // NetworkAnalyzerLegacy analyzer(analyzerInitial);
    // Graph &RRGAnalyzed = analyzer.RRG();
    // NOrderValidator validator(validatorInitial);
    // size_t unplacibleCount = 0;
    // for (const auto &vertexDFG : coarseDFG.vertices()) {
    //     if (!getPostfix(vertexDFG.first).empty() || vertexDFG.first.find("block") != string::npos) {
    //         continue;
    //     }
    //     if (compat.find(vertexDFG.first) == compat.end()) {
    //         WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
    //         return {unordered_map<string, string>(), router};
    //     }
    //     unordered_set<string> compatibles;
    //     for(const auto &vertexRRG: compat[vertexDFG.first]){
    //         clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
    //         if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) { //NorderValidate
    //             compatibles.insert(vertexRRG);
    //         }
    //     }
    //     clog << vertexDFG.first << ": " << compat[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
    //     compat[vertexDFG.first] = compatibles;
    // }
    // if(unplacibleCount > 0){
    //     clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
    //     return {unordered_map<string, string>(), router};
    // }

    //Init
    vector<string> contractVertex;
    unordered_map<string, unordered_set<string>> contractCompat;
    unordered_map<string, unordered_set<string>> contractPorts;
    string exchange ;
    for (const auto &item : compat) {
        if (getPostfix(item.first).empty()) {
            if (vertexDFG2RRG.find(item.first) == vertexDFG2RRG.end()) {
                contractVertex.push_back(item.first);
                contractCompat.insert(item);
            }
        } else {
            string prefix = getPrefix(item.first);
            if (contractPorts.find(prefix) == contractPorts.end()) {
                contractPorts[prefix] = {};
            }
            contractPorts[prefix].insert(getPostfix(item.first));
        }
    }
    clog << contractCompat <<endl;

    FastMatcher matchPair(contractCompat);
    size_t numMatched = matchPair.match();
    if (numMatched < contractCompat.size()) {
        clog << "FastPlace_annealing: FAILED, initial matching failed. " << endl;
        for (const auto &item : contractCompat) {
            if (matchPair.matchTable().find(item.first) == matchPair.matchTable().end()) {
                clog << " -> Unmatched vertex: " << item.first << ": " << item.second << endl;
            }
        }
        return { unordered_map<string, string>(), router };
    }
    unordered_map<string, string> matched = matchPair.matchTable();
    clog << matched << endl;

    unordered_map<string, string> vertexDFG2RRGTmp;
    unordered_map<string, string> vertexRRG2DFGTmp;
    vector<pair<string, string>> edgesToMap;
    vector<string> edgesSignal;

    auto recover = [&](const unordered_map<string, string> &matched, const unordered_set<string> &related = {}) {
        if (related.empty()) {
            vertexDFG2RRGTmp = vertexDFG2RRG;
            vertexRRG2DFGTmp = unordered_map<string, string>();
            for (const auto &item : vertexDFG2RRGTmp) {
                vertexRRG2DFGTmp[item.second] = item.first;
            }
        } else {
            for (const auto &item : matched) {
                if (related.find(item.first) != related.end()) {
                    string oldDFG = item.first;
                    string oldRRG = vertexDFG2RRGTmp[item.first];
                    vertexDFG2RRGTmp.erase(oldDFG);
                    vertexRRG2DFGTmp.erase(oldRRG);
                    for (const auto &port : contractPorts[oldDFG]) {
                        vertexDFG2RRGTmp.erase(oldDFG + "." + port);
                        vertexRRG2DFGTmp.erase(oldRRG + "." + port);
                    }
                }
            }
        }
        for (const auto &item : matched) {
            if (related.empty() || related.find(item.first) != related.end()) {
                vertexDFG2RRGTmp[item.first] = item.second;
                vertexRRG2DFGTmp[item.second] = item.first;
                for (const auto &port : contractPorts[item.first]) {
                    vertexDFG2RRGTmp[item.first + "." + port] = item.second + "." + port;
                    vertexRRG2DFGTmp[item.second + "." + port] = item.first + "." + port;
                }
            }
        }
    };

    auto evaluate = [&](const unordered_map<string, string> &matched, const unordered_set<string> &related = {}) -> double {
        if (!related.empty()) {
            vector<pair<string, string>> edgesToRip;
            for (const auto &vertex : graphDFG.vertices()) {
                for (const auto &edge : graphDFG.edgesOut(vertex.first)) {
                    if (vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end() ||
                        vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()) {
                        string fromFU = getPrefix(edge.from());
                        string toFU = getPrefix(edge.to());
                        if (related.find(fromFU) != related.end() || related.find(toFU) != related.end()) {
                            edgesToRip.push_back({ vertexDFG2RRGTmp[edge.from()], vertexDFG2RRGTmp[edge.to()] });
                        }
                    }
                }
            }
            router.unroute(edgesToRip);
        }

        recover(matched, related);
        //         router.clear();
        if (related.empty()) {
            router.clear();
            router = routeInitialezed;
        }
        edgesToMap = vector<pair<string, string>>();
        edgesSignal = vector<string>();
        for (const auto &vertex : graphDFG.vertices()) {
            for (const auto &edge : graphDFG.edgesOut(vertex.first)) {
                if (vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end() ||
                    vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()) {
                    edgesToMap.push_back({ vertexDFG2RRGTmp[edge.from()], vertexDFG2RRGTmp[edge.to()] });
                    edgesSignal.push_back(edge.from());
                }
            }
        }
        size_t countRouted = 0;
        for (size_t idx = 0; idx < edgesToMap.size(); idx++) {
            if (getPrefix(edgesToMap[idx].first) != getPrefix(edgesToMap[idx].second)) {
                clog << "FastPlace_annealing: -> Routing " << idx << "/" << countRouted << "/" << edgesToMap.size() << "; path: " << edgesToMap[idx] << (router.pathUsed(edgesToMap[idx], edgesSignal[idx]) ? " ; routed. " : "; to route. ") << endl;
            }
            // Skip if unconnected
            const string &fromVertex = edgesToMap[idx].first;
            const string &toVertex = edgesToMap[idx].second;
            bool found = false;
            for (const auto &edge : analyzerInitial.RRG().edgesOut(fromVertex)) {
                if (edge.to() == toVertex) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                continue;
            }
            // route
            countRouted += router.pathUsed(edgesToMap[idx], edgesSignal[idx]) ? 1 : router.norm_route({ edgesToMap[idx] }, { edgesSignal[idx] });
        }
        for (size_t idx = 0; idx < edgesToMap.size(); idx++) {
            if (router.pathUsed(edgesToMap[idx], edgesSignal[idx])) {
                continue;
            }
            exchange = vertexRRG2DFGTmp[edgesToMap[idx].first];
            clog << "fail path:  signal: " << edgesSignal[idx] << ";  (" << edgesToMap[idx].first << " : " << edgesToMap[idx].second << ") " << endl;
        }
        return countRouted;
    };


    unordered_map<string, string> dfg2rrgPair = matched;
    //     double score = 0.0;
    double score = evaluate(dfg2rrgPair);
    clog << endl
         << "FastPlace_annealing: Initialized "
         << ": " << score << endl;

    if (score == 0) {
        FastRouter bestRouter(_RRG);
        return { unordered_map<string, string>(), bestRouter };
    }
    // for (size_t iteration = 0; iteration < 1024; iteration++) //PARAM
    // {
    //     // Move
    //     size_t index;
    //     bool indexOk = false;
    //     while (!indexOk) {
    //         index = rand() % contractVertex.size();
    //         if (contractCompat[contractVertex[index]].size() > 1) {
    //             indexOk = true;
    //         }
    //     }
    //     vector<string> candidates(contractCompat[contractVertex[index]].begin(),
    //                               contractCompat[contractVertex[index]].end());
    //     size_t jndex;
    //     bool jndexOk = false;
    //     while (!jndexOk) {
    //         jndex = rand() % candidates.size();
    //         if (candidates[jndex] != vertexDFG2RRGTmp[contractVertex[index]]) {
    //             jndexOk = true;
    //         }
    //     }

    //     const string &moveDFG = contractVertex[index];
    //     const string &fromRRG = vertexDFG2RRGTmp[moveDFG];
    //     const string &toRRG = candidates[jndex];

    //     clog << endl
    //          << "FastPlace_annealing: Shuffle iteration " << iteration;
    //     unordered_set<string> related;
    //     if (vertexRRG2DFGTmp.find(toRRG) != vertexRRG2DFGTmp.end()) {
    //         const string &toDFG = vertexRRG2DFGTmp[toRRG];
    //         if (contractCompat[toDFG].find(fromRRG) != contractCompat[toDFG].end()) // fromRRG
    //         {
    //             dfg2rrgPair[moveDFG] = toRRG;
    //             dfg2rrgPair[toDFG] = fromRRG;
    //             related.insert(moveDFG);
    //             related.insert(toDFG);
    //             clog << "; swap: "
    //                  << "(" << moveDFG << ", " << fromRRG << ") and (" << toDFG << ", " << toRRG << ")" << endl;
    //         } else {
    //             clog << ": must reject. " << endl;
    //             continue;
    //         }
    //     } else {
    //         dfg2rrgPair[moveDFG] = toRRG;
    //         related.insert(moveDFG);
    //         clog << "; move: "
    //              << "(" << moveDFG << ", " << fromRRG << ") to " << toRRG << endl;
    //     }

    //     recover(dfg2rrgPair);
    // }

    // Test
    score = evaluate(dfg2rrgPair);
    FastRouter bestRouter(_RRG);
    bestRouter = router;
    unordered_map<string, string> bestSofarCoarse = dfg2rrgPair;
    unordered_map<string, string> bestSofarDFG2RRG = vertexDFG2RRGTmp;
    unordered_map<string, string> bestSofarRRG2DFG = vertexRRG2DFGTmp;
    clog << endl
         << "FastPlace_annealing: Initialized "
         << ": " << score << endl;
    //     double temperature = 1.0;
    double temperature = 0.9;
    size_t iteration = 0;
    for (; iteration < 1024 * 16; iteration++) //PARAM 1024 * 16 256
    // for(; iteration < 1024 * 4; iteration++) //PARAM 1024 * 16 256
    {
        if (iteration > 0 && iteration % 256 == 0) {
            //             temperature /= 1.25;
            temperature /= 1.1;
        }
        dfg2rrgPair = bestSofarCoarse;
        vertexDFG2RRGTmp = bestSofarDFG2RRG;
        vertexRRG2DFGTmp = bestSofarRRG2DFG;
        router = bestRouter;
        // Move
        size_t index;
        bool indexOk = false;
        while (!indexOk) {
            // index = rand() % contractVertex.size();
            // if (contractCompat[contractVertex[index]].size() > 1) {
            //     indexOk = true;
            // }
            if (contractCompat[exchange].size() > 1) {
                indexOk = true;
            }
        }
        vector<string> candidates(contractCompat[exchange].begin(),
                                  contractCompat[exchange].end());
        size_t jndex;
        bool jndexOk = false;
        while (!jndexOk) {
            jndex = rand() % candidates.size();
            if (candidates[jndex] != vertexDFG2RRG[contractVertex[index]]) {
                jndexOk = true;
            }
        }

        const string &moveDFG = contractVertex[index];
        const string &fromRRG = vertexDFG2RRGTmp[moveDFG];
        const string &toRRG = candidates[jndex];

        clog << endl
             << "FastPlace_annealing: Iteration " << iteration;
        unordered_set<string> related;
        if (vertexRRG2DFGTmp.find(toRRG) != vertexRRG2DFGTmp.end()) {
            const string &toDFG = vertexRRG2DFG[toRRG];
            if (contractCompat[toDFG].find(fromRRG) != contractCompat[toDFG].end()) {
                dfg2rrgPair[moveDFG] = toRRG;
                dfg2rrgPair[toDFG] = fromRRG;
                related.insert(moveDFG);
                related.insert(toDFG);
                clog << "; swap: "
                     << "(" << moveDFG << ", " << fromRRG << ") and (" << toDFG << ", " << toRRG << ")" << endl;
            } else {
                clog << ": must reject. " << endl;
                continue;
            }
        } else {
            vertexDFG2RRGTmp[moveDFG] = toRRG;
            related.insert(moveDFG);
            clog << "; move: "
                 << "(" << moveDFG << ", " << fromRRG << ") to " << toRRG << endl;
        }

        // Evaluate
        double scoreCurrent = evaluate(dfg2rrgPair, related);
        clog << "FastPlace_annealing: -> " << scoreCurrent << " vs. " << score << endl;
        bool accepted = false;
        double delta = scoreCurrent - score;
        if (delta > 0.0) {
            accepted = true;
            clog << "FastPlace_annealing: -> Accepted. " << endl;
        } else {
            double prob = exp(delta / temperature) / 2.0;
            double rn = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
            if (rn < prob) {
                accepted = true;
                clog << "FastPlace_annealing: -> Accepted with prob " << rn << "/" << prob << endl;
            } else {
                clog << "FastPlace_annealing: -> Rejected with prob " << rn << "/" << prob << endl;
            }
        }
        if (accepted) {
            score = scoreCurrent;
            bestSofarCoarse = dfg2rrgPair;
            bestSofarDFG2RRG = vertexDFG2RRGTmp;
            bestSofarRRG2DFG = vertexRRG2DFGTmp;
            bestRouter = router;
        }

        clog << endl;

        if (score >= edgesToMap.size()) {
            break;
        }
    }

    clog << "FastPlace_annealing: Final score: " << score << endl;
    if (score < edgesToMap.size()) {
        clog << "END FastPlace_annealing: FAILED: " << endl
             << endl;
        return { unordered_map<string, string>(), bestRouter };
    }

    clog << "END FastPlace_annealing: finished placing the DFG. Failure count: " << iteration << "." << endl
         << endl;

    return { bestSofarDFG2RRG, bestRouter };

}



std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::chisel_place(const Graph &DFG, const std::unordered_map<std::string, 
                                                    std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                    const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial, const NetworkAnalyzerLegacy &analyzerInitial, const NOrderValidator &validatorInitial,
                                                 const std::vector<std::string> &order)
{
    const size_t Max_Failed_Times = 64;

    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);
    vector<string> Goorder = order;
    unordered_map<string, string> vertexDFG2RRG = usedDFG2RRGInitial;
    unordered_map<string, string> vertexRRG2DFG;
    unordered_map<string, bool>   travesalVertexDFG;

    unordered_map<string, string> coarse2RRG;
    unordered_map<string, string> RRG2coarse;
    unordered_map<string, unordered_set<string>> compat = compatible;
    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;
  
    
    // for(const auto &vertex: vertexDFG2RRG)
    // {
    //     travesalVertexDFG[vertex.first] = true;
    //     vertexRRG2DFG[vertex.second] = vertex.first;
    // }
    vector<string> contractVertex;
    unordered_map<string, unordered_set<string>> contractCompat;
    unordered_map<string, unordered_set<string>> contractPorts;
    for(const auto &item: compat){
        if (getPostfix(item.first).empty()) {
                contractVertex.emplace_back(item.first);
                contractCompat.emplace(item);
        }
    }

    for(const auto &vertexDFG: DFG.vertices())
    {
        travesalVertexDFG[vertexDFG.first] = false;
        if(!getPostfix(vertexDFG.first).empty()){
            string prefix = getPrefix(vertexDFG.first);
            if (contractPorts.find(prefix) == contractPorts.end()) {
                contractPorts[prefix] = {};
            }
            contractPorts[prefix].emplace(getPostfix(vertexDFG.first));
        }
    }

    //N-order validation
    NetworkAnalyzerLegacy analyzer(analyzerInitial);
    Graph &RRGAnalyzed = analyzer.RRG();
    NOrderValidator validator(validatorInitial);
    // size_t unplacibleCount = 0;
    // for (const auto &vertexDFG : coarseDFG.vertices()) {
    //     if (!getPostfix(vertexDFG.first).empty() || vertexDFG.first.find("block") != string::npos) {
    //         continue;
    //     }
    //     if (compat.find(vertexDFG.first) == compat.end()) {
    //         WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
    //         return {unordered_map<string, string>(), router};
    //     }
    //     unordered_set<string> compatibles;
    //     for(const auto &vertexRRG: compat[vertexDFG.first]){
    //         clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
    //         if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) { //NorderValidate
    //             compatibles.insert(vertexRRG);
    //         }
    //     }
    //     clog << vertexDFG.first << ": " << compat[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
    //     compat[vertexDFG.first] = compatibles;
    // }
    // if(unplacibleCount > 0){
    //     clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
    //     return {unordered_map<string, string>(), router};
    // }

    size_t furthest = 0;
    size_t coarseIter = 0;
    size_t failureCount = 0;
    cout << endl << "FastPlacer: Begin placing. " << endl;
    vector<string> temp;
    // for(const auto &vertex: order)
    // {
    //     if(vertex.find("inport") != string::npos)
    //     {
    //         temp.push_back(vertex);
    //     }
    // }
    // for(const auto &vertex: order)
    // {
    //     if(vertex.find("insert") == string::npos)
    //     {
    //         temp.push_back(vertex);
    //     }
    // }
    // for(const auto &vertex: order)
    // {
    //     if(vertex.find("insert") != string::npos && vertex.find("inport") == string::npos)
    //     {
    //         temp.push_back(vertex);
    //     }
    // }
    // Goorder = temp;
    // cout << Goorder << endl;
    // cout << compatible << endl;
    // exit(0);
    string toMap = Goorder[1];
    cout << toMap << endl;
    unordered_set<string> verticesToTry = contractCompat.find(toMap)->second;
    while(coarseIter < Goorder.size()){
        furthest = max(furthest, coarseIter);
        clog << "FastPlacer: New iteration, size of stack: " << vertexDFG2RRG.size() << " / " << travesalVertexDFG.size() << "; furthest: " << furthest << endl;
        bool failed = false;
        string toTry = "";
        unordered_set<string> toDelete;
        // Prepare to delete used RRG vertices
        for(const auto &vertexToTry: verticesToTry){
            if(vertexRRG2DFG.find(vertexToTry) != vertexRRG2DFG.end()){
                toDelete.insert(vertexToTry);
            }
        }
        // Prepare to unconnectable RRG vertice
        for(const auto &vertexToTry: verticesToTry){
            if(toDelete.find(vertexToTry) != toDelete.end()){
                continue;
            }
            // bool available = true;
            // unordered_multimap<string, string> linksToValidate;
            // for(const auto &edge: DFG.edgesIn(toMap)){
            //     if(vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end()){
            //         continue;
            //     }
            //     const string &fromRRG = vertexDFG2RRG[edge.from()];
            //     linksToValidate.insert({fromRRG, vertexToTry});

            // }
            // for(const auto &edge: DFG.edgesOut(toMap)){
            //     if(vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()){
            //         continue;
            //     }
            //     const string &toRRG = vertexDFG2RRG[edge.to()];
            //     linksToValidate.insert({vertexToTry, toRRG});
            // }
            // for(const auto &link: linksToValidate){
            //     bool found = false;
            //     for(const auto &edge: _RRGAnalyzed.edgesOut(link.first)){
            //         if(edge.to() == link.second){
            //             found = true;
            //             break;
            //         }
            //     }
            //     if(!found){
            //         available = false;
            //         break;
            //     }
            // }
            // if(!available){
            //     toDelete.insert(vertexToTry);
            // }
        }
        //Delete
        for (const auto &vertex : toDelete) {
            verticesToTry.erase(vertex);
        }
        
        clog << "FastPlacer: -> toMap: " << toMap << "; Candidates After Purge: " << verticesToTry.size() << endl;
        vector<string> verticesToTryRanked(verticesToTry.begin(), verticesToTry.end());
        unordered_set<string> coreVertex;
        vector<pair<string, string>> edgesToMap;
        vector<string> edgesSignal;
        unordered_map<string, string> port2rrg;
        if (verticesToTry.empty()) {
            failed = true;
            clog << "FastPlacer: Failed. Nothing to try for " << toMap << endl;
        } else {
            //  sort the candidates by sharedNet
            // unordered_map<string, size_t> sharedNet;
            unordered_set<string> inPortRRG;
            unordered_set<string> outPortRRG;
            for(const auto &vertexToTry: verticesToTry){
                for(const auto &port: contractPorts[toMap]){
                    for(const auto &edge: DFG.edgesIn(toMap + "." + port)){//(out)->in
                        if(vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end()){
                            continue;
                        }
                        string core = getFront(vertexDFG2RRG[edge.from()]);
                        
                        if(vertexToTry.find(core) != string::npos){
                            coreVertex.emplace(vertexToTry);
                            // cout <<core <<  "  " << vertexToTry<< endl ;
                            // cout << "A" <<endl;
                        }
                        inPortRRG.emplace(vertexDFG2RRG[edge.from()]);
                        // const string &fromRRG = vertexDFG2RRG[edge.from()];
                        // const string &toRRG = vertexToTry + "." + port;
                        // linksToValidate.insert({fromRRG, toRRG});
                    }
                    for(const auto &edge: DFG.edgesOut(toMap + "." + port)){//out->(in)
                        if(vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()){
                            continue;
                        }
                        string core = getFront(vertexDFG2RRG[edge.to()]);
                        if(vertexToTry.find(core) != string::npos){
                            coreVertex.emplace(vertexToTry);
                            // cout <<core <<  "  " << vertexToTry<< endl ;
                        }
                        outPortRRG.insert(vertexDFG2RRG[edge.to()]);
                        // const string &toRRG = vertexDFG2RRG[edge.to()];
                        // const string &fromRRG = vertexToTry + "." + port;
                        // linksToValidate.insert({fromRRG, toRRG});
                    }
                }
            }
            verticesToTryRanked.insert(verticesToTryRanked.begin(), coreVertex.begin(),coreVertex.end());
            //     unordered_set<string> inPortRRG;
            //     unordered_set<string> outPortRRG;
            //     for(const auto &edge: DFG.edgesIn(toMap)){
            //         inPortRRG.insert(vertexToTry);
            //     }
            //     for(const auto &edge: DFG.edgesOut(toMap)){

            //         outPortRRG.insert(vertexToTry);
            //         // if(toMapType == "Coin"){
            //         //     const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
            //         //     outPortRRG.insert(fromRRG);
            //         // } else {
            //         //     const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
            //         //     inPortRRG.insert(fromRRG);
            //         // }
            //     }
            //     for(const auto &inport: inPortRRG){
            //         for(const auto &edge: _RRGAnalyzed.edgesIn(inport)){
            //             if(vertexRRG2DFG.find(edge.from()) != vertexRRG2DFG.end()){
            //                 sharedNet[vertexToTry]++;
            //             }
            //         }
            //     }
            //     for(const auto &outport: outPortRRG){
            //         for(const auto &edge: _RRGAnalyzed.edgesOut(outport)){
            //             if(vertexRRG2DFG.find(edge.to()) != vertexRRG2DFG.end()){
            //                 sharedNet[vertexToTry]++;
            //             }
            //         }
            //     }
            // }
            // random_shuffle(verticesToTryRanked.end(), verticesToTryRanked.end());
            // sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return sharedNet[a] > sharedNet[b];});
            
            // try map vertex
            bool isSuccess = false;
            size_t IterVertextoTry = 0;
            while(!isSuccess && IterVertextoTry < verticesToTryRanked.size()){
                toTry = verticesToTryRanked[IterVertextoTry++];
                clog << "FastPlacer: try to map " << toMap << " to " << toTry << endl;
                // -> Find edges that need to be mapped
                edgesToMap.clear();
                edgesSignal.clear();
                unordered_map<string, unordered_map<string, vector<string>>> pathsGiven;
                if (!isSuccess) {
                    vector<string> portVertex = {};
                    port2rrg.clear();
                    for(const auto &port: contractPorts[toMap]){
                        port2rrg[toMap + "." + port] = toTry + "." + port;
                        if(port.find("in") != string::npos){
                            edgesToMap.emplace_back(pair<string, string>(toTry + "." + port, toTry));
                            edgesSignal.emplace_back(toMap + "." + port);
                        }
                        else{
                            edgesToMap.emplace_back(pair<string, string>(toTry, toTry + "." + port));
                            edgesSignal.emplace_back(toMap);
                        }
                    }
                    // travesalVertexDFG[toMap] = true;
                    // vertexDFG2RRG[toMap] = toTry;
                    for(const auto &vertex: port2rrg){
                        for (const auto &edge : DFG.edgesIn(vertex.first)) {
                            if (travesalVertexDFG[edge.from()] ) {
                                edgesToMap.emplace_back(pair<string, string>(vertexDFG2RRG[edge.from()], vertex.second));
                                edgesSignal.emplace_back(edge.from());
                            }
                        }
                        for (const auto &edge : DFG.edgesOut(vertex.first)) {
                            if (travesalVertexDFG[edge.to()]) {
                                edgesToMap.emplace_back(pair<string, string>(vertex.second, vertexDFG2RRG[edge.to()]));
                                edgesSignal.emplace_back(vertex.first);
                            }
                        }
                    }
                    // travesalVertexDFG[toMap] = false;
                    // vertexDFG2RRG.erase(toMap);
                    clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
                }
                if(edgesToMap.size() > 0){
                    isSuccess = router.norm_route(edgesToMap, edgesSignal);
                    // if(!isSuccess){
                    //     clog << "FastPlacer: Choose a loose route." << endl;
                    //     isSuccess = router.route(edgesToMap, edgesSignal);
                    // }
                } else {
                    isSuccess = true;
                }
            }
            if(!isSuccess){
                failed = true;
            }
        }
    
        if(!failed){
            vertexDFG2RRG[toMap] = toTry;
            vertexRRG2DFG[toTry] = toMap;
            travesalVertexDFG[toMap]=true;
            for(const auto &port: port2rrg){
                vertexDFG2RRG[port.first] = port.second;
                vertexRRG2DFG[port.second] = port.first;
                travesalVertexDFG[port.first] = true;
            }
            stackVerticesToTry.push(verticesToTry);
            stackEdgesToMap.push(edgesToMap);

            if(++coarseIter < Goorder.size()){
                toMap = Goorder[coarseIter];
                verticesToTry = compat.find(toMap)->second;
            } else {
                break;
            }
        } else {
                std::ofstream outfile ("./hardpath.txt");

                for(auto node: router.hardPath()){

                    outfile << getPrefix(node.first) << " " << getPrefix(node.second) << std::endl;

                }
                outfile.close();
            failureCount++;
            if (_failedVertices.find(toMap) == _failedVertices.end()) {
                _failedVertices[toMap] = 0;
            }
            _failedVertices[toMap]++;
            if( _failedVertices[toMap] > Max_Failed_Times || failureCount > 32 * Max_Failed_Times){
                clog << "FastPlacer: FAILED. Too many failure. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            if(coarseIter == 0){
                clog << "FastPlacer: ALL FAILED. Nothing to try. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            toMap = Goorder[--coarseIter];
            verticesToTry = stackVerticesToTry.top();
            stackVerticesToTry.pop();
            clog << "FastPlacer: Roll back to: " << toMap << "; Candidates: " << verticesToTry.size() << endl;
            vector<pair<string, string>> edgesToDelte = stackEdgesToMap.top();
            stackEdgesToMap.pop();
            router.unroute(edgesToDelte);
            vertexRRG2DFG.erase(vertexDFG2RRG[toMap]);
            vertexDFG2RRG.erase(toMap);
            for(const auto &port: port2rrg){
                vertexDFG2RRG.erase(port.first);
                vertexRRG2DFG.erase(port.second);
                travesalVertexDFG[port.first] = false;
            }
            travesalVertexDFG[toMap]=false;
        }
        clog << endl << endl;
    }

    clog << "FastPlacer: finished placing the DFG. Failure count: " << failureCount << "." << endl
         << endl;

    return{vertexDFG2RRG, router};

}


}