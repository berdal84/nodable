![Nodable Logo](https://github.com/berdal84/Nodable/blob/develop/data/icon.png)

Nodable is a simple node based program !
========================================
[![Build Status](https://travis-ci.org/berdal84/Nodable.svg?branch=master)](https://travis-ci.org/berdal84/Nodable)

This software is a command line prompt able to evaluate numerical expressions by constructing an execution tree in realtime.
When user type an expression, the program split it into tokens, build the execution tree and evaluates it. Each nodes and its links are displayed with Dear ImGui (O. Cornut).

Examples :
==========

![Example1](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_05_12_GUI_Simple.png)

![Example2](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_05_12_GUI_Advanced.png)

![Example3](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_05_12_GUI_Complex.png)


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

