#ifndef __PRUANALYSE__
#define __PRUANALYSE__
#include "../../common/HyperGraph.h"

using namespace std;
namespace FastCGRA
{
    namespace PRUANALYSE{
    class PruAnalyser {
        private:
            Graph RRG;

            unordered_map<string, unordered_map<string, vector<string>>> linkTable;
            unordered_map<string, vector<string>> pathTable;
            
            unordered_map<string, unordered_map<string, vector<vector<string>>>> routesTable;
            unordered_map<string, unordered_map<string, unordered_set<string>>> routesIdx;

          public:
            PruAnalyser() = default;
            PruAnalyser(const string &rrgFile) : RRG(Graph(rrgFile)), linkTable(), pathTable(), routesTable(), routesIdx() { analyze(); }
            PruAnalyser(const string &rrgFile, const string &routesFile) : RRG(Graph(rrgFile)), linkTable(), pathTable(), routesTable(), routesIdx() {analyze(); Parse(routesFile); }
            PruAnalyser(const string &rrgFile, const string &routesFile, const string & archModel) : RRG(Graph(rrgFile)), linkTable(), pathTable(), routesTable(), routesIdx()
            {
                if(archModel == "ARCH_TOP"){
                    Graph _RRG;
                    unordered_map<string, string> switch2Fu;
                    unordered_map<string, string> switchInIdx;
                    _analyze(switchInIdx, switch2Fu);
                    Parse(routesFile, switchInIdx, switch2Fu);
                }
                else if (archModel == "TOP_SWITCH")
                {
                    analyze();
                    Parse(routesFile);
                }
            }

            PruAnalyser(const PruAnalyser & pruAnalyser) : RRG(pruAnalyser.RRG), linkTable(pruAnalyser.linkTable), pathTable(pruAnalyser.pathTable), routesTable(pruAnalyser.routesTable), routesIdx(pruAnalyser.routesIdx) {}
            PruAnalyser(PruAnalyser && pruAnalyser) : RRG(pruAnalyser.RRG), linkTable(pruAnalyser.linkTable), pathTable(pruAnalyser.pathTable), routesTable(pruAnalyser.routesTable), routesIdx(pruAnalyser.routesIdx) {}

            const PruAnalyser &operator=(const PruAnalyser &pruAnalyser)
            {
                RRG = pruAnalyser.RRG;
                linkTable = pruAnalyser.linkTable;
                pathTable = pruAnalyser.pathTable;
                routesTable = pruAnalyser.routesTable;
                routesIdx = pruAnalyser.routesIdx;

                return *this;
            }

            const PruAnalyser &operator=(PruAnalyser && pruAnalyser)
            {
                RRG = pruAnalyser.RRG;
                linkTable = pruAnalyser.linkTable;
                pathTable = pruAnalyser.pathTable;
                routesTable = pruAnalyser.routesTable;
                routesIdx = pruAnalyser.routesIdx;

                return *this;
            }

            const unordered_map<string, vector<string>> &operator()(const string &name) const
            {
                assert(linkTable.find(name) != linkTable.end());
                return linkTable.find(name)->second;
            }
            const vector<string> &operator()(const string &from, const string &to) const
            {
                assert(linkTable.find(from) != linkTable.end() && linkTable.find(from)->second.find(to) != linkTable.find(from)->second.end());
                return linkTable.find(from)->second.find(to)->second;
            }
            const vector<string> &operator[](const string &path) const
            {
                assert(pathTable.find(path) != pathTable.end());
                return pathTable.find(path)->second;
            }
        
            const unordered_map<string, unordered_map<string, vector<string>>> & getLinkTable() const {return linkTable;}
            const unordered_map<string, vector<string>> & getPathTable() const {return pathTable;}
            const unordered_map<string, unordered_map<string, vector<vector<string>>>> & getRoutesTable() const {return routesTable;}
            const unordered_map<string, unordered_map<string, unordered_set<string>> > & getRoutesIdx() const {return routesIdx;} 
            const size_t getRoutesCounts() const;

            void setRoutesTable(const string & routesFile) {Parse(routesFile);}
            void setRoutesTable(string && routesFile) {Parse(routesFile);}
            void setRRG(const Graph & rrg) { RRG = rrg; analyze();}
            void setRRG(Graph && rrg) { RRG = rrg ; analyze();} 

            bool analyze();
            bool _analyze(unordered_map<string, string> & switchInIdx, unordered_map<string, string> & switch2Fu);
            bool Parse(const string & routesFile);
            bool Parse(string && routesFile);
            bool Parse(const string & routesFile, unordered_map<string, string> & switchInIdx, unordered_map<string, string> & switch2Fu);
            bool Parse(string && routesFile, unordered_map<string, string> & switchInIdx, unordered_map<string, string> & switch2Fu);

            bool _catchSwitchGraph(Graph &_switchGraph, unordered_map<string, string> & switchInIdx, unordered_map<string, string> & switch2Fu);

            unordered_map<string, vector<string>> DeletePathVanilla(); 
            unordered_map<string, vector<string>> DeletePathAnnealingVersion0(); // 模拟退火
            unordered_set<string> DeleteVerticesEdgesIn(const unordered_map<string, vector<string>> &pathToDelete);

            const bool dumpVerticesToDelete(const string & filePath, const string & model, unordered_set<string> & verticesToDelete) const;
    };
    }
}




#endif //
