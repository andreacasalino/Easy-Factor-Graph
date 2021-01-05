**Easy Factor Graph**, aka **EFG**, is a general purpose c++ library for handling **undirected graphical models**.
**undirected graphical models** are probabilistic models similar to **bayesian networks**, but offerring much more nice 
properties. Not familiar with this kind of things? Don't worry, have a look at the [documentation](https://github.com/andreacasalino/Easy-Factor-Graph/blob/master/doc/EFG.pdf) in the **doc** folder
before diving into the code ;).
**undirected graphical models** can be saved as **xml** files and later re-imported.
The library contains some minimal functionalities to also perform training.

**EFG** is completely **cross platform**: let [CMake](https://cmake.org) do all the work for you.

**Contents**

 * the documentation in ./doc explains both how to use **EFG** as well give some theoretical background 
 * the sources of the library are contained in ./Lib
 * ./EFG_Samples contains 8 classes of examples, extensively showing how to use **EFG**

**Compile**

 * After cloning the library you need to initialize the git submodules by running the following commands from the root folder:
   
   * `git submodule init`
   * `git submodule update`
   
 * Configure and generate the project using [CMake](https://cmake.org)

   * check this [tutorial](https://www.youtube.com/watch?v=LxHV-KNEG3k) (it is valid for any kind of IDE and not only VS) if you are not familiar
   * some computations are speed up using a thread pool implemented in an external library. However,
the compilation of such library might cause problems in some systems. Therefore, you can also decide to not use the thread pool (and not compile it at all) by selecting **OFF** for the [CMake](https://cmake.org) option **THREAD_POOL_OPT**
      
 * Compile the library and the samples
   
[Easy-Factor-Graph-GUI](https://github.com/andreacasalino/Easy-Factor-Graph-GUI) wraps this library as C++ backend to a nice graphical user interactive application:

![Example of model handled by this library](https://github.com/andreacasalino/Easy-Factor-Graph/blob/master/img/img1.png)

