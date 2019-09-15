/* #ident	"@(#)x11:contrib/clients/xloadimage/cmuwmrast.c 1.5 93/07/23 Labtam" */
/*
 *
 * handle CMU Window Manager (ITC) raster image type
 *
 * dan lovinger (dl2n+@andrew.cmu.edu) 07.11.90
 *
 * the format is essentially a byte-reversed sun raster w/o encoding
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#include "copyright.h"
#include "xli.h"
#include "cmuwmrast.h"

/* SUPPRESS 558 */

int babble(name, headerp)
char *name;
struct cmuwm_header *headerp;
{
  printf("%s is a %dx%d %d plane CMU WM raster\n",
         name,
         memToVal(headerp->width, sizeof(long)),
         memToVal(headerp->height, sizeof(long)),
         memToVal(headerp->depth, sizeof(short)));
}

int cmuwmIdent(fullname, name)
char *fullname, *name;
{
    ZFILE *zf;
    struct cmuwm_header header;
    int r;

    if (!(zf = zopen(fullname)))
      {
	  perror("cmuwmIdent");
	  return(0);
      }

    switch (zread(zf, (byte *)&header, sizeof(struct cmuwm_header)))
      {
      case sizeof(struct cmuwm_header):
	if (memToVal(header.magic, sizeof(long)) != CMUWM_MAGIC)
	  {
	      r = 0;
	      break;
	  }
	  babble(name, &header);
	  r = 1;
	  break;

      case -1:
	  perror("cmuwmIdent");
      default:
	  r = 0;
	  break;
      }

    zclose(zf);
    return r;
}

Image* cmuwmLoad(fullname, image_ops, verbose)
 ImageOptions *image_ops;
 char         *fullname;
 boolean       verbose;
{
  ZFILE *zf;
  char  *name = image_ops->name;
  struct cmuwm_header header;
  Image *image;
  int height, width, row, linelen, r;
  byte *lineptr;

  if (!(zf= zopen(fullname))) 
  {
    perror("cmuwmLoad");
    return(NULL);
  }

  switch (zread(zf, (byte *)&header, sizeof(struct cmuwm_header))) 
  {
    case sizeof(struct cmuwm_header):
      if (memToVal(header.magic, sizeof(long)) != CMUWM_MAGIC)
      {
        zclose(zf);
        return(NULL);
      }
      if (verbose) babble(name, &header);
        break;

    default:
      zclose(zf);
      return(NULL);
  }

  if (memToVal(header.depth, sizeof(short)) != 1)
  {
    fprintf(stderr,"cmuwmLoad: %s - raster is of depth %d, must be 1\n",
	    name,
	    header.depth);
    return(NULL);
  }

  image = newBitImage(width = memToVal(header.width, sizeof(long)),
		      height = memToVal(header.height, sizeof(long)));

  image->title = dupString(name);
  linelen = (width / 8) + (width % 8 ? 1 : 0);
  lineptr = image->data;
  znocache(zf);

  for (row = 0; row < height; row++)
  {
    r = zread(zf, lineptr, linelen);

    if (r != linelen)
    {
      printf("cmuwmLoad: %s - short raster\n",name);
             zclose(zf);
      return image;
    }

    for (r = 0; r < linelen; r++)
    {
      lineptr[r] ^= 0xff;
    }

    lineptr += linelen;
  }

  read_trail_opt(image_ops,zf,image,verbose);

  zclose(zf);
  return image;
}

	  



