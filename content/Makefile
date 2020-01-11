all: build_lib build_samples
clean: clean_lib clean_samples

build_lib: XML_Manager.o Belief_propagation.o Graphical_model.o Node.o Potential.o Subgraph.o Trainer.o Training_set.o
	mkdir -p bin \
	ar rvs ./bin/EFG.a \
	./bin/XML_Manager.o \
	./bin/Belief_propagation.o \
	./bin/Graphical_model.o \
	./bin/Node.o \
	./bin/Potential.o \
	./bin/Subgraph.o \
	./bin/Trainer.o \
	./bin/Training_set.o
	rm ./bin/*.o
	
XML_Manager.o:
	g++ -c ./EFG/XML_Manager/XML_Manager.cpp -O3
	mv XML_Manager.o ./bin/XML_Manager.o
Belief_propagation.o:
	g++ -c ./EFG/Source/Belief_propagation.cpp -O3
	mv Belief_propagation.o ./bin/Belief_propagation.o
Graphical_model.o:
	g++ -c ./EFG/Source/Graphical_model.cpp -O3
	mv Graphical_model.o ./bin/Graphical_model.o
Node.o:
	g++ -c ./EFG/Source/Node.cpp -O3
	mv Node.o ./bin/Node.o
Potential.o:
	g++ -c ./EFG/Source/Potential.cpp -O3
	mv Potential.o ./bin/Potential.o
Subgraph.o:
	g++ -c ./EFG/Source/Subgraph.cpp -O3
	mv Subgraph.o ./bin/Subgraph.o
Trainer.o:
	g++ -c ./EFG/Source/Trainer.cpp -O3
	mv Trainer.o ./bin/Trainer.o
Training_set.o:
	g++ -c ./EFG/Source/Training_set.cpp -O3
	mv Training_set.o ./bin/Training_set.o
	
build_samples: build_lib
	g++ -o ./bin/Sample_01 ./EFG_Samples/Sample_01_Potential_handling/Main_01.cpp ./bin/EFG.a -O3
	g++ -o ./bin/Sample_02 ./EFG_Samples/Sample_02_Belief_propagation_A/Main_02.cpp ./bin/EFG.a -O3
	g++ -o ./bin/Sample_03 ./EFG_Samples/Sample_03_Belief_propagation_B/Main_03.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_03_graphs -rf
	cp ./EFG_Samples/Sample_03_Belief_propagation_B/Sample_03_graphs ./bin/Sample_03_graphs -r
	g++ -o ./bin/Sample_04 ./EFG_Samples/Sample_04_HMM_chain/Main_04.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_04_graphs -rf
	cp ./EFG_Samples/Sample_04_HMM_chain/Sample_04_graphs ./bin/Sample_04_graphs -r
	g++ -o ./bin/Sample_05 ./EFG_Samples/Sample_05_Graph_Matrix/Main_05.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_05_graphs -rf
	cp ./EFG_Samples/Sample_05_Graph_Matrix/Sample_05_graphs ./bin/Sample_05_graphs -r
	g++ -o ./bin/Sample_06 ./EFG_Samples/Sample_06_Learning_A/Main_06.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_06_graphs -rf
	cp ./EFG_Samples/Sample_06_Learning_A/Sample_06_graphs ./bin/Sample_06_graphs -r
	g++ -o ./bin/Sample_07 ./EFG_Samples/Sample_07_Learning_B/Main_07.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_07_graphs -rf
	cp ./EFG_Samples/Sample_07_Learning_B/Sample_07_graphs ./bin/Sample_07_graphs -r
	g++ -o ./bin/Sample_08 ./EFG_Samples/Sample_08_subgraph_operation/Main_08.cpp ./bin/EFG.a -O3
	rm ./bin/Sample_08_graphs -rf
	cp ./EFG_Samples/Sample_08_subgraph_operation/Sample_08_graphs ./bin/Sample_08_graphs -r
	
clean_lib:
	rm ./bin/EFG.a -rf

clean_samples:
	rm ./bin/Sample_01 -rf
	rm ./bin/Sample_02 -rf
	rm ./bin/Sample_03 -rf
	rm ./bin/Sample_04 -rf
	rm ./bin/Sample_05 -rf
	rm ./bin/Sample_06 -rf
	rm ./bin/Sample_07 -rf
	rm ./bin/Sample_08 -rf