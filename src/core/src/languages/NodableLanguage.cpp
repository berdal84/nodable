#include <nodable/core/languages/NodableLanguage.h>

#include <nodable/core/String.h>
#include <nodable/core/System.h>
#include <nodable/core/Operator.h>
#include <nodable/core/reflection/invokable.h>
#include <nodable/core/languages/NodableLibrary_math.h>
#include <nodable/core/languages/NodableLibrary_biology.h>

using namespace ndbl;

NodableLanguage::NodableLanguage(): m_parser(*this), m_serializer(*this)
{
    // ignored
    add_regex(std::regex("(//(.+?)($|\n))"), Token_t::ignore); // Single line
    add_regex(std::regex("(/\\*(.+?)\\*/)"), Token_t::ignore); // Multi line

    // keywords
    add_string(k_keyword_if           , Token_t::keyword_if);
    add_string(k_keyword_else         , Token_t::keyword_else);
    add_string(k_keyword_for          , Token_t::keyword_for);
    add_string(k_keyword_operator     , Token_t::keyword_operator);
    add_type(type::get<bool>()        , Token_t::keyword_bool    , k_keyword_bool);
    add_type(type::get<std::string>() , Token_t::keyword_string  , k_keyword_string);
    add_type(type::get<double>()      , Token_t::keyword_double  , k_keyword_double);
    add_type(type::get<i16_t>()       , Token_t::keyword_int     , k_keyword_int);

    // punctuation
    add_char(k_tab                , Token_t::ignore );
    add_char(k_space              , Token_t::ignore);
    add_char(k_open_curly_brace   , Token_t::scope_begin );
    add_char(k_close_curly_brace  , Token_t::scope_end);
    add_char(k_open_bracket       , Token_t::fct_params_begin);
    add_char(k_close_bracket      , Token_t::fct_params_end );
    add_char(k_coma               , Token_t::fct_params_separator);
    add_char(k_semicolon          , Token_t::end_of_instruction);
    add_char(k_end_of_line        , Token_t::end_of_line);

    // literals
    // USE OTHER METHOD: insert(std::regex("(true|false)")                , Token_t::literal_bool  , type::get<bool>());
    add_regex(std::regex(R"(("[^"]*"))")                , Token_t::literal_string , type::get<std::string>());
    add_regex(std::regex("(0|([1-9][0-9]*))(\\.[0-9]+)"), Token_t::literal_double , type::get<double>());
    add_regex(std::regex("(0|([1-9][0-9]*))")           , Token_t::literal_int    , type::get<i16_t>());

    // identifier
    add_regex(std::regex("([a-zA-Z_]+[a-zA-Z0-9]*)"), Token_t::identifier);

    // operators
    add_regex(std::regex("(<=>)"), Token_t::operator_); // 3 chars
    add_regex(std::regex("([=\\|&]{2}|(<=)|(>=)|(=>)|(!=))"), Token_t::operator_); // 2 chars
    add_regex(std::regex("[/+\\-*!=<>]"), Token_t::operator_); // single char

    add_operator( "-"  , Operator_t::Unary, 5); // --------- unary (sorted by precedence)
    add_operator( "!"  , Operator_t::Unary, 5);

    add_operator( "/"  , Operator_t::Binary , 20); // ------- binary (sorted by precedence)
    add_operator( "*"  , Operator_t::Binary , 20);
    add_operator( "+"  , Operator_t::Binary , 10);
    add_operator( "-"  , Operator_t::Binary , 10);
    add_operator( "||" , Operator_t::Binary , 10);
    add_operator( "&&" , Operator_t::Binary , 10);
    add_operator( ">=" , Operator_t::Binary , 10);
    add_operator( "<=" , Operator_t::Binary , 10);
    add_operator( "=>" , Operator_t::Binary , 10);
    add_operator( "==" , Operator_t::Binary , 10);
    add_operator( "<=>", Operator_t::Binary , 10);
    add_operator( "!=" , Operator_t::Binary , 10);
    add_operator( ">"  , Operator_t::Binary , 10);
    add_operator( "<"  , Operator_t::Binary , 10);
    add_operator( "="  , Operator_t::Binary , 0);

    load_library<NodableLibrary_math>();
    load_library<NodableLibrary_biology>();
}

NodableLanguage::~NodableLanguage()
{
}

std::shared_ptr<const iinvokable> NodableLanguage::find_function(const func_type* _signature) const
{
    auto is_compatible = [&](std::shared_ptr<const iinvokable> fct)
    {
        return fct->get_type().is_compatible(_signature);
    };

    auto it = std::find_if(m_functions.begin(), m_functions.end(), is_compatible);

    if (it != m_functions.end())
    {
        return *it;
    }

    return nullptr;
}

std::shared_ptr<const iinvokable> NodableLanguage::find_operator_fct_exact(const func_type* _type) const
{
    if(!_type)
    {
        return nullptr;
    }

    auto is_exactly = [&](std::shared_ptr<const iinvokable> _invokable)
    {
        return _type->is_exactly(&_invokable->get_type());
    };

    auto found = std::find_if(m_operators_impl.cbegin(), m_operators_impl.cend(), is_exactly );

    if (found != m_operators_impl.end() )
    {
        return *found;
    }

    return nullptr;
}

std::shared_ptr<const iinvokable> NodableLanguage::find_operator_fct(const func_type* _type) const
{
    if(!_type)
    {
        return nullptr;
    }
    auto exact = find_operator_fct_exact(_type);
    if( !exact) return find_operator_fct_fallback(_type);
    return exact;
}

std::shared_ptr<const iinvokable> NodableLanguage::find_operator_fct_fallback(const func_type* _type) const
{

    auto is_compatible = [&](std::shared_ptr<const iinvokable> _invokable)
    {
        return _type->is_compatible(&_invokable->get_type());
    };

    auto found = std::find_if(m_operators_impl.cbegin(), m_operators_impl.cend(), is_compatible );

    if (found != m_operators_impl.end() )
    {
        return *found;
    }

    return nullptr;
}

void NodableLanguage::add_function(std::shared_ptr<const iinvokable> _invokable)
{
    m_functions.push_back(_invokable);

    const func_type* type = &_invokable->get_type();

    std::string type_as_string;
    m_serializer.serialize(type_as_string, type);

    // Stops if no operator having the same identifier and argument count is found
    if( !find_operator(type->get_identifier(), static_cast<Operator_t >( type->get_arg_count() )  ))
    {
        LOG_VERBOSE("NodableLanguage", "add function: %s (in m_functions)\n", type_as_string.c_str() );
        return;
    }

    // Register the invokable as an operator implementation
    auto found = std::find(m_operators_impl.begin(), m_operators_impl.end(), _invokable);
    NDBL_ASSERT( found == m_operators_impl.end() )
    m_operators_impl.push_back(_invokable);
    LOG_VERBOSE("NodableLanguage", "add operator: %s (in m_functions and m_operator_implems)\n", type_as_string.c_str() );
}

void NodableLanguage::add_operator(const char* _id, Operator_t _type, int _precedence)
{
    const Operator* op = new Operator(_id, _type, _precedence);

    NDBL_ASSERT( std::find( m_operators.begin(), m_operators.end(), op) == m_operators.end() )
    m_operators.push_back(op);
}

const Operator* NodableLanguage::find_operator(const std::string& _identifier, Operator_t _type) const
{
    auto is_exactly = [&](const Operator* op)
    {
        return op->identifier == _identifier && op->type == _type;
    };

    auto found = std::find_if(m_operators.cbegin(), m_operators.cend(), is_exactly );

    if (found != m_operators.end() )
        return *found;

    return nullptr;
}


void NodableLanguage::add_regex(const std::regex& _regex, Token_t _token_t)
{
    m_token_regex.push_back(_regex);
    m_token_t_by_regex_index.push_back(_token_t);
}

void NodableLanguage::add_regex(const std::regex& _regex, Token_t _token_t, type _type)
{
    m_token_regex.push_back(_regex);
    m_token_t_by_regex_index.push_back(_token_t);

    m_type_regex.push_back(_regex);
    m_type_by_regex_index.push_back(_type);

    m_token_type_keyword_to_type.insert({_token_t, _type});
    m_type_hashcode_to_token_t.insert({_type.hash_code(), _token_t});
}

void NodableLanguage::add_type(type _type, Token_t _token_t, std::string _string)
{
    m_token_type_keyword_to_type.insert({_token_t, _type});
    m_type_hashcode_to_token_t.insert({_type.hash_code(), _token_t});
    m_type_hashcode_to_string.insert({_type.hash_code(), _string});
    add_string(_string, _token_t);
}

void NodableLanguage::add_string(std::string _string, Token_t _token_t)
{
    m_token_t_to_string.insert({_token_t, _string});
    add_regex(std::regex("(" + _string + ")"), _token_t);
}

void NodableLanguage::add_char(const char _char, Token_t _token_t)
{
    m_token_t_to_char.insert({_token_t, _char});
    m_char_to_token_t.insert({_char, _token_t});
}

std::string& NodableLanguage::to_string(std::string& _out, type _type) const
{
    auto found = m_type_hashcode_to_string.find(_type.hash_code());
    if( found != m_type_hashcode_to_string.cend() )
    {
        return _out.append( found->second );
    }
    return _out;
}

std::string& NodableLanguage::to_string(std::string& _out, Token_t _token_t) const
{
    switch (_token_t) {
        case Token_t::ignore: return _out.append("ignore");
        case Token_t::operator_: return _out.append("operator");
        case Token_t::identifier: return _out.append("identifier");
    }

    {
        auto found = m_token_t_to_char.find(_token_t);
        if (found != m_token_t_to_char.cend())
        {
            _out.push_back( found->second );
            return _out;
        }
    }
    auto found = m_token_t_to_string.find(_token_t);
    if (found != m_token_t_to_string.cend())
    {
        return _out.append( found->second );
    }

    return  _out.append("<?>");
}

std::string NodableLanguage::to_string(type _type) const
{
    std::string result;
    return to_string(result, _type);
}

std::string NodableLanguage::to_string(Token_t _token) const
{
    std::string result;
    return to_string(result, _token);
}

int NodableLanguage::get_precedence(const iinvokable* _invokable) const
{
    if( !_invokable ) return std::numeric_limits<int>::min(); // default

    auto type = _invokable->get_type();
    auto oper = find_operator(type.get_identifier(), static_cast<Operator_t>(type.get_arg_count()) );

    if( !oper ) return 0; // default

    return oper->precedence;
}
type NodableLanguage::get_type(Token_t _token) const
{
    NDBL_EXPECT(is_a_type_keyword(_token), "_token_t is not a type keyword!");
    return m_token_type_keyword_to_type.find(_token)->second;
}
