#pragma once

/** REFLECT_BASE_CLASS is replaced by a get_type() function if inserted in a class body */
#define REFLECT_BASE_CLASS() \
public:\
    virtual const tools::ClassDesc* get_class() const \
    { return tools::type::get_class(this); }\
private:

/** REFLECT_DERIVED_CLASS is replaced by a get_type() function if inserted in a class body. Must have a parent class with REFLECT_BASE_CLASS */
#define REFLECT_DERIVED_CLASS() \
public:\
    virtual const tools::ClassDesc* get_class() const override \
    { return tools::type::get_class(this); }\
private:

