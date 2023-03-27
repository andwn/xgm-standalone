    .include "macros.i"

/* Z80 */
EQU Z80Ram,     0xA00000   /* Where Z80 RAM starts */
EQU Z80BusReq,  0xA11100   /* Z80 bus request line */
EQU Z80Reset,   0xA11200   /* Z80 reset line */

/* Z80 Addresses to communicate with XGM driver */
EQU Z80DrvCommand, 0xA00100  /* XGM Command */
EQU Z80DrvStatus,  0xA00102  /* XGM Status */
EQU Z80DrvParams,  0xA00104  /* XGM Parameters */
EQU Z80SMPTable,   0xA01C00  /* XGM Sample List */

    .section .bss

VAR xgmTempo,    w, 1
VAR xgmTempoCnt, w, 1
VAR xgmTempoDef, w, 1

    .section .text

/* XGM Driver blob */
BIN z80_xgm,    "../z80-xgm.bin"
z80_xgm_end:

BIN stop_xgm,   "stop_xgm.bin"

z80_xgm_size:
        dc.w    z80_xgm_end-z80_xgm-1


FUNC xgm_init
        move.w  #0x000,(Z80Reset)
        move.w  #0x100,(Z80BusReq)
        move.w  #0x100,(Z80Reset)

        lea     (z80_xgm),a0
        lea     (Z80Ram),a1
        move.w  (z80_xgm_size),d0
    xgm_init_copy:
        move.b  (a0)+,(a1)+
        dbf     d0,xgm_init_copy


        lea     (Z80SMPTable),a1
        move.l  #BlankData,d0
        lsr.l   #8,d0
        move.b  d0,(a1)+
        lsr.w   #8,d0
        move.b  d0,(a1)+
        move.b  #1,(a1)+
        move.b  #0,(a1)

        move.w  #0x000,(Z80Reset)
        move.w  #20,d0
    xgm_init_wait:
        dbf     d0,xgm_init_wait

        move.w  #0x100,(Z80Reset)
        move.w  #0x000,(Z80BusReq)

        move.w  #60,xgmTempo
        move.w  #60,xgmTempoDef
        move.w  (VdpCtrl),d0
        andi.w  #1,d0
        beq.s   xgm_init_notpal
        move.w  #50,xgmTempoDef
    xgm_init_notpal:
        rts

FUNC xgm_music_play
        move.l  d2,-(sp)
        move.l  8(sp),a0
        lea     (Z80SMPTable+4),a1
        move.w  #0x3E,d0

        DisableInts
        PauseZ80

    xgm_music_play_smpcopy:
        move.b  (a0)+,d1                /* Copy sample address */
        lsl.w   #8,d1
        move.b  (a0)+,d1
        ror.w   #8,d1
        cmp.w   #0xFFFF,d1              /* Silent sample? */
        beq.s   xgm_music_play_smpnull
        move.l  8(sp),d2                /* Not blank sample, */
        lsr.l   #8,d2
        add.w   d2,d1                   /* Add xgc address and header size */
        addq.w  #1,d1
        move.b  d1,(a1)+
        lsr.w   #8,d1
        move.b  d1,(a1)+
        bra.s   xgm_music_play_notsmpnull
    xgm_music_play_smpnull:
        move.l  #BlankData,d1           /* Just copy the blank sample address */
        lsr.l   #8,d1
        move.b  d1,(a1)+
        lsr.w   #8,d1
        move.b  d1,(a1)+
    xgm_music_play_notsmpnull:
        move.b  (a0)+,(a1)+             /* Copy sample size */
        move.b  (a0)+,(a1)+
        dbf     d0,xgm_music_play_smpcopy

        move.l  8(sp),d0                /* Get music data address */
        add.l   #0x104,d0
        moveq.l #0,d1
        move.b  (a0)+,d1
        ror.l   #8,d1
        move.b  (a0),d1
        rol.l   #8,d1
        lsl.l   #8,d1
        add.l   d1,d0

        lea     (Z80DrvParams),a1       /* Set music data address */
        move.b  d0,(a1)+
        lsr.l   #8,d0
        move.b  d0,(a1)+
        lsr.l   #8,d0
        move.b  d0,(a1)+
        lsr.w   #8,d0
        move.b  d0,(a1)

        lea     (Z80DrvCommand),a1      /* Set music play command */
        move.b  (a1),d0
        or.b    #0x40,d0
        move.b  d0,(a1)

        lea     (Z80DrvParams+0xF),a1     /* Clear pending frame */
        move.b  #0,(a1)

        ResumeZ80
        EnableInts

        move.l  (sp)+,d2
        rts

FUNC xgm_music_pause
        DisableInts
        PauseZ80

        lea     (Z80DrvCommand),a0      /* Set music pause command */
        move.b  (a0),d0
        or.b    #0x10,d0
        move.b  d0,(a0)

        ResumeZ80
        EnableInts
        rts

FUNC xgm_pcm_set
        DisableInts
        PauseZ80

        lea     (Z80SMPTable),a1    /* a1 = Z80SMPTable + (id << 2) */

        moveq.l #0,d0
#        move.b  5(sp),d0            /* id (-mshort) */
        move.b  7(sp),d0            /* id */
        lsl.w   #2,d0
        add.l   d0,a1

#        move.l  6(sp),d0            /* copy sample address (-mshort) */
        move.l  8(sp),d0            /* copy sample address */
        lsr.l   #8,d0
        move.b  d0,(a1)+
        lsr.w   #8,d0
        move.b  d0,(a1)+

#        move.l  10(sp),d0           /* copy length (-mshort) */
        move.l  12(sp),d0           /* copy length */
        lsr.l   #8,d0
        move.b  d0,(a1)+
        lsr.w   #8,d0
        move.b  d0,(a1)+

        ResumeZ80
        EnableInts
        rts

FUNC xgm_pcm_play
        DisableInts
        PauseZ80

#        move.w  8(sp),d0	    /* channel (-mshort) */
        move.w  14(sp),d0	   /* channel */
        move.w  d0,d1
        add.w   d1,d1
        lea     (Z80DrvParams+4),a0
        add.w   d1,a0
#        move.b  7(sp),(a0)+     /* priority (-mshort) */
        move.b  11(sp),(a0)+    /* priority */
#        move.b  5(sp),(a0)+     /* id (-mshort) */
        move.b  7(sp),(a0)+     /* id */

        moveq   #1,d1
        lsl.w   d0,d1
        lea     (Z80DrvCommand),a0
        move.b  (a0),d0
        or.b    d1,d0
        move.b  d0,(a0)

        ResumeZ80
        EnableInts
        rts

FUNC xgm_vblank_process
		/* XGM VBlank Process */
		lea    (Z80BusReq),a0           /* a0 = Z80 Halt Port */
		lea    (Z80Reset),a1            /* a1 = Z80 Reset Port */
		clr.w  d1					    /* num = 0 */
		move.w xgmTempoCnt,d0		    /* cnt = xgmTempoCnt */
		bgt.s  XGM_CntSkip				/* cnt > 0 */
    XGM_CntLoop:
		addq.w #1,d1				    /* num++ */
		add.w  xgmTempoDef,d0		    /* cnt += xgmTempoDef */
		ble.s  XGM_CntLoop				/* cnt <= 0 */
    XGM_CntSkip:
		sub.w  xgmTempo,d0				/* xgmTempoCnt = cnt - xgmTempo */
		move.w d0,xgmTempoCnt
    XGM_WhileTRUE:
		move.w #0x0100,(a0)			    /* Take Z80 bus and reset */
		move.w #0x0100,(a1)
    XGM_WaitZ80Get:
		move.w (a0),d0				    /* Wait for bus to be taken */
		btst   #8,d0
		bne.s  XGM_WaitZ80Get
		move.b (Z80DrvParams+0xE),d0	/* if (!z80_drv_params[0x0E]) break */
		beq.s  XGM_VBlankEnd
		move.w #0,(a0)					/* Release Z80 bus */
		movm.l d0-d3,-(sp)		        /* Wait about 80 cycles */
		movm.l (sp)+,d0-d3
    XGM_WaitZ80Rel:
		move.w (a0),d0				    /* Wait for bus to be released */
		btst   #8,d0
		beq.s  XGM_WaitZ80Rel
		bra.s  XGM_WhileTRUE
    XGM_VBlankEnd:
		move.b (Z80DrvParams+0xF),d0	/* z80_drv_params[0x0F] += num */
		add.b  d0,d1
		move.b d1,(Z80DrvParams+0xF)
		move.w #0,(a0)					/* Release Z80 bus */
		rts
