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

// File generated at Sun 19 Apr 2015 20:31:39

#ifndef CNE6SSMSusy_TWO_SCALE_HIGH_SCALE_CONSTRAINT_H
#define CNE6SSMSusy_TWO_SCALE_HIGH_SCALE_CONSTRAINT_H

#include "CNE6SSMSusy_high_scale_constraint.hpp"
#include "CNE6SSMSusy_two_scale_input_parameters.hpp"
#include "two_scale_constraint.hpp"

namespace flexiblesusy {

template <class T>
class CNE6SSMSusy;

class Two_scale;

template<>
class CNE6SSMSusy_high_scale_constraint<Two_scale> : public Constraint<Two_scale> {
public:
   CNE6SSMSusy_high_scale_constraint();
   CNE6SSMSusy_high_scale_constraint(CNE6SSMSusy<Two_scale>*);
   virtual ~CNE6SSMSusy_high_scale_constraint();
   virtual void apply();
   virtual double get_scale() const;
   virtual void set_model(Two_scale_model*);

   void clear();
   double get_initial_scale_guess() const;
   const CNE6SSMSusy_input_parameters<Two_scale>& get_input_parameters() const;
   CNE6SSMSusy<Two_scale>* get_model() const;
   void initialize();
   void set_scale(double); ///< fix unification scale (0 = unfixed)

protected:
   void update_scale();

private:
   double scale;
   double initial_scale_guess;
   CNE6SSMSusy<Two_scale>* model;
};

} // namespace flexiblesusy

#endif
