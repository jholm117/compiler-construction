// #include <L3.h>
#include <parser.h>
#include <codeGenerator.h>
#include <unistd.h>

using namespace std;

void printInstructions(L3::Program & p){
    for (auto f : p.functions){
        cout << "num instructions" << f->instructions.size() << "\n";
        for (auto i : f->instructions){
            cout << i->toString() << "\n";
        }
    }
    cout << endl;
}

int main( int argc, char **argv ) {

    L3::Program p = L3::parseFile(argv[optind]);

    // cout << "parsed" << endl;
    // printInstructions(p);

    L3::transformLabels(p);

    // cout << "labels" << endl;
    // printInstructions(p);

    L3::generateCode(p);

    // cout << "code gen" << endl;
    // printInstructions(p);

    return 0;
}
