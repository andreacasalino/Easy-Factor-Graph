RD ".\bin" /S /Q
MD ".\bin"

echo --------> build the library <----------
g++ -c ./EFG/XML_Manager/XML_Manager.cpp -O3
g++ -c ./EFG/Source/Potential.cpp -O3
g++ -c ./EFG/Source/Node.cpp -O3
g++ -c ./EFG/Source/Belief_propagation.cpp -O3
g++ -c ./EFG/Source/Graphical_model.cpp -O3
g++ -c ./EFG/Source/Learning_handler.cpp -O3
g++ -c ./EFG/Source/Trainer.cpp -O3
g++ -c ./EFG/Source/Training_set.cpp -O3
g++ -c ./EFG/Source/Subgraph.cpp -O3
ar rvs ./bin/EFG.lib ^
	XML_Manager.o ^
	Belief_propagation.o ^
	Graphical_model.o ^
	Learning_handler.o ^
	Node.o ^
	Potential.o ^
	Trainer.o ^
	Training_set.o ^
	Subgraph.o
	del *.o

pause

echo --------> build samples <----------
	g++ -o ./bin/Sample_01 ./EFG_Samples/Sample_01_Potential_handling/Main_01.cpp ./bin/EFG.lib -O3
	
	g++ -o ./bin/Sample_02 ./EFG_Samples/Sample_02_Belief_propagation_A/Main_02.cpp ./bin/EFG.lib -O3
	
	g++ -o ./bin/Sample_03 ./EFG_Samples/Sample_03_Belief_propagation_B/Main_03.cpp ./bin/EFG.lib -O3
	RD "./bin/Sample_03_graphs" /S /Q
	MD "./bin/Sample_03_graphs"
	XCOPY "./EFG_Samples/Sample_03_Belief_propagation_B/Sample_03_graphs" "./bin/Sample_03_graphs" /S

	g++ -o ./bin/Sample_04 ./EFG_Samples/Sample_04_HMM_chain/Main_04.cpp ./bin/EFG.lib -O3
	RD "./bin/Sample_04_graphs" /S /Q
	MD "./bin/Sample_04_graphs"
	XCOPY "./EFG_Samples/Sample_04_HMM_chain/Sample_04_graphs" "./bin/Sample_04_graphs" /S

	g++ -o ./bin/Sample_05 ./EFG_Samples/Sample_05_Graph_Matrix/Main_05.cpp ./bin/EFG.lib -O3
	RD "./bin/Sample_05_graphs" /S /Q
	MD "./bin/Sample_05_graphs"
	XCOPY "./EFG_Samples/Sample_05_Graph_Matrix/Sample_05_graphs" "./bin/Sample_05_graphs" /S

	g++ -o ./bin/Sample_06 ./EFG_Samples/Sample_06_Learning_A/Main_06.cpp ./bin/EFG.lib -O3
	RD "./bin/Sample_06_graphs" /S /Q
	MD "./bin/Sample_06_graphs"
	XCOPY "./EFG_Samples/Sample_06_Learning_A/Sample_06_graphs" "./bin/Sample_06_graphs" /S

	g++ -o ./bin/Sample_07 ./EFG_Samples/Sample_07_Learning_B/Main_07.cpp ./bin/EFG.lib -O3
	RD "./bin/Sample_07_graphs" /S /Q
	MD "./bin/Sample_07_graphs"
	XCOPY "./EFG_Samples/Sample_07_Learning_B/Sample_07_graphs" "./bin/Sample_07_graphs" /S

	g++ -o ./bin/Sample_08 ./EFG_Samples/Sample_08_subgraph_operation/Main_08.cpp ./bin/EFG.lib -O3
	RD "./bin/Sample_08_graphs" /S /Q
	MD "./bin/Sample_08_graphs"
	XCOPY "./EFG_Samples/Sample_08_subgraph_operation/Sample_08_graphs" "./bin/Sample_08_graphs" /S
	