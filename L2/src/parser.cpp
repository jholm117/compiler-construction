#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include <L2.h>
#include <parser.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

#define POP_ARG parsed_args.back(); parsed_args.pop_back(); 
#define SEPS 

namespace L2 {

  /* 
   * Data required to parse
   */ 
  std::vector<L2::L2_Item*> parsed_items;
  instructionType i_type = I_TYPE_ERROR;
  
  L2_Item* popItem(){
    L2_Item* i = parsed_items.back();
    parsed_items.pop_back();
    return i;
  }

  /* 
   * Grammar rules from now on.
   */

  struct var:
    pegtl::seq<
      pegtl::plus< 
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    >{};

  struct variable:
    var{};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      var
    > {};

  struct function_name:
    label {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    >{};

  struct argument_number:
    number {};

  struct local_number:
    number {} ;

  struct comment: 
    pegtl::disable< 
      TAOCPP_PEGTL_STRING( "//" ), 
      pegtl::until< pegtl::eolf > 
    > {};

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct L2_label_rule:
    label {};
  
  struct reg:
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "rdi" ),
      TAOCPP_PEGTL_STRING( "rsi" ),
      TAOCPP_PEGTL_STRING( "rax" ),
      TAOCPP_PEGTL_STRING( "rbx" ),
      TAOCPP_PEGTL_STRING( "rdx" ),
      TAOCPP_PEGTL_STRING( "rcx" ),
      TAOCPP_PEGTL_STRING( "rbp" ),
      TAOCPP_PEGTL_STRING( "rsp" ),
      TAOCPP_PEGTL_STRING( "r8" ),
      TAOCPP_PEGTL_STRING( "r9" ),
      TAOCPP_PEGTL_STRING( "r10" ),
      TAOCPP_PEGTL_STRING( "r11" ),
      TAOCPP_PEGTL_STRING( "r12" ),
      TAOCPP_PEGTL_STRING( "r13" ),
      TAOCPP_PEGTL_STRING( "r14" ),
      TAOCPP_PEGTL_STRING( "r15" )
    >{};

  
  struct stack_arg:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("stack-arg"),
      seps,
      number
    >{};

  struct mem:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("mem"),
      seps,
      pegtl::sor<
        reg,
        variable
      >,
      seps,
      number
    >{};

  struct source:
    pegtl::sor<
      reg,
      L2_label_rule,
      number,
      mem,
      stack_arg,
      variable
    >{};

  
  struct sop:
    sor<
      TAOCPP_PEGTL_STRING("<<="),
      TAOCPP_PEGTL_STRING(">>=")
    >{};

  struct aop:
    pegtl::sor<
      TAOCPP_PEGTL_STRING("+="),
      TAOCPP_PEGTL_STRING("-="),
      TAOCPP_PEGTL_STRING("*="),
      TAOCPP_PEGTL_STRING("&=")
    >{};
  

  struct arrow:
    TAOCPP_PEGTL_STRING("<-"){};
  
  struct op:
    pegtl::sor<
      arrow,
      aop,
      sop
    >{};


  struct dest:
    pegtl::sor<
      reg,
      variable
    >{};

  struct inc_dec_op:
    pegtl::sor<
      TAOCPP_PEGTL_STRING("++"),
      TAOCPP_PEGTL_STRING("--")
    >{};

  struct cmp:
    pegtl::sor<
      TAOCPP_PEGTL_STRING("<="),
      one< '<' >,
      one< '=' >
    >{};
  

  struct cjump_i:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("cjump"),
      seps,
      source,
      seps,
      cmp,
      seps,
      source,
      seps,
      L2_label_rule,
      seps,
      L2_label_rule
    >{};

  struct goto_i:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("goto"),
      seps,
      L2_label_rule
    >{};

  struct label_i:
    pegtl::seq<
      L2_label_rule
    >{};

  struct ret:
    TAOCPP_PEGTL_STRING("return") {};

  
  struct aop_suffix:
    pegtl::seq<
      aop,
      seps,
      source
    >{};

  struct sop_suffix:
    pegtl::seq<
      sop,
      seps,
      source
    >{};

  struct src_cmp_src_suffix:
    pegtl::seq<
      cmp,
      seps,
      source
    >{};
  
  struct assign_suffix:
    pegtl::seq<
      arrow,
      seps,
      source,
      seps,
      pegtl::star<src_cmp_src_suffix>
    >{};

  struct inc_dec_suffix:
    inc_dec_op {};

  struct at_op:
    pegtl::one< '@' >{};
  
  struct lea_suffix:
    pegtl::seq<
      at_op,
      seps,
      source,
      seps,
      source,
      seps,
      number
    >{};

  struct local_i:
    pegtl::seq<
      pegtl::sor<
        reg,
        variable
      >,
      seps,
      pegtl::sor<
        aop_suffix,
        sop_suffix,
        inc_dec_suffix,
        assign_suffix,
        lea_suffix
      >
    >{};

  struct mem_i:
    pegtl::seq<
      mem,
      seps,
      pegtl::sor<
        aop_suffix,
        sop_suffix,
        assign_suffix
      >
    >{};

  struct runtime_suffix:
    pegtl::sor<
      TAOCPP_PEGTL_STRING("print 1"),
      TAOCPP_PEGTL_STRING("allocate 2"),
      TAOCPP_PEGTL_STRING("array-error 2")
    >{};

  struct call_local_suffix:
    pegtl::seq<
      source,
      seps,
      number
    >{}; 

  struct call_i:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("call"),
      seps,
      pegtl::sor<
        runtime_suffix,
        call_local_suffix
      >
    >{};

  struct instruction:
    pegtl::seq<
      seps,
      pegtl::sor<
        ret,
        label_i,
        cjump_i,
        call_i,
        mem_i,
        goto_i,
        local_i
      >,
      seps
    >{};

  struct L2_function_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      pegtl::plus< instruction >,
      pegtl::one< ')' >,
      seps
    > {};

  struct L2_functions_rule:
    pegtl::seq<
      seps,
      pegtl::plus< L2_function_rule >
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      L2_functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > { };

  struct var_to_spill:
    var{};
  
  struct spill_prefix:
    var{};

  struct function_file_grammar:
    pegtl::must<
      L2_function_rule
    >{};

  struct spill_file_grammar:
    pegtl::must<
      L2_function_rule,
      seps,
      var_to_spill,
      seps,
      spill_prefix
    >{};


  struct grammar: 
    pegtl::must< 
      entry_point_rule
    > {};

  /*
   * Helper functions used in actions
   */
  void CacheNumber(std::string str) {
    L2::Number* num = new Number (std::stoi(str));
    parsed_items.push_back(num);
  }

  void CacheLabel(std::string str) {
    L2::Label* label = new Label(str);
    parsed_items.push_back(label);
  }
  
  void CacheVariable(std::string str) {
    L2::Variable* var = new Variable(str);
    parsed_items.push_back(var);
  }

  void CacheRegister(std::string str) {
    L2::Register* reg = new Register (chooseRegisterType(str));
    parsed_items.push_back(reg);
  }

  void CacheOperator(std::string str) {
    L2::Operator* op = new Operator(chooseOperatorType(str));
    parsed_items.push_back(op);
  }
  
  void InitInstruction( instructionType t, int num, Program p){
    Instruction* i = new Instruction();
    i->type = t;
    vector<L2_Item*>::iterator first = parsed_items.end() - num;
    i->args = vector<L2_Item*> (first, parsed_items.end());
    parsed_items.resize(parsed_items.size() - num);
    p.functions.back()->instructions.push_back(i);
    i_type = I_TYPE_ERROR;
  }
    
  void MakeInstruction( instructionType t, Program p ){
    int numArgs = chooseNumArgs(t);
    InitInstruction( t, numArgs, p);
  }

  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if (p.entryPointLabel.empty()){
        p.entryPointLabel = in.string();
      }
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      L2::Function *newF = new L2::Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < L2_label_rule > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheLabel(in.string());
    }
  };

  template<> struct action < mem > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p ){
      L2_Item* num = popItem();
      L2_Item* address = popItem();

      L2::Mem* m = new Mem (address, num);
      parsed_items.push_back(m);
    }
  };
  
  template<> struct action < stack_arg > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p ){
      L2_Item* num = popItem();

      L2_Item* m = new StackArg (num);
      parsed_items.push_back(m);
    }
  };

  template<> struct action < L2_function_rule > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p ){
    }
  };

  template<> struct action < number > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheNumber(in.string());
    }
  };

  template<> struct action < reg > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheRegister(in.string());
    }
  };
  
  template<> struct action < variable > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheVariable(in.string());
    }
  };

  // Operator actions

  template<> struct action < aop > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheOperator(in.string());
    }
  };

  template<> struct action < sop > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheOperator(in.string());
    }
  };

  template<> struct action < arrow > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheOperator(in.string());
    }
  };

  template<> struct action < at_op > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
            
      CacheOperator(in.string());
    }
  };

  template<> struct action < cmp > {
    template< typename Input >
		static void apply( const Input & in, L2::Program & p){
      CacheOperator(in.string());
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      L2::Function *currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      L2::Function *currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  // Suffix Actions
  
  template<> struct action < aop_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = AOP;
    }
  };

  template<> struct action < sop_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = SOP;
    }
  };

  template<> struct action < assign_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if (i_type != ASSIGN_CMP) i_type = ASSIGN;
    }
  };

  template<> struct action < inc_dec_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      CacheOperator(in.string());
      i_type = INC_DEC;
    }
  };

  template<> struct action < lea_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = LEA;
    }
  };

  template<> struct action < src_cmp_src_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = ASSIGN_CMP;
    }
  };

  template<> struct action < call_local_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = CALL_LOCAL;
    }
  };

  template<> struct action < runtime_suffix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      L2::RuntimeFunction* r = new RuntimeFunction();
      r->name = in.string().substr(0,in.string().length()-2);
      int numArgs = std::stoi(in.string().substr(in.string().length()-1));
      L2::Number* n = new Number (numArgs);

      parsed_items.push_back(r);
      parsed_items.push_back(n);

      i_type = CALL_RUNTIME;
    }
  };

  // Instruction actions

  template<> struct action < label_i > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = LABEL;
    }
  };

  template<> struct action < ret > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = RETURN;
    }
  };

  template<> struct action < goto_i > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = GOTO;
    }
  };

  template<> struct action < cjump_i > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      i_type = CJUMP;
    }
  };

  template<> struct action < instruction > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      MakeInstruction(i_type, p);
    }
  };

  template<> struct action < var_to_spill > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      p.spill.varToSpill = new Variable(in.string());
    }
  };

  template<> struct action < spill_prefix > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      p.spill.prefixSpilledVars = in.string();
    }
  };



  Program parse_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< L2::grammar >();

    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L2::Program p;
    parse< L2::grammar, L2::action >(fileInput, p);
    return p;
  }
  
  Program parse_function_file(char* fileName){
    pegtl::analyze< L2::function_file_grammar >();


    file_input< > fileInput(fileName);
    L2::Program p;
    parse < L2::function_file_grammar, L2::action >(fileInput, p);
    return p;
  }

  Program parse_spill_file(char *fileName){
    pegtl::analyze< L2::spill_file_grammar >();
     file_input< > fileInput(fileName);
    L2::Program p;
    parse < L2::spill_file_grammar, L2::action >(fileInput, p);
    return p;
  }
} // L2
