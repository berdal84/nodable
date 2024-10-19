
# Coding Guidelines

Guidelines to myself, still WIP.

## General
### `auto` keyword

Use it only when it reduces redundancy.
Here, `var`'s type is obviously `MyClass`. Replacing `auto` by `MyClass` is unnecessary.
```c++
auto var = new MyClass();
```
However, in the following code, using auto lead to a less readable code. What's the type of `var`? We have to use our IDE... it's less readable.
```c++
if ( auto& var = some_instance.get_name() )
{
    // ...    
}
```
We should do this instead (using `std::string` here as an example).
```c++
if ( std::string& var = some_instance.get_name() )
{
    // ...    
}

```
## Naming

### Classes

#### Declaration

```c++
namespace ndbl // lower case, short
{
    class MyClassName // CamelCase
    {
    public:
        // ...
    private:
        // ...
    };
}
```

#### Getters/Setters

When we add a getter/setter for a given member, we do the following:
```c++
class SomeClass
{
public:
    int& value() { return m_value; }
    int  value() const { return m_value; }
    void set_value(int val) { m_value = val; }
private:
    int m_value; // or _value
```

When a value is computed only, and if that computation is not trivial, we do:
```c++
class SomeClass
{
public:
    int calc_sum() const { ... }
    int compute_sum() const { ... } // or
```

## Nesting

When code is sparse, omitting braces is file if we add an empty line before and after

```c++

if ( condition )
    do_this();
else if ( else_condition )
    do_that();
else
    do_something_else();

```

When code is dense, we might add braces aligned vertically with the keyword if

```c++

if ( condition )
{
    do_this();
    // more code ...
}
else if ( else_condition )
{
    do_that();
    // more code ...
}
else
{
    do_something_else();
    // more code ...
}

```

Instead of making a "complex" condition with several `&&`, using multiple `if` is easier to read. It also gives the opportunity to comment each of the conditions:

```c++

if ( condition1 ) // because ...
    if ( condition2 ) // because ...
        if (condition 3 ) // and also, ...
            do_this();
```