
# Coding Guidelines

Notes to myself. B.

- Think data oriented,
- Allocation and initialization must be separated,
- Avoid custom constructors/destructors: data should have my_struct_init/deinit(My_Struct*)functions instead,
- Use flat structs as much as possible, avoid methods as much as possible.
- Use templates only if no choice (typed-unions are more flexible, and faster to compile)

## Classes / Structs

### Declaration

```c++
namespace ndbl // lower case, short
{
    struct Struct_Name 
    {
        // ...
    };
}
```

### Getters/Setters

#### General

Try not to use any when possible. I want plain flat structs as much as possible.

```c++
struct My_Data
{
    int     value;
    char*   name;
}
```

In case I want some side effects before or after setting a member, I can write a C-style API function like this:

```c++
void my_data_set_value(My_Data* data, int new_value)
{
    changed = data->value != new_value;

    if (changed)
    {
        // ... some code here
    }

    data->value   = new_value;

    if (changed)
    {
        // ... or some code here
    }
}
```

or

```c++
bool my_data_set_value(My_Data* data, int new_value)
{
    changed = data->value != new_value;
    data->value   = new_value;
    return changed;
}
```

In case I really need to, I would write getter/setter like this:

```c++
struct My_Data
{
    inline int& value()             { return _value; }
    inline int  value() const       { return _value; }
    void        set_value(int val)  { _value = val; /* side effects here */ }
private:
    int _value; // or _value
}
```

Note that if I switch from simple member method `int value` to getter/setter, the compiler will warn us at each call site, and it will be very easy to fix the code that does not compile.


When a value is computed only, and if that computation is not trivial, should implement a function.

```c++
int compute_something(const My_Data& data)
{
    // compute results
    int result = ...

    return result;
}
```

I don't want to polute the struct with a `compute_something`member, except if the computation is cheap and easy to use from a syntactic point of view.

```c++
struct My_Data
{
    u8_t  value; // (u8_t)-1 means "no value"
    inline bool has_value() const { return value != -1; }
}
```

#### Alias members

Jai programming language proposes a nice feature to add aliases to existing members as a different typ (using `#place` or `#override`). I cannot do the same with C++, however, I could do some nice MACROS to make it easier.

Here is an example, I want to add some aliases to `Vec4` to get a `Vec2` (xy) and an `Vec3` (xyz) from it. I would do this manually:

```c++
struct Vec2
{
    float x, y;
}

struct Vec4
{
    float x, y, z, w;

    Vec2* xy()             { return reinterpret_cast<Vec2*>(this) };
    const Vec2* xy() const { return reinterpret_cast<const Vec2*>(this) };

    Vec3* xyz()             { return reinterpret_cast<Vec3*>(this) };
    const Vec3* xyz() const { return reinterpret_cast<const Vec3*>(this) };
}
```

This is very annoying to write, so I added some macros in [src/tools/core/reflection/GETTERS_SETTERS.h]().

Here is a simple usage example:

```c++
struct Vec4
{
    float x, y, z, w;
    GETTERS_reinterpret_cast(Vec2*, xy,  this)
    GETTERS_reinterpret_cast(Vec3*, xyz, this)
}
```



## Nesting

When code is sparse, omitting braces is file if I add an empty line before and after

```c++
if ( condition )
    // a single line of code
else if ( else_condition )
    // a single line of code
else
    // a single line of code
```

The objective is readability, not compactness. So, when code is dense, I might add braces aligned on the left.

```c++
if ( condition )
{
    //
    // multiple lines of code ...
    //
}
else if ( else_condition )
{
    //
    // multiple lines of code ...
    //
}
else
{
    //
    // multiple lines of code ...
    //
}
```

Instead of making a "complex" condition with several `&&`, using multiple `if` is easier to read. It also gives the opportunity to add a comment for each and also to disable/enable them line by line.

```c++
if ( condition1 ) // because ...
    if ( condition2 ) // because ...
        if (condition 3 ) // and also, ...
            do_this();
```

## The `auto` keyword

Use it only when it reduces redundancy.
Here, `var`'s type is obviously `My_Data`.

```c++
auto var = new My_Data();
```

Replacing `auto` by `My_Data` is unnecessary.

```c++
My_Data var = new My_Data();
```

However, in the following code, the use of `auto` leads to a less readable code. We can't tell `var`'s type without to check manually `get_name()`'s signature.

```c++
if ( auto var = some_instance.get_name() )
{
    // ...    
}
```

I would do this instead:

```c++
if ( const char* name = some_instance.get_name() )
{
    // ...    
}
```

Yes, it was a `const char*`, so it can be converted to a `bool`.