/* #ident	"@(#)x11:contrib/clients/xloadimage/xli.h 1.4 94/07/29 Labtam" */
/* xli.h:
 *
 * jim frost 06.21.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#if defined(SVR4) && !defined(SYSV)
# define SYSV		/* SYSV is out System V flag */
#endif

#include "copyright.h"
#include <stdio.h>
#include <math.h>

#include "ddxli.h"
#include "image.h"
#include "options.h"

#ifdef MSDOS	/* gcc under MSDOS compatibility fudges */
#define EISDIR  37		/* Attempt to write on a directory */
#endif /* MSDOS */


#ifndef MAXIMAGES
#define MAXIMAGES BUFSIZ /* max # of images we'll try to load at once */
#endif


/* image name and option structure used when processing arguments
 */

typedef struct {
  char         *name;         /* name of image */
  int           loader_idx;   /* loader last used successfully with image */
  int           atx, aty;     /* location to load image at */
  boolean       ats;          /* TRUE if atx and aty have been set */
  unsigned int  bright;       /* brightness multiplier */
  unsigned int  center;       /* true if image is to be centered */
  int           clipx, clipy; /* Offset and area of image to be used */
  unsigned int  clipw, cliph; 
  char         *border;       /* Border colour used in clipping */
  xliXColor     bordercol;    /* X RGB of above */
  unsigned int  colordither;  /* true if color reduction is to dither image */
  unsigned int  colors;       /* max # of colors to use for this image */
  int           delay;        /* # of seconds delay before auto pic advance */
  unsigned int  dither;       /* true if image is to be dithered */
  boolean       expand;       /* true if image should be forced to TrueColor depth */
  float         gamma;        /* display gamma */
  unsigned int  gray;         /* true if image is to be grayed */
  unsigned int  merge;        /* true if we should merge onto previous */
  unsigned int  normalize;    /* true if image is to be normalized */
  int           rotate;	      /* # degrees to rotate image */
  unsigned int  smooth;       /* true if image is to be smoothed */
  char         *title;        /* Overide title on image */
  unsigned int  xzoom, yzoom; /* zoom percentages */
  char         *fg, *bg;      /* foreground/background colors if mono image */
  boolean       done_to;      /* TRUE if we have already looked for trailing options */
  char        **to_argv;      /* IF non NULL, text of trailing options, parsed as for argv */
  int           xpmkeyc;      /* Overideing color context key value for xpm pixmaps */
} ImageOptions;

/* globals and global options
 */

typedef struct {
  DisplayInfo	dinfo;		/* device dependent display information */
  char         *argv0;		/* name of this programs */
  char         *lastfunc;	/* name of last function called (used in error reps) */
  int           _Xdebug;	/* dump on X error flag */
  int           _DumpCore;	/* dump on signal flag */
  boolean       verbose;
  float         display_gamma;
  char         *dname;
  boolean       do_fork;
  boolean       fillscreen;
  boolean       fit;
  boolean       fullscreen;
  char         *go_to;        /* label to goto */
  boolean       identify;
  boolean       install;
  boolean       onroot;
  boolean       private_cmap;
  boolean       set_default;
  boolean       use_pixmap;
  char         *user_geometry;	/* -geometry passed by user */
  int           visual_class;    /* user-defined visual class */
#ifndef MSDOS	/* If X11 version */
  unsigned int  dest_window;    /* window id to put image onto */
#endif
} GlobalsRec;

/* Global declarations */

extern GlobalsRec globals;

#define CURRFUNC(aa) (globals.lastfunc = (aa))

/* Gamma correction stuff
 */

/* the default target display gamma. This can be overridden on the */
/* command line or by settin an environment variable. */
#define DEFAULT_DISPLAY_GAMMA 2.2

/* the default IRGB image gamma. This can be overridden on the */
/* command line. */
#define DEFAULT_IRGB_GAMMA 2.2

/* Compare gammas for equality */
#define GAMMA_NOT_EQUAL(g1,g2)   ((g1) > ((g2) + 0.00001) || (g1) < ((g2) - 0.00001))

/*
 * Cached/uudecoded/uncompressed file I/O structures.
 */

struct cache {
  byte         *end;
  byte          buf[BUFSIZ];
  struct cache *next;
  boolean       eof;
};

#define UULEN 128   /* uudecode buffer length */
#define UUBODY 1   /* uudecode state - reading body of file */
#define UUSKIP 2   /* uudecode state - skipping garbage */
#define UUEOF 3   /* uudecode stat - found eof */

typedef struct {
  unsigned int  type;     /* ZIO file type */
  boolean       nocache;  /* TRUE if caching has been disabled */
  FILE         *stream;   /* file input stream */
  char         *filename; /* filename */
  struct cache *data;     /* data cache */
  struct cache *dataptr;  /* ptr to current cache block */
  byte         *bufptr;   /* ptr within current cache block */
  byte         *endptr;   /* ptr to end of current cache block */
  byte         *auxb;     /* non NULL if auxiliary buffer in use */
  byte         *oldbufptr;   /* save bufptr here when aux buffer is in use */
  byte         *oldendptr;   /* save endptr here when aux buffer is in use */
  boolean       eof;      /* TRUE if we are at encountered EOF */
  byte          buf[BUFSIZ]; /* getc buffer when un-cached */
  boolean       uudecode; /* TRUE if being uudecoded */
  byte          uubuf[UULEN]; /* uu decode buffer */
  byte         *uunext,*uuend;
  int           uustate;      /* state of uu decoder */
} ZFILE;

#define ZSTANDARD 0 /* standard file */
#define ZPIPE     1 /* file is a pipe (ie uncompress) */
#define ZSTDIN    2 /* file is stdin */

/*
   C library functions that may not be decalared elsewehere
 */

int    atoi();
long   atol();
double atof();
char *getenv();

/*
 * function declarations
 */

/*
 * Note on image processing functions :-
 *
 * The assumption is always that an image processing function that returns
 * an image may return a new image or one of the input images, and that
 * input images are preserved.
 *
 * This means that within an image processing function the following
 * technique of managinging intermediate images is recomended:
 *
 * Image *func(isrc)
 * Image *isrc;
 *   {
 *   Image *src = isrc, *dst, *tmp;
 *   .
 *   .
 *   tmp = other_func(src);
 *   if (src != tmp && src != isrc)
 *     freeImage(src);
 *   src = tmp;
 *   .
 *   .
 *   dst = process(src);
 *   .
 *   .
 *   if (src != isrc)
 *     freeImage(src);
 *   return (dst);
 *
 * This may seem redundant in places, but allows changes to be made
 * without looking at the overal image structure usage.
 *
 */

void supportedImageTypes(); /* imagetypes.c */

char *tail(); /* misc.c */
void memoryExhausted();
void internalError();
void version();
void usage();
void goodImage();
Image *processImage();
int errorHandler();
extern short LEHexTable[];  /* Little Endian conversion value */
extern short BEHexTable[];  /* Big Endian conversion value */
#define HEXSTART_BAD -1		/* bitmap_faces useage */
#define HEXDELIM_BAD -2
#define HEXDELIM_IGNORE -3
#define HEXBAD   -4
void initLEHexTable();
void initBEHexTable();
int hstoi();				/* ascii hex number to integer (string, length) */

char *expandPath(); /* path.c */
int findImage();
void listImages();
void loadPathsAndExts();
void showPath();

void imageOnRoot(); /* root.c */

void cleanUpWindow();	/* window.c */
char imageInWindow();

int   visualClassFromName();	/* options.c */
char *nameOfVisualClass();

Image *clip(); /* clip.c */

void brighten(); /* bright.c */
void equalize();
void gray();
Image *normalize();
void gammacorrect();
extern int gammamap[256];
#define GAMMA16(color16) (gammamap[(color16)>>8]<<8)
#define GAMMA8(color8) (gammamap[(color8)])
#define GAMMA16to8(color16) (gammamap[(color16)>>8])

void compress(); /* compress.c */

Image *dither(); /* dither.c */

void fill(); /* fill.c */

void fold(); /* fold.c */

Image *halftone(); /* halftone.c */

Image *loadImage(); /* imagetypes.c */
void   identifyImage();
void   goodImage();

Image *merge(); /* merge.c */

extern unsigned long DepthToColorsTable[]; /* new.c */
unsigned long colorsToDepth();
char  *dupString();
Image *newBitImage();
Image *newRGBImage();
Image *newTrueImage();
void   freeImage();
void   freeImageData();
void   newRGBMapData();
void   resizeRGBMapData();
void   freeRGBMapData();
byte  *lcalloc();
byte  *lmalloc();
byte  *lrealloc();
void   lfree();

Image *reduce(); /* reduce.c */
Image *expandtotrue();
Image *expandbittoirgb();
Image *expandirgbdepth();

Image *rotate(); /* rotate.c */

Image *smooth(); /* smooth.c */

void          flipBits();	/* value.c */

ZFILE   *zopen(); /* zio.c */
int      zread();
void     zunread();
char    *zgets();
boolean  zrewind();
void     zclose();
void     znocache();
void     zreset();
int     _zgetc();
boolean _zopen();
void    _zreset();
void    _zclear();
boolean _zreopen();

#define zgetc(zf) (((zf)->bufptr < (zf)->endptr) ? *(zf)->bufptr++ : _zgetc(zf))

Image *zoom(); /* zoom.c */

boolean xliOpenDisplay();	/* ddxli.c */

#ifdef MSDOS
void argexp();		/* doslib.c */
#endif

