all: build_lib Sample_01 Sample_02 Sample_03 Sample_04 Sample_05 Sample_06 Sample_07 Sample_08
clean: rm -f -r bin

build_lib: 
	rm -f -r bin
	mkdir bin
	g++ -o ./bin/XML_Manager.o -c ./EFG/XML_Manager/XML_Manager.cpp -O3
	g++ -o ./bin/Belief_propagation.o -c ./EFG/Source/Belief_propagation.cpp -O3
	g++ -o ./bin/Graphical_model.o -c ./EFG/Source/Graphical_model.cpp -O3
	g++ -o ./bin/Node.o -c ./EFG/Source/Node.cpp -O3
	g++ -o ./bin/Potential.o -c ./EFG/Source/Potential.cpp -O3
	g++ -o ./bin/Subgraph.o -c ./EFG/Source/Subgraph.cpp -O3
	g++ -o ./bin/Trainer.o -c ./EFG/Source/Trainer.cpp -O3
	g++ -o ./bin/Training_set.o -c ./EFG/Source/Training_set.cpp -O3
	g++ -o ./bin/Learning_handler.o -c ./EFG/Source/Learning_handler.cpp -O3
	ar rvs ./bin/EFG.a \
	./bin/XML_Manager.o \
	./bin/Belief_propagation.o \
	./bin/Graphical_model.o \
	./bin/Node.o \
	./bin/Potential.o \
	./bin/Subgraph.o \
	./bin/Trainer.o \
	./bin/Training_set.o \
	./bin/Learning_handler.o
	rm ./bin/*.o

Sample_01:
	g++ -o ./bin/Sample_01 ./EFG_Samples/Sample_01_Potential_handling/Main_01.cpp ./bin/EFG.a -O3
Sample_02:
	g++ -o ./bin/Sample_02 ./EFG_Samples/Sample_02_Belief_propagation_A/Main_02.cpp ./bin/EFG.a -O3
Sample_03:
	g++ -o ./bin/Sample_03 ./EFG_Samples/Sample_03_Belief_propagation_B/Main_03.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_03_graphs -rf
	cp ./EFG_Samples/Sample_03_Belief_propagation_B/Sample_03_graphs ./bin/Sample_03_graphs -r
Sample_04:
	g++ -o ./bin/Sample_04 ./EFG_Samples/Sample_04_HMM_chain/Main_04.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_04_graphs -rf
	cp ./EFG_Samples/Sample_04_HMM_chain/Sample_04_graphs ./bin/Sample_04_graphs -r
Sample_05:
	g++ -o ./bin/Sample_05 ./EFG_Samples/Sample_05_Graph_Matrix/Main_05.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_05_graphs -rf
	cp ./EFG_Samples/Sample_05_Graph_Matrix/Sample_05_graphs ./bin/Sample_05_graphs -r
Sample_06:
	g++ -o ./bin/Sample_06 ./EFG_Samples/Sample_06_Learning_A/Main_06.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_06_graphs -rf
	cp ./EFG_Samples/Sample_06_Learning_A/Sample_06_graphs ./bin/Sample_06_graphs -r
Sample_07:
	g++ -o ./bin/Sample_07 ./EFG_Samples/Sample_07_Learning_B/Main_07.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_07_graphs -rf
	cp ./EFG_Samples/Sample_07_Learning_B/Sample_07_graphs ./bin/Sample_07_graphs -r
Sample_08:
	g++ -o ./bin/Sample_08 ./EFG_Samples/Sample_08_subgraph_operation/Main_08.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_08_graphs -rf
	cp ./EFG_Samples/Sample_08_subgraph_operation/Sample_08_graphs ./bin/Sample_08_graphs -r