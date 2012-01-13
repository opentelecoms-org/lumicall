
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 decoderg.o dec_ld8g.o basic_op.o bitsg.o bwfwfunc.o\
 calcexc.o decgaine.o dec_lag3.o dec_sidf.o qsidgain.o\
 dspfunc.o filtere.o gainpred.o lpcfunc.o lpcg.o\
 lspdece.o lspgetqe.o deacelpe.o de_acelp.o oper_32b.o\
 post_pro.o pred_lt3.o pstg.o p_parity.o tab_dtx.o\
 tab_ld8e.o tab_ld8k.o taming.o track_pi.o util.o

# Generation of the executable

decoderg : $(OBJETS)
	cc -o decoderg $(OBJETS) -lm

# Compilations if necessary

decoderg.o : decoderg.c typedef.h basic_op.h ld8k.h\
    ld8e.h ld8g.h dtx.h octet.h
	cc -c $(CC_OPT) decoderg.c

dec_ld8g.o : dec_ld8g.c typedef.h basic_op.h ld8k.h\
    ld8e.h ld8g.h dtx.h sid.h
	cc -c $(CC_OPT) dec_ld8g.c

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c

bitsg.o : bitsg.c typedef.h ld8k.h ld8e.h\
    ld8g.h tab_ld8k.h tab_ld8e.h octet.h
	cc -c $(CC_OPT) bitsg.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8e.h\
    basic_op.h oper_32b.h tab_ld8e.h
	cc -c $(CC_OPT) bwfwfunc.c

calcexc.o : calcexc.c typedef.h ld8k.h dtx.h\
    basic_op.h oper_32b.h
	cc -c $(CC_OPT) calcexc.c

decgaine.o : decgaine.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) decgaine.c

dec_lag3.o : dec_lag3.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) dec_lag3.c

dec_sidf.o : dec_sidf.c typedef.h ld8k.h tab_ld8k.h\
    basic_op.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) dec_sidf.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c

filtere.o : filtere.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	cc -c $(CC_OPT) filtere.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) gainpred.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) lpcfunc.c

lpcg.o : lpcg.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8e.h tab_ld8k.h ld8g.h
	cc -c $(CC_OPT) lpcg.c

lspdece.o : lspdece.c typedef.h basic_op.h ld8k.h\
    ld8e.h tab_ld8k.h tab_ld8e.h
	cc -c $(CC_OPT) lspdece.c

lspgetqe.o : lspgetqe.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	cc -c $(CC_OPT) lspgetqe.c

deacelpe.o : deacelpe.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) deacelpe.c

de_acelp.o : de_acelp.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) de_acelp.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) oper_32b.c

post_pro.o : post_pro.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) post_pro.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

pstg.o : pstg.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8g.h ld8e.h
	cc -c $(CC_OPT) pstg.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	cc -c $(CC_OPT) tab_dtx.c

tab_ld8e.o : tab_ld8e.c typedef.h ld8k.h ld8e.h\
    tab_ld8e.h
	cc -c $(CC_OPT) tab_ld8e.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) taming.c

track_pi.o : track_pi.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	cc -c $(CC_OPT) track_pi.c

util.o : util.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) util.c
