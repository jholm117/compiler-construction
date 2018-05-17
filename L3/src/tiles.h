#include <L3.h>

using namespace std;
using namespace L3;

struct L2_Tile {
    // virtual bool match(Tree & t) = 0;
};

struct L2_Assign : L2_Tile {
    L2_Item* src;
    Variable* dest;
    static bool match(Tree & t){
        if( t is VAR and )
    }
};

struct L2_PlusEquals : L2_Tile {
    Variable* dest;
    L2_Item* src;
    bool match(Tree & t){
        // conditions:
        // root is variable
        // one child and is plus
        // two children of plus
        // one is the same variable as root
    }
}