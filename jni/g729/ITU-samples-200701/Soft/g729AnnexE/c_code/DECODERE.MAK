
# Define compiler options 
#CC=cc # Default compiler in Digital, Sun, SGI
#CC=gcc # GNU C compiler
CC=cc

#CC_OPT=-std # ANSI-C option for Digital $(CC) compiler 
#CC_OPT=-Wall # GNU C compiler
CC_OPT=

# Targets 

OBJETS = \
 decodere.o dec_ld8e.o track_pi.o bitse.o lpce.o\
 bwfwfunc.o deacelpe.o decgaine.o filtere.o lspgetqe.o\
 lspdece.o tab_ld8e.o pste.o de_acelp.o\
 post_pro.o pred_lt3.o dec_lag3.o p_parity.o tab_ld8k.o\
 util.o gainpred.o basic_op.o dspfunc.o lpcfunc.o\
 oper_32b.o

# Generation of the executable

decodere : $(OBJETS)
	$(CC) -o decodere $(OBJETS) -lm

# Compilations if necessary

decodere.o : decodere.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) decodere.c

dec_ld8e.o : dec_ld8e.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) dec_ld8e.c

track_pi.o : track_pi.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) track_pi.c

bitse.o : bitse.c typedef.h ld8k.h ld8e.h\
    tab_ld8e.h tab_ld8k.h
	$(CC) -c $(CC_OPT) bitse.c

lpce.o : lpce.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h ld8e.h
	$(CC) -c $(CC_OPT) lpce.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8e.h\
    basic_op.h oper_32b.h tab_ld8e.h
	$(CC) -c $(CC_OPT) bwfwfunc.c

deacelpe.o : deacelpe.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) deacelpe.c

decgaine.o : decgaine.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) -c $(CC_OPT) decgaine.c

filtere.o : filtere.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) filtere.c

lspgetqe.o : lspgetqe.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	$(CC) -c $(CC_OPT) lspgetqe.c

lspdece.o : lspdece.c typedef.h basic_op.h ld8k.h\
    ld8e.h tab_ld8k.h tab_ld8e.h
	$(CC) -c $(CC_OPT) lspdece.c

tab_ld8e.o : tab_ld8e.c typedef.h ld8k.h ld8e.h\
    tab_ld8e.h
	$(CC) -c $(CC_OPT) tab_ld8e.c

pste.o : pste.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8e.h
	$(CC) -c $(CC_OPT) pste.c

de_acelp.o : de_acelp.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) de_acelp.c

post_pro.o : post_pro.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) post_pro.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) -c $(CC_OPT) pred_lt3.c

dec_lag3.o : dec_lag3.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) dec_lag3.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) p_parity.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) tab_ld8k.c

util.o : util.c typedef.h basic_op.h ld8k.h
	$(CC) -c $(CC_OPT) util.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	$(CC) -c $(CC_OPT) gainpred.c

basic_op.o : basic_op.c typedef.h basic_op.h
	$(CC) -c $(CC_OPT) basic_op.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) -c $(CC_OPT) dspfunc.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) -c $(CC_OPT) lpcfunc.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	$(CC) -c $(CC_OPT) oper_32b.c
