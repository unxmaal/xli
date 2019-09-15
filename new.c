/* #ident	"@(#)x11:contrib/clients/xloadimage/new.c 6.11 94/07/29 Labtam" */
/* new.c:
 *
 * functions to allocate and deallocate structures and structure data
 *
 * jim frost 09.29.89
 *
 * Copyright 1989, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "xli.h"

/* this table is useful for quick conversions between depth and ncolors
 */

unsigned long DepthToColorsTable[] = {
  /*  0 */ 1,
  /*  1 */ 2,
  /*  2 */ 4,
  /*  3 */ 8,
  /*  4 */ 16,
  /*  5 */ 32,
  /*  6 */ 64,
  /*  7 */ 128,
  /*  8 */ 256,
  /*  9 */ 512,
  /* 10 */ 1024,
  /* 11 */ 2048,
  /* 12 */ 4096,
  /* 13 */ 8192,
  /* 14 */ 16384,
  /* 15 */ 32768,
  /* 16 */ 65536,
  /* 17 */ 131072,
  /* 18 */ 262144,
  /* 19 */ 524288,
  /* 20 */ 1048576,
  /* 21 */ 2097152,
  /* 22 */ 4194304,
  /* 23 */ 8388608,
  /* 24 */ 16777216,
  /* 25 */ 33554432,
  /* 26 */ 67108864,
  /* 27 */ 134217728,
  /* 28 */ 268435456,
  /* 29 */ 536870912,
  /* 30 */ 1073741824,
  /* 31 */ 2147483648,
  /* 32 */ 2147483648 /* bigger than unsigned int; this is good enough */
};

unsigned long colorsToDepth(ncolors)
     unsigned long ncolors;
{ unsigned long a;

  for (a= 0; (a < 32) && (DepthToColorsTable[a] < ncolors); a++)
    /* EMPTY */
    ;
  return(a);
}

char *dupString(s)
     char *s;
{ char *d;

  if (!s)
    return(NULL);
  d= (char *)lmalloc(strlen(s) + 1);
  strcpy(d, s);
  return(d);
}

void newRGBMapData(rgb, size)
     RGBMap       *rgb;
     unsigned int  size;
{
  CURRFUNC("newRGBMapData");
  rgb->used= 0;
  rgb->size= size;
  rgb->compressed= FALSE;
  rgb->red= (Intensity *)lmalloc(sizeof(Intensity) * size);
  rgb->green= (Intensity *)lmalloc(sizeof(Intensity) * size);
  rgb->blue= (Intensity *)lmalloc(sizeof(Intensity) * size);
}

void resizeRGBMapData(rgb, size)
     RGBMap       *rgb;
     unsigned int  size;
{
  CURRFUNC("resizeRGBMapData");
  rgb->red= (Intensity *)lrealloc(rgb->red, sizeof(Intensity) * size);
  rgb->green= (Intensity *)lrealloc(rgb->green, sizeof(Intensity) * size);
  rgb->blue= (Intensity *)lrealloc(rgb->blue, sizeof(Intensity) * size);
  rgb->size= size;	/* new size */
}

void freeRGBMapData(rgb)
     RGBMap *rgb;
{
  CURRFUNC("freeRGBMapData");
  lfree((byte *)rgb->red);
  lfree((byte *)rgb->green);
  lfree((byte *)rgb->blue);
}

Image *newBitImage(width, height)
     unsigned int width, height;
{ Image        *image;
  unsigned int  linelen;

  CURRFUNC("newBitImage");
  image= (Image *)lmalloc(sizeof(Image));
  image->type= IBITMAP;
  image->title= NULL;
  newRGBMapData(&(image->rgb), (unsigned int)2);
  *(image->rgb.red)= *(image->rgb.green)= *(image->rgb.blue)= 65535;
  *(image->rgb.red + 1)= *(image->rgb.green + 1)= *(image->rgb.blue + 1)= 0;
  image->rgb.used= 2;
  image->width= width;
  image->height= height;
  image->depth= 1;
  linelen= ((width + 7) / 8);
  image->data= (unsigned char *)lcalloc(linelen * height);
  image->gamma= 0.0;	/* nonsense value */
  return(image);
}

Image *newRGBImage(width, height, depth)
     unsigned int width, height, depth;
{ Image        *image;
  unsigned int  pixlen, numcolors, a;

  CURRFUNC("newRGBImage");

  if (depth == 0) /* special case for `zero' depth image, which is */
    depth= 1;     /* sometimes interpreted as `one color' */
  pixlen= ((depth+7) / 8);
  numcolors = depthToColors(depth);
  image= (Image *)lmalloc(sizeof(Image));
  image->type= IRGB;
  image->title= NULL;
  newRGBMapData(&(image->rgb), numcolors);
  image->width= width;
  image->height= height;
  image->depth= depth;
  image->pixlen= pixlen;
  image->data= (unsigned char *)lmalloc(width * height * pixlen);
  image->gamma= 0.0;	/* nonsense value */
  return(image);
}

Image *newTrueImage(width, height)
     unsigned int width, height;
{ Image        *image;
  unsigned int  pixlen, numcolors, a;

  CURRFUNC("NewTrueImage");
  image= (Image *)lmalloc(sizeof(Image));
  image->type= ITRUE;
  image->title= NULL;
  image->rgb.used= image->rgb.size= 0;
  image->width= width;
  image->height= height;
  image->depth= 24;
  image->pixlen= 3;
  image->data= (unsigned char *)lmalloc(width * height * 3);
  image->gamma= 0.0;	/* nonsense value */
  return(image);
}

void freeImageData(image)
     Image *image;
{
  if (image->title) {
    lfree((byte *)image->title);
    image->title= NULL;
  }
  if (!TRUEP(image))
    freeRGBMapData(&(image->rgb));
  lfree(image->data);
}

void freeImage(image)
     Image *image;
{
  freeImageData(image);
  lfree((byte *)image);
}

byte *lmalloc(size)
     unsigned int size;
{ byte *area;

  if (size == 0) {
    size= 1;
    if (globals._Xdebug)
      fprintf(stderr, "lmalloc given zero size!\n");
  }
  if (!(area= (byte *)malloc(size))) {
    memoryExhausted();
    /* NOTREACHED */
  }
  return(area);
}

byte *lcalloc(size)
     unsigned int size;
{ byte *area;

  if (size == 0) {
    size= 1;
    if (globals._Xdebug)
      fprintf(stderr, "lcalloc given zero size!\n");
  }
  if (!(area= (byte *)calloc(1, size))) {
    memoryExhausted();
    /* NOTREACHED */
  }
  return(area);
}

byte *lrealloc(old,size)
     byte *old;
     unsigned int size;
{ byte *area;

  if (size == 0) {
    size= 1;
    if (globals._Xdebug)
      fprintf(stderr, "lrealloc given zero size!\n");
  }
  if (!(area= (byte *)realloc(old,size))) {
    memoryExhausted();
    /* NOTREACHED */
  }
  return(area);
}

void lfree(area)
     byte *area;
{
  free(area);
}
