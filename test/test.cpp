/*--------------------------------------------------------------------------*/
/*------------------------------ File test.cpp -----------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Unit test of GeneralizedAssignmentBlock: constructs a
 * GeneralizedAssignmentBlock via the Block factory, checking that the module
 * links correctly and the class is registered, and then solves a small
 * instance with the RepairGeneralizedAssignmentSolver, constructed via the
 * Solver factory, free, with a job fixed to an agent and with fixings that
 * admit no feasible assignment.
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include <cmath>

#include <iostream>

#include "RepairGeneralizedAssignmentSolver.h"

/*--------------------------------------------------------------------------*/
/*-------------------------------- USING -----------------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*-------------------------------- FUNCTIONS -------------------------------*/
/*--------------------------------------------------------------------------*/
// whether the Variable of the GeneralizedAssignmentBlock hold a feasible
// assignment worth value

static bool check_solution( GeneralizedAssignmentBlock * gab , double value )
{
 const auto M = gab->get_NAgents();
 const auto N = gab->get_NJobs();
 double cost = 0;
 std::vector< double > load( M , 0 );
 for( Block::Index j = 0 ; j < N ; ++j ) {
  double sum = 0;
  for( Block::Index i = 0 ; i < M ; ++i ) {
   const double x = gab->get_x( i , j );
   sum += x;
   cost += gab->get_Cost( i , j ) * x;
   load[ i ] += gab->get_Weight( i , j ) * x;
   }
  if( std::abs( sum - 1 ) > 1e-9 )
   return( false );
  }

 for( Block::Index i = 0 ; i < M ; ++i )
  if( load[ i ] > gab->get_Capacity( i ) + 1e-9 )
   return( false );

 return( std::abs( cost - value ) <= 1e-9 );
 }

/*--------------------------------------------------------------------------*/
/*-------------------------------- main() ----------------------------------*/
/*--------------------------------------------------------------------------*/

int main( int argc , char ** argv )
{
 // construct a GeneralizedAssignmentBlock via the Block factory: this
 // checks that the class is registered and the library is linked in
 auto block = Block::new_Block( "GeneralizedAssignmentBlock" );
 auto gab = dynamic_cast< GeneralizedAssignmentBlock * >( block );
 if( ! gab ) {
  std::cerr << "no GeneralizedAssignmentBlock from the Block factory"
            << std::endl;
  delete block;
  return( 1 );
  }

 // 2 agents, 3 jobs of resource 1, capacity 2 each: each job costs 1 on
 // agent 0 and 2 on agent 1, and the optimum is worth 4
 gab->load( 2 , 3 , { 1 , 1 , 1 , 2 , 2 , 2 } , { 1 , 1 , 1 , 1 , 1 , 1 } ,
            { 2 , 2 } );
 gab->generate_abstract_variables();
 gab->generate_abstract_constraints();

 auto slvr = dynamic_cast< RepairGeneralizedAssignmentSolver * >(
	       Solver::new_Solver( "RepairGeneralizedAssignmentSolver" ) );
 if( ! slvr ) {
  std::cerr << "no RepairGeneralizedAssignmentSolver from the Solver factory"
            << std::endl;
  delete block;
  return( 1 );
  }
 gab->register_Solver( slvr );

 bool ok = true;

 // free: a feasible assignment worth 4
 auto rc = slvr->compute();
 if( ( rc != Solver::kLowPrecision ) || ( ! slvr->has_var_solution() ) ||
     ( slvr->get_var_value() != 4 ) ) {
  std::cerr << "free: status " << rc << ", value " << slvr->get_var_value()
            << std::endl;
  ok = false;
  }
 else {
  slvr->get_var_solution();
  if( ! check_solution( gab , 4 ) ) {
   std::cerr << "free: the solution is not a feasible one worth 4"
             << std::endl;
   ok = false;
   }
  }

 // job 0 fixed to agent 1: it stays there, and the optimum is still 4
 gab->get_knapsack( 1 )->fix_x( true , 0 );
 rc = slvr->compute();
 if( ( rc != Solver::kLowPrecision ) || ( slvr->get_var_value() != 4 ) ) {
  std::cerr << "fixed: status " << rc << ", value " << slvr->get_var_value()
            << std::endl;
  ok = false;
  }
 else {
  slvr->get_var_solution();
  if( ( gab->get_x( 1 , 0 ) != 1 ) || ( ! check_solution( gab , 4 ) ) ) {
   std::cerr << "fixed: the solution does not respect the fixing"
             << std::endl;
   ok = false;
   }
  }

 // job 0 fixed to both agents: infeasible
 gab->get_knapsack( 0 )->fix_x( true , 0 );
 rc = slvr->compute();
 if( ( rc != Solver::kInfeasible ) || slvr->has_var_solution() ) {
  std::cerr << "infeasible fixings: status " << rc << std::endl;
  ok = false;
  }

 gab->unregister_Solver( slvr , true );
 delete block;

 if( ! ok )
  return( 1 );

 std::cout << "GeneralizedAssignmentBlock: all tests passed" << std::endl;

 return( 0 );
 }

/*--------------------------------------------------------------------------*/
/*---------------------------- End File test.cpp ---------------------------*/
/*--------------------------------------------------------------------------*/
