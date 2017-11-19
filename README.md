![Nodable Logo](https://github.com/berdal84/Nodable/blob/develop/data/icon.png)

Nodable is a simple node based program !
========================================
[![Build Status](https://travis-ci.org/berdal84/Nodable.svg?branch=master)](https://travis-ci.org/berdal84/Nodable)

This program provides a command line prompt able to evaluate numerical expressions.

Examples :
==========

example 1 :
-----------

`10 + 4 / 2`   
`Result: 12`  

example 2 :
-----------

̀`3 + 10/10 + 5`   
`Result: 9̀`  

example 3 :
-----------

`10 * 0.5`   
`Result: 5`

example 4 :
-----------

`a=10+50`   
`Result: 60`   
`b=a-10`  
`Result: 50`

Expressions:
============

Supports binary operations only.

Expression -> (Operand, Operator, Expression) | Operand

Operands :
==========

An operand could be :

- a number (ex: 1, 0.5, 100.456, etc.)
- a symbol (ex: a, b, myVar, etc.)

Operand -> Number | Symbol

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
