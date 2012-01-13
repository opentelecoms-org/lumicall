
# Define compiler options 

CC_OPT=-std # ANSI-C option for Digital CC compiler 

# Targets 

OBJETS = \
 decodcp.o decld8cp.o bitscp.o dacelpcp.o declagcp.o\
 degaincp.o dec_sid.o gainpred.o lpccp.o lpfunccp.o\
 lspdece.o lspgetqe.o post_pro.o pstcp.o p_parity.o\
 tab_dtx.o tabld8cp.o tab_ld8k.o taming.o track_pi.o\
 pred_lt3.o calcexc.o filtere.o phdisp.o bwfwfunc.o\
 qsidgain.o utilcp.o

# Generation of the executable

decodcp : $(OBJETS)
	cc -o decodcp $(OBJETS) -lm

# Compilations if necessary

decodcp.o : decodcp.c typedef.h ld8k.h ld8cp.h\
    dtx.h octet.h
	cc -c $(CC_OPT) decodcp.c

decld8cp.o : decld8cp.c typedef.h ld8k.h ld8cp.h\
    dtx.h sid.h
	cc -c $(CC_OPT) decld8cp.c

bitscp.o : bitscp.c typedef.h ld8k.h ld8cp.h\
    tab_ld8k.h tabld8cp.h vad.h dtx.h tab_dtx.h\
    octet.h
	cc -c $(CC_OPT) bitscp.c

dacelpcp.o : dacelpcp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) dacelpcp.c

declagcp.o : declagcp.c typedef.h ld8k.h ld8cp.h
	cc -c $(CC_OPT) declagcp.c

degaincp.o : degaincp.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) degaincp.c

dec_sid.o : dec_sid.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h vad.h dtx.h sid.h\
    tab_dtx.h
	cc -c $(CC_OPT) dec_sid.c

gainpred.o : gainpred.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) gainpred.c

lpccp.o : lpccp.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h tabld8cp.h
	cc -c $(CC_OPT) lpccp.c

lpfunccp.o : lpfunccp.c typedef.h ld8k.h tab_ld8k.h\
    ld8cp.h
	cc -c $(CC_OPT) lpfunccp.c

lspdece.o : lspdece.c typedef.h ld8k.h ld8cp.h\
    tab_ld8k.h tabld8cp.h
	cc -c $(CC_OPT) lspdece.c

lspgetqe.o : lspgetqe.c typedef.h ld8k.h ld8cp.h
	cc -c $(CC_OPT) lspgetqe.c

post_pro.o : post_pro.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) post_pro.c

pstcp.o : pstcp.c typedef.h ld8k.h ld8cp.h\
    tab_ld8k.h
	cc -c $(CC_OPT) pstcp.c

p_parity.o : p_parity.c typedef.h ld8k.h
	cc -c $(CC_OPT) p_parity.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	cc -c $(CC_OPT) tab_dtx.c

tabld8cp.o : tabld8cp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) tabld8cp.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h
	cc -c $(CC_OPT) tab_ld8k.c

taming.o : taming.c typedef.h ld8k.h
	cc -c $(CC_OPT) taming.c

track_pi.o : track_pi.c typedef.h ld8k.h ld8cp.h
	cc -c $(CC_OPT) track_pi.c

pred_lt3.o : pred_lt3.c typedef.h ld8k.h tab_ld8k.h
	cc -c $(CC_OPT) pred_lt3.c

calcexc.o : calcexc.c typedef.h ld8k.h ld8cp.h\
    dtx.h
	cc -c $(CC_OPT) calcexc.c

filtere.o : filtere.c typedef.h ld8k.h ld8cp.h
	cc -c $(CC_OPT) filtere.c

phdisp.o : phdisp.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) phdisp.c

bwfwfunc.o : bwfwfunc.c typedef.h ld8k.h ld8cp.h\
    tabld8cp.h
	cc -c $(CC_OPT) bwfwfunc.c

qsidgain.o : qsidgain.c typedef.h ld8k.h vad.h\
    dtx.h sid.h tab_dtx.h
	cc -c $(CC_OPT) qsidgain.c

utilcp.o : utilcp.c typedef.h ld8k.h
	cc -c $(CC_OPT) utilcp.c
