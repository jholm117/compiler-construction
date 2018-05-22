#include <code_generator.h>

using namespace std;

namespace IR {
    
    void printParsedItems(Program & p){
        for (auto f : p.functions){
            cout << f->name.name;
            for (auto p : f->parameters){
                cout << " " << utils::pair_first(p)->name << " " << utils::pair_second(p).name;
            }
            cout << " {" << endl;
            for (auto bb : f->basicBlocks){
                cout << bb->label.name << endl;
                for (auto i : bb->instructions){
                    for(auto a : i->args){
                        cout << a->name << " ";
                    }
                    cout << endl;
                }
            }
            cout << "}" << endl << endl;
        }
    }

    void generate_code(Program & p){
        printParsedItems(p);
    }

}