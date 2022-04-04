/*
	Macros to easily bind cpp functions to a Nodable::Language.
    These MACROS must be used within Nodable::Language non-static scope
*/

/**
* Wrap a native function with a specific signature.
* If your function is not polymorphic you can also use WRAP_FUNCTION
*
* ex: Same functions with two different signatures:
*
*  POLYFUNC( sin, double(double) )
*  POLYFUNC( sin, double(float) )
*/
#define POLYFUNC( function, function_type ) \
    { \
        std::string identifier = #function; \
        sanitize_function_identifier( identifier );\
        const IInvokable* invokable_fct = new InvokableFunction<function_type>(function, identifier.c_str()); \
        add( invokable_fct );\
    }

/**
* Wrap a native function with a specific signature as an operator.
* If your function is not polymorphic you can also use OPER
*
* ex: Same function name with different label and signatures:
 *
*  POLYOPER( add, "+", "+ Add", double(double, double) )
*  POLYOPER( add, "+", "Cat.", std::string(std::string, double) )
*/
#define POLYOPER( function, identifier, function_type ) \
    { \
        std::string function_identifier = identifier; \
        sanitize_operator_fct_identifier( function_identifier );\
        const IInvokable* invokable = new InvokableFunction<function_type>(function, function_identifier.c_str(), identifier ); \
        const Operator* op = find_operator(identifier, invokable->get_signature());\
        const InvokableOperator* invokable_op = new InvokableOperator( op, invokable );\
        add( invokable_op );\
    }

/**
* Wrap a native non-polymorphic function.
*
* ex: FUNC( my_unique_name_function )
*/
#define FUNC( function ) POLYFUNC( function, decltype(function))

/**
* Wrap a native non-polymorphic function.
*
* ex: WRAP_OPERATOR( my_unique_name_function, "+", "+ Add")
*/
#define OPER( function, identifier ) POLYOPER( function, identifier, decltype(function) )