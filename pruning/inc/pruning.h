#ifndef __PRUNING__
#define __PRUNING__
#include "../../common/HyperGraph.h"

using namespace std;

namespace FastCGRA
{
    namespace PRUNING
    {
        class Pruning
        {
            private:

                Graph ArchRRG;
                Graph ArchRRG_backup;
                Graph CoreRRG;
                Graph ARIRRG;
                vector<string> fbDeleteVertices;
                unordered_map<string, unordered_map<string, string>> fbReplaceVertices;
                unordered_map<string, unordered_set<string>> switchDeleteVertices;

            public:
                Pruning() = default;
                Pruning(const string &filePath) : ArchRRG(Graph("./arch6/Top_RRG.txt")), ArchRRG_backup(Graph("./arch6/Top_RRG.txt")), CoreRRG(Graph("./arch6/Core_RRG.txt")), ARIRRG(Graph("./arch6/BlockARI_RRG.txt")), fbDeleteVertices(), fbReplaceVertices(), switchDeleteVertices() { ParsePruningFuFile(filePath); }
                Pruning(const string &filePath0, const string &filePath1) : ArchRRG(Graph("./arch6/Top_RRG.txt")), ArchRRG_backup(Graph("./arch6/Top_RRG.txt")), CoreRRG(Graph("./arch6/Core_RRG.txt")), ARIRRG(Graph("./arch6/BlockARI_RRG.txt")), fbDeleteVertices(), fbReplaceVertices(), switchDeleteVertices() { Parse(filePath0, filePath1); }

                Pruning &operator=(const Pruning & pruning) {ArchRRG = pruning.ArchRRG; CoreRRG = pruning.CoreRRG; ARIRRG = pruning.ARIRRG; return *this;}
                Pruning &operator=(Pruning && pruning) {ArchRRG = pruning.ArchRRG; CoreRRG = pruning.CoreRRG; ARIRRG = pruning.ARIRRG; return *this;}

                const vector<string> & getFbDeleteVertices() const {return fbDeleteVertices;}
                const unordered_map<string, unordered_map<string, string>> & getFbReplaceVertices() const {return fbReplaceVertices;} 

                bool ParsePruningSwitchFile(const string &filePath);
                bool ParsePruningFuFile(const string &filePath);
                bool Parse(const string &filePath0, const string &filePath1);

                bool pruningRRG();
                bool _pruningSwitch();
                bool _pruningFu();
                bool _replaceFu_SW(Graph & graph, const string & _fuType, const string & _fuDelete, const string & _fuReplace);
                bool _replaceFu_MUX(Graph & graph, const string & _fuType, const string & _fuDelete, const string & _fuReplace);
                bool _deleteFu(Graph & graph, const string & _fuDelete);

                const bool dumpRRG(const string &archPath, const string & corePath, const string & ariPath) const;
        
                const bool evaluation() const;

        };



    }
}



#endif