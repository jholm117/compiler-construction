#include <parser.h>
#include <unistd.h>

using namespace std;

int main( int argc, char **argv ) {

    IR::Program p = IR::parseFile(argv[optind]);
    
    return 0;
}
