/**
 * Author:    Andrea Casalino
 * Created:   04.06.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef ADVANCED_TRAINERS_ENABLED
#include <trainers/strategies/BFGS.h>

namespace EFG::train {
    void BFGS::updateInvHessian(const Vect& deltaWeight, const Vect& deltaGrad) {
        float rho = 1.f / dot(deltaWeight, deltaGrad);
        Matr V(deltaGrad, deltaWeight);
        V *= - rho;
        V.addIdentity();
        Matr Vtrasp(deltaWeight, deltaGrad);
        Vtrasp *= -rho;
        Vtrasp.addIdentity();

        this->invHessianApprox *= V;
        Vtrasp *= this->invHessianApprox;
        this->invHessianApprox = std::move(Vtrasp);
        Matr S(deltaWeight, deltaWeight);
        S *= rho;
        this->invHessianApprox += S;
    }
}
#endif
