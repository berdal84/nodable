/*
	Macros to easily bind cpp functions to a Nodable::Language.
    These MACROS must be used within Nodable::Language non-static scope
*/

/**
* Wrap a native non-polymorphic function.
*
* ex: WRAP_FUNCTION( my_unique_name_function )
*/
#define WRAP_FUNCTION( function ) \
    { \
        std::string function_name = #function; \
        sanitizeFunctionName( function_name ); \
        using function_type = decltype(function); \
        IInvokable* invokable = new InvokableFunction<function_type>(function, function_name.c_str()); \
        addToAPI( invokable ); \
    }

/**
* Wrap a native non-polymorphic function.
*
* ex: WRAP_OPERATOR( my_unique_name_function, "+", 0, "+ Add")
*/
#define WRAP_OPERATOR( function, identifier, precedence, label ) \
    { \
        using function_type = decltype(function); \
        std::string function_name = identifier; \
        sanitizeOperatorFunctionName( function_name ); \
        IInvokable* invokable = new InvokableFunction<function_type>(function, function_name.c_str(), identifier ); \
        InvokableOperator* op = new InvokableOperator( invokable, precedence, identifier );\
        addOperator( op );\
        addToAPI( invokable ); \
    }

/**
 * Wrap a native function with a specific signature.
 * If your function is not polymorphic you can also use WRAP_FUNCTION
 *
 * ex: Same functions with two different signatures:
 *
 *  WRAP_POLYFUNC( sin, double(double) )
 *  WRAP_POLYFUNC( sin, double(float) )
 */
#define WRAP_POLYFUNC( function, function_type ) \
    { \
        std::string function_name = #function; \
        sanitizeFunctionName( function_name ); \
        IInvokable* invokable = new InvokableFunction<function_type>(function, function_name.c_str()); \
        addToAPI( invokable ); \
    }

/**
* Wrap a native function with a specific signature as an operator.
* If your function is not polymorphic you can also use WRAP_OPERATOR
*
* ex: Same function name with different label and signatures:
 *
*  WRAP_POLYOPER( add, "+", 0, "+ Add", double(double, double) )
*  WRAP_POLYOPER( add, "+", 0, "Cat.", std::string(std::string, double) )
*/
#define WRAP_POLYOPER( function, identifier, precedence, label, function_type ) \
    { \
        std::string function_name = identifier; \
        sanitizeOperatorFunctionName( function_name ); \
        IInvokable* invokable = new InvokableFunction<function_type>(function, function_name.c_str(), identifier ); \
        InvokableOperator* op = new InvokableOperator( invokable, precedence, identifier );\
        addOperator( op );\
        addToAPI( invokable ); \
    }
