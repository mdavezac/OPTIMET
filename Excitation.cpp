#include "Excitation.h"

#include "Algebra.h"
#include "AuxCoefficients.h"
#include "CompoundIterator.h"
#include "Coupling.h"
#include "Tools.h"
#include "constants.h"

#include <cmath>
#include <iostream>

namespace optimet {
Excitation::Excitation(unsigned long type, SphericalP<std::complex<double>> Einc,
                       Spherical<double> waveKInc, int nMax)
    : Einc(Einc), vKInc(waveKInc), nMax(nMax), type(type), dataIncAp(Tools::iteratorMax(nMax)),
      dataIncBp(Tools::iteratorMax(nMax)), waveK(waveKInc.rrr) {}

void Excitation::update(unsigned long type_, SphericalP<std::complex<double>> Einc_,
                        Spherical<double> vKInc_, int nMax_) {
  type = type_;
  Einc = Einc_;
  vKInc = vKInc_;
  nMax = nMax_;

  waveK = vKInc.rrr;
  populate();
}

int Excitation::populate() {
  optimet::AuxCoefficients coef(Spherical<double>(0.0, vKInc.the, vKInc.phi), waveK, 1, nMax);

  CompoundIterator p;

  for(p = 0; p < p.max(nMax); p++) {
    SphericalP<std::complex<double>> C_local = coef.C(static_cast<long>(p));
    SphericalP<std::complex<double>> B_local = coef.B(static_cast<long>(p));

    SphericalP<std::complex<double>> conjAux(std::conj(C_local.rrr), std::conj(C_local.the),
                                             std::conj(C_local.phi)); // std::complex conjugate of C
    dataIncAp[p] = 4 * constant::pi * std::pow(-1.0, p.second) * std::pow(consCi, p.first) *
                   coef.dn(p.first) * (conjAux * Einc) *
                   std::exp(consCmi * (double)p.second * vKInc.phi);

    conjAux =
        SphericalP<std::complex<double>>(std::conj(B_local.rrr), std::conj(B_local.the),
                                         std::conj(B_local.phi)); // std::complex conjugate of B
    dataIncBp[p] = 4 * constant::pi * std::pow(-1.0, p.second) * std::pow(consCi, p.first - 1) *
                   coef.dn(p.first) * (conjAux * Einc) *
                   std::exp(consCmi * (double)p.second * vKInc.phi);
  }

  return 0;
}

int Excitation::getIncLocal(Spherical<double> point_, std::complex<double> *Inc_local_,
                            int nMax_) const {
  Spherical<double> Rrel = point_ - Spherical<double>(0.0, 0.0, 0.0);
  optimet::Coupling const coupling(Rrel, waveK, nMax_, false);

  CompoundIterator p, q;

  int pMax = p.max(nMax_);
  int qMax = q.max(nMax_);

  std::complex<double> *Inc_direct = new std::complex<double>[2 * pMax];
  std::complex<double> **T_AB = new std::complex<double> *[2 * (p.max(nMax))];
  for(p = 0; p < (int)(2 * p.max(nMax)); p++) {
    T_AB[p] = new std::complex<double>[2 * p.max(nMax)];
  }

  for(p = 0; p < pMax; p++) {
    Inc_direct[p] = dataIncAp[p];
    Inc_direct[p + pMax] = dataIncBp[p];
  }

  for(p = 0; p < pMax; p++) {
    for(q = 0; q < qMax; q++) {
      T_AB[p][q] = coupling.diagonal(q, p);
      T_AB[p + pMax][q + qMax] = coupling.diagonal(q, p);
      T_AB[p + pMax][q] = coupling.offdiagonal(q, p);
      T_AB[p][q + qMax] = coupling.offdiagonal(q, p);
    }
  }

  Algebra::multiplyVectorMatrix(T_AB, 2 * pMax, 2 * pMax, Inc_direct, Inc_local_, consC1, consC0);

  delete[] Inc_direct;

  for(p = 0; p < 2 * pMax; p++) {
    delete[] T_AB[p];
  }

  delete[] T_AB;

  return 0;
}

void Excitation::updateWavelength(double lambda_) {
  Spherical<double> vKInc_local = vKInc;
  vKInc_local.rrr = 2 * constant::pi / lambda_;

  update(type, Einc, vKInc_local, nMax);
}
}
