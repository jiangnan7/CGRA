#include "../inc/pruAnalyse.h"

using namespace std;
namespace FastCGRA
{
    namespace PRUANALYSE{
    bool filtIO(const Vertex &vertex) 
    {
        const string &name = vertex.name();
        const string &type = vertex.getAttr("type").getStr(); 
        const string &device = vertex.getAttr("device").getStr(); 
        string devicePrefix = getPrefix(device);
        if (
            (type == "__MODULE_INPUT_PORT__" || type == "__MODULE_OUTPUT_PORT__") && name.find("SW") != name.npos && device.find("ANYCONN") != device.npos) { //
            return true;
        } 
        // cerr << vertex.name() << ": false" << endl;
        return false;
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

    bool PruAnalyser::analyze()
    {
        vector<string> input;
        vector<string> output;
        auto func0 = [&](const Vertex &vertex)
        {
            return filtFunc0(vertex);
        };

        auto funcSink0 = [&](const Vertex &vertex) {
            const string &type = vertex.getAttr("type").getStr();
            const string &device = vertex.getAttr("device").getStr();
            string devicePrefix = getPrefix(device);
            if ((type == "__MODULE_INPUT_PORT__" && device.find("SWITCH") != device.npos)|| (type == "__MODULE_OUTPUT_PORT__" && device.find("SWITCH") != device.npos)) {
                return true;
            } 
            return false;
        };

        for (const auto &_vertex : RRG.vertices()) {
            if (filtIO(_vertex.second)) {
                const string &type = _vertex.second.getAttr("type").getStr();
                const string &device = _vertex.second.getAttr("device").getStr();

                if (type.find("INPUT") != type.npos) {
                    input.push_back(_vertex.first);
                } else {
                    output.push_back(_vertex.first);
                }
            }
        }

        for (const auto &_input : input) {
            for (const auto &_output : output) {
                size_t dot1 = _input.find(".");
                size_t dot2 = _input.find(".", dot1 + 1);
                string mid1 = _input.substr(dot1 + 1, dot2 - dot1 - 1);
                size_t dot3 = _output.find(".");
                size_t dot4 = _output.find(".", dot1 + 1);
                string mid2 = _output.substr(dot1 + 1, dot2 - dot1 - 1);
                if(mid1 == mid2){
                    auto tmp = RRG.findPaths(_input, _output, func0);
                    if (!tmp.empty()) {
                        // clog << tmp.size() << endl;
                        int counts = 0;
                        if (linkTable.find(_input) == linkTable.end()) {
                            linkTable[_input] = unordered_map<string, vector<string>>();
                            linkTable[_input][_output] = vector<string>();
                        }
                        if (linkTable[_input].find(_output) == linkTable[_input].end()) {
                            linkTable[_input][_output] = vector<string>();
                        }
                        for (const auto &_path : tmp) {
                            // if (counts > 0) clog << _input + "->" + _output + "<" + num2str(counts) + ">" << endl;
                            string tmpStr = _input + "->" + _output + "<" + num2str(counts++) + ">";
                            linkTable[_input][_output].push_back(tmpStr);
                            pathTable[tmpStr] = vector<string>(_path);
                        }
                    }
                }
            }
        }

        return true;

    }

    bool PruAnalyser::_analyze(unordered_map<string, string> & switchInIdx, unordered_map<string, string> &switch2Fu)
    {
        Graph _RRG;
        _catchSwitchGraph(_RRG, switchInIdx, switch2Fu);

        vector<string> input;
        vector<string> output;
        auto func0 = [&](const Vertex &vertex) {
            return filtFunc0(vertex);
        };

        auto funcSink0 = [&](const Vertex &vertex) {
            const string &type = vertex.getAttr("type").getStr();
            const string &device = vertex.getAttr("device").getStr();
            string devicePrefix = getPrefix(device);
            if ((type == "__MODULE_INPUT_PORT__" && device.find("SWITCH") != device.npos) || (type == "__MODULE_OUTPUT_PORT__" && device.find("SWITCH") != device.npos)) {
                return true;
            }
            return false;
        };

        for (const auto &_vertex : _RRG.vertices()) {
            if (filtIO(_vertex.second)) {
                const string &type = _vertex.second.getAttr("type").getStr();
                const string &device = _vertex.second.getAttr("device").getStr();

                if (type.find("INPUT") != type.npos) {
                    input.push_back(_vertex.first);
                } else {
                    output.push_back(_vertex.first);
                }
            }
        }

        // clog << "input: " << input << endl;
        // clog << "output: " << output << endl;

        for (const auto &_input : input) {
            for (const auto &_output : output) {
                auto tmp = _RRG.findPaths(_input, _output, func0);
                if (!tmp.empty()) {
                    // clog << tmp.size() << endl;
                    int counts = 0;
                    if (linkTable.find(_input) == linkTable.end()) {
                        linkTable[_input] = unordered_map<string, vector<string>>();
                        linkTable[_input][_output] = vector<string>();
                    }
                    if (linkTable[_input].find(_output) == linkTable[_input].end()) {
                        linkTable[_input][_output] = vector<string>();
                    }
                    for (const auto &_path : tmp) {
                        // if (counts > 0) clog << _input + "->" + _output + "<" + num2str(counts) + ">" << endl;
                        string tmpStr = _input + "->" + _output + "<" + num2str(counts++) + ">";
                        linkTable[_input][_output].push_back(tmpStr);
                        pathTable[tmpStr] = vector<string>(_path);
                    }
                }
            }
        }

        // return true;
        clog << "linkTable: " << endl;
        clog << linkTable << endl;
        clog << "_analyze finished" << endl;
        return true;
    }

    bool PruAnalyser::Parse(const string &routesFile)
    {
        unordered_map<string, unordered_map<string, vector<vector<string>>>> _routesTable;
        unordered_map<string, unordered_map<string, unordered_set<string>>> _routesIdx;

        ifstream fin(routesFile);
        assert(fin);
        string dfgIdx = "TBD", preIdx = "TBD";
        while (!fin.eof()) {
            string line;
            getline(fin, line);
            if (line.find("benchmarks") != string::npos) {
                istringstream sin(line);
                string tmpStr;
                sin >> tmpStr;
                dfgIdx = tmpStr;
                continue;
            }
            // if (line.find(":") != string::npos && line.find("dfg") == string::npos) {
            //     istringstream sin(line);
            //     string tmpStr;
            //     sin >> tmpStr;
            //     preIdx = tmpStr.substr(0, tmpStr.find(":"));
            //     continue;
            // }

            istringstream sin(line);
            string tmpStr;

            vector<string> tmpRoute;
            sin >> tmpStr;
            while (!tmpStr.empty()) {
                tmpRoute.push_back(tmpStr);
                tmpStr.clear();
                sin >> tmpStr;
            }
            if (_routesTable.find(dfgIdx) == _routesTable.end()) {
                _routesTable[dfgIdx] = unordered_map<string, vector<vector<string>>>();
                _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
            }
            if (_routesTable[dfgIdx].find(preIdx) == _routesTable[dfgIdx].end()) {
                _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
            }

            _routesTable[dfgIdx][preIdx].push_back(vector<string>(tmpRoute));
        }

        fin.close();

        for (const auto &_dfgItem : _routesTable) {
            if (_routesIdx.find(_dfgItem.first) == _routesIdx.end()) {
                _routesIdx[_dfgItem.first] = unordered_map<string, unordered_set<string>>();
            }
            // clog << _dfgItem.first << endl;
            for (const auto &_preItem : _dfgItem.second) {
                if (_routesIdx[_dfgItem.first].find(_preItem.first) == _routesIdx[_dfgItem.first].end()) {
                    _routesIdx[_dfgItem.first][_preItem.first] = unordered_set<string>();
                }
                // clog << _preItem.first << endl;
                vector<string> _tmp;
                for (const auto &_tmpPath : _preItem.second) {
                    if (_tmpPath.empty())
                        continue;
                    const string _fr = _tmpPath.front();
                    const string _to = _tmpPath.back();
                    // clog << _fr << " -> " << _to << endl;
                    assert(linkTable.find(_fr) != linkTable.end());
                    if (linkTable[_fr].find(_to) == linkTable[_fr].end()) {
                        cerr << "wrong link: " << _fr << "->" << _to << endl;
                        return 0;
                    }
                    // if (linkTable[_fr][_to].size() == 1) {
                    //     _routesIdx[_dfgItem.first][_preItem.first].insert(_fr + "->" + _to + "<0>");
                    //     continue;
                    // }
                    _tmp.clear();
                    _tmp.assign(_tmpPath.begin() + 1, _tmpPath.end() - 1);
                    for (const auto &_idx : linkTable[_fr][_to]) {
                        // clog << _fr << "->" << _to << endl;
                        if (pathTable[_idx] == _tmp) {
                            _routesIdx[_dfgItem.first][_preItem.first].insert(_idx);
                            // clog << _idx << endl;
                        }
                    }
                    // _routesIdx[_dfgItem.first][_preItem.first].insert(_tmpPath.front() + "->" + _tmpPath.back());
                }
            }
        }

        this->routesTable = move(_routesTable);
        this->routesIdx = move(_routesIdx);

        return true;
    }

    // bool PruAnalyser::Parse(string && routesFile)
    // {
    //     unordered_map<string, unordered_map<string, vector<vector<string>>>> _routesTable;
    //     unordered_map<string, unordered_map<string, unordered_set<string>>> _routesIdx;
    //     ifstream fin(routesFile);
    //     assert(fin);
    //     string dfgIdx = "TBD", preIdx = "TBD";
    //     while (!fin.eof()) {
    //         string line;
    //         getline(fin, line);
    //         if (line.find("dfg") != string::npos) {
    //             istringstream sin(line);
    //             string tmpStr;
    //             sin >> tmpStr;
    //             dfgIdx = tmpStr.substr(0, tmpStr.find(":"));
    //             continue;
    //         }
    //         if (line.find(":") != string::npos && line.find("dfg") == string::npos) {
    //             istringstream sin(line);
    //             string tmpStr;
    //             sin >> tmpStr;
    //             preIdx = tmpStr.substr(0, tmpStr.find(":"));
    //             continue;
    //         }

    //         istringstream sin(line);
    //         string tmpStr;

    //         vector<string> tmpRoute;
    //         sin >> tmpStr;
    //         while (!tmpStr.empty()) {
    //             tmpRoute.push_back(tmpStr);
    //             tmpStr.clear();
    //             sin >> tmpStr;
    //         }
    //         if (_routesTable.find(dfgIdx) == _routesTable.end()) {
    //             _routesTable[dfgIdx] = unordered_map<string, vector<vector<string>>>();
    //             _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
    //         }
    //         if (_routesTable[dfgIdx].find(preIdx) == _routesTable[dfgIdx].end()) {
    //             _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
    //         }

    //         if (tmpRoute.empty()) continue;
    //         _routesTable[dfgIdx][preIdx].push_back(vector<string>(tmpRoute));

    //     }
    //     fin.close();

    //     for (const auto &_dfgItem : _routesTable) {
    //         if (_routesIdx.find(_dfgItem.first) == _routesIdx.end()) {
    //             _routesIdx[_dfgItem.first] = unordered_map<string, unordered_set<string>>();
    //         }
    //         // clog << _dfgItem.first << endl;
    //         for (const auto &_preItem : _dfgItem.second) {
    //             if (_routesIdx[_dfgItem.first].find(_preItem.first) == _routesIdx[_dfgItem.first].end()) {
    //                 _routesIdx[_dfgItem.first][_preItem.first] = unordered_set<string>();
    //             }
    //             // clog << _preItem.first << endl;
    //             vector<string> _tmp;
    //             for (const auto &_tmpPath : _preItem.second) {
    //                 if (_tmpPath.empty())
    //                     continue;
    //                 const string _fr = _tmpPath.front();
    //                 const string _to = _tmpPath.back();
    //                 // clog << _fr << " -> " << _to << endl;
    //                 assert(linkTable.find(_fr) != linkTable.end());
    //                 if (linkTable[_fr].find(_to) == linkTable[_fr].end()) {
    //                     cerr << "wrong link: " << _fr << "->" << _to << endl;
    //                     return 0;
    //                 }
    //                 if (linkTable[_fr][_to].size() == 1) {
    //                     _routesIdx[_dfgItem.first][_preItem.first].insert(_fr + "->" + _to + "<0>");
    //                     continue;
    //                 }
    //                 _tmp.clear();
    //                 _tmp.assign(_tmpPath.begin() + 1, _tmpPath.end() - 1);
    //                 for (const auto &_idx : linkTable[_fr][_to]) {
    //                     // clog << _fr << "->" << _to << endl;
    //                     if (pathTable[_idx] == _tmp) {
    //                         _routesIdx[_dfgItem.first][_preItem.first].insert(_idx);
    //                         // clog << _idx << endl;
    //                     }
    //                 }
    //                 // _routesIdx[_dfgItem.first][_preItem.first].insert(_tmpPath.front() + "->" + _tmpPath.back());
    //             }
    //         }
    //     }

    //     this->routesTable = move(_routesTable);
    //     this->routesIdx = move(_routesIdx);

    //     return true;
    // }

    bool PruAnalyser::Parse(const string &routesFile, unordered_map<string, string> &switchInIdx, unordered_map<string, string> &switch2Fu){
        unordered_map<string, unordered_map<string, vector<vector<string>>>> _routesTable;
        unordered_map<string, unordered_map<string, unordered_set<string>>> _routesIdx;
        ifstream fin(routesFile);
        assert(fin);
        string dfgIdx = "TBD", preIdx = "TBD";
        while (!fin.eof()) {
            string line;
            getline(fin, line);
            if (line.find("dfg") != string::npos) {
                istringstream sin(line);
                string tmpStr;
                sin >> tmpStr;
                dfgIdx = tmpStr.substr(0, tmpStr.find(":"));
                continue;
            }
            if (line.find(":") != string::npos && line.find("dfg") == string::npos) {
                istringstream sin(line);
                string tmpStr;
                sin >> tmpStr;
                preIdx = tmpStr.substr(0, tmpStr.find(":"));
                continue;
            }

            istringstream sin(line);
            string tmpStr;

            vector<string> tmpRoute;
            sin >> tmpStr;
            while (!tmpStr.empty()) {
                tmpRoute.push_back(tmpStr);
                tmpStr.clear();
                sin >> tmpStr;
            }
            if (_routesTable.find(dfgIdx) == _routesTable.end()) {
                _routesTable[dfgIdx] = unordered_map<string, vector<vector<string>>>();
                _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
            }
            if (_routesTable[dfgIdx].find(preIdx) == _routesTable[dfgIdx].end()) {
                _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
            }

            if (tmpRoute.empty())
                continue;
            _routesTable[dfgIdx][preIdx].push_back(vector<string>(tmpRoute));
        }
        fin.close();

        for (const auto &_dfgItem : _routesTable) {
            if (_routesIdx.find(_dfgItem.first) == _routesIdx.end()) {
                _routesIdx[_dfgItem.first] = unordered_map<string, unordered_set<string>>();
            }
            // clog << _dfgItem.first << endl;
            for (const auto &_preItem : _dfgItem.second) {
                if (_routesIdx[_dfgItem.first].find(_preItem.first) == _routesIdx[_dfgItem.first].end()) {
                    _routesIdx[_dfgItem.first][_preItem.first] = unordered_set<string>();
                }
                // clog << _preItem.first << endl;
                // vector<string> _tmp;
                for (const auto &_tmpPath : _preItem.second) {
                    if (_tmpPath.empty())
                        continue;
                    const string _fr = switchInIdx[_tmpPath.front()];
                    const string _to = switch2Fu[_tmpPath.back()];
                    // clog << _tmpPath.front() << " -> " << _tmpPath.back() << endl;
                    // clog << _fr << " -> " << _to << endl;
                    assert(linkTable.find(_fr) != linkTable.end());
                    if (linkTable[_fr].find(_to) == linkTable[_fr].end()) {
                        cerr << "wrong link: " << _fr << "->" << _to << endl;
                        return 0;
                    }
                    if (linkTable[_fr][_to].size() == 1) {
                        _routesIdx[_dfgItem.first][_preItem.first].insert(_fr + "->" + _to + "<0>");
                        continue;
                    }
                    for (const auto &_idx : linkTable[_fr][_to]) {
                        // clog << _fr << "->" << _to << endl;
                        if (pathTable[_idx] == _tmpPath) {
                            _routesIdx[_dfgItem.first][_preItem.first].insert(_idx);
                            // clog << _idx << endl;
                        }
                    }
                }
            }
        }

        this->routesTable = move(_routesTable);
        this->routesIdx = move(_routesIdx);

        clog << "switchInIdx:" << switchInIdx << endl;
        clog << "switch2Fu:" << switch2Fu << endl;

        clog << "PruAnalyser parser finished" << endl;
        return true;
    }

    bool PruAnalyser::Parse(string &&routesFile, unordered_map<string, string> &switchInIdx, unordered_map<string, string> &switch2Fu){
        unordered_map<string, unordered_map<string, vector<vector<string>>>> _routesTable;
        unordered_map<string, unordered_map<string, unordered_set<string>>> _routesIdx;
        ifstream fin(routesFile);
        assert(fin);
        string dfgIdx = "TBD", preIdx = "TBD";
        while (!fin.eof()) {
            string line;
            getline(fin, line);
            if (line.find("dfg") != string::npos) {
                istringstream sin(line);
                string tmpStr;
                sin >> tmpStr;
                dfgIdx = tmpStr.substr(0, tmpStr.find(":"));
                continue;
            }
            if (line.find(":") != string::npos && line.find("dfg") == string::npos) {
                istringstream sin(line);
                string tmpStr;
                sin >> tmpStr;
                preIdx = tmpStr.substr(0, tmpStr.find(":"));
                continue;
            }

            istringstream sin(line);
            string tmpStr;

            vector<string> tmpRoute;
            sin >> tmpStr;
            while (!tmpStr.empty()) {
                tmpRoute.push_back(tmpStr);
                tmpStr.clear();
                sin >> tmpStr;
            }
            if (_routesTable.find(dfgIdx) == _routesTable.end()) {
                _routesTable[dfgIdx] = unordered_map<string, vector<vector<string>>>();
                _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
            }
            if (_routesTable[dfgIdx].find(preIdx) == _routesTable[dfgIdx].end()) {
                _routesTable[dfgIdx][preIdx] = vector<vector<string>>();
            }

            if (tmpRoute.empty())
                continue;
            _routesTable[dfgIdx][preIdx].push_back(vector<string>(tmpRoute));
        }
        fin.close();

        for (const auto &_dfgItem : _routesTable) {
            if (_routesIdx.find(_dfgItem.first) == _routesIdx.end()) {
                _routesIdx[_dfgItem.first] = unordered_map<string, unordered_set<string>>();
            }
            // clog << _dfgItem.first << endl;
            for (const auto &_preItem : _dfgItem.second) {
                if (_routesIdx[_dfgItem.first].find(_preItem.first) == _routesIdx[_dfgItem.first].end()) {
                    _routesIdx[_dfgItem.first][_preItem.first] = unordered_set<string>();
                }
                // clog << _preItem.first << endl;
                // vector<string> _tmp;
                for (const auto &_tmpPath : _preItem.second) {
                    if (_tmpPath.empty())
                        continue;
                    const string _fr = switchInIdx[_tmpPath.front()];
                    const string _to = switch2Fu[_tmpPath.back()];
                    // clog << _tmpPath.front() << " -> " << _tmpPath.back() << endl;
                    // clog << _fr << " -> " << _to << endl;
                    assert(linkTable.find(_fr) != linkTable.end());
                    if (linkTable[_fr].find(_to) == linkTable[_fr].end()) {
                        cerr << "wrong link: " << _fr << "->" << _to << endl;
                        return 0;
                    }
                    if (linkTable[_fr][_to].size() == 1) {
                        _routesIdx[_dfgItem.first][_preItem.first].insert(_fr + "->" + _to + "<0>");
                        continue;
                    }
                    // _tmp.clear();
                    // _tmp.assign(_tmpPath.begin() + 1, _tmpPath.end() - 1);
                    for (const auto &_idx : linkTable[_fr][_to]) {
                        // clog << _fr << "->" << _to << endl;
                        if (pathTable[_idx] == _tmpPath) {
                            _routesIdx[_dfgItem.first][_preItem.first].insert(_idx);
                            // clog << _idx << endl;
                        }
                    }
                    // _routesIdx[_dfgItem.first][_preItem.first].insert(_tmpPath.front() + "->" + _tmpPath.back());
                }
            }
        }

        this->routesTable = move(_routesTable);
        this->routesIdx = move(_routesIdx);

        clog << "switchInIdx:" << switchInIdx << endl;
        clog << "switch2Fu:" << switch2Fu << endl;
        clog << "PruAnalyser parser finished" << endl;
        return true;
    }

    unordered_map<string, vector<string>> PruAnalyser::DeletePathAnnealingVersion0()
    {
        unordered_map<string, unordered_set<string>> _muxOccupation;
        unordered_map<string, unordered_set<string>> _clashTable;
        unordered_map<string, unordered_set<string>> _reservedClashPathTable;
        unordered_map<string, unordered_map<string, unordered_set<string>>> _subPathTable;
        unordered_map<string, vector<string>> _pathToDelete(pathTable);

        auto tmpF = [&](string from, string to) { 
            // clog << "from: " << from << ", to: " << to << "\n";
            assert(linkTable.find(from) != linkTable.end() && linkTable.find(from)->second.find(to) != linkTable.find(from)->second.end());
            return linkTable.find(from)->second.find(to)->second;
        };

        for (const auto &_path : pathTable) { 
            string _idx = _path.first;
            for (const auto &_ : _path.second) {
                string name = RRG.vertex(_).name();
                // string tmpType = RRG.vertex(_).getAttr("type").getStr();
                if (name.find("out0") == name.npos)
                    continue;
                if (_muxOccupation.find(_) == _muxOccupation.end()) {
                    _muxOccupation[_] = unordered_set<string>();
                }
                _muxOccupation[_].insert(_idx); 
            }
        }

        for (const auto &_idx : _muxOccupation) { 
            string _mux = _idx.first;
            for (const auto &_path : _idx.second) {
                if (_clashTable.find(_path) == _clashTable.end()) { 
                    _clashTable[_path] = unordered_set<string>();
                }
                string tmpStr = _path.substr(0, _path.find("<")); 
                for (const auto &_tmp : _idx.second) {
                    if (_tmp == _path || _tmp.find(tmpStr) != _tmp.npos) 
                        continue;
                    _clashTable[_path].insert(_tmp);
                }
            }
        }

        for (const auto &_dfgItem : routesIdx) { 
            string _dfg = _dfgItem.first;
            for (const auto &_preItem : _dfgItem.second) {
                string _pre = _preItem.first;
                for (const auto &_path : _preItem.second) {
                    string _fr = _path.substr(0, _path.find("->")); 
                    string _to = _path.substr(_path.find("->") + 2, _path.find("<") - _path.find("->") - 2); 
                    // clog << "_fr " << _fr << " _to " << _to << endl;
                    vector<string> _tmp = tmpF(_fr, _to); 
                    if (_tmp.size() < 2) { 
                        // _reservedPathTable.insert(_path);
                        continue;
                    }
                    // unordered_map<string, unordered_map<string, unordered_set<string>>> _subPathTable;
                    for (const auto &_ : _tmp) {
                        _subPathTable[_dfg][_pre].insert(_);
                    }
                }
            }
        }

        for (const auto &_dfgItem : routesIdx) {
            string _dfg = _dfgItem.first;
            for (const auto &_preItem : _dfgItem.second) {
                string _pre = _preItem.first;
                for (const auto &_path : _preItem.second) {
                    // unordered_map<string, unordered_set<string>> _reservedClashPathTable; 
                    if (_reservedClashPathTable.find(_path) == _reservedClashPathTable.end()) { 
                        _reservedClashPathTable[_path] = unordered_set<string>();
                    }
                    if (_subPathTable[_dfg][_pre].empty() || _clashTable.find(_path) == _clashTable.end() || _clashTable.find(_path)->second.size() == 0)
                        continue;
                    for (const auto &_ : _subPathTable[_dfg][_pre]) { 
                        if (_clashTable[_path].find(_) != _clashTable[_path].end()) { 
                            string _fr = _.substr(0, _.find("->")); 
                            string _to = _.substr(_.find("->") + 2, _.find("<") - _.find("->") - 2); 
                            // clog << "_fr " << _fr << " 2_to " << _to << endl;
                            auto tmp = tmpF(_fr, _to); 
                            for (const auto &_reserved : tmp) {
                                if (routesIdx[_dfg][_pre].find(_reserved) != routesIdx[_dfg][_pre].end()) {
                                    _reservedClashPathTable[_path].insert(_);
                                }
                            }
                        }
                    }
                }
            }
        }

        unordered_set<string> resPaths;
        unordered_set<string> dfgPaths;
        unordered_map<string, unordered_set<string>> resPathIndex;

        auto initialPathTable = [&]()
        {
            for (const auto & _it : _reservedClashPathTable) { 
                resPaths.insert(_it.first);
                dfgPaths.insert(_it.first);
                string pathPrefix = _it.first.substr(0, _it.first.find("<")); 
                string pathidx = _it.first.substr(_it.first.find("<"), (_it.first.length() - _it.first.find("<"))); 

                if(resPathIndex.find(pathPrefix) == resPathIndex.end()) 
                {
                    resPathIndex[pathPrefix] = unordered_set<string>();
                }
                resPathIndex[pathPrefix].insert(pathidx);

                for (const auto & _ : _it.second)
                {
                    resPaths.insert(_);
                    dfgPaths.insert(_);
                    int _pos = _.find("<");
                    string _prefix = _.substr(0, _pos); 
                    string _idx = _.substr(_pos, _.length() - _pos); 

                    if (resPathIndex.find(_prefix) == resPathIndex.end()) 
                    {
                        resPathIndex[_prefix] = unordered_set<string>();
                    }
                    resPathIndex[_prefix].insert(_idx);
                }
            }

            for (const auto & fr : linkTable) 
            {
                string _fr = fr.first;
                for (const auto & to : fr.second)
                {
                    string _to = to.first;
                    string linkIdx = _fr + "->" + _to;
                    if (resPathIndex.find(linkIdx) == resPathIndex.end()) 
                    {
                        resPaths.insert(*to.second.begin()); 
                        resPathIndex[linkIdx] = unordered_set<string>();
                        resPathIndex[linkIdx].insert("<0>");
                    }
                }
            }

        };

        auto evaluate = [&](const unordered_set<string> & tmp)
        {
            ssize_t res = 0;
            unordered_map<string, int> tmpPathScore;
            for (const auto & _ : tmp)
            {
                if (dfgPaths.find(_) != dfgPaths.end()) 
                {
                    tmpPathScore[_] = -1024;
                    continue;
                }
                if (_clashTable.find(_) == _clashTable.end() || _clashTable.find(_)->second.empty()) 
                {
                    tmpPathScore[_] = 0;
                    continue;
                }
                tmpPathScore[_] = 0; 
                for (const auto & _clashPath : _clashTable[_])
                {
                    if (tmp.find(_clashPath) != tmp.end())
                    {
                        tmpPathScore[_] += 1;
                    }
                }
            }

            for (const auto & _ : tmpPathScore)
            {
                // clog << _.first << " " << _.second << " " << _.second << endl;
                res += _.second;
                // clog << "nowRes: " << res << endl;
            }

            return res;
        };

        auto update = [&](const unordered_set<string> & tmp, const string & idx)
        {
            unordered_set<string> res(tmp);
            string tmpPathPrefix = idx.substr(0, idx.find("<"));
            string tmpPathIdx = idx.substr(idx.find("<"), idx.length() - idx.find("<"));
            string backIdx;

            if (resPathIndex.find(tmpPathPrefix) == resPathIndex.end())
            {
                clog << "ERROR: Unable to find path " << tmpPathPrefix << " in resPathIndex" << endl;
                assert(false);
            }

            for (const auto & _ : resPathIndex[tmpPathPrefix])
            {
                if (_ == tmpPathIdx) continue;
                backIdx = _;
                break;
            }
            
            if (!backIdx.empty())
            {
                string backPath = tmpPathPrefix + backIdx;
                res.erase(backPath);
                res.insert(idx);
                resPathIndex[tmpPathPrefix].erase(backIdx);
                resPathIndex[tmpPathPrefix].insert(tmpPathIdx);
            }
            return res;

        };

        initialPathTable(); 
        ssize_t nowScore = evaluate(resPaths);
        clog << "NowScore: " << nowScore << "\n";
        size_t countIters = 0;
        double temperature = 100;
        ssize_t finalCount = 0;
        double finalScore = 0;
        unordered_set<string> backPaths(resPaths);
        unordered_map<string, unordered_set<string>> backPathIndex(resPathIndex);
        vector<string> pathSpace;
        for (const auto & _ : pathTable) 
        {
            pathSpace.push_back(_.first);
        }

        while(countIters < 2048 || temperature == 1)
        {

            if (countIters % 16 == 0) 
            {
                temperature /= 1.05;
            }
            size_t jdx = rand() % pathSpace.size();  

            string randPath = pathSpace[jdx]; 
            unordered_set<string> newRes = update(backPaths, randPath);  
            ssize_t newScore = evaluate(newRes); 

            double delta = nowScore - newScore; 
            double probAc = exp(delta / temperature);
            double prob = static_cast<double>(rand()) / static_cast<double>(RAND_MAX); 
            bool accepted = false;

            if (delta > 0.0)
            {
                accepted = true;
                clog << "new path annealing: Accepted. Score: " << newScore << " vs " << nowScore << endl;
            }
            else if (prob < probAc)
            {
                accepted = true;
                clog << "new path annealing: Accepted. prob: " << prob << "/" << probAc << " this Score: " << newScore << " last Score: " << nowScore << endl;
            }
            else 
            {
                accepted = false;
                clog << "new path annealing: Rejected. prob: " << prob << "/" << probAc << " this Score: " << newScore << " last Score: " << newScore << endl;
            }

            if (accepted) 
            {
                nowScore = newScore; 
                backPaths = newRes; 
                backPathIndex = resPathIndex; 
            }
            else
            {
                resPathIndex = backPathIndex; 
            }
            countIters++;

        }

        resPaths = backPaths;  
        for (const auto & _ : resPaths)
        {
            _pathToDelete.erase(_);
        }
        return _pathToDelete; 
    }

    unordered_set<string> PruAnalyser::DeleteVerticesEdgesIn(const unordered_map<string, vector<string>> &pathToDelete)
    {
        // unordered_map<string, vector<string>> _pathToDelete(pathToDelete);
        // unordered_set<string> _wasted;
        unordered_set<string> _reservedVertices; 
        unordered_set<string> _results; 

        auto tmpF = [&](string from, string to) { 
            assert(linkTable.find(from) != linkTable.end() && linkTable.find(from)->second.find(to) != linkTable.find(from)->second.end());
            return linkTable.find(from)->second.find(to)->second;
        };

        for (const auto & _path : pathTable) { 
            string _idx = _path.first;
            if (pathToDelete.find(_idx) == pathToDelete.end()) 
            {
                for (const auto & _ : pathTable[_idx]) 
                {
                    _reservedVertices.insert(_);
                }
            }
        }

        // unordered_set<string> tmpVertices
        for (const auto & _path : pathToDelete) 
        {
            // if (_wasted.find(_path) != _wasted.end()) continue;
            for (const auto & _ : _path.second)
            {
                if (_reservedVertices.find(_) != _reservedVertices.end())
                {
                    continue;
                }
                _results.insert(_);
                break;
            }
        }
        return _results;
    }

    const size_t PruAnalyser::getRoutesCounts() const
    {
        size_t result = 0;
        for (const auto & fir : routesIdx)
        {
            string dfg = fir.first;
            for (const auto & sec : fir.second)
            {
                string pre = sec.first;
                result += sec.second.size();
            }
        }
        return result;
    }

    bool PruAnalyser::_catchSwitchGraph(Graph &_switchGraph, unordered_map<string, string> &switchInIdx, unordered_map<string, string> &switch2Fu)
    {
        unordered_map<string, string> fu2Switch;
        int inSize = 0; 
        int outSize = 0;

        for (const auto &_vertex : RRG.vertices()) {
            if (_vertex.first.find("Matrix") != _vertex.first.npos) {
                _switchGraph.addVertex(_vertex.second);
                // auto _edges = graph.edgesOut(_vertex.first);
                for (const auto &_edge : RRG.edgesOut(_vertex.first)) {
                    string fr = _edge.from();
                    string to = _edge.to();
                    if (to.find("Matrix") == to.npos) {
                        if (switch2Fu.find(fr) == switch2Fu.end()) {
                            Vertex tmpVertex("ARCH_SWITCH.out" + num2str(outSize));
                            Attribute _type("__MODULE_OUTPUT_PORT__");
                            Attribute _device("ARCH_SWITCH.out" + num2str(outSize));
                            tmpVertex.setAttr("type", _type);
                            tmpVertex.setAttr("device", _device);
                            _switchGraph.addVertex(tmpVertex);
                            switch2Fu[fr] = "ARCH_SWITCH.out" + num2str(outSize);
                            // clog << "switch2Fu: fr: " << fr << " " << "ARCH_SWITCH.out" + num2str(outSize) << endl;
                            _switchGraph.addEdge(Edge(fr, switch2Fu[fr]));
                            outSize++;
                        }
                    }
                }
                for (const auto &_edge : RRG.edgesIn(_vertex.first)) {
                    string fr = _edge.from();
                    string to = _edge.to();

                    if (fr.find("Matrix") == fr.npos) {
                        if (fu2Switch.find(fr) == fu2Switch.end()) {
                            Vertex tmpVertex("ARCH_SWITCH.in" + num2str(inSize));
                            Attribute _type("__MODULE_INPUT_PORT__");
                            Attribute _device("ARCH_SWITCH.in" + num2str(inSize));
                            tmpVertex.setAttr("type", _type);
                            tmpVertex.setAttr("device", _device);
                            _switchGraph.addVertex(tmpVertex);
                            fu2Switch[fr] = "ARCH_SWITCH.in" + num2str(inSize);
                            switchInIdx[to] = "ARCH_SWITCH.in" + num2str(inSize);
                            // clog << "switchInIdx: to: " << to << "  " << "ARCH_SWITCH.in" + num2str(inSize) << endl;
                            _switchGraph.addEdge(Edge(fu2Switch[fr], to));
                            inSize++;
                        } else {
                            clog << "Warning: fu to many Matrix: " << fr << endl;
                            _switchGraph.addEdge(Edge(fu2Switch[fr], to));
                            switchInIdx[to] = fu2Switch[fr];
                            // clog << "switchInIdx: to: " << to << "  " << fu2Switch[fr] << endl;
                        }
                    }
                }
            }
        }

        clog << "insize: " << inSize << endl;
        clog << "outsize: " << outSize << endl;

        for (const auto &_vertex : RRG.vertices()) {
            if (_vertex.first.find("Matrix") != _vertex.first.npos) {
                for (const auto &_edge : RRG.edgesOut(_vertex.first)) {
                    string fr = _edge.from();
                    string to = _edge.to();

                    if (to.find("Matrix") != to.npos) {
                        _switchGraph.addEdge(Edge(fr, to));
                    }
                }
            }
        }

        _switchGraph.dump("./routes/tmpArchSwitch.txt");
        clog << "_catchSwitchGraph finished" << endl;
        return true;
    }

    const bool PruAnalyser::dumpVerticesToDelete(const string &filePath, const string &model, unordered_set<string> &verticesToDelete) const
    {
        if (verticesToDelete.empty()) return true;
        ofstream fout(filePath, ios::app);
        if (!fout)
        {
            clog << "pruAnalyse dumpVerticesToDelete failed: file can't be opened" << filePath << endl;
            return false;
        }
        int verticesCounts = 0;
        fout << model << endl;
        for (const auto &vertex : verticesToDelete)
        {
            fout << vertex << " ";
            verticesCounts++;
            if (verticesCounts == 10) {
                fout << endl;
                verticesCounts = 0;
            }
        }
        fout << endl;
        fout.close();

        return true;
    }
    }
}