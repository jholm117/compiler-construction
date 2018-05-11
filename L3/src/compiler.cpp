// #include <L3.h>
#include <parser.h>
#include <codeGenerator.h>
#include <unistd.h>

int main( int argc, char **argv ) {

    L3::Program p = L3::parseFile(argv[optind]);

    L3::transformLabels(p);

    L3::generateCode(p);

    return 0;
}
