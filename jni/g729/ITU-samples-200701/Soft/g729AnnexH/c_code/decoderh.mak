
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 decoderh.o dec_ld8h.o bitsh.o bwfwfunc.o deacelph.o\
 decgainh.o declagh.o filtere.o lpce.o gainpred.o\
 lpcfunc.o lspdece.o lspgetqe.o oper_32b.o phdisp.o\
 post_pro.o pred_lt3.o pste.o p_parity.o tabld8kd.o\
 tab_ld8e.o tab_ld8k.o taming.o track_pi.o util.o\
 basic_op.o dspfunc.o

# Generation of the executable

decoderh : $(OBJETS)
	cc -o decoderh $(OBJETS) -lm

# Compilations if necessary

decoderh.o : decoderh.c typedef.h basic_op.h ld8k.h\
    ld8e.h ld8h.h
	cc -c $(CC_OPT) decoderh.c

dec_ld8h.o : dec_ld8h.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8e.h ld8h.h
	cc -c $(CC_OPT) dec_ld8h.c

bitsh.o : bitsh.c typedef.h ld8k.h ld8e.h\
    ld8kd.h ld8h.h tab_ld8k.h tab_ld8e.h tabld8kd.h
	cc -c $(CC_OPT) bitsh.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8e.h\
    basic_op.h oper_32b.h tab_ld8e.h
	cc -c $(CC_OPT) bwfwfunc.c

deacelph.o : deacelph.c typedef.h basic_op.h ld8k.h\
    ld8kd.h tabld8kd.h
	cc -c $(CC_OPT) deacelph.c

decgainh.o : decgainh.c typedef.h basic_op.h ld8k.h\
    ld8kd.h ld8h.h tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) decgainh.c

declagh.o : declagh.c typedef.h basic_op.h ld8k.h\
    ld8h.h
	cc -c $(CC_OPT) declagh.c

filtere.o : filtere.c typedef.h basic_op.h ld8k.h\
    ld8e.h
	cc -c $(CC_OPT) filtere.c

lpce.o : lpce.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h ld8e.h
	cc -c $(CC_OPT) lpce.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) gainpred.c

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

post_pro.o : post_pro.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) post_pro.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

pste.o : pste.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8e.h
	cc -c $(CC_OPT) pste.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

tabld8kd.o : tabld8kd.c typedef.h ld8k.h ld8kd.h\
    tab_ld8k.h tabld8kd.h
	cc -c $(CC_OPT) tabld8kd.c

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

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c
