##############################################################################
################################## makefile ##################################
##############################################################################
#                                                                            #
#   makefile of GeneralizedAssignmentBlock                                   #
#                                                                            #
#   Note that $(SMS++INC) is assumed to include any -I directive             #
#   corresponding to external libraries needed by SMS++, at least to the     #
#   extent in which they are needed by the parts of SMS++ used by            #
#   GeneralizedAssignmentBlock, and that the same holds for $(BKBkINC) and   #
#   BinaryKnapsackBlock.                                                     #
#                                                                            #
#   Input:  $(CC)        = compiler command                                  #
#           $(SW)        = compiler options                                  #
#           $(SMS++INC)  = the -I$( core SMS++ directory )                   #
#           $(SMS++OBJ)  = the libSMS++ library itself                       #
#           $(BKBkINC)   = the -I$( BinaryKnapsackBlock directory )          #
#           $(BKBkH)     = the .h files of BinaryKnapsackBlock               #
#           $(GAPBkSDR)  = the directory where the source is                 #
#                                                                            #
#   Output: $(GAPBkOBJ)  = the final object(s) / library                     #
#           $(GAPBkH)    = the .h files to include                           #
#           $(GAPBkINC)  = the -I$( source directory )                       #
#                                                                            #
#                             Antonio Frangioni                              #
#                         Dipartimento di Informatica                        #
#                             Universita' di Pisa                            #
#                                                                            #
##############################################################################

# macros to be exported - - - - - - - - - - - - - - - - - - - - - - - - - - -

GAPBkOBJ = $(GAPBkSDR)/obj/GeneralizedAssignmentBlock.o \
	$(GAPBkSDR)/obj/RepairGeneralizedAssignmentSolver.o

GAPBkINC = -I$(GAPBkSDR)/include

GAPBkH   = $(GAPBkSDR)/include/GeneralizedAssignmentBlock.h \
	$(GAPBkSDR)/include/RepairGeneralizedAssignmentSolver.h

# the C++20 module smspp.generalizedassignmentblock - - - - - - - - - - - - - -
# [see makefile-mod of SMS++]

$(eval $(call SMSppMODrule,GAPBk,$(GAPBkSDR),GeneralizedAssignmentBlock,\
	smspp.generalizedassignmentblock,BKBk))

# clean - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -f $(GAPBkOBJ) $(GAPBkSDR)/*~

# dependencies: every .o from its .cpp + every recursively included .h- - - -

$(GAPBkSDR)/obj/GeneralizedAssignmentBlock.o: \
	$(GAPBkSDR)/src/GeneralizedAssignmentBlock.cpp $(GAPBkH) $(BKBkH) \
	$(SMS++H) $(SMS++OBJ)
	$(CC) -c $(GAPBkSDR)/src/GeneralizedAssignmentBlock.cpp -o $@ \
	$(GAPBkINC) $(BKBkINC) $(SMS++INC) $(SW)

$(GAPBkSDR)/obj/RepairGeneralizedAssignmentSolver.o: \
	$(GAPBkSDR)/src/RepairGeneralizedAssignmentSolver.cpp $(GAPBkH) \
	$(BKBkH) $(SMS++H) $(SMS++OBJ)
	$(CC) -c $(GAPBkSDR)/src/RepairGeneralizedAssignmentSolver.cpp -o $@ \
	$(GAPBkINC) $(BKBkINC) $(SMS++INC) $(SW)

########################## End of makefile ###################################
