#-----------------------------------------------------------------------
# Makefile for compiler the decoder of the 
# G.729 Annex C Floating point implementation of 
# the 8 kbit/s G.729 (main body) algorithm
# This makefile was prepared for Unix systems
#-----------------------------------------------------------------------

# Define compiler options
#CC_OPT=-std # For Digital cc compiler

# Targets
OBJETS = \
 decoder.o dec_ld8k.o lpcfunc.o postfil.o tab_ld8k.o\
 bits.o dec_lag3.o de_acelp.o dec_gain.o gainpred.o\
 lspgetq.o lspdec.o filter.o post_pro.o pred_lt3.o\
 p_parity.o util.o

# Generation of the executable
decoder : $(OBJETS)
	$(CC) -o decoder $(OBJETS) -lm

# Compilations if necessary

decoder.o : decoder.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) decoder.c

dec_ld8k.o : dec_ld8k.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) dec_ld8k.c

lpcfunc.o : lpcfunc.c typedef.h ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) lpcfunc.c

postfil.o : postfil.c typedef.h ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) postfil.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) tab_ld8k.c

bits.o : bits.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) bits.c

dec_lag3.o : dec_lag3.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) dec_lag3.c

de_acelp.o : de_acelp.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) de_acelp.c

dec_gain.o : dec_gain.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) dec_gain.c

gainpred.o : gainpred.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) gainpred.c

lspgetq.o : lspgetq.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) lspgetq.c

lspdec.o : lspdec.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) lspdec.c

filter.o : filter.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) filter.c

post_pro.o : post_pro.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) post_pro.c

pred_lt3.o : pred_lt3.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) pred_lt3.c

p_parity.o : p_parity.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) p_parity.c

util.o : util.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) util.c
