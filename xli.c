/* #ident	"@(#)x11:contrib/clients/xloadimage/xli.c 1.4 94/07/29 Labtam" */
/* xli.c:
 *
 * generic image loader for X11
 *
 * Graeme Gill 93/2/24                                                           <
 *                                                                               <
 * Based on xloadimage by
 * jim frost 09.27.89
 *
 * Copyright 1989, 1990, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "xli.h"
#include "patchlevel"
#include <signal.h>

GlobalsRec  globals;    /* General options, flags and global info. */

/* used for the -default option.  this is the root weave bitmap with
 * the bits in the order that xli likes.
 */

#define root_weave_width 4
#define root_weave_height 4
static byte root_weave_bits[] = {
  0xe0, 0xb0, 0xd0, 0x70
};

/* the real thing
 */

main(argc, argv)
     int argc;
     char *argv[];
{ Image          *dispimage;      /* image that will be sent to the display */
  Image          *newimage;       /* new image we're loading */
  Image          *tmpimage;
  ImageOptions    images[MAXIMAGES + 1]; /* list of image w/ options to load */
  int             a;
  unsigned int    imagecount;     /* number of images in ImageName array */
  unsigned int    winwidth, winheight;  /* geometry of image */
  ImageOptions    persist_ops;    /* options which persist from image to image*/
  char		  switchval;

  CURRFUNC("main");

#if defined(MSDOS) && defined(DO_ARG_EXP)
	argexp(&argc,&argv);	/* do wildcard expansion - better than go32 */
#endif

  /* initialise some variables
   */
  persist_ops.border= NULL;
  persist_ops.bright= 0;
  persist_ops.colordither= 0;
  persist_ops.colors= 0;     /* remaining images */
  persist_ops.delay= -1;
  persist_ops.dither= 0;
  persist_ops.gamma= 0.0;	/* nonsense value */
  persist_ops.normalize= 0;
  persist_ops.smooth= 0;
  persist_ops.xpmkeyc= 0;		/* none */
  persist_ops.xzoom= 0;
  persist_ops.yzoom= 0;

  /* set up internal error handlers
   */

  signal(SIGSEGV, internalError);
  signal(SIGBUS, internalError);
  signal(SIGFPE, internalError);
  signal(SIGILL, internalError);

#if defined(_AIX) && defined(_IBMR2)
  /* the RS/6000 (AIX 3.1) has a new signal, SIGDANGER, which you get
   * when memory is exhausted.  since malloc() can overcommit, it's a good
   * idea to trap this one.
   */
  signal(SIGDANGER, memoryExhausted);
#endif

  if (argc < 2)
    usage(argv[0]);

  /* defaults and other initial settings.  some of these depend on what
   * our name was when invoked.
   */

  loadPathsAndExts();

  xliDefaultDispinfo(&globals.dinfo);
  globals.argv0 = argv[0];		/* so we can get at this elsewhere */
  globals._Xdebug = FALSE;
  globals._DumpCore = FALSE;
  globals.onroot= FALSE;
  globals.verbose= TRUE;
  if (!strcmp(tail(argv[0]), "xview")) {
    globals.onroot= FALSE;
    globals.verbose= TRUE;
  }
  else if (!strcmp(tail(argv[0]), "xsetbg")) {
    globals.onroot= TRUE;
    globals.verbose= FALSE;
  }

  {	/* Get display gamma from the environment if we can */
    char *gstr;
    globals.display_gamma = 0.0;
    if ((gstr = getenv("DISPLAY_GAMMA")) != NULL) {
	  globals.display_gamma= atof(gstr);
	}
    if (globals.display_gamma > 20.0	/* Ignore silly values */
     || globals.display_gamma < 0.05)
    globals.display_gamma = DEFAULT_DISPLAY_GAMMA;
  }
  globals.dname= NULL;
  globals.fit= FALSE;
  globals.fillscreen= FALSE;
  globals.fullscreen= FALSE;
  globals.go_to= NULL;
  globals.do_fork= FALSE;
  globals.identify= FALSE;
  globals.install= FALSE;
  globals.private_cmap= FALSE;
  globals.use_pixmap= FALSE;
  globals.set_default= FALSE;
#ifndef MSDOS
  globals.dest_window= 0;
#endif
  globals.user_geometry = NULL;
  globals.visual_class= -1;
  winwidth= winheight= 0;

#define INIT_IMAGE_OPTS(istr){	\
    istr.name= NULL;		\
    istr.loader_idx= -1;	\
    istr.atx= istr.aty= 0;	\
    istr.ats= FALSE;		\
    istr.border= NULL;		\
    istr.bright= 0;		\
    istr.center= 0;		\
    istr.clipx= istr.clipy= 0;	\
    istr.clipw= istr.cliph= 0;	\
    istr.colordither= 0;	\
    istr.colors= 0;		\
    istr.delay= -1;		\
    istr.dither= 0;		\
    istr.gamma= 0.0;		\
    istr.gray= 0;		\
    istr.merge= 0;		\
    istr.normalize= 0;		\
    istr.rotate= 0;		\
    istr.smooth= 0;		\
    istr.title= NULL;		\
    istr.xpmkeyc= 0;		\
    istr.xzoom= istr.yzoom= 0;	\
    istr.fg= istr.bg= NULL;	\
    istr.done_to= FALSE;	\
    istr.to_argv= NULL; }

  imagecount= 0;
  for (a= 0; a < MAXIMAGES; a++) {
    INIT_IMAGE_OPTS(images[a]);
  }
  /*
   * Process the command line
   */
  for (a= 1; a < argc; a++) {
    OptionId opid;
    opid = optionNumber(argv[a]);
	
    if (opid ==  OPT_BADOPT) {
      printf("%s: Bad option\n", argv[a]);
      usage(argv[0]);
      /* NOTREACHED */
    } else
    if (opid ==  OPT_NOTOPT || opid == NAME) {
      if (opid == NAME && argv[++a] == NULL)
        continue;
      if (imagecount == MAXIMAGES)
	printf("%s: Too many images (ignoring)\n", argv[a]);
      else {
	images[imagecount].name= argv[a];
	images[imagecount].border= persist_ops.border;
	images[imagecount].bright= persist_ops.bright;
	images[imagecount].colordither= persist_ops.colordither;
	images[imagecount].colors= persist_ops.colors;
	images[imagecount].delay= persist_ops.delay;
	images[imagecount].dither= persist_ops.dither;
	images[imagecount].gamma= persist_ops.gamma;
	images[imagecount].normalize= persist_ops.normalize;
	images[imagecount].smooth= persist_ops.smooth;
	images[imagecount].xpmkeyc= persist_ops.xpmkeyc;
	images[imagecount].xzoom= persist_ops.xzoom;
	images[imagecount].yzoom= persist_ops.yzoom;
	imagecount++;
      }
    } else
    if (opid == OPT_SHORTOPT) {
      printf("%s: Not enough characters to identify option\n", argv[a]);
      usage(argv[0]);
      /* NOTREACHED */
    } else
    if (isGeneralOption(opid)) {

      /* process options general to everything
       */

      a += doGeneralOption(opid, &argv[a], &persist_ops, &images[imagecount]);

    } else
    if (isLocalOption(opid)) {

      /* process options local to an image
       */

      a += doLocalOption(opid, &argv[a], TRUE, &persist_ops, &images[imagecount]);

    } else {

      /* this should not happen!
       */

      printf("%s: Internal error parsing arguments\n", argv[0]);
      exit(1);
    }
  }

  if (globals._DumpCore) {	/* Don't trap signals, - dump core */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
  }

  if (globals.fit && (globals.visual_class != -1)) {
    printf("-fit and -visual options are mutually exclusive (ignoring -visual)\n");
    globals.visual_class= -1;
  }

  if (!imagecount && !globals.set_default) /* NO-OP from here on */
    exit(0);

  if (globals.identify) {                    /* identify the named image(s) */
    for (a= 0; a < imagecount; a++)
      identifyImage(images[a].name);
    exit(0);
  }

  /* start talking to the display
   */

  if (! xliOpenDisplay(&globals.dinfo,globals.dname)) {
    fprintf(stderr,"%s: Cannot open display '%s'\n", globals.argv0,xliDisplayName(globals.dname));
    exit(1);
  }

  /* background ourselves if the user asked us to
   */

  if (globals.do_fork)
#ifdef MSDOS
    fprintf(stderr,"Warning: -fork option not supported\n");
#else
    switch(fork()) {
    case -1:
      perror("fork");
      /* FALLTHRU */
    case 0:
      break;
    default:
      exit(0);
    }
#endif /* !MSDOS */

  /* handle -default option.  this resets the colormap and loads the
   * default root weave.
   */

  if (globals.set_default) {
    byte *old_data;
    ImageOptions ioptions;
    boolean tempverb;

    INIT_IMAGE_OPTS(ioptions);
    dispimage= newBitImage(root_weave_width, root_weave_height);
    dispimage->gamma = globals.display_gamma;
    old_data= dispimage->data;
    dispimage->data= root_weave_bits;
    tempverb = globals.verbose;
    globals.verbose = FALSE;
    imageOnRoot(&globals.dinfo, dispimage, &ioptions);
    globals.verbose = tempverb;
    dispimage->data= old_data;
    freeImage(dispimage);
    if (!imagecount) { /* all done */
      xliCloseDisplay(&globals.dinfo);
      exit(0);
    }
  }

  dispimage= NULL;

  /* load in each named image
   */

  for (a= 0; a < imagecount; a++) {
    if (! (newimage= loadImage(&images[a], globals.verbose)))
      continue;
    
    if (images[a].gamma != 0.0)		/* override image gamma */
      newimage->gamma = images[a].gamma;
    else if(newimage->gamma == 0.0)
      defaultgamma(newimage);

    /* Process any other options that need doing here */
    if (images[a].border) {
      xliParseXColor(&globals.dinfo, images[a].border, &images[a].bordercol);
      xliGammaCorrectXColor(&images[a].bordercol, DEFAULT_DISPLAY_GAMMA);
    }

    /* if this is the first image and we're putting it on the root window
     * in fullscreen mode, set the zoom factors to something reasonable.
     */

    if ((a == 0) &&
    ((globals.onroot && globals.fullscreen) || globals.fillscreen) &&
	(images[0].xzoom == 0) && (images[0].yzoom == 0) &&
	(images[0].center == 0)) {
    
      if ((newimage->width > globals.dinfo.width) ||
	  (newimage->height > globals.dinfo.height)) {
        if (!globals.onroot || !globals.fillscreen)	/* make just fit */
 	  images[0].xzoom= images[0].yzoom=
	    (newimage->width - globals.dinfo.width >
	     newimage->height - globals.dinfo.height ?
	     (unsigned int)((float)globals.dinfo.width / (float)newimage->width * 100.0 + 0.5) :
	     (unsigned int)((float)globals.dinfo.height / (float)newimage->height * 100.0 + 0.5));
        else				/* make fully fill */
 	  images[0].xzoom= images[0].yzoom=
	    (newimage->width - globals.dinfo.width >
	     newimage->height - globals.dinfo.height ?
	     (unsigned int)((float)globals.dinfo.height / (float)newimage->height * 100.0 + 0.5) :
	     (unsigned int)((float)globals.dinfo.width / (float)newimage->width * 100.0 + 0.5));
      }
      else {
        if (!globals.onroot || !globals.fillscreen)	/* make just fit */
	  images[0].xzoom= images[0].yzoom=
	    (globals.dinfo.width - newimage->width <
	     globals.dinfo.height - newimage->height ?
	     (unsigned int)((float)globals.dinfo.width / (float)newimage->width * 100.0 + 0.5) :
	     (unsigned int)((float)globals.dinfo.height / (float)newimage->height * 100.0 + 0.5));
        else				/* make fully fill */
	  images[0].xzoom= images[0].yzoom=
	    (globals.dinfo.width - newimage->width <
	     globals.dinfo.height - newimage->height ?
	     (unsigned int)((float)globals.dinfo.height / (float)newimage->height * 100.0 + 0.5) : 
	     (unsigned int)((float)globals.dinfo.width / (float)newimage->width * 100.0 + 0.5));
      }
    }

    tmpimage= processImage(&globals.dinfo, newimage, &images[a]);
    if (tmpimage != newimage)	/* Dont't keep un-processed image */
	freeImage(newimage);
    newimage = tmpimage;

    if (dispimage) {	/* If there is something to merge with */
      if (images[a].center) {
        images[a].atx= (int)(dispimage->width - newimage->width) / 2;
        images[a].aty= (int)(dispimage->height - newimage->height) / 2;
      }
      if (! dispimage->title) {
	dispimage->title= dupString(newimage->title);
	dispimage->gamma= newimage->gamma;
      }
      tmpimage= merge(dispimage, newimage, images[a].atx, images[a].aty,
		      &images[a]);
      if (dispimage != tmpimage) {
	freeImage(dispimage);
	dispimage= tmpimage;
      }
      freeImage(newimage);
    }

    /* else if this is the first image on root, set its position and any border needed */
    else if (a == 0 && globals.onroot &&
      (winwidth || winheight || images[0].center ||
      images[0].ats || globals.fullscreen ||
      globals.fillscreen)) {
      int atx = images[a].atx,aty = images[a].aty;
      if (!winwidth)
	winwidth= globals.dinfo.width;
      if (!winheight)
        winheight= globals.dinfo.height;

      if (!images[0].ats) {	/* center image by default */
        atx= (int)(winwidth - newimage->width) / 2;
        aty= (int)(winheight - newimage->height) / 2;
      }
      /* use clip to put border around image */
      tmpimage= clip(newimage, -atx, -aty, winwidth, winheight, &images[a]);
      if (tmpimage != newimage) {
	freeImage(newimage);
	newimage= tmpimage;
      }
      dispimage= newimage;
    }
    else
      dispimage= newimage;

    /* if next image is to be merged onto this one, then go on to the next image.
     */

    if (globals.onroot || (((a+1) < imagecount) && (images[a + 1].merge)))
      continue;

    switchval = imageInWindow(&globals.dinfo, dispimage, &images[a], argc, argv);
    switch(switchval) {
    case '\0': /* window got nuked by someone */
      xliCloseDisplay(&globals.dinfo);
      exit(1);
    case '\003':
    case 'q':  /* user quit */
      cleanUpWindow(&globals.dinfo);
      xliCloseDisplay(&globals.dinfo);
      exit(0);
    case ' ':
    case 'f':  /* forwards or */
    case 'n':  /* next image */
      if (a >= (imagecount -1) && globals.go_to != NULL) {
	int b;
	for (b= 0; b < imagecount; b++)
	  if (!strcmp(images[b].name, globals.go_to)) {
	    a= b - 1;
	    goto next_image;
	  }
	fprintf(stderr, "Target for -goto %s was not found\n",
		globals.go_to);
      next_image:
	;
      }
      break;

    case 'b':  /* backwards or */
    case 'p':  /* previous image */
      if (a > 0)
	a -= 2;
      else
	a--;
      break;

    case '.':  /* re-load current image */
      a--;
      break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      /* change gamma same way as acommand line option */
      if (images[a].gamma == 0.0)
        images[a].gamma = dispimage->gamma;
      switch (switchval) {
	int l;
	double jump;
        case '0':	/* set gamma to default image gamma */
 	  images[a].gamma = DEFAULT_DISPLAY_GAMMA;     
	  break;
        case '1':	/* set gamma to 1.0 */
 	  images[a].gamma = 1.0;
	  break;
	default:
	  l = (switchval - '6');
          if (l>=0) l++;
          if (l>0)
            jump = 1.1;
          else
            jump = 0.909;
          if (l<0) l = -l;
          for(l--;l>0;l--) jump *= jump;
          images[a].gamma *= jump;
	  break;
      }
      if (globals.verbose)
	printf("Image Gamma now %f\n",images[a].gamma);
      a--;
      break;

    case 'l':	/* Rotate to the left */
    case 'r':	/* Rotate to the right */
      switch (switchval) {
	int l;
	double jump;
        case 'l':	/* set gamma to default image gamma */
 	  images[a].rotate -= 90;
	  break;
        case 'r':
 	  images[a].rotate += 90;
	  break;
      }
      while (images[a].rotate >= 360)
	images[a].rotate -= 360;
      while (images[a].rotate < 0)
	images[a].rotate += 360;
      if (globals.verbose)
	printf("Image rotation is now %d\n",images[a].rotate);
      a--;
      break;
    }
    freeImage(dispimage);
    dispimage= NULL;
  }

  if (dispimage != NULL && globals.onroot)
    imageOnRoot(&globals.dinfo, dispimage, &images[imagecount-1]);
  xliCloseDisplay(&globals.dinfo);
  exit(0);
}
