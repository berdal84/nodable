#include "Delegate.h"
#include "tools/core/log.h"
#include <gtest/gtest.h>

using namespace tools;

TEST(Delegate, default_constructor )
{
    Delegate<void> d{};
}

TEST(Delegate, default_call )
{
    Delegate<void> d{};
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
    auto d = Delegate<void>::from_method<&MyClass::method>(&obj);
    d.call();

    EXPECT_TRUE(success);
}

TEST(Delegate, void_no_args__on_structs )
{
    static bool success = false;

    struct MyStruct
    {
        void method() { success = true; };
    };

    MyStruct obj;
    auto d = Delegate<void>::from_method<&MyStruct::method>(&obj);
    d.call();

    EXPECT_TRUE(success);
}

TEST(Delegate, bind )
{
    static bool success = false;

    struct MyStruct
    {
        void method() { success = true; };
    };

    MyStruct obj;

    auto d = Delegate<void>::from_method<&MyStruct::method>();
    d.bind(&obj);
    d.call();

    EXPECT_TRUE(success);
}