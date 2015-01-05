// ====================================================================
// Does a grid scan of the CNE6SSM parameter space, printing
// out the coefficients in the expansions of the soft masses
// mHd2 and mHu2, and in the tree level expansion of Lambdax
// ====================================================================

#include "CNE6SSM_input_parameters.hpp"
#include "CNE6SSM_scan_parameters.hpp"
#include "CNE6SSM_scan_utilities.hpp"
#include "CNE6SSM_spectrum_generator.hpp"

#include "scan_command_line_options.hpp"
#include "error.hpp"
#include "grid_scanner.hpp"
#include "lowe.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <sys/time.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

std::size_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

static std::minstd_rand generator(seed);

namespace flexiblesusy {
   
   void set_default_parameter_values(CNE6SSM_input_parameters& input)
   {
      if (is_zero(input.TanBeta))
         input.TanBeta = 10.0;

      if (is_zero(input.SignLambdax))
         input.SignLambdax = 1;

      input.ssumInput = 40000.0; // GeV
      input.QS = 5.;
      
      input.hEInput(0,0) = 0.;
      input.hEInput(0,1) = 0.;
      input.hEInput(1,0) = 0.;
      input.hEInput(1,1) = 0.;
      input.hEInput(2,0) = 0.;
      input.hEInput(2,1) = 0.;
      
      input.SigmaLInput = 3.0e-1;
      input.KappaPrInput = 2.0e-2;
      input.SigmaxInput = 1.0e-1;
      
      input.gDInput(0,0) = 0.;
      input.gDInput(0,1) = 0.;
      input.gDInput(0,2) = 0.;
      input.gDInput(1,0) = 0.;
      input.gDInput(1,1) = 0.;
      input.gDInput(1,2) = 0.;
      input.gDInput(2,0) = 0.;
      input.gDInput(2,1) = 0.;
      input.gDInput(2,2) = 0.;
      
      input.KappaInput(0,0) = 2.0e-1;
      input.KappaInput(0,1) = 0.;
      input.KappaInput(0,2) = 0.;
      input.KappaInput(1,0) = 0.;
      input.KappaInput(1,1) = 2.0e-1;
      input.KappaInput(1,2) = 0.;
      input.KappaInput(2,0) = 0.;
      input.KappaInput(2,1) = 0.;
      input.KappaInput(2,2) = 2.0e-1;
      
      input.Lambda12Input(0,0) = 5.0e-1;
      input.Lambda12Input(0,1) = 0.;
      input.Lambda12Input(1,0) = 0.;
      input.Lambda12Input(1,1) = 5.0e-1;
      
      input.fuInput(0,0) = 1.0e-7;
      input.fuInput(0,1) = 0.;
      input.fuInput(1,0) = 0.;
      input.fuInput(1,1) = 1.0e-7;
      input.fuInput(2,0) = 1.0e-7;
      input.fuInput(2,1) = 0.;
      
      input.fdInput(0,0) = 1.0e-7;
      input.fdInput(0,1) = 0.;
      input.fdInput(1,0) = 0.;
      input.fdInput(1,1) = 1.0e-7;
      input.fdInput(2,0) = 0.;
      input.fdInput(2,1) = 1.0e-7;
      
      input.MuPrInput = 1.0e4;
      input.MuPhiInput = 0.;
      input.BMuPrInput = 1.0e4;
      input.BMuPhiInput = 0.;
      
   }

   void set_minpar_values(CNE6SSM_scan_parameters params, const std::vector<std::size_t>& posn, CNE6SSM_input_parameters& input)
   {
      if (params.get_is_grid_scan()) {

         input.m0 = params.get_m0_lower() + params.get_m0_incr() * posn.at(0);
         input.m12 = params.get_m12_lower() + params.get_m12_incr() * posn.at(1);
         input.TanBeta = params.get_TanBeta_lower() + params.get_TanBeta_incr() * posn.at(2);
         input.SignLambdax = params.get_SignLambdax_lower() + params.get_SignLambdax_incr() * posn.at(3);
         input.Azero = params.get_Azero_lower() + params.get_Azero_incr() * posn.at(4);

      } else {
         input.m0 = params.get_random_m0(generator);
         input.m12 = params.get_random_m12(generator);
         input.TanBeta = params.get_random_TanBeta(generator);
         input.SignLambdax = params.get_random_SignLambdax(generator);
         input.Azero = params.get_random_Azero(generator);
      }
   }

   inline void trim(std::string& str)
   {
      std::size_t startpos = str.find_first_not_of(" \t\n\v\f\r");
      if (startpos != std::string::npos) str.erase(0, startpos);
      std::size_t endpos = str.find_last_not_of(" \t\n\v\f\r");
      if (endpos != std::string::npos) str.erase(endpos+1);
   }

   CNE6SSM_scan_parameters parse_scan_inputs_file(const std::string& scan_input_file)
   {
      std::ifstream ifs(scan_input_file, std::ifstream::in);
      if (ifs.fail()) {
         throw ReadError("unable to open file " + scan_input_file);
      }
      
      double m0_lower = 0.;
      double m0_upper = 0.;
      int m0_npts = 1;
      double m12_lower = 0.;
      double m12_upper = 0.;
      int m12_npts = 1;
      double TanBeta_lower = 1.;
      double TanBeta_upper = 1.;
      int TanBeta_npts = 1;
      int SignLambdax_lower = 1;
      int SignLambdax_upper = 1;
      int SignLambdax_npts = 1;
      double Azero_lower = 0.;
      double Azero_upper = 0.;
      int Azero_npts = 1;
      int total_npts = 1;
      double output_scale = -1.;
      bool output_at_susy_scale = true;
      bool is_grid_scan = true;

      // read from file
      // # starts a (single line) comment
      // \n, ;, and , are delimiters
      std::string line;
      while (std::getline(ifs, line)) {
         if (line.find("#") != std::string::npos) {
            line = line.substr(0, line.find("#"));
         }
         if (line.empty())
            continue;

         // break up into individual fields
         std::vector<std::string> fields;
         boost::split(fields, line, boost::is_any_of(",;"));

         for (std::size_t i = 0; i < fields.size(); ++i) {
            // remove whitespace
            trim(fields[i]);

            // get field name and value
            if (!fields[i].empty()) {
               std::vector<std::string> field;
               boost::split(field, fields[i], boost::is_any_of("="));
               if (field.size() < 2) {
                  WARNING("Ignoring invalid input '" + fields[i] + "'");
               } else {
                  trim(field[0]);
                  trim(field[1]);

                  // compare against valid inputs
                  if (field[0] == "is_grid_scan") {
                     boost::to_lower(field[1]);
                     if (field[1] == "false" || field[1] == "f") {
                        is_grid_scan = false;
                     } else if (field[1] == "true" || field[1] == "t") {
                        is_grid_scan = true;
                     } else {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                     }
                  } else if (field[0] == "output_scale") {
                     if (field[1] == "susy_scale") {
                        output_scale = -1.;
                        output_at_susy_scale = true;
                     } else {
                        try {
                           output_scale = boost::lexical_cast<double>(field[1]);
                           if (output_scale > 0.)
                              output_at_susy_scale = false;
                        } catch (const boost::bad_lexical_cast& error) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           output_scale = -1.;
                           output_at_susy_scale = true;
                        }
                     }
                  } else if (field[0] == "m0_lower") {
                     try {
                        m0_lower = boost::lexical_cast<double>(field[1]);
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        m0_lower = 0.;
                     }
                  } else if (field[0] == "m0_upper") {
                     try {
                        m0_upper = boost::lexical_cast<double>(field[1]);
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        m0_upper = 0.;
                     }
                  } else if (field[0] == "m0_npts") {
                     try {
                        m0_npts = boost::lexical_cast<int>(field[1]);
                        if (!is_grid_scan) {
                           WARNING("Random scan requested, input '" + fields[i] + "' will be ignored");
                        } else if (m0_npts <= 0) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           m0_npts = 1;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        m0_npts = 1;
                     }
                  } else if (field[0] == "m12_lower") {
                     try {
                        m12_lower = boost::lexical_cast<double>(field[1]);
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        m12_lower = 0.;
                     }
                  } else if (field[0] == "m12_upper") {
                     try {
                        m12_upper = boost::lexical_cast<double>(field[1]);
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        m12_upper = 0.;
                     }
                  } else if (field[0] == "m12_npts") {
                     try {
                        m12_npts = boost::lexical_cast<int>(field[1]);
                        if (!is_grid_scan) {
                           WARNING("Random scan requested, input '" + fields[i] + "' will be ignored");
                        } else if (m12_npts <= 0) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           m12_npts = 1;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        m12_npts = 1;
                     }
                  } else if (field[0] == "TanBeta_lower") {
                     try {
                        TanBeta_lower = boost::lexical_cast<double>(field[1]);
                        if (TanBeta_lower < 1. || TanBeta_lower > 1000.) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           TanBeta_lower = 1.;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        TanBeta_lower = 1.;
                     }
                  } else if (field[0] == "TanBeta_upper") {
                     try {
                        TanBeta_upper = boost::lexical_cast<double>(field[1]);
                        if (TanBeta_upper < 1. || TanBeta_upper > 1000.) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           TanBeta_upper = 1.;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        TanBeta_upper = 1.;
                     }
                  } else if (field[0] == "TanBeta_npts") {
                     try {
                        TanBeta_npts = boost::lexical_cast<int>(field[1]);
                        if (!is_grid_scan) {
                           WARNING("Random scan requested, input '" + fields[i] + "' will be ignored");
                        } else if (TanBeta_npts <= 0) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           TanBeta_npts = 1;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        TanBeta_npts = 1;
                     }
                  } else if (field[0] == "SignLambdax_lower") {
                     try {
                        SignLambdax_lower = Sign(boost::lexical_cast<double>(field[1]));
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        SignLambdax_lower = -1;
                     }
                  } else if (field[0] == "SignLambdax_upper") {
                     try {
                        SignLambdax_upper = Sign(boost::lexical_cast<double>(field[1]));
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        SignLambdax_upper = 1;
                     }
                  } else if (field[0] == "SignLambdax_npts") {
                     try {
                        SignLambdax_npts = boost::lexical_cast<int>(field[1]);
                        if (!is_grid_scan) {
                           WARNING("Random scan requested, input '" + fields[i] + "' will be ignored");
                        } else if (SignLambdax_npts <= 0) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           SignLambdax_npts = 1;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        SignLambdax_npts = 1;
                     }
                  } else if (field[0] == "Azero_lower") {
                     try {
                        Azero_lower = boost::lexical_cast<double>(field[1]);
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        Azero_lower = 0.;
                     }
                  } else if (field[0] == "Azero_upper") {
                     try {
                        Azero_upper = boost::lexical_cast<double>(field[1]);
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        Azero_upper = 0.;
                     }
                  } else if (field[0] == "Azero_npts") {
                     try {
                        Azero_npts = boost::lexical_cast<int>(field[1]);
                        if (!is_grid_scan) {
                           WARNING("Random scan requested, input '" + fields[i] + "' will be ignored");
                        } else if (Azero_npts <= 0) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           Azero_npts = 1;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        Azero_npts = 1;
                     }
                  } else if (field[0] == "total_npts") {
                     try {
                        total_npts = boost::lexical_cast<int>(field[1]);
                        if (is_grid_scan) {
                           WARNING("Grid scan requested, input '" + fields[i] + "' will be ignored");
                        } else if (total_npts <= 0) {
                           WARNING("Ignoring invalid input '" + fields[i] + "'");
                           total_npts = 1;
                        }
                     } catch (const boost::bad_lexical_cast& error) {
                        WARNING("Ignoring invalid input '" + fields[i] + "'");
                        total_npts = 1;
                     }
                  } else {
                     WARNING("Ignoring invalid input '" + fields[i] + "'");
                  }
               } //< if (field.size() < 2)
            } //< if (!fields[i].empty())
         } //< for(std::size_t i = 0; i < fields.size(); ++i)
      } //< while(std::getline(ifs, line))

      // initialise scan parameters
      if (is_grid_scan) {
         if (output_at_susy_scale) {
            return CNE6SSM_scan_parameters(m0_lower, m0_upper, m0_npts,
                                           m12_lower, m12_upper, m12_npts,
                                           TanBeta_lower, TanBeta_upper, TanBeta_npts,
                                           SignLambdax_lower, SignLambdax_upper, SignLambdax_npts,
                                           Azero_lower, Azero_upper, Azero_npts);
         } else {
            return CNE6SSM_scan_parameters(m0_lower, m0_upper, m0_npts,
                                           m12_lower, m12_upper, m12_npts,
                                           TanBeta_lower, TanBeta_upper, TanBeta_npts,
                                           SignLambdax_lower, SignLambdax_upper, SignLambdax_npts,
                                           Azero_lower, Azero_upper, Azero_npts, output_scale);
         }
      } else {
         if (output_at_susy_scale) {
            return CNE6SSM_scan_parameters(m0_lower, m0_upper,
                                           m12_lower, m12_upper,
                                           TanBeta_lower, TanBeta_upper, 
                                           SignLambdax_lower, SignLambdax_upper,
                                           Azero_lower, Azero_upper, total_npts);
         } else {
            return CNE6SSM_scan_parameters(m0_lower, m0_upper,
                                           m12_lower, m12_upper,
                                           TanBeta_lower, TanBeta_upper, 
                                           SignLambdax_lower, SignLambdax_upper,
                                           Azero_lower, Azero_upper, total_npts, output_scale);
         }
      }
   }

   double get_soft_parameter_from_inputs(CNE6SSM<Two_scale> model, CNE6SSM_info::Parameters p, 
                                         double m0, double m12, double Azero, double output_scale)
   {
      const auto Ye = model.get_Ye();
      const auto Yd = model.get_Yd();
      const auto Yu = model.get_Yu();
      const auto KappaPr = model.get_KappaPr();
      const auto Sigmax = model.get_Sigmax();
      const auto hE = model.get_hE();
      const auto SigmaL = model.get_SigmaL();
      const auto gD = model.get_gD();
      const auto fu = model.get_fu();
      const auto fd = model.get_fd();
      const auto Kappa = model.get_Kappa();
      const auto Lambda12 = model.get_Lambda12();
      const auto Lambdax = model.get_Lambdax();
      
      model.set_TYe(Azero * Ye);
      model.set_TYd(Azero * Yd);
      model.set_TYu(Azero * Yu);
      model.set_TKappaPr(Azero * KappaPr);
      model.set_TSigmax(Azero * Sigmax);
      model.set_ThE(Azero * hE);
      model.set_TSigmaL(Azero * SigmaL);
      model.set_TgD(Azero * gD);
      model.set_Tfu(Azero * fu);
      model.set_Tfd(Azero * fd);
      model.set_TKappa(Azero * Kappa);
      model.set_TLambda12(Azero * Lambda12);
      model.set_TLambdax(Azero * Lambdax);
      model.set_mHd2(Sqr(m0));
      model.set_mHu2(Sqr(m0));
      model.set_ms2(Sqr(m0));
      model.set_msbar2(Sqr(m0));
      model.set_mphi2(Sqr(m0));
      model.set_mHp2(Sqr(m0));
      model.set_mHpbar2(Sqr(m0));
      model.set_mH1I2(Sqr(m0)*UNITMATRIX(2));
      model.set_mH2I2(Sqr(m0)*UNITMATRIX(2));
      model.set_mSI2(Sqr(m0)*UNITMATRIX(3));
      model.set_mq2(Sqr(m0)*UNITMATRIX(3));
      model.set_ml2(Sqr(m0)*UNITMATRIX(3));
      model.set_md2(Sqr(m0)*UNITMATRIX(3));
      model.set_mu2(Sqr(m0)*UNITMATRIX(3));
      model.set_me2(Sqr(m0)*UNITMATRIX(3));
      model.set_mDx2(Sqr(m0)*UNITMATRIX(3));
      model.set_mDxbar2(Sqr(m0)*UNITMATRIX(3));
      model.set_MassB(m12);
      model.set_MassWB(m12);
      model.set_MassG(m12);
      model.set_MassBp(m12);
      
      model.run_to(output_scale);
      
      return model.get_parameter(p);
   }

   double get_tree_level_Lambdax_soft_term(CNE6SSM<Two_scale> model, double mHd2_coeff, double mHu2_coeff)
   {
      const auto vd = model.get_vd();
      const auto vu = model.get_vu();
      const auto vs = model.get_vs();
      
      double coeff = 2.0 * (Sqr(vd) * mHd2_coeff - Sqr(vu) * mHu2_coeff) / (Sqr(vs) * (Sqr(vu) - Sqr(vd)));
      
      return coeff;
   }
   
   double get_tree_level_Lambdax_constant_term(CNE6SSM<Two_scale> model)
   {
      const auto g1 = model.get_g1();
      const auto g2 = model.get_g2();
      const auto g1p = model.get_g1p();
      const auto vd = model.get_vd();
      const auto vu = model.get_vu();
      const auto vs = model.get_vs();
      const auto vsb = model.get_vsb();
      const auto QS = model.get_input().QS;
      
      double coeff = - 0.25 * (Sqr(g2) + 0.6 * Sqr(g1)) * (Sqr(vd) + Sqr(vu)) / Sqr(vs)
         + 0.025 * Sqr(g1p) * (2.0 * Sqr(vu) - 3.0 * Sqr(vd)) * 
         (-3.0 * Sqr(vd) - 2.0 * Sqr(vu) + QS * (Sqr(vs) - Sqr(vsb))) / (Sqr(vs) * (Sqr(vu) - Sqr(vd)));
      
      return coeff;
   }

} // namespace flexiblesusy

double get_wall_time()
{
   struct timeval time;
   if (gettimeofday(&time,NULL)) {
      return 0;
   }
   return (double)time.tv_sec + (double)time.tv_usec*0.000001;
}

double get_cpu_time()
{
   return (double)clock() / CLOCKS_PER_SEC;
}

int main(int argc, const char* argv[])
{
   using namespace flexiblesusy;
   using namespace softsusy;
   typedef Two_scale algorithm_type;
   typedef std::chrono::duration<int,std::micro> microseconds_t;

   std::chrono::high_resolution_clock::time_point start_point = std::chrono::high_resolution_clock::now();
   double wall_start = get_wall_time();
   double cpu_start = get_cpu_time();

   std::uniform_real_distribution<double> distribution(0., 1.);

   Scan_command_line_options options(argc, argv);
   if (options.must_print_model_info())
      CNE6SSM_info::print(std::cout);
   if (options.must_exit())
      return options.status();

   const std::string scan_input_file(options.get_scan_input_file());
   const std::string pole_mass_output_file(options.get_pole_mass_output_file());
   const std::string drbar_mass_output_file(options.get_drbar_mass_output_file());
   const std::string drbar_susy_pars_output_file(options.get_drbar_susy_pars_output_file());
   const std::string drbar_soft_pars_output_file(options.get_drbar_soft_pars_output_file());
   const std::string drbar_mixings_output_file(options.get_drbar_mixings_output_file());

   CNE6SSM_scan_parameters parameters;

   if (scan_input_file.empty()) {
      WARNING("No scan input file given!\n"
              "   Default scan parameters will be used.\n"
              "   You can provide them via the option --scan-input-file=");
   } else {
      try {
         parameters = parse_scan_inputs_file(scan_input_file);
      } catch (const ReadError& error) {
         ERROR(error.what());
         return EXIT_FAILURE;
      }
   }

   // output streams
   bool must_write_pole_masses = false;

   std::ofstream pole_mass_out_stream;
   if (!pole_mass_output_file.empty()) {
      must_write_pole_masses = true;
      pole_mass_out_stream.open(pole_mass_output_file, std::ofstream::out);
   }

   std::ostream & pole_mass_out = pole_mass_output_file.empty() ? std::cout : pole_mass_out_stream;

   bool must_write_drbar_masses = false;
   bool must_write_drbar_susy_pars = false;
   bool must_write_drbar_soft_pars = false;
   bool must_write_drbar_mixings = false;

   std::ofstream drbar_mass_out_stream;
   std::ofstream drbar_susy_pars_out_stream;
   std::ofstream drbar_soft_pars_out_stream;
   std::ofstream drbar_mixings_out_stream;
   if (!drbar_mass_output_file.empty()) {
      must_write_drbar_masses = true;
      drbar_mass_out_stream.open(drbar_mass_output_file, std::ofstream::out);
   }
   if (!drbar_susy_pars_output_file.empty()) {
      must_write_drbar_susy_pars = true;
      drbar_susy_pars_out_stream.open(drbar_susy_pars_output_file, std::ofstream::out);
   }
   if (!drbar_soft_pars_output_file.empty()) {
      must_write_drbar_soft_pars = true;
      drbar_soft_pars_out_stream.open(drbar_soft_pars_output_file, std::ofstream::out);
   }
   if (!drbar_mixings_output_file.empty()) {
      must_write_drbar_mixings = true;
      drbar_mixings_out_stream.open(drbar_mixings_output_file, std::ofstream::out);
   }

   CNE6SSM_input_parameters input;
   set_default_parameter_values(input);

   // attempt to read scan input file

   QedQcd oneset;
   oneset.toMz();

   CNE6SSM_spectrum_generator<algorithm_type> spectrum_generator;
   spectrum_generator.set_precision_goal(1.0e-3);
   spectrum_generator.set_max_iterations(0);   // 0 == automatic
   spectrum_generator.set_calculate_sm_masses(0); // 0 == no
   // note: alternate ewsb flag is currently unused
   spectrum_generator.set_alternate_ewsb(1); // 1 == yes
   if (parameters.get_output_scale() <= 0.) {
      spectrum_generator.set_parameter_output_scale(0); // 0 == susy scale 
   } else {
      spectrum_generator.set_parameter_output_scale(parameters.get_output_scale());
   }

   std::vector<std::size_t> scan_dimensions = {parameters.get_m0_npts(), parameters.get_m12_npts(), 
                                               parameters.get_TanBeta_npts(), 
                                               parameters.get_SignLambdax_npts(), 
                                               parameters.get_Azero_npts()};

   Grid_scanner scan(scan_dimensions);

   CNE6SSM_pole_mass_writer pole_mass_writer;
   CNE6SSM_drbar_values_writer drbar_values_writer;
   bool must_write_comment_line = true;

   // print comment line to standard output for coefficients
   std::size_t width = 18;
   std::cout << "# "
             << std::left << std::setw(width) << "m0/GeV" << ' '
             << std::left << std::setw(width) << "m12/GeV" << ' '
             << std::left << std::setw(width) << "TanBeta" << ' '
             << std::left << std::setw(width) << "SignLambdax" << ' '
             << std::left << std::setw(width) << "Azero/GeV" << ' '
             << std::left << std::setw(width) << "ssumInput/GeV" << ' '
             << std::left << std::setw(width) << "QS" << ' '
             << std::left << std::setw(width) << "hEInput(0,0)" << ' '
             << std::left << std::setw(width) << "hEInput(1,0)" << ' '
             << std::left << std::setw(width) << "hEInput(2,0)" << ' '
             << std::left << std::setw(width) << "hEInput(0,1)" << ' '
             << std::left << std::setw(width) << "hEInput(1,1)" << ' '
             << std::left << std::setw(width) << "hEInput(2,1)" << ' '
             << std::left << std::setw(width) << "SigmaLInput" << ' '
             << std::left << std::setw(width) << "KappaPrInput" << ' '
             << std::left << std::setw(width) << "SigmaxInput" << ' '
             << std::left << std::setw(width) << "gDInput(0,0)" << ' '
             << std::left << std::setw(width) << "gDInput(1,0)" << ' '
             << std::left << std::setw(width) << "gDInput(2,0)" << ' '
             << std::left << std::setw(width) << "gDInput(0,1)" << ' '
             << std::left << std::setw(width) << "gDInput(1,1)" << ' '
             << std::left << std::setw(width) << "gDInput(2,1)" << ' '
             << std::left << std::setw(width) << "gDInput(0,2)" << ' '
             << std::left << std::setw(width) << "gDInput(1,2)" << ' '
             << std::left << std::setw(width) << "gDInput(2,2)" << ' '
             << std::left << std::setw(width) << "KappaInput(0,0)" << ' '
             << std::left << std::setw(width) << "KappaInput(1,0)" << ' '
             << std::left << std::setw(width) << "KappaInput(2,0)" << ' '
             << std::left << std::setw(width) << "KappaInput(0,1)" << ' '
             << std::left << std::setw(width) << "KappaInput(1,1)" << ' '
             << std::left << std::setw(width) << "KappaInput(2,1)" << ' '
             << std::left << std::setw(width) << "KappaInput(0,2)" << ' '
             << std::left << std::setw(width) << "KappaInput(1,2)" << ' '
             << std::left << std::setw(width) << "KappaInput(2,2)" << ' '
             << std::left << std::setw(width) << "Lambda12Input(0,0)" << ' '
             << std::left << std::setw(width) << "Lambda12Input(1,0)" << ' '
             << std::left << std::setw(width) << "Lambda12Input(0,1)" << ' '
             << std::left << std::setw(width) << "Lambda12Input(1,1)" << ' '
             << std::left << std::setw(width) << "fuInput(0,0)" << ' '
             << std::left << std::setw(width) << "fuInput(1,0)" << ' '
             << std::left << std::setw(width) << "fuInput(2,0)" << ' '
             << std::left << std::setw(width) << "fuInput(0,1)" << ' '
             << std::left << std::setw(width) << "fuInput(1,1)" << ' '
             << std::left << std::setw(width) << "fuInput(2,1)" << ' '
             << std::left << std::setw(width) << "fdInput(0,0)" << ' '
             << std::left << std::setw(width) << "fdInput(1,0)" << ' '
             << std::left << std::setw(width) << "fdInput(2,0)" << ' '
             << std::left << std::setw(width) << "fdInput(0,1)" << ' '
             << std::left << std::setw(width) << "fdInput(1,1)" << ' '
             << std::left << std::setw(width) << "fdInput(2,1)" << ' '
             << std::left << std::setw(width) << "MuPrInput/GeV" << ' '
             << std::left << std::setw(width) << "MuPhiInput/GeV" << ' '
             << std::left << std::setw(width) << "BMuPrInput/GeV^2" << ' '
             << std::left << std::setw(width) << "BMuPhiInput/GeV^2" << ' '
             << std::left << std::setw(width) << "Q/GeV" << ' '
             << std::left << std::setw(width) << "g1(Q)" << ' '
             << std::left << std::setw(width) << "g2(Q)" << ' '
             << std::left << std::setw(width) << "g3(Q)" << ' '
             << std::left << std::setw(width) << "g1p(Q)" << ' '
             << std::left << std::setw(width) << "Lambdax(Q)" << ' '
             << std::left << std::setw(width) << "vd(Q)/GeV" << ' '
             << std::left << std::setw(width) << "vu(Q)/GeV" << ' '
             << std::left << std::setw(width) << "vs(Q)/GeV" << ' '
             << std::left << std::setw(width) << "vsb(Q)/GeV" << ' '
             << std::left << std::setw(width) << "vphi(Q)/GeV" << ' '
             << std::left << std::setw(width) << "aHd(Q)" << ' '
             << std::left << std::setw(width) << "bHd(Q)" << ' '
             << std::left << std::setw(width) << "cHd(Q)" << ' '
             << std::left << std::setw(width) << "dHd(Q)" << ' '
             << std::left << std::setw(width) << "mHd2(Q)" << ' '
             << std::left << std::setw(width) << "mHd2Approx(Q)" << ' '
             << std::left << std::setw(width) << "aHu(Q)" << ' '
             << std::left << std::setw(width) << "bHu(Q)" << ' '
             << std::left << std::setw(width) << "cHu(Q)" << ' '
             << std::left << std::setw(width) << "dHu(Q)" << ' '
             << std::left << std::setw(width) << "mHu2(Q)" << ' '
             << std::left << std::setw(width) << "mHu2Approx(Q)" << ' '
             << std::left << std::setw(width) << "aS(Q)" << ' '
             << std::left << std::setw(width) << "bS(Q)" << ' '
             << std::left << std::setw(width) << "cS(Q)" << ' '
             << std::left << std::setw(width) << "dS(Q)" << ' '
             << std::left << std::setw(width) << "ms2(Q)" << ' '
             << std::left << std::setw(width) << "ms2Approx(Q)" << ' '
             << std::left << std::setw(width) << "aSb(Q)" << ' '
             << std::left << std::setw(width) << "bSb(Q)" << ' '
             << std::left << std::setw(width) << "cSb(Q)" << ' '
             << std::left << std::setw(width) << "dSb(Q)" << ' '
             << std::left << std::setw(width) << "msbar2(Q)" << ' '
             << std::left << std::setw(width) << "msbar2Approx(Q)" << ' '
             << std::left << std::setw(width) << "aphi(Q)" << ' '
             << std::left << std::setw(width) << "bphi(Q)" << ' '
             << std::left << std::setw(width) << "cphi(Q)" << ' '
             << std::left << std::setw(width) << "dphi(Q)" << ' '
             << std::left << std::setw(width) << "mphi2(Q)" << ' '
             << std::left << std::setw(width) << "mphi2Approx(Q)" << ' '
             << std::left << std::setw(width) << "aLambdax(Q)" << ' '
             << std::left << std::setw(width) << "bLambdax(Q)" << ' '
             << std::left << std::setw(width) << "cLambdax(Q)" << ' '
             << std::left << std::setw(width) << "dLambdax(Q)" << ' '
             << std::left << std::setw(width) << "lLambdax(Q)" << ' '
             << std::left << std::setw(width) << "Lambdax0lp(Q)" << ' '
             << std::left << std::setw(width) << "Lambdax0lpApprox(Q)" << ' '
             << std::left << std::setw(width) << "error" << '\n';
   
   while (!scan.has_finished()) {
      set_minpar_values(parameters, scan.get_position(), input);

      spectrum_generator.run(oneset, input);

      const CNE6SSM<algorithm_type>& model = spectrum_generator.get_model();

      // calculate coefficients
      const Problems<CNE6SSM_info::NUMBER_OF_PARTICLES>& problems
         = spectrum_generator.get_problems();

      const bool error = problems.have_serious_problem();

      double high_scale = spectrum_generator.get_high_scale();
      double susy_scale = spectrum_generator.get_susy_scale();

      double output_scale = susy_scale;
      if (parameters.get_output_scale() > 0.) {
         output_scale = parameters.get_output_scale();
      }

      double aHd = 0.;
      double bHd = 0.;
      double cHd = 0.;
      double dHd = 0.;
      double aHu = 0.;
      double bHu = 0.;
      double cHu = 0.;
      double dHu = 0.;
      double aS = 0.;
      double bS = 0.;
      double cS = 0.;
      double dS = 0.;
      double aSb = 0.;
      double bSb = 0.;
      double cSb = 0.;
      double dSb = 0.;
      double aphi = 0.;
      double bphi = 0.;
      double cphi = 0.;
      double dphi = 0.;
      double aLambdax = 0.;
      double bLambdax = 0.;
      double cLambdax = 0.;
      double dLambdax = 0.;
      double lLambdax = 0.;

      bool coeffs_error = false;

      double m0_centre = model.get_input().m0;
      double m12_centre = model.get_input().m12;
      double Azero_centre = model.get_input().Azero;

      double tree_level_Lambdax = 0.;

      if (!error) {

         CNE6SSM<algorithm_type> running_model(model);

         running_model.run_to(high_scale);

         const std::size_t num_terms = 4;

         Eigen::Matrix<double, num_terms, num_terms> scalar_input_values;
         Eigen::VectorXd mHu2_values(num_terms);
         Eigen::VectorXd mHd2_values(num_terms);
         Eigen::VectorXd ms2_values(num_terms);
         Eigen::VectorXd msbar2_values(num_terms);
         Eigen::VectorXd mphi2_values(num_terms);

         std::normal_distribution<double> m0_distribution(m0_centre, 0.1 * m0_centre);
         std::normal_distribution<double> m12_distribution(m12_centre, 0.1 * m12_centre);
         std::normal_distribution<double> Azero_distribution(Azero_centre, 0.1 * Azero_centre);

         for (std::size_t i = 0; i < num_terms; ++i) {
            double m0_tmp = m0_distribution(generator);
            double m12_tmp = m12_distribution(generator);
            double Azero_tmp = Azero_distribution(generator);

            scalar_input_values(i, 0) = Sqr(m0_tmp);
            scalar_input_values(i, 1) = Sqr(m12_tmp);
            scalar_input_values(i, 2) = m12_tmp * Azero_tmp;
            scalar_input_values(i, 3) = Sqr(Azero_tmp);

            // get soft mass values
            mHu2_values(i) = get_soft_parameter_from_inputs(running_model, CNE6SSM_info::mHu2, m0_tmp, m12_tmp, Azero_tmp, output_scale);
            mHd2_values(i) = get_soft_parameter_from_inputs(running_model, CNE6SSM_info::mHd2, m0_tmp, m12_tmp, Azero_tmp, output_scale);
            ms2_values(i) = get_soft_parameter_from_inputs(running_model, CNE6SSM_info::ms2, m0_tmp, m12_tmp, Azero_tmp, output_scale);
            msbar2_values(i) = get_soft_parameter_from_inputs(running_model, CNE6SSM_info::msbar2, m0_tmp, m12_tmp, Azero_tmp, output_scale);
            mphi2_values(i) = get_soft_parameter_from_inputs(running_model, CNE6SSM_info::mphi2, m0_tmp, m12_tmp, Azero_tmp, output_scale);
         }

         // solve for the coefficients
         Eigen::FullPivHouseholderQR<Eigen::Matrix<double, num_terms, num_terms> > scalar_input_values_decomp(scalar_input_values);

         Eigen::VectorXd mHu2_coeffs = scalar_input_values_decomp.solve(mHu2_values);
         Eigen::VectorXd mHd2_coeffs = scalar_input_values_decomp.solve(mHd2_values);
         Eigen::VectorXd ms2_coeffs = scalar_input_values_decomp.solve(ms2_values);
         Eigen::VectorXd msbar2_coeffs = scalar_input_values_decomp.solve(msbar2_values);
         Eigen::VectorXd mphi2_coeffs = scalar_input_values_decomp.solve(mphi2_values);

         if (!(scalar_input_values * mHu2_coeffs).isApprox(mHu2_values)
             || !(scalar_input_values * mHd2_coeffs).isApprox(mHd2_values)
             || !(scalar_input_values * ms2_coeffs).isApprox(ms2_values)
             || !(scalar_input_values * msbar2_coeffs).isApprox(msbar2_values)
             || !(scalar_input_values * mphi2_coeffs).isApprox(mphi2_values)) {
            coeffs_error = true;
         }

         aHu = mHu2_coeffs(0);
         bHu = mHu2_coeffs(1);
         cHu = mHu2_coeffs(2);
         dHu = mHu2_coeffs(3);

         aHd = mHd2_coeffs(0);
         bHd = mHd2_coeffs(1);
         cHd = mHd2_coeffs(2);
         dHd = mHd2_coeffs(3);

         aS = ms2_coeffs(0);
         bS = ms2_coeffs(1);
         cS = ms2_coeffs(2);
         dS = ms2_coeffs(3);

         aSb = msbar2_coeffs(0);
         bSb = msbar2_coeffs(1);
         cSb = msbar2_coeffs(2);
         dSb = msbar2_coeffs(3);

         aphi = mphi2_coeffs(0);
         bphi = mphi2_coeffs(1);
         cphi = mphi2_coeffs(2);
         dphi = mphi2_coeffs(3);

         aLambdax = get_tree_level_Lambdax_soft_term(model, aHd, aHu);
         bLambdax = get_tree_level_Lambdax_soft_term(model, bHd, bHu);
         cLambdax = get_tree_level_Lambdax_soft_term(model, cHd, cHu);
         dLambdax = get_tree_level_Lambdax_soft_term(model, dHd, dHu);
         lLambdax = get_tree_level_Lambdax_constant_term(model);

         running_model.run_to(susy_scale);
         running_model.solve_ewsb_tree_level();

         if (parameters.get_output_scale() > 0.) {
            running_model.run_to(output_scale);
         }

         tree_level_Lambdax = running_model.get_Lambdax();
      }

      if (must_write_pole_masses)
         pole_mass_writer.extract_pole_masses(model);
      if (must_write_drbar_masses)
         drbar_values_writer.extract_drbar_masses(model);
      if (must_write_drbar_susy_pars)
         drbar_values_writer.extract_drbar_susy_pars(model);
      if (must_write_drbar_soft_pars)
         drbar_values_writer.extract_drbar_soft_pars(model);
      if (must_write_drbar_mixings)
         drbar_values_writer.extract_drbar_mixings(model);

      if (must_write_comment_line) {
         if (must_write_pole_masses)
            pole_mass_writer.write_pole_masses_comment_line(pole_mass_out);
         if (must_write_drbar_masses)
            drbar_values_writer.write_drbar_masses_comment_line(drbar_mass_out_stream);
         if (must_write_drbar_susy_pars)
            drbar_values_writer.write_drbar_susy_pars_comment_line(drbar_susy_pars_out_stream);
         if (must_write_drbar_soft_pars)
            drbar_values_writer.write_drbar_soft_pars_comment_line(drbar_soft_pars_out_stream);
         if (must_write_drbar_mixings)
            drbar_values_writer.write_drbar_mixings_comment_line(drbar_mixings_out_stream);

         must_write_comment_line = false;
      }
      if (must_write_pole_masses)
         pole_mass_writer.write_pole_masses_line(pole_mass_out);

      if (must_write_drbar_masses)
         drbar_values_writer.write_drbar_masses_line(drbar_mass_out_stream);
      if (must_write_drbar_susy_pars)
         drbar_values_writer.write_drbar_susy_pars_line(drbar_susy_pars_out_stream);
      if (must_write_drbar_soft_pars)
         drbar_values_writer.write_drbar_soft_pars_line(drbar_soft_pars_out_stream);
      if (must_write_drbar_mixings)
         drbar_values_writer.write_drbar_mixings_line(drbar_mixings_out_stream);

      // write output
      std::cout << "  "
                << std::left << std::setw(width) << input.m0 << ' '
                << std::left << std::setw(width) << input.m12 << ' '
                << std::left << std::setw(width) << input.TanBeta << ' '
                << std::left << std::setw(width) << input.SignLambdax << ' '
                << std::left << std::setw(width) << input.Azero << ' '
                << std::left << std::setw(width) << input.ssumInput << ' '
                << std::left << std::setw(width) << input.QS << ' '
                << std::left << std::setw(width) << input.hEInput(0,0) << ' '
                << std::left << std::setw(width) << input.hEInput(1,0) << ' '
                << std::left << std::setw(width) << input.hEInput(2,0) << ' '
                << std::left << std::setw(width) << input.hEInput(0,1) << ' '
                << std::left << std::setw(width) << input.hEInput(1,1) << ' '
                << std::left << std::setw(width) << input.hEInput(2,1) << ' '
                << std::left << std::setw(width) << input.SigmaLInput << ' '
                << std::left << std::setw(width) << input.KappaPrInput << ' '
                << std::left << std::setw(width) << input.SigmaxInput << ' '
                << std::left << std::setw(width) << input.gDInput(0,0) << ' '
                << std::left << std::setw(width) << input.gDInput(1,0) << ' '
                << std::left << std::setw(width) << input.gDInput(2,0) << ' '
                << std::left << std::setw(width) << input.gDInput(0,1) << ' '
                << std::left << std::setw(width) << input.gDInput(1,1) << ' '
                << std::left << std::setw(width) << input.gDInput(2,1) << ' '
                << std::left << std::setw(width) << input.gDInput(0,2) << ' '
                << std::left << std::setw(width) << input.gDInput(1,2) << ' '
                << std::left << std::setw(width) << input.gDInput(2,2) << ' '
                << std::left << std::setw(width) << input.KappaInput(0,0) << ' '
                << std::left << std::setw(width) << input.KappaInput(1,0) << ' '
                << std::left << std::setw(width) << input.KappaInput(2,0) << ' '
                << std::left << std::setw(width) << input.KappaInput(0,1) << ' '
                << std::left << std::setw(width) << input.KappaInput(1,1) << ' '
                << std::left << std::setw(width) << input.KappaInput(2,1) << ' '
                << std::left << std::setw(width) << input.KappaInput(0,2) << ' '
                << std::left << std::setw(width) << input.KappaInput(1,2) << ' '
                << std::left << std::setw(width) << input.KappaInput(2,2) << ' '
                << std::left << std::setw(width) << input.Lambda12Input(0,0) << ' '
                << std::left << std::setw(width) << input.Lambda12Input(1,0) << ' '
                << std::left << std::setw(width) << input.Lambda12Input(0,1) << ' '
                << std::left << std::setw(width) << input.Lambda12Input(1,1) << ' '
                << std::left << std::setw(width) << input.fuInput(0,0) << ' '
                << std::left << std::setw(width) << input.fuInput(1,0) << ' '
                << std::left << std::setw(width) << input.fuInput(2,0) << ' '
                << std::left << std::setw(width) << input.fuInput(0,1) << ' '
                << std::left << std::setw(width) << input.fuInput(1,1) << ' '
                << std::left << std::setw(width) << input.fuInput(2,1) << ' '
                << std::left << std::setw(width) << input.fdInput(0,0) << ' '
                << std::left << std::setw(width) << input.fdInput(1,0) << ' '
                << std::left << std::setw(width) << input.fdInput(2,0) << ' '
                << std::left << std::setw(width) << input.fdInput(0,1) << ' '
                << std::left << std::setw(width) << input.fdInput(1,1) << ' '
                << std::left << std::setw(width) << input.fdInput(2,1) << ' '
                << std::left << std::setw(width) << input.MuPrInput << ' '
                << std::left << std::setw(width) << input.MuPhiInput << ' '
                << std::left << std::setw(width) << input.BMuPrInput << ' '
                << std::left << std::setw(width) << input.BMuPhiInput << ' '
                << std::left << std::setw(width) << output_scale << ' '
                << std::left << std::setw(width) << model.get_g1() << ' '
                << std::left << std::setw(width) << model.get_g2() << ' '
                << std::left << std::setw(width) << model.get_g3() << ' '
                << std::left << std::setw(width) << model.get_g1p() << ' '
                << std::left << std::setw(width) << model.get_Lambdax() << ' '
                << std::left << std::setw(width) << model.get_vd() << ' '
                << std::left << std::setw(width) << model.get_vu() << ' '
                << std::left << std::setw(width) << model.get_vs() << ' '
                << std::left << std::setw(width) << model.get_vsb() << ' '
                << std::left << std::setw(width) << model.get_vphi() << ' '
                << std::left << std::setw(width) << aHd << ' '
                << std::left << std::setw(width) << bHd << ' '
                << std::left << std::setw(width) << cHd << ' '
                << std::left << std::setw(width) << dHd << ' '
                << std::left << std::setw(width) << model.get_mHd2() << ' '
                << std::left << std::setw(width) << aHd * Sqr(m0_centre) + bHd * Sqr(m12_centre) 
         + cHd * m12_centre * Azero_centre + dHd * Sqr(Azero_centre) << ' '
                << std::left << std::setw(width) << aHu << ' '
                << std::left << std::setw(width) << bHu << ' '
                << std::left << std::setw(width) << cHu << ' '
                << std::left << std::setw(width) << dHu << ' '
                << std::left << std::setw(width) << model.get_mHu2() << ' '
                << std::left << std::setw(width) << aHu * Sqr(m0_centre) + bHu * Sqr(m12_centre) 
         + cHu * m12_centre * Azero_centre + dHu * Sqr(Azero_centre) << ' '
                << std::left << std::setw(width) << aS << ' '
                << std::left << std::setw(width) << bS << ' '
                << std::left << std::setw(width) << cS << ' '
                << std::left << std::setw(width) << dS << ' '
                << std::left << std::setw(width) << model.get_ms2() << ' '
                << std::left << std::setw(width) << aS * Sqr(m0_centre) + bS * Sqr(m12_centre) 
         + cS * m12_centre * Azero_centre + dS * Sqr(Azero_centre) << ' '
                << std::left << std::setw(width) << aSb << ' '
                << std::left << std::setw(width) << bSb << ' '
                << std::left << std::setw(width) << cSb << ' '
                << std::left << std::setw(width) << dSb << ' '
                << std::left << std::setw(width) << model.get_msbar2() << ' '
                << std::left << std::setw(width) << aSb * Sqr(m0_centre) + bSb * Sqr(m12_centre) 
         + cSb * m12_centre * Azero_centre + dSb * Sqr(Azero_centre) << ' '
                << std::left << std::setw(width) << aphi << ' '
                << std::left << std::setw(width) << bphi << ' '
                << std::left << std::setw(width) << cphi << ' '
                << std::left << std::setw(width) << dphi << ' '
                << std::left << std::setw(width) << model.get_mphi2() << ' '
                << std::left << std::setw(width) << aphi * Sqr(m0_centre) + bphi * Sqr(m12_centre) 
         + cphi * m12_centre * Azero_centre + dphi * Sqr(Azero_centre) << ' '
                << std::left << std::setw(width) << aLambdax << ' '
                << std::left << std::setw(width) << bLambdax << ' '
                << std::left << std::setw(width) << cLambdax << ' '
                << std::left << std::setw(width) << dLambdax << ' '
                << std::left << std::setw(width) << lLambdax << ' '
                << std::left << std::setw(width) << tree_level_Lambdax << ' '
                << std::left << std::setw(width) << input.SignLambdax * 
         Sqrt(aLambdax * Sqr(m0_centre) + bLambdax * Sqr(m12_centre) 
              + cLambdax * m12_centre * Azero_centre + dLambdax * Sqr(Azero_centre) + lLambdax) << ' '
                << std::left << std::setw(width) << (error || coeffs_error);

      if (error || coeffs_error) {
         if (error && !coeffs_error) {
            std::cout << "\t# " << problems;
         } else if (!error && coeffs_error) {
            std::cout << "\t# coefficients inaccurate";
         } else {
            std::cout << "\t# coefficients inaccurate, " << problems;
         }
      }

      std::cout << '\n';

      scan.step_forward();
   }

   if (pole_mass_out_stream.is_open())
      pole_mass_out_stream.close();

   if (drbar_mass_out_stream.is_open())
      drbar_mass_out_stream.close();

   if (drbar_susy_pars_out_stream.is_open())
      drbar_susy_pars_out_stream.close();

   if (drbar_soft_pars_out_stream.is_open())
      drbar_soft_pars_out_stream.close();

   if (drbar_mixings_out_stream.is_open())
      drbar_mixings_out_stream.close();

   std::chrono::high_resolution_clock::time_point end_point = std::chrono::high_resolution_clock::now();
   microseconds_t duration(std::chrono::duration_cast<microseconds_t>(end_point - start_point));
   double time_in_seconds = duration.count() * 0.000001;
   double wall_end = get_wall_time();
   double cpu_end = get_cpu_time();

   cout << "# Scan completed in " << time_in_seconds << " seconds\n";
   cout << "# Wall time = " << wall_end - wall_start << " seconds\n";
   cout << "# CPU time  = " << cpu_end - cpu_start << " seconds\n";

   return 0;
}
