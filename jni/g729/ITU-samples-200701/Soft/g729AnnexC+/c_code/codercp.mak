
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 mus_dtct.o codercp.o codld8cp.o q_gaincp.o tabld8cp.o\
 lpfunccp.o dtx.o pre_proc.o bitscp.o lpccp.o\
 acelp_cp.o bwfw.o bwfwfunc.o cor_func.o filtere.o\
 gainpred.o lspgetqe.o pitchcp.o pred_lt3.o pwf.o\
 pwfe.o p_parity.o qsidgain.o qsidlsf.o qua_lspe.o\
 tab_dtx.o tab_ld8k.o taming.o utilcp.o vad.o\
 calcexc.o dec_sid.o phdisp.o

# Generation of the executable

codercp : $(OBJETS)
	cc -o codercp $(OBJETS) -lm

# Compilations if necessary

mus_dtct.o : mus_dtct.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h vad.h
	cc -c $(CC_OPT) mus_dtct.c

codercp.o : codercp.c typedef.h ld8k.h ld8cp.h\
    dtx.h octet.h
	cc -c $(CC_OPT) codercp.c

codld8cp.o : codld8cp.c typedef.h ld8k.h ld8cp.h\
    tab_ld8k.h vad.h dtx.h sid.h
	cc -c $(CC_OPT) codld8cp.c

q_gaincp.o : q_gaincp.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) q_gaincp.c

tabld8cp.o : tabld8cp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) tabld8cp.c

lpfunccp.o : lpfunccp.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) lpfunccp.c

dtx.o : dtx.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h vad.h dtx.h tab_dtx.h sid.h
	cc -c $(CC_OPT) dtx.c

pre_proc.o : pre_proc.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) pre_proc.c

bitscp.o : bitscp.c typedef.h ld8k.h ld8cp.h\
    tab_ld8k.h tabld8cp.h vad.h dtx.h tab_dtx.h\
    octet.h
	cc -c $(CC_OPT) bitscp.c

lpccp.o : lpccp.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) lpccp.c

acelp_cp.o : acelp_cp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) acelp_cp.c

bwfw.o : bwfw.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) bwfw.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) bwfwfunc.c

cor_func.o : cor_func.c typedef.h ld8k.h
	cc -c $(CC_OPT) cor_func.c

filtere.o : filtere.c typedef.h ld8k.h ld8cp.h
	cc -c $(CC_OPT) filtere.c

gainpred.o : gainpred.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) gainpred.c

lspgetqe.o : lspgetqe.c typedef.h ld8k.h ld8cp.h
	cc -c $(CC_OPT) lspgetqe.c

pitchcp.o : pitchcp.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) pitchcp.c

pred_lt3.o : pred_lt3.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

pwf.o : pwf.c typedef.h ld8k.h
	cc -c $(CC_OPT) pwf.c

pwfe.o : pwfe.c typedef.h
	cc -c $(CC_OPT) pwfe.c

p_parity.o : p_parity.c typedef.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

qsidgain.o : qsidgain.c typedef.h ld8k.h vad.h\
    dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

qsidlsf.o : qsidlsf.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h sid.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) qsidlsf.c

qua_lspe.o : qua_lspe.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) qua_lspe.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	cc -c $(CC_OPT) tab_dtx.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

taming.o : taming.c typedef.h ld8k.h
	cc -c $(CC_OPT) taming.c

utilcp.o : utilcp.c typedef.h ld8k.h
	cc -c $(CC_OPT) utilcp.c

vad.o : vad.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) vad.c

calcexc.o : calcexc.c typedef.h ld8k.h ld8cp.h\
    dtx.h
	cc -c $(CC_OPT) calcexc.c

dec_sid.o : dec_sid.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h vad.h dtx.h sid.h\
    tab_dtx.h
	cc -c $(CC_OPT) dec_sid.c

phdisp.o : phdisp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) phdisp.c
