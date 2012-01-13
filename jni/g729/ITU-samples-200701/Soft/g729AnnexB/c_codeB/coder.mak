# G.729 with ANNEX B   Version 1.5    Last modified: October 2006 

# makefile for ANSI-C version of G.729 Annex B.
# NOTE: Edit these options to reflect your particular system

# Options for GCC C compiler
CC= gcc
CFLAGS = -O2 -Wall

# macro noms des objets
OBJETS = \
 acelp_co.o basic_op.o bits.o cod_ld8k.o coder.o\
 dspfunc.o filter.o gainpred.o lpc.o lpcfunc.o\
 lspgetq.o oper_32b.o p_parity.o pitch.o pre_proc.o\
 pred_lt3.o pwf.o qua_gain.o qua_lsp.o tab_ld8k.o\
 util.o taming.o vad.o dtx.o qsidgain.o\
 qsidlsf.o calcexc.o tab_dtx.o dec_sid.o

# edition de liens
coder : $(OBJETS)
	$(CC) -o coder $(OBJETS) -lm

# Compilations si changements

acelp_co.o : acelp_co.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c acelp_co.c

basic_op.o : basic_op.c typedef.h basic_op.h
	$(CC) $(CFLAGS) -c basic_op.c

bits.o : bits.c typedef.h ld8k.h tab_ld8k.h\
    vad.h dtx.h tab_dtx.h octet.h
	$(CC) $(CFLAGS) -c bits.c

cod_ld8k.o : cod_ld8k.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h vad.h dtx.h sid.h
	$(CC) $(CFLAGS) -c cod_ld8k.c

coder.o : coder.c typedef.h basic_op.h ld8k.h\
    dtx.h octet.h
	$(CC) $(CFLAGS) -c coder.c

dspfunc.o : dspfunc.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) $(CFLAGS) -c dspfunc.c

filter.o : filter.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c filter.c

gainpred.o : gainpred.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h oper_32b.h
	$(CC) $(CFLAGS) -c gainpred.c

lpc.o : lpc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c lpc.c

lpcfunc.o : lpcfunc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c lpcfunc.c

lspgetq.o : lspgetq.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c lspgetq.c

oper_32b.o : oper_32b.c typedef.h basic_op.h oper_32b.h
	$(CC) $(CFLAGS) -c oper_32b.c

p_parity.o : p_parity.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c p_parity.c

pitch.o : pitch.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c pitch.c

pre_proc.o : pre_proc.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c pre_proc.c

pred_lt3.o : pred_lt3.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) $(CFLAGS) -c pred_lt3.c

pwf.o : pwf.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c pwf.c

qua_gain.o : qua_gain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c qua_gain.c

qua_lsp.o : qua_lsp.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h
	$(CC) $(CFLAGS) -c qua_lsp.c

tab_ld8k.o : tab_ld8k.c typedef.h ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c tab_ld8k.c

util.o : util.c typedef.h basic_op.h ld8k.h
	$(CC) $(CFLAGS) -c util.c

taming.o : taming.c typedef.h basic_op.h oper_32b.h\
    ld8k.h tab_ld8k.h
	$(CC) $(CFLAGS) -c taming.c

vad.o : vad.c typedef.h ld8k.h basic_op.h\
    oper_32b.h tab_ld8k.h vad.h dtx.h tab_dtx.h
	$(CC) $(CFLAGS) -c vad.c

dtx.o : dtx.c typedef.h basic_op.h ld8k.h\
    oper_32b.h tab_ld8k.h vad.h dtx.h tab_dtx.h\
    sid.h
	$(CC) $(CFLAGS) -c dtx.c

qsidgain.o : qsidgain.c typedef.h basic_op.h oper_32b.h\
    ld8k.h vad.h dtx.h sid.h tab_dtx.h
	$(CC) $(CFLAGS) -c qsidgain.c

qsidlsf.o : qsidlsf.c typedef.h basic_op.h ld8k.h\
    tab_ld8k.h sid.h vad.h dtx.h tab_dtx.h
	$(CC) $(CFLAGS) -c qsidlsf.c

calcexc.o : calcexc.c typedef.h ld8k.h dtx.h\
    basic_op.h oper_32b.h
	$(CC) $(CFLAGS) -c calcexc.c

tab_dtx.o : tab_dtx.c typedef.h ld8k.h vad.h\
    dtx.h tab_dtx.h
	$(CC) $(CFLAGS) -c tab_dtx.c

dec_sid.o : dec_sid.c typedef.h ld8k.h tab_ld8k.h\
    basic_op.h vad.h dtx.h sid.h tab_dtx.h
	$(CC) $(CFLAGS) -c dec_sid.c
