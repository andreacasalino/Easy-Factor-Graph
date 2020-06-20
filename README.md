<h1> Easy factor graph: EFG </h1>

General purpose c++ library for handling undirected graphical models.

<h2> CONTENTS <h2>

<li>  ./EFG: contains the sources of the library </li>
<li> ./EFG_Samples: contains 8 classes of examples, showing how to use EFG  </li>
<li> ./EFG_GUI: contains an interactive application for managing medium-size factor graphs through a responsive interface (for the moment only for Windows) </li>
<li> ./EFG.pdf: an extensive guide that contains details about the theoretical concepts EFG is based on as well the structure of the classes constituting EFG. It explains also how to use the EFG_GUI. </li>

<h2> HOW TO COMPILE <h2>

<li> Windows: Use the Visual Studio solution ./EFG.sln to compile EFG as well as all the examples. A folder, at .\x64\Debug or .\x64\Release will be created with all the binaries. Alternatively you can build the library and all the examples using g++, by launching gpp_make.bat </li>
<li> UNIX: Use ./Makefile to compile EFG as well as all the examples.   A folder, at .\bin or .\bin will be created with all the binaries. </li>

<h2> RUN THE EXAMPLES <h2>
After compiling, a folder called bin is created storing a static library and all the examples, together with the materials required to run them. Launch the example you are interested in from that folder.