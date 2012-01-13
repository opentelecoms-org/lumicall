#-----------------------------------------------------------------------
# Makefile for compiler the encoder of the 
# G.729 Annex C Floating point implementation of 
# the 8 kbit/s G.729 Annex A algorithm
# This makefile was prepared for Unix systems
#-----------------------------------------------------------------------

# Define compiler options
#CC_OPT=-std # For Digital cc compiler

# Targets
OBJETS = \
 codera.o cod_ld8a.o acelp_ca.o pitch_a.o lpca.o\
 lpcfunca.o tab_ld8a.o bits.o cor_func.o filter.o\
 gainpred.o lspgetq.o p_parity.o pre_proc.o pred_lt3.o\
 qua_gain.o qua_lsp.o taming.o util.o

# Generation of the executable
codera : $(OBJETS)
	$(CC) -o codera $(OBJETS) -lm

# Compilations if necessary

codera.o : codera.c typedef.h ld8a.h
	$(CC) -c $(CC_OPT) codera.c

cod_ld8a.o : cod_ld8a.c typedef.h ld8a.h
	$(CC) -c $(CC_OPT) cod_ld8a.c

acelp_ca.o : acelp_ca.c typedef.h ld8a.h
	$(CC) -c $(CC_OPT) acelp_ca.c

pitch_a.o : pitch_a.c typedef.h ld8a.h
	$(CC) -c $(CC_OPT) pitch_a.c

lpca.o : lpca.c typedef.h ld8a.h tab_ld8a.h
	$(CC) -c $(CC_OPT) lpca.c

lpcfunca.o : lpcfunca.c typedef.h ld8a.h tab_ld8a.h
	$(CC) -c $(CC_OPT) lpcfunca.c

tab_ld8a.o : tab_ld8a.c typedef.h ld8a.h
	$(CC) -c $(CC_OPT) tab_ld8a.c

bits.o : bits.c typedef.h version.h ld8a.h\
    tab_ld8a.h
	$(CC) -c $(CC_OPT) bits.c

cor_func.o : cor_func.c typedef.h version.h ld8a.h
	$(CC) -c $(CC_OPT) cor_func.c

filter.o : filter.c typedef.h version.h ld8a.h
	$(CC) -c $(CC_OPT) filter.c

gainpred.o : gainpred.c typedef.h version.h ld8a.h\
    tab_ld8a.h
	$(CC) -c $(CC_OPT) gainpred.c

lspgetq.o : lspgetq.c typedef.h version.h ld8a.h
	$(CC) -c $(CC_OPT) lspgetq.c

p_parity.o : p_parity.c typedef.h version.h ld8a.h
	$(CC) -c $(CC_OPT) p_parity.c

pre_proc.o : pre_proc.c typedef.h version.h ld8a.h\
    tab_ld8a.h 
	$(CC) -c $(CC_OPT) pre_proc.c

pred_lt3.o : pred_lt3.c typedef.h version.h ld8a.h\
    tab_ld8a.h 
	$(CC) -c $(CC_OPT) pred_lt3.c

qua_gain.o : qua_gain.c typedef.h version.h ld8a.h\
    tab_ld8a.h 
	$(CC) -c $(CC_OPT) qua_gain.c

qua_lsp.o : qua_lsp.c typedef.h version.h ld8a.h\
    tab_ld8a.h 
	$(CC) -c $(CC_OPT) qua_lsp.c

taming.o : taming.c typedef.h version.h ld8a.h
	$(CC) -c $(CC_OPT) taming.c

util.o : util.c typedef.h version.h ld8a.h
	$(CC) -c $(CC_OPT) util.c

