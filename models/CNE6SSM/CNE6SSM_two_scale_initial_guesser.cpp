// ====================================================================
// This file is part of FlexibleSUSY.
//
// FlexibleSUSY is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// FlexibleSUSY is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FlexibleSUSY.  If not, see
// <http://www.gnu.org/licenses/>.
// ====================================================================

// File generated at Fri 26 Sep 2014 11:53:35

#include "CNE6SSM_two_scale_initial_guesser.hpp"
#include "CNE6SSM_two_scale_model.hpp"
#include "lowe.h"
#include "error.hpp"
#include "ew_input.hpp"
#include "wrappers.hpp"

#include <Eigen/Core>
#include <cassert>

namespace flexiblesusy {

#define INPUTPARAMETER(p) input_pars.p
#define MODELPARAMETER(p) model->get_##p()
#define SM(p) Electroweak_constants::p
#define MODEL model

CNE6SSM_initial_guesser<Two_scale>::CNE6SSM_initial_guesser(
   CNE6SSM<Two_scale>* model_,
   const CNE6SSM_input_parameters& input_pars_,
   const QedQcd& oneset_,
   const CNE6SSM_low_scale_constraint<Two_scale>& low_constraint_,
   const CNE6SSM_susy_scale_constraint<Two_scale>& susy_constraint_,
   const CNE6SSM_high_scale_constraint<Two_scale>& high_constraint_
)
   : Initial_guesser<Two_scale>()
   , model(model_)
   , input_pars(input_pars_)
   , oneset(oneset_)
   , mu_guess(0.)
   , mc_guess(0.)
   , mt_guess(0.)
   , md_guess(0.)
   , ms_guess(0.)
   , mb_guess(0.)
   , me_guess(0.)
   , mm_guess(0.)
   , mtau_guess(0.)
   , running_precision(1.0e-3)
   , low_constraint(low_constraint_)
   , susy_constraint(susy_constraint_)
   , high_constraint(high_constraint_)
{
   assert(model && "CNE6SSM_initial_guesser: Error: pointer to model"
          " CNE6SSM<Two_scale> must not be zero");
}

CNE6SSM_initial_guesser<Two_scale>::~CNE6SSM_initial_guesser()
{
}

/**
 * Guesses the DR-bar model parameters by calling
 * guess_susy_parameters() and guess_soft_parameters() .
 */
void CNE6SSM_initial_guesser<Two_scale>::guess()
{
   std::cout << "####################################################\n";
   std::cout << "\tstarting initial guess\n";
   std::cout << "####################################################\n";
   guess_susy_parameters();
   guess_soft_parameters();
   std::cout << "####################################################\n";
   std::cout << "\tfinished initial guess\n";
   std::cout << "####################################################\n";
}

/**
 * Guesses the SUSY parameters (gauge, Yukawa couplings) at
 * \f$m_\text{top}^\text{pole}\f$ from the Standard Model gauge
 * couplings and fermion masses.  Threshold corrections are ignored.
 * The user-defined initial guess at the low-scale
 * (InitialGuessAtLowScale) is applied here:
 *
 * \code{.cpp}
   const auto TanBeta = INPUTPARAMETER(TanBeta);
   const auto ssumInput = INPUTPARAMETER(ssumInput);

   MODEL->set_vd(SM(vev)/Sqrt(1 + Sqr(TanBeta)));
   MODEL->set_vu((TanBeta*SM(vev))/Sqrt(1 + Sqr(TanBeta)));
   calculate_Yu_DRbar();
   calculate_Yd_DRbar();
   calculate_Ye_DRbar();
   MODEL->set_XiF(Power(SM(MZ),3));
   MODEL->set_LXiF(Power(SM(MZ),3));
   MODEL->set_vs(0.7071067811865475*ssumInput);
   MODEL->set_vsb(0.7071067811865475*ssumInput);
   MODEL->set_vphi(ssumInput);

 * \endcode
 */
void CNE6SSM_initial_guesser<Two_scale>::guess_susy_parameters()
{
   std::cout << "****************************************************\n";
   std::cout << "\tguessing susy parameters\n";
   std::cout << "****************************************************\n";
   QedQcd leAtMt(oneset);
   const double MZ = Electroweak_constants::MZ;
   const double MW = Electroweak_constants::MW;
   const double sinThetaW2 = 1.0 - Sqr(MW / MZ);
   const double mtpole = leAtMt.displayPoleMt();

   mu_guess = leAtMt.displayMass(mUp);
   mc_guess = leAtMt.displayMass(mCharm);
   mt_guess = leAtMt.displayMass(mTop) - 30.0;
   md_guess = leAtMt.displayMass(mDown);
   ms_guess = leAtMt.displayMass(mStrange);
   mb_guess = leAtMt.displayMass(mBottom);
   me_guess = leAtMt.displayMass(mElectron);
   mm_guess = leAtMt.displayMass(mMuon);
   mtau_guess = leAtMt.displayMass(mTau);

   // guess gauge couplings at mt
   const DoubleVector alpha_sm(leAtMt.getGaugeMu(mtpole, sinThetaW2));

   model->set_g1(sqrt(4.0 * M_PI * alpha_sm(1)));
   model->set_g2(sqrt(4.0 * M_PI * alpha_sm(2)));
   model->set_g3(sqrt(4.0 * M_PI * alpha_sm(3)));
   model->set_scale(mtpole);

   // apply user-defined initial guess at the low scale
   const auto TanBeta = INPUTPARAMETER(TanBeta);
   const auto ssumInput = INPUTPARAMETER(ssumInput);

   MODEL->set_vd(SM(vev)/Sqrt(1 + Sqr(TanBeta)));
   MODEL->set_vu((TanBeta*SM(vev))/Sqrt(1 + Sqr(TanBeta)));
   calculate_Yu_DRbar();
   calculate_Yd_DRbar();
   calculate_Ye_DRbar();
   MODEL->set_XiF(Power(SM(MZ),3));
   MODEL->set_LXiF(Power(SM(MZ),3));
   MODEL->set_vs(0.7071067811865475*ssumInput);
   MODEL->set_vsb(0.7071067811865475*0.95*ssumInput);
   //DH::note change
   MODEL->set_vphi(0.5*ssumInput);
   std::cout << "****************************************************\n";
   std::cout << "Model parameters set at scale = " << model->get_scale() << ":\n";
   std::cout << "g1 = " << model->get_g1() << ", g2 = " << model->get_g2() 
             << ", g3 = " << model->get_g3() << ", g1p = " << model->get_g1p() << "\n";
   std::cout << "Yu(0,0) = " << model->get_Yu(0,0) << ", Yu(1,1) = " << model->get_Yu(1,1)
             << ", Yu(2,2) = " << model->get_Yu(2,2) << "\n";
   std::cout << "Yd(0,0) = " << model->get_Yd(0,0) << ", Yd(1,1) = " << model->get_Yd(1,1)
             << ", Yd(2,2) = " << model->get_Yd(2,2) << "\n";
   std::cout << "Ye(0,0) = " << model->get_Ye(0,0) << ", Yu(1,1) = " << model->get_Ye(1,1)
             << ", Ye(2,2) = " << model->get_Ye(2,2) << "\n";
   std::cout << "vu = " << model->get_vu() << ", vd = " << model->get_vd() << ", vs = "
             << model->get_vs() << ", vsb = " << model->get_vsb() << ", vphi = " << model->get_vphi()
             << "\n";
   std::cout << "XiF = " << model->get_XiF() << ", LXiF = " << model->get_LXiF() << "\n";
   std::cout << "****************************************************\n";
   std::cout << "\tfinished guessing susy parameters\n";
   std::cout << "****************************************************\n";
}

void CNE6SSM_initial_guesser<Two_scale>::calculate_DRbar_yukawa_couplings()
{
   calculate_Yu_DRbar();
   calculate_Yd_DRbar();
   calculate_Ye_DRbar();
}

/**
 * Calculates the Yukawa couplings Yu of the up-type quarks
 * from the Standard Model up-type quark masses (ignoring threshold
 * corrections).
 */
void CNE6SSM_initial_guesser<Two_scale>::calculate_Yu_DRbar()
{
   Eigen::Matrix<double,3,3> topDRbar(Eigen::Matrix<double,3,3>::Zero());
   topDRbar(0,0) = mu_guess;
   topDRbar(1,1) = mc_guess;
   topDRbar(2,2) = mt_guess;

   const auto vu = MODELPARAMETER(vu);
   MODEL->set_Yu(Diag((1.4142135623730951*topDRbar)/vu));

}

/**
 * Calculates the Yukawa couplings Yd of the down-type
 * quarks from the Standard Model down-type quark masses (ignoring
 * threshold corrections).
 */
void CNE6SSM_initial_guesser<Two_scale>::calculate_Yd_DRbar()
{
   Eigen::Matrix<double,3,3> bottomDRbar(Eigen::Matrix<double,3,3>::Zero());
   bottomDRbar(0,0) = md_guess;
   bottomDRbar(1,1) = ms_guess;
   bottomDRbar(2,2) = mb_guess;

   const auto vd = MODELPARAMETER(vd);
   MODEL->set_Yd(Diag((1.4142135623730951*bottomDRbar)/vd));

}

/**
 * Calculates the Yukawa couplings Ye of the leptons
 * from the Standard Model down-type lepton masses (ignoring threshold
 * corrections).
 */
void CNE6SSM_initial_guesser<Two_scale>::calculate_Ye_DRbar()
{
   Eigen::Matrix<double,3,3> electronDRbar(Eigen::Matrix<double,3,3>::Zero());
   electronDRbar(0,0) = me_guess;
   electronDRbar(1,1) = mm_guess;
   electronDRbar(2,2) = mtau_guess;

   const auto vd = MODELPARAMETER(vd);
   MODEL->set_Ye(Diag((1.4142135623730951*electronDRbar)/vd));

}

/**
 * Guesses the soft-breaking parameters.  At first it runs to the
 * guess of the high-scale (HighScaleFirstGuess) and imposes the
 * high-scale constraint (HighScaleInput):
 *
 * \code{.cpp}

 * \endcode
 *
 * Afterwards, it runs to the low-scale guess (LowScaleFirstGuess) and
 * solves the EWSB conditions at the tree-level.  Finally the DR-bar
 * mass spectrum is calculated.
 */
void CNE6SSM_initial_guesser<Two_scale>::guess_soft_parameters()
{
   std::cout << "****************************************************\n";
   std::cout << "\tguessing soft parameters\n";
   std::cout << "****************************************************\n";
   const double low_scale_guess = low_constraint.get_initial_scale_guess();
   const double high_scale_guess = high_constraint.get_initial_scale_guess();

   // DH:: prevent VEVs from running above the initial estimate
   // for the SUSY scale
   const double susy_scale_guess = susy_constraint.get_initial_scale_guess();
   model->set_vev_running_cutoff_scale(2.0 * susy_scale_guess);

   model->run_to(high_scale_guess, running_precision);

   // apply high-scale constraint
   high_constraint.set_model(model);
   high_constraint.apply();

   // apply user-defined initial guess at the high scale


   model->run_to(low_scale_guess, running_precision);

   // apply EWSB constraint
   model->solve_ewsb_tree_level();

   // calculate tree-level spectrum
   model->calculate_DRbar_parameters();
   std::cout << "****************************************************\n";
   std::cout << "Model parameters set at scale = " << model->get_scale() << ":\n";
   std::cout << "g1 = " << model->get_g1() << ", g2 = " << model->get_g2() 
             << ", g3 = " << model->get_g3() << ", g1p = " << model->get_g1p() << "\n";
   std::cout << "Yu(0,0) = " << model->get_Yu(0,0) << ", Yu(1,1) = " << model->get_Yu(1,1)
             << ", Yu(2,2) = " << model->get_Yu(2,2) << "\n";
   std::cout << "Yd(0,0) = " << model->get_Yd(0,0) << ", Yd(1,1) = " << model->get_Yd(1,1)
             << ", Yd(2,2) = " << model->get_Yd(2,2) << "\n";
   std::cout << "Ye(0,0) = " << model->get_Ye(0,0) << ", Yu(1,1) = " << model->get_Ye(1,1)
             << ", Ye(2,2) = " << model->get_Ye(2,2) << "\n";
   std::cout << "vu = " << model->get_vu() << ", vd = " << model->get_vd() << ", vs = "
             << model->get_vs() << ", vsb = " << model->get_vsb() << ", vphi = " << model->get_vphi()
             << "\n";
   std::cout << "XiF = " << model->get_XiF() << ", LXiF = " << model->get_LXiF() << "\n";
   std::cout << "****************************************************\n";
   std::cout << "\tfinished guessing soft parameters\n";
   std::cout << "****************************************************\n";
}

} // namespace flexiblesusy
