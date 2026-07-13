#include "Delegate.h"
#include "tools/core/Log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(Simple_Delegate, default_constructor )
{
    Simple_Delegate d;
}

void my_static_function_the_answer()
{
}

TEST(Simple_Delegate, call_static_function )
{
    Simple_Delegate d{&my_static_function_the_answer};
    d.call();
}

TEST(Delegate, void_no_args__on_classes )
{
    static bool success = false;

    class MyClass
    {
    public:
        void method() { success = true; };
    };

    MyClass obj;
    auto d = Simple_Delegate::from<&MyClass::method>(&obj);
    d.call();

    EXPECT_TRUE(success);
}

TEST(Delegate, void_no_args__on_structs )
{
    struct MyStruct
    {
        bool ok = false;
        void set_ok() { ok = true; };
    };

    MyStruct obj;
    auto d = Simple_Delegate::from<&MyStruct::set_ok>(&obj);
    d.call();
    EXPECT_TRUE(obj.ok);
}

TEST(Delegate, bind )
{
    struct MyStruct
    {
        bool ok = false;
        void set_ok() { ok = true; };
    };

    MyStruct obj;
    auto d = Simple_Delegate::from<&MyStruct::set_ok>(nullptr);
    d.bind(&obj);
    d.call();
    EXPECT_TRUE(obj.ok);
}