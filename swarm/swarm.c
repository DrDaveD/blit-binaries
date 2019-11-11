/*
	swarm -- public-domain implementation of a widget which chases the mouse
		around, for Blit-like terminals (5620 DMD and 630 MTG, at least)

	last edit:	91/06/10	D A Gwyn

	SCCS ID:	@(#)swarm.c	1.3

To compile:
	$ dmdcc -o swarm.m swarm.c	# -g -O also recommended

To run:
	$ dmdld swarm.m			# only useful in mpx (layers) mode
	$ dmdld swarm.m -r		# chase a randomly moving gnat
					# rather than the mouse cursor

To operate:
	Operates itself.  Typing "r" to the swarm layer will toggle the "-r"
	(random target) flag.  To terminate, select the "swarm" layer then type
	"q" (deleting the layer will leave dead gnats on the screen).  Works
	best in a small layer near a corner of the screen, so you can find it.

Credits:
	This program was inspired by the "sperm" program by Drew Olbrich
	of CMU.  However, the present code was written from scratch by
	Douglas A. Gwyn, who has placed it into the public domain.  David
	Connett (dcon@iwtng.att.com) added the "r" key mode toggling and
	cleanup upon layer deletion.

Bugs (insects, actually):
	Dead gnats are left behind not only when the "swarm layer" is deleted,
	but also whenever layer contents are updated while gnats are present.
	Due to the implementation technique (writing to the "physical" screen),
	this behavior would be hard to correct; thus, consider it a "feature".
	(If you really want to be annoyed, get my implementation of "crabs"!)
*/

#ifndef lint
static char	SCCS_ID[] = "@(#)swarm.c	1.3 91/06/10";	/* for "what" utility */
#endif

#ifndef STATIC
#define	STATIC	static			/* empty string when debugging */
#endif


#ifdef DMD630

#include	<dmd.h>
#include	<5620.h>		/* (easier than thinking about it) */

#else	/* !DMD630 */			/* model 5620 assumed */

#include	<blit.h>		/* old name for <dmd.h> */

#include	<sa.h>
#define	RealMouse	Smouse		/* what a kludge! */

#ifndef T_background
STATIC Texture16	T_background =		/* background texture */
	{
	0x1111, 0x4444, 0x1111, 0x4444,
	0x1111, 0x4444, 0x1111, 0x4444,
	0x1111, 0x4444, 0x1111, 0x4444,
	0x1111, 0x4444, 0x1111, 0x4444
	};
#endif

#ifndef physical
STATIC Bitmap	physical =		/* full screen definition */
	{
	(Word *)0x700000L,		/* DMD screen image base address */
	(XMAX + 31) / 32,		/* bitmap width in 32-bit Words */
	0, 0, XMAX, YMAX		/* screen rectangle within bitmap */
	};
#endif

#endif	/* DMD630 */


/* Tweakable parameters for process tuning: */

#define	PERIOD	2			/* sleep time (ticks) per cycle */

#define	NGNATS	50			/* total number of gnats */

#define	MAXVEL	10			/* abs. bound on velocity component */


typedef int	bool;			/* Boolean data type */
#define	false	0
#define	true	1

STATIC bool	leader;			/* true => randomly chase gnat # 0;
					   false => chase mouse cursor */

STATIC struct
	{
	Point	ulc;			/* upper left corner screen coords */
	Point	vel;			/* velocity (pixels/cycle) */
	}	gnat[NGNATS];		/* keeps track of gnats' state */
/* We rely on this forcing the following array to be Word-aligned! */

STATIC short	gnatbits[] =
	{
	0x4200, 0x0000,
	0xA500, 0x0000,
	0x1800, 0x0000,
	0x1800, 0x0000
	};
STATIC Bitmap	gnatmap = { (Word *)gnatbits, 32 / WORDSIZE, 0, 0, 32, 4 };


STATIC void	DrawGnat(), InitGnats(), ModVel(), MoveGnat(), NewVel();
STATIC int	RandInt();


STATIC void
InitGnats()				/* set up initial gnat swarm */
	{
	register int	i;		/* gnat # */

	for ( i = 0; i < NGNATS; ++i )
		{
		/* Assign random position within "gnats" layer: */

		gnat[i].ulc.x = RandInt( display.rect.origin.x,
					 display.rect.corner.x - 8
				       );
		gnat[i].ulc.y = RandInt( display.rect.origin.y,
					 display.rect.corner.y - 4
				       );

		/* Assign random velocity: */

		NewVel( i );

		DrawGnat( i );		/* display the critter */
		}

/* DEBUG
	display.rect.corner = display.rect.origin;	/* make unpickable */
	}


STATIC void
MoveGnat( i )				/* one motion cycle for a gnat */
	register int	i;		/* gnat # */
	{
	Point		p;		/* new gnat upper left corner */
	Point		target;		/* current coordinates of target */

	if ( leader )
		target = gnat[0].ulc;
	else
		target = RealMouse.xy;

	/* Nudge toward the target: */

	if ( !leader || i != 0 )
		{
		if ( target.x < gnat[i].ulc.x )
			--gnat[i].vel.x;
		else
			++gnat[i].vel.x;

		if ( target.y < gnat[i].ulc.y )
			--gnat[i].vel.y;
		else
			++gnat[i].vel.y;
		}

	/* Velocity can become rather huge; causes overshoot, a nifty effect. */

	for ( ; ; )			/* determine a new position */
		{
		p.x = gnat[i].ulc.x + gnat[i].vel.x;	/* motion */
		p.y = gnat[i].ulc.y + gnat[i].vel.y;

		if ( p.x >= 0 && p.x < XMAX - 8
		  && p.y >= 0 && p.y < YMAX - 4
		   )
			break;		/* on-screen, proceed */

		/* Bounce off edge of screen: */

		NewVel( i );		/* assign new velocity */
		}

	gnat[i].ulc = p;

	ModVel( i );			/* randomly alter gnat velocity */
	}


STATIC void
DrawGnat( i )				/* draw specified gnat */
	int	i;			/* gnat # */
	{
	bitblt( &gnatmap, Rect( 0, 0, 8, 4 ), &physical, gnat[i].ulc, F_XOR );
	}


STATIC void
NewVel( i )				/* assign new velocity to gnat */
	int	i;			/* gnat # */
	{
	gnat[i].vel.x = RandInt( 1 - MAXVEL, MAXVEL );
	gnat[i].vel.y = RandInt( 1 - MAXVEL, MAXVEL );

	/* Velocity (0,0) is okay since we repeatedly modify all velocities. */
	}


STATIC void
ModVel( i )				/* randomly modify gnat velocity */
	register int	i;		/* gnat # */
	{
	register int	d;		/* increment */

	if ( gnat[i].vel.x >= MAXVEL - 1 )
		d = RandInt( -1, 1 );
	else if ( gnat[i].vel.x <= 1 - MAXVEL )
		d = RandInt( 0, 2 );
	else
		d = RandInt( -1, 2 );

	gnat[i].vel.x += d;

	if ( gnat[i].vel.y >= MAXVEL - 1 )
		d = RandInt( -1, 1 );
	else if ( gnat[i].vel.y <= 1 - MAXVEL )
		d = RandInt( 0, 2 );
	else
		d = RandInt( -1, 2 );

	gnat[i].vel.y += d;
	}


STATIC int
RandInt( lo, hi )			/* generate random integer in range */
	int	lo, hi;			/* range lo..hi-1 */
	{
	return lo + (int)((long)(hi - lo) * (long)(rand() & 0x7FFF) / 32768L);
	}


main( argc, argv )
	int		argc;
	char		*argv[];
	{
	register int	i;		/* gnat number */
	register bool	drawn;		/* true iff gnats have been drawn */
	register int	new_res;	/* current own() result */
	register int	old_res;	/* previous own() result */
	register int	c;		/* input keystroke (or -1) */

#ifdef DMD630
	(void)local();			/* detach from host channel */
	(void)cache( "swarm", 0 );
#endif

	leader = argc > 1;		/* determine desired style of chasing */
	drawn = false;			/* to avoid undrawing garbage */
	old_res = 0;

	request( KBD|MOUSE|DELETE );	/* so we can tell if layer is current */

	P->state |= RESHAPED;		/* force initial erasure of layer */

	while ( ((new_res = own()) & DELETE) == 0 && (c = kbdchar()) != 'q' )
		{
		if ( c == 'r' )
			leader = !leader;

		/* If the "currency" state of this layer has toggled, the border
		   has changed and should be restored to background texture. */

		if ( ((new_res ^ old_res) & MOUSE) != 0	/* "current" toggled */
		  || (P->state & (MOVED | RESHAPED)) != 0 /* geometry change */
		   )	{
			if ( drawn )
				for ( i = 0; i < NGNATS; ++i )
					DrawGnat( i );	/* erase old swarm */

			/* Clear layer, including the border, to background: */

			texture16( &display, display.rect, &T_background,
				   F_STORE
				 );

			InitGnats();	/* display new initial gnat swarm */
			drawn = true;

			P->state &= ~(MOVED | RESHAPED);	/* repainted */
			}

		old_res = new_res;

		for ( i = NGNATS; --i >= 0; )
			{
			/* The following procedure can flicker slightly,
			   just like real gnats! */
			DrawGnat( i );	/* erase from old position */
			MoveGnat( i );	/* compute new position */
			DrawGnat( i );	/* draw in new position */
			}

		request( KBD|DELETE );	/* let layersys handle button-3 hits */
		sleep( PERIOD );	/* relinquish the processor */
		request( KBD|MOUSE|DELETE );	/* so we can tell if current */
		}

	/* Clean up before terminating: */

	if ( drawn )
		for ( i = 0; i < NGNATS; ++i )
			DrawGnat( i );	/* erase from last position */

#ifndef DMD630
	if ( (new_res & DELETE) != 0 )
#endif
		delete();

	exit();
	}
