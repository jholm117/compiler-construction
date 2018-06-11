#include <parser.h>
#include <code_generator.h>
#include <code_transformation.h>
#include <unistd.h>
#include <fstream>

using namespace std;

void print_to_file(string & p){
    std::ofstream output_file;
    output_file.open("prog.a");
    output_file << p << endl;
    output_file.close();
    return;
}

void print_to_console(string & p){
    cout << p << endl;
    return;
}

int main( int argc, char **argv ) {
    // cout << "before parse" << endl;
    LB::Program p = LB::parseFile(argv[optind]);
    // cout << "before flatten_scopes" << endl;
    
    LB::flatten_scopes(p);
    // cout << "before control_structures" << endl;
    
    LB::translate_control_structures(p);

    // cout << "before gen code" << endl;
    string compiled_program = LB::generate_code(p);

    if(argc == 3 && argv[2][1] == 'c'){
        print_to_console(compiled_program);
    } else {
        print_to_file(compiled_program);
    }
    
    return 0;
}
