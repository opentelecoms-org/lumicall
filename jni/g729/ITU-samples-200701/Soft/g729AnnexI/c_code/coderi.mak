
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 coderi.o codld8cp.o mus_dtct.o acelp_cp.o basic_op.o\
 bitscp.o bwfw.o bwfwfunc.o dspfunc.o filtere.o\
 gainpred.o lpccp.o lpcfunc.o lspgetqe.o oper_32b.o\
 pitchcp.o pred_lt3.o pre_proc.o pwf.o pwfe.o\
 p_parity.o qua_lspe.o q_gaincp.o tabld8cp.o tab_dtx.o\
 tab_ld8k.o utilcp.o vad.o dtx.o calcexc.o\
 qsidgain.o qsidlsf.o taming.o dec_sid.o phdisp.o

# Generation of the executable

coderi : $(OBJETS)
	cc -o coderi $(OBJETS) -lm

# Compilations if necessary

coderi.o : coderi.c typedef.h basic_op.h ld8k.h\
    ld8cp.h dtx.h octet.h
	cc -c $(CC_OPT) coderi.c

codld8cp.o : codld8cp.c typedef.h basic_op.h ld8k.h\
    ld8cp.h tab_ld8k.h oper_32b.h vad.h dtx.h\
    sid.h
	cc -c $(CC_OPT) codld8cp.c

mus_dtct.o : mus_dtct.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h basic_op.h oper_32b.h vad.h
	cc -c $(CC_OPT) mus_dtct.c

acelp_cp.o : acelp_cp.c typedef.h basic_op.h ld8k.h\
    ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) acelp_cp.c

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c

bitscp.o : bitscp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h tab_ld8k.h octet.h
	cc -c $(CC_OPT) bitscp.c

bwfw.o : bwfw.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) bwfw.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8cp.h\
    basic_op.h oper_32b.h tabld8cp.h
	cc -c $(CC_OPT) bwfwfunc.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c

filtere.o : filtere.c typedef.h basic_op.h ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) filtere.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) gainpred.c

lpccp.o : lpccp.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) lpccp.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) lpcfunc.c

lspgetqe.o : lspgetqe.c typedef.h basic_op.h ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) lspgetqe.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) oper_32b.c

pitchcp.o : pitchcp.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8cp.h tab_ld8k.h
	cc -c $(CC_OPT) pitchcp.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) pre_proc.c

pwf.o : pwf.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) pwf.c

pwfe.o : pwfe.c typedef.h
	cc -c $(CC_OPT) pwfe.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

qua_lspe.o : qua_lspe.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) qua_lspe.c

q_gaincp.o : q_gaincp.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) q_gaincp.c

tabld8cp.o : tabld8cp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) tabld8cp.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	cc -c $(CC_OPT) tab_dtx.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

utilcp.o : utilcp.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) utilcp.c

vad.o : vad.c typedef.h ld8k.h basic_op.h\
    oper_32b.h tab_ld8k.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) vad.c

dtx.o : dtx.c typedef.h basic_op.h ld8k.h\
    ld8cp.h oper_32b.h tab_ld8k.h vad.h dtx.h\
    tab_dtx.h sid.h
	cc -c $(CC_OPT) dtx.c

calcexc.o : calcexc.c typedef.h ld8k.h ld8cp.h\
    dtx.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) calcexc.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

qsidlsf.o : qsidlsf.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h sid.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) qsidlsf.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) taming.c

dec_sid.o : dec_sid.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h basic_op.h vad.h dtx.h\
    sid.h tab_dtx.h
	cc -c $(CC_OPT) dec_sid.c

phdisp.o : phdisp.c typedef.h basic_op.h ld8k.h\
    ld8cp.h tab_ld8k.h tabld8cp.h
	cc -c $(CC_OPT) phdisp.c
