/*
	draw -- draw things with mouse (from DMD programming class)

	last edit:	85/07/15	D A Gwyn
*/
static char	SCCSID[] = "@(#)draw.c	1.1";

#include	<blit.h>
/* #include	<font.h>	/* not yet implemented */

static char	*menutxt[] =
	{
	"doodle",
	"line",
	"box",
	"rectangle",
	"triangle",
	"ellipse",
	"elliptical disk",
	"texture rect",
	"clear rect",
	"wipe layer",
	"quit",
	NULL
	};

static Menu	menu = { menutxt, };

static Texture	grey =
	{
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa,
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa,
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa,
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa,
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa,
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa,
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa,
	0x55555555,	0xaaaaaaaa,	0x55555555,	0xaaaaaaaa
	};

static void	doodle(), drwlne(), drwbox(), drwrect(), drwtri(),
		drwell(), drwdsk(), drwbkg(), clrsec(), clrscr(),
		jbxx(), jbkg(), jclr(), jrect(), jell(), jdsk();

/*ARGSUSED*/
main( argc, argv )
	int	argc;
	char	*argv[];
	{
	request( MOUSE );

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn3() )
			sleep( 2 );

		switch( menuhit( &menu, 3 ) )
			{
		case 0:			/* doodle */
			doodle();
			break;

		case 1:			/* line */
			drwlne();
			break;

		case 2:			/* box */
			drwbox();
			break;

		case 3:			/* rectangle */
			drwrect();
			break;

		case 4:			/* triangle */
			drwtri();
			break;

		case 5:			/* ellipse */
			drwell();
			break;

		case 6:			/* elliptical disk */
			drwdsk();
			break;

		case 7:			/* texture rect */
			drwbkg();
			break;

		case 8:			/* clear rect */
			clrsec();
			break;

		case 9:			/* wipe layer */
			clrscr();
			break;

		case 10:		/* quit */
			exit();

/*		default:
			break;		/* no selection (-1) */
			}
		}
	}


static void
doodle()
	{
	Point	current, next;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		current = mouse.xy;
		point( &display, current, F_XOR );

		cursinhibit();
		while ( wait( MOUSE ), bttn2() )
			{
			next = mouse.xy;
			if ( !eqpt( next, current ) )
				{
				/* use segment instead of jsegment
				   to avoid coincident pixel problem */
				segment( &display, current, next, F_XOR );

				current = next;
				}

			sleep( 1 );
			}
		cursallow();
		}
	}


static void
drwlne()
	{
	Point	start, end;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		jmoveto( start = end = mouse.jxy );

		cursinhibit();
		while ( wait( MOUSE ), bttn2() )
			{
			Point	newend;

			newend = mouse.jxy;
			if ( !same( end, newend ) )
				{
				/* draw new line */
				jmoveto( start );
				jlineto( newend, F_XOR );

				/* erase previous line */
				jmoveto( start );
				jlineto( end, F_XOR );

				end = newend;
				}

			sleep( 1 );
			}
		cursallow();
		}
	}


static void
drwbox()
	{
	Point	ulc, lrc;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		jmoveto( ulc = lrc = mouse.jxy );

		cursinhibit();
		while ( wait( MOUSE ), bttn2() )
			{
			Point	newlrc;

			newlrc = mouse.jxy;
			if ( !same( lrc, newlrc ) )
				{
				/* draw new box */
				jbxx( ulc, newlrc );

				/* erase previous box */
				jbxx( ulc, lrc );

				lrc = newlrc;
				}

			sleep( 1 );
			}
		cursallow();
		}
	}


static void
drwrect()
	{
	Point	ulc, lrc;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		jmoveto( ulc = lrc = mouse.jxy );

		cursinhibit();
		while ( wait( MOUSE ), bttn2() )
			{
			Point	newlrc;

			newlrc = mouse.jxy;
			if ( !same( lrc, newlrc ) )
				{
				/* draw new rectangle */
				jrect( ulc, newlrc );

				/* erase previous rectangle */
				jrect( ulc, lrc );

				lrc = newlrc;
				}

			sleep( 1 );
			}
		cursallow();
		}
	}


static void
drwtri()
	{
	Point	a, b, c;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		jmoveto( a = b = mouse.jxy );

		cursinhibit();
		while ( wait( MOUSE ), bttn2() )
			{
			Point	newb;

			newb = mouse.jxy;
			if ( !same( b, newb ) )
				{
				/* draw new line */
				jmoveto( a );
				jlineto( newb, F_XOR );

				/* erase previous line */
				jmoveto( a );
				jlineto( b, F_XOR );

				b = newb;
				}

			sleep( 1 );
			}

		c = b;

		while ( wait( MOUSE ), !bttn2() )
			sleep( 2 );

		while ( wait( MOUSE ), bttn2() )
			{
			Point	newc;

			newc = mouse.jxy;
			if ( !same( c, newc ) )
				{
				/* draw new line */
				jmoveto( b );
				jlineto( newc, F_XOR );

				/* erase previous line */
				jmoveto( b );
				jlineto( c, F_XOR );

				c = newc;
				}

			sleep( 1 );
			}
		cursallow();

		jlineto( a, F_XOR );
		}
	}


static void
drwell()
	{
	Point	mid, diag;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		mid = diag = mouse.jxy;
		jell( mid, diag );

		while ( wait( MOUSE ), bttn2() )
			{
			Point	newdiag;

			newdiag = mouse.jxy;
			if ( !same( diag, newdiag ) )
				{
				/* draw new ellipse */
				jell( mid, newdiag );

				/* erase previous ellipse */
				jell( mid, diag );

				diag = newdiag;
				}

			sleep( 1 );
			}
		}
	}


static void
drwdsk()
	{
	Point	mid, diag;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		mid = diag = mouse.jxy;
		jdsk( mid, diag );

		while ( wait( MOUSE ), bttn2() )
			{
			Point	newdiag;

			newdiag = mouse.jxy;
			if ( !same( diag, newdiag ) )
				{
				/* draw new disk */
				jdsk( mid, newdiag );

				/* erase previous disk */
				jdsk( mid, diag );

				diag = newdiag;
				}

			sleep( 1 );
			}
		}
	}


static void
drwbkg()
	{
	Point	ulc, lrc;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		jmoveto( ulc = lrc = mouse.jxy );

		cursinhibit();
		while ( wait( MOUSE ), bttn2() )
			{
			Point	newlrc;

			newlrc = mouse.jxy;
			if ( !same( lrc, newlrc ) )
				{
				/* draw new bkg rect */
				jbkg( ulc, newlrc, &grey );

				/* erase previous bkg rect */
				jbkg( ulc, lrc, &grey );

				lrc = newlrc;
				}

			sleep( 1 );
			}
		cursallow();
		}
	}


static void
clrsec()
	{
	Point	ulc, lrc;

	for ( ; ; )
		{
		while ( wait( MOUSE ), !bttn2() )
			{
			if ( bttn13() )
				{
				while ( wait( MOUSE ), bttn13() )
					sleep( 2 );

				return;
				}

			sleep( 2 );
			}

		jmoveto( ulc = lrc = mouse.jxy );

		cursinhibit();
		while ( wait( MOUSE ), bttn2() )
			{
			Point	newlrc;

			newlrc = mouse.jxy;
			if ( !same( lrc, newlrc ) )
				{
				/* draw new rectangle */
				jrect( ulc, newlrc );

				/* erase previous rectangle */
				jrect( ulc, lrc );

				lrc = newlrc;
				}

			sleep( 1 );
			}
		cursallow();

		/* erase rectangle */
		jrect( ulc, lrc );

		/* clear section */
		jclr( ulc, lrc );
		}
	}


static void
clrscr()
	{
	cursinhibit();			/* avoid mouse track */
	jrectf( Jrect, F_CLR );
	cursallow();
	}


static void
jbxx( ulc, lrc )
	Point	ulc, lrc;
	{
	int	l = ulc.x < lrc.x ? ulc.x : lrc.x;
	int	r = ulc.x > lrc.x ? ulc.x : lrc.x;
	int	t = ulc.y < lrc.y ? ulc.y : lrc.y;
	int	b = ulc.y > lrc.y ? ulc.y : lrc.y;

	jrectf( Rect( l, t, r, b ), F_XOR );
	}


static void
jbkg( ulc, lrc, tex )
	Point	ulc, lrc;
	Texture	*tex;
	{
	int	l = ulc.x < lrc.x ? ulc.x : lrc.x;
	int	r = ulc.x > lrc.x ? ulc.x : lrc.x;
	int	t = ulc.y < lrc.y ? ulc.y : lrc.y;
	int	b = ulc.y > lrc.y ? ulc.y : lrc.y;

	jtexture( Rect( l, t, r, b ), tex, F_XOR );
	}


static void
jclr( ulc, lrc )
	Point	ulc, lrc;
	{
	int	l = ulc.x < lrc.x ? ulc.x : lrc.x;
	int	r = ulc.x > lrc.x ? ulc.x : lrc.x;
	int	t = ulc.y < lrc.y ? ulc.y : lrc.y;
	int	b = ulc.y > lrc.y ? ulc.y : lrc.y;

	cursinhibit();			/* avoid mouse track */
	jrectf( Rect( l, t, r, b ), F_CLR );
	cursallow();
	}


static void
jrect( ulc, lrc )
	Point	ulc, lrc;
	{
	jsegment( ulc, Pt( ulc.x, lrc.y ), F_XOR );
	jsegment( Pt( ulc.x, lrc.y ), lrc, F_XOR );
	jsegment( lrc, Pt( lrc.x, ulc.y ), F_XOR );
	jsegment( Pt( lrc.x, ulc.y ), ulc, F_XOR );
	}


static void
jell( mid, box )
	Point	mid, box;
	{
	int	a = box.x - mid.x;
	int	b = box.y - mid.y;

	if ( a < 0 )
		a = -a;
	if ( a < 10 )			/* avoid library bug */
		a = 10;

	if ( b < 0 )
		b = -b;
	if ( b < 10 )			/* avoid library bug */
		b = 10;

	jellipse( mid, a, b, F_XOR );
	}


static void
jdsk( mid, box )
	Point	mid, box;
	{
	int	a = box.x - mid.x;
	int	b = box.y - mid.y;

	if ( a < 0 )
		a = -a;
	if ( a < 10 )			/* avoid library bug */
		a = 10;

	if ( b < 0 )
		b = -b;
	if ( b < 10 )			/* avoid library bug */
		b = 10;

	jeldisc( mid, a, b, F_XOR );
	}


static int
same( a, b )
	Point	a, b;
	{
	return eqpt( transform( a ), transform( b ) );
	}
