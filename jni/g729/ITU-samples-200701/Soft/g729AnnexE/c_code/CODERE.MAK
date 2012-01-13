
# Define compiler options 
#CC=cc # Default compiler in Digital, Sun, SGI
#CC=gcc # GNU C compiler
CC=cc

#CC_OPT=-std # ANSI-C option for Digital CC compiler 
#CC_OPT=-Wall # GNU C compiler
CC_OPT=

# Targets 

OBJETS = \
 codere.o cod_ld8e.o qua_lspe.o lspgetqe.o bwfw.o\
 bwfwfunc.o acelp_e.o bitse.o filtere.o pwfe.o\
 lpce.o tab_ld8e.o basic_op.o dspfunc.o gainpred.o\
 lpcfunc.o oper_32b.o pred_lt3.o pre_proc.o p_parity.o\
 qua_gain.o pwf.o pitch.o tab_ld8k.o util.o\
 acelp_co.o

# Generation of the executable

codere : $(OBJETS)
	$(CC) -o codere $(OBJETS) -lm

# Compilations if necessary

codere.o : codere.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) codere.c

cod_ld8e.o : cod_ld8e.c typedef.h basic_op.h ld8k.h\
    ld8e.h tab_ld8k.h oper_32b.h
	$(CC) -c $(CC_OPT) cod_ld8e.c

qua_lspe.o : qua_lspe.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h ld8e.h tab_ld8e.h
	$(CC) -c $(CC_OPT) qua_lspe.c

lspgetqe.o : lspgetqe.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) lspgetqe.c

bwfw.o : bwfw.c typedef.h ld8k.h ld8e.h\
    tab_ld8e.h basic_op.h oper_32b.h
	$(CC) -c $(CC_OPT) bwfw.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8e.h\
    basic_op.h oper_32b.h tab_ld8e.h
	$(CC) -c $(CC_OPT) bwfwfunc.c

acelp_e.o : acelp_e.c typedef.h basic_op.h ld8k.h\
    ld8e.h tab_ld8e.h
	$(CC) -c $(CC_OPT) acelp_e.c

bitse.o : bitse.c typedef.h ld8k.h ld8e.h\
    tab_ld8e.h tab_ld8k.h
	$(CC) -c $(CC_OPT) bitse.c

filtere.o : filtere.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) filtere.c

pwfe.o : pwfe.c typedef.h
	$(CC) -c $(CC_OPT) pwfe.c

lpce.o : lpce.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h ld8e.h
	$(CC) -c $(CC_OPT) lpce.c

tab_ld8e.o : tab_ld8e.c typedef.h ld8k.h ld8e.h\
    tab_ld8e.h
	$(CC) -c $(CC_OPT) tab_ld8e.c

basic_op.o : basic_op.c typedef.h basic_op.h
	$(CC) -c $(CC_OPT) basic_op.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) -c $(CC_OPT) dspfunc.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	$(CC) -c $(CC_OPT) gainpred.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) lpcfunc.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	$(CC) -c $(CC_OPT) oper_32b.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) -c $(CC_OPT) pred_lt3.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) pre_proc.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) p_parity.c

qua_gain.o : qua_gain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) qua_gain.c

pwf.o : pwf.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) pwf.c

pitch.o : pitch.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) pitch.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) tab_ld8k.c

util.o : util.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) util.c

acelp_co.o : acelp_co.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) acelp_co.c
