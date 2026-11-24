/*--------------------------------------------------------------------------*/
/*------------------ File GeneralizedAssignmentBlock.cpp -------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Implementation of the GeneralizedAssignmentBlock class.
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

#include "GeneralizedAssignmentBlock.h"

#include "LinearFunction.h"

/*--------------------------------------------------------------------------*/
/*------------------------- NAMESPACE AND USING ----------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*----------------------------- STATIC MEMBERS -----------------------------*/
/*--------------------------------------------------------------------------*/

// register GeneralizedAssignmentBlock in the Block factory

SMSpp_insert_in_factory_cpp_1( GeneralizedAssignmentBlock );

/*--------------------------------------------------------------------------*/
/*---------------- METHODS OF GeneralizedAssignmentBlock -------------------*/
/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::load( Index m , Index n ,
				       std::vector< double > && Costs ,
				       std::vector< double > && Weights ,
				       std::vector< double > && Capacities )
{
 static const std::string _prfx = "GeneralizedAssignmentBlock::load: ";

 if( Costs.size() != m * n )
  throw( std::invalid_argument( _prfx + "Costs of the wrong size" ) );
 if( Weights.size() != m * n )
  throw( std::invalid_argument( _prfx + "Weights of the wrong size" ) );
 if( Capacities.size() != m )
  throw( std::invalid_argument( _prfx + "Capacities of the wrong size" ) );

 guts_of_destructor();  // discard the previous instance, if any

 f_M = m;
 f_N = n;
 v_C = std::move( Costs );
 v_A = std::move( Weights );
 v_B = std::move( Capacities );

 build( _prfx );

 if( anyone_there() )
  add_Modification( std::make_shared< NBModification >( this ) );

 }  // end( GeneralizedAssignmentBlock::load( memory ) )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::load( std::istream & input , char frmt )
{
 static const std::string _prfx = "GeneralizedAssignmentBlock::load: ";

 guts_of_destructor();  // discard the previous instance, if any

 if( ! ( input >> eatcomments >> f_M ) )
  throw( std::invalid_argument( _prfx +
				"error reading the number of agents" ) );
 if( ! ( input >> eatcomments >> f_N ) )
  throw( std::invalid_argument( _prfx +
				"error reading the number of jobs" ) );

 const auto read = [ & ]( std::vector< double > & v , Index size ,
			  const char * what ) {
  v.resize( size );
  for( auto & e : v )
   if( ! ( input >> eatcomments >> e ) )
    throw( std::invalid_argument( _prfx + "error reading the " + what ) );
  };

 read( v_C , f_M * f_N , "costs" );
 read( v_A , f_M * f_N , "resources" );
 read( v_B , f_M , "capacities" );

 build( _prfx );

 if( anyone_there() )
  add_Modification( std::make_shared< NBModification >( this ) );

 }  // end( GeneralizedAssignmentBlock::load( istream ) )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::deserialize( const netCDF::NcGroup & group )
{
 static const std::string _prfx =
                                "GeneralizedAssignmentBlock::deserialize: ";

 guts_of_destructor();  // discard the previous instance, if any

 const auto get_dim = [ & ]( const char * name ) -> Index {
  auto d = group.getDim( name );
  if( d.isNull() )
   throw( std::invalid_argument( _prfx + name + " dimension is required" ) );
  return( d.getSize() );
  };

 f_M = get_dim( "NAgents" );
 f_N = get_dim( "NJobs" );

 const auto get_var = [ & ]( const char * name , auto & v , Index size ) {
  v.resize( size );
  auto nv = group.getVar( name );
  if( nv.isNull() )
   throw( std::invalid_argument( _prfx + name + " are required" ) );
  if( size )
   nv.getVar( v.data() );
  };

 get_var( "Costs" , v_C , f_M * f_N );
 get_var( "Weights" , v_A , f_M * f_N );
 get_var( "Capacities" , v_B , f_M );

 build( _prfx );

 Block::deserialize( group );  // this issues the NBModification

 }  // end( GeneralizedAssignmentBlock::deserialize )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::generate_abstract_variables(
							      Configuration * stvv )
{
 if( AR & HasVar )  // the Variable are there already
  return;           // nothing to do

 Block::generate_abstract_variables( stvv );  // these of the sub-Block

 AR |= HasVar;

 }  // end( GeneralizedAssignmentBlock::generate_abstract_variables )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::generate_abstract_constraints(
							      Configuration * stcc )
{
 if( AR & HasCns )  // the Constraint are there already
  return;           // nothing to do

 // the linking Constraint use the Variable of the sub-Block
 generate_abstract_variables();

 // the assignment Constraint: sum_i x_{ij} = 1 for each job j

 v_assign.resize( f_N );
 for( Index j = 0 ; j < f_N ; ++j ) {
  LinearFunction::v_coeff_pair coeffs( f_M );
  for( Index i = 0 ; i < f_M ; ++i )
   coeffs[ i ] = { get_knapsack( i )->get_Var( j ) , 1 };

  v_assign[ j ].set_both( 1 , eNoMod );
  v_assign[ j ].set_function( new LinearFunction( std::move( coeffs ) ) ,
			      eNoMod );
  }

 add_static_constraint( v_assign , "assign" );

 Block::generate_abstract_constraints( stcc );  // these of the sub-Block

 AR |= HasCns;

 }  // end( GeneralizedAssignmentBlock::generate_abstract_constraints )

/*--------------------------------------------------------------------------*/
/*------------------------- Methods for R3 Blocks --------------------------*/
/*--------------------------------------------------------------------------*/

Block * GeneralizedAssignmentBlock::get_R3_Block( Configuration * r3bc ,
						  Block * base ,
						  Block * father )
{
 if( r3bc )
  throw( std::invalid_argument( "GeneralizedAssignmentBlock::get_R3_Block: "
				"only the copy is supported" ) );

 GeneralizedAssignmentBlock * copy;
 if( base ) {
  copy = dynamic_cast< GeneralizedAssignmentBlock * >( base );
  if( ! copy )
   throw( std::invalid_argument( "GeneralizedAssignmentBlock::get_R3_Block:"
				 " base is not a GeneralizedAssignmentBlock"
				 ) );
  }
 else
  copy = new GeneralizedAssignmentBlock( father );

 copy->load( f_M , f_N , std::vector< double >( v_C ) ,
	     std::vector< double >( v_A ) , std::vector< double >( v_B ) );

 // the sub-Block that load() has built are made copies of these of this,
 // with the items they have fixed, if any
 for( Index i = 0 ; i < v_Block.size() ; ++i )
  v_Block[ i ]->get_R3_Block( nullptr , copy->v_Block[ i ] , copy );

 return( copy );

 }  // end( GeneralizedAssignmentBlock::get_R3_Block )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::map_back_solution( Block * R3B ,
						    Configuration * r3bc ,
						    Configuration * solc )
{
 auto copy = dynamic_cast< GeneralizedAssignmentBlock * >( R3B );
 if( ( ! copy ) || ( copy->v_Block.size() != v_Block.size() ) )
  throw( std::invalid_argument( "GeneralizedAssignmentBlock::"
				"map_back_solution: R3B is not a copy" ) );

 for( Index i = 0 ; i < v_Block.size() ; ++i )
  v_Block[ i ]->map_back_solution( copy->v_Block[ i ] );

 }  // end( GeneralizedAssignmentBlock::map_back_solution )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::map_forward_solution( Block * R3B ,
						       Configuration * r3bc ,
						       Configuration * solc )
{
 auto copy = dynamic_cast< GeneralizedAssignmentBlock * >( R3B );
 if( ( ! copy ) || ( copy->v_Block.size() != v_Block.size() ) )
  throw( std::invalid_argument( "GeneralizedAssignmentBlock::"
				"map_forward_solution: R3B is not a copy" ) );

 for( Index i = 0 ; i < v_Block.size() ; ++i )
  v_Block[ i ]->map_forward_solution( copy->v_Block[ i ] );

 }  // end( GeneralizedAssignmentBlock::map_forward_solution )

/*--------------------------------------------------------------------------*/
/*-- METHODS FOR LOADING, PRINTING & SAVING THE GeneralizedAssignmentBlock -*/
/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::serialize( netCDF::NcGroup & group ) const
{
 Block::serialize( group );

 auto m = group.addDim( "NAgents" , f_M );
 auto n = group.addDim( "NJobs" , f_N );

 const std::vector< netCDF::NcDim > mn = { m , n };
 auto c = group.addVar( "Costs" , netCDF::NcDouble() , mn );
 auto a = group.addVar( "Weights" , netCDF::NcDouble() , mn );
 auto b = group.addVar( "Capacities" , netCDF::NcDouble() , m );

 if( f_M && f_N ) {
  c.putVar( v_C.data() );
  a.putVar( v_A.data() );
  }
 if( f_M )
  b.putVar( v_B.data() );

 }  // end( GeneralizedAssignmentBlock::serialize )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::print( std::ostream & output ,
					char vlvl ) const
{
 output << "GeneralizedAssignmentBlock with " << f_M << " agents and " << f_N
	<< " jobs" << std::endl;
 if( ! vlvl )
  return;

 for( Index i = 0 ; i < f_M ; ++i ) {
  output << "agent " << i << ", capacity " << v_B[ i ] << std::endl
	 << "cost:";
  for( Index j = 0 ; j < f_N ; ++j )
   output << " " << v_C[ i * f_N + j ];
  output << std::endl << "resource:";
  for( Index j = 0 ; j < f_N ; ++j )
   output << " " << v_A[ i * f_N + j ];
  output << std::endl;
  }
 }  // end( GeneralizedAssignmentBlock::print )

/*--------------------------------------------------------------------------*/
/*-------------------------- PRIVATE METHODS -------------------------------*/
/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::guts_of_destructor( void )
{
 // the Function of the linking Constraint are emptied before the
 // sub-Block, whose Variable they use, are deleted
 Constraint::clear( v_assign );
 reset_static_constraints();

 for( auto b : v_Block )
  delete b;
 v_Block.clear();

 AR = 0;

 }  // end( GeneralizedAssignmentBlock::guts_of_destructor )

/*--------------------------------------------------------------------------*/

void GeneralizedAssignmentBlock::build( const std::string & prfx )
{
 for( Index i = 0 ; i < f_M ; ++i ) {
  if( v_B[ i ] < 0 )
   throw( std::invalid_argument( prfx + "negative capacity of agent " +
				 std::to_string( i ) ) );
  for( Index j = 0 ; j < f_N ; ++j )
   if( v_A[ i * f_N + j ] < 0 )
    throw( std::invalid_argument( prfx + "negative resource of job " +
				  std::to_string( j ) + " on agent " +
				  std::to_string( i ) ) );
  }

 // one BinaryKnapsackBlock for each agent i, whose items are the jobs, with
 // the resources as weights and the costs as profits, to be minimized
 v_Block.reserve( f_M );
 for( Index i = 0 ; i < f_M ; ++i ) {
  std::vector< double > W( v_A.begin() + i * f_N ,
			   v_A.begin() + ( i + 1 ) * f_N );
  std::vector< double > P( v_C.begin() + i * f_N ,
			   v_C.begin() + ( i + 1 ) * f_N );

  auto bkb = new BinaryKnapsackBlock( this );
  bkb->load( f_N , v_B[ i ] , std::move( W ) , std::move( P ) );
  bkb->set_objective_sense( false , eNoMod , eNoMod );
  v_Block.push_back( bkb );
  }
 }  // end( GeneralizedAssignmentBlock::build )

/*--------------------------------------------------------------------------*/
/*---------------- End File GeneralizedAssignmentBlock.cpp -----------------*/
/*--------------------------------------------------------------------------*/
