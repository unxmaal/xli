/* #ident	"@(#)x11:contrib/clients/xloadimage/ddxli.c 1.4 93/07/29 Labtam" */
/*
 * General Device dependent code for xli.
 *
 * Author; Graeme W. Gill
 */

#include "xli.h"
#include <ctype.h>

#ifndef MSDOS	/* X11 Version */

/* Initialise a display info structure to default values */
xliDefaultDispinfo(dinfo)
DisplayInfo *dinfo;
	{
	dinfo->width = 0;
	dinfo->height = 0;
	dinfo->disp= NULL;
	dinfo->scrn= 0;
	dinfo->colormap = 0;
	}

/* open up a display and screen, and stick the info away */
/* Return FALSE on failure */
boolean
xliOpenDisplay(dinfo,name)
DisplayInfo *dinfo;	/* display record */
char *name;			/* name of display */
	{
	Display	*disp;
	int      scrn;
    if (! (disp= XOpenDisplay(globals.dname)))
	  return FALSE;	/* failed */
	scrn= DefaultScreen(disp);
	dinfo->disp = disp;
	dinfo->scrn = scrn;
	dinfo->colormap = DefaultColormap(disp, scrn);
	dinfo->width = DisplayWidth(disp, scrn);
	dinfo->height = DisplayHeight(disp, scrn);
	XSetErrorHandler(errorHandler);
	return TRUE;
	}

/* report display name when connection fails */
char *
xliDisplayName(name)
char *name;
	{
    return XDisplayName(name);
	}

xliCloseDisplay(dinfo)
DisplayInfo *dinfo;	/* display record */
	{
	XCloseDisplay(dinfo->disp);
	}

/*
 * simple X11 error handler.  this provides us with some kind of error recovery.
 */

int errorHandler(disp, error)
     Display *disp;
     XErrorEvent *error;
{ char errortext[BUFSIZ];

  XGetErrorText(disp, error->error_code, errortext, BUFSIZ);
  fprintf(stderr, "xli: X Error: %s on 0x%x\n",
	  errortext, error->resourceid);
  if (globals._Xdebug) /* if -debug mode is enabled, dump a core when we hit this */
    abort();
  else
    return(0);
}

/* Return the default visual class */
int
xliDefaultVisual()
	{
    return (DefaultVisual(globals.dinfo.disp, globals.dinfo.scrn)->class);
	}

/* return the default depth of the default visual */
xliDefaultDepth()
	{
    return DefaultDepth(globals.dinfo.disp, globals.dinfo.scrn);
	}

/* Print some information about the display */
tellAboutDisplay(dinfo)
DisplayInfo *dinfo;
	{
	Screen *screen;
	int a,b;
	if (dinfo->disp)
		{
		screen= ScreenOfDisplay(dinfo->disp, dinfo->scrn);
		printf("Server: %s Version %d\n", ServerVendor(dinfo->disp), VendorRelease(dinfo->disp));
		printf("Depths and visuals supported:\n");
		for (a= 0; a < screen->ndepths; a++)
			{
			printf("%2d:", screen->depths[a].depth);
			for (b= 0; b < screen->depths[a].nvisuals; b++)
				printf(" %s", nameOfVisualClass(screen->depths[a].visuals[b].class));
				printf("\n");
			}
		}
	else
		printf("[No information on server; error occurred before connection]\n");
	}

#else /* MSDOS version */

static init_colors();

/* Initialise a display info structure to default values */
xliDefaultDispinfo(dinfo)
DisplayInfo *dinfo;
	{
	dinfo->base = NULL;
	dinfo->width = 0;
	dinfo->height = 0;
	dinfo->span = 0;
	}

/* open up a display and screen, and stick the info away */
/* Return FALSE on fail */
boolean
xliOpenDisplay(dinfo,name)
DisplayInfo *dinfo;	/* display record */
char *name;			/* name of display */
	{
#ifndef GDEBUG
	GrSetMode(GR_default_graphics);
	dinfo->base = (unsigned char *)0xd0000000;
	dinfo->width = GrSizeX();
	dinfo->height = GrSizeY();
	dinfo->span = GrSizeX();
#else
	dinfo->base = (unsigned char *)0xd0000000;
	dinfo->width = 1024;
	dinfo->height = 768;
	dinfo->span = 1024;
#endif
	init_colors();
	/* alocate two default colors, black and white, for use */
	/* by the default DOS text out. */
	/* We know these will be indexes 0 and 7 */
	xliAllocColorIndex(0,0,0,0);
	xliAllocColorIndex(7,180,180,180);
	return TRUE;
	}

/* report display name when connection fails */
char *
xliDisplayName(name)
char *name;
	{
    return "VGA";
	}

xliCloseDisplay(dinfo)
DisplayInfo *dinfo;	/* display record */
	{
#ifndef GDEBUG
	GrSetMode(GR_default_text);
#endif
	}

/* Return the default visual class */
int
xliDefaultVisual()
	{
	return PseudoColor;
	}

/* return the default depth of the default visual */
xliDefaultDepth()
	{
    return 8;
	}

/* Print some information about the display */
tellAboutDisplay(dinfo)
DisplayInfo *dinfo;
	{
	if (dinfo->base != NULL)
		{
		printf("VGA display, resolurion %d by %d\n",dinfo->width,dinfo->height);
		printf("Depths %d, Visual %s\n",xliDefaultDepth(),nameOfVisualClass(xliDefaultVisual()));
		}
	else
		printf("[No information on display; error occurred before opening]\n");
	}

/* ---------------------------------------------- */
/* Routines to keep track of VGA color allocation */
#define VGA_COLORS 256
static int vga_colors[VGA_COLORS];
static int last_color;

static
init_colors()
	{
	int i;
	for(i=0; i< VGA_COLORS; i++)
		vga_colors[i] = -1;
	last_color = 0;
	}

/* allocate a color and return the index */
int
xliAllocColor(red,green,blue)
unsigned char red;
unsigned char green;
unsigned char blue;
	{
	int i;
	for(i=last_color; vga_colors[i]>=0 && i < VGA_COLORS; i++);
	if(i >= VGA_COLORS)
		return -1;	/* none left */
	_GrSetColor(i, red, green, blue);
	vga_colors[i] = i;
	last_color = i+1;
	return i;
	}

/* allocate a color of a specified index */
/* return -1 if fails */
int
xliAllocColorIndex(i,red,green,blue)
int i;
unsigned char red;
unsigned char green;
unsigned char blue;
	{
	if(vga_colors[i] >= 0)
		return -1;	/* already used */
	_GrSetColor(i, red, green, blue);
	vga_colors[i] = i;
	return i;
	}

xliFreeColor(i)
int i;
	{
	vga_colors[i] = -1;
	if(i < last_color)
		last_color = i;
	}

/* count and return the number of free colors */
xliAvailColors()
	{
	int i,j;
	for(j=i=0; i< VGA_COLORS; i++)
		if(vga_colors[i] < 0)
			j++;
	return j;
	}

/* --------------------------------- */
/* routines to manage a polled timer */

#include <sys/types.h>

static time_t timeout = 0;

xliSetTimeout(secs)
int secs;
	{
	if(secs == 0)
		timeout = 0;	/* turn it off */
	else
		timeout = time(NULL) + secs;
	}

xliHasTimedout()
	{
	return (timeout != 0 && (time(NULL) >= timeout));
	}

#endif	/* MSDOS version */

/* --------------------------------------------- */
/* A standalone color name to RGB lookup routine */


	typedef struct {
		char *name;
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	} xliXColorTableEntry;

#include "rgbtab.h"		/* Equivalent of an X color table */

/* parse an X like color. */
int xliParseXColor(dinfo,spec,xlicolor)
DisplayInfo *dinfo;
char *spec;
xliXColor *xlicolor;
	{
	int stat;
#ifndef MSDOS
	XColor xcolor;
	if (dinfo->disp)
		{
		xcolor.pixel = xlicolor->pixel;
		xcolor.red = xlicolor->red;
		xcolor.green = xlicolor->green;
		xcolor.blue = xlicolor->blue;
		xcolor.flags = xlicolor->flags;
		stat = XParseColor(globals.dinfo.disp, globals.dinfo.colormap, spec, &xcolor);
		xlicolor->pixel = xcolor.pixel;
		xlicolor->red = xcolor.red;
		xlicolor->green = xcolor.green;
		xlicolor->blue = xcolor.blue;
		xlicolor->flags = xcolor.flags;
		}
	else	/* else use standalone version */
#endif
		stat = saParseXColor(dinfo,spec,xlicolor);

	return stat;
	}

saParseXColor(dinfo,spec,xlicolor)
DisplayInfo *dinfo;
char *spec;
xliXColor *xlicolor;
	{
	int i,n,r,g,b;
	xliXColorTableEntry *tp;

	xlicolor->red = 0;
	xlicolor->green = 0;
	xlicolor->blue = 0;
	n = strlen(spec);
	if (n > 0)
		{
		for (i = 0; i < n; i++) {
			if (isupper (spec[i]))
				spec[i] = tolower (spec[i]);
			}
		if (spec[0] == '#')
			{
			switch (n-1)
				{			
				case 3:
					r = hstoi(spec+1,1);
					g = hstoi(spec+2,1);
					b = hstoi(spec+3,1);
					if (r >= 0 && g >= 0 && b >= 0)
						{
						xlicolor->red = (r * 65535) / 15;
						xlicolor->green = (g * 65535) / 15;
						xlicolor->blue = (b * 65535) / 15;
						return;
						}
					break;
				case 6:
					r = hstoi(spec+1,2);
					g = hstoi(spec+3,2);
					b = hstoi(spec+5,2);
					if (r >= 0 && g >= 0 && b >= 0)
						{
						xlicolor->red = (r * 65535) / 255;
						xlicolor->green = (g * 65535) / 255;
						xlicolor->blue = (b * 65535) / 255;
						return;
						}
					break;
				case 9:
					r = hstoi(spec+1,3);
					g = hstoi(spec+4,3);
					b = hstoi(spec+7,3);
					if (r >= 0 && g >= 0 && b >= 0)
						{
						xlicolor->red = (r * 65535) / 4095;
						xlicolor->green = (g * 65535) / 4095;
						xlicolor->blue = (b * 65535) / 4095;
						return;
						}
					break;
				case 12:
					r = hstoi(spec+1,4);
					g = hstoi(spec+5,4);
					b = hstoi(spec+9,4);
					if (r >= 0 && g >= 0 && b >= 0)
						{
						xlicolor->red = r;
						xlicolor->green = g;
						xlicolor->blue = b;
						return;
						}
					break;
				}			
			}
		else	/* else its' a color name */
			{	/* Do a linear search for it (binary search would be better) */
			for (n=0,tp = &xliXColorTable[0]; tp->name[0] != '\000'; tp++)
				{
				if(strcmp(spec,tp->name)==0)
					{
					xlicolor->red = ((int)tp->red * 65535) / 255;
					xlicolor->green = ((int)tp->green * 65535) / 255;
					xlicolor->blue = ((int)tp->blue * 65535) / 255;
					return 1;
					}
				}
			}
		}
		/* default - return black */
		xlicolor->red = 0;
		xlicolor->green = 0;
		xlicolor->blue = 0;
		return 0;
	}


/* gamma correct an xliXColor */
void
xliGammaCorrectXColor(xlicolor,gamma)
xliXColor *xlicolor;
double gamma;
	{
	xlicolor->red = (int)(0.5 + 65535.0 * pow( (double)xlicolor->red / 65535.0, gamma ));
	xlicolor->green = (int)(0.5 + 65535.0 * pow( (double)xlicolor->green / 65535.0, gamma ));
	xlicolor->blue = (int)(0.5 + 65535.0 * pow( (double)xlicolor->blue / 65535.0, gamma ));
	}










