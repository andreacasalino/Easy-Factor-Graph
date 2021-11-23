///**
// * Author:    Andrea Casalino
// * Created:   04.06.2021
// *
// * report any bug to andrecasa91@gmail.com.
// **/
//
//#include <trainers/strategies/lBFGS.h>
//
// namespace EFG::train {
//    void LBFGS::updateInvHessian(const Eigen::VectorXf& deltaW, const
//    Eigen::VectorXf& deltaGrad) {
//        {
//            auto rhoV = computeRhoAndV(deltaW, deltaGrad);
//            this->pastIterations.emplace_back(RhoV{rhoV.first, rhoV.second});
//        }
//        if (this->pastIterations.size() <= this->memorySize) {
//            // BFGS standard update
//            this->invHessianApprox = this->pastIterations.back().V.transpose()
//            * this->invHessianApprox * this->pastIterations.back().V;
//            this->invHessianApprox += (1.f / this->pastIterations.back().rho)
//            * deltaW * deltaW.transpose();
//        }
//        else {
//            throw 0; //todo
//        }
//    }
//
//    void LBFGS::reset() {
//        this->HessianApproximator::reset();
//        this->setMemorySize(this->model->getWeights().size());
//    }
//}
