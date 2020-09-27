/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 02: Belief propagation, part B' of the documentation

#include <iostream>
#include <string>
using namespace std;

#include <Graphical_model.h>
#include "../Utilities.h"
using namespace EFG;



void part_01();
void part_02();
void part_03();
void part_04();

int main() {

	///////////////////////////////////////////
	//            part 01 graph_1, simple politree            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	part_01();
	
	/////////////////////////////////////////////
	////            part 02 graph_2, complex politree            //
	/////////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	part_02();

	/////////////////////////////////////////////
	////            part 03 graph_3, simple loopy graph            //
	/////////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 03 \n\n\n";
	cout << "-----------------------\n";
	part_03();

	/////////////////////////////////////////////
	////            part 04 graph_4, complex loopy graph            //
	/////////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 04 \n\n\n";
	cout << "-----------------------\n";
	part_04();


	return 0;
}

void part_01() {

	//import the graph described in 'Sample 03: Belief propagation, part B / part 01' from an existing xml file
	Graph graph_1("graph_1.xml", Get_prefix() + "Sample_03_graphs/"); 

	vector<float> marginals;

	float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);
	float Z;

	// set E=1 as an evidence
	graph_1.Set_Evidences(vector<pair<string, size_t>>{ {"E", 1} });
	cout << endl << endl;
	cout << "E=1\n";

	// compute the marginals distributions of the other variables and compare it 
	// with the theoretical results (see also 'Sample 03: Belief propagation, part B / part 01')
	marginals = graph_1.Get_marginal_distribution("A");
	cout << "P(A|E)\n";
	print_distribution(marginals);
	Z = (1.f + a) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ (a*(g + e) + (1 + g * e)) / Z, ((g + e) + a * (1 + g * e)) / Z }));
	cout << endl;

	marginals =  graph_1.Get_marginal_distribution("B");
	cout << "P(B|E)\n";
	print_distribution(marginals);
	Z = 1.f + e + g + e * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (g + e) / Z,(1 + g * e) / Z }));
	cout << endl;

	marginals = graph_1.Get_marginal_distribution("C");
	cout << "P(C|E)\n";
	print_distribution(marginals);
	Z = (1.f + b) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ (b*(g + e) + (1 + g * e)) / Z, ((g + e) + b * (1 + g * e)) / Z }));
	cout << endl;

	marginals = graph_1.Get_marginal_distribution("D");
	cout << "P(D|E)\n";
	print_distribution(marginals);
	Z = 1.f + e;
	cout << "theoretical\n";
	print_distribution(list<float>({ 1.f / Z,e / Z }));
	cout << endl;


	// set E=0 as an evidence and recompute the marginals
	graph_1.Set_Evidences( vector<size_t>{ 0 });
	cout << endl << endl;
	cout << "E=0\n";



	marginals = graph_1.Get_marginal_distribution("A");
	cout << "P(A|E)\n";
	print_distribution(marginals);
	Z = (1.f + a) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ ((g + e) + a * (1 + g * e)) / Z, (a*(g + e) + (1 + g * e)) / Z }));
	cout << endl;

	marginals = graph_1.Get_marginal_distribution("B");
	cout << "P(B|E)\n";
	print_distribution(marginals);
	Z = 1.f + e + g + e * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (1 + g * e) / Z , (g + e) / Z }));
	cout << endl;

	marginals =  graph_1.Get_marginal_distribution("C");
	cout << "P(C|E)\n";
	print_distribution(marginals);
	Z = (1.f + b) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ ((g + e) + b * (1 + g * e)) / Z , (b*(g + e) + (1 + g * e)) / Z }));
	cout << endl;

	marginals = graph_1.Get_marginal_distribution("D");
	cout << "P(D|E)\n";
	print_distribution(marginals);
	Z = 1.f + e;
	cout << "theoretical\n";
	print_distribution(list<float>({ e / Z, 1.f / Z }));
	cout << endl;



	// set D=1 as an evidence and recompute the marginals of the hidden variables (including E)
	graph_1.Set_Evidences(vector<pair<string, size_t>>{ {"D", 1} });
	cout << endl << endl;
	cout << "D=1\n";


	marginals = graph_1.Get_marginal_distribution("A");
	cout << "P(A|D)\n";
	print_distribution(marginals);
	Z = 1.f + a + g + a * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (a + g) / Z, (1.f + a * g) / Z }));
	cout << endl;

	marginals = graph_1.Get_marginal_distribution("B");
	cout << "P(B|D)\n";
	print_distribution(marginals);
	Z = 1.f + g;
	cout << "theoretical\n";
	print_distribution(list<float>({ 1.f / Z, g / Z }));
	cout << endl;

	marginals = graph_1.Get_marginal_distribution("C");
	cout << "P(C|D)\n";
	print_distribution(marginals);
	Z = 1.f + b + g + b * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (b + g) / Z, (1.f + b * g) / Z }));
	cout << endl;

	marginals = graph_1.Get_marginal_distribution("E");
	cout << "P(E|D)\n";
	print_distribution(marginals);
	Z = 1.f + e;
	cout << "theoretical\n";
	print_distribution(list<float>({ 1.f / Z, e / Z }));
	cout << endl;

}

void part_02() {

	//import the graph described in 'Sample 03: Belief propagation, part B / part 02' from an existing xml file
	Graph politree("graph_2.xml", Get_prefix() +  "Sample_03_graphs/");

	vector<float> marginals;

	// set v1,v2,v3 as observations and use a Gibbs sampler 
	// to produce samples for the joint conditioned (to the observations) distribution of the hidden variables
	politree.Set_Evidences(vector<pair<string, size_t>>{ {"v1",1}, { "v2",1 }, { "v3",1 }});
	std::list<std::vector<size_t>> sample = politree.Gibbs_Sampling_on_Hidden_set(100, 200);

	vector<Categoric_var*> hidden_set = politree.Get_Actual_Hidden_Set();

	//compare the computed marginals with the ones coming from the samples obtained by the Gibbs sampler, for some hidden variables
	cout << "P(v10 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, politree.Find_Variable("v10"), hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	marginals = politree.Get_marginal_distribution("v10");
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

	cout << "P(v11 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, politree.Find_Variable("v11"), hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	marginals = politree.Get_marginal_distribution("v11");
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

	cout << "P(v12 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, politree.Find_Variable("v12"), hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	marginals = politree.Get_marginal_distribution("v12");
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

}

void part_03() {

	vector<float> marginals;
	vector<float> marginals_theoretical;

	//import the graph described in 'Sample 03: Belief propagation, part B / part 03' from an existing xml file
	Graph loop("graph_3.xml", Get_prefix() +  "Sample_03_graphs/");

	// set the observation
	loop.Set_Evidences(vector<pair<string, size_t>>{ {"E",1}});
	cout << endl << endl;
	cout << "E=1\n";

	// compute the marginals distributions of the hidden variables and compare it 
	// with the theoretical results (see also 'Sample 03: Belief propagation, part B / part 03')

	float M = expf(1.f);
	float M_alfa = powf(M, 3) + M + 2.f*powf(M, 2);
	float M_beta = powf(M, 4) + 2.f*M + powf(M, 2);

	cout << "P(D|E)\n";
	marginals_theoretical = { 3.f*M + powf(M,3), powf(M,4) + 3.f*powf(M,2) };
	normalize(&marginals_theoretical);
	cout << "theoretical\n";
	print_distribution(marginals_theoretical);
	marginals = loop.Get_marginal_distribution("D");
	print_distribution(marginals);
	cout << endl << endl;

	cout << "P(C|E)\n";
	marginals_theoretical = { M_alfa, M_beta };
	normalize(&marginals_theoretical);
	cout << "theoretical\n";
	print_distribution(marginals_theoretical);
	marginals = loop.Get_marginal_distribution("C");
	print_distribution(marginals);
	cout << endl << endl;

	cout << "P(B|E)\n";
	marginals_theoretical = { M_alfa, M_beta };
	normalize(&marginals_theoretical);
	cout << "theoretical\n";
	print_distribution(marginals_theoretical);
	marginals = loop.Get_marginal_distribution("B");
	print_distribution(marginals);
	cout << endl << endl;

	cout << "P(A|E)\n";
	marginals_theoretical = { M*M_alfa + M_beta, M_alfa + M * M_beta };
	normalize(&marginals_theoretical);
	cout << "theoretical\n";
	print_distribution(marginals_theoretical);
	marginals = loop.Get_marginal_distribution("A");
	print_distribution(marginals);
	cout << endl << endl;

}

void part_04() {

	//import the graph described in 'Sample 03: Belief propagation, part B / part 04' from an existing xml file
	Graph loop("graph_4.xml", Get_prefix() +  "Sample_03_graphs/");

	//Categoric_var* v1 = loop.Find_Variable("v1");
	//Categoric_var* v8 = loop.Find_Variable("v8");

	vector<float> marginals;

	// set v1=1 as an evidence and use a Gibbs sampler 
	// to produce samples for the joint conditioned (to the observations) distribution of the hidden variables
	loop.Set_Evidences(vector<pair<string, size_t>>{ {"v1", 1} });

	list<vector<size_t>> sample = loop.Gibbs_Sampling_on_Hidden_set(200, 50);

	vector<Categoric_var*> hidden_set= loop.Get_Actual_Hidden_Set();

	//compare the computed marginals with the ones coming from the samples obtained by the Gibbs sampler
	cout << "P(v8 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, loop.Find_Variable("v8"), hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	marginals = loop.Get_marginal_distribution("v8");
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

}
