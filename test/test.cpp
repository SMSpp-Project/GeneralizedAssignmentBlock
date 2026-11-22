/*--------------------------------------------------------------------------*/
/*------------------------------ File test.cpp -----------------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Smoke test for GeneralizedAssignmentBlock: constructs a GeneralizedAssignmentBlock via the Block
 * factory, checking that the module links correctly and the class is
 * registered. Replace it with real tests exercising the module.
 *
 * \author Antonio Frangioni \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Antonio Frangioni
 */
/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include <iostream>

#include "GeneralizedAssignmentBlock.h"

/*--------------------------------------------------------------------------*/
/*-------------------------------- USING -----------------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*-------------------------------- main() ----------------------------------*/
/*--------------------------------------------------------------------------*/

int main( int argc , char ** argv )
{
 // construct a GeneralizedAssignmentBlock via the Block factory: this checks that the
 // class is registered and the library is linked in (whole-archive)
 auto block = Block::new_Block( "GeneralizedAssignmentBlock" );

 if( ! block ) {
  std::cerr << "GeneralizedAssignmentBlock not present in Block factory" << std::endl;
  return( 1 );
  }

 if( ! dynamic_cast< GeneralizedAssignmentBlock * >( block ) ) {
  std::cerr << "factory did not return a GeneralizedAssignmentBlock" << std::endl;
  delete block;
  return( 1 );
  }

 delete block;

 std::cout << "GeneralizedAssignmentBlock: all tests passed" << std::endl;

 return( 0 );
 }

/*--------------------------------------------------------------------------*/
/*---------------------------- End File test.cpp ---------------------------*/
/*--------------------------------------------------------------------------*/
