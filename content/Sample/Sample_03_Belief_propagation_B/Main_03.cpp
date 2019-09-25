#include <iostream>
#include <string>
using namespace std;

#include "../../CRF/Header/Graphical_model.h"
#include "../Prefix_path.h"
#include "../Utilities.h"
using namespace Segugio;
#ifdef _DEBUG
#pragma comment (lib, "../../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../../x64/Release/CRF.lib")
#endif // DEBUG



void process_graph_1(const string& prefix);
void process_graph_2(const string& prefix);
void process_graph_3(const string& prefix);
void process_graph_4(const string& prefix);

int main() {
	string prefix = compute_prefix() + "Sample_03_Belief_propagation_B" + "/";

	///////////////////////////////////////////
	//            part 01 graph_1            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	process_graph_1(prefix);
	system("pause");
	
	///////////////////////////////////////////
	//            part 02 graph_2            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	process_graph_2(prefix);
	system("pause");

	///////////////////////////////////////////
	//            part 03 graph_3            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 03 \n\n\n";
	cout << "-----------------------\n";
	process_graph_3(prefix);
	system("pause");

	///////////////////////////////////////////
	//            part 04 graph_4            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 04 \n\n\n";
	cout << "-----------------------\n";
	process_graph_4(prefix);


	system("pause");
	return 0;
}

void process_graph_1(const string& prefix) {

	Graph graph_1("graph_1.xml", prefix);

	list<float> marginals;

	float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);
	float Z;


	graph_1.Set_Observation_Set_var({ graph_1.Find_Variable("E") });
	graph_1.Set_Observation_Set_val({1});
	cout << endl << endl;
	cout << "E=1\n";


	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("A"));
	cout << "P(A|E)\n";
	print_distribution(marginals);
	Z = (1.f + a) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ (a*(g + e) + (1 + g * e)) / Z, ((g + e) + a * (1 + g * e)) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("B"));
	cout << "P(B|E)\n";
	print_distribution(marginals);
	Z = 1.f + e + g + e * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (g + e) / Z,(1 + g * e) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("C"));
	cout << "P(C|E)\n";
	print_distribution(marginals);
	Z = (1.f + b) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ (b*(g + e) + (1 + g * e)) / Z, ((g + e) + b * (1 + g * e)) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("D"));
	cout << "P(D|E)\n";
	print_distribution(marginals);
	Z = 1.f + e;
	cout << "theoretical\n";
	print_distribution(list<float>({ 1.f / Z,e / Z }));
	cout << endl;





	graph_1.Set_Observation_Set_var({ graph_1.Find_Variable("E") });
	graph_1.Set_Observation_Set_val({ 0 });
	cout << endl << endl;
	cout << "E=0\n";



	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("A"));
	cout << "P(A|E)\n";
	print_distribution(marginals);
	Z = (1.f + a) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ ((g + e) + a * (1 + g * e)) / Z, (a*(g + e) + (1 + g * e)) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("B"));
	cout << "P(B|E)\n";
	print_distribution(marginals);
	Z = 1.f + e + g + e * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (1 + g * e) / Z , (g + e) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("C"));
	cout << "P(C|E)\n";
	print_distribution(marginals);
	Z = (1.f + b) * (1.f + g + e + e * g);
	cout << "theoretical\n";
	print_distribution(list<float>({ ((g + e) + b * (1 + g * e)) / Z , (b*(g + e) + (1 + g * e)) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("D"));
	cout << "P(D|E)\n";
	print_distribution(marginals);
	Z = 1.f + e;
	cout << "theoretical\n";
	print_distribution(list<float>({ e / Z, 1.f / Z }));
	cout << endl;



	graph_1.Set_Observation_Set_var({ graph_1.Find_Variable("D") });
	graph_1.Set_Observation_Set_val({ 1 });
	cout << endl << endl;
	cout << "D=1\n";


	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("A"));
	cout << "P(A|D)\n";
	print_distribution(marginals);
	Z = 1.f + a + g + a * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (a + g) / Z, (1.f + a * g) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("B"));
	cout << "P(B|D)\n";
	print_distribution(marginals);
	Z = 1.f + g;
	cout << "theoretical\n";
	print_distribution(list<float>({ 1.f / Z, g / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("C"));
	cout << "P(C|D)\n";
	print_distribution(marginals);
	Z = 1.f + b + g + b * g;
	cout << "theoretical\n";
	print_distribution(list<float>({ (b + g) / Z, (1.f + b * g) / Z }));
	cout << endl;

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("E"));
	cout << "P(E|D)\n";
	print_distribution(marginals);
	Z = 1.f + e;
	cout << "theoretical\n";
	print_distribution(list<float>({ 1.f / Z, e / Z }));
	cout << endl;

}

void process_graph_2(const string& prefix) {

	Graph politree("graph_2.xml", prefix);

	list<Categoric_var*> vars;
	politree.Get_All_variables_in_model(&vars);

	auto it_var = vars.begin();
	Categoric_var* v1 = *it_var; it_var++;
	Categoric_var* v2 = *it_var; it_var++;
	Categoric_var* v3 = *it_var;

	list<Categoric_var*> hidden_set;


	list<float> marginals;

	//use a Gibbs sampler to produce a training set, conditioned to the observations
	politree.Set_Observation_Set_var({ v1,v2,v3 });
	politree.Set_Observation_Set_val({ 1,1,1 });
	list<list<size_t>> sample;
	politree.Gibbs_Sampling_on_Hidden_set(&sample, 100, 200);

	politree.Get_Actual_Hidden_Set(&hidden_set);

	//compare the computed marginals with the ones coming from the samples obtained by the Gibbs sampler, for some hidden variables
	cout << "P(v10 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, politree.Find_Variable("v10"), hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	politree.Get_marginal_distribution(&marginals, politree.Find_Variable("v10"));
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

	cout << "P(v11 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, politree.Find_Variable("v11"), hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	politree.Get_marginal_distribution(&marginals, politree.Find_Variable("v11"));
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

	cout << "P(v12 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, politree.Find_Variable("v12"), hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	politree.Get_marginal_distribution(&marginals, politree.Find_Variable("v12"));
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

}

void process_graph_3(const string& prefix) {

	list<float> marginals;
	list<float> marginals_theoretical;


	Graph loop("graph_3.xml", prefix);

	Categoric_var* Vars[5];
	Vars[0] = loop.Find_Variable("A");
	Vars[1] = loop.Find_Variable("B");
	Vars[2] = loop.Find_Variable("C");
	Vars[3] = loop.Find_Variable("D");
	Vars[4] = loop.Find_Variable("E");



	loop.Set_Observation_Set_var({ Vars[4] });
	loop.Set_Observation_Set_val({ 1 });
	cout << endl << endl;
	cout << "E=1\n";


	float M = expf(1.f);
	float M_alfa = powf(M, 3) + M + 2.f*powf(M, 2);
	float M_beta = powf(M, 4) + 2.f*M + powf(M, 2);

	cout << "P(D|E)\n";
	marginals_theoretical = { 3.f*M + powf(M,3), powf(M,4) + 3.f*powf(M,2) };
	normalize(&marginals_theoretical);
	print_distribution(marginals_theoretical);
	cout << endl;
	loop.Get_marginal_distribution(&marginals, Vars[3]);
	print_distribution(marginals);
	cout << endl << endl;

	cout << "P(C|E)\n";
	marginals_theoretical = { M_alfa, M_beta };
	normalize(&marginals_theoretical);
	print_distribution(marginals_theoretical);
	cout << endl;
	loop.Get_marginal_distribution(&marginals, Vars[2]);
	print_distribution(marginals);
	cout << endl << endl;

	cout << "P(B|E)\n";
	marginals_theoretical = { M_alfa, M_beta };
	normalize(&marginals_theoretical);
	print_distribution(marginals_theoretical);
	cout << endl;
	loop.Get_marginal_distribution(&marginals, Vars[1]);
	print_distribution(marginals);
	cout << endl << endl;

	cout << "P(A|E)\n";
	marginals_theoretical = { M*M_alfa + M_beta, M_alfa + M * M_beta };
	normalize(&marginals_theoretical);
	print_distribution(marginals_theoretical);
	cout << endl;
	loop.Get_marginal_distribution(&marginals, Vars[0]);
	print_distribution(marginals);
	cout << endl << endl;

}

void process_graph_4(const string& prefix) {

	Graph loop("graph_4.xml", prefix);

	Categoric_var* v1 = loop.Find_Variable("v1");
	Categoric_var* v8 = loop.Find_Variable("v8");


	list<float> marginals;

	//use a Gibbs sampler to produce a training set, conditioned to the observations
	loop.Set_Observation_Set_var({ v1 });
	loop.Set_Observation_Set_val({ 1 });

	list<list<size_t>> sample;
	loop.Gibbs_Sampling_on_Hidden_set(&sample, 200, 50);

	list<Categoric_var*> hidden_set;
	loop.Get_Actual_Hidden_Set(&hidden_set);

	//compare the computed marginals with the ones coming from the samples obtained by the Gibbs sampler
	cout << "P(v8 | Observations): \n";
	Get_empirical_frequencies(&marginals, sample, v8, hidden_set);
	cout << "empirical ";
	print_distribution(marginals);

	loop.Get_marginal_distribution(&marginals, v8);
	cout << "          ";
	print_distribution(marginals);
	cout << endl;

}