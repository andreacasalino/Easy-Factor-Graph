**Easy Factor Graph**, aka **EFG**, is a general purpose c++ library for handling **undirected graphical models**.
**undirected graphical models** are probabilistic models similar to **bayesian networks**, but offerring much more nice 
properties. Not familiar with this kind of things? Don't worry, have a look at the [documentation](https://github.com/andreacasalino/Easy-Factor-Graph/blob/master/doc/EFG.pdf) in the **doc** folder
before diving into the code ;).
**undirected graphical models** can be saved as **xml** files and later re-imported.
The library contains some minimal functionalities to also perform training.

**EFG** is completely cross platform: use [CMake](https://cmake.org) to configure the entire project.

**Contents**

 * the documentation in ./doc explains both how to use **EFG** as well give some theoretical background 
 * the sources of the library are contained in ./Lib
 * ./EFG_Samples contains 8 classes of examples, extensively showing how to use **EFG**

[XML-GUI](https://github.com/andreacasalino/XML-GUI) wraps this library as C++ backend to a nice graphical user interactive application:

![the kind of model handled by this library](https://github.com/andreacasalino/Easy-Factor-Graph/tree/master/img/img1.png)

