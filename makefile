##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of GeneralizedAssignmentBlock                                   #
#                                                                            #
#   Note that $(SMS++INC) is assumed to include any -I directive             #
#   corresponding to external libraries needed by SMS++, at least to the     #
#   extent in which they are needed by the parts of SMS++ used by            #
#   GeneralizedAssignmentBlock.                                              #
#                                                                            #
#   Input:  $(CC)        = compiler command                                  #
#           $(SW)        = compiler options                                  #
#           $(SMS++INC)  = the -I$( core SMS++ directory )                   #
#           $(SMS++OBJ)  = the libSMS++ library itself                       #
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

GAPBkOBJ = $(GAPBkSDR)/obj/GeneralizedAssignmentBlock.o

GAPBkINC = -I$(GAPBkSDR)/include

GAPBkH   = $(GAPBkSDR)/include/GeneralizedAssignmentBlock.h

# the C++20 module [see makefile-mod of SMS++] - - - - - - - - - - - - - - -
# the prefixes of the modules of the dependencies (e.g., UCBck) go in place
# of SMS++

$(eval $(call SMSppMODrule,GAPBk,$(GAPBkSDR),GeneralizedAssignmentBlock,\
	smspp.generalizedassignmentblock,SMS++))

# clean - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -f $(GAPBkOBJ) $(GAPBkSDR)/*~

# dependencies: every .o from its .cpp + every recursively included .h- - - -

$(GAPBkSDR)/obj/GeneralizedAssignmentBlock.o: $(GAPBkSDR)/src/GeneralizedAssignmentBlock.cpp \
	$(GAPBkSDR)/include/GeneralizedAssignmentBlock.h $(SMS++H) $(SMS++OBJ)
	$(CC) -c $(GAPBkSDR)/src/GeneralizedAssignmentBlock.cpp -o $@ \
	$(GAPBkINC) $(SMS++INC) $(SW)

########################## End of makefile ###################################
