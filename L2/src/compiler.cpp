// #include <string>
// #include <vector>
#include <utility>
#include <algorithm>
// #include <set>
#include <iterator>
// #include <iostream>
// #include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
// #include <iostream>
#include <L2.h>
// #include <my_utils.h>
#include <parser.h>
#include <fstream>
//#include <analysis.h>
//#include <transformer.h>
#include <code_generator.h>
//#include <spiller.h>
//#include <register_allocation.h>
//#include <utils.h>

using namespace std;

void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l 1|2] [-i] SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  auto spill_only = false;
  auto interference_only = false;
  int32_t liveness_only = 0;
  int32_t optLevel = 0;

  /* 
   * Check the compiler arguments.
   */
  // Utils::verbose = false;
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:sl:i")) != -1) {
    switch (opt){

      case 'l':
        liveness_only = strtoul(optarg, NULL, 0);
        break ;

      case 'i':
        interference_only = true;
        break ;

      case 's':
        spill_only = true;
        break ;

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        break ;

      case 'v':
        // Utils::verbose = true;
        break ;

      default:
        print_help(argv[0]);
        return 1;
    }
  }

  /*
   * Parse the input file.
   */
  L2::Program p;
  if (spill_only){

    /* 
     * Parse an L2 function and the spill arguments.
     */
    p = L2::parse_spill_file(argv[optind]);
 
  } else if (liveness_only){

    /*
     * Parse an L2 function.
     */
    p = L2::parse_function_file(argv[optind]);

  } else if (interference_only){

    /*
     * Parse an L2 function.
     */
    p = L2::parse_function_file(argv[optind]);

  } else {

    /* 
     * Parse the L2 program.
     */
    p = L2::parse_file(argv[optind]);

  }

  /* 
   * Print the source program.
   */
  if (false){
    for (auto f : p.functions){
      // cout << *f << endl;
    }
  }

  /*
   * Transform the code.
   */
  // L2::MID_transformCode(p, optLevel);

  /*
   * Special cases.
   */
  if (spill_only){

    /*
     * Spill.
     */
    // cout << "var: " << p.spill.varToSpill->toString() << endl;
    // cout << "prefix: " << p.spill.prefixSpilledVars << endl;
    L2::SPILLER_spill(p, p.functions[0], p.spill.varToSpill, p.spill.prefixSpilledVars);

    /*
     * Dump the L2 code.
     */
    // cout << "hello" << endl;
    std::ofstream outputFile;
    outputFile.open("i27.L2f");
    outputFile << p.functions[0]->toString() << endl;
    outputFile.close();
    // cout << "goodbye" << endl;

    return 0;
  }

  /*
   * Liveness test.
   */
  if (liveness_only){
    for (auto f : p.functions){

      /*
       * Compute the liveness analysis.
       */
      auto liveness = L2::computeLivenessAnalysis(p, f);

      /*
       * Print the liveness.
       */
      // cout << liveness->toString(f, liveness_only > 1) << endl;
      cout << liveness->toString() << endl;

      /*
       * Free the memory.
       */
      delete liveness;
    }

    return 0;
  }

  /*
   * Interference graph test.
   */
  if (interference_only){
    for (L2::Function* f : p.functions){

      /*
       * Compute the liveness analysis.
       */
      L2::DataFlowResult* liveness = L2::computeLivenessAnalysis(p, f);

      /*
       * Compute the interference graph.
       */
      L2::InterferenceGraph* ig = L2::computeInterferenceGraph(f, liveness);

      /*
       * Print the interference graph.
       */
      for (auto p : *ig){
        std::string n1 = p.first->toString();
        // cout << "key: " << n1 << " values: ";
        cout << n1 << " ";
        for (auto n2 : p.second){
          cout << n2->toString() << " ";
        }
        cout << std::endl;
      }
      // cout << "FINISHED INTERFERENCE" << endl;
      // testing graph coloring
      // L2::ColoredGraph* cg = L2::computeColoredGraph(ig);
      // cout << "FINISHED COLORING" << endl;
      // for( auto p : *cg ) {
      //   std::cout << p.first->toString() << " " <<  chooseRegisterName(p.second) << std::endl;
      // }

      /*
       * Free the memory.
       */
      delete liveness;
      delete ig;
    }

    return 0;
  }



  /*
   * Generate the target code.
   */
  if (enable_code_generator){
    L2::generate_code(p);
  }

  return 0;
}
