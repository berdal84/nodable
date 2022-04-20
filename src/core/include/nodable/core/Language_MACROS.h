/*
	Macros to easily bind cpp functions to a Nodable::ILanguage.
    These MACROS must be used within Nodable scope scope

    example:

    void NodableLanguage_biology::bind_to_language(ILanguage* _language) const
    {
        BIND_TO(_language)
        BIND_FUNCTION(dna_to_protein)
    }

*/

#include <nodable/core/ILanguage.h>
#include <nodable/core/SignatureBuilder.h>
#include <nodable/core/reflection/invokable.h>

#define BIND_TO( language ) \
ILanguage* __language = language;

/**
* Wrap a native function with a specific signature.
* If your function is not polymorphic you can also use BIND_FUNCTION
*
* ex: Same functions with two different signatures:
*
*  BIND_FUNCTION_T( sin, double(double) )
*  BIND_FUNCTION_T( sin, double(float) )
*/
#define BIND_FUNCTION_T( func, func_t ) \
    { \
        func_type* type = SignatureBuilder<func_t>::new_function(#func, __language); \
        __language->add_invokable( new invokable<func_t>(func, type ) );\
    }

/**
* Wrap a native function with a specific signature as an operator.
* If your function is not polymorphic you can also use BIND_OPERATOR
*
* ex: Same function name with different label and signatures:
 *
*  BIND_OPERATOR_T( add, "+", double(double, double) )
*  BIND_OPERATOR_T( add, "+", std::string(std::string, double) )
*/
#define BIND_OPERATOR_T( func, id, func_t ) \
    { \
        func_type* type = SignatureBuilder<func_t>::new_operator(id, __language);\
        __language->add_invokable( new invokable<func_t>(func, type ) );\
    }

/**
* Wrap a native non-polymorphic function.
*
* ex: BIND_FUNCTION( my_unique_name_function )
*/
#define BIND_FUNCTION( function ) BIND_FUNCTION_T( function, decltype(function))

/**
* Wrap a native non-polymorphic function.
*
* ex: BIND_OPERATOR( my_unique_name_function, "+")
*/
#define BIND_OPERATOR( function, identifier ) BIND_OPERATOR_T( function, identifier, decltype(function) )