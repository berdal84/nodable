# xxHash - a super-fast hash algorithm in a single C++ header

This is an implementation of Yann Collet's xxHash32 and xxHash64 algorithms (https://github.com/Cyan4973/xxHash).
Just include the short [xxhash32.h](xxhash32.h) or [xxhash64.h](xxhash64.h) header - that's it, no external dependencies !

Performance of my library is usually on par with his original code.
His code can be hard to understand due to the massive use of #ifdef, therefore I posted a detailled explanation of the algorithm on my website https://create.stephan-brumme.com/xxhash/
(which is also the main repository; GitHub serves as a mirror only).
