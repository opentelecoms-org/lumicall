
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 decoderf.o dec_ld8f.o bitsf.o calcexc.o de_acelp.o\
 deacelpd.o dec_sidf.o dspfunc.o qsidgain.o filter.o\
 gainpred.o dec_g8k.o dec_g6k.o lpcfunc.o lspdec.o\
 lspgetq.o oper_32b.o phdisp.o post_pro.o pred_lt3.o\
 pst.o p_parity.o declag3d.o tabld8kd.o tab_dtx.o\
 tab_ld8k.o taming.o util.o basic_op.o

# Generation of the executable

decoderf : $(OBJETS)
	cc -o decoderf $(OBJETS) -lm

# Compilations if necessary

decoderf.o : decoderf.c typedef.h basic_op.h ld8k.h\
    ld8f.h dtx.h octet.h
	cc -c $(CC_OPT) decoderf.c

dec_ld8f.o : dec_ld8f.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8f.h tabld8kd.h dtx.h sid.h
	cc -c $(CC_OPT) dec_ld8f.c

bitsf.o : bitsf.c typedef.h ld8f.h ld8k.h\
    ld8kd.h tabld8kd.h tab_ld8k.h octet.h
	cc -c $(CC_OPT) bitsf.c

calcexc.o : calcexc.c typedef.h ld8k.h ld8f.h\
    dtx.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) calcexc.c

de_acelp.o : de_acelp.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) de_acelp.c

deacelpd.o : deacelpd.c typedef.h basic_op.h ld8k.h\
    ld8kd.h tabld8kd.h
	cc -c $(CC_OPT) deacelpd.c

dec_sidf.o : dec_sidf.c typedef.h ld8k.h tab_ld8k.h\
    basic_op.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) dec_sidf.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

filter.o : filter.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) filter.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) gainpred.c

dec_g8k.o : dec_g8k.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dec_g8k.c

dec_g6k.o : dec_g6k.c typedef.h basic_op.h ld8k.h\
    ld8kd.h tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) dec_g6k.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) lpcfunc.c

lspdec.o : lspdec.c typedef.h ld8k.h basic_op.h\
    tab_ld8k.h
	cc -c $(CC_OPT) lspdec.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) lspgetq.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) oper_32b.c

phdisp.o : phdisp.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8f.h tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) phdisp.c

post_pro.o : post_pro.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) post_pro.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

pst.o : pst.c typedef.h ld8k.h basic_op.h\
    oper_32b.h
	cc -c $(CC_OPT) pst.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

declag3d.o : declag3d.c typedef.h basic_op.h ld8k.h\
    ld8kd.h tabld8kd.h
	cc -c $(CC_OPT) declag3d.c

tabld8kd.o : tabld8kd.c typedef.h ld8k.h ld8kd.h\
    tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) tabld8kd.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	cc -c $(CC_OPT) tab_dtx.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) taming.c

util.o : util.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) util.c

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c
