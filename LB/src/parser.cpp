#include <parser.h>
#include <algorithm>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace std;
using namespace pegtl;

namespace LB {

    vector<LB::LB_Item*> parsedItems;
    vector<LB::Scope> active_scopes; 
    
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

    struct name:
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

    struct variable_rule:
        pegtl::seq<
            pegtl::one< '%' >,
            name
        > {};

    struct label_rule:
        pegtl::seq<
            pegtl::one< ':' >,
            name
        >{};

    struct tuple_rule:
        TAOCPP_PEGTL_STRING("tuple"){};

    struct code_rule:
        TAOCPP_PEGTL_STRING("code"){};
    
    struct void_rule:
        TAOCPP_PEGTL_STRING("void"){};

    struct int64_array_rule:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("int64"),
            pegtl::plus< TAOCPP_PEGTL_STRING("[]") >
        >{};


    struct int64_rule:
        TAOCPP_PEGTL_STRING("int64"){};

    struct type_rule:
        pegtl::sor<
            tuple_rule,
            code_rule,
            int64_array_rule,
            int64_rule
        >{};
        
    struct return_type_rule:
        pegtl::sor<
            tuple_rule,
            code_rule,
            void_rule,
            int64_rule
        >{};

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
            TAOCPP_PEGTL_STRING("<="),
            TAOCPP_PEGTL_STRING("<"),
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

    struct parameters_rule:
        pegtl::seq<
            parameter_rule,
            pegtl::star<
                pegtl::seq<
                    pegtl::one< ',' >,
                    parameter_rule
                >
            >
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

    struct vars_rule:
        pegtl::seq<
            variable_rule,
            pegtl::star<
                pegtl::seq<
                    seps,
                    one< ',' >,
                    seps,
                    variable_rule
                >
            >
        >{};
    
    struct type_vars_i:
        pegtl::seq<
            type_rule,
            seps,
            vars_rule
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
        pegtl::seq<
            variable_or_number,
            pegtl::star<
                seps,
                pegtl::one< ',' >,
                seps,
                variable_or_number
            >
        > {};


    struct runtime_callee_rule:
        pegtl::sor<
            TAOCPP_PEGTL_STRING("print"),
            TAOCPP_PEGTL_STRING("allocate"),
            TAOCPP_PEGTL_STRING("array-error")
        >{};

    struct function_callee_rule:
        name{}; 

    struct open_paren:
        pegtl::seq< 
            seps,
            one< '(' >
        >{};
    
    struct callee_rule:
        pegtl::sor<
            pegtl::seq<
                pegtl::at<
                    pegtl::seq<
                        runtime_callee_rule,
                        open_paren
                    >
                >,
                runtime_callee_rule
            >,
            pegtl::seq<
                pegtl::at<
                    pegtl::seq<
                        function_callee_rule,
                        open_paren
                    >
                >,
                function_callee_rule
            >,
            variable_rule
        >{};

    struct condition_rule:
        pegtl::seq<
            one< '(' >,
            seps,
            variable_or_number,
            seps,
            op_rule,
            seps,
            variable_or_number,
            seps,
            one< ')' >,
            seps,
            label_rule,
            seps,
            label_rule
        >{};

    struct if_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("if"),
            seps,
            condition_rule
        >{};

    struct while_i:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("while"),
            seps,
            condition_rule
        >{};
    
    struct continue_i:
        TAOCPP_PEGTL_STRING("continue"){};

    struct break_i:
        TAOCPP_PEGTL_STRING("break"){};

    struct call_rule:
        pegtl::seq<
            TAOCPP_PEGTL_STRING("call"),
            seps,
            callee_rule,
            seps,
            one< '(' >,
            seps,
            pegtl::opt< args_rule >,
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

    struct label_i:
        pegtl::seq< label_rule >{};

    struct instruction_rule:
        pegtl::seq<
            seps,
            pegtl::sor<
                pegtl::seq< pegtl::at< continue_i >, continue_i >,            
                pegtl::seq< pegtl::at< break_i >, break_i >,            
                pegtl::seq< pegtl::at< while_i >, while_i >,            
                pegtl::seq< pegtl::at< if_i >, if_i >,            
                pegtl::seq< pegtl::at< label_i >, label_i >,            
                pegtl::seq< pegtl::at< conditional_branch_i >, conditional_branch_i >,
                pegtl::seq< pegtl::at< branch_i >, branch_i >,
                pegtl::seq< pegtl::at< type_vars_i >, type_vars_i >,
                pegtl::seq< pegtl::at< assign_op_i >, assign_op_i >,
                pegtl::seq< pegtl::at< array_load_i >, array_load_i >,
                pegtl::seq< pegtl::at< array_store_i >, array_store_i >,
                pegtl::seq< pegtl::at< length_i >, length_i >,
                pegtl::seq< pegtl::at< call_i >, call_i >,
                pegtl::seq< pegtl::at< assign_call_i >, assign_call_i >,
                pegtl::seq< pegtl::at< new_array_i >, new_array_i >,
                pegtl::seq< pegtl::at< new_tuple_i >, new_tuple_i >,
                pegtl::seq< pegtl::at< return_value_i >, return_value_i >,
                pegtl::seq< pegtl::at< return_i >, return_i >,
                pegtl::seq< pegtl::at< assign_i >, assign_i >
            >,
            seps
        >{};

    struct new_scope_rule:
        pegtl::one< '{' >{};

    struct end_scope_rule:
        pegtl::one< '}' >{};

    struct function_name_rule:
        name {};

    // struct terminus_rule:
    //     pegtl::sor<
    //     >{};

    struct function_rule:
        pegtl::seq<
            seps,
            return_type_rule,
            seps,
            function_name_rule,
            seps,
            one< '(' >,
            seps,
            pegtl::opt< parameters_rule >,
            seps,
            one< ')' >,
            seps,
            // pegtl::one< '{' >,
            pegtl::plus<
                seps,
                pegtl::sor<
                    new_scope_rule,
                    instruction_rule,
                    end_scope_rule
                >,
                seps
            >,
            // seps,
            // terminus_rule,
            // seps,
            // pegtl::one< '}' >,
            seps
        >{};
    
    struct entryPointRule:
        pegtl::plus< function_rule >{};

    struct grammar : 
        pegtl::must< entryPointRule >{};

    template< typename Rule >
    struct action : pegtl::nothing< Rule > {};

    void cacheStringItem(LB_Item* i, std::string str){
        i->name = str;
        parsedItems.push_back(i);
    }

    void addToVarMap(Variable* v, Type* t ){
        active_scopes.back().type_map.emplace(v->name, t);
    }

    /*
    *  LB_Item Actions
    */

    template<> struct action < variable_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Variable(), in.string());
        }
    };

    template<> struct action < runtime_callee_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Runtime_Callee(), in.string());
        }
    };

    template<> struct action < function_callee_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Function_Name(), in.string());
        }
    };

    template<> struct action < label_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Label(), in.string());

        }
    };

    template<> struct action < number_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            auto n = new Number(in.string());
            n->value =  stoi(in.string());;
            parsedItems.push_back(n);
        }
    };

    template<> struct action < tuple_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Tuple(), in.string());
        }
    };

    template<> struct action < code_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Code(), in.string());
        }
    };

    template<> struct action < void_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Void(), in.string());
        }
    };

    template<> struct action < int64_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Int64(), in.string());
        }
    };

    template<> struct action < int64_array_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Int64_Array(), in.string());
        }
    };

    template<> struct action < op_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            cacheStringItem(new Operator(), in.string());
        }
    };

    /*
    *   Instruction Actions
    */

    void instructionAction(Instruction* inst, Program & p){
        inst->args = parsedItems;
        parsedItems.clear();
        active_scopes.back().instructions.push_back(inst);
    }

    template<> struct action < assign_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Assign_I(), p);
        }
    };

    template<> struct action < label_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Label_I(), p);
        }
    };

    template<> struct action < branch_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Branch_I(), p);
        }
    };

    template<> struct action < conditional_branch_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Conditional_Branch_I(), p);
        }
    };

    template<> struct action < return_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Return_I(), p);
        }
    };

    template<> struct action < return_value_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Return_Value_I(), p);
        }
    };

    template<> struct action < type_vars_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            // auto type = dynamic_cast<Type*>(parsedItems[0]);
            // for(auto it = parsedItems.begin()+1; it != parsedItems.end(); ++it){
            //     auto var = dynamic_cast<Variable*>(*it);
            //     addToVarMap(var, type);
            // }
            instructionAction(new Type_Var_I(), p);
        }
    };

    template<> struct action < assign_op_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Assign_Cond_I(), p);
        }
    };

    template<> struct action < array_load_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction( new Array_Load_I(), p);
        }
    };

    template<> struct action < array_store_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction( new Array_Store_I(), p);
        }
    };

    template<> struct action < length_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Length_I(), p);
        }
    };

    template<> struct action < call_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Call_I(), p);
        }
    };

    template<> struct action < assign_call_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Assign_Call_I(), p);
        }
    };

    template<> struct action < new_tuple_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new New_Tuple_I(), p);
        }
    };

    template<> struct action < new_array_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new New_Array_I(), p);
        }
    };

    template<> struct action < continue_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Continue_I(), p);
        }
    };

    template<> struct action < break_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new Break_I(), p);
        }
    };

    template<> struct action < while_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new While_I(), p);
        }
    };

    template<> struct action < if_i > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            instructionAction(new If_I(), p);
        }
    };

    /*
    *   Function-Level Actions
    */

   template<> struct action < new_scope_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            active_scopes.push_back( Scope() );
        }
    };

    template<> struct action < end_scope_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            auto scope = utils::pop_item(active_scopes);
            if(active_scopes.empty()){
                p.functions.back()->scope = scope;
            } else {
                auto i = new Scope_I();
                i->scope = scope;
                active_scopes.back().instructions.push_back(i);

            }
        }
    };

    template<> struct action < function_name_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;

            auto f = new Function();
            f->name.name = in.string();
            auto type = dynamic_cast<Type*>(utils::pop_item(parsedItems));
            f->returnType = type;            
            p.functions.push_back(f);
        }
    };
    
    template<> struct action < parameter_rule > {
        template< typename Input >
        static void apply( const Input & in, LB::Program & p){
            // cout << in.string() << endl;
            auto var_cast = dynamic_cast<Variable*>(utils::pop_item(parsedItems));
            auto type_cast = dynamic_cast<Type*>(utils::pop_item(parsedItems));

            // addToVarMap(var_cast, type_cast);

            LB::Parameter param ( type_cast, *var_cast );

            p.functions.back()->parameters.push_back(param);
        }
    };

    Program parseFile(char* filename){
        pegtl::analyze< LB::grammar >();
        file_input< > fileInput(filename);
        LB::Program p;
        parse< LB::grammar, LB::action >(fileInput, p);
        return p;
    }
}