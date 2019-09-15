/* #ident	"@(#)x11:contrib/clients/xloadimage/ddxli.h 1.3 94/07/29 Labtam" */
/*
 * General Device dependent code for xli.
 *
 * Author; Graeme W. Gill
 */

/**********************/
/* OS dependent stuff */

#ifndef MSDOS	/* If X11 version */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#else /* MSDOS Version */
#include <graphics.h>
#include <pc.h>
#include <sys/registers.h>

#endif /* MSDOS Version */

#if defined(SYSV) || defined(VMS)
#include <string.h>
#ifndef index /* some SysV's do this for you */
#define index strchr
#endif
#ifndef rindex
#define rindex strrchr
#endif
#ifndef HAS_MEMCPY
#define HAS_MEMCPY
#endif
#else /* !SYSV && !VMS */
#include <strings.h>
#endif /* !SYSV && !VMS */

#ifdef VMS
#define R_OK 4
#define NO_UNCOMPRESS
#endif

/* equate bcopy w/ memcpy and bzero w/ memset where appropriate.
 */

#ifdef HAS_MEMCPY
#ifndef bcopy
#define bcopy(S,D,N) memcpy((char *)(D),(char *)(S),(N))
#endif
#ifndef bzero
#define bzero(P,N) memset((P),'\0',(N))
#endif
#ifndef bfill
#define bfill(P,N,C) memset((P),(C),(N))
#endif
#else /* not HAS_MEMCPY */
void	bfill();
#endif /* not HAS_MEMCPY */

/***************************/
/* xli specific data types */

#undef HAS_SIGNED_CHAR			/* If signed char is a legal declaration */
								/* else assume char is signed by default */

typedef unsigned long  Pixel;     /* what X thinks a pixel is */
typedef unsigned short Intensity; /* what X thinks an RGB intensity is */
typedef unsigned char  byte;      /* unsigned byte type */
#ifdef HAS_SIGNED_CHAR
typedef signed   char  sbyte;     /* signed byte type */
#else
typedef char  sbyte;              /* signed byte type */
#endif

#ifndef HAVE_BOOLEAN
#define HAVE_BOOLEAN
typedef int boolean;
#endif

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

/**************************************************/
/* Display device dependent Information structure */
typedef struct {
	int  width;		/* Display width and height */
	int  height;
#ifndef MSDOS	/* If X11 version */
	Display      *disp;
	int           scrn;
	Colormap      colormap;
#else	/* else is MSDOS version */
	unsigned char *base;
	int  span;
#endif	/* MSDOS version */
} DisplayInfo;

#ifndef MSDOS	/* If X11 version */

/* This struct holds the X-client side bits for a rendered image.
 */

typedef struct {
  Display  *disp;       /* destination display */
  int       scrn;       /* destination screen */
  int       depth;      /* depth of drawable we want/have */
  Drawable  drawable;   /* drawable to send image to */
  Pixel    *index;		/* array of pixel values allocated */
  int       no;		    /* number of pixels in the array */
  boolean   rootimage;	/* True if is a root image - eg, retain colors */
  Pixel     foreground; /* foreground and background pixels for mono images */
  Pixel     background;
  Colormap  cmap;       /* colormap used for image */
  GC        gc;         /* cached gc for sending image */
  XImage   *ximage;     /* ximage structure */
} XImageInfo;

/* Function declarations */
void        sendXImage(); /* send.c */
XImageInfo *imageToXImage();
Pixmap      ximageToPixmap();
void        freeXImage();

Visual *getBestVisual(); /* visual.c */

char *xliDisplayName();		/* ddxli.c */
int xliParseXColor();
int xliDefaultVisual();
void xliGammaCorrectXColor();

#else /* else is MSDOS version */

/* We need to understand X11 style visuals for xwd and xpm files. */
#define StaticGray		0
#define GrayScale		1
#define StaticColor		2
#define PseudoColor		3
#define TrueColor		4
#define DirectColor		5

/* and some flags for our imitation lookup color */
#define DoRed			(1<<0)
#define DoGreen			(1<<1)
#define DoBlue			(1<<2)

/* and some xwd definitions */
#define XYBitmap		0	/* depth 1, XYFormat */
#define XYPixmap		1	/* depth == drawable depth */
#define ZPixmap			2	/* depth == drawable depth */

/* and some data order definitions */
#define LSBFirst		0
#define MSBFirst		1

#endif /* else is MSDOS version */
