/*--------------------------------------------------------------------------*/
/*------------------- File GeneralizedAssignmentBlock.h --------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Header file for the class GeneralizedAssignmentBlock, which implements the
 * Block concept [see Block.h] for the Generalized Assignment Problem, as a
 * set of BinaryKnapsackBlock linked by assignment constraints.
 *
 * \author Antonio Frangioni \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Antonio Frangioni, Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*----------------------------- DEFINITIONS --------------------------------*/
/*--------------------------------------------------------------------------*/

#ifndef __GeneralizedAssignmentBlock
 #define __GeneralizedAssignmentBlock
                      /* self-identification: #endif at the end of the file */

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "BinaryKnapsackBlock.h"

#include "FRowConstraint.h"

/*--------------------------------------------------------------------------*/
/*------------------------------ NAMESPACE ---------------------------------*/
/*--------------------------------------------------------------------------*/

/// namespace for the Structured Modeling System++ (SMS++)
namespace SMSpp_di_unipi_it
{
/*--------------------------------------------------------------------------*/
/*-------------------------------- CLASSES ---------------------------------*/
/*--------------------------------------------------------------------------*/
/** @defgroup GeneralizedAssignmentBlock_CLASSES Classes in \
 *            GeneralizedAssignmentBlock.h
 *  @{ */

/*--------------------------------------------------------------------------*/
/*-------------------- CLASS GeneralizedAssignmentBlock --------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------- GENERAL NOTES --------------------------------*/
/*--------------------------------------------------------------------------*/
/// a Block for the Generalized Assignment Problem
/** The GeneralizedAssignmentBlock class implements the Block concept [see
 * Block.h] for the Generalized Assignment Problem (GAP). It is defined on a
 * set of M agents and a set of N jobs; assigning job j to agent i costs
 * C[ i ][ j ] and uses A[ i ][ j ] of the resource of agent i, whose
 * capacity is B[ i ]. Each job is assigned to exactly one agent, the
 * resource used by each agent is at most its capacity, and the total cost is
 * minimized:
 * \f[
 *  \min \Bigl\{ \, \sum_{ i = 1 }^M \sum_{ j = 1 }^N C_{ij} x_{ij} \;:\;
 *  \sum_{ i = 1 }^M x_{ij} = 1 \;\; j = 1 , \ldots , N \,,\;
 *  \sum_{ j = 1 }^N A_{ij} x_{ij} \leq B_i \;\; i = 1 , \ldots , M \,,\;
 *  x \in \{ 0 , 1 \}^{ M \times N } \, \Bigr\} \;.
 * \f]
 *
 * The problem is represented by M sub-Block, one BinaryKnapsackBlock for
 * each agent, the i-th one having the N jobs as its items, in their order,
 * with the A[ i ][ j ] as weights, the C[ i ][ j ] as profits and B[ i ] as
 * capacity, and a minimization objective. Thus, the
 * GeneralizedAssignmentBlock itself only has the static Constraint linking
 * the sub-Block, i.e., the N "assignment" ones above, and no Variable and
 * Objective of its own: the objective is the sum of these of the
 * sub-Block. Relaxing the assignment Constraint, as a LagrangianDualSolver
 * does, leaves one knapsack for each agent, and the Lagrangian Dual gives
 * the bound of the Dantzig-Wolfe reformulation of the problem. */

class GeneralizedAssignmentBlock : public Block
{
/*--------------------------------------------------------------------------*/
/*----------------------- PUBLIC PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 public:

/*--------------------------------------------------------------------------*/
/*--------------------- CONSTRUCTOR AND DESTRUCTOR -------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Constructor and Destructor
 *  @{ */

 /// constructor of GeneralizedAssignmentBlock, taking a pointer to the father
 /** Constructor of GeneralizedAssignmentBlock. It accepts a pointer to the
  * father Block, which can be of any type, defaulting to nullptr so that
  * this can also be used as the void constructor required by the Block
  * factory. */

 explicit GeneralizedAssignmentBlock( Block * father = nullptr )
  : Block( father ) , f_M( 0 ) , f_N( 0 ) , AR( 0 ) {}

/*--------------------------------------------------------------------------*/
 /// destructor of GeneralizedAssignmentBlock: deletes the sub-Block

 ~GeneralizedAssignmentBlock() override { guts_of_destructor(); }

/**@} ----------------------------------------------------------------------*/
/*-------------------------- OTHER INITIALIZATIONS -------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Other initializations
 *  @{ */

 /// loads the Generalized Assignment instance from memory
 /** Loads the Generalized Assignment instance from memory, moving the data
  * from the parameters:
  *
  * - m           is the number of agents
  *
  * - n           is the number of jobs
  *
  * - Costs       is the vector of the costs, of size m * n, arranged
  *               agent-wise: C[ i ][ j ] is Costs[ i * n + j ]
  *
  * - Weights     is the vector of the resources used, of size m * n,
  *               arranged as Costs
  *
  * - Capacities  is the vector of the capacities of the agents, of size m
  *
  * Any previous instance, together with its sub-Block and its abstract
  * representation, is discarded. Like load( std::istream & ), if there is
  * any Solver attached to this GeneralizedAssignmentBlock then a
  * NBModification (the "nuclear option") is issued. */

 void load( Index m , Index n , std::vector< double > && Costs ,
            std::vector< double > && Weights ,
            std::vector< double > && Capacities );

/*--------------------------------------------------------------------------*/
 /// loads the GeneralizedAssignmentBlock out of an istream
 /** Loads the GeneralizedAssignmentBlock out of an istream, in the format of
  * one instance of the OR-Library:
  *
  * - the number m of agents and the number n of jobs
  *
  * - for each agent i = 0 , ... , m - 1, the n costs C[ i ][ j ]
  *
  * - for each agent i = 0 , ... , m - 1, the n resources A[ i ][ j ]
  *
  * - the m capacities B[ i ]
  *
  * the elements being separated by whitespaces; \p frmt is ignored. Any
  * previous instance is discarded, and if there is any Solver attached to
  * this GeneralizedAssignmentBlock then a NBModification is issued. */

 void load( std::istream & input , char frmt = 0 ) override;

/*--------------------------------------------------------------------------*/
 /// extends Block::deserialize( netCDF::NcGroup )
 /** Extends Block::deserialize( netCDF::NcGroup ) to the specific format of
  * a GeneralizedAssignmentBlock. Besides what is managed by
  * Block::deserialize(), the group must contain:
  *
  * - the dimension "NAgents", the number of agents;
  *
  * - the dimension "NJobs", the number of jobs;
  *
  * - the variable "Costs", of type double and indexed over "NAgents" and
  *   "NJobs", the costs of the assignments;
  *
  * - the variable "Weights", of type double and indexed over "NAgents" and
  *   "NJobs", the resources used by the assignments;
  *
  * - the variable "Capacities", of type double and indexed over "NAgents",
  *   the capacities of the agents.
  *
  * Any previous instance is discarded, and the NBModification is issued by
  * Block::deserialize(). */

 void deserialize( const netCDF::NcGroup & group ) override;

/*--------------------------------------------------------------------------*/
 /// generates the Variable of the sub-Block
 /** The GeneralizedAssignmentBlock has no Variable of its own: this
  * generates these of the sub-Block, which the linking Constraint use. */

 void generate_abstract_variables( Configuration * stvv = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// generates the linking Constraint, and these of the sub-Block
 /** Generates the N "assignment" Constraint, which link the sub-Block
  * (whose Variable are generated first, if they are not there yet), and
  * then the Constraint of the sub-Block. The group is named "assign". No
  * Modification is issued. */

 void generate_abstract_constraints( Configuration * stcc = nullptr )
  override;

/**@} ----------------------------------------------------------------------*/
/*------ Methods for reading the data of the GeneralizedAssignmentBlock ----*/
/*--------------------------------------------------------------------------*/
/** @name Methods for reading the data of the GeneralizedAssignmentBlock
 *  @{ */

 /// the sense of the Objective, which is to be minimized

 [[nodiscard]] int get_objective_sense( void ) const override final {
  return( Objective::eMin );
  }

/*--------------------------------------------------------------------------*/
 /// the number of agents

 [[nodiscard]] Index get_NAgents( void ) const { return( f_M ); }

/*--------------------------------------------------------------------------*/
 /// the number of jobs

 [[nodiscard]] Index get_NJobs( void ) const { return( f_N ); }

/*--------------------------------------------------------------------------*/
 /// the cost of assigning job j to agent i

 [[nodiscard]] double get_Cost( Index i , Index j ) const {
  check( i , j , "get_Cost" );
  return( v_C[ i * f_N + j ] );
  }

/*--------------------------------------------------------------------------*/
 /// the resource of agent i used by job j

 [[nodiscard]] double get_Weight( Index i , Index j ) const {
  check( i , j , "get_Weight" );
  return( v_A[ i * f_N + j ] );
  }

/*--------------------------------------------------------------------------*/
 /// the capacity of agent i

 [[nodiscard]] double get_Capacity( Index i ) const {
  check( i , 0 , "get_Capacity" );
  return( v_B[ i ] );
  }

/*--------------------------------------------------------------------------*/
 /// the vector of the costs, arranged agent-wise

 [[nodiscard]] const std::vector< double > & get_Costs( void ) const {
  return( v_C );
  }

/*--------------------------------------------------------------------------*/
 /// the vector of the resources used, arranged agent-wise

 [[nodiscard]] const std::vector< double > & get_Weights( void ) const {
  return( v_A );
  }

/*--------------------------------------------------------------------------*/
 /// the vector of the capacities

 [[nodiscard]] const std::vector< double > & get_Capacities( void ) const {
  return( v_B );
  }

/*--------------------------------------------------------------------------*/
 /// the sub-Block of agent i

 [[nodiscard]] BinaryKnapsackBlock * get_knapsack( Index i ) const {
  check( i , 0 , "get_knapsack" );
  return( static_cast< BinaryKnapsackBlock * >( v_Block[ i ] ) );
  }

/**@} ----------------------------------------------------------------------*/
/*-------------------------- Methods for R3 Blocks -------------------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for R3 Blocks
 *  @{ */

 /// gives an identical copy of the GeneralizedAssignmentBlock
 /** The only R3 Block is the copy, asked with \p r3bc == nullptr: a
  * GeneralizedAssignmentBlock with the same data, \p base if it is given
  * (which must be a GeneralizedAssignmentBlock) and a new one whose father
  * is \p father otherwise, whose sub-Block are the copies of these of this
  * one [see BinaryKnapsackBlock::get_R3_Block()]. */

 Block * get_R3_Block( Configuration * r3bc = nullptr ,
                       Block * base = nullptr ,
                       Block * father = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// writes the solution of a copy into the Variable of this one
 /** \p R3B must be a copy given by get_R3_Block(): the solution of each of
  * its sub-Block is written into the corresponding one of this [see
  * BinaryKnapsackBlock::map_back_solution()]. */

 void map_back_solution( Block * R3B , Configuration * r3bc = nullptr ,
                         Configuration * solc = nullptr ) override;

/*--------------------------------------------------------------------------*/
 /// writes the solution of this into the Variable of a copy
 /** The reverse of map_back_solution(). */

 void map_forward_solution( Block * R3B , Configuration * r3bc = nullptr ,
                            Configuration * solc = nullptr ) override;

/**@} ----------------------------------------------------------------------*/
/*----------------------- Methods for handling Solution --------------------*/
/*--------------------------------------------------------------------------*/
/** @name Methods for handling Solution
 *  @{ */

 /// the value of \f$ x_{ij} \f$, i.e., whether job j is assigned to agent i
 /** The value of \f$ x_{ij} \f$ in the solution of the sub-Block of agent i
  * (0 if no solution is there). */

 [[nodiscard]] double get_x( Index i , Index j ) const {
  check( i , j , "get_x" );
  return( get_knapsack( i )->get_x( j ) );
  }

/**@} ----------------------------------------------------------------------*/
/*-- METHODS FOR LOADING, PRINTING & SAVING THE GeneralizedAssignmentBlock -*/
/*--------------------------------------------------------------------------*/
/** @name Methods for loading, printing & saving the
 *        GeneralizedAssignmentBlock
 *  @{ */

 /// extends Block::serialize( netCDF::NcGroup )
 /** Extends Block::serialize( netCDF::NcGroup ) to the specific format of a
  * GeneralizedAssignmentBlock [see deserialize( netCDF::NcGroup )]. */

 void serialize( netCDF::NcGroup & group ) const override;

/*--------------------------------------------------------------------------*/
 /// prints the data of the GeneralizedAssignmentBlock on an ostream

 void print( std::ostream & output , char vlvl = 0 ) const override;

/** @} ---------------------------------------------------------------------*/
/*-------------------- PROTECTED PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 protected:

/*--------------------------------------------------------------------------*/
/*---------------------------- PROTECTED FIELDS ----------------------------*/
/*--------------------------------------------------------------------------*/

 Index f_M;                   ///< the number of agents
 Index f_N;                   ///< the number of jobs

 std::vector< double > v_C;   ///< the costs, agent-wise
 std::vector< double > v_A;   ///< the resources used, agent-wise
 std::vector< double > v_B;   ///< the capacities

 unsigned char AR;            ///< bit-wise coded: what abstract is there

 static constexpr unsigned char HasVar = 1;
 ///< first bit of AR == 1 if the Variable have been constructed

 static constexpr unsigned char HasCns = 2;
 ///< second bit of AR == 1 if the Constraint have been constructed

 /// the assignment Constraint: each job to exactly one agent
 std::vector< FRowConstraint > v_assign;

/*--------------------------------------------------------------------------*/
/*--------------------- PRIVATE PART OF THE CLASS --------------------------*/
/*--------------------------------------------------------------------------*/

 private:

/*--------------------------------------------------------------------------*/
/*-------------------------- PRIVATE METHODS -------------------------------*/
/*--------------------------------------------------------------------------*/

 /// throws if agent i or job j is not there

 void check( Index i , Index j , const char * who ) const {
  if( ( i >= f_M ) || ( j >= f_N ) )
   throw( std::invalid_argument( std::string(
		  "GeneralizedAssignmentBlock::" ) + who + ": invalid index" ) );
  }

/*--------------------------------------------------------------------------*/
 /// discards the instance, its sub-Block and its abstract representation

 void guts_of_destructor( void );

/*--------------------------------------------------------------------------*/
 /// checks the data, then builds the sub-Block

 void build( const std::string & prfx );

/*--------------------------------------------------------------------------*/
/*---------------------------- PRIVATE FIELDS ------------------------------*/
/*--------------------------------------------------------------------------*/

 SMSpp_insert_in_factory_h;  // insert it in the Block factory

/*--------------------------------------------------------------------------*/

 };  // end( class( GeneralizedAssignmentBlock ) )

/** @} end( group( GeneralizedAssignmentBlock_CLASSES ) ) */

/*--------------------------------------------------------------------------*/

}  // end( namespace SMSpp_di_unipi_it )

#endif  /* GeneralizedAssignmentBlock.h included */

/*--------------------------------------------------------------------------*/
/*----------------- End File GeneralizedAssignmentBlock.h ------------------*/
/*--------------------------------------------------------------------------*/
