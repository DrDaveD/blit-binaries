/*
	dmdp -- public-domain implementation of DMD printer program

	last edit:	88/06/01	D A Gwyn

	This code was produced from scratch to mimic the behavior of
	the real "dmdp" program as described in the AT&T 5620 DMD
	User Guide and distributed with Release 2.0 of the DMD
	software, which I did not have at the time.  This emulation
	has a few shortcomings, but it served our needs until the
	official "dmdp" program became available.  It also works with
	the new model 630 terminal, for which no official "dmdp" yet
	exists.

	The only printer types supported at present are the Teletype
	5310 and 5320 (later versions), the H-P 2225 ThinkJet, and the
	DEC LA50.  For the 5620, the ThinkJet must have switch 5 UP
	(Alternate mode); for the 630, set switch 5 DOWN (HP mode).

	If you're using the 5620 Release 1.3 layersys or a 630, Host
	mode will not work for an arbitrary layer, just for the "dmdp"
	layer itself.  (The 630 has built-in transparent printer
	support.)  5620 DMD Release 2.0 software supports arbitary
	layers for Host mode.

	Note:  During printing, you may have to hold down a mouse
	button for an inordinate amount of time before "dmdp" will
	acknowledge it.

	If you have improvements, please mail them to: Gwyn@BRL.MIL
*/
#ifndef lint
static char	SCCSid[] = "@(#)dmdp.c	1.4";
#endif

#define	MPXTERM	1			/* enable process control definitions */

#ifdef DMD630

#include	<dmd.h>
#include	<font.h>
#include	<dmdproc.h>
#include	<5620.h>		/* easier than thinking about it */
#include	<object.h>

#else	/* !DMD630 */			/* model 5620 assumed */

#include	<blit.h>		/* old name for <dmd.h> */
#include	<font.h>
#include	<jerqproc.h>		/* old name for <dmdproc.h> */

#ifdef	Ishort				/* DMD Release 2.0 software */

#define	point2layer
#define	whichproc
#include	<pandora.h>

#else					/* DMD Release 1.2, 1.3 */

#define	point2layer	debug
#undef psendchar			/* wrong in DMD Release 1.2 */
#define	psendchar(c)	Iint(192)(c)	/* print one character */

static Bitmap	physical =		/* actual screen display bitmap */
	{
	(Word *)0x700000L,		/* DMD screen image base address */
	(XMAX + 31) / 32,		/* bitmap width in 32-bit words */
	0, 0, XMAX, YMAX		/* coords of full-screen rectangle */
	};

#endif

#ifndef PSEND				/* not defined in DMD Release 1.2 */
#define	PSEND	64			/* printer port resource */
#endif

#endif	/* DMD630 */

typedef int	bool;
#define	false	0
#define	true	1

#define	MARGIN	2			/* space around image in layer */
#define	CHARHT	defont.height

static Bitmap	*mb;			/* bitmap for scan buffer */
static int	interlace;		/* interlace factor (normally 1 or 2) */
#define	MAX_INTERLACE	2
static int	w;			/* width of data for one byte */

/* printer-specific data: */

typedef struct
	{
	char	*pname;			/* name of printer */
	char	*pinit;			/* graphics initiation string */
	char	*pexit;			/* graphics termination string */
	char	*bol;			/* beginning-of-scan string */
	char	*eol;			/* end-of-scan string */
	char	*ctpfx;			/* byte group count prefix */
	char	*ctsfx;			/* byte group count suffix */
	int	ctmax;			/* max # bytes per group;
					   ctmax < 0 means "use ASCII digits" */
	int	gdelay;			/* time delay per byte group (ticks) */
	int	sdelay;			/* time delay per `scangrp' scans */
	int	ldelay;			/* time delay per line (ticks) */
	int	fdelay;			/* time delay per page (ticks) */
	short	scangrp;		/* (see `sdelay' above) */
	short	rows;			/* rasters / scan (1 => horiz chunks) */
	short	dots;			/* width of scan */
	short	chunk;			/* dot bits per byte */
	short	times;			/* # of times to send each plot byte */
	bool	oddpar;			/* transmit odd parity */
	bool	hifirst;		/* high-order bit leftmost or topmost */
	char	offset;			/* amount to add to binary byte */
	}	pinfo;			/* printer information */

static pinfo	pdata[] =
	{
#define	PT_TRANSPARENT	0		/* transparent attachment to layer */
	{	"transparent",
		"",
		"",
		"",
		"",
		"",
		"",
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		1,
#ifdef DMD630
		false,
#else
		true,			/* required by 5620 printer port */
#endif
		true,
		0
	},
#define	PT_5310		1		/* Teletype 5310 */
	{	"5310",
		"\r\033P\0351q",
		"$\033\\\n",
		"",
		"-",
		"",
		"",
		0,
		0,
		60,			/* determined empirically */
		15,			/* manual says 4 lines/sec */
		90,			/* determined empirically */
		1,
		12,
		1194,
		6,
		1,
		true,
		false,
		0x3F
	},
#define	PT_5320		2		/* Teletype 5320 */
	{	"5320",
		"\r\033P\0351q",
		"$\033\\\n",
		"",
		"-",
		"",
		"",
		0,
		0,
		60,			/* determined empirically */
		15,			/* manual says 4 lines/sec */
		90,			/* determined empirically */
		1,
		12,
		1974,
		6,
		1,
		true,
		false,
		0x3F
	},
#define	PT_LA50SQ	3		/* DEC LA50 "squeeze" , SW1-5 open
					   (supplied by Scott Mulligan) */
	{	"la50-sq",
		"\r\033Pq",
		"$\033\\\n",
		"$",
		"-",
		"",
		"",
		0,
		0,
		25,			/* determined empirically */
		15,			/* determined empirically */
		90,			/* determined empirically */
		1,
		6,
		1152,			/* 1:2 aspect ratio, yuck */
		6,
		1,
		false,
		false,
		0x3F
	},
#define	PT_LA50W	4		/* DEC LA50 "wide", SW1-5 closed
					   (supplied by Scott Mulligan) */
	{	"la50-w",
		"\r\033Pq",
		"$\033\\\n",
		"$",
		"-",
		"",
		"",
		0,
		0,
		25,			/* determined empirically */
		15,			/* determined empirically */
		90,			/* determined empirically */
		1,
		6,
		720,			/* 4:5 aspect ratio */
		6,
		2,
		false,
		false,
		0x3F
	},
#define	PT_LA50		5		/* DEC LA50 default, SW1-5 open
					   (supplied by Scott Mulligan) */
	{	"la50",
		"\r\033Pq",
		"$\033\\\n",
		"$",
		"-",
		"",
		"",
		0,
		0,
		25,			/* determined empirically */
		15,			/* determined empirically */
		90,			/* determined empirically */
		1,
		6,
		576,			/* 1:1 aspect ratio */
		6,
		2,
		false,
		false,
			0x3F
	},
#define	PT_THINKJET	6		/* H-P 2225 ThinkJet */
	{	"thinkjet",
#ifdef DMD630				/* 19.2K bps, 8 bits permits HP mode */
		"\r\033*r640S\033*rA",
		"\033*rB",
		"",
		"",
		"\033*b",
		"W",
		-640,
		0,
		0,			/* determined empirically */
		30,			/* determined empirically */
		300,			/* determined empirically */
		12,
		1,
		640,
		8,
		1,
		false,
		true,
		0
#else	/* !DMD630 */			/* 7-bit path forces Alternate mode */
		"\r\0331",
		"\0332\n",
		"",
		"\r\n",
		"\033K",
		"\200",
		127,
		40,			/* determined empirically */
		120,			/* determined empirically */
		30,			/* determined empirically */
		300,			/* determined empirically */
		1,
		7,
		640,
		7,
		1,
		true,			/* required by 5620 printer port */
		true,
		0
#endif	/* DMD630 */
	},
/* add info for your favorite printer just above this line and mail me a copy */
	};
#define	N_PTYPES	(sizeof pdata / sizeof pdata[0])

static int	ptype = PT_5320;	/* selected printer type */

static bool	attached = false;	/* Host mode, attached to process I/O */
static bool	printing = false;	/* printer busy */
static bool	frozen = false;		/* control process suspended */

static struct Proc	*pp = 0;	/* layer being printed */
static int		pstate;		/* prior state of suspended process */
static Bitmap		*bb;		/* bitmap being printed */
static Rectangle	rr;		/* remaining rectangle to be printed */

static Texture16	printer =	/* printer icon */
	{
	0x01FF, 0x0201, 0x02FA, 0x0402,
	0x05F4, 0x0804, 0x0BEF, 0x100B,
	0x3FFD, 0x4009, 0xFFF1, 0x8011,
	0x8012, 0x8014, 0x8018, 0xFFF0
	};

static void	Attach(), Block(), Detach(), DoExit(), DoHost(), DoMain(),
		DoPCntl(), DoPrnt(), DoPrOn(), DoScreen(), DoSetUp(),
		Freeze(), Magnify(), Monitor(), PrintChar(), PrintNum(),
		PrintStr(), ShowOff(), Tell(), Unblock(), Unfreeze();

main( argc, argv )
	int	argc;
	char	**argv;
	{
#ifdef DMD630
	if ( argc > 2 )
		(void)local();		/* detach from host channel */

	(void)cache( "dmdp", 0 );
#endif

	/* process command line argument (printer type) */

	if ( argc > 1 )			/* user specified printer type */
		{
		for ( ptype = 0; ptype < N_PTYPES; ++ptype )
			if ( strcmp( argv[1], pdata[ptype].pname ) == 0 )
				break;

		if ( ptype == N_PTYPES )	/* not found */
			{
			Tell( "unknown printer type" );
			sleep( 300 );	/* allow time to read message */
			exit();
			}
		}
	/* else use default printer type */

	if ( pdata[ptype].rows > 1 )
		{			/* vertical chunks */
		interlace = pdata[ptype].rows / pdata[ptype].chunk;
		if ( interlace > MAX_INTERLACE )
			{
			Tell( "coding error: interlace" );
			sleep( 600 );	/* allow time to record message */
			exit();
			}
		w = 1;
		}
	else	{			/* horizontal chunks */
		interlace = 1;
		w = pdata[ptype].chunk;
		}

	request( MOUSE | ALARM | PSEND | RCV );

	(void)cursswitch( &printer );

	P->state |= RESHAPED;
	alarm( 1 );			/* force ShowOff */

	DoMain();

	/*NOTREACHED*/
	}

static void
DoMain()
	{
	static char	*maintext[] =
		{
		"Screen",
		"Host",
		"Exit",
		(char *)0
		};
	static Menu	main_menu = { maintext };
	register int	item;

	if ( ptype == PT_TRANSPARENT )
		main_menu.item = &maintext[1];	/* no Screen option */

	for ( ; ; )
		{
		Monitor( "Main Menu" );

		item = menuhit( &main_menu, 3 );
		if ( ptype == PT_TRANSPARENT && item >= 0 )
			++item;
		switch( item )
			{
		case 0:			/* Screen */
			DoScreen();				
			break;

		case 1:			/* Host */
			DoHost();
			break;

		case 2:			/* Exit */
			DoExit();
			break;
			}
		}
	}

static void
Monitor( s )				/* watch for mouse buttons & reshape */
	register char	*s;		/* state string */
	{
	Tell( s );

	for ( ; ; )
		{
		register int	got;

		got = wait( MOUSE | ALARM | RCV );

		if ( P->state & RESHAPED )
			{
			ShowOff( &printer );	/* display logo */
			Tell( s );

			P->state &= ~RESHAPED;
			}

		if ( got & MOUSE )
			{
			if ( bttn2() && !printing && ptype != PT_TRANSPARENT )
				{
				DoSetUp();	/* temporary diversion */

				Tell( s );
				}
			else if ( bttn3() )
				return;	/* something to do */
			}

		if ( got & RCV )	/* attached in Host mode, else junk */
			{
			register int	c = rcvchar();

			if ( attached && printing )
				PrintChar( c );	/* copy through */
			/* else discard */
			}

		if ( !(attached && printing) || (own() & RCV) == 0 )
			alarm( 60 );	/* periodically check for reshape */
		}
	}

static void
DoSetUp()
	{
	Tell( "Setup not implemented" );

	while ( bttn2() )		/* DEBUG */
		;			/* DEBUG */
	}

static void
DoScreen()
	{
	static char	*seltext[] =
		{
		"Select Layer",
		"Sweep Rectangle",
		"Whole Screen",
		"Main Menu",
		(char *)0
		};
	static Menu	sel_menu = { seltext };

	/* prepare scan buffer: */

	if ( (mb = balloc( Rect( 0, 0, pdata[ptype].dots, pdata[ptype].rows ) ))
	  == (Bitmap *)0
	   )	{
		Tell( "No Memory in DMD" );
		sleep( 300 );		/* allow time to read message */
		return;
		}

	for ( ; ; )
		{
		Monitor( "Select Print Area" );

		switch ( menuhit( &sel_menu, 3 ) )
			{
		case 0:			/* Select Layer */
			Tell( "Select Layer" );

			if ( (pp = point2layer()) == P )
				pp = (struct Proc *)0;

			(void)cursswitch( &printer );

			if ( pp == (struct Proc *)0 )
				break;

			bb = (Bitmap *)pp->layer;
			rr = bb->rect;

			Block();
			DoPrnt();
			Unblock();
			goto done;

		case 1:			/* Sweep Rectangle */
			Tell( "Sweep Rectangle" );

			bb = &physical;
			rr = getrect();

			if ( bttn12() )
				break;

			Freeze();	/* (too conservative) */
			DoPrnt();
			Unfreeze();
			goto done;

		case 2:			/* Whole Screen */
			bb = &physical;
			rr = Jrect;

			Freeze();
			DoPrnt();
			Unfreeze();
			goto done;

		case 3:			/* Main Menu */
    done:
			bfree( mb );
			return;
			}
		}
	}

static void
DoPrnt()				/* highlight rectangle and init print */
	{
	static char	*pontext[] =
		{
		"Print",
		"Main Menu",
		(char *)0
		};
	static Menu	pon_menu = { pontext };
	register short	xmax = rr.origin.x + pdata[ptype].dots;

	/* fudge to fit printer width, if necessary */

	if ( rr.corner.x > xmax )
		rr.corner.x = xmax;

	rectf( bb, rr, F_XOR );		/* highlight selected area */

	for ( ; ; )
		{
		Monitor( "Print Area Selected" );

		switch ( menuhit( &pon_menu, 3 ) )
			{
		case 0:			/* Print */
			DoPCntl();
			goto done;

		case 1:			/* Main Menu */
			goto done;
			}
		}

    done:
	rectf( bb, rr, F_XOR );		/* unhighlight remaining area */
	return;
	}

static void
DoPCntl()				/* printer control */
	{
	static char	*pctltext[] =
		{
		"Pause",
		"Main Menu",
		"Continue",
		(char *)0
		};
	static Menu	pctl_menu = { pctltext };
	short		cycle;		/* issue sdelay when cycle reaches 0 */

	printing = true;
	Tell( "Printing" );

	PrintStr( pdata[ptype].pinit );	/* print initiation string */

	for ( cycle = pdata[ptype].scangrp; ; )
		{
		if ( P->state & RESHAPED )
			{
			ShowOff( &printer );	/* display logo */
			Tell( "Printing" );

			P->state &= ~RESHAPED;
			}

		if ( own() & MOUSE && bttn3() )
			{
    menu:
			switch ( menuhit( &pctl_menu, 3 ) )
				{
			case 0:		/* Pause */
				printing = false;	/* allows setup (???) */
				Monitor( "Pause" );
				goto menu;

			case 1:		/* Main Menu */
				goto done;

			case 2:		/* Continue */
				printing = true;
				Tell( "Printing" );
				break;
				}
			}

		/* The following is not elegant, but I was in a hurry. */

		if ( printing )
			{		/* print next scan */
			register int	i;	/* interlace offset */
			Rectangle	rs;	/* strip of screen image */

			rs.origin = rr.origin;
			rs.corner.x = rr.corner.x;
			rs.corner.y = rr.origin.y + pdata[ptype].rows;
			if ( rs.corner.y > rr.corner.y )
				rs.corner.y = rr.corner.y;

			/* restore from reverse video */

			rectf( bb, rs, F_XOR );

			/* clear background (margin) in strip buffer */

			rectf( mb, mb->rect, F_CLR );

			/* move image strip into strip buffer */

			bitblt( bb, rs, mb, mb->rect.origin, F_STORE );

			PrintStr( pdata[ptype].bol );

			for ( i = 0; i < interlace; ++i )
				{
				register int	count;	/* byte group counter */
				register short	nextx;
				short		lastx =
						rs.corner.x - rs.origin.x;
				short		lasty =
						rs.corner.y - rs.origin.y;

				for ( count = nextx = 0; nextx < lastx;
				      nextx += w
				    )	{
					static union
						{
						Word	dummy[8*MAX_INTERLACE];
							/* for alignment */
						char	c[8 * MAX_INTERLACE]
							 [sizeof(Word)];
							/* c[][0] data */
						}	u;	/* init 0s */
					static Bitmap	cb =
						{
						u.dummy,	/* base */
						1,		/* width */
						0, 0, 8, 8 * MAX_INTERLACE,
								/* rect */
						(char *)0	/* _null */
						};
					register char	d;	/* accum bits */
					register int	j;	/* bit cntr */

					/* The following is for count-oriented
					   printers such as the H-P ThinkJet. */

					if ( count == 0
					  && pdata[ptype].ctmax != 0
					   )	{
						PrintStr( pdata[ptype].ctpfx );
						count = (lastx - nextx + w - 1)
							/ w;

						if ( pdata[ptype].ctmax < 0 )
							{
							if ( count >
							     -pdata[ptype].ctmax
							   )
								count =
								-pdata[ptype]
									.ctmax;

							PrintNum( count );
							}
						else	{
							if ( count >
							     pdata[ptype].ctmax
							   )
								count =
								pdata[ptype]
									.ctmax;

							PrintChar( count );
							}

						PrintStr( pdata[ptype].ctsfx );
						}

					/* get (interlaced) rasters for byte */

					d = 0;	/* init bit accumulator */

					if ( w > 1 )
						{	/* horizontal chunk */
						bitblt( mb, Rect( nextx, 0,
								  nextx + w, 1
								),
							&cb, Pt( 8 - w, 0 ),
							F_STORE
						      );

						if ( pdata[ptype].hifirst )
							d = u.c[0][0];
						else	{	/* rev. bits */
							register int	e =
									1 << w;

							for ( j = 0; j < w; ++j
							    )	{
								d <<= 1;
								e >>= 1;

								if ( (u.c[0][0]
								    & e
								     ) != 0
								   )
									d |= 1;
								}
							}
						}
					else	{	/* vertical chunk */

						/* fetch (offset) data chunk */

						bitblt( mb, Rect( nextx, i,
								  nextx + 1,
								  lasty
								),
							&cb, Pt( 0, 0 ),
							F_STORE
						      );

						/* convert to prt bit pattern */

						if ( pdata[ptype].hifirst )
							for ( j = 0;
							      j < pdata[ptype]
									.chunk;
							      ++j
							    )	{
								d <<= 1;

								if (
							u.c[j * interlace][0]
									!= 0
								   )
									d |= 1;
								}
						else	{
							j = pdata[ptype].chunk;
							while ( --j >= 0 )
								{
								d <<= 1;

								if (
							u.c[j * interlace][0]
									!= 0
								   )
									d |= 1;
								}
							}
						}

					for ( j = 0; j < pdata[ptype].times; ++j
					    )
						PrintChar( d
							 + pdata[ptype].offset
							 );

					if ( --count == 0 )
						if ( frozen )
							nap( pdata[ptype].gdelay
							   );
						else
							sleep( pdata[ptype]
									.gdelay
							     );
					}

				PrintStr( pdata[ptype].eol );

				if ( --cycle == 0 )
					{
					cycle = pdata[ptype].scangrp;

					if ( frozen )
						nap( pdata[ptype].sdelay );
					else
						sleep( pdata[ptype].sdelay );
					}
				}

			if ( (rr.origin.y = rs.corner.y) == rr.corner.y )
				{
    done:
				if ( cycle > 0 )	/* partial scan group */
					if ( frozen )
						nap( pdata[ptype].sdelay );
					else
						sleep( pdata[ptype].sdelay );

				PrintStr( pdata[ptype].pexit );
					/* print termination string */

				printing = false;
				return;	/* done printing, back to Main Menu */
				}
			}
		}
	}

static void
Block()					/* suspend layer process */
	{
	pstate = pp->state;		/* save state */
	pp->state &= ~(RUN | WAKEUP);
	}

static void
Unblock()				/* resume layer process */
	{
	pp->state |= pstate & (RUN | WAKEUP);
	}

static void
Freeze()				/* suspend control process */
	{
	frozen = true;
/*	setnorun( &proctab[CONTROL] );	*/
	}

static void
Unfreeze()				/* resume control process */
	{
	frozen = false;
/*	setrun( &proctab[CONTROL] );	*/
	}

static void
DoHost()
	{
	Tell( "Select Layer" );

	pp = point2layer();

	(void)cursswitch( &printer );
	if ( pp == (struct Proc *)0 )
		return;

	Attach();

	DoPrOn();

	Detach();

	return;
	}

static void
DoPrOn()
	{
	static char	*pontext[] =
		{
		"Printer On",		/* or "Printer Off" */
		"Main Menu",
		(char *)0
		};
	static Menu	pon_menu = { pontext };

	for ( ; ; )
		{
		Monitor( printing ? "Printer On" : "Printer Off" );
				/* actual printing done inside Monitor() */

		pontext[0] = printing ? "Printer Off" : "Printer On";
		switch ( menuhit( &pon_menu, 3 ) )
			{
		case 0:			/* Printer On/Off */
			if ( printing = !printing )
				/* flush all already-queued input */
				while ( own() & RCV )
					(void)rcvchar();
			break;

		case 1:			/* Main Menu */
			printing = false;
			return;
			}
		}
	}

static void
Attach()				/* attach to process I/O */
	{
	if ( pp != P )
		{
#ifdef PIPETO
		pp->state |= (whichproc( P->layer ) << 16) & PIPETO;
#else
		/* golly, I can't figure out how to tap the I/O stream */

		Tell( "Attach to own layer only" );
		sleep( 300 );		/* allow time to read message */
#endif
		}

	attached = true;
	}

static void
Detach()				/* detach from process I/O */
	{
	if ( pp != P )
		{
#ifdef PIPETO
		pp->state &= ~PIPETO;
#else
		/* I obviously can't figure out how to untap I/O, either */
#endif
		}

	attached = false;
	}

static void
DoExit()
	{
	static Texture16	sunset =
		{
		0x5006, 0xA819, 0x00A0, 0x04A0,
		0x049F, 0x12A4, 0x0808, 0x03E0,
		0x2412, 0x0808, 0x0808, 0x3FFF,
		0x3C1F, 0x7E7E, 0x783E, 0xFCFC,
		};

	(void)cursswitch( &sunset );

	P->state |= RESHAPED;
	alarm( 1 );			/* force display */

	for ( ; ; )
		{
		register int	got = wait( MOUSE | ALARM );

		if ( P->state & RESHAPED )
			{
			ShowOff( &sunset );	/* happy trails */
			Tell( "Exiting" );

			P->state &= ~RESHAPED;
			}

		if ( got & MOUSE && bttn123() )
			break;

		alarm( 60 );		/* periodically check for reshape */
		}

	while ( bttn3() )
		;

	if ( !bttn12() )
		exit();

	while ( bttn123() )
		;

	(void)cursswitch( &printer );
	ShowOff( &printer );
	Tell( "idle" );
	}

static void
ShowOff( tp )
	Texture16	*tp;		/* 16x16 template */
	{
	Point		fac;		/* magnification factor */
	Point		size;		/* magnified image size */
	register Bitmap	*b;		/* temporary source bitmap */

	rectf( &display, Drect, F_CLR );

	if ( (b = balloc( Rect( 0, 0, 16, 16 ) )) == (Bitmap *)0 )
		{
		string( &defont, "dmdp", &display, Pt( 3, 3 ), F_STORE );
		return;
		}

#ifdef DMD630
	texture( b, b->rect, tp, F_STORE );
#else
	texture16( b, b->rect, tp, F_STORE );
#endif

	size = sub( Drect.corner, Drect.origin );

	fac = sub( size, Pt( 2 * MARGIN, 3 * MARGIN + CHARHT ) );

	while ( fac.x > 2 * fac.y )
		fac.x /= 2;

	while ( fac.y > 2 * fac.x )
		fac.y /= 2;

	fac = div( fac, 16 );

	size = div( sub( size, mul( fac, 16 ) ), 2 );
	size.y -= (MARGIN + CHARHT) / 2;
	Magnify( b, b->rect, &display, add( Drect.origin, size ), fac );

	bfree( b );
	}

static void
Magnify( b, r, tb, p, fac )		/* adapted from "lens" */
	register Bitmap	*b, *tb;
	Rectangle	r;
	Point		p, fac;
	{
	register Bitmap	*stage;
	register int	i, shift;
	Point		d;
	Rectangle	s;

#if 0
	if ( fac.x < 1 || fac.y < 1 )	/* "can't happen" */
		return;
#endif

	d = sub( r.corner, r.origin );
	s.origin = p;
	s.corner = add( p, Pt( fac.x * d.x, fac.y * d.y ) );

	/* Copy source into origin of dest */

	bitblt( b, r, tb, p, F_STORE );

	/* Clear rest of dest */

	rectf( tb, Rect( s.origin.x + d.x, s.origin.y,
			 s.corner.x, s.corner.y
		       ),
	       F_CLR
	     );
	rectf( tb, Rect( s.origin.x, s.origin.y + d.y,
			 s.origin.x + d.x, s.corner.y
		       ),
	       F_CLR
	     );

	/* Now we expand in place */

	/* 1: expand horizontally */
	if ( fac.x > 1 )
		for( i = d.x - 1; i > 0; --i )
			{
			bitblt( tb, Rect( p.x + i, p.y, p.x + i + 1, p.y + d.y),
				tb, Pt( p.x + i * fac.x, p.y ), F_OR
			      );
			rectf( tb, Rect( p.x + i, p.y, p.x + i + 1, p.y + d.y ),
			       F_CLR
			     );
			}

	/* 2: smear horizontally */
	for( i = 1; i < fac.x; i *= 2 )
		{
		shift = min( i, fac.x - i );
		bitblt( tb, Rect( p.x, p.y, s.corner.x - shift, p.y + d.y ),
			tb, Pt( p.x + shift, p.y ), F_OR
		      );
		}

	/* 3: expand vertically */	
	if ( fac.y > 1 )
		for ( i = d.y - 1; i > 0; --i )
			{
			bitblt( tb, Rect( p.x, p.y + i, s.corner.x, p.y + i + 1
					),
				tb, Pt( p.x, p.y + i * fac.y ), F_OR
			      );
			rectf( tb, Rect( p.x, p.y + i, s.corner.x, p.y + i + 1
				       ),
			       F_CLR
			     );
			}

	/* 4: smear vertically */
	for ( i = 1; i < fac.y; i *= 2 )
		{
		shift = min( i, fac.y - i );
		bitblt( tb, Rect( p.x, p.y, s.corner.x, s.corner.y - shift ),
			tb, Pt( p.x, p.y + shift ), F_OR
		      );
		}	
	}

static void
Tell( s )				/* display message line */
	char	*s;			/* message */
	{
	rectf( &display, Rect( Drect.origin.x, Drect.corner.y - MARGIN - CHARHT,
			       Drect.corner.x, Drect.corner.y
			     ),
	       F_CLR
	     );

	(void)string( &defont, s, &display, Pt( Drect.origin.x + MARGIN,
						Drect.corner.y - MARGIN - CHARHT
					      ),
		      F_OR
		    );
	}

static void
PrintNum( n )
	register int	n;		/* assumed nonnegative */
	{
	register int	c = n % 10 + '0';

	n /= 10;			/* avoid 68000 SGS code bug */
	if ( n != 0 )
		PrintNum( n );

	PrintChar( c );
	}

static void
PrintStr( s )
	register char	*s;
	{
	for ( ; *s != '\0'; ++s )
		PrintChar( *s );
	}

static void
PrintChar( c )
	register int	c;
	{
	static char	parity[128] =	/* odd-parity bits for 7-bit data */
		{
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00,
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00,
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x00,	0x80,	0x80,	0x00,	0x80,	0x00,	0x00,	0x80,
		0x80,	0x00,	0x00,	0x80,	0x00,	0x80,	0x80,	0x00
		};

	c &= 0xFF;			/* outwit sign extension */

	if ( ptype != PT_TRANSPARENT )
		{
		if ( attached )		/* interpret control codes */
			{
			static int	col = 0;

			switch ( c )
				{
			case '\r':
			case '\f':
				col = 0;
				break;

			case '\n':
			case '\v':
			case '\007':	/* BEL */
				break;

			case '\b':
				if ( col > 0 )
					--col;
				break;

			case '\t':
				do
					PrintChar( ' ' );
				while ( col % 8 != 0 );
				return;

			default:
				if ( c < ' ' )
					return;
				break;
				}
			}

		if ( pdata[ptype].oddpar )
			{
			c &= 0x7F;
			c |= parity[c];	/* make odd parity */
			}
		}

	while ( psendchar( c ) == 0 )
		;

	if ( ptype != PT_TRANSPARENT && attached )	/* handle delays */
		{
		if ( pdata[ptype].oddpar )
			c &= 0x7F;	/* strip parity */

		switch ( c )
			{
		case '\v':
		case '\f':
			sleep( pdata[ptype].fdelay );
			break;

		case '\n':
			sleep( pdata[ptype].ldelay );
			break;
			}
		}
	}
