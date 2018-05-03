![Nodable Logo](https://github.com/berdal84/Nodable/blob/develop/data/icon.png)

Nodable is a simple node based program !
========================================
[![Build Status](https://travis-ci.org/berdal84/Nodable.svg?branch=master)](https://travis-ci.org/berdal84/Nodable)

This program provides a command line prompt able to evaluate numerical expressions.

Examples :
==========

example 1 :
-----------
```
>>> 10 + 4 / 2

Execution step by step :
4.000000 / 2.000000 = 2.000000
10.000000 + result = 12.000000

Tree view :
[Result : 12.000000]
   [Add]
   [10.000000]
   [result : 2.000000]
      [Divide]
      [4.000000]
      [2.000000]

Result: 12.000000
```
example 2 :
-----------
```
>>> 3 + 10 / 10 + 5

Execution step by step :
10.000000 / 10.000000 = 1.000000
result + 5.000000 = 6.000000
3.000000 + result = 9.000000

Tree view :
[Result : 9.000000]
   [Add]
   [3.000000]
   [result : 6.000000]
      [Add]
      [result : 1.000000]
         [Divide]
         [10.000000]
         [10.000000]
      [5.000000]

Result: 9.000000
```

example 3 :
-----------
```
>>> 10 * 0.5

Execution step by step :
10.000000 * 0.500000 = 5.000000

Tree view :
[Result : 5.000000]
   [Multiply]
   [10.000000]
   [0.500000]

Result: 5.000000
```

example 4 :
-----------
```
>>> a = 10 + 50

Execution step by step :
10.000000 + 50.000000 = 60.000000
a = result (result 60.000000)

Tree view :
[Result : 60.000000]
   [Assign]
   [a : 60.000000]
   [result : 60.000000]
      [Add]
      [10.000000]
      [50.000000]

Result: 60.000000
>>> b = a - 10

Execution step by step :
a - 10.000000 = 50.000000
b = result (result 50.000000)

Tree view :
[Result : 50.000000]
   [Assign]
   [b : 50.000000]
   [result : 50.000000]
      [Substract]
      [a : 60.000000]
      [10.000000]

Result: 50.000000
```

Expressions:
============

Supports binary operations only.

Expression -> (Operand, Operator, Expression) | Operand

Operands :
==========

An operand could be :

- a Number (ex: 1, 0.5, 100.456, etc.)
- a Variable (ex: a, b, myVar, etc.)

Operand -> Number | Variable

Operators:
==========

Supports operator precedence with the following operators :

- Addition (+)
- Substraction (-)
- Multiplication (*)
- Division (/)
- Assignment (=)

Operator -> +|-|*|/|=

Platform compatibility :
------------------------
Should work on all platforms but only tested under GNU/Linux Ubuntu 17.x (64bits)

How to install ? :
------------------------
- clone the project `git clone https://github.com/berdal84/Nodable`
- compile `make`
- install `make install`
- run `nodable`

