![Nodable Logo](https://github.com/berdal84/Nodable/blob/develop/data/icon.png)

[![Build Status](https://travis-ci.org/berdal84/Nodable.svg?branch=master)](https://travis-ci.org/berdal84/Nodable)

Nodable is node-able !
======================

This software is a command line prompt able to evaluate literal expressions by constructing an execution tree in realtime.
When user type an expression the program split it into tokens and build the execution tree. Each frame each node is updated only if needed. The GUI uses the famous library *Dear ImGui* developed by OmarCornut (@ocornut).

Examples :
==========

![Example1](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_05_13_GUI_Value_Editable.png)


There is 3 display modes: simple, advanced and complex.

![Example2](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_05_12_GUI_Simple.png)

![Example3](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_05_12_GUI_Advanced.png)

![Example4](https://github.com/berdal84/Nodable/blob/master/screenshots/2018_05_12_GUI_Complex.png)


The language :
==============

Expressions:
------------

Supports binary operations only.

Expression -> (Operand, Operator, Expression) | Operand

Operands :
----------

An operand could be :

- a Number (ex: 1, 0.5, 100.456, etc.)
- a String (ex: "Hello", "World", etc.)
- a Variable (ex: a, b, myVar, etc.)

Operand -> Number | String | Variable

Operators:
----------

Supports operator precedence with the following operators :

- Addition (+)
- Substraction (-)
- Multiplication (*)
- Division (/)
- Assignment (=)

Operator -> +|-|*|/|=


Platform compatibility :
========================
Should work on all platforms but only tested under GNU/Linux Ubuntu 17.x (64bits)

How to compile ? :
==================
Install dependencies (optionnal):

```
sudo add-apt-repository -y ppa:team-xbmc/ppa
sudo add-apt-repository -y ppa:pyglfw/pyglfw
sudo apt-get update -qq
sudo apt-get install -y --no-install-recommends libsdl2-dev gcc-4.8 g++-4.8 libusb-1.0-0-dev
sudo apt-get install -y --no-install-recommends libglfw3-dev libxrandr-dev libxi-dev libxxf86vm-dev
```
Clone the project with submodules:
```
git clone --recurse-submodules https://github.com/berdal84/Nodable
```

Compile:
```
cd ./Nodable
make
```

Run:
```
cd ./bin/linux64
.nodable
```

How to run the software ? :
===========================
- move to bin/linux64/ and run `./nodable`


Licence:
=========
**Nodable** is licensed under the GPL License, see LICENSE for more information.

submodules: **Dear ImGui** is licensed under the MIT License.
