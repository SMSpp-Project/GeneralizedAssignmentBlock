/*--------------------------------------------------------------------------*/
/*---------------- File RepairGeneralizedAssignmentSolver.h ----------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Header file for the class RepairGeneralizedAssignmentSolver, a heuristic
 * Solver for the GeneralizedAssignmentBlock that turns the point the
 * Variable hold (say, the solution of a Lagrangian relaxation, which may
 * violate the assignment constraints) into a feasible assignment, by a
 * greedy construction guided by that point followed by a local search with
 * shift and swap moves.
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*----------------------------- DEFINITIONS --------------------------------*/
/*--------------------------------------------------------------------------*/

#ifndef __RepairGeneralizedAssignmentSolver
 #define __RepairGeneralizedAssignmentSolver
                      /* self-identification: #endif at the end of the file */

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "CDASolver.h"

#include "GeneralizedAssignmentBlock.h"

#include <chrono>

/*--------------------------------------------------------------------------*/
/*------------------------------ NAMESPACE ---------------------------------*/
/*--------------------------------------------------------------------------*/

/// namespace for the Structured Modeling System++ (SMS++)
namespace SMSpp_di_unipi_it
{

/*--------------------------------------------------------------------------*/
/*-------------------------------- CLASSES ---------------------------------*/
/*--------------------------------------------------------------------------*/
/** @defgroup RepairGeneralizedAssignmentSolver_CLASSES Classes in \
 *            RepairGeneralizedAssignmentSolver.h
 *  @{ */

/*--------------------------------------------------------------------------*/
/*----------------- CLASS RepairGeneralizedAssignmentSolver ----------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------- GENERAL NOTES --------------------------------*/
/*--------------------------------------------------------------------------*/
/// a repair heuristic for the GeneralizedAssignmentBlock
/** The RepairGeneralizedAssignmentSolver class implements a heuristic
 * Solver for the Generalized Assignment Problem represented by a
 * GeneralizedAssignmentBlock [see GeneralizedAssignmentBlock.h]. It reads
 * the values \f$ \bar{x}_{ij} \in [ 0 , 1 ] \f$ that the Variable of the
 * sub-Block hold, which are typically the solution of the Lagrangian
 * relaxation of the assignment constraints (each job then being assigned to
 * zero, one or more agents) or a convex combination of such solutions, and
 * uses them as the guide of a feasible assignment \f$ \sigma \f$ of the jobs
 * to the agents, built in three phases:
 *
 * - construction: each job j is given the key
 *   \f$ k_{ij} = C_{ij} - \Lambda_j \bar{x}_{ij} \f$ for each agent i it
 *   can go to, with \f$ \Lambda_j \f$ one more than the range of the
 *   \f$ C_{ij} \f$ over these agents, so that a larger \f$ \bar{x}_{ij} \f$
 *   comes before any difference of cost, and the cost decides among the
 *   agents with the same \f$ \bar{x}_{ij} \f$; the jobs are taken in order
 *   of nonincreasing regret, i.e., the difference between their second best
 *   and their best key, and each goes to the agent of smallest key that has
 *   room for it, or, if none has, to the one whose capacity it exceeds
 *   least;
 *
 * - repair: a local search with best improvement over the shift moves (a
 *   job to another agent) and, when none improves, over the swap moves (two
 *   jobs of different agents exchange their agent), where a move improves
 *   if it decreases the total excess
 *   \f$ E( \sigma ) = \sum_i \max\{ 0 , \sum_{ j : \sigma( j ) = i } A_{ij}
 *   - B_i \} \f$, or if it leaves it unchanged and decreases the cost
 *   \f$ \sum_j C_{\sigma( j ) j} \f$; it ends in a feasible assignment
 *   unless it gets stuck in one with \f$ E( \sigma ) > 0 \f$;
 *
 * - improvement: intNRounds rounds of the same local search on the
 *   penalized cost \f$ \sum_j C_{\sigma( j ) j} + \sum_i \alpha_i \max\{ 0 ,
 *   \sum_{ j : \sigma( j ) = i } A_{ij} - B_i \} \f$, which lets the search
 *   cross the infeasible assignments; the weights start from the ratio
 *   between the total cost and the total resource of the instance, and
 *   after each round those of the agents over their capacity are multiplied
 *   by dblPenaltyIncr if the assignment is infeasible, while all of them are
 *   multiplied by dblPenaltyDecr if it is feasible, as in
 *
 *     M. Yagiura, T. Ibaraki, F. Glover "An ejection chain approach for the
 *     generalized assignment problem" INFORMS Journal on Computing 16(2),
 *     133-151, 2004
 *
 * The best feasible assignment met along the way, after any move, is the
 * solution. The Variable that are fixed (in the sub-Block, or as
 * ColVariable) are respected: a job whose Variable of agent i is fixed to 1
 * stays with agent i, and a job never goes to an agent whose Variable is
 * fixed to 0; if the fixings themselves cannot be part of a feasible
 * assignment (a job fixed to two agents or to none, or agents whose fixed
 * jobs exceed their capacity) compute() returns kInfeasible.
 *
 * The costs are those of the GeneralizedAssignmentBlock, which nobody
 * changes, while the Objective of the sub-Block may hold different ones (a
 * LagrangianDualSolver writes there the Lagrangian costs); the resources
 * and the capacities are those of the sub-Block, which hold the capacity
 * Constraint. compute() does not write into the Variable, which still hold
 * the guide afterwards; get_var_solution() writes the solution there.
 *
 * Being a heuristic, the Solver never proves optimality: when a feasible
 * assignment is found compute() returns kLowPrecision, or kStopIter /
 * kStopTime if intMaxIter moves or dblMaxTime seconds have been spent;
 * when none is found it returns kStopIter / kStopTime in the same cases,
 * and kError otherwise. get_ub() and get_var_value() are the cost of the
 * solution, get_lb() is - Inf. */

class RepairGeneralizedAssignmentSolver : public CDASolver
{

/*--------------------------------------------------------------------------*/
/*----------------------- PUBLIC PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 public:

/*--------------------------------------------------------------------------*/
/*---------------------------- PUBLIC TYPES --------------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Public Types
 *  @{ */

 using Index = Block::Index;

/*--------------------------------------------------------------------------*/
 /// public enum for the int algorithmic parameters
 /** Public enum describing the int algorithmic parameters that
  * RepairGeneralizedAssignmentSolver has in addition to those of Solver;
  * the value intLastRGASlvPar is provided so that the list can be extended
  * by derived classes. */

 enum int_par_type_RGASlv {
  intNRounds = intLastAlgPar ,  ///< rounds of the penalized local search
  intLastRGASlvPar  ///< first allowed new int parameter for derived classes
  };

/*--------------------------------------------------------------------------*/
 /// public enum for the double algorithmic parameters
 /** Public enum describing the double algorithmic parameters that
  * RepairGeneralizedAssignmentSolver has in addition to those of Solver;
  * the value dblLastRGASlvPar is provided so that the list can be extended
  * by derived classes. */

 enum dbl_par_type_RGASlv {
  dblPenaltyIncr = dblLastAlgPar ,  ///< increase of an excess weight
  dblPenaltyDecr ,                  ///< decrease of the excess weights
  dblLastRGASlvPar  ///< first allowed new dbl parameter for derived classes
  };

/** @} ---------------------------------------------------------------------*/
/*--------------------- CONSTRUCTOR AND DESTRUCTOR -------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Constructor and Destructor
 *  @{ */

 /// constructor: no Block, default parameters
 RepairGeneralizedAssignmentSolver( void ) : CDASolver() {}

/*--------------------------------------------------------------------------*/
 /// destructor
 ~RepairGeneralizedAssignmentSolver() override = default;

/** @} ---------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Other initializations
 *  @{ */

 /// set the (pointer to the) Block, which must be a GeneralizedAssignmentBlock
 void set_Block( Block * block ) override;

/*--------------------------------------------------------------------------*/

 using CDASolver::set_par;  // keep the other set_par() visible

 /// set the int parameters of RepairGeneralizedAssignmentSolver
 /** Set the int parameters of RepairGeneralizedAssignmentSolver:
  *
  * - intMaxIter [Inf]: the maximum number of moves of the local search in
  *   each compute(), the repair and the rounds together;
  *
  * - intLogVerb [0]: with a positive value and a log stream, compute()
  *   writes there the cost of the solution and the number of moves;
  *
  * - intNRounds [10]: the number of rounds of the local search on the
  *   penalized cost after the repair, 0 meaning that the solution is the
  *   one of the repair. */

 void set_par( idx_type par , int value ) override;

/*--------------------------------------------------------------------------*/
 /// set the double parameters of RepairGeneralizedAssignmentSolver
 /** Set the double parameters of RepairGeneralizedAssignmentSolver:
  *
  * - dblMaxTime [Inf]: the maximum time, in seconds, of each compute();
  *
  * - dblPenaltyIncr [1.3]: after a round ending in an infeasible
  *   assignment, the factor of the weight of each agent over its capacity;
  *
  * - dblPenaltyDecr [0.8]: after a round ending in a feasible assignment,
  *   the factor of the weight of every agent. */

 void set_par( idx_type par , double value ) override;

/*--------------------------------------------------------------------------*/

 [[nodiscard]] idx_type get_num_int_par( void ) const override {
  return( idx_type( intLastRGASlvPar ) );
  }

 [[nodiscard]] idx_type get_num_dbl_par( void ) const override {
  return( idx_type( dblLastRGASlvPar ) );
  }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

 [[nodiscard]] int get_dflt_int_par( idx_type par ) const override {
  if( par == intNRounds )
   return( 10 );
  return( CDASolver::get_dflt_int_par( par ) );
  }

 [[nodiscard]] double get_dflt_dbl_par( idx_type par ) const override {
  if( par == dblPenaltyIncr )
   return( 1.3 );
  if( par == dblPenaltyDecr )
   return( 0.8 );
  return( CDASolver::get_dflt_dbl_par( par ) );
  }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

 [[nodiscard]] int get_int_par( idx_type par ) const override;

 [[nodiscard]] double get_dbl_par( idx_type par ) const override;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

 [[nodiscard]] idx_type int_par_str2idx( const std::string & name )
  const override {
  if( name == "intNRounds" )
   return( intNRounds );
  return( CDASolver::int_par_str2idx( name ) );
  }

 [[nodiscard]] idx_type dbl_par_str2idx( const std::string & name )
  const override {
  if( name == "dblPenaltyIncr" )
   return( dblPenaltyIncr );
  if( name == "dblPenaltyDecr" )
   return( dblPenaltyDecr );
  return( CDASolver::dbl_par_str2idx( name ) );
  }

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

 [[nodiscard]] const std::string & int_par_idx2str( idx_type idx )
  const override {
  static const std::string _nr = "intNRounds";
  if( idx == intNRounds )
   return( _nr );
  return( CDASolver::int_par_idx2str( idx ) );
  }

 [[nodiscard]] const std::string & dbl_par_idx2str( idx_type idx )
  const override {
  static const std::string _pi = "dblPenaltyIncr";
  static const std::string _pd = "dblPenaltyDecr";
  if( idx == dblPenaltyIncr )
   return( _pi );
  if( idx == dblPenaltyDecr )
   return( _pd );
  return( CDASolver::dbl_par_idx2str( idx ) );
  }

/** @} ---------------------------------------------------------------------*/
/*--------------------- METHODS FOR SOLVING THE MODEL ----------------------*/
/*--------------------------------------------------------------------------*/
/** @name Solving the GeneralizedAssignmentBlock
 *  @{ */

 /// looks for a feasible assignment guided by the point of the Variable
 /** Runs the construction, the repair and the improvement described in the
  * general notes of the class, out of the current values and fixings of
  * the Variable, and returns what is said there. \p changedvars is ignored,
  * the data being read again at each call. */

 int compute( bool changedvars = true ) override;

/** @} ---------------------------------------------------------------------*/
/*---------------------- METHODS FOR READING RESULTS -----------------------*/
/*--------------------------------------------------------------------------*/
/** @name Accessing the found solutions (if any)
 *  @{ */

 /// no lower bound is known: - Inf
 [[nodiscard]] OFValue get_lb( void ) override {
  return( - Inf< OFValue >() );
  }

/*--------------------------------------------------------------------------*/
 /// the cost of the solution, + Inf if there is none
 [[nodiscard]] OFValue get_ub( void ) override { return( f_value ); }

/*--------------------------------------------------------------------------*/
 /// the cost of the solution, + Inf if there is none
 [[nodiscard]] OFValue get_var_value( void ) override { return( f_value ); }

/*--------------------------------------------------------------------------*/
 /// whether the last compute() has found a feasible assignment
 [[nodiscard]] bool has_var_solution( void ) override {
  return( f_has_sol );
  }

/*--------------------------------------------------------------------------*/
 /// whether the last compute() has found a feasible assignment
 [[nodiscard]] bool is_var_feasible( void ) override {
  return( f_has_sol );
  }

/*--------------------------------------------------------------------------*/
 /// writes the solution into the Variable of the sub-Block
 /** Writes the solution into the Variable of the sub-Block, x_{ij} = 1 if
  * job j is assigned to agent i and 0 otherwise; it is an error to call it
  * if has_var_solution() is false. */

 void get_var_solution( Configuration * solc = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// there is no dual solution
 [[nodiscard]] bool has_dual_solution( void ) override { return( false ); }

/*--------------------------------------------------------------------------*/
 /// there is no dual solution: nothing is done
 void get_dual_solution( Configuration * solc = nullptr ) override {}

/*--------------------------------------------------------------------------*/
 /// the number of moves of the last compute()
 [[nodiscard]] long get_elapsed_iterations( void ) const override {
  return( f_moves );
  }

/*--------------------------------------------------------------------------*/
 /// the time of the last compute(), in seconds
 [[nodiscard]] double get_elapsed_time( void ) const override {
  return( f_time );
  }

/** @} ---------------------------------------------------------------------*/
/*------------- METHODS FOR ADDING / REMOVING / CHANGING DATA --------------*/
/*--------------------------------------------------------------------------*/
/** @name Changing the data of the model
 *  @{ */

 /// the Modification are not needed: compute() reads the data each time
 void add_Modification( sp_Mod & mod ) override {}

/** @} ---------------------------------------------------------------------*/
/*-------------------- PROTECTED PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 protected:

/*--------------------------------------------------------------------------*/
/*--------------------------- PROTECTED METHODS ----------------------------*/
/*--------------------------------------------------------------------------*/

 /// reads the data, the guide and the fixings; false if these are infeasible
 bool read_instance( void );

/*--------------------------------------------------------------------------*/
 /// the greedy construction of the assignment out of the guide
 void construct( void );

/*--------------------------------------------------------------------------*/
 /// the local search, lexicographic if \p alpha is empty, penalized if not
 /** Moves the current assignment by the best improving shift move, or the
  * best improving swap move if no shift move improves, until none improves
  * or a limit is hit; with \p alpha empty a move improves if it decreases
  * the total excess, or it leaves it unchanged and decreases the cost, with
  * \p alpha nonempty if it decreases the cost plus the excess of each agent
  * i weighted by alpha[ i ]. Returns false if a limit has been hit. */

 bool local_search( const std::vector< double > & alpha );

/*--------------------------------------------------------------------------*/
 /// records the current assignment if it is feasible and better than the best
 void record( void );

/*--------------------------------------------------------------------------*/
 /// the excess of agent i if its load were \p load
 [[nodiscard]] double excess( Index i , double load ) const {
  return( load > v_B[ i ] + v_tol[ i ] ? load - v_B[ i ] : 0 );
  }

/*--------------------------------------------------------------------------*/
 /// whether job j may go to agent i
 [[nodiscard]] bool allowed( Index i , Index j ) const {
  return( v_fix[ i * f_N + j ] != 1 );
  }

/*--------------------------------------------------------------------------*/
 /// whether the load of every agent is within its capacity
 [[nodiscard]] bool feasible( void ) const {
  for( Index i = 0 ; i < f_M ; ++i )
   if( v_load[ i ] > v_B[ i ] + v_tol[ i ] )
    return( false );
  return( true );
  }

/*--------------------------------------------------------------------------*/
 /// whether a limit (moves or time) has been hit
 bool out_of_limits( void );

/*--------------------------------------------------------------------------*/
/*---------------------------- PROTECTED FIELDS ----------------------------*/
/*--------------------------------------------------------------------------*/

 GeneralizedAssignmentBlock * f_GAB = nullptr;  ///< the Block, as what it is

 int f_max_iter = Inf< int >();  ///< intMaxIter
 int f_log_verb = 0;             ///< intLogVerb
 int f_n_rounds = 10;            ///< intNRounds
 double f_max_time = Inf< double >();  ///< dblMaxTime
 double f_pen_incr = 1.3;        ///< dblPenaltyIncr
 double f_pen_decr = 0.8;        ///< dblPenaltyDecr

 Index f_M = 0;                  ///< the number of agents
 Index f_N = 0;                  ///< the number of jobs
 std::vector< double > v_C;      ///< the costs, agent-wise
 std::vector< double > v_A;      ///< the resources used, agent-wise
 std::vector< double > v_B;      ///< the capacities
 std::vector< double > v_tol;    ///< the tolerance on each capacity
 std::vector< double > v_guide;  ///< the guide, agent-wise
 std::vector< unsigned char > v_fix;
 ///< agent-wise: 0 = free, 1 = fixed to 0, 2 = fixed to 1
 std::vector< char > v_forced;   ///< 1 if job j is fixed to its agent

 std::vector< Index > v_asg;     ///< the current assignment
 std::vector< double > v_load;   ///< the current load of each agent
 double f_cost = 0;              ///< the cost of the current assignment

 std::vector< Index > v_sol;     ///< the best feasible assignment
 double f_value = Inf< double >();  ///< its cost, + Inf if there is none
 bool f_has_sol = false;         ///< whether there is one

 long f_moves = 0;               ///< the moves of the last compute()
 double f_time = 0;              ///< the time of the last compute()
 bool f_stop_iter = false;       ///< intMaxIter has stopped compute()
 bool f_stop_time = false;       ///< dblMaxTime has stopped compute()
 std::chrono::steady_clock::time_point f_start;  ///< start of compute()

/*--------------------------------------------------------------------------*/
/*--------------------- PRIVATE PART OF THE CLASS --------------------------*/
/*--------------------------------------------------------------------------*/

 private:

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE FIELDS ------------------------------*/
/*--------------------------------------------------------------------------*/

 SMSpp_insert_in_factory_h;  // insert it in the Solver factory

/*--------------------------------------------------------------------------*/

 };  // end( class( RepairGeneralizedAssignmentSolver ) )

/** @} end( group( RepairGeneralizedAssignmentSolver_CLASSES ) ) */

/*--------------------------------------------------------------------------*/

}  // end( namespace SMSpp_di_unipi_it )

#endif  /* RepairGeneralizedAssignmentSolver.h included */

/*--------------------------------------------------------------------------*/
/*-------------- End File RepairGeneralizedAssignmentSolver.h --------------*/
/*--------------------------------------------------------------------------*/
