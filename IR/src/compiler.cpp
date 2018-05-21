#include <parser.h>
#include <code_generator.h>
#include <unistd.h>

using namespace std;

int main( int argc, char **argv ) {

    IR::Program p = IR::parseFile(argv[optind]);

    IR::generate_code(p);
    
    return 0;
}
