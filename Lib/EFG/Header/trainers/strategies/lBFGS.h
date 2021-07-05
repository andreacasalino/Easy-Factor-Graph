///**
// * Author:    Andrea Casalino
// * Created:   04.06.2021
// *
// * report any bug to andrecasa91@gmail.com.
// **/
//
//#ifdef ADVANCED_TRAINERS_ENABLED
//#ifndef EFG_TRAINER_LBFGS_H
//#define EFG_TRAINER_LBFGS_H
//
//#include <trainers/components/HessianApproximator.h>
//#include <Error.h>
//
//namespace EFG::train {
//    class LBFGS
//        : public HessianApproximator {
//    public:
//        void setMemorySize(const std::size_t size) {
//            if (0 == size) {
//                throw Error("Invalid memory size");
//            }
//            this->memorySize = size;
//        };
//
//    private:
//        void updateInvHessian(const Eigen::VectorXf& deltaW, const Eigen::VectorXf& deltaGrad) override;
//
//        void reset() override;
//
//        struct RhoV {
//            float rho;
//            Eigen::MatrixXf V;
//        };
//        std::list<RhoV> pastIterations;
//
//        std::size_t memorySize = 1;
//    };
//}
//
//#endif
//#endif
