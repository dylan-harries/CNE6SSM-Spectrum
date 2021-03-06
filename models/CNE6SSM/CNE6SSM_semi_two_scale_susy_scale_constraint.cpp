// ====================================================================
// Test implementation of SUSY scale constraint to be used in
// semianalytic version of the two-scale algorithm
// ====================================================================

#include "CNE6SSM_semi_two_scale_susy_scale_constraint.hpp"
#include "CNE6SSM_semi_two_scale_model.hpp"
#include "wrappers.hpp"
#include "logger.hpp"
#include "ew_input.hpp"
#include "gsl_utils.hpp"
#include "minimizer.hpp"
#include "root_finder.hpp"

#include <cassert>
#include <cmath>

namespace flexiblesusy {

#define INPUTPARAMETER(p) model->get_input().p
#define MODELPARAMETER(p) model->get_##p()
#define PHASE(p) model->get_##p()
#define BETAPARAMETER(p) beta_functions.get_##p()
#define BETA(p) beta_##p
#define LowEnergyConstant(p) Electroweak_constants::p
#define STANDARDDEVIATION(p) Electroweak_constants::Error_##p
#define Pole(p) model->get_physical().p
#define MODEL model
#define MODELCLASSNAME CNE6SSM_semianalytic<Two_scale>

CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::CNE6SSM_semianalytic_susy_scale_constraint()
   : Constraint<Two_scale>()
   , scale(0.)
   , initial_scale_guess(0.)
   , model(0)
   , high_constraint(0)
{
}

CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::CNE6SSM_semianalytic_susy_scale_constraint(
   CNE6SSM_semianalytic<Two_scale>* model_)
   : Constraint<Two_scale>()
   , model(model_)
   , high_constraint(0)
{
   initialize();
}

CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::~CNE6SSM_semianalytic_susy_scale_constraint()
{
}

void CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::apply()
{
   assert(model && "Error: CNE6SSM_semianalytic_susy_scale_constraint::apply():"
          " model pointer must not be zero");

   double input_scale;
   if (high_constraint) {
      input_scale = high_constraint->get_scale();
   } else {
      input_scale = scale;
   }

   model->calculate_coefficients(input_scale);
   model->calculate_DRbar_masses();
   update_scale();

   // apply user-defined susy scale constraints

   // the parameters, which are fixed by the EWSB eqs., will now be
   // defined at this scale (at the EWSB loop level defined in the
   // model)
   model->solve_ewsb();

   // DH: note saving VEV values to freeze D-terms at this scale
   model->save_vev_values();
}

double CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::get_scale() const
{
   return scale;
}

double CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::get_initial_scale_guess() const
{
   return initial_scale_guess;
}

const CNE6SSM_semianalytic_input_parameters<Two_scale>& CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::get_input_parameters() const
{
   assert(model && "Error: CNE6SSM_semianalytic_susy_scale_constraint::"
          "get_input_parameters(): model pointer is zero.");

   return model->get_input();
}

CNE6SSM_semianalytic<Two_scale>* CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::get_model() const
{
   return model;
}

void CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::set_model(Two_scale_model* model_)
{
   model = cast_model<CNE6SSM_semianalytic<Two_scale>*>(model_);
}

CNE6SSM_semianalytic_high_scale_constraint<Two_scale>* CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::get_input_scale_constraint() const
{
   return high_constraint;
}

void CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::set_input_scale_constraint(CNE6SSM_semianalytic_high_scale_constraint<Two_scale>* constraint)
{
   high_constraint = constraint;
}

void CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::clear()
{
   scale = 0.;
   initial_scale_guess = 0.;
   model = NULL;
   high_constraint = NULL;
}

void CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::initialize()
{
   assert(model && "CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::"
          "initialize(): model pointer is zero.");

   const auto sInput = INPUTPARAMETER(sInput);
   const auto SigmaxInput = INPUTPARAMETER(SigmaxInput);

   initial_scale_guess = SigmaxInput * sInput;

   scale = initial_scale_guess;
}

void CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::update_scale()
{
   assert(model && "CNE6SSM_semianalytic_susy_scale_constraint<Two_scale>::"
          "update_scale(): model pointer is zero.");

   const auto ZU = MODELPARAMETER(ZU);
   const auto MSu = MODELPARAMETER(MSu);

   scale = Sqrt(Power(MSu(0),Sqr(Abs(ZU(0,2))) + Sqr(Abs(ZU(0,5))))*Power(MSu(1
      ),Sqr(Abs(ZU(1,2))) + Sqr(Abs(ZU(1,5))))*Power(MSu(2),Sqr(Abs(ZU(2,2))) +
      Sqr(Abs(ZU(2,5))))*Power(MSu(3),Sqr(Abs(ZU(3,2))) + Sqr(Abs(ZU(3,5))))*Power
      (MSu(4),Sqr(Abs(ZU(4,2))) + Sqr(Abs(ZU(4,5))))*Power(MSu(5),Sqr(Abs(ZU(5,2))
      ) + Sqr(Abs(ZU(5,5)))));


}

} // namespace flexiblesusy
