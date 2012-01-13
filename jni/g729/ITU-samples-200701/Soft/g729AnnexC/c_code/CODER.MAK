#-----------------------------------------------------------------------
# Makefile for compiler the encoder of the 
# G.729 Annex C Floating point implementation of 
# the 8 kbit/s G.729 (main body) algorithm
# This makefile was prepared for Unix systems
#-----------------------------------------------------------------------

# Define compiler options
#CC_OPT=-std # For Digital CC compiler

# Targets
OBJETS = \
 coder.o cod_ld8k.o acelp_co.o lpc.o lpcfunc.o\
 pitch.o tab_ld8k.o pwf.o bits.o filter.o\
 gainpred.o lspgetq.o pred_lt3.o pre_proc.o p_parity.o\
 qua_lsp.o qua_gain.o util.o cor_func.o taming.o

# Generation of the executable
coder : $(OBJETS)
	$(CC) -o coder $(OBJETS) -lm

# Compilations if necessary

coder.o : coder.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) coder.c

cod_ld8k.o : cod_ld8k.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) cod_ld8k.c

acelp_co.o : acelp_co.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) acelp_co.c

lpc.o : lpc.c typedef.h ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) lpc.c

lpcfunc.o : lpcfunc.c typedef.h ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) lpcfunc.c

pitch.o : pitch.c typedef.h ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) pitch.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) tab_ld8k.c

pwf.o : pwf.c typedef.h ld8k.h
	$(CC) -c $(CC_OPT) pwf.c

bits.o : bits.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) bits.c

filter.o : filter.c typedef.h version.h ld8k.h
	$(CC) -c $(CC_OPT) filter.c

gainpred.o : gainpred.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) gainpred.c

lspgetq.o : lspgetq.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) lspgetq.c

pred_lt3.o : pred_lt3.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) pred_lt3.c

pre_proc.o : pre_proc.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) pre_proc.c

p_parity.o : p_parity.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) p_parity.c

qua_lsp.o : qua_lsp.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) qua_lsp.c

qua_gain.o : qua_gain.c typedef.h version.h \
     ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) qua_gain.c

util.o : util.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) util.c

cor_func.o : cor_func.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) cor_func.c

taming.o : taming.c typedef.h version.h \
    ld8k.h
	$(CC) -c $(CC_OPT) taming.c
