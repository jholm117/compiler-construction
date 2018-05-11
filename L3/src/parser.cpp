#include <parser.h>
#include <vector>


#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace std;
using namespace pegtl;

namespace L3{

    std::vector<L3::L3_Item*> parsedItems;

    L3_Item* pop_item(){
        auto ret = parsedItems.back();
        parsedItems.pop_back();
        return ret;
    }

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
    
    struct keyword:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("return"),
            TAOCPP_PEGTL_STRING("call"),
            TAOCPP_PEGTL_STRING("br"),
            TAOCPP_PEGTL_STRING("store"),
            TAOCPP_PEGTL_STRING("load"),
            TAOCPP_PEGTL_STRING("print"),
            TAOCPP_PEGTL_STRING("array-error"),
            TAOCPP_PEGTL_STRING("allocate")
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

    struct variable:
        pegtl::seq<
            pegtl::not_at< keyword >,
            var
        > {};

    struct label:
        pegtl::seq<
            pegtl::one<':'>,
            var
        > {};
    
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
    
    struct op:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("+"),
            TAOCPP_PEGTL_STRING("-"),
            TAOCPP_PEGTL_STRING("*"),
            TAOCPP_PEGTL_STRING("&"),
            TAOCPP_PEGTL_STRING("<<"),
            TAOCPP_PEGTL_STRING(">>")
        >{};
    
    struct cmp:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("<"),
            TAOCPP_PEGTL_STRING("<="),
            TAOCPP_PEGTL_STRING("="),
            TAOCPP_PEGTL_STRING(">="),
            TAOCPP_PEGTL_STRING(">")
        >{};
    
    struct arrow:
        TAOCPP_PEGTL_STRING("<-") {};

    struct runtime_function:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("print"),
            TAOCPP_PEGTL_STRING("allocate"),
            TAOCPP_PEGTL_STRING("array-error")
        >{};

    struct source:
        pegtl::sor<
            number,
            label,
            variable
        >{};
    
    struct callee:
        pegtl::sor<
            runtime_function,
            variable,
            label
        >{};
    
    struct return_i:
        TAOCPP_PEGTL_STRING("return"){};
    
    struct return_var_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("return"),
            seps,
            pegtl::sor<
                variable,
                number
            >
        >{};
    
    struct assign_i:
        pegtl::seq<
            variable,
            seps,
            arrow,
            seps,
            source
        >{};

    struct assign_op_i:
        pegtl::seq<
            variable,
            seps,
            arrow,
            seps,
            source,
            seps,
            op,
            seps,
            source
        > {};
    
    struct assign_comp_i:
        pegtl::seq<
            variable,
            seps,
            arrow,
            seps,
            source,
            seps,
            cmp,
            seps,
            source
        >{};

    struct load_i:
        pegtl::seq<
            variable,
            seps,
            arrow,
            seps,
            TAOCPP_PEGTL_STRING("load"),
            seps,
            variable
        >{};
    
    struct store_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("store"),
            seps,
            variable,
            seps,
            arrow,
            seps,
            source
        >{};
    
    struct branch_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("br"),
            seps,
            label
        >{};

    struct conditional_branch_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("br"),
            seps,
            variable,
            seps,
            label,
            seps,
            label
        >{};
    
    struct label_i:
        pegtl::seq<label> {};

    struct args:
        pegtl::sor<
            pegtl::seq<
                source,
                pegtl::star<
                    seps,
                    pegtl::one< ',' >,
                    seps,
                    source
                >
            >,
            pegtl::success   
        > {};

    struct params:
        args{};

    struct call:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("call"),
            seps,
            callee,
            seps,
            pegtl::one< '(' >,
            seps,
            args,
            seps,
            pegtl::one< ')'>
        >{};

    struct call_i:
        call {};

    struct assign_call_i:
        pegtl::seq <
            variable,
            seps,
            arrow,
            seps,
            call
        >{};

    struct instructionRule:
        pegtl::seq<
            seps,
            pegtl::sor<
                pegtl::seq<pegtl::at<return_var_i>, return_var_i>,
                pegtl::seq<pegtl::at<return_i>, return_i>,
                pegtl::seq<pegtl::at<call_i>, call_i>,
                pegtl::seq<pegtl::at<label_i>, label_i>,
                pegtl::seq<pegtl::at<conditional_branch_i>, conditional_branch_i>,
                pegtl::seq<pegtl::at<branch_i>, branch_i>,
                pegtl::seq<pegtl::at<store_i>, store_i>,
                pegtl::seq<pegtl::at<load_i>, load_i>,
                pegtl::seq<pegtl::at<assign_comp_i>, assign_comp_i>,
                pegtl::seq<pegtl::at<assign_op_i>, assign_op_i>,
                pegtl::seq<pegtl::at<assign_call_i>, assign_call_i>,                
                pegtl::seq<pegtl::at<assign_i>, assign_i>
            >,
            seps
        >{};

    struct functionNameRule:
        label{};
        
    struct functionRule:
        pegtl::seq<
            seps,
            TAOCPP_PEGTL_STRING("define"),
            seps,
            functionNameRule,
            seps,
            pegtl::one< '(' >,
            seps,
            params,
            seps,
            pegtl::one< ')' >,
            seps,
            pegtl::one< '{' >,
            pegtl::star< instructionRule >,
            pegtl::one< '}'>,
            seps
        > {};

    struct entryPointRule:
        pegtl::seq<
            seps,
            pegtl::star< functionRule >,
            seps
        > {};

    struct grammar:
        pegtl::must<
            entryPointRule
        > {};

    void cacheOperator(std::string str){
        L3::Operator* n = new L3::Operator();
        n->op = stringToOperatorType(str);
        parsedItems.push_back(n);
    }

    void instructionAction(Instruction* i, Program& p){
        i->args = parsedItems;
        parsedItems.clear();
        p.functions.back()->instructions.push_back(i);
    }

    template< typename Rule >
    struct action : pegtl::nothing< Rule > {};

    /*
        L3_Item Actions
    */

    template<> struct action < number > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            L3::Number* n = new L3::Number();
            n->value = std::stoi(in.string());
            parsedItems.push_back(n);
        }
    };

    template<> struct action < label > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            auto n = new L3::Label();
            n->name = in.string();
            parsedItems.push_back(n);
            if(in.string().length() > p.longestLabel.length()){
                p.longestLabel = in.string();
            }
        }
    };

    template<> struct action < variable > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            L3::Variable* n = new L3::Variable();
            n->name = in.string();
            parsedItems.push_back(n);
        }
    };

    template<> struct action < runtime_function > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            auto r = new L3::Runtime_Function();
            r->name = in.string();
            parsedItems.push_back(r);
        }
    };

    template<> struct action < op > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            cacheOperator(in.string());
        }
    };

    template<> struct action < cmp > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            cacheOperator(in.string());
        }
    };

    /*
        Instruction Actions
    */

    template<> struct action < assign_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Assign_I(), p);
        }
    };

    template<> struct action < return_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction( new Return_I() , p);
        }
    };

    template<> struct action < return_var_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Return_Value_I(), p);
        }
    };
    
    template<> struct action < assign_call_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Assign_Call_I(), p);
        }
    };

    template<> struct action < assign_comp_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Assign_Op_I(), p);
        }
    };

    template<> struct action < assign_op_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Assign_Op_I(), p);
        }
    };

    template<> struct action < load_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Load_I(), p);
        }
    };

    template<> struct action < store_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Store_I(), p);
        }
    };

    template<> struct action < conditional_branch_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Conditional_Branch_I(), p);
        }
    };

    template<> struct action < call_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Call_I(), p);
        }
    };

    template<> struct action < label_i > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            instructionAction(new Label_I(), p);
        }
    };

    template<> struct action < functionNameRule > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            auto l = new Label();
            l->name = in.string();
            Function* f = new Function();
            f->name = (Label*)l;
            p.functions.push_back(f);
        }
    };

    template<> struct action < params > {
        template< typename Input >
        static void apply( const Input & in, L3::Program & p){
            for (auto item : parsedItems) 
                p.functions.back()->arguments.push_back((Variable*) item);
            parsedItems.clear();
        }
    };

    Program parseFile(char* filename){
        pegtl::analyze< L3::grammar >();
        file_input< > fileInput(filename);
        L3::Program p;
        parse< L3::grammar, L3::action >(fileInput, p);
        return p;
    }
}
