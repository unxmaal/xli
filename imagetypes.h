/* #ident	"@(#)x11:contrib/clients/xloadimage/imagetypes.h 6.11 94/07/29 Labtam" */
/* imagetypes.h:
 *
 * supported image types and the imagetypes array declaration.  when you
 * add a new image type, only the makefile and this header need to be
 * changed.
 *
 * jim frost 10.15.89
 */

Image *facesLoad();
Image *pbmLoad();
Image *sunRasterLoad();
Image *gifLoad();
Image *jpegLoad();
Image *rleLoad();
Image *bmpLoad();
Image *pcdLoad();
Image *xwdLoad();
Image *xbitmapLoad();
Image *xpixmapLoad();
Image *g3Load();
Image *fbmLoad();
Image *pcxLoad();
Image *imgLoad();
Image *macLoad();
Image *cmuwmLoad();
Image *mcidasLoad();
Image *tgaLoad();
#ifdef USE_TIFF
Image *tiffLoad();
#endif

int facesIdent();
int pbmIdent();
int sunRasterIdent();
int gifIdent();
int jpegIdent();
int rleIdent();
int bmpIdent();
int pcdIdent();
int xwdIdent();
int xbitmapIdent();
int xpixmapIdent();
int g3Ident();
int fbmIdent();
int pcxIdent();
int imgIdent();
int macIdent();
int cmuwmIdent();
int mcidasIdent();
int tgaIdent();
#ifdef USE_TIFF
int tiffIdent();
#endif

/* some of these are order-dependent
 */

struct {
  int    (*identifier)(); /* print out image info if this kind of image */
  Image *(*loader)();     /* load image if this kind of image */
  char  *name;            /* name of this image format */
} ImageTypes[] = {
  fbmIdent,       fbmLoad,       "FBM Image",
  sunRasterIdent, sunRasterLoad, "Sun Rasterfile",
  cmuwmIdent,     cmuwmLoad,     "CMU WM Raster",
  pbmIdent,       pbmLoad,       "Portable Bit Map (PBM, PGM, PPM)",
  facesIdent,     facesLoad,     "Faces Project",
#ifdef USE_TIFF
  tiffIdent,      tiffLoad,      "TIFF Image",
#endif
  gifIdent,       gifLoad,       "GIF Image",
  jpegIdent,      jpegLoad,      "JFIF style jpeg Image",
  rleIdent,       rleLoad,       "Utah RLE Image",
  bmpIdent,       bmpLoad,       "Windows, OS/2 RLE Image",
  pcdIdent,       pcdLoad,       "Photograph on CD Image",
  xwdIdent,       xwdLoad,       "X Window Dump",
  tgaIdent,       tgaLoad,       "Targa Image",
  mcidasIdent,    mcidasLoad,    "McIDAS areafile",
  g3Ident,        g3Load,        "G3 FAX Image",
  pcxIdent,       pcxLoad,       "PC Paintbrush Image",
  imgIdent,       imgLoad,       "GEM Bit Image",
  macIdent,       macLoad,       "MacPaint Image",
  xpixmapIdent,   xpixmapLoad,   "X Pixmap",
  xbitmapIdent,   xbitmapLoad,   "X Bitmap",
  NULL,           NULL,          NULL
};
