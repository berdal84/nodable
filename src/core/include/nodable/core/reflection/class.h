#pragma once

#define REFLECT_BASE_CLASS() \
public:\
    virtual type get_type() const { return type::get<decltype(*this)>(); }\
private:

#define REFLECT_DERIVED_CLASS(...) \
public:\
    virtual type get_type() const override { return type::get<decltype(*this)>(); }\
private:

