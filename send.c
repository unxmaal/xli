/* #ident	"@(#)x11:contrib/clients/xloadimage/send.c 6.21 94/07/29 Labtam" */
/* send.c:
 *
 * send an Image to an X pixmap
 *
 * jim frost 10.02.89
 *
 * Copyright 1989, 1990, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "xli.h"

/* extra colors to try allocating in private color maps to minimise flashing */
#define NOFLASH_COLORS 256
/* number of colors to allow in the default colormap */
#define DEFAULT_COLORS 16

#ifndef MSDOS	/* If we are the X11 version */

static int GotError;

static int pixmapErrorTrap(disp, pErrorEvent)
    Display	*disp;
    XErrorEvent * pErrorEvent;
{
#define MAXERRORLEN 100
    char buf[MAXERRORLEN+1];
    GotError = 1;
    XGetErrorText(disp, pErrorEvent->error_code, buf, MAXERRORLEN);
    printf("serial #%d (request code %d) Got Error %s\n",
	pErrorEvent->serial,
	pErrorEvent->request_code,
	buf);
    return(0);
}

Pixmap ximageToPixmap(disp, parent, ximageinfo)
     Display    *disp;
     Window      parent;
     XImageInfo *ximageinfo;
{
  int         (*old_handler)();
  Pixmap        pixmap;

  GotError = 0;
  old_handler = XSetErrorHandler(pixmapErrorTrap);
  XSync(disp, FALSE);
  pixmap= XCreatePixmap(disp, parent,
			ximageinfo->ximage->width, ximageinfo->ximage->height,
			ximageinfo->depth);
  (void)XSetErrorHandler(old_handler);
  if (GotError)
    return(None);
  ximageinfo->drawable= pixmap;
  sendXImage(ximageinfo, 0, 0, 0, 0,
	     ximageinfo->ximage->width, ximageinfo->ximage->height);
  return(pixmap);
}

/* find the best pixmap depth supported by the server for a particular
 * visual and return that depth.
 *
 * this is complicated by R3's lack of XListPixmapFormats so we fake it
 * by looking at the structure ourselves.
 */

static unsigned int bitsPerPixelAtDepth(disp, scrn, depth)
     Display      *disp;
     int           scrn;
     unsigned int  depth;
{
#if defined(XlibSpecificationRelease) && (XlibSpecificationRelease >= 4)
  /* the way things are */
  XPixmapFormatValues *xf;
  int nxf, a;

  xf = XListPixmapFormats(disp, &nxf);
  for (a = 0; a < nxf; a++)
    if (xf[a].depth == depth)
      {
      int bpp;
      bpp = xf[a].bits_per_pixel;
      XFree(xf);
      return (unsigned int) bpp;
      }
  XFree(xf);
#else /* the way things were (X11R3) */
  unsigned int a;

  for (a= 0; a < disp->nformats; a++)
    if (disp->pixmap_format[a].depth == depth)
      return(disp->pixmap_format[a].bits_per_pixel);
#endif

  /* this should never happen; if it does, we're in trouble
   */

  fprintf(stderr, "bitsPerPixelAtDepth: Can't find pixmap depth info!\n");
  exit(1);
}
     
XImageInfo *imageToXImage(disp, scrn, visual, ddepth, image, private_cmap, fit, options)
     Display      *disp;
     int           scrn;
     Visual       *visual; /* visual to use */
     unsigned int  ddepth; /* depth of the visual to use */
     Image        *image;
     unsigned int  private_cmap;
     unsigned int  fit;
     ImageOptions *options;
{
  float        display_gamma = globals.display_gamma;
  unsigned int  verbose = globals.verbose;
  Pixel        *redvalue, *greenvalue, *bluevalue;
  unsigned int  a, b, c=0, newmap, x, y, linelen, dpixlen, dbits;
  XColor        xcolor;
  XGCValues     gcv;
  XImageInfo   *ximageinfo;
  Image        *orig_image;
  boolean	dogamma=FALSE;
  int           gammamap[256];

  CURRFUNC("imageToXImage");

  /* set up to adjust the image gamma for the display gamma we've got */
  if(GAMMA_NOT_EQUAL(display_gamma,image->gamma) && !(BITMAPP(image))) {
    make_gamma(display_gamma/image->gamma,gammamap);
    dogamma = TRUE;
  }

  xcolor.flags= DoRed | DoGreen | DoBlue;
  redvalue= greenvalue= bluevalue= NULL;
  orig_image= image;
  ximageinfo= (XImageInfo *)lmalloc(sizeof(XImageInfo));
  ximageinfo->disp= disp;
  ximageinfo->scrn= scrn;
  ximageinfo->depth= 0;
  ximageinfo->drawable= None;
  ximageinfo->index= NULL;
  ximageinfo->rootimage= FALSE;	/* assume not */
  ximageinfo->foreground= ximageinfo->background= 0;
  ximageinfo->gc= NULL;
  ximageinfo->ximage= NULL;

  /* process image based on type of visual we're sending to
   */

  switch (image->type) {
  case ITRUE:
    switch (visual->class) {
    case TrueColor:
    case DirectColor:
      /* goody goody */
      break;
    default:
      if (visual->map_entries > 2) {
	Image *dimage;
	int ncols = visual->map_entries;
	if(ncols >= 256)
          ncols -= DEFAULT_COLORS;	/* allow for some default colors */
	dimage = reduce(image, ncols, options->colordither, display_gamma, verbose);
        if(dimage != image && orig_image != image)
          freeImage(image);
        image = dimage;
        if(GAMMA_NOT_EQUAL(display_gamma,image->gamma)) {
          make_gamma(display_gamma/image->gamma,gammamap);
          dogamma = TRUE;
        }
      }
      else {	/* it must be monochrome */
        Image *dimage;
        dimage= dither(image, verbose);
        if(dimage != image && orig_image != image)
          freeImage(image);
        image = dimage;
      }
    }
    break;

  case IRGB:
    switch(visual->class) {
    case TrueColor:
    case DirectColor:
      /* no problem, we handle this just fine */
      break;
    default:
      if (visual->map_entries <= 2) {	/* monochrome */
        Image *dimage;
	dimage= dither(image, verbose);
        if(dimage != image && orig_image != image)
          freeImage(image);
        image = dimage;
      }
      break;
    }

  case IBITMAP:
    /* no processing ever needs to be done for bitmaps */
    break;
  }

  /* do color allocation
   */

  switch (visual->class) {
  case TrueColor:
  case DirectColor:
    { Pixel pixval;
      unsigned int redcolors, greencolors, bluecolors;
      unsigned int redstep, greenstep, bluestep;
      unsigned int redbottom, greenbottom, bluebottom;
      unsigned int redtop, greentop, bluetop;

      redvalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);
      greenvalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);
      bluevalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);

      if (visual == DefaultVisual(disp, scrn))
	ximageinfo->cmap= DefaultColormap(disp, scrn);
      else
	ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					  visual, AllocNone);

      retry_direct: /* tag we hit if a DirectColor allocation fails on
		     * default colormap */

      /* calculate number of distinct colors in each band
       */

      redcolors= greencolors= bluecolors= 1;
      for (pixval= 1; pixval; pixval <<= 1) {
	if (pixval & visual->red_mask)
	  redcolors <<= 1;
	if (pixval & visual->green_mask)
	  greencolors <<= 1;
	if (pixval & visual->blue_mask)
	  bluecolors <<= 1;
      }
      
      /* sanity check
       */

      if ((redcolors > visual->map_entries) ||
	  (greencolors > visual->map_entries) ||
	  (bluecolors > visual->map_entries)) {
	fprintf(stderr, "Warning: inconsistency in color information (this may be ugly)\n");
      }

      redstep= 256 / redcolors;
      greenstep= 256 / greencolors;
      bluestep= 256 / bluecolors;
      redbottom= greenbottom= bluebottom= 0;
      for (a= 0; a < visual->map_entries; a++) {
	if (redbottom < 256)
	  redtop= redbottom + redstep;
	if (greenbottom < 256)
	  greentop= greenbottom + greenstep;
	if (bluebottom < 256)
	  bluetop= bluebottom + bluestep;

	xcolor.red= (redtop - 1) << 8;
	xcolor.green= (greentop - 1) << 8;
	xcolor.blue= (bluetop - 1) << 8;
	if (! XAllocColor(disp, ximageinfo->cmap, &xcolor)) {

	  /* if an allocation fails for a DirectColor default visual then
	   * we should create a private colormap and try again.
	   */

	  if ((visual->class == DirectColor) &&
	      (visual == DefaultVisual(disp, scrn))) {
	    ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					      visual, AllocNone);
	    goto retry_direct;
	  }

	  /* something completely unexpected happened
	   */

	  fprintf(stderr, "imageToXImage: XAllocColor failed on a TrueColor/Directcolor visual\n");
          lfree((byte *)redvalue);
          lfree((byte *)greenvalue);
          lfree((byte *)bluevalue);
          lfree((byte *)ximageinfo);
	  return(NULL);
	}

	/* fill in pixel values for each band at this intensity
	 */

	while ((redbottom < 256) && (redbottom < redtop))
	  redvalue[redbottom++]= xcolor.pixel & visual->red_mask;
	while ((greenbottom < 256) && (greenbottom < greentop))
	  greenvalue[greenbottom++]= xcolor.pixel & visual->green_mask;
	while ((bluebottom < 256) && (bluebottom < bluetop))
	  bluevalue[bluebottom++]= xcolor.pixel & visual->blue_mask;
      }
    }
    break;

  default:	/* Not TrueColor or DirectColor */
  retry: /* this tag is used when retrying because we couldn't get a fit */
    ximageinfo->index= (Pixel *)lmalloc(sizeof(Pixel) * (image->rgb.used+NOFLASH_COLORS));

    /* private_cmap flag is invalid if not a dynamic visual
     */

    switch (visual->class) {
    case StaticColor:
    case StaticGray:
      private_cmap= 0;
    }

    /* get the colormap to use.
     */

    if (private_cmap) { /* user asked us to use a private cmap */
      newmap= 1;
      fit= 0;
    }
    else if ((visual == DefaultVisual(disp, scrn)) ||
	     (visual->class == StaticGray) ||
	     (visual->class == StaticColor)) {

      /* if we're using the default visual, try to alloc colors shareable.
       * otherwise we're using a static visual and should treat it
       * accordingly.
       */

      if (visual == DefaultVisual(disp, scrn))
	ximageinfo->cmap= DefaultColormap(disp, scrn);
      else
	ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					  visual, AllocNone);
      newmap= 0;

      /* allocate colors shareable (if we can)
       */

      for (a= 0; a < image->rgb.used; a++) {
        if (dogamma) {
	  xcolor.red= GAMMA16(*(image->rgb.red + a));
	  xcolor.green= GAMMA16(*(image->rgb.green + a));
	  xcolor.blue= GAMMA16(*(image->rgb.blue + a));
        }
        else {
	  xcolor.red= *(image->rgb.red + a);
	  xcolor.green= *(image->rgb.green + a);
	  xcolor.blue= *(image->rgb.blue + a);
        }
	if (! XAllocColor(disp, ximageinfo->cmap, &xcolor))
	  if ((visual->class == StaticColor) ||
	      (visual->class == StaticGray)) {
	    printf("imageToXImage: XAllocColor failed on a static visual\n");
	    ximageinfo->no = a;
            freeXImage(image, ximageinfo);
	    return(NULL);
	  }
	  else {

	    /* we can't allocate the colors shareable so free all the colors
	     * we had allocated and create a private colormap (or fit
	     * into the default cmap if `fit' is true).
	     */

	    XFreeColors(disp, ximageinfo->cmap, ximageinfo->index, a, 0);
            ximageinfo->no = 0;
	    newmap= 1;
	    break;
	  }
	*(ximageinfo->index + a)= xcolor.pixel;
      }
    ximageinfo->no = a;		/* number of pixels allocated in default visual */
    }
    else {
      newmap= 1;
      fit= 0;
    }

    if (newmap) {
      if (fit) {
        /* fit the image into the map we have.  Fitting the
         * colors is hard, we have to:
         *  1. count the available colors using XAllocColorCells.
         *  2. reduce the depth of the image to fit.
         *  3. grab the server so no one can goof with the colormap.
         *  4. free the colors we just allocated.
         *  5. allocate the colors again shareable.
         *  6. ungrab the server and continue on our way.
         */

	if (verbose)
	  printf("  Fitting image into default colormap\n");

        for (a= 0; a < image->rgb.used; a++) /* count entries we got */
	  if (! XAllocColorCells(disp, ximageinfo->cmap, FALSE, NULL, 0,
			         ximageinfo->index + a, 1))
	    break;

	if (a <= 2) {
          Image *dimage;
	  if (verbose) {
	    printf("  Cannot fit into default colormap, dithering...");
	    fflush(stdout);
	  }
	  dimage= dither(image, 0);
          if(dimage != image && orig_image != image)
            freeImage(image);
          image = dimage;
	  if (verbose)
	    printf("done\n");
	  fit= 0;
          if(a > 0)
            XFreeColors(disp, ximageinfo->cmap, ximageinfo->index, a, 0);
	  lfree(ximageinfo->index);
          ximageinfo->index = NULL;
	  goto retry;
	}

        if (a < image->rgb.used) {	/* if image has too many colors, reduce it */
          Image *dimage;
	  dimage= reduce(image, a, options->colordither, display_gamma, verbose);
          if(dimage != image && orig_image != image)
            freeImage(image);
          image = dimage;
          if(GAMMA_NOT_EQUAL(display_gamma,image->gamma)) { /* reduce may have changed it */
            make_gamma(display_gamma/image->gamma,gammamap);
            dogamma = TRUE;
            }
        }

	XGrabServer(disp);	/* stop someone else snarfing the colors */
        XFreeColors(disp, ximageinfo->cmap, ximageinfo->index, a, 0);
	for (a= 0; a < image->rgb.used; a++) {
          if(dogamma) {
	    xcolor.red= GAMMA16(*(image->rgb.red + a));
	    xcolor.green= GAMMA16(*(image->rgb.green + a));
	    xcolor.blue= GAMMA16(*(image->rgb.blue + a));
          }
          else {
	    xcolor.red= *(image->rgb.red + a);
	    xcolor.green= *(image->rgb.green + a);
	    xcolor.blue= *(image->rgb.blue + a);
          }

	  /* if this fails we're in trouble
	   */

	  if (! XAllocColor(disp, ximageinfo->cmap, &xcolor)) {
	    printf("XAllocColor failed while fitting colormap!\n");
	    XUngrabServer(disp);
	    ximageinfo->no = a;
            freeXImage(image, ximageinfo);
	    return(NULL);
	  }
	  *(ximageinfo->index + a)= xcolor.pixel;
	}
	XUngrabServer(disp);
	ximageinfo->no = a;

      } else {	/* not fit */
      /*
       * we create a private cmap and allocate
       * the colors writable.
       */
	if (verbose)
	  printf("  Using private colormap\n");

	/* create new colormap
	 */

	ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					  visual, AllocNone);
        for (a= 0; a < image->rgb.used; a++) /* count entries we got */
	  if (! XAllocColorCells(disp, ximageinfo->cmap, FALSE, NULL, 0,
			         ximageinfo->index + a, 1))
	    break;

        if (a == 0) {
	  fprintf(stderr, "imageToXImage: Color allocation failed!\n");
	  lfree(ximageinfo->index);
          ximageinfo->index = NULL;
          freeXImage(image, ximageinfo);
	  return(NULL);
        }

        if (a < image->rgb.used) {	/* if image has too many colors, reduce it */
          Image *dimage;
	  dimage= reduce(image, a, options->colordither, display_gamma, verbose);
          if(dimage != image && orig_image != image)
            freeImage(image);
          image = dimage;
          if(GAMMA_NOT_EQUAL(display_gamma,image->gamma)) { /* reduce may have changed it */
            make_gamma(display_gamma/image->gamma,gammamap);
            dogamma = TRUE;
            }
        }

	/* See if we can allocate some more colors for a few */
	/* entries from the default color map to reduce flashing.*/
	for(c=0; c < NOFLASH_COLORS; c++) {
	  if (! XAllocColorCells(disp, ximageinfo->cmap, FALSE, NULL, 0,
			       ximageinfo->index + a + c, 1))
	    break;
	}
	for(b=0; b < c; b++) {	/* set default colors */
	  xcolor.pixel = *(ximageinfo->index + b);
	  XQueryColor(disp, DefaultColormap(disp, scrn), &xcolor);
	  XStoreColor(disp, ximageinfo->cmap, &xcolor);
	}
        if(dogamma) {
	  for (b=0; b < a; b++) {
	    xcolor.pixel= *(ximageinfo->index + c + b);
	    xcolor.red= GAMMA16(*(image->rgb.red + b));
	    xcolor.green= GAMMA16(*(image->rgb.green + b));
	    xcolor.blue= GAMMA16(*(image->rgb.blue + b));
	    XStoreColor(disp, ximageinfo->cmap, &xcolor);
          }
        }
        else {
	  for (b=0; b < a; b++) {
	    xcolor.pixel= *(ximageinfo->index + c + b);
	    xcolor.red= *(image->rgb.red + b);
	    xcolor.green= *(image->rgb.green + b);
	    xcolor.blue= *(image->rgb.blue + b);
	    XStoreColor(disp, ximageinfo->cmap, &xcolor);
          }
	}
      ximageinfo->no = a + c;
      }
    }
    break;
  }

  /* create an XImage and related colormap based on the image type
   * we have.
   */

  if (verbose) {
    printf("  Building XImage...");
    fflush(stdout);
  }


  switch (image->type) {
  case IBITMAP:
    { byte *data;

      /* we copy the data to be more consistent
       */

      linelen = ((image->width + 7) / 8);
      data= lmalloc(linelen * image->height);
      bcopy(image->data, data, linelen * image->height);

      gcv.function= GXcopy;
      ximageinfo->ximage= XCreateImage(disp, visual, 1, XYBitmap,
				       0, (char *)data, image->width, image->height,
				       8, linelen);
      ximageinfo->depth= ddepth;
      if(visual->class == DirectColor || visual->class == TrueColor) {
        Pixel pixval;
        dbits= bitsPerPixelAtDepth(disp, scrn, ddepth);
        dpixlen= (dbits + 7) / 8;
        pixval= redvalue[image->rgb.red[0] >> 8] |
                greenvalue[image->rgb.green[0] >> 8] |
                bluevalue[image->rgb.blue[0] >> 8];
        ximageinfo->background = pixval;
        pixval= redvalue[image->rgb.red[1] >> 8] |
                greenvalue[image->rgb.green[1] >> 8] |
                bluevalue[image->rgb.blue[1] >> 8];
        ximageinfo->foreground = pixval;
      }
      else {	/* Not Direct or True Color */
        ximageinfo->foreground= *((ximageinfo->index + c) + 1);
        ximageinfo->background= *(ximageinfo->index + c);
      }
      ximageinfo->ximage->bitmap_bit_order= MSBFirst;
      ximageinfo->ximage->byte_order= MSBFirst;
      break;
    }

  case IRGB:
  case ITRUE:

    /* modify image data to match visual and colormap
     */

    dbits= bitsPerPixelAtDepth(disp, scrn, ddepth);	/* bits per pixel */
    ximageinfo->depth= ddepth;
    dpixlen= (dbits + 7) / 8;	/* bytes per pixel */

    switch (visual->class) {
    case DirectColor:
    case TrueColor:
      { byte *data, *destptr, *srcptr;
	Pixel pixval, newpixval;

	ximageinfo->ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0,
					  NULL, image->width, image->height,
					  8, image->width * dpixlen);

	data= (byte *)lmalloc(image->width * image->height * dpixlen);
	ximageinfo->ximage->data= (char *)data;
	destptr= data;
	srcptr= image->data;
	switch (image->type) {
	case ITRUE:
          if(image->pixlen == 3 && dpixlen ==3) /* most common */
            if(dogamma)
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  newpixval= redvalue[GAMMA8(*srcptr++)] |
          	  greenvalue[GAMMA8(*srcptr++)] |
          	  bluevalue[GAMMA8(*srcptr++)];
                  valToMem(newpixval, destptr, 3);
                  destptr += 3;
                }
            else
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  newpixval= redvalue[*srcptr++] |
          	  greenvalue[*srcptr++] |
          	  bluevalue[*srcptr++];
                  valToMem(newpixval, destptr, 3);
                  destptr += 3;
                }
          else	/* less common */
            if(dogamma)
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  pixval= memToVal(srcptr, image->pixlen);
                  newpixval= redvalue[GAMMA8(TRUE_RED(pixval))] |
          	  greenvalue[GAMMA8(TRUE_GREEN(pixval))] |
          	  bluevalue[GAMMA8(TRUE_BLUE(pixval))];
                  valToMem(newpixval, destptr, dpixlen);
                  srcptr += image->pixlen;
                  destptr += dpixlen;
                }
            else
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  pixval= memToVal(srcptr, image->pixlen);
                  newpixval= redvalue[TRUE_RED(pixval)] |
          	  greenvalue[TRUE_GREEN(pixval)] |
          	  bluevalue[TRUE_BLUE(pixval)];
                  valToMem(newpixval, destptr, dpixlen);
                  srcptr += image->pixlen;
                  destptr += dpixlen;
                }
	  break;
	case IRGB:
           if(image->pixlen == 1 && dpixlen ==3) /* most usual case */
            if(dogamma)
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  pixval= memToVal(srcptr, 1);
                  pixval= redvalue[GAMMA8(image->rgb.red[pixval] >> 8)] |
          	  greenvalue[GAMMA8(image->rgb.green[pixval] >> 8)] |
          	  bluevalue[GAMMA8(image->rgb.blue[pixval] >> 8)];
                  valToMem(pixval, destptr, 3);
                  srcptr += 1;
                  destptr += 3;
                }
            else
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  pixval= memToVal(srcptr, 1);
                  pixval= redvalue[image->rgb.red[pixval] >> 8] |
          	  greenvalue[image->rgb.green[pixval] >> 8] |
          	  bluevalue[image->rgb.blue[pixval] >> 8];
                  valToMem(pixval, destptr, 3);
                  srcptr += 1;
                  destptr += 3;
                }
          else	/* more general case */
            if(dogamma)
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  pixval= memToVal(srcptr, image->pixlen);
                  pixval= redvalue[GAMMA8(image->rgb.red[pixval] >> 8)] |
          	  greenvalue[GAMMA8(image->rgb.green[pixval] >> 8)] |
          	  bluevalue[GAMMA8(image->rgb.blue[pixval] >> 8)];
                  valToMem(pixval, destptr, dpixlen);
                  srcptr += image->pixlen;
                  destptr += dpixlen;
                }
            else
              for (y= 0; y < image->height; y++)
                for (x= 0; x < image->width; x++) {
                  pixval= memToVal(srcptr, image->pixlen);
                  pixval= redvalue[image->rgb.red[pixval] >> 8] |
          	  greenvalue[image->rgb.green[pixval] >> 8] |
          	  bluevalue[image->rgb.blue[pixval] >> 8];
                  valToMem(pixval, destptr, dpixlen);
                  srcptr += image->pixlen;
                  destptr += dpixlen;
                }
	  break;
	default: /* something's broken */
	  printf("Unexpected image type for DirectColor/TrueColor visual!\n");
	  exit(0);
	}
	ximageinfo->ximage->byte_order= MSBFirst; /* trust me, i know what
						   * i'm talking about */
	break;
      }
    default:

      /* only IRGB images make it this far.
       */

      /* if our XImage doesn't have modulus 8 bits per pixel, it's unclear
       * how to pack bits so we instead use an XYPixmap image.  this is
       * slower.
       */

      if (dbits % 8) {
	byte *data, *destdata, *destptr, *srcptr, mask;
	Pixel pixmask, pixval;

	linelen= (image->width + 7) / 8;
	ximageinfo->ximage = XCreateImage(disp, visual, ddepth, XYPixmap, 0,
					  NULL, image->width, image->height,
					  8, linelen);

	data= (byte *)lmalloc(image->width * image->height * dpixlen);
	ximageinfo->ximage->data= (char *)data;
	bzero(data, image->width * image->height * dpixlen);
	ximageinfo->ximage->bitmap_bit_order= MSBFirst;
	ximageinfo->ximage->byte_order= MSBFirst;
	for (a= 0; a < dbits; a++) {
	  pixmask= 1 << a;
	  destdata= data + ((dbits - a - 1) * image->height * linelen);
	  srcptr= image->data;
	  for (y= 0; y < image->height; y++) {
	    destptr= destdata + (y * linelen);
	    *destptr= 0;
	    mask= 0x80;
	    for (x= 0; x < image->width; x++) {
	      pixval= memToVal(srcptr, image->pixlen);
	      srcptr += image->pixlen;
	      if (ximageinfo->index[c + pixval] & pixmask)
		*destptr |= mask;
	      mask >>= 1;
	      if (mask == 0) {
		mask= 0x80;
		destptr++;
	      }
	    }
	  }
	}
      }
      else {
	byte *data, *srcptr, *destptr;

	ximageinfo->ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0,
					  NULL, image->width, image->height,
					  8, image->width*dpixlen);

	data= (byte *)lmalloc(image->width * image->height * dpixlen);
	ximageinfo->ximage->data= (char *)data;
	ximageinfo->ximage->byte_order= MSBFirst; /* trust me, i know what
						   * i'm talking about */
	srcptr= image->data;
	destptr= data;
        if(image->pixlen == 1 && dpixlen == 1)	/* most common */
          for (y= 0; y < image->height; y++)
            for (x= 0; x < image->width; x++) {
              *destptr = ximageinfo->index[c + *srcptr];
              srcptr ++;
              destptr ++;
            }
        else	/* less common */
          for (y= 0; y < image->height; y++)
            for (x= 0; x < image->width; x++) {
              register unsigned long temp;
              temp = memToVal(srcptr, image->pixlen);
              valToMem(ximageinfo->index[c + temp], destptr, dpixlen);
              srcptr += image->pixlen;
              destptr += dpixlen;
            }
      }
      break;
    }
  }

  if (verbose)
    printf("done\n");

  if (redvalue) {
    lfree((byte *)redvalue);
    lfree((byte *)greenvalue);
    lfree((byte *)bluevalue);
  }
  if (verbose && dogamma)
    printf("  Have adjusted image from %4.2f to display gamma of %4.2f\n",image->gamma,display_gamma);
  if (image != orig_image)
    freeImage(image);
  return(ximageinfo);
}

/* Given an XImage and a drawable, move a rectangle from the Ximage
 * to the drawable.
 */

void sendXImage(ximageinfo, src_x, src_y, dst_x, dst_y, w, h)
     XImageInfo  *ximageinfo;
     int          src_x, src_y, dst_x, dst_y;
     unsigned int w, h;
{
  XGCValues gcv;

  /* build and cache the GC
   */

  if (!ximageinfo->gc) {
    gcv.function= GXcopy;
    if (ximageinfo->ximage->depth == 1) {
      gcv.foreground= ximageinfo->foreground;
      gcv.background= ximageinfo->background;
      ximageinfo->gc= XCreateGC(ximageinfo->disp, ximageinfo->drawable,
				GCFunction | GCForeground | GCBackground,
				&gcv);
    }
    else
      ximageinfo->gc= XCreateGC(ximageinfo->disp, ximageinfo->drawable,
				GCFunction, &gcv);
  }
  XPutImage(ximageinfo->disp, ximageinfo->drawable, ximageinfo->gc,
	    ximageinfo->ximage, src_x, src_y, dst_x, dst_y, w, h);
}

/* free up anything cached in the local Ximage structure.
 */

void freeXImage(image, ximageinfo)
     Image        *image;
     XImageInfo   *ximageinfo;
{
  if (ximageinfo->index != NULL) {	/* if we allocated colors */
    if (ximageinfo->no > 0 && !ximageinfo->rootimage)	/* don't free root colors */
      XFreeColors(ximageinfo->disp, ximageinfo->cmap, ximageinfo->index, ximageinfo->no, 0);
    lfree(ximageinfo->index);
  }
  if (ximageinfo->gc)
    XFreeGC(ximageinfo->disp, ximageinfo->gc);
  lfree((byte *)ximageinfo->ximage->data);
  ximageinfo->ximage->data= NULL;
  XDestroyImage(ximageinfo->ximage);
  lfree((byte *)ximageinfo);
  /* should we free private color map to ??? */
}

#else	/* else we are the MSDOS version */

#endif
