#include <cstdio>

#define BDC_DEBUG_ALLOCATORS
#define BDC_ENABLE_LOGS
#include "Allocators.hpp"
#include "Array.hpp"
#include "String.hpp"
#include "String_Builder.hpp"
#include "String_Hash.hpp"
#include "Hash_Map.hpp"
// we include *.cpp to compile as a single translation unit
#include "Allocators.cpp"
#include "Array.cpp"
#include "String.cpp"
#include "String_Builder.cpp"
#include "String_Hash.cpp"
#include "Hash_Map.cpp"
//
#include "Testing.hpp"

using namespace bdc;

#define hashmap_print(hashmap)\
{ \
    printf("Printing non occupied slots (%u entries in total):\n", hashmap.entries.size ); \
    if( hashmap.entries.size == 0 ) printf("    (empty)\n"); \
    HASHMAP_WALK( entry, hashmap ) \
    { \
        if( entry.state == 0) continue; \
        printf("    #%i | state: %i | hash: %#010x | key: \"%s\" | value: \"%s\" \n", i, entry.state, entry.hash, entry.key.c_str(), entry.value.c_str() ); \
    } \
    HASHMAP_WALK_END \
    printf(" --- \n"); \
}


int main()
{

    TEST_SUITE_BEGIN(Memory_Manager)
    {
        #ifdef BDC_DEBUG_ALLOCATORS
        TEST_BEGIN( "temp_allocator: malloc + realloc with a malloc in between")
        {
            memory_manager_init();

            void* data = temp_allocator()->proc_malloc(sizeof(size_t));
            *(size_t*)data = 42;

            temp_allocator()->proc_malloc(64);

            data = temp_allocator()->proc_realloc(data, 2*sizeof(size_t) );
            TEST_EXPECTS( *(size_t*)data == 42 );

            memory_manager_shutdown();           
        }
        TEST_END
        
        TEST_BEGIN(memory_leaks_detect_a_leak)
        {
            memory_manager_init();

            heap_allocator()->proc_malloc(64); // leak on purpose...

            Memory_Manager_Report report{};
            memory_manager_generate_report(&report);
            memory_manager_report_print(&report, false);

            memory_manager_shutdown();

            TEST_EXPECTS( report.has_leaked );
        }
        TEST_END
        
        TEST_BEGIN(memory_leaks_detect_no_leaks)
        {
            memory_manager_init();

            void* data = heap_allocator()->proc_malloc(64);
            heap_allocator()->proc_free(data);

            Memory_Manager_Report report{};
            memory_manager_generate_report(&report);
            memory_manager_report_print(&report, false);
            
            memory_manager_shutdown();

            TEST_EXPECTS( !report.has_leaked );
        }
        TEST_END

        TEST_BEGIN(memory_leaks_is_not_problem_with_temp_allocator)
        {
            memory_manager_init();

            temp_allocator()->proc_malloc(64); // we don't care this leaks

            Memory_Manager_Report report{};
            memory_manager_generate_report(&report);
            memory_manager_report_print(&report, false);

            memory_manager_shutdown();

            TEST_EXPECTS( !report.has_leaked );
        }
        TEST_END

        TEST_BEGIN(temp_allocator_buffer_overflow)
        {
            memory_manager_init();

            temp_allocator()->proc_malloc( temp_allocator_buffer().size / 3 * 2 );            // take 66%
            void* data1 = temp_allocator()->proc_malloc( temp_allocator_buffer().size / 2 );    // try to get 50%
            TEST_EXPECTS( data1 == nullptr );

            temp_allocator_buffer_reset();

            void* data2 = temp_allocator()->proc_malloc( temp_allocator_buffer().size + 1 ); // try to get more than possible
            TEST_EXPECTS( data2 == nullptr );

            Memory_Manager_Report report{};
            memory_manager_generate_report(&report);
            memory_manager_report_print(&report, false);

            memory_manager_shutdown();

        }
        TEST_END

        #endif //ifdef BDC_DEBUG_ALLOCATORS
    }
    TEST_SUITE_END

    TEST_SUITE_BEGIN(String)
    {
        memory_manager_init();

        TEST_BEGIN( String s )
        {
            String s;
            // undefined behavior!
        }
        TEST_END

        TEST_BEGIN( String{} )
        {
            String s{};
            TEST_EXPECTS(s.data == nullptr);
            TEST_EXPECTS(s.size == 0);
        }
        TEST_END

        TEST_BEGIN( "String{ char*, u32_t }" )
        {
            char buffer[24]{};
            String s{ buffer, sizeof(buffer) };

            TEST_EXPECTS(s.data == buffer);
            TEST_EXPECTS(s.size == 24);
        }
        TEST_END

        TEST_BEGIN( constexpr_constructor_from_cstr )
        {
            constexpr String str = "Hello, World";

            TEST_EXPECTS(strcmp(str.data, "Hello, World") == 0);
            TEST_EXPECTS(str.size == strlen("Hello, World"));
        }
        TEST_END

        TEST_BEGIN( constructor_from_cstr )
        {
            String str = "Hello, World";

            TEST_EXPECTS(strcmp(str.data, "Hello, World") == 0);
            TEST_EXPECTS(str.size == strlen("Hello, World"));
        }
        TEST_END

        TEST_BEGIN( String(const Array<char>&) )
        {
            Array<char> arr{ 'B', 'o', 'n', 'j', 'o', 'u', 'r'};
            
            String str = arr;

            TEST_EXPECTS( arr[0] == 'B' );
            str[0] = 'b';
            TEST_EXPECTS( arr[0] == 'b' );
        }
        TEST_END

        TEST_BEGIN( String::operator== )
        {
            TEST_EXPECTS(        String{""} == ""         );
            TEST_EXPECTS(String{"Bérenger"} == "Bérenger" );
        }
        TEST_END

        TEST_BEGIN( String::operator!= )
        {
            TEST_EXPECTS(      !(String{""} != "")        );
            TEST_EXPECTS(        String{""} != " "        );
            TEST_EXPECTS(String{"Bérenger"} != "Béranger" );
        }
        TEST_END

        TEST_BEGIN( string_printf )
        {
            TEST_EXPECTS( string_printf("%s", "Hello, World") == "Hello, World");
            TEST_EXPECTS( string_printf("%s%s%s", "Hello", ", ", "World") == "Hello, World");
        }
        TEST_END

        TEST_BEGIN( string_rfind )
        {
            TEST_EXPECTS( string_rfind( "filename.ext", '.') == 8);
            TEST_EXPECTS( string_rfind( "01234,678901234,6789", ',') == 15 );
            TEST_EXPECTS( string_rfind( "filename.ext", '/') == String::invalid_pos);
        }
        TEST_END

        TEST_BEGIN( string_stem )
        {
            String s = "file.jpg";

            TEST_EXPECTS( string_stem(s) == "file" );
        }
        TEST_END

        TEST_BEGIN( string_copy )
        {
            String a = "file a.jpg";
            String b = string_copy( a, temp_allocator());

            TEST_EXPECTS( a.data != b.data);
            TEST_EXPECTS( a.size == b.size);
            TEST_EXPECTS( strncmp(a.data, b.data, a.size) == 0);
        }
        TEST_END

        TEST_BEGIN( string_rfind )
        {
            TEST_EXPECTS( string_rfind("filename.ext", '.') == 8);
            TEST_EXPECTS( string_rfind("01234,678901234,6789", ',') == 15 );
            TEST_EXPECTS( string_rfind("filename.ext", '/') == String::invalid_pos);
        }
        TEST_END

        TEST_BEGIN( string_reset )
        {
            String s1 = "Bonjour";
            string_reset(s1);
            TEST_EXPECTS(s1.data == nullptr);
            TEST_EXPECTS(s1.size == 0);
        }
        TEST_END

        TEST_BEGIN( operator Array<char> )
        {
            String      str  = "Bonjour";            
            Array<char> arr1 = str;
            TEST_EXPECTS( strcmp(arr1.data, "Bonjour") == 0);
        }
        TEST_END

        TEST_BEGIN( string_concat )
        {
            Array<char> arr = string_concat(
                "Hello", 
                ", World", 
                temp_allocator()
            );            
            TEST_EXPECTS( arr.size == 5 + 7);
            TEST_EXPECTS( arr == "Hello, World") ;
        }
        TEST_END

        memory_manager_shutdown();
    }
    TEST_SUITE_END


    TEST_SUITE_BEGIN( Array<int> )
    {
        TEST_BEGIN( array_append(int) )
        {
            memory_manager_init();

            Resizable_Array<int> arr;
            array_init(arr, 0, temp_allocator());

            for(int i = 0; i < 10; ++i) array_append(arr, i+1);
            for(int i = 0; i < 10; ++i) TEST_EXPECTS( arr[i] == i+1);

            memory_manager_shutdown();
        }
        TEST_END

        TEST_BEGIN( array_find() )
        {
            memory_manager_init();

            Resizable_Array<int> arr;
            array_init(arr, 0, temp_allocator());

            array_append(arr, 10);
            array_append(arr, 42);
            array_append(arr, 1);

            auto result = array_find(arr, 42);
            TEST_EXPECTS( result.found );
            TEST_EXPECTS( result.at_pos == 1);

            memory_manager_shutdown();
        }
        TEST_END

        TEST_BEGIN( array_remove_ordered )
        {
            memory_manager_init();

            Resizable_Array<int> arr;
            array_init(arr, 0, temp_allocator());

            array_append(arr, 0);
            array_append(arr, 1);
            array_append(arr, 2);

            array_remove_ordered(arr, 1);

            TEST_EXPECTS( arr[1] == 2 );

            memory_manager_shutdown();
        }
        TEST_END

        TEST_BEGIN( array_resize(size_t) )
        {
            memory_manager_init();

            Resizable_Array<int> arr;
            array_init(arr, 0, temp_allocator());

            array_resize(arr, 16);
            for(u32_t i = 0; i < arr.size; ++i)
                arr[i] = i;
            TEST_EXPECTS( arr.size == 16);
            
            // resize must preserve data on the first 16 indices
            array_resize(arr, 32);
            for(u32_t i = 0; i < 16; ++i)
                TEST_EXPECTS( arr[i] == i);

            memory_manager_shutdown();
        }
        TEST_END

    }
    TEST_SUITE_END


    TEST_SUITE_BEGIN( String_Builder )
    {
        TEST_BEGIN( string_builder() )
        {
            memory_manager_init();

            String_Builder sb;
            string_builder_init(sb);
            TEST_EXPECTS(sb.allocator == temp_allocator()); // a String_Builder is designed too be used in short period of time, to build a string. By default it is allocated on temp memory.

            memory_manager_shutdown();
        }
        TEST_END

        TEST_BEGIN( string_builder append and build string )
        {
            memory_manager_init();

            String_Builder sb;
            string_builder_init(sb);
            string_builder_append(sb, "Bonjour");
            string_builder_append(sb,  "je m'appelle René.");
            String str = string_builder_build_string(sb, ", " ); // uses temp_allocator() by default.
            printf("\"%s\"\n", str.c_str());
            TEST_EXPECTS( str == "Bonjour, je m'appelle René." );

            Memory_Manager_Report* report = memory_manager_generate_report();
            TEST_EXPECTS(!report->has_leaked);

            memory_manager_shutdown();
        }
        TEST_END

        TEST_BEGIN( string_builder appendf and build string )
        {
            memory_manager_init();

            String_Builder sb;
            string_builder_init(sb);
            string_builder_appendf(sb, "%s", "Bonjour");
            string_builder_appendf(sb, "%s", "je m'appelle Josiane.");
            String str = string_builder_build_string(sb, ", " ); // uses temp_allocator() by default.
            printf("\"%s\"\n", str.c_str());
            TEST_EXPECTS( str == "Bonjour, je m'appelle Josiane." );

            Memory_Manager_Report* report = memory_manager_generate_report();
            TEST_EXPECTS(!report->has_leaked);

            memory_manager_shutdown();
        }
        TEST_END
    }
    TEST_SUITE_END

    TEST_SUITE_BEGIN( Hash_Map )
    {
        struct Data
        {
            String name;
            u16_t  birth_year;
        };

        memory_manager_init(1024*1024);

        TEST_BEGIN( "Hash_Map<String, ...> init/release" )
        {
            Hash_Map<String, Data> hashmap;
            hashmap_init(hashmap, temp_allocator() );

            TEST_EXPECTS(hashmap.capacity > 0);
            TEST_EXPECTS(hashmap.size == 0);

            hashmap_release(hashmap);

            TEST_EXPECTS(!hashmap.entries.size);
        }
        TEST_END

        TEST_BEGIN( "Hash_Map<String, ...> adding same key twice must add once" )
        {
            Hash_Map<String, String> hashmap;

            hashmap_init(hashmap, temp_allocator() );

            TEST_EXPECTS( hashmap_add(hashmap, "bzh", "Merlin") );
            hashmap_print(hashmap);
            TEST_EXPECTS( hashmap_find(hashmap, "bzh") );

            hashmap_print(hashmap);
            TEST_EXPECTS( !hashmap_add(hashmap, "bzh", "Merlin") );

            hashmap_release(hashmap);
            TEST_EXPECTS( !hashmap_find(hashmap, "bzh") );
        }
        TEST_END

        TEST_BEGIN( "Hash_Map<String, ...> hashmap_add" )
        {
            Hash_Map<String, String> hashmap;

            hashmap_init(hashmap, temp_allocator() );
            hashmap_add(hashmap, "bzh", "Merlin");

            TEST_EXPECTS( hashmap_find(hashmap, "bzh") );

            hashmap_release(hashmap);

            TEST_EXPECTS( !hashmap_find(hashmap, "bzh").ok );
        }
        TEST_END

        TEST_BEGIN( "Hash_Map<String, ...> hashmap_remove" )
        {
            Hash_Map<String, String> hashmap;

            hashmap_init(hashmap, temp_allocator() );
            hashmap_add(hashmap, "bzh", "Merlin");
            hashmap_print(hashmap);

            hashmap_remove(hashmap, "bzh");
            TEST_EXPECTS( !hashmap_find(hashmap, "bzh") );

            hashmap_print(hashmap);
            hashmap_release(hashmap);

            TEST_EXPECTS( !hashmap_find(hashmap, "bzh").ok );
        }
        TEST_END

        TEST_BEGIN( "Hash_Map<String, ...> with a custom _hash_proc" )
        {
            struct Stupid_Hash
            {
                // The purpose of this hash is to get maximum collisions!
                static u32_t hash(const String& str)
                {
                    return 42;
                };
            };

            Hash_Map<String, String> hashmap;
            hashmap_init(hashmap, temp_allocator(), Stupid_Hash::hash );

            hashmap_add(hashmap, "georges-brassens", "Georges Brassens");
            hashmap_add(hashmap, "jean-ferrat", "Jean Ferrat");

            TEST_EXPECTS( hashmap.size == 2);
            TEST_EXPECTS( hashmap_find(hashmap, "georges-brassens") );
            TEST_EXPECTS( hashmap_find(hashmap, "jean-ferrat") );

            hashmap_print(hashmap);
            hashmap_release(hashmap);

            TEST_EXPECTS(!hashmap.entries.size);
        }
        TEST_END

        TEST_BEGIN( "Hash_Map<String, ...> go above 75% load should increase capacity" )
        {
            Hash_Map<String, String> hashmap;
            hashmap_init(hashmap, temp_allocator() );

            const u32_t initial_capacity = hashmap.capacity;
            for(int i = 0; i < initial_capacity; ++i)
            {
                hashmap_add(hashmap, string_printf(temp_allocator(), "elem-%i", i) , string_printf(temp_allocator(), "Valeur de l'élément %i", i) );
            }

            hashmap_print(hashmap);
            hashmap_release(hashmap);

            TEST_EXPECTS(!hashmap.entries.size);
        }
        TEST_END

        memory_manager_shutdown();
    }
    TEST_SUITE_END

}