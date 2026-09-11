
--- BDC* standard library ---

The goal of this library is to provide a basic standard library that
does not rely on std (std::string, std::vector, etc.) because I want a
POD (plain old data) way of thinking.

This is highly inspired by Jai's standard library and also C libraries.
No constructors/destructors (only the default ones), initialisation is decorrelated from ressource acquisition.
I do not put too much stuff in constructors and rather rely on utility functions like xxx_init(XXX*) and xxx_deinit(XXX*) where
allocation may take place.

Zero is default value for initialization everywhere, so you can memset any struct to zeros.

Allocators:     memory management (temporary and heap allocations, new/delete replacements)
Array:          view and resizable arrays, Array<T> and Resizable_Array<T> respectively.
                TODO:
                    - Resizable_Array<T>:
                        - they do not grow exponentially, this is good when used wrapped into something, but not great when you use it as a regular arrray,
                        perhaps we should be able to configure how it grows.
                        - when an element is removed, user might need to release some memory,
                        I should perhaps return the element: Elem_Type array_remove(u32_t pos).
Hash_Map:        a simple hash map
                 TODO:
                    - optimize the way a slot is selected when the actual ideal slot is occupied.
                    - remove the State (OCCUPIED, FREE) by using a zero-initialized value for the keys or for the hash.
                    - do we allow multiple values for a given key? (like std::unordered_multimap).
                    - when an element is removed, user might need to release some memory,
                       I should perhaps return the element: Elem_Type array_remove(u32_t pos).
String:         is like an Array<char>, not resizable by nature. It's a view.
                TODO:
                    - in some cases, when we call c_str() on a String, we get 1 byte allocated (for the null-terminator), however we should instead use a lptr in String.data instead.
String_Builder: tool to concatenate N strings while allocating once.
String_Hash:    a djb2 hash implementation.
Types:          some typedefs for integers.
                                          

*BDC: stands for Bérenger Dalle-Cort ^^

--- Bé, Aug 28 2026 ---