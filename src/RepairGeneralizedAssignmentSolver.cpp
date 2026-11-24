/*--------------------------------------------------------------------------*/
/*--------------- File RepairGeneralizedAssignmentSolver.cpp ---------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Implementation of the RepairGeneralizedAssignmentSolver class.
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*----------------------------- IMPLEMENTATION -----------------------------*/
/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "RepairGeneralizedAssignmentSolver.h"

#include <algorithm>

#include <numeric>

/*--------------------------------------------------------------------------*/
/*------------------------- NAMESPACE AND USING ----------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*----------------------------- STATIC MEMBERS -----------------------------*/
/*--------------------------------------------------------------------------*/

// register RepairGeneralizedAssignmentSolver in the Solver factory

SMSpp_insert_in_factory_cpp_0( RepairGeneralizedAssignmentSolver );

/*--------------------------------------------------------------------------*/
/*------------------------------- CONSTANTS --------------------------------*/
/*--------------------------------------------------------------------------*/

// what a move has to improve by to be taken, and the relative tolerance on
// the capacities
static constexpr double kEps = 1e-9;

/*--------------------------------------------------------------------------*/
/*-------------- METHODS OF RepairGeneralizedAssignmentSolver --------------*/
/*--------------------------------------------------------------------------*/

void RepairGeneralizedAssignmentSolver::set_Block( Block * block )
{
 if( block == f_Block )
  return;

 GeneralizedAssignmentBlock * gab = nullptr;
 if( block ) {
  gab = dynamic_cast< GeneralizedAssignmentBlock * >( block );
  if( ! gab )
   throw( std::invalid_argument( "RepairGeneralizedAssignmentSolver::"
				 "set_Block: the Block is not a "
				 "GeneralizedAssignmentBlock" ) );
  }

 CDASolver::set_Block( block );
 f_GAB = gab;
 f_has_sol = false;
 f_value = Inf< double >();
 v_sol.clear();

 }  // end( RepairGeneralizedAssignmentSolver::set_Block )

/*--------------------------------------------------------------------------*/

void RepairGeneralizedAssignmentSolver::set_par( idx_type par , int value )
{
 switch( par ) {
  case( intMaxIter ): f_max_iter = value; break;
  case( intLogVerb ): f_log_verb = value; break;
  case( intNRounds ): f_n_rounds = std::max( value , 0 ); break;
  default:            CDASolver::set_par( par , value );
  }
 }

/*--------------------------------------------------------------------------*/

void RepairGeneralizedAssignmentSolver::set_par( idx_type par ,
						 double value )
{
 switch( par ) {
  case( dblMaxTime ):     f_max_time = value; break;
  case( dblPenaltyIncr ): f_pen_incr = value; break;
  case( dblPenaltyDecr ): f_pen_decr = value; break;
  default:                CDASolver::set_par( par , value );
  }
 }

/*--------------------------------------------------------------------------*/

int RepairGeneralizedAssignmentSolver::get_int_par( idx_type par ) const
{
 switch( par ) {
  case( intMaxIter ): return( f_max_iter );
  case( intLogVerb ): return( f_log_verb );
  case( intNRounds ): return( f_n_rounds );
  default:            return( CDASolver::get_int_par( par ) );
  }
 }

/*--------------------------------------------------------------------------*/

double RepairGeneralizedAssignmentSolver::get_dbl_par( idx_type par ) const
{
 switch( par ) {
  case( dblMaxTime ):     return( f_max_time );
  case( dblPenaltyIncr ): return( f_pen_incr );
  case( dblPenaltyDecr ): return( f_pen_decr );
  default:                return( CDASolver::get_dbl_par( par ) );
  }
 }

/*--------------------------------------------------------------------------*/

int RepairGeneralizedAssignmentSolver::compute( bool changedvars )
{
 if( ! f_GAB )
  throw( std::logic_error( "RepairGeneralizedAssignmentSolver::compute: "
			   "no Block" ) );

 f_start = std::chrono::steady_clock::now();
 f_moves = 0;
 f_stop_iter = f_stop_time = false;
 f_has_sol = false;
 f_value = Inf< double >();
 v_sol.clear();

 auto elapsed = [ this ]() {
  return( std::chrono::duration< double >(
	    std::chrono::steady_clock::now() - f_start ).count() );
  };

 if( ! read_instance() ) {
  f_time = elapsed();
  if( f_log && ( f_log_verb > 0 ) )
   *f_log << "RepairGeneralizedAssignmentSolver: the fixings are infeasible"
	  << std::endl;
  return( kInfeasible );
  }

 construct();
 record();

 // the repair, then the rounds on the penalized cost, whose weights start
 // from the cost of a unit of resource in the instance
 if( local_search( {} ) && ( f_n_rounds > 0 ) && ( f_M > 1 ) ) {
  double sc = 0;
  double sa = 0;
  for( Index k = 0 ; k < f_M * f_N ; ++k )
   if( v_fix[ k ] != 1 ) {
    sc += std::abs( v_C[ k ] );
    sa += std::abs( v_A[ k ] );
    }
  const double a0 = ( sa > 0 ) && ( sc > 0 ) ? sc / sa : 1;

  std::vector< double > alpha( f_M , a0 );
  for( int r = 0 ; r < f_n_rounds ; ++r ) {
   if( ! local_search( alpha ) )
    break;

   if( feasible() )
    for( auto & a : alpha )
     a *= f_pen_decr;
   else
    for( Index i = 0 ; i < f_M ; ++i )
     if( v_load[ i ] > v_B[ i ] + v_tol[ i ] )
      alpha[ i ] *= f_pen_incr;
   }
  }

 f_time = elapsed();

 if( f_log && ( f_log_verb > 0 ) ) {
  *f_log << "RepairGeneralizedAssignmentSolver: ";
  if( f_has_sol )
   *f_log << "cost = " << f_value;
  else
   *f_log << "no feasible assignment";
  *f_log << ", " << f_moves << " moves, " << f_time << " s" << std::endl;
  }

 if( f_stop_time )
  return( kStopTime );
 if( f_stop_iter )
  return( kStopIter );
 return( f_has_sol ? kLowPrecision : kError );

 }  // end( RepairGeneralizedAssignmentSolver::compute )

/*--------------------------------------------------------------------------*/

void RepairGeneralizedAssignmentSolver::get_var_solution(
						      Configuration * solc )
{
 if( ! f_has_sol )
  throw( std::logic_error( "RepairGeneralizedAssignmentSolver::"
			   "get_var_solution: no solution" ) );

 std::vector< double > x( f_N );
 for( Index i = 0 ; i < f_M ; ++i ) {
  for( Index j = 0 ; j < f_N ; ++j )
   x[ j ] = ( v_sol[ j ] == i ) ? 1 : 0;
  f_GAB->get_knapsack( i )->set_x( x.cbegin() );
  }
 }

/*--------------------------------------------------------------------------*/
/*--------------------------- PROTECTED METHODS ----------------------------*/
/*--------------------------------------------------------------------------*/

bool RepairGeneralizedAssignmentSolver::read_instance( void )
{
 f_M = f_GAB->get_NAgents();
 f_N = f_GAB->get_NJobs();

 v_C = f_GAB->get_Costs();
 v_A.resize( f_M * f_N );
 v_B.resize( f_M );
 v_tol.resize( f_M );
 v_guide.assign( f_M * f_N , 0 );
 v_fix.assign( f_M * f_N , 0 );

 for( Index i = 0 ; i < f_M ; ++i ) {
  const auto bkb = f_GAB->get_knapsack( i );
  if( bkb->get_NItems() != f_N )
   throw( std::logic_error( "RepairGeneralizedAssignmentSolver::"
			    "read_instance: a sub-Block does not have one "
			    "item for each job" ) );

  v_B[ i ] = bkb->get_Capacity();
  v_tol[ i ] = kEps * std::max( 1.0 , std::abs( v_B[ i ] ) );
  std::copy( bkb->get_Weights().begin() , bkb->get_Weights().end() ,
	     v_A.begin() + i * f_N );

  // the fixings are those of the sub-Block and those of the ColVariable,
  // which whoever fixes them without a Modification leaves there alone
  const bool hasvar = bkb->get_number_static_variables() > 0;
  const auto & fxd = bkb->get_fxd();
  for( Index j = 0 ; j < f_N ; ++j ) {
   const auto k = i * f_N + j;
   const double x = hasvar ? bkb->get_x( j ) : 0;
   unsigned char f = ( j < fxd.size() ) ? fxd[ j ] : 0;
   if( ( ! f ) && hasvar && bkb->get_Var( j )->is_fixed() )
    f = ( x >= 0.5 ) ? 2 : 1;
   v_fix[ k ] = f;
   v_guide[ k ] = f ? ( f == 2 ? 1 : 0 ) : std::clamp( x , 0.0 , 1.0 );
   }
  }

 // the jobs fixed to an agent, which must be one, and those that can go
 // nowhere
 v_asg.assign( f_N , Inf< Index >() );
 v_forced.assign( f_N , 0 );
 v_load.assign( f_M , 0 );
 for( Index j = 0 ; j < f_N ; ++j ) {
  Index nallowed = 0;
  for( Index i = 0 ; i < f_M ; ++i ) {
   const auto f = v_fix[ i * f_N + j ];
   if( f == 2 ) {
    if( v_forced[ j ] )
     return( false );
    v_forced[ j ] = 1;
    v_asg[ j ] = i;
    v_load[ i ] += v_A[ i * f_N + j ];
    }
   if( f != 1 )
    ++nallowed;
   }
  if( ! nallowed )
   return( false );
  }

 return( feasible() );

 }  // end( RepairGeneralizedAssignmentSolver::read_instance )

/*--------------------------------------------------------------------------*/

void RepairGeneralizedAssignmentSolver::construct( void )
{
 // the key of job j for agent i, which puts a larger guide before any
 // difference of cost
 std::vector< double > lambda( f_N , 0 );
 std::vector< double > regret( f_N , 0 );
 std::vector< Index > order;
 order.reserve( f_N );

 for( Index j = 0 ; j < f_N ; ++j ) {
  if( v_forced[ j ] )
   continue;

  double cmin = Inf< double >();
  double cmax = - Inf< double >();
  for( Index i = 0 ; i < f_M ; ++i )
   if( allowed( i , j ) ) {
    cmin = std::min( cmin , v_C[ i * f_N + j ] );
    cmax = std::max( cmax , v_C[ i * f_N + j ] );
    }
  lambda[ j ] = cmax - cmin + 1;

  double k1 = Inf< double >();
  double k2 = Inf< double >();
  for( Index i = 0 ; i < f_M ; ++i )
   if( allowed( i , j ) ) {
    const double k = v_C[ i * f_N + j ] - lambda[ j ] * v_guide[ i * f_N + j ];
    if( k < k1 ) {
     k2 = k1;
     k1 = k;
     }
    else
     k2 = std::min( k2 , k );
    }
  regret[ j ] = k2 - k1;  // + Inf with only one agent
  order.push_back( j );
  }

 std::stable_sort( order.begin() , order.end() ,
		   [ & regret ]( Index a , Index b ) {
		    return( regret[ a ] > regret[ b ] ); } );

 // each job to the agent of smallest key that has room for it, or to the
 // one whose capacity it exceeds least
 for( const auto j : order ) {
  Index best = Inf< Index >();
  double bkey = Inf< double >();
  Index least = Inf< Index >();
  double lex = Inf< double >();
  double lcost = Inf< double >();
  for( Index i = 0 ; i < f_M ; ++i ) {
   if( ! allowed( i , j ) )
    continue;
   const auto k = i * f_N + j;
   const double load = v_load[ i ] + v_A[ k ];
   if( load <= v_B[ i ] + v_tol[ i ] ) {
    const double key = v_C[ k ] - lambda[ j ] * v_guide[ k ];
    if( key < bkey ) {
     bkey = key;
     best = i;
     }
    }
   else
    if( best == Inf< Index >() ) {
     const double ex = excess( i , load ) - excess( i , v_load[ i ] );
     if( ( ex < lex - kEps ) ||
	 ( ( ex <= lex + kEps ) && ( v_C[ k ] < lcost ) ) ) {
      lex = ex;
      lcost = v_C[ k ];
      least = i;
      }
     }
   }

  const Index i = ( best != Inf< Index >() ) ? best : least;
  v_asg[ j ] = i;
  v_load[ i ] += v_A[ i * f_N + j ];
  }

 f_cost = 0;
 for( Index j = 0 ; j < f_N ; ++j )
  f_cost += v_C[ v_asg[ j ] * f_N + j ];

 }  // end( RepairGeneralizedAssignmentSolver::construct )

/*--------------------------------------------------------------------------*/

bool RepairGeneralizedAssignmentSolver::local_search(
				       const std::vector< double > & alpha )
{
 const bool lex = alpha.empty();

 // the change of the excess, and of the penalty, of agent i when its load
 // moves by delta
 auto d_ex = [ this ]( Index i , double delta ) {
  return( excess( i , v_load[ i ] + delta ) - excess( i , v_load[ i ] ) );
  };
 auto d_pen = [ & ]( Index i , double delta ) {
  return( lex ? 0 : alpha[ i ] * d_ex( i , delta ) );
  };

 for( ;; ) {
  if( out_of_limits() )
   return( false );

  // the best move: kind 0 = shift of job j to agent h, 1 = swap of jobs j
  // and h; lexicographic on ( be , bc ), on bc alone when penalized
  int kind = -1;
  Index mj = 0;
  Index mh = 0;
  double be = 0;
  double bc = lex ? 0 : - kEps;

  auto better = [ & ]( double de , double dc ) {
   if( ! lex )
    return( dc < bc );
   return( ( de < be - kEps ) ||
           ( ( de <= be + kEps ) && ( dc < bc - kEps ) ) );
   };

  for( Index j = 0 ; j < f_N ; ++j ) {
   if( v_forced[ j ] )
    continue;
   const Index i = v_asg[ j ];
   const double aij = v_A[ i * f_N + j ];
   const double cij = v_C[ i * f_N + j ];
   const double ei = lex ? d_ex( i , - aij ) : 0;
   const double pi = d_pen( i , - aij );
   for( Index h = 0 ; h < f_M ; ++h ) {
    if( ( h == i ) || ( ! allowed( h , j ) ) )
     continue;
    const double ahj = v_A[ h * f_N + j ];
    const double dc = v_C[ h * f_N + j ] - cij;
    const double de = lex ? ei + d_ex( h , ahj ) : 0;
    const double dv = lex ? dc : dc + pi + d_pen( h , ahj );
    if( better( de , dv ) ) {
     kind = 0;
     mj = j;
     mh = h;
     be = de;
     bc = dv;
     }
    }
   }

  if( kind < 0 )
   for( Index j = 0 ; j < f_N ; ++j ) {
    if( v_forced[ j ] )
     continue;
    const Index i = v_asg[ j ];
    for( Index l = j + 1 ; l < f_N ; ++l ) {
     const Index h = v_asg[ l ];
     if( v_forced[ l ] || ( h == i ) || ( ! allowed( h , j ) ) ||
	 ( ! allowed( i , l ) ) )
      continue;
     const double di = v_A[ i * f_N + l ] - v_A[ i * f_N + j ];
     const double dh = v_A[ h * f_N + j ] - v_A[ h * f_N + l ];
     const double dc = v_C[ h * f_N + j ] + v_C[ i * f_N + l ]
                       - v_C[ i * f_N + j ] - v_C[ h * f_N + l ];
     const double de = lex ? d_ex( i , di ) + d_ex( h , dh ) : 0;
     const double dv = lex ? dc : dc + d_pen( i , di ) + d_pen( h , dh );
     if( better( de , dv ) ) {
      kind = 1;
      mj = j;
      mh = l;
      be = de;
      bc = dv;
      }
     }
    }

  if( kind < 0 )  // a local optimum
   return( true );

  if( kind == 0 ) {
   const Index i = v_asg[ mj ];
   v_load[ i ] -= v_A[ i * f_N + mj ];
   v_load[ mh ] += v_A[ mh * f_N + mj ];
   f_cost += v_C[ mh * f_N + mj ] - v_C[ i * f_N + mj ];
   v_asg[ mj ] = mh;
   }
  else {
   const Index i = v_asg[ mj ];
   const Index h = v_asg[ mh ];
   v_load[ i ] += v_A[ i * f_N + mh ] - v_A[ i * f_N + mj ];
   v_load[ h ] += v_A[ h * f_N + mj ] - v_A[ h * f_N + mh ];
   f_cost += v_C[ h * f_N + mj ] + v_C[ i * f_N + mh ]
             - v_C[ i * f_N + mj ] - v_C[ h * f_N + mh ];
   v_asg[ mj ] = h;
   v_asg[ mh ] = i;
   }

  ++f_moves;
  record();
  }
 }  // end( RepairGeneralizedAssignmentSolver::local_search )

/*--------------------------------------------------------------------------*/

void RepairGeneralizedAssignmentSolver::record( void )
{
 if( f_has_sol &&
     ( f_cost >= f_value - kEps * std::max( 1.0 , std::abs( f_value ) ) ) )
  return;

 if( ! feasible() )
  return;

 // the cost is computed again, the one kept along the moves drifting
 f_cost = 0;
 for( Index j = 0 ; j < f_N ; ++j )
  f_cost += v_C[ v_asg[ j ] * f_N + j ];

 if( f_has_sol && ( f_cost >= f_value ) )
  return;

 v_sol = v_asg;
 f_value = f_cost;
 f_has_sol = true;

 }  // end( RepairGeneralizedAssignmentSolver::record )

/*--------------------------------------------------------------------------*/

bool RepairGeneralizedAssignmentSolver::out_of_limits( void )
{
 if( f_moves >= f_max_iter ) {
  f_stop_iter = true;
  return( true );
  }

 if( f_max_time < Inf< double >() ) {
  const std::chrono::duration< double > el =
   std::chrono::steady_clock::now() - f_start;
  if( el.count() >= f_max_time ) {
   f_stop_time = true;
   return( true );
   }
  }

 return( false );

 }  // end( RepairGeneralizedAssignmentSolver::out_of_limits )

/*--------------------------------------------------------------------------*/
/*------------ End File RepairGeneralizedAssignmentSolver.cpp --------------*/
/*--------------------------------------------------------------------------*/
