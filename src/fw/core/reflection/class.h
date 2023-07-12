#pragma once

#define REFLECT_BASE_CLASS() \
public:\
    virtual const fw::type* get_type() const { return fw::type::get<decltype(*this)>(); }\
private:

#define REFLECT_DERIVED_CLASS(...) \
public:\
    virtual const fw::type* get_type() const override { return fw::type::get<decltype(*this)>(); }\
private:

