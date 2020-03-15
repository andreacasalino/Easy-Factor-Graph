# Easy factor graph (EFG)
General purpose c++ library for handling undirected graphical models.

CONTENTS:

./content/EFG: contains the sources of the library
./content/EFG_Samples: contains 8 classes of examples, showing how to use EFG
./content/EFG_GUI: contains an interactive application for managing medium-size factor graphs through a responsive interface
./content/EFG.pdf: an extensive guide that contains details about the theoretical concepts EFG is based on as well the structure of the classes constituting EFG.
					It explains also how to use the EFG_GUI.

INSTALLATION:

->Windows: Use the Visual Studio solution ./content/EFG.sln to compile EFG as well as all the examples. A folder, at .\x64\Debug or .\x64\Release will be created with all the binaries.
		   Alternatively you can build the library and all the examples using g++, by launching gpp_make.bat

->UNIX: Use ./content/Makefile to compile EFG as well as all the examples.   A folder, at .\bin or .\bin will be created with all the binaries.
