#include <parser.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace std;
using namespace pegtl;

namespace IR {

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

    struct number_rule:
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

    struct keyword:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("return"),
            TAOCPP_PEGTL_STRING("call"),
            TAOCPP_PEGTL_STRING("br"),
            TAOCPP_PEGTL_STRING("length"),
            TAOCPP_PEGTL_STRING("Array"),
            TAOCPP_PEGTL_STRING("print"),
            TAOCPP_PEGTL_STRING("array-error"),
            TAOCPP_PEGTL_STRING("new"),
            TAOCPP_PEGTL_STRING("define"),
            TAOCPP_PEGTL_STRING("tuple"),
            TAOCPP_PEGTL_STRING("code"),
            TAOCPP_PEGTL_STRING("Tuple")           
        > {};

    struct variable_rule:
        pegtl::seq<
            pegtl::one< '%' >,
            // pegtl::not_at<
            //     pegtl::seq<
            //         keyword,
            //         pegtl::sor< 
            //             pegtl::ascii::space, 
            //             comment 
            //         >
            //     >
            // >,
            var
        > {};

    struct label_rule:
        pegtl::seq<
            pegtl::one< ':' >,
            var
        >{};

    struct type_rule:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("tuple"),
            TAOCPP_PEGTL_STRING("code"),
            TAOCPP_PEGTL_STRING("void"),
            pegtl::seq<
                TAOCPP_PEGTL_STRING("int64"),
                pegtl::star< TAOCPP_PEGTL_STRING("[]") >
            >
        >{};
        
    struct return_type_rule:
        type_rule {};

    struct variable_or_number:
        pegtl::sor<
            variable_rule,
            number_rule
        >{};

    struct source:
        pegtl::sor<
            variable_or_number,
            label_rule
        >{};
    
    struct op_rule:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("+"),
            TAOCPP_PEGTL_STRING("-"),
            TAOCPP_PEGTL_STRING("*"),
            TAOCPP_PEGTL_STRING("&"),
            TAOCPP_PEGTL_STRING("<<"),
            TAOCPP_PEGTL_STRING(">>"),
            TAOCPP_PEGTL_STRING("<"),
            TAOCPP_PEGTL_STRING("<="),
            TAOCPP_PEGTL_STRING("="),
            TAOCPP_PEGTL_STRING(">="),
            TAOCPP_PEGTL_STRING(">")
        >{};

    struct parameter_rule:
        pegtl::seq<
            seps,
            type_rule,
            seps,
            variable_rule,
            seps
        >{};

    struct conditional_branch_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("br"),
            seps,
            variable_or_number,
            seps,
            label_rule,
            seps,
            label_rule
        >{};

    struct branch_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("br"),
            seps,
            label_rule
        >{};
    
    struct return_value_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("return"),
            seps,
            variable_or_number
        >{};
    
    struct return_i:
        TAOCPP_PEGTL_STRING("return"){};
    
    struct type_var_i:
        pegtl::seq<
            type_rule,
            seps,
            variable_rule
        >{};
    
    struct assign_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            source
        >{};
    
    struct assign_op_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            variable_or_number,
            seps,
            op_rule,
            seps,
            variable_or_number
        >{};

    struct array_dimension:
        pegtl::seq<
            one< '[' >,
            variable_or_number,
            one< ']' >
        >{};

    struct array_load_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            variable_rule,
            pegtl::plus< array_dimension >
        >{};

    struct array_store_i:
        pegtl::seq<
            variable_rule,
            pegtl::plus< array_dimension >,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            source
        >{};

    struct length_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            TAOCPP_PEGTL_STRING("length"),
            seps,
            variable_rule,
            seps,
            variable_or_number
        >{};

    struct args_rule:
        pegtl::sor<
            pegtl::seq<
                variable_or_number,
                pegtl::star<
                    seps,
                    pegtl::one< ',' >,
                    seps,
                    variable_or_number
                >
            >,
            pegtl::success   
        > {};

    struct runtime_function:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("print"),
            TAOCPP_PEGTL_STRING("array-error")
        >{};
    
    struct callee_rule:
        pegtl::sor<
            variable_rule,
            label_rule,
            runtime_function
        >{};

    struct call_rule:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("call"),
            seps,
            callee_rule,
            seps,
            one< '(' >,
            seps,
            args_rule,
            seps,
            one< ')' >
        >{};

    struct call_i:
        call_rule {};

    struct assign_call_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            call_rule
        >{};
    
    struct new_array_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            TAOCPP_PEGTL_STRING("new"),
            seps,
            TAOCPP_PEGTL_STRING("Array"),
            seps,
            one< '(' >,
            seps,
            args_rule,
            seps,
            one< ')' >
        >{};
    
    struct new_tuple_i:
        pegtl::seq<
            variable_rule,
            seps,
            TAOCPP_PEGTL_STRING("<-"),
            seps,
            TAOCPP_PEGTL_STRING("new"),
            seps,
            TAOCPP_PEGTL_STRING("Tuple"),
            seps,
            one< '(' >,
            seps,
            variable_or_number,
            seps,
            one< ')' >
        >{};

    struct terminus_rule:
        pegtl::sor<
            pegtl::seq< pegtl::not_at< conditional_branch_i >, conditional_branch_i >,
            pegtl::seq< pegtl::not_at< branch_i >, branch_i >,
            pegtl::seq< pegtl::not_at< return_value_i >, return_value_i >,
            pegtl::seq< pegtl::not_at< return_i >, return_i >
        >{};

    struct instruction_rule:
        pegtl::seq<
            seps,
            pegtl::sor<
                pegtl::seq< pegtl::not_at< type_var_i >, type_var_i >,
                pegtl::seq< pegtl::not_at< assign_i >, assign_i >,
                pegtl::seq< pegtl::not_at< assign_op_i >, assign_op_i >,
                pegtl::seq< pegtl::not_at< array_load_i >, array_load_i >,
                pegtl::seq< pegtl::not_at< array_store_i >, array_store_i >,
                pegtl::seq< pegtl::not_at< length_i >, length_i >,
                pegtl::seq< pegtl::not_at< call_i >, call_i >,
                pegtl::seq< pegtl::not_at< assign_call_i >, assign_call_i >,
                pegtl::seq< pegtl::not_at< new_array_i >, new_array_i >,
                pegtl::seq< pegtl::not_at< new_tuple_i >, new_tuple_i >
            >,
            seps
        >{};

    struct basic_block_rule:
        pegtl::seq<
            seps,
            label_rule,
            pegtl::star< instruction_rule >,
            terminus_rule,
            seps
        >{};

    struct function_rule:
        pegtl::seq<
            seps,
            TAOCPP_PEGTL_STRING("define"),
            seps,
            return_type_rule,
            seps,
            label_rule,
            seps,
            one< '(' >,
            pegtl::star< parameter_rule >,
            one< ')' >,
            seps,
            one< '{' >,
            pegtl::plus< basic_block_rule >,
            one< '}' >,
            seps
        >{};
    
    struct entryPointRule:
        pegtl::plus< function_rule >{};

    struct grammar : 
        pegtl::must< entryPointRule >{};

    template< typename Rule >
    struct action : pegtl::nothing< Rule > {};

    Program parseFile(char* filename){
        pegtl::analyze< IR::grammar >();
        file_input< > fileInput(filename);
        IR::Program p;
        parse< IR::grammar, IR::action >(fileInput, p);
        return p;
    }
}