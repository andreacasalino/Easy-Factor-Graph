#include <gtest/gtest.h>
#include <model/Graph.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

// tests of Evidence Setter and changer: check hidden clusters and evidence are correctly updated 

class GraphTest 
: public ::testing::Test
, public model::Graph { 
protected: 
    GraphTest() = default;
};

TEST_F(GraphTest, resetEvidences) {
    
}

TEST_F(GraphTest, addEvidence) {
    
}

TEST_F(GraphTest, setEvidences) {

}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
