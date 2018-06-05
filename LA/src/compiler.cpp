#include <parser.h>
#include <code_generator.h>
#include <code_transformation.h>
#include <unistd.h>
#include <fstream>

using namespace std;

void print_to_file(string & p){
    std::ofstream output_file;
    output_file.open("prog.IR");
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
    LA::Program p = LA::parseFile(argv[optind]);

    LA::encode_decode(p);

    LA::check_array_access(p);

    LA::generate_basic_blocks(p);

    // cout << "before gen code" << endl;
    string compiled_program = LA::generate_code(p);
    // cout << "done gen code" << endl;
    if(argc == 3 && argv[2][1] == 'c'){
        print_to_console(compiled_program);
    } else {
        print_to_file(compiled_program);
    }
    
    return 0;
}
