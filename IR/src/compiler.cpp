#include <parser.h>
#include <code_generator.h>
#include <unistd.h>
#include <fstream>

using namespace std;

void print_to_file(string & p){
    std::ofstream output_file;
    output_file.open("prog.L3");
    output_file << p << endl;
    output_file.close();
    return;
}

void print_to_console(string & p){
    cout << p << endl;
    return;
}

int main( int argc, char **argv ) {

    IR::Program p = IR::parseFile(argv[optind]);


    string L3_p = IR::generate_code(p);

    if(argc == 3 && argv[2][1] == 'c'){
        print_to_console(L3_p);
    } else {
        print_to_file(L3_p);
    }
    
    return 0;
}
