#include <code_generator.h>

using namespace std;

namespace IR {
    void generate_code(Program & p){
        for (auto f : p.functions){
            cout << f->name.name << " {" << endl;
            for (auto bb : f->basicBlocks){
                cout << bb->label.name << endl;
                for (auto i : bb->instructions){
                    for(auto a : i->args){
                        cout << a->name << " ";
                    }
                    cout << endl;
                }
            }
            cout << "}" << endl;
        }
    }
}