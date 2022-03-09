/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <model/Graph.h>
#include <io/xml/Importer.h>
#include <Presenter.h>
#include <Frequencies.h>
#include <print/ProbabilityDistributionPrint.h>
#include <math.h>
#include <iostream>
using namespace std;
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::io;

int main() {
	EFG::sample::samplePart([]() {
		model::Graph graph;
		//import the graph described in 'Sample 03: Belief propagation, part B / part 01' from an existing xml file
		xml::Importer::importFromXml(graph, EFG::io::FilePath(SAMPLE_FOLDER, "graph_1.xml"));
		float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);

		// set E=1 as an evidence
		graph.resetEvidences(std::map<std::string, std::size_t>{ {"E", 1} });
		cout << endl << endl;
		cout << "E=1\n";

		// compute the marginals distributions of the other variables and compare it 
		cout << "P(A|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ (a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e)) }) << endl;
		cout << graph.getMarginalDistribution("A") << endl << endl;

		cout << "P(B|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ (g + e) ,(1 + g * e) }) << endl;
		cout << graph.getMarginalDistribution("B") << endl << endl;

		cout << "P(C|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ (b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e)) }) << endl;
		cout << graph.getMarginalDistribution("C") << endl << endl;

		cout << "P(D|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ 1.f, e }) << endl;
		cout << graph.getMarginalDistribution("D") << endl << endl;

		// set E=0 as an evidence and recompute the marginals
		graph.setEvidences({ 0 });
		cout << endl << endl;
		cout << "E=0\n";

		cout << "P(A|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ (g + e) + a * (1 + g * e), a * (g + e) + (1 + g * e) }) << endl;
		cout << graph.getMarginalDistribution("A") << endl << endl;

		cout << "P(B|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ 1 + g * e , g + e }) << endl;
		cout << graph.getMarginalDistribution("B") << endl << endl;

		cout << "P(C|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ (g + e) + b * (1 + g * e) , b * (g + e) + (1 + g * e) }) << endl;
		cout << graph.getMarginalDistribution("C") << endl << endl;

		cout << "P(D|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ e , 1.f  }) << endl;
		cout << graph.getMarginalDistribution("D") << endl << endl;

		// set D=1 as an evidence and recompute the marginals of the hidden variables (including E)
		graph.resetEvidences(std::map<std::string, std::size_t>{ {"D", 1} });
		cout << endl << endl;
		cout << "D=1\n";

		cout << "P(A|D)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ a + g, 1.f + a * g }) << endl;
		cout << graph.getMarginalDistribution("A") << endl << endl;

		cout << "P(B|D)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ 1.f, g }) << endl;
		cout << graph.getMarginalDistribution("B") << endl << endl;

		cout << "P(C|D)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ b + g, 1.f + b * g }) << endl;
		cout << graph.getMarginalDistribution("C") << endl << endl;

		cout << "P(E|D)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ 1.f, e }) << endl;
		cout << graph.getMarginalDistribution("E") << endl << endl;
	}, "Simple polytree belief propagation", "refer to Section 4.3.1 of the documentation");

	EFG::sample::samplePart([]() {
		model::Graph politree;
		//import the graph an existing xml file
		xml::Importer::importFromXml(politree, EFG::io::FilePath(SAMPLE_FOLDER, "graph_2.xml"));
		
		//active the thread pool to fasten the computation
#ifdef THREAD_POOL_ENABLED
		politree.setThreadPoolSize(3);
#endif

		// set v1,v2,v3 as observations and use a Gibbs sampler 
		// to produce samples for the joint conditioned (to the observations) distribution of the hidden variables
		politree.resetEvidences(std::map<std::string, std::size_t>{ {"v1", 1}, {"v2", 1}, { "v3", 1 } });
		auto samples = politree.getHiddenSetSamples(500, 100);
		
		auto hidden_set = politree.getHiddenVariables();
		
		//compare the computed marginals with the ones coming from the samples obtained by the Gibbs sampler, for some hidden variables
		cout << "P(v10 | Observations): \n";
		cout << "empirical\n";
		cout << sample::getEmpiricalMarginalFrequencies(politree.findVariable("v10"), samples, hidden_set) << endl;
		cout << politree.getMarginalDistribution("v10") << endl << endl;

		cout << "P(v11 | Observations): \n";
		cout << "empirical\n";
		cout << sample::getEmpiricalMarginalFrequencies(politree.findVariable("v11"), samples, hidden_set) << endl;
		cout << politree.getMarginalDistribution("v11") << endl << endl;

		cout << "P(v12 | Observations): \n";
		cout << "empirical\n";
		cout << sample::getEmpiricalMarginalFrequencies(politree.findVariable("v12"), samples, hidden_set) << endl;
		cout << politree.getMarginalDistribution("v12") << endl << endl;
	}, "Complex polytree belief propagation", "refer to Section 4.3.2 of the documentation");

	EFG::sample::samplePart([]() {
		model::Graph loop;
		//import the graph an existing xml file
		xml::Importer::importFromXml(loop, EFG::io::FilePath(SAMPLE_FOLDER, "graph_3.xml"));
		
		// set the observation
		loop.resetEvidences(std::map<std::string, std::size_t>{ {"E", 1} });
		cout << endl << endl;
		cout << "E=1\n";
		
		// compute the marginals distributions of the hidden variables and compare it 		
		float M = expf(1.f);
		float M_alfa = powf(M, 3) + M + 2.f*powf(M, 2);
		float M_beta = powf(M, 4) + 2.f*M + powf(M, 2);

		cout << "P(D|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ 3.f * M + powf(M,3), powf(M,4) + 3.f * powf(M,2) }) << endl;
		cout << loop.getMarginalDistribution("D") << endl << endl;

		cout << "P(C|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ M_alfa, M_beta }) << endl;
		cout << loop.getMarginalDistribution("C") << endl << endl;

		cout << "P(B|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ M_alfa, M_beta }) << endl;
		cout << loop.getMarginalDistribution("B") << endl << endl;
		
		cout << "P(A|E)\n";
		cout << "theoretical\n";
		cout << sample::makeDistribution({ M * M_alfa + M_beta, M_alfa + M * M_beta }) << endl;
		cout << loop.getMarginalDistribution("A") << endl << endl;
	}, "Simple loopy model belief propagation", "refer to Section 4.3.3 of the documentation");

	EFG::sample::samplePart([]() {
		model::Graph loop;
		//import the graph from an existing xml file
		xml::Importer::importFromXml(loop, EFG::io::FilePath(SAMPLE_FOLDER, "graph_4.xml"));
		
		//active the thread pool to fasten the computation
#ifdef THREAD_POOL_ENABLED
		loop.setThreadPoolSize(3);
#endif
		
		// set v1=1 as an evidence and use a Gibbs sampler 
		// to produce samples for the joint conditioned (to the observations) distribution of the hidden variables
		loop.resetEvidences(std::map<std::string, std::size_t>{ {"v1", 1} });
		
		auto samples = loop.getHiddenSetSamples(500, 100);
		
		auto hidden_set = loop.getHiddenVariables();
		
		//compare the computed marginals with the ones coming from the samples obtained by the Gibbs sampler
		cout << "P(v8 | Observations): \n";
		cout << "empirical\n";
		cout << sample::getEmpiricalMarginalFrequencies(loop.findVariable("v8"), samples, hidden_set) << endl;
		cout << loop.getMarginalDistribution("v8") << endl << endl;
	}, "Complex loopy model belief propagation", "refer to Section 4.3.4 of the documentation");

	return EXIT_SUCCESS;
}
