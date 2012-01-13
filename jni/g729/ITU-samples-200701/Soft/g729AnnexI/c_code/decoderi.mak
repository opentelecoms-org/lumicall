
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 decoderi.o bitscp.o lpccp.o track_pi.o bwfwfunc.o\
 filtere.o lspgetqe.o lspdece.o pstcp.o utilcp.o\
 pred_lt3.o oper_32b.o basic_op.o dspfunc.o lpcfunc.o\
 post_pro.o p_parity.o tab_ld8k.o declagcp.o gainpred.o\
 dec_sid.o degaincp.o dacelpcp.o phdisp.o tab_dtx.o\
 calcexc.o taming.o qsidgain.o decld8cp.o tabld8cp.o

# Generation of the executable

decoderi : $(OBJETS)
	cc -o decoderi $(OBJETS) -lm

# Compilations if necessary

decoderi.o : decoderi.c typedef.h basic_op.h ld8k.h\
    ld8cp.h dtx.h octet.h
	cc -c $(CC_OPT) decoderi.c

bitscp.o : bitscp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h tab_ld8k.h octet.h
	cc -c $(CC_OPT) bitscp.c

lpccp.o : lpccp.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) lpccp.c

track_pi.o : track_pi.c typedef.h basic_op.h ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) track_pi.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8cp.h\
    basic_op.h oper_32b.h tabld8cp.h
	cc -c $(CC_OPT) bwfwfunc.c

filtere.o : filtere.c typedef.h basic_op.h ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) filtere.c

lspgetqe.o : lspgetqe.c typedef.h basic_op.h ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) lspgetqe.c

lspdece.o : lspdece.c typedef.h basic_op.h ld8k.h\
    ld8cp.h tab_ld8k.h tabld8cp.h
	cc -c $(CC_OPT) lspdece.c

pstcp.o : pstcp.c typedef.h basic_op.h oper_32b.h\
    ld8k.h ld8cp.h
	cc -c $(CC_OPT) pstcp.c

utilcp.o : utilcp.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) utilcp.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) oper_32b.c

basic_op.o : basic_op.c typedef.h basic_op.h
	cc -c $(CC_OPT) basic_op.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	cc -c $(CC_OPT) dspfunc.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) lpcfunc.c

post_pro.o : post_pro.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) post_pro.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

declagcp.o : declagcp.c typedef.h basic_op.h ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) declagcp.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	cc -c $(CC_OPT) gainpred.c

dec_sid.o : dec_sid.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h basic_op.h vad.h dtx.h\
    sid.h tab_dtx.h
	cc -c $(CC_OPT) dec_sid.c

degaincp.o : degaincp.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) degaincp.c

dacelpcp.o : dacelpcp.c typedef.h basic_op.h ld8k.h\
    ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) dacelpcp.c

phdisp.o : phdisp.c typedef.h basic_op.h ld8k.h\
    ld8cp.h tab_ld8k.h tabld8cp.h
	cc -c $(CC_OPT) phdisp.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	cc -c $(CC_OPT) tab_dtx.c

calcexc.o : calcexc.c typedef.h ld8k.h ld8cp.h\
    dtx.h basic_op.h oper_32b.h
	cc -c $(CC_OPT) calcexc.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) taming.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

decld8cp.o : decld8cp.c typedef.h basic_op.h ld8k.h\
    ld8cp.h dtx.h sid.h
	cc -c $(CC_OPT) decld8cp.c

tabld8cp.o : tabld8cp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) tabld8cp.c
