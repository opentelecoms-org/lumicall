
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 coderg.o cod_ld8g.o acelp_co.o acelp_e.o basic_op.o\
 bitsg.o bwfwfunc.o bwfwg.o calcexc.o dtxg.o\
 filtere.o gainpred.o lpcfunc.o lpcg.o lspgetqe.o\
 mus_dtct.o oper_32b.o pitch.o pred_lt3.o pre_proc.o\
 pwf.o pwfe.o qua_gain.o p_parity.o dec_sidf.o\
 qsidgain.o qsidlsf.o qua_lspe.o tab_dtx.o tab_ld8e.o\
 tab_ld8k.o taming.o vadg.o util.o dspfunc.o

# Generation of the executable

coderg : $(OBJETS)
	cc -o coderg $(OBJETS) -lm

# Compilations if necessary

coderg.o : coderg.c typedef.h basic_op.h ld8k.h\
    ld8e.h ld8g.h dtx.h octet.h
	cc -c $(CC_OPT) coderg.c

cod_ld8g.o : cod_ld8g.c typedef.h basic_op.h ld8k.h\
    ld8g.h ld8e.h tab_ld8k.h oper_32b.h vad.h\
    dtx.h sid.h
	cc -c $(CC_OPT) cod_ld8g.c

acelp_co.o : acelp_co.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) acelp_co.c

acelp_e.o : acelp_e.c typedef.h basic_op.h ld8k.h\
    ld8e.h tab_ld8e.h
	cc -c $(CC_OPT) acelp_e.c

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c

bitsg.o : bitsg.c typedef.h ld8k.h ld8e.h\
    ld8g.h tab_ld8k.h tab_ld8e.h octet.h
	cc -c $(CC_OPT) bitsg.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8e.h\
    basic_op.h oper_32b.h tab_ld8e.h
	cc -c $(CC_OPT) bwfwfunc.c

bwfwg.o : bwfwg.c typedef.h ld8k.h ld8e.h\
    ld8g.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) bwfwg.c

calcexc.o : calcexc.c typedef.h ld8k.h dtx.h\
    basic_op.h oper_32b.h
	cc -c $(CC_OPT) calcexc.c

dtxg.o : dtxg.c typedef.h basic_op.h ld8k.h\
    ld8e.h ld8g.h oper_32b.h tab_ld8k.h vad.h\
    dtx.h tab_dtx.h sid.h
	cc -c $(CC_OPT) dtxg.c

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

lspgetqe.o : lspgetqe.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	cc -c $(CC_OPT) lspgetqe.c

mus_dtct.o : mus_dtct.c typedef.h ld8k.h tab_ld8k.h\
    ld8g.h basic_op.h oper_32b.h vad.h
	cc -c $(CC_OPT) mus_dtct.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) oper_32b.c

pitch.o : pitch.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) pitch.c

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

qua_gain.o : qua_gain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) qua_gain.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

dec_sidf.o : dec_sidf.c typedef.h ld8k.h tab_ld8k.h\
    basic_op.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) dec_sidf.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

qsidlsf.o : qsidlsf.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h sid.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) qsidlsf.c

qua_lspe.o : qua_lspe.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h ld8e.h tab_ld8e.h
	cc -c $(CC_OPT) qua_lspe.c

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

vadg.o : vadg.c typedef.h ld8k.h basic_op.h\
    oper_32b.h tab_ld8k.h vad.h dtx.h tab_dtx.h
	cc -c $(CC_OPT) vadg.c

util.o : util.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) util.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c
