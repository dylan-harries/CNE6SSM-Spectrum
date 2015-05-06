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

// File generated at Sun 19 Apr 2015 20:37:13

/**
 * @file CNE6SSM_two_scale_model.hpp
 * @brief contains class for model with routines needed to solve boundary
 *        value problem using the two_scale solver by solving EWSB
 *        and determine the pole masses and mixings
 *
 * This file was generated at Sun 19 Apr 2015 20:37:13 with FlexibleSUSY
 * 1.0.4 (git commit: v1.0.3-cpc6-749-g227a308) and SARAH 4.5.3 .
 */

#ifndef CNE6SSM_TWO_SCALE_H
#define CNE6SSM_TWO_SCALE_H

#include "CNE6SSM_model.hpp"
#include "CNE6SSM_mass_eigenstates.hpp"
#include "CNE6SSM_info.hpp"
#include "CNE6SSM_two_scale_input_parameters.hpp"
#include "two_scale_model.hpp"
#include "config.h"

#include <iosfwd>
#include <string>

#include <gsl/gsl_vector.h>
#include <Eigen/Core>

namespace flexiblesusy {

class EWSB_solver;
class Two_scale;
/**
 * @class CNE6SSM<Two_scale>
 * @brief model class with routines for determing masses and mixings and EWSB
 */
template<>
class CNE6SSM<Two_scale> : public Two_scale_model, public CNE6SSM_mass_eigenstates {
public:
   explicit CNE6SSM(const CNE6SSM_input_parameters<Two_scale>& input_ = CNE6SSM_input_parameters<Two_scale>());
   virtual ~CNE6SSM();

   virtual void clear();
   void set_ewsb_iteration_precision(double);
   void set_ewsb_loop_order(unsigned);
   void set_number_of_ewsb_iterations(std::size_t);
   const CNE6SSM_input_parameters<Two_scale>& get_input() const;
   void set_input_parameters(const CNE6SSM_input_parameters<Two_scale>&);
   double get_ewsb_iteration_precision() const;
   double get_ewsb_loop_order() const;
   int solve_ewsb_tree_level();
   int solve_ewsb_one_loop();
   int solve_ewsb();            ///< solve EWSB at ewsb_loop_order level

   // interface functions
   virtual void calculate_spectrum();
   virtual void clear_problems();
   virtual std::string name() const;
   virtual void run_to(double scale, double eps = -1.0);
   virtual void print(std::ostream&) const;
   virtual void set_precision(double);

   // model interface
   double get_parameter(unsigned) const;
   void set_parameter(unsigned, double);

   // calculate coefficients in expansion of low energy parameters
   Eigen::Array<double,2,1> get_soft_gaugino_mass_coeffs(CNE6SSM_info::Parameters, double, double) const;
   Eigen::Array<double,4,1> get_soft_scalar_mass_coeffs(CNE6SSM_info::Parameters, double, double) const;
   Eigen::Array<double,2,1> get_soft_trilinear_coeffs(CNE6SSM_info::Parameters, double, double) const;

   double get_soft_mass_squared(const CNE6SSM_soft_parameters&, CNE6SSM_info::Parameters) const;
   double get_soft_trilinear(const CNE6SSM_soft_parameters&, CNE6SSM_info::Parameters) const;
   void set_pars_at_high_scale(CNE6SSM_soft_parameters &, double, double, double) const;

private:
   struct EWSB_args {
      CNE6SSM<Two_scale>* model;
      unsigned ewsb_loop_order;
   };

   // input parameters
   CNE6SSM_input_parameters<Two_scale> input;

   std::size_t number_of_ewsb_iterations;
   unsigned ewsb_loop_order;
   double precision;              ///< RG running precision
   double ewsb_iteration_precision;
   static const std::size_t number_of_ewsb_equations = 5;

   int solve_ewsb_iteratively();
   int solve_ewsb_iteratively(unsigned);
   int solve_ewsb_iteratively_with(EWSB_solver*, const double[number_of_ewsb_equations]);
   int solve_ewsb_tree_level_via_soft_higgs_masses();
   int check_ewsb_solution(double);
   void ewsb_initial_guess(double[number_of_ewsb_equations]);
   int ewsb_step(double[number_of_ewsb_equations]);
   static int ewsb_step(const gsl_vector*, void*, gsl_vector*);
   void tadpole_equations(double[number_of_ewsb_equations]) const;
   static int tadpole_equations(const gsl_vector*, void*, gsl_vector*);

};

std::ostream& operator<<(std::ostream&, const CNE6SSM<Two_scale>&);

} // namespace flexiblesusy

#endif
