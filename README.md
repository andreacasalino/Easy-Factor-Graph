![binaries_compilation](https://github.com/andreacasalino/Easy-Factor-Graph/actions/workflows/installArtifacts.yml/badge.svg)
![binaries_compilation](https://github.com/andreacasalino/Easy-Factor-Graph/actions/workflows/runTests.yml/badge.svg)

**Easy Factor Graph**, aka **EFG**, is a general purpose c++ library for handling **undirected graphical models**.
**undirected graphical models** are probabilistic models similar to **bayesian networks**, but offerring some nicer 
properties. Not familiar with this kind of concepts? Don't worry, have a look at the [documentation](https://github.com/andreacasalino/Easy-Factor-Graph/blob/master/doc/EFG.pdf) in the **doc** folder before diving into the code ;).
**Random Fields** as well as **Conditional Random Fields** are particular classes of **undirected graphical models** and can be easily created and **trained** using this library.
**undirected graphical models** can be saved as **xml** or **json** files and later re-imported.

Training can be done using the gradient-base approaches implemented of [this](https://github.com/andreacasalino/TrainingTools) external library.

With respect to similar libraries, **EFG** is able to:
 * TODO
 * TODO
 * TODO

## CONTENTS

This project is structured as follows:
 * the documentation in ./doc explains both how to use **EFG** as well give some theoretical background about **undirected graphical models**
 * the sources of the **EFG** library are contained in ./src
 * ./samples contains 8 classes of examples, extensively showing how to use **EFG**

## USAGE

Haven't yet left a **star**? Do it now! :).

### FACTORS CONSTRUCTION
### MODELS CONSTRUCTION
### QUERY THE MODEL
### TUNE THE MODEL
### GIBBS SAMPLING

## SAMPLES

Haven't yet left a **star**? Do it now! :).

The samples contained in ./samples extensively shows how to use **EFG**.
All samples consume a library of utilities called **Samples-Helpers**, which contain common functionalities like printing utilities and something else.

## CMAKE SUPPORT

Haven't yet left a **star**? Do it now! :).

To consume this library you can rely on [CMake](https://cmake.org).
More precisely, You can fetch this package and link to the **EFG** library:
```cmake
include(FetchContent)
FetchContent_Declare(
efg
GIT_REPOSITORY https://github.com/andreacasalino/Easy-Factor-Graph
GIT_TAG        master
)
FetchContent_MakeAvailable(efg)
```

and then link to the **EFG** library:
```cmake
target_link_libraries(${TARGET_NAME}
   EFG
)
```
### TRAINING CAPABILITIES

The possibility to train a model is by deafult enabled. However, such functionalities rely on [this](https://github.com/andreacasalino/TrainingTools) heavy external library. This consumes a significant amount of time when configuring the project.
Therefore, if you don't need such functionalities you can set the CMake option **BUILD_EFG_TRAINER_TOOLS** equal to **OFF**.
However, after disabling that option you will still able to get the tunable weights of a model, as well as their gradient, allowing you to use or implement another gradient based trainer. 

### XML SUPPORT

By default, the features to export and import models from **XML** files are enabled. If you don't need them, put the CMake option **BUILD_EFG_XML_CONVERTER** to **OFF**.

### JSON SUPPORT

By default, the features to export and import models from **JSON** files are enabled. They rely on the famous [nlohmann](https://github.com/nlohmann/json) library, which is internally fetched and linked.
If you don't need such functionalities, put the CMake option **BUILD_EFG_JSON_CONVERTER** to **OFF**.

### VISUAL STUDIO COMPATIBILITY

This library exploits virtual inheritance to define some objects hierarchies. This might trigger [this](https://stackoverflow.com/questions/6864550/c-inheritance-via-dominance-warning) weird warning when compiling in Windows with Visual Studio. You can simply ignore it or tell Visual Studio to ignore warning code 4250, which is something that can be done as explained [here](https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4996?view=msvc-170).
    
## EFG GUI

If you have found this library useful, please find the time to leave a star :). Just before you go, be aware that [Easy-Factor-Graph-GUI](https://github.com/andreacasalino/Easy-Factor-Graph-GUI) wraps this library as C++ backend to a nice graphical user interactive application:

![What you should see when running the application](https://github.com/andreacasalino/Easy-Factor-Graph-GUI/blob/master/Example.png)
