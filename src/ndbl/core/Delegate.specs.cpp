#include "Delegate.h"
#include "ndbl/core/Log.h"
#include <gtest/gtest.h>

#include "ndbl/core/reflection/index.h"
#include "ndbl/core/Log.h"
#include "test/fixtures/basic_test.h"

using namespace ndbl;

typedef ::testing::Basic_Test Delegate_;
typedef ::testing::Basic_Test Simple_Delegate_;

TEST_F(Simple_Delegate_, default_constructor )
{
    Simple_Delegate d;
}

void my_static_function_the_answer()
{
}

TEST_F(Simple_Delegate_, call_static_function )
{
    Simple_Delegate d{&my_static_function_the_answer};
    d.call();
}

TEST_F(Delegate_, void_no_args__on_classes )
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

TEST_F(Delegate_, void_no_args__on_structs )
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

TEST_F(Delegate_, bind )
{
    struct MyStruct
    {
        bool ok = false;
        void set_ok() { ok = true; };
    };

    MyStruct obj;
    auto d = Simple_Delegate::from<&MyStruct::set_ok>();
    d.bind(&obj);
    d.call();
    EXPECT_TRUE(obj.ok);
}