#include "../inc/pruning.h"

using namespace std;

namespace FastCGRA
{
    namespace PRUNING
    {
        bool Pruning::Parse(const string &filePath0, const string &filePath1)
        {
            clog << "start pruning parse" << endl;
            ParsePruningSwitchFile(filePath0);
            ParsePruningFuFile(filePath1);

            clog << "Pruning Parse finished" << endl;
            return true;
        }

        bool Pruning::ParsePruningSwitchFile(const string & filePath)
        {
            ifstream fin(filePath);
            if (!fin)
            {
                clog << "Pruning switch file parse error: " << filePath << " cannot be opened" << endl;
                return false;
            }
            switchDeleteVertices["TOP"] = unordered_set<string> ();
            string Model;
            while (!fin.eof()) 
            {
                string line;
                getline(fin, line);
                if (line == "TOP")
                {
                    Model = "TOP";
                    continue;
                }

                istringstream sin(line);
                string tmpStr;
                sin >> tmpStr;
                while(!tmpStr.empty())
                {
                    switchDeleteVertices[Model].insert(tmpStr);
                    tmpStr.clear();
                    sin >> tmpStr;
                }
            }
            fin.close();

            clog << "Finished Pruning parse switch File" << endl;
            return true;
        }

        bool Pruning::ParsePruningFuFile(const string &filePath)
        {
            clog << "parse Pruning Fu started: " << endl;
            clog << filePath << endl;
            ifstream fin(filePath);
            if (!fin)
            {
                clog << "ParsePruningFuFile failed: " << filePath << " cannot be opened" << endl;
                return false;
            }
            string Model;
            string line;
            while(!fin.eof())
            {
                getline(fin, line);
                // clog << "line : " << line << endl;
                if (line.find("Rep") != line.npos)
                {
                    Model = "Replace";
                    continue;
                }
                if (line.find("Del") != line.npos)
                {
                    Model = "Delete";
                    continue;
                }
                istringstream sin(line);
                if (Model == "Replace")
                {
                    string _fuType, _fuDelete, _fuReplace;
                    sin >> _fuType >> _fuDelete >> _fuReplace;
                    unordered_map<string, string> tmpdic;
                    tmpdic.insert(make_pair(_fuReplace, _fuType));
                    fbReplaceVertices.insert(make_pair(_fuDelete, tmpdic));
                }
                if (Model == "Delete")
                {
                    string tmpStr, fbType;
                    sin >> tmpStr;
                    while(!tmpStr.empty())
                    {
                        fbDeleteVertices.push_back(tmpStr);
                        tmpStr.clear();
                        sin >> tmpStr;
                    }
                }  
            }
            fin.close();
            clog << "fbReplaceVertices : " << fbReplaceVertices << endl;
            clog << "fbDeleteVertices : " << fbDeleteVertices << endl;
            clog << "parse pruning Fu file finished" << endl;
            return true;
        }

        bool Pruning::pruningRRG()
        {
            _pruningFu();
            _pruningSwitch();
            return true;
        }

        bool Pruning::_pruningSwitch()
        {
            for (const auto &vertex : switchDeleteVertices["TOP"]) {
                ArchRRG.delVertex(vertex);
            }
            return true;
        }
        
        bool Pruning::_pruningFu()
        {
            for (const auto &item : fbReplaceVertices){
                string _fuDelete = item.first;
                for(const auto & rep_map : item.second)
                {
                    string _fuReplace = rep_map.first;
                    string _fuType = rep_map.second;
                    clog << "Replace starting with " << _fuType << " with " << _fuDelete << " " << _fuReplace << endl;

                    // _replaceFu_SW(ARIRRG, _fuType, _fuDelete, _fuReplace);
                    // _replaceFu_SW(CoreRRG, _fuType, _fuDelete, _fuReplace);
                    _replaceFu_SW(ArchRRG, _fuType, _fuDelete, _fuReplace);

                    clog << "finished _replaceFu" << endl;
                }
            }

            for (const auto &item : fbDeleteVertices){
                clog << "Delete starting with " << item << endl;

                // _deleteFu(ARIRRG, item);
                // _deleteFu(CoreRRG, item);
                _deleteFu(ArchRRG, item);

                clog << "finished _deleteFu" << endl;
            }

            clog << "pruning fu finished" << endl;
            return true;
        }

        bool Pruning::_replaceFu_MUX(Graph & graph, const string & _fuType, const string & _fuDelete, const string & _fuReplace)
        {
            unordered_map<string, string> lastMux; // 键：fuInports的各个值；值：键对应的前面最近的MUX。
            unordered_map<string, string> replaceMux; // 键：被替换的FU的输入端口最近的MUX；值：用于替换的FU的输入端口最近的MUX。

            vector<string> delFus; // 存放要删除的节点，包括：1. graph中，含_fuDelete但不含.in/.out的字符串；2. 被替换的FU的输入端口最近的MUX（如MUX2）。
            unordered_map<string, string> fuOutportContrast; // 键：被替换的FU的输出端口；值：用于替换的FU的输出端口。
            unordered_set<string> fuInports; // 存放被替换和用于替换的FU的所有输入端口
            unordered_set<string> delFuOutports; // 存放被替换的FU的输出端口

            for (const auto &_ : graph.vertices()) {
                if (_.first.find(_fuDelete + ".in") != _.first.npos) {
                    fuInports.insert(_.first);
                    continue;
                }

                if (_.first.find(_fuDelete + ".out") != _.first.npos) {
                    delFuOutports.insert(_.first);
                    continue;
                }

                string tmp = "block0" + _fuDelete;
                if (_.first == tmp) {
                    clog << "delFUS1 " << _.first << endl;
                    delFus.push_back(_.first);
                }

                if (_.first.find(_fuReplace + ".in") != _.first.npos) {
                    fuInports.insert(_.first);
                }
            }

            for (const auto &_ : fuInports) { 
                queue<string> traverseQueue;
                traverseQueue.push(_);
                bool findMux = false;
                while (!findMux) {
                    auto tmp_vertex = traverseQueue.front();
                    auto tmp_Edges = graph.edgesIn(tmp_vertex); 
                    for (const auto &_edge : tmp_Edges) {
                        auto tmp_fr = _edge.from();
                        if (tmp_fr.find("MUX") != tmp_fr.npos) { 
                            lastMux[_] = tmp_fr.substr(0, tmp_fr.rfind("."));
                            // clog << "find last mux: " << tmp_fr.substr(0, tmp_fr.rfind(".")) << endl;
                            findMux = true;
                            break;
                        }
                        traverseQueue.push(tmp_fr); 
                        traverseQueue.pop();
                    }
                }
            }

            for (const auto &_ : graph.vertices()) { 
                if (_.first.find(_fuDelete + ".in") != _.first.npos) { 
                    // clog << "_.first: " << _.first << endl;
                    // clog << _.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind(".")) << endl;
                    // clog << lastMux[_.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind("."))] << endl;
                    replaceMux[lastMux[_.first]] = lastMux[_.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind("."))];
                    if(lastMux[_.first].find("UNROLL") != lastMux[_.first].npos){
                        delFus.push_back(lastMux[_.first]); 
                        clog << "delFUS2 " << lastMux[_.first] << endl;
                    }
                    continue;
                }
                if (_.first.find(_fuDelete + ".out") != _.first.npos) { 
                    fuOutportContrast[_.first] = _.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind("."));
                    // clog << _.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind(".")) << endl;
                    continue;
                }
                // if (_.first.find(_fuDelete) != _.first.npos) { 
                //     delFus.push_back(_.first);
                // }
            }

            for (const auto &_delMux : replaceMux) { 
                for (const auto &_vertex : graph.vertices()) {
                    if (_vertex.first.find(_delMux.first + ".in") != _vertex.first.npos) { 
                        string tmpStr = _delMux.second + "." + "virtual_" + _vertex.first.substr(_vertex.first.rfind(".") + 1); 
                        // clog << "delMux.sec: " << _delMux.second << endl;
                        // clog << "ad ver: " << tmpStr << endl;
                        auto _verAttr = _vertex.second.attrs();
                        _verAttr["device"] = Attribute("MUX.virtual_" + _vertex.first.substr(_vertex.first.find(".in") + 1));
                        // for (const auto & _attr : _verAttr)
                        // {
                        //     clog << _attr.first << ": " << _attr.second.getStr() << endl;
                        // }
                        Vertex tmpVertex(tmpStr, _verAttr);
                        graph.addVertex(tmpVertex); 
                        Edge tmp_edge(tmpStr, _delMux.second + ".out0");
                        graph.addEdge(tmp_edge); 
                        clog << "ad v_in to mux edge: " << tmpStr << "->" << _delMux.second + ".out0" << endl; 
                        auto _tmpEdgesIn = graph.edgesIn(_vertex.first);
                        for (const auto &_ : _tmpEdgesIn) {
                            auto tmp_fr = _.from();
                            Edge tmp_edge(tmp_fr, tmpStr);
                            graph.addEdge(tmp_edge);  
                            clog << "ad source to v_in edge: " << tmp_fr << "->" << tmpStr << endl; 
                        }
                    }
                }
            }

            for (const auto &_fuOut : fuOutportContrast) { 
                // clog << "fu_out: " << _fuOut << endl;
                for (const auto &_edge : graph.edgesOut(_fuOut.first)) { 
                    string tmp_To = _edge.to();
                    Edge tmp_Edge(_fuOut.second, tmp_To);
                    graph.addEdge(tmp_Edge); 
                    clog << " ad edge: " << _fuOut.second + "->" + tmp_To << endl; 
                }
            }

            for (const auto &_del_fu : delFus) { 
                graph.deleteFu(_del_fu);
            }

            clog << "replace fu: " << _fuDelete << " -> " << _fuReplace << " finished! " << endl;

            return true;
        }

        bool Pruning::_replaceFu_SW(Graph & graph, const string & _fuType, const string & _fuDelete, const string & _fuReplace)
        {
            unordered_map<string, string> lastMux; // 键：fuInports的各个值；值：键对应的前面最近的""SW""(不带输入输出端口)。
            unordered_map<string, string> replaceMux; // 键：被替换的FU的输入端口最近的""SW""；值：用于替换的FU的输入端口最近的""SW""。

            vector<string> delFus; // 存放要删除的节点，包括：1. graph中，含_fuDelete但不含.in/.out的字符串；2. 被替换的FU的输入端口最近的""SW""。
            unordered_map<string, string> fuOutportContrast; // 键：被替换的FU的输出端口；值：用于替换的FU的输出端口。
            unordered_set<string> fuInports; // 存放被替换和用于替换的FU的所有输入端口
            unordered_set<string> delFuOutports; // 存放被替换的FU的输出端口

            for (const auto &_ : graph.vertices()) {
                if (_.first.find(_fuDelete + ".in") != _.first.npos) {
                    fuInports.insert(_.first);
                    continue;
                }

                if (_.first.find(_fuDelete + ".out") != _.first.npos) {
                    delFuOutports.insert(_.first);
                    continue;
                }

                string tmp = "block0." + _fuDelete;
                if (_.first == tmp) {
                    clog << "delFUS1 " << _.first << endl;
                    delFus.push_back(_.first);
                }

                if (_.first.find(_fuReplace + ".in") != _.first.npos) {
                    fuInports.insert(_.first);
                }
            }

            for (const auto &_ : fuInports) { 
                queue<string> traverseQueue;
                traverseQueue.push(_);
                bool findMux = false;
                while (!findMux) {
                    auto tmp_vertex = traverseQueue.front();
                    auto tmp_Edges = graph.edgesIn(tmp_vertex); 
                    for (const auto &_edge : tmp_Edges) {
                        auto tmp_fr = _edge.from();
                        if (tmp_fr.find("SW") != tmp_fr.npos) { 
                            lastMux[_] = tmp_fr.substr(0, tmp_fr.rfind("."));
                            // clog << "find last mux: " << tmp_fr.substr(0, tmp_fr.rfind(".")) << endl;
                            findMux = true;
                            break;
                        }
                        traverseQueue.push(tmp_fr); 
                        traverseQueue.pop();
                    }
                }
            }

            for (const auto &_ : graph.vertices()) { 
                if (_.first.find(_fuDelete + ".in") != _.first.npos) { 
                    // clog << "_.first: " << _.first << endl;
                    // clog << _.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind(".")) << endl;
                    // clog << lastMux[_.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind("."))] << endl;
                    replaceMux[lastMux[_.first]] = lastMux[_.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind("."))];
                    if(lastMux[_.first].find("UNROLL") != lastMux[_.first].npos){
                        delFus.push_back(lastMux[_.first]); 
                        clog << "delFUS2 " << lastMux[_.first] << endl;
                    }
                    continue;
                }
                if (_.first.find(_fuDelete + ".out") != _.first.npos) { 
                    fuOutportContrast[_.first] = _.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind("."));
                    // clog << _.first.substr(0, _.first.find(_fuDelete)) + _fuReplace + _.first.substr(_.first.rfind(".")) << endl;
                    continue;
                }
                // if (_.first.find(_fuDelete) != _.first.npos) { 
                //     delFus.push_back(_.first);
                // }
            }

            for (const auto &_delMux : replaceMux) { 
                for (const auto &_vertex : graph.vertices()) {
                    if (_vertex.first.find(_delMux.first + ".in") != _vertex.first.npos) { 
                        string tmpStr = _delMux.second + "." + "virtual_" + _vertex.first.substr(_vertex.first.rfind(".") + 1); 
                        // clog << "delMux.sec: " << _delMux.second << endl;
                        // clog << "ad ver: " << tmpStr << endl;
                        auto _verAttr = _vertex.second.attrs();
                        _verAttr["device"] = Attribute("SW.virtual_" + _vertex.first.substr(_vertex.first.find(".in") + 1));
                        // for (const auto & _attr : _verAttr)
                        // {
                        //     clog << _attr.first << ": " << _attr.second.getStr() << endl;
                        // }
                        Vertex tmpVertex(tmpStr, _verAttr);
                        graph.addVertex(tmpVertex); 
                        Edge tmp_edge(tmpStr, _delMux.second + ".out0");
                        graph.addEdge(tmp_edge); 
                        clog << "ad v_in to mux edge: " << tmpStr << "->" << _delMux.second + ".out0" << endl; 
                        auto _tmpEdgesIn = graph.edgesIn(_vertex.first);
                        for (const auto &_ : _tmpEdgesIn) {
                            auto tmp_fr = _.from();
                            Edge tmp_edge(tmp_fr, tmpStr);
                            graph.addEdge(tmp_edge);  
                            clog << "ad source to v_in edge: " << tmp_fr << "->" << tmpStr << endl; 
                        }
                    }
                }
            }

            for (const auto &_fuOut : fuOutportContrast) { 
                // clog << "fu_out: " << _fuOut << endl;
                for (const auto &_edge : graph.edgesOut(_fuOut.first)) { 
                    string tmp_To = _edge.to();
                    Edge tmp_Edge(_fuOut.second, tmp_To);
                    graph.addEdge(tmp_Edge); 
                    clog << " ad edge: " << _fuOut.second + "->" + tmp_To << endl; 
                }
            }

            for (const auto &_del_fu : delFus) { 
                graph.deleteFu(_del_fu);
            }

            clog << "replace fu: " << _fuDelete << " -> " << _fuReplace << " finished! " << endl;

            return true;
        }

        bool Pruning::_deleteFu(Graph & graph, const string & _fuDelete)
        {
            graph.deleteFu(_fuDelete);
            return true;
        }

        const bool Pruning::dumpRRG(const string &archPath, const string & corePath, const string & ariPath) const
        {
            ArchRRG.dumptoRRG(archPath);
            // CoreRRG.dumptoRRG(corePath);
            // ARIRRG.dumptoRRG(ariPath);
            return true;
        }

        bool filtFunc0(const Vertex & vertex)
        {
            const string &name = vertex.name();
            const string &type = vertex.getAttr("type").getStr(); 
            const string &device = vertex.getAttr("device").getStr();
            string devicePrefix = getPrefix(device);
            if (type == "__ELEMENT_INPUT_PORT__" || type == "__ELEMENT_OUTPUT_PORT__"){
                return true;
            }
            // cerr << vertex.name() << ": false" << endl;
            return false;
        }

        const bool Pruning::evaluation() const
        {
            // evaluation 1 : MUX Port number
            int muxCount0 = 0, symCount0 = 0, muxPortCount0 = 0, connectsPortCount0 = 0;
            int muxCount1 = 0, symCount1 = 0, muxPortCount1 = 0, connectsPortCount1 = 0;
            for (const auto & _vertex : ArchRRG_backup.vertices())
            {
                string name = _vertex.second.name();
                string type = _vertex.second.getAttr("type").getStr();
                string device = _vertex.second.getAttr("device").getStr();
                if((name.find("SW") != name.npos) && (name.find("MUX") != name.npos) && (name.find("out") != name.npos)){
                    muxCount0++;
                }
                if((name.find("SW") != name.npos) && (name.find("MUX") != name.npos) && (name.find("in") != name.npos)){
                    muxPortCount0++;
                }
            }

            clog << "muxCount0:" << muxCount0 << endl;
            clog << "muxPortCount0:" << muxPortCount0 << endl;

            for (const auto &_vertex : ArchRRG.vertices()) 
            {
                string name = _vertex.second.name();
                string type = _vertex.second.getAttr("type").getStr();
                string device = _vertex.second.getAttr("device").getStr();
                if((name.find("SW") != name.npos) && (name.find("MUX") != name.npos) && (name.find("out") != name.npos)){
                    muxCount1++;
                }
                if((name.find("SW") != name.npos) && (name.find("MUX") != name.npos) && (name.find("in") != name.npos)){
                    muxPortCount1++;
                }
            }

            clog << "muxCount1: " << muxCount1 << endl;
            clog << "muxPortCount1: " << muxPortCount1 << endl;

            double rms = 1 - double(muxCount1) / double(muxCount0);
            double rmps = 1 - double(muxPortCount1) / double(muxPortCount0);
            clog << "reduced mux sources: " << rms << endl;
            clog << "reduced mux port sources: " << rmps << endl;
            clog << "-----------------------------------------------------------------------" << endl;

            // evaluation 2 : number of paths in each Matrix
            clog << "number of paths in each Matrix: " << endl;
            clog << "-----------------------------------------------------------------------" << endl;
            // common
            auto func0 = [&](const Vertex &vertex)
            {
                return filtFunc0(vertex);
            };

            int SWnumber = 0;
            for (const auto &_vertex : ArchRRG_backup.vertices()){
                string name = _vertex.second.name();
                string type = _vertex.second.getAttr("type").getStr();
                string device = _vertex.second.getAttr("device").getStr();
                if((name.find("SW") != name.npos) && (name.find("MUX") == name.npos) && (name.find("in0") != name.npos) && (name.find("UNROLL") == name.npos) && (name.find("INPORT") == name.npos) && (name.find("OUTPORT") == name.npos)){
                    size_t startIdx = name.find("SW");
                    size_t endIdx = name.find(".in", startIdx);
                    string SWnum = name.substr(startIdx + 2, endIdx - (startIdx + 2));
                    SWnumber = SWnumber > stoi(SWnum) ? SWnumber : stoi(SWnum);
                }
            }

            // ArchRRG_backup, original arch
            unordered_map<string, unordered_map<string, vector<string>>> allinoutports;
            for (int i = 0; i <= SWnumber; i ++){
                allinoutports["SW" + to_string(i)]["in"] = vector<string>();
                allinoutports["SW" + to_string(i)]["out"] = vector<string>();
            }

            for (const auto &_vertex : ArchRRG_backup.vertices()){
                string name = _vertex.second.name();
                string type = _vertex.second.getAttr("type").getStr();
                string device = _vertex.second.getAttr("device").getStr();
                if((name.find("SW") != name.npos) && (name.find("MUX") == name.npos) && (name.find("in") != name.npos) && (name.find("UNROLL") == name.npos) && (name.find("INPORT") == name.npos) && (name.find("OUTPORT") == name.npos)){
                    size_t startIdx = name.find("SW");
                    size_t endIdx = name.find(".in", startIdx);
                    string SWcurrent = name.substr(startIdx, endIdx - startIdx);
                    allinoutports[SWcurrent]["in"].push_back(name);
                }
                if((name.find("SW") != name.npos) && (name.find("MUX") == name.npos) && (name.find("out") != name.npos) && (name.find("UNROLL") == name.npos) && (name.find("INPORT") == name.npos) && (name.find("OUTPORT") == name.npos)){
                    size_t startIdx = name.find("SW");
                    size_t endIdx = name.find(".out", startIdx);
                    string SWcurrent = name.substr(startIdx, endIdx - startIdx);
                    allinoutports[SWcurrent]["out"].push_back(name);
                }
            }

            int oriAllPathCount = 0;
            for (const auto &_SW : allinoutports){
                int singleMatrixPathCount = 0;
                vector<string> allIn = _SW.second.at("in");
                vector<string> allOut = _SW.second.at("out");
                for (const auto &_in : allIn){
                    for (const auto &_out : allOut){
                        auto tmp = ArchRRG_backup.findPaths(_in, _out, func0);
                        singleMatrixPathCount += tmp.size();
                    }
                }
                oriAllPathCount += singleMatrixPathCount;
                clog << "original arch, " << _SW.first << ", paths number is " << to_string(singleMatrixPathCount) << endl;
            }
            clog << "original arch, all paths number is " << to_string(oriAllPathCount) << endl;
            clog << "-----------------------------------------------------------------------" << endl;

            // ArchRRG, pruned arch
            unordered_map<string, unordered_map<string, vector<string>>> allinoutports2;
            for (int i = 0; i <= SWnumber; i ++){
                allinoutports2["SW" + to_string(i)]["in"] = vector<string>();
                allinoutports2["SW" + to_string(i)]["out"] = vector<string>();
            }

            for (const auto &_vertex : ArchRRG.vertices()){
                string name = _vertex.second.name();
                string type = _vertex.second.getAttr("type").getStr();
                string device = _vertex.second.getAttr("device").getStr();
                if((name.find("SW") != name.npos) && (name.find("MUX") == name.npos) && (name.find("in") != name.npos) && (name.find("UNROLL") == name.npos) && (name.find("INPORT") == name.npos) && (name.find("OUTPORT") == name.npos)){
                    size_t startIdx = name.find("SW");
                    size_t endIdx = name.find(".in", startIdx);
                    string SWcurrent = name.substr(startIdx, endIdx - startIdx);
                    allinoutports2[SWcurrent]["in"].push_back(name);
                }
                if((name.find("SW") != name.npos) && (name.find("MUX") == name.npos) && (name.find("out") != name.npos) && (name.find("UNROLL") == name.npos) && (name.find("INPORT") == name.npos) && (name.find("OUTPORT") == name.npos)){
                    size_t startIdx = name.find("SW");
                    size_t endIdx = name.find(".out", startIdx);
                    string SWcurrent = name.substr(startIdx, endIdx - startIdx);
                    allinoutports2[SWcurrent]["out"].push_back(name);
                }
            }

            int prunedAllPathCount = 0;
            for (const auto &_SW : allinoutports2){
                int singleMatrixPathCount = 0;
                vector<string> allIn = _SW.second.at("in");
                vector<string> allOut = _SW.second.at("out");
                for (const auto &_in : allIn){
                    for (const auto &_out : allOut){
                        auto tmp = ArchRRG.findPaths(_in, _out, func0);
                        singleMatrixPathCount += tmp.size();
                    }
                }
                prunedAllPathCount += singleMatrixPathCount;
                clog << "pruned arch, " << _SW.first << ", paths number is " << to_string(singleMatrixPathCount) << endl;
            }
            clog << "pruned arch, all paths number is " << to_string(prunedAllPathCount) << endl;
            clog << "-----------------------------------------------------------------------" << endl;

            return true;
        }



    }
}