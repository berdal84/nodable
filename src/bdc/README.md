
# BDC* standard library

## Introduction

The goal of this library is to provide a basic standard library that
does not rely on std (std::string, std::vector, etc.) because I want a
POD (plain old data) way of thinking.

This is highly inspired by Jai's standard library and also C libraries.
No constructors/destructors (only the default ones), initialisation is decorrelated from ressource acquisition.
I do not put too much stuff in constructors and rather rely on utility functions like xxx_init(XXX*) and xxx_deinit(XXX*) where
allocation may take place.

Zero is default value for initialization everywhere, so you can memset any struct to zeros.

*BDC: stands for Bérenger Dalle-Cort ^^

## Overview

Allocators:     memory management (temporary and heap allocations, new/delete replacements)
Array:          view and resizable arrays, Array<T> and Resizable_Array<T> respectively.
Hash_Map:       a simple hash map
Hash:           a djb2 hash implementation.
MACROS:         a set of useful macros
Maths:          some maths functions...
String_Builder: a tool to concatenate N strings while allocating once.
String_Hash:    string hash with data to access its original String.
String:         is like an Array<char>, not resizable by nature. It's a view.
Testing:        a simple testing toolkit
Type_Traits:    some meta programming tools                                          
Types:          some typedefs.

## CHANGELOG:
- 2026/10/03: Improve Hash_Map's hash to index, Improve Resizable_Array's realloc method (2^n).
- 2026/08/28: First version
