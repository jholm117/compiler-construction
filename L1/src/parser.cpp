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

#include <L1.h>
#include <parser.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>


#include <iostream>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L1 {

  /* 
   * Data required to parse
   */ 
  std::vector<L1_item> parsed_registers;


  /* 
   * Grammar rules from now on.
   */
  struct label:
    pegtl::seq<
      pegtl::one<':'>,
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
    > {};
  struct L1_label_rule:
    label {};
 
  struct function_name:
    label {};

  struct number:
    seq<
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
  //rdi rsi rdx rcx r8 r9
  struct argument_register:
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "rdi" ),
      TAOCPP_PEGTL_STRING( "rsi" ),
      TAOCPP_PEGTL_STRING( "rdx" ),
      TAOCPP_PEGTL_STRING( "rcx" ),
      TAOCPP_PEGTL_STRING( "r8" ),
      TAOCPP_PEGTL_STRING( "r9" )
    >{};
  
  // argument register rax rbx rbp r10 r11 r12 r13 r14 r15
  struct writeable_register:
    pegtl::sor<
      argument_register,
      TAOCPP_PEGTL_STRING( "rax" ),
      TAOCPP_PEGTL_STRING( "rbx" ),
      TAOCPP_PEGTL_STRING( "rbp" ),
      TAOCPP_PEGTL_STRING( "r10" ),
      TAOCPP_PEGTL_STRING( "r11" ),
      TAOCPP_PEGTL_STRING( "r12" ),
      TAOCPP_PEGTL_STRING( "r13" ),
      TAOCPP_PEGTL_STRING( "r14" ),
      TAOCPP_PEGTL_STRING( "r15" )
    >{};

  struct rsp:
    TAOCPP_PEGTL_STRING( "rsp" ){};      

  // writeable rsp
  struct reg:
    pegtl::sor<
      rsp,
      writeable_register
    >{};

  struct tee:
    pegtl::sor<
      reg,
      number
    >{};

  struct you:
    sor<
      writeable_register,
      L1_label_rule
    >{};

  struct plus_equals:
    TAOCPP_PEGTL_STRING("+="){};

  struct minus_equals:
    TAOCPP_PEGTL_STRING("-="){};

  struct plus_minus_equals:
    sor<
      plus_equals,
      minus_equals
    >{};

  struct aop:
    sor<
      plus_equals,
      minus_equals,
      TAOCPP_PEGTL_STRING("*="),
      TAOCPP_PEGTL_STRING("&=")
    >{};

  struct sop:
    sor<
      TAOCPP_PEGTL_STRING("<<="),
      TAOCPP_PEGTL_STRING(">>=")
    >{};

  struct cmp:
    sor<
      TAOCPP_PEGTL_STRING("<="),
      pegtl::one<'<'>,
      pegtl::one<'='>
    >{};

  struct M:
    number {};      

  // register number label
  struct source:
    pegtl::sor<
      tee,
      L1_label_rule
    >{}; 

  struct arrow:
    TAOCPP_PEGTL_STRING("<-"){};


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

  struct seps_arrow:
    seq<
      seps,
      arrow,
      seps
    >{};

  struct mem:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("mem"),
      seps,
      reg,
      seps,
      M
    >{};

  // w <- s
  struct local_to_local:
    seq<
      writeable_register,
      seps_arrow,
      source
    >{};

  // w <- mem
  struct mem_to_local:
    seq<
      writeable_register,
      seps_arrow,
      mem
    > {};

  // mem <- s
  struct local_to_mem:
    seq<
      mem,
      seps_arrow,
      source
    >{};

  // w aop t
  struct local_math:
    seq<
      writeable_register,
      seps,
      aop,
      seps,
      tee
    >{};

  struct sx:
    TAOCPP_PEGTL_STRING("rcx"){};

  // w sop rcx, w sop N
  struct shift_i:
    seq<
      writeable_register,
      seps,
      sop,
      seps,
      sor<
        sx,
        number
      >
    >{};

  // mem += t, mem -= t
  struct local_to_mem_math:
    seq<
      mem,
      seps,
      plus_minus_equals,
      seps,
      tee
    >{};

  // w += mem, w-= mem
  struct mem_to_local_math:
    seq<
      writeable_register,
      seps,
      plus_minus_equals,
      seps,
      mem
    >{};

  struct math:
    sor<
      local_math,
      local_to_mem_math,
      mem_to_local_math
    >{};

  // w <- t cmp t
  struct cmp_assign:
    seq<
      writeable_register,
      seps_arrow,
      tee,
      seps,
      cmp,
      seps,
      tee
    >{};

  struct cmp_assign_num_to_num:
    seq<
      writeable_register,
      seps_arrow,
      M,
      seps,
      cmp,
      seps,
      M
    >{};    

  struct cjump_num_to_num:
    seq<
      TAOCPP_PEGTL_STRING("cjump"),
      seps,
      M,
      seps,
      cmp,
      seps,
      M,
      seps,
      L1_label_rule,
      seps,
      L1_label_rule
    >{};


  // cjump t cmp t label label
  struct cjump:
    seq<
      TAOCPP_PEGTL_STRING("cjump"),
      seps,
      tee,
      seps,
      cmp,
      seps,
      tee,
      seps,
      L1_label_rule,
      seps,
      L1_label_rule
    >{};

  // goto label
  struct goTo:
    seq<
      TAOCPP_PEGTL_STRING("goto"),
      seps,
      L1_label_rule
    >{};
  
  //call
  struct call:
    TAOCPP_PEGTL_STRING("call"){};

  //call u N:
  struct callFunction:
    seq<
      call,
      seps,
      you,
      seps,
      number
    >{};
  
  //call print 1
  struct callPrint:
    seq<
      call,
      seps,
      TAOCPP_PEGTL_STRING("print"),
      seps,
      one< '1' >
    >{};

  struct callAllocate:
    seq<
      call,
      seps,
      TAOCPP_PEGTL_STRING("allocate"),
      seps,
      one<'2'>
    >{};

  // call array-error 2
  struct callArrayError:
    seq<
      call,
      seps,
      TAOCPP_PEGTL_STRING("array-error"),
      seps,
      one<'2'>
    >{};

  struct callRuntimeF:
    sor<
      callPrint,
      callAllocate,
      callArrayError
    >{};

  // w++
  struct increment:
    seq<
      writeable_register,
      seps,
      TAOCPP_PEGTL_STRING("++")
    >{};

  //w--
  struct decrement:
    seq<
      writeable_register,
      seps,
      TAOCPP_PEGTL_STRING("--")
    >{};

  // w @ w w E
  struct lea:
    seq<
      writeable_register,
      seps,
      one<'@'>,
      seps,
      writeable_register,
      seps,
      writeable_register,
      seps,
      M
    >{};

  struct ret:
    TAOCPP_PEGTL_STRING( "return" ) {};
  
  struct assignment_i:
    sor<
      local_to_local,
      mem_to_local,
      local_to_mem
    > {};

  struct label_i:
    label{};

  struct instruction:
    seq<
      seps,
      sor<
        cmp_assign_num_to_num,
        cmp_assign,
        assignment_i,
        math,
        shift_i,
        cjump_num_to_num,
        cjump,
        ret,
        label_i,
        goTo,
        callRuntimeF,
        callFunction,
        increment,
        decrement,
        lea
      >,
      seps
    >{};

  struct argument_number:
    number {};

  struct local_number:
    number {} ;

  struct L1_function_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      star< instruction >,
      pegtl::one< ')' >,
      seps
    > {};

  struct L1_functions_rule:
    pegtl::seq<
      seps,
      pegtl::plus< L1_function_rule >
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      L1_functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > { };

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};

  #define push_i(i) p.functions.back()->instructions.push_back(i)
  #define pop(v) v.back(); v.pop_back()
  #define push_reg(in) L1_item i; i.labelName = "%" + in.string(); parsed_registers.push_back(i)
  #define push_op(in) L1_item i; i.labelName = in.string(); parsed_registers.push_back(i)

  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      if (p.entryPointLabel.empty()){
        p.entryPointLabel = in.string();
      }
      // cout << "label" << endl;
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *newF = new L1::Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < L1_label_rule > {
    template< typename Input >
		static void apply( const Input & in, L1::Program & p){
      L1_item i;
      i.labelName = "$_" + in.string().substr(1);
      // cout << "Label rule: " << i.labelName << endl;
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < label_i > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // L1_item i;
      // cout << "Label rule: " << i.labelName << endl;
      // parsed_registers.push_back(i);
      L1::Instruction* labelll = new L1::Label(in.string());
      push_i(labelll);
    }
  };

  template<> struct action < callRuntimeF > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Instruction* newF = new L1::CallRuntime(in.string());
      push_i(newF);
    }
  };  

  template<> struct action < writeable_register > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item i;
      i.labelName = "%" + in.string();
      // cout << "writeable: " << i.labelName << endl;
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

 template<> struct action < increment > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item reg = pop(parsed_registers);
      L1::Instruction* inc = new L1::Inc(reg);
      push_i(inc);
    } 
  };

  template<> struct action < decrement > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item reg = pop(parsed_registers);
      L1::Instruction* dec = new L1::Dec(reg);
      push_i(dec);
    }
  };

  template<> struct action < ret > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *currentF = p.functions.back();
      L1::Instruction *newRet = new L1::Ret();
      currentF->instructions.push_back(newRet);
    }
  };
  
  // w <- s
  template<> struct action < assignment_i > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::L1_item s = pop(parsed_registers);
      L1::L1_item w = pop(parsed_registers);

      L1::Instruction *a = new L1::Assignment(w, s);

      push_i(a);
    }
  };

  template<> struct action < goTo > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::L1_item labbel = pop(parsed_registers);
      L1::Instruction *a = new L1::GOTO(labbel);
      push_i(a);
    }
  };

  template<> struct action < cmp_assign_num_to_num > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item num2 = pop(parsed_registers);
      L1_item op = pop(parsed_registers);
      L1_item num1 = pop(parsed_registers);
      L1_item dest = pop(parsed_registers);
      L1::Instruction* inst = new L1::CmpNumbers(dest, num1, op, num2);
      push_i(inst);
    }
  };  

  template<> struct action < sx > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      push_reg(in);
    }
  };


  template<> struct action < rsp > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // save register
      L1_item i;
      i.labelName = "%" + in.string();
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // save register
      L1_item i;
      i.labelName = "$" + in.string();
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < M > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // save register
      L1_item i;
      i.labelName = in.string();
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < aop > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // save register
      L1_item i;
      i.labelName = in.string();
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < plus_minus_equals > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // save register
      push_op(in);
    }
  };

  template<> struct action < sop > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      push_op(in);
    }
  };

  template<> struct action < shift_i > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item src = pop(parsed_registers);
      L1_item op = pop(parsed_registers);
      L1_item dst = pop(parsed_registers);
      L1::Instruction* sop = new L1::AOP(dst,src,op.labelName);
      push_i(sop);
    }
  };

  template<> struct action < math > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item src = pop(parsed_registers);
      L1_item op = pop(parsed_registers);
      L1_item dst = pop(parsed_registers);
      L1::Instruction* aop = new L1::AOP(dst,src,op.labelName);
      push_i(aop);
    }
  };

  template<> struct action < cmp > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // cout << "cmp" << endl;
      push_op(in);
    }
  };

  template<> struct action < seps > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // cout << "seps" << endl;
    }
  };
  template<> struct action < arrow > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // cout << "arrow" << endl;
    }
  };

  template<> struct action < cmp_assign > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // cout << "cmp_assign" << endl;
      L1_item src2 = pop(parsed_registers);
      L1_item op = pop(parsed_registers);
      L1_item src1 = pop(parsed_registers);
      L1_item dst = pop(parsed_registers);
      L1::Instruction* cmp = new L1::Cmp(dst, src1, src2, op.labelName);
      push_i(cmp);
    }
  };

  template<> struct action < mem > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // save register
      L1_item i;
      L1::L1_item num = pop(parsed_registers);
      L1::L1_item reg = pop(parsed_registers);
      i.labelName = num.labelName + "(" + reg.labelName + ")";
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < lea > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item e = pop(parsed_registers);
      L1_item reg2 = pop(parsed_registers);
      L1_item reg1 = pop(parsed_registers);
      L1_item dest = pop(parsed_registers);
      L1::Instruction* lena = new L1::LEA(dest, reg1, reg2, e);
      push_i(lena);
    }
  };

  template<> struct action < cjump > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // cout << "hi" << endl;
      L1_item fLabel = pop(parsed_registers);
      L1_item tLabel = pop(parsed_registers);
      L1_item comp2 = pop(parsed_registers);
      L1_item op = pop(parsed_registers);
      L1_item comp1 = pop(parsed_registers);
      L1::Instruction* cjump = new L1::CJump(comp1, comp2, tLabel, fLabel, op);
      push_i(cjump);
    }
  };

  template<> struct action < cjump_num_to_num > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      // cout << "in numtonum" << endl;
      // cout << "hi" << endl;
      L1_item fLabel = pop(parsed_registers);
      L1_item tLabel = pop(parsed_registers);
      L1_item num2 = pop(parsed_registers);
      L1_item op = pop(parsed_registers);
      L1_item num1 = pop(parsed_registers);
      L1::Instruction* cjump = new L1::CJumpNum(num1, num2, tLabel, fLabel, op);
      push_i(cjump);
    }
  };

  template<> struct action < callFunction > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1_item args = pop(parsed_registers);
      L1_item callee = pop(parsed_registers);
      L1::Instruction* call = new L1::CallFunction(callee, args);
      push_i(call);
    }
  };

  Program parse_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< L1::grammar >();

    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L1::Program p;
    parse< L1::grammar, L1::action >(fileInput, p);
    return p;
  }

} // L1
