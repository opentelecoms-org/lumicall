
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 coderh.o cod_ld8h.o bitsh.o acelp_h.o bwfwh.o\
 bwfwfunc.o dspfunc.o filtere.o gainpred.o lpce.o\
 lpcfunc.o lspdece.o lspgetqe.o oper_32b.o phdisp.o\
 pitchh.o pred_lt3.o pre_proc.o p_parity.o qua_lspe.o\
 qua_g6k.o qua_g8k.o pwf.o pwfe.o tab_ld8k.o\
 tabld8kd.o tab_ld8e.o taming.o util.o basic_op.o

# Generation of the executable

coderh : $(OBJETS)
	cc -o coderh $(OBJETS) -lm

# Compilations if necessary

coderh.o : coderh.c typedef.h basic_op.h ld8k.h\
    ld8h.h ld8e.h
	cc -c $(CC_OPT) coderh.c

cod_ld8h.o : cod_ld8h.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8e.h ld8h.h tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) cod_ld8h.c

bitsh.o : bitsh.c typedef.h ld8k.h ld8e.h\
    ld8kd.h ld8h.h tab_ld8k.h tab_ld8e.h tabld8kd.h
	cc -c $(CC_OPT) bitsh.c

acelp_h.o : acelp_h.c typedef.h basic_op.h ld8k.h\
    ld8e.h ld8kd.h ld8h.h tab_ld8e.h tabld8kd.h
	cc -c $(CC_OPT) acelp_h.c

bwfwh.o : bwfwh.c typedef.h ld8k.h ld8e.h\
    ld8h.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) bwfwh.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8e.h\
    basic_op.h oper_32b.h tab_ld8e.h
	cc -c $(CC_OPT) bwfwfunc.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c

filtere.o : filtere.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	cc -c $(CC_OPT) filtere.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) gainpred.c

lpce.o : lpce.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h ld8e.h
	cc -c $(CC_OPT) lpce.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) lpcfunc.c

lspdece.o : lspdece.c typedef.h basic_op.h ld8k.h\
    ld8e.h tab_ld8k.h tab_ld8e.h
	cc -c $(CC_OPT) lspdece.c

lspgetqe.o : lspgetqe.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	cc -c $(CC_OPT) lspgetqe.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) oper_32b.c

phdisp.o : phdisp.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8h.h tabld8kd.h
	cc -c $(CC_OPT) phdisp.c

pitchh.o : pitchh.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8h.h tab_ld8k.h
	cc -c $(CC_OPT) pitchh.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) pre_proc.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

qua_lspe.o : qua_lspe.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h ld8e.h tab_ld8e.h
	cc -c $(CC_OPT) qua_lspe.c

qua_g6k.o : qua_g6k.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8kd.h tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) qua_g6k.c

qua_g8k.o : qua_g8k.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) qua_g8k.c

pwf.o : pwf.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) pwf.c

pwfe.o : pwfe.c typedef.h
	cc -c $(CC_OPT) pwfe.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

tabld8kd.o : tabld8kd.c typedef.h ld8k.h ld8kd.h\
    tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) tabld8kd.c

tab_ld8e.o : tab_ld8e.c typedef.h ld8k.h ld8e.h\
    tab_ld8e.h
	cc -c $(CC_OPT) tab_ld8e.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) taming.c

util.o : util.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) util.c

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c
