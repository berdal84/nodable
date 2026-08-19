
Single-Header for BDC* library.

The goal of this library is to provide a basic standard library that
does not rely on std (std::string, std::vector, etc.) because I want a
POD (plain old data) way of thinking.
No constructors/destructors, memory is managed manually using xxx_init(XXX*), xxx_deinit(XXX*)
and if needed some bdc::Allocators.
Zero is default value for initialization everywhere, so you can memset any struct to zeros.

Content:
 - Allocator:    some allocators (dynamic on heap, fixed-size on stack, or temporary)
 - Array:        dynamic arrays
 - Hash_Map:     TODO...
 - String:       similar to a dynamic array for characters, no disctiction between String and String_View for now.
                 Perhaps String should be a view, and String_View should be a pointer to data and a size.

 Bé, Aug 18 2026.

 *BDC: stands for Bérenger Dalle-Cort ^^
