# G.729A with ANNEX B   Version 1.5    Last modified: October 2006 

# makefile for ANSI-C version of G.729A with Annex B.
# NOTE: Edit these options to reflect your particular system

# Options for GCC C compiler
CC= gcc
CFLAGS = -O2 -Wall

# macro noms des objets
OBJETS = \
 acelp_ca.o basic_op.o bits.o cod_ld8a.o coder.o\
 dspfunc.o filter.o gainpred.o lpc.o lpcfunc.o\
 lspgetq.o oper_32b.o p_parity.o pitch_a.o pre_proc.o\
 pred_lt3.o qua_gain.o qua_lsp.o tab_ld8a.o util.o\
 taming.o cor_func.o vad.o tab_dtx.o dtx.o\
 qsidgain.o qsidlsf.o calcexc.o dec_sid.o

# edition de liens
coder : $(OBJETS)
	$(CC) -o coder $(OBJETS) -lm

# Compilations si changements

acelp_ca.o : acelp_ca.c typedef.h basic_op.h ld8a.h
	$(CC) $(CFLAGS) -c acelp_ca.c

basic_op.o : basic_op.c typedef.h basic_op.h
	$(CC) $(CFLAGS) -c basic_op.c

bits.o : bits.c typedef.h ld8a.h tab_ld8a.h\
    vad.h dtx.h tab_dtx.h octet.h
	$(CC) $(CFLAGS) -c bits.c

cod_ld8a.o : cod_ld8a.c typedef.h basic_op.h ld8a.h\
    vad.h dtx.h sid.h
	$(CC) $(CFLAGS) -c cod_ld8a.c

coder.o : coder.c typedef.h basic_op.h ld8a.h\
    dtx.h octet.h
	$(CC) $(CFLAGS) -c coder.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8a.h\
    tab_ld8a.h
	$(CC) $(CFLAGS) -c dspfunc.c

filter.o : filter.c typedef.h basic_op.h ld8a.h
	$(CC) $(CFLAGS) -c filter.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8a.h\
    tab_ld8a.h oper_32b.h
	$(CC) $(CFLAGS) -c gainpred.c

lpc.o : lpc.c typedef.h basic_op.h oper_32b.h\
    ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c lpc.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c lpcfunc.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8a.h
	$(CC) $(CFLAGS) -c lspgetq.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	$(CC) $(CFLAGS) -c oper_32b.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8a.h
	$(CC) $(CFLAGS) -c p_parity.c

pitch_a.o : pitch_a.c typedef.h basic_op.h oper_32b.h\
    ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c pitch_a.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h\
    ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c pre_proc.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8a.h\
    tab_ld8a.h
	$(CC) $(CFLAGS) -c pred_lt3.c

qua_gain.o : qua_gain.c typedef.h basic_op.h oper_32b.h\
    ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c qua_gain.c

qua_lsp.o : qua_lsp.c typedef.h basic_op.h ld8a.h\
    tab_ld8a.h
	$(CC) $(CFLAGS) -c qua_lsp.c

tab_ld8a.o : tab_ld8a.c typedef.h ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c tab_ld8a.c

util.o : util.c typedef.h basic_op.h ld8a.h
	$(CC) $(CFLAGS) -c util.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8a.h tab_ld8a.h
	$(CC) $(CFLAGS) -c taming.c

cor_func.o : cor_func.c typedef.h basic_op.h ld8a.h
	$(CC) $(CFLAGS) -c cor_func.c

vad.o : vad.c typedef.h ld8a.h basic_op.h\
    oper_32b.h tab_ld8a.h vad.h dtx.h tab_dtx.h
	$(CC) $(CFLAGS) -c vad.c

tab_dtx.o : tab_dtx.c typedef.h ld8a.h vad.h\
    dtx.h tab_dtx.h
	$(CC) $(CFLAGS) -c tab_dtx.c

dtx.o : dtx.c typedef.h basic_op.h ld8a.h\
    oper_32b.h tab_ld8a.h vad.h dtx.h tab_dtx.h\
    sid.h
	$(CC) $(CFLAGS) -c dtx.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8a.h vad.h dtx.h sid.h tab_dtx.h
	$(CC) $(CFLAGS) -c qsidgain.c

qsidlsf.o : qsidlsf.c typedef.h basic_op.h ld8a.h\
    tab_ld8a.h sid.h vad.h dtx.h tab_dtx.h
	$(CC) $(CFLAGS) -c qsidlsf.c

calcexc.o : calcexc.c typedef.h ld8a.h dtx.h\
    basic_op.h oper_32b.h
	$(CC) $(CFLAGS) -c calcexc.c

dec_sid.o : dec_sid.c typedef.h ld8a.h tab_ld8a.h\
    basic_op.h vad.h dtx.h sid.h tab_dtx.h
	$(CC) $(CFLAGS) -c dec_sid.c
