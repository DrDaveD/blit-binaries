/*
	crabs -- see September 1985 Scientific American pages 18..23

	last edit:	89/02/25	D A Gwyn

	SCCS ID:	@(#)crabs.c	1.3	for 5620 DMD and 630 MTG

To compile:
	$ dmdcc -o crabs.m crabs.c	# -g -O also recommended

To run:
	$ dmdld crabs.m			# runs only in mpx mode
or
	$ dmdld crabs.m -		# for invisible crabs
*/

#ifndef lint
static char	SCCS_ID[] = "@(#)crabs.c	1.3 89/02/25";	/* for "what" utility */
#endif


#include	<dmd.h>

#ifdef DMD630

#define	texture16	texture

#else	/* 5620 */

static Texture16	T_background =		/* background texture */
	{
	0x1111, 0x4444, 0x1111, 0x4444,
	0x1111, 0x4444, 0x1111, 0x4444,
	0x1111, 0x4444, 0x1111, 0x4444,
	0x1111, 0x4444, 0x1111, 0x4444
	};

static Bitmap	physical =		/* full screen definition */
	{
	(Word *)0x700000L,		/* DMD screen image base address */
	(XMAX + 31) / 32,		/* bitmap width in 32-bit Words */
	0, 0, XMAX, YMAX		/* screen rectangle within bitmap */
	};

#endif


#define	NCRABS	32			/* total number of crabs (1..32) */

#define	MAXVEL	8			/* abs. bound on velocity component */

#define	PERIOD	2			/* sleep time (ticks) per cycle */


typedef int	bool;			/* Boolean data type */
#define	false	0
#define	true	1


static bool	visible;		/* true if crabs are to be shown */

static struct
	{
	Point	ulc;			/* upper left corner screen coords */
	Point	vel;			/* velocity (pixels/cycle) */
	}	crab[NCRABS];		/* keeps track of crabs' state */
/* We rely on this forcing the following arrays to be Word-aligned! */

/* There are 4 possible crab orientations, each of which
   has 4 possible relationships with the grey background.
   (Scientific American article says 8, but it's wrong.) */

/* Crab images XORed with grey background texture at various offsets: */

static short	upcrab[] =		/* facing up */
	{
	0x6E4C, 0x2A66,
	0xB377, 0xE6D5,
	0x8081, 0x8101,
	0xB935, 0xAC9D,
	0x6E5C, 0x3A76,
	0x3A76, 0x6E5C,
	0xAC9D, 0xB935,
	0x0242, 0x4240
	};

static short	downcrab[] =		/* facing down */
	{
	0x4240, 0x0242,
	0xB935, 0xAC9D,
	0x6E5C, 0x3A76,
	0x3A76, 0x6E5C,
	0xAC9D, 0xB935,
	0x8101, 0x8081,
	0xEECD, 0xAB67,
	0x3276, 0x6654
	};

static short	rightcrab[] =		/* facing right */
	{
	0x4E4C, 0x0A46,
	0xB333, 0xA291,
	0x6A59, 0x3B73,
	0x3A72, 0x6A58,
	0x6859, 0x3971,
	0x3B73, 0x6A59,
	0xA291, 0xB333,
	0x0A46, 0x4E4C
	};

static short	leftcrab[] =		/* facing left */
	{
	0x6250, 0x3272,
	0x8945, 0xCCCD,
	0xCEDC, 0x9A56,
	0x9A16, 0x8E9C,
	0x4E5C, 0x1A56,
	0x9A56, 0xCEDC,
	0xCCCD, 0x8945,
	0x3272, 0x6250
	};

/* The bitmaps for the four orientations: */

static Bitmap	upmap = { (Word *)upcrab, 32/WORDSIZE, 0, 0, 32, 8 };
static Bitmap	downmap = { (Word *)downcrab, 32/WORDSIZE, 0, 0, 32, 8 };
static Bitmap	rightmap = { (Word *)rightcrab, 32/WORDSIZE, 0, 0, 32, 8 };
static Bitmap	leftmap = { (Word *)leftcrab, 32/WORDSIZE, 0, 0, 32, 8 };

/* Crab "vicinities" are recorded in the following
   global map; see Collide() and Draw() for details: */

static long	vicinity[(XMAX + 31) / 32 + 2][(YMAX + 31) / 32 + 2];
					/* includes margins all around */

static void	Cycle(), DrawCrab(), HideCrabs(), Init(), ModVel(), NewVel();
static int	RandInt();
static long	Collide();

main( argc, argv )
	int	argc;
	char	*argv[];
	{
	Init( argc, argv );		/* set up initial grey crab layer */

	for ( ; ; )			/* no way out! */
		{
		sleep( PERIOD );	/* relinquish the processor */

		Cycle();		/* move the crabs */
		}
	/*NOTREACHED*/
	}


static void
Init( argc, argv )			/* set up initial crab layer */
	int	argc;
	char	*argv[];
	{
	int	i;			/* crab # */

	visible = argc <= 1;		/* default is to show crabs */

	texture16( &display, display.rect, &T_background, F_STORE );
					/* crab layer */

	/* Create initial set of crabs: */

	for ( i = 0; i < NCRABS; ++i )
		{
		/* Assign random position within "crabs" layer: */

		crab[i].ulc.x = RandInt( display.rect.origin.x,
					 display.rect.corner.x - 8
				       );
		crab[i].ulc.y = RandInt( display.rect.origin.y,
					 display.rect.corner.y - 8
				       );

		/* Assign random velocity: */

		NewVel( i );

		/* Draw crab at initial position (within "crabs" layer): */

		if ( visible )
			DrawCrab( i );
		}

/* DEBUG
	display.rect.corner = display.rect.origin;	/* make unpickable */
	}


static void
Cycle()					/* one motion cycle for all crabs */
	{
	static long	old[8];		/* old contents of new crab position */
	static Bitmap	oldmap = { (Word *)old, 32/WORDSIZE, 0, 0, 8, 8 };
	Point		p;		/* new crab upper left corner */
	Rectangle	r;		/* new crab area */
	long		syndrome;	/* crab collision mask */
	int		i;		/* crab # */
	int		w;		/* index for old[.] */

	for ( i = 0; i < NCRABS; ++i )
		{
		DrawCrab( i );		/* erase crab from previous position */

		for ( ; ; )		/* determine a new position */
			{
			p.x = crab[i].ulc.x + crab[i].vel.x;	/* motion */
			p.y = crab[i].ulc.y + crab[i].vel.y;

			if ( p.x >= 0 && p.x < XMAX - 8
			  && p.y >= 0 && p.y < YMAX - 8
			   )
				break;	/* on-screen, proceed */

			/* Bounce off edge of screen: */

			NewVel( i );	/* assign new velocity */
			}

		r.origin = p;
		r.corner.x = p.x + 8;
		r.corner.y = p.y + 8;

		/* Check for collision with other crabs;
		   if you don't worry about this, you get
		   crud left behind from crab collisions
		   (visible in Scientific American article).
		   (Note that crab # i has been removed.) */

		/* The strategy is: only undraw possibly colliding crabs.
		   The obvious alternative, not showing any crabs until
		   all locations have been painted, would probably cause
		   the set of crabs to flicker or to appear too faint. */

		if ( (syndrome = Collide( p )) != 0L )
			HideCrabs( syndrome );	/* save from following code */

		/* Save old contents of new crab location: */

		bitblt( &physical, r, &oldmap, Pt( 0, 0 ), F_STORE );

		/* Paint the new location grey: */

		texture16( &physical, r, &T_background, F_STORE );

		/* Determine if new location used to be grey: */

		bitblt( &physical, r, &oldmap, Pt( 0, 0 ), F_XOR );

		for ( w = 0; w < 8; ++w )
			if ( old[w] != 0L )
				{	/* this location has been nibbled */
				p = crab[i].ulc;	/* reset position */

				NewVel( i );	/* bounce away from bite */

				break;
				}

		if ( syndrome != 0L )
			HideCrabs( syndrome );	/* bring them back */

		/* Draw the crab in its new position: */

		crab[i].ulc = p;

		ModVel( i );		/* randomly alter crab velocity */

		DrawCrab( i );
		}
	}


static long
Collide( p )				/* return syndrome for crab collision */
	Point	p;			/* crab upper left corner */
	{
	long	syndrome;		/* accumulate syndrome here */
	bool	right = p.x % 32 > 32 - 8,
		down = p.y % 32 > 32 - 8;	/* more than one vicinity? */
	int	x32 = p.x / 32,
		y32 = p.y / 32;		/* vicinity array indices */

	/* "Or" in crabs from overlapping vicinities: */

	syndrome = vicinity[x32 + 1][y32 + 1];
	if ( right )
		syndrome |= vicinity[x32 + 1 + 1][y32 + 1];
	if ( down )
		syndrome |= vicinity[x32 + 1][y32 + 1 + 1];
	if ( right && down )
		syndrome |= vicinity[x32 + 1 + 1][y32 + 1 + 1];	

	return syndrome;
	}


static void
HideCrabs( syndrome )			/* draw crabs contained in syndrome */
	long	syndrome;		/* syndrome (crab bit flags) */
	{
	int	i;			/* indexes crab[.] */
	long	m;			/* bit mask for crab # i */

	for ( m = 1L, i = 0; i < NCRABS; m <<= 1, ++i )
		if ( (m & syndrome) != 0L )	/* crab contained in syndrome */
			DrawCrab( i );	/* toggle crab */
	}


static void
DrawCrab( i )				/* draw specified crab */
	int	i;			/* crab # (0..NCRABS-1) */
	{
	Point	p;			/* upper left corner for crab image */
	Point	v;			/* crab velocity */
	Bitmap	*whichmap;		/* -> 1 of 4 possible orientations */
	int	index;			/* selects 1 of 4 offsets wrt grey */
	int	x32, y32;		/* vicinity array indices */
	bool	right, down;		/* more than one vicinity? */
	long	syn_bit;		/* crab possible-occupancy bit */

	if ( visible )
		{
		p = crab[i].ulc;
		v = crab[i].vel;

		if ( abs( v.x ) >= abs( v.y ) )
			if ( v.x > 0 )
				whichmap = &upmap;
			else
				whichmap = &downmap;
		else
			if ( v.y > 0 )
				whichmap = &rightmap;
			else
				whichmap = &leftmap;

		index = (p.x + p.y * 2) % 4 * 8;
		bitblt( whichmap,
			Rect( index, 0, index + 8, 8 ),
			&physical,
			p,
			F_XOR
		      );

		/* A crab's vicinities are the disjoint 32x32 regions
		   that contain any piece of the crab's 8x8 square.
		   On the average, 9 out of 16 crabs occupy just 1
		   vicinity; 6 out of 16 crabs occupy 2 vicinities,
		   and 1 out of every 16 crabs occupies 4 vicinities. */

		x32 = p.x / 32;
		y32 = p.y / 32;		/* coords for upper left vicinity */

		right = p.x % 32 > 32 - 8;	/* also next vicinity right? */
		down = p.y % 32 > 32 - 8;	/* also next vicinty down? */

		/* Toggle crab's occupancy bit in all occupied vicinities: */

		syn_bit = i == 0 ? 1L : 1L << i;

		vicinity[x32 + 1][y32 + 1] ^= syn_bit;
		if ( right )
			vicinity[x32 + 1 + 1][y32 + 1] ^= syn_bit;
		if ( down )
			vicinity[x32 + 1][y32 + 1 + 1] ^= syn_bit;
		if ( right && down )
			vicinity[x32 + 1 + 1][y32 + 1 + 1] ^= syn_bit;	
		}
	/* else nibble away but don't show crabs */
	}


static void
NewVel( i )				/* assign new velocity to crab */
	int	i;			/* crab # */
	{
	crab[i].vel.x = RandInt( 1 - MAXVEL, MAXVEL );
	crab[i].vel.y = RandInt( 1 - MAXVEL, MAXVEL );

	/* Velocity (0,0) is okay since we repeatedly modify all velocities. */
	}


static void
ModVel( i )				/* randomly modify crab velocity */
	int	i;			/* crab # */
	{
	int	d;			/* increment */

	if ( crab[i].vel.x >= MAXVEL - 1 )
		d = RandInt( -1, 1 );
	else if ( crab[i].vel.x <= 1 - MAXVEL )
		d = RandInt( 0, 2 );
	else
		d = RandInt( -1, 2 );

	crab[i].vel.x += d;

	if ( crab[i].vel.y >= MAXVEL - 1 )
		d = RandInt( -1, 1 );
	else if ( crab[i].vel.y <= 1 - MAXVEL )
		d = RandInt( 0, 2 );
	else
		d = RandInt( -1, 2 );

	crab[i].vel.y += d;
	}


static int
RandInt( lo, hi )			/* generate random integer in range */
	int	lo, hi;			/* range lo..hi-1 */
	{
	return lo + (int)((long)(hi - lo) * (long)(rand() & 0x7FFF) / 32768L);
	}
