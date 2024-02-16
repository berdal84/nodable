#pragma once

/** REFLECT_BASE_CLASS is replaced by a get_type() function if inserted in a class body */
#define REFLECT_BASE_CLASS() \
public:\
    virtual const fw::type* get_type() const { return fw::type::get<decltype(*this)>(); }\
private:

/** REFLECT_DERIVED_CLASS is replaced by a get_type() function if inserted in a class body. Must have a parent class with REFLECT_BASE_CLASS */
#define REFLECT_DERIVED_CLASS() \
public:\
    virtual const fw::type* get_type() const override { return fw::type::get<decltype(*this)>(); }\
private:

