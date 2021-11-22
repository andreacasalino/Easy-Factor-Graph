![binaries_compilation](https://github.com/andreacasalino/Easy-Factor-Graph/actions/workflows/installArtifacts.yml/badge.svg)
![binaries_compilation](https://github.com/andreacasalino/Easy-Factor-Graph/actions/workflows/runTests.yml/badge.svg)

**Easy Factor Graph**, aka **EFG**, is a general purpose c++ library for handling **undirected graphical models**.
**undirected graphical models** are probabilistic models similar to **bayesian networks**, but offerring some more nice 
properties. Not familiar with this kind of concepts? Don't worry, have a look at the [documentation](https://github.com/andreacasalino/Easy-Factor-Graph/blob/master/doc/EFG.pdf) in the **doc** folder
before diving into the code ;).
**Random Fields** as well as **Conditional Random Fields** are particular classes of **undirected graphical models** and can be easily created and **trained** using this library.
**undirected graphical models** can be saved as **xml** or **json** files and later re-imported.
The library contains some minimal functionalities to also perform training using the gradient descend approach.

**EFG** is completely **cross platform**: let [CMake](https://cmake.org) do all the work for you.

**Contents**

 * the documentation in ./doc explains both how to use **EFG** as well give some theoretical background 
 * the sources of the library are contained in ./Lib
 * ./Samples contains 8 classes of examples, extensively showing how to use **EFG**

**Compile**
   
 * Configure and generate the project using [CMake](https://cmake.org)

   * check this [tutorial](https://www.youtube.com/watch?v=LxHV-KNEG3k) (it is valid for any kind of IDE and not only VS) if you are not familiar
   * When setting the CMake LIB_OPT flag to ON, the libraries that are part of this project will be compiled as shared, otherwise as static. By default the static behaviour is set  
   * some computations are speed up using a thread pool implemented in an external library. However,
the compilation of such library might cause problems in some systems. Therefore, you can also decide to not use the thread pool (and not compile it at all) by selecting **OFF** for the [CMake](https://cmake.org) option **THREAD_POOL_OPT**
   * This library contains many different iterative tuners, that can be used to train any kind of factor graph. However, if you don't need to use it or you simply need to use a basic fixed step size gradient descend, you can disable the compilation 
    of such advanced trainers by setting the [CMake](https://cmake.org) option **ADVANCED_TRAINERS_OPT** to **OFF**, which is set to **ON** by default.
	  
 * Compile the library and the samples

 * You can also just download the compiled binaries (compiled in Release) of the library of the latest master commit [here](https://github.com/andreacasalino/Easy-Factor-Graph/actions) 
 
**What else?**

If you have found this library useful, please find the time to leave a star :). Just before you go, be aware that [Easy-Factor-Graph-GUI](https://github.com/andreacasalino/Easy-Factor-Graph-GUI) wraps this library as C++ backend to a nice graphical user interactive application:

![What you should see when running the application](https://github.com/andreacasalino/Easy-Factor-Graph-GUI/blob/master/Example.png)

