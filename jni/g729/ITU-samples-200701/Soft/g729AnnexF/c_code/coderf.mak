
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 coderf.o cod_ld8f.o lpc.o lpcfunc.o lspgetq.o\
 filter.o pre_proc.o pred_lt3.o pitchd.o phdisp.o\
 acelpcod.o bitsf.o calcexc.o dtx.o gainpred.o\
 dec_sidf.o qsidgain.o qua_g6k.o qua_g8k.o qua_lsp.o\
 tabld8kd.o tab_ld8k.o tab_dtx.o taming.o util.o\
 vad.o oper_32b.o basic_op.o dspfunc.o pwf.o\
 p_parity.o qsidlsf.o

# Generation of the executable

coderf : $(OBJETS)
	cc -o coderf $(OBJETS) -lm

# Compilations if necessary

coderf.o : coderf.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8f.h tabld8kd.h dtx.h octet.h
	cc -c $(CC_OPT) coderf.c

cod_ld8f.o : cod_ld8f.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8f.h tab_ld8k.h oper_32b.h vad.h\
    dtx.h sid.h
	cc -c $(CC_OPT) cod_ld8f.c

lpc.o : lpc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) lpc.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) lpcfunc.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) lspgetq.c

filter.o : filter.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) filter.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) pre_proc.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

pitchd.o : pitchd.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8kd.h tabld8kd.h tab_ld8k.h
	cc -c $(CC_OPT) pitchd.c

phdisp.o : phdisp.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8f.h tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) phdisp.c

acelpcod.o : acelpcod.c typedef.h basic_op.h ld8k.h\
    ld8kd.h tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) acelpcod.c

bitsf.o : bitsf.c typedef.h ld8f.h ld8k.h\
    ld8kd.h tabld8kd.h tab_ld8k.h octet.h
	cc -c $(CC_OPT) bitsf.c

calcexc.o : calcexc.c typedef.h ld8k.h ld8f.h\
    dtx.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) calcexc.c

dtx.o : dtx.c typedef.h basic_op.h ld8k.h\
    oper_32b.h tab_ld8k.h vad.h dtx.h tab_dtx.h\
    sid.h
	cc -c $(CC_OPT) dtx.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) gainpred.c

dec_sidf.o : dec_sidf.c typedef.h ld8k.h tab_ld8k.h\
    basic_op.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) dec_sidf.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

qua_g6k.o : qua_g6k.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8kd.h tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) qua_g6k.c

qua_g8k.o : qua_g8k.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) qua_g8k.c

qua_lsp.o : qua_lsp.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) qua_lsp.c

tabld8kd.o : tabld8kd.c typedef.h ld8k.h ld8kd.h\
    tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) tabld8kd.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	cc -c $(CC_OPT) tab_dtx.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) taming.c

util.o : util.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) util.c

vad.o : vad.c typedef.h ld8k.h basic_op.h\
    oper_32b.h tab_ld8k.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) vad.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) oper_32b.c

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c

pwf.o : pwf.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) pwf.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

qsidlsf.o : qsidlsf.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h sid.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) qsidlsf.c
