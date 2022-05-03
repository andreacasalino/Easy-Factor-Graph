/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/trainable/ModelTrainer.h>

using namespace EFG::model;
using namespace EFG::distribution;
using namespace EFG::categoric;
using namespace EFG::strct;
using namespace EFG::io;
using namespace EFG::train;

// you can also use another iterative trainer
#include <TrainingTools/iterative/solvers/QuasiNewton.h>

using namespace train;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  {
    SampleSection section("Tuning of a conditional random field ", "4.7");

    RandomField temporary_imported_structure;
    xml::Importer::importFromFile(temporary_imported_structure,
                                  SAMPLE_FOLDER +
                                      std::string{"cond_graph.xml"});
    ConditionalRandomField conditional_field(temporary_imported_structure,
                                             false);

    cout << "creating the training set, might take a while" << endl;
    TrainSet train_set(conditional_field.makeTrainSet(
        GibbsSampler::SamplesGenerationContext{20, 50, 0}, 0.7f, 3));
    cout << "training set created" << endl;
    cout << train_set.getCombinations().size() << endl;

    const auto expected_weights = conditional_field.getWeights();

    // set all weights to 1 and train the model on the previously generated
    // train set
    set_ones(conditional_field);
    QuasiNewton trainer;
    trainer.setMaxIterations(15);
    cout << "training the model, this might take a while as conditional random "
            "field are much more computationally demanding"
         << endl;
    trainer.enablePrintAdvancement();
    train_model(conditional_field, trainer, train_set, TrainInfo{3, 1.f});

    cout << "expected weights:    " << expected_weights << endl;
    cout << "wieghts after train: " << conditional_field.getWeights() << endl;
  }

  return EXIT_SUCCESS;
}
