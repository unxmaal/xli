/* #ident   "@(#)x11:contrib/clients/xloadimage/jpeg.c 1.19 94/07/29 Labtam" */
/*
 * jpeg - This file provides interface routines to connect xli to
 * the jpeg decoding routines taken from the Independent JPEG Group's
 * free JPEG software.  See build-jpeg for more information.
 *
 * This code is based on example.c from the IJG v4 distribution,
 * and the original xloadimage/xli jpeg.c for the IJG v1 distribution.
 */

#include "xli.h"	/* xli declarations */
#include "jpeg.h"		/* IJG declarations */
#include <setjmp.h>		/* need setjmp/longjmp */

/* JFIF defines the gamma of pictures to be 1.0. */
/* Unfortunately no-one takes any notice (sigh), */
/* and the majority of images are like gifs - they */
/* are adjusted to look ok on a "typical" monitor */
/* using a gamma naive viewer/windowing system. */
/* We give in to the inevitable here and use 2.2 */
#define RETURN_GAMMA 2.2	/* Request JPEG load with gamma of 2.2 */

static ZFILE * input_file;	/* tells input routine where to read JPEG */
struct Decompress_info_struct gcinfo;

/* Error-catching routines */

static char *filename;
static unsigned int verbose;
static unsigned int identify;
static jmp_buf setjmp_buffer;	/* for return to caller */
static external_methods_ptr emethods; /* needed for access to message_parm */

static void
trace_message (msgtext)
     char *msgtext;
{
  fprintf(stderr, "jpegLoad: %s - ", filename);
  fprintf(stderr, msgtext,
	  emethods->message_parm[0], emethods->message_parm[1],
	  emethods->message_parm[2], emethods->message_parm[3],
	  emethods->message_parm[4], emethods->message_parm[5],
	  emethods->message_parm[6], emethods->message_parm[7]);
  fprintf(stderr, "\n");	/* there is no \n in the format string! */
}

static void
error_exit (msgtext)
     char *msgtext;
{
  trace_message(msgtext);	/* report the error message */
  zunread(input_file,gcinfo.next_input_byte,gcinfo.bytes_in_buffer);
  (*emethods->free_all) ();	/* clean up memory allocation & temp files */
  longjmp(setjmp_buffer, 1);	/* return control to outer routine */
}


/* Output-acceptance routines */

static Image *image;		/* xli image being returned */
static int rows_put;		/* Number of rows copied to image */

static void
output_init (cinfo)
     decompress_info_ptr cinfo;
/* Initialize for output */
{
  int i;

  if (cinfo->out_color_space == CS_GRAYSCALE) {
    image = newRGBImage(cinfo->image_width,cinfo->image_height,8);
    image->title = dupString(filename);
    /* set a linear map */
    for(i=0;i<256;i++) {
      *(image->rgb.red + i) = 
	*(image->rgb.green + i) = 
	  *(image->rgb.blue + i) = i<<8;
    }
    image->rgb.used = 256;
  } else if (cinfo->out_color_space == CS_RGB) {
    image = newTrueImage(cinfo->image_width,cinfo->image_height);
    image->title = dupString(filename);
  } else {
    image = NULL;
    ERREXIT(cinfo->emethods, "Can't cope with JPEG image colorspace");
  }
  rows_put = 0;
}


static void
put_color_map (cinfo, num_colors, colormap)
     decompress_info_ptr cinfo;
     int num_colors;
     JSAMPARRAY colormap;
/* Write the color map -- should not be called */
{
  fprintf(stderr, "put_color_map called: there's a bug here somewhere!\n");
}


static void
put_pixel_rows (cinfo, num_rows, pixel_data)
     decompress_info_ptr cinfo;
     int num_rows;
     JSAMPIMAGE pixel_data;
/* Write some rows of output data */
{
  register unsigned char *bufp;
  register JSAMPROW ptr0, ptr1, ptr2;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  if (cinfo->out_color_space == CS_GRAYSCALE) {
    bufp = image->data + rows_put * width;
    /* Assume JSAMPLE == chars */
    for (row = 0; row < num_rows; row++) {
      bcopy(pixel_data[0][row],bufp,width);
      bufp += width;
    }
  } else {
    bufp = image->data + rows_put * width * 3;
    for (row = 0; row < num_rows; row++) {
      ptr0 = pixel_data[0][row];
      ptr1 = pixel_data[1][row];
      ptr2 = pixel_data[2][row];
      for (col = width; col > 0; col--) {
	*bufp++ = *ptr0++;
	*bufp++ = *ptr1++;
	*bufp++ = *ptr2++;
      }
    }
  }
  rows_put += num_rows;
}


static void
output_term (cinfo)
     decompress_info_ptr cinfo;
/* Finish up at the end of the output */
{
  zunread(input_file,cinfo->next_input_byte,cinfo->bytes_in_buffer);
}


/* Input-file-reading routine */

static int
read_jpeg_data (cinfo)
     decompress_info_ptr cinfo;
{
  cinfo->next_input_byte = cinfo->input_buffer + MIN_UNGET;

  cinfo->bytes_in_buffer = zread(input_file,
				 cinfo->next_input_byte,
				 JPEG_BUF_SIZE);
  
  if (cinfo->bytes_in_buffer <= 0) {
    WARNMS(cinfo->emethods, "Premature EOF in JPEG file");
    (*emethods->free_all) ();	/* clean up memory allocation & temp files */
    longjmp(setjmp_buffer, 1);	/* return control to outer routine */
  }

  return JGETC(cinfo);
}


/*  Required control-hook routine */


static void
d_ui_method_selection (cinfo)
     decompress_info_ptr cinfo;
{
  /* if grayscale input, force grayscale output; */
  /* else leave the output colorspace as set by main routine. */
  if (cinfo->jpeg_color_space == CS_GRAYSCALE)
    cinfo->out_color_space = CS_GRAYSCALE;

  /* Create display of image parameters */
  if (verbose) {
    printf("%s is a %dx%d JPEG image, color space ", filename,
	   cinfo->image_width, cinfo->image_height);
    switch (cinfo->jpeg_color_space) {
    case CS_UNKNOWN:
      printf("Unknown");
      break;
    case CS_GRAYSCALE:
      printf("Grayscale");
      break;
    case CS_RGB:
      printf("RGB");
      break;
    case CS_YCbCr:
      printf("YCbCr");
      break;
    case CS_YIQ:
      printf("YIQ");
      break;
    case CS_CMYK:
      printf("CMYK");
      break;
    }
    printf(", %d comp%s,", cinfo->num_components,
	   cinfo->num_components ? "s." : ".");
    if (cinfo->arith_code)
      printf(" Arithmetic coding\n");
    else
      printf(" Huffman coding\n");
  }

  /* Turn off caching beyond this point of the file */
  znocache(input_file);

  /* If we only wanted to identify the image, abort now */
  if (identify) {
    (*emethods->free_all) ();	/* clean up memory allocation & temp files */
    longjmp(setjmp_buffer, 10);	/* return control with success code */
  }

  /* select output routines */
  cinfo->methods->output_init = output_init;
  cinfo->methods->put_color_map = put_color_map;
  cinfo->methods->put_pixel_rows = put_pixel_rows;
  cinfo->methods->output_term = output_term;
}


/* Main control routine for loading */


Image *
jpegLoad (fullname, image_ops, vbose)
     char *fullname;
     ImageOptions *image_ops;
     unsigned int vbose;
{
  char  *name = image_ops->name;
  struct Decompress_methods_struct dc_methods;
  struct External_methods_struct e_methods;
  int ljrt;

  CURRFUNC("jpegLoad");
  input_file = zopen(fullname);	/* Open the input file */
  if (input_file == NULL)
    return NULL;

  /* Quick check to see if file starts with JPEG SOI marker */
  if (zgetc(input_file) != 0xFF || zgetc(input_file) != 0xD8) {
    zclose(input_file);
    return NULL;
  }
  zrewind(input_file);

  filename = name;		/* copy parms to static vars */
  verbose = vbose;
  identify = 0;

  image = NULL;			/* in case we fail before creating image */

  gcinfo.methods = &dc_methods;	/* links to method structs */
  gcinfo.emethods = &e_methods;
  emethods = &e_methods;	/* save struct addr for possible access */
  e_methods.error_exit = error_exit; /* supply error-exit routine */
  e_methods.trace_message = trace_message; /* supply trace-message routine */
  e_methods.trace_level = 0;	/* default = no tracing */
  e_methods.num_warnings = 0;	/* no warnings emitted yet */
  e_methods.first_warning_level = 0; /* display first corrupt-data warning */
  e_methods.more_warning_level = 3; /* but suppress additional ones */

  /* prepare setjmp context for possible exit from error_exit */
  ljrt = setjmp(setjmp_buffer);
  if (ljrt != 0) {
    /* If we get here, the JPEG code has signaled an error. */
    zclose(input_file);
    if (ljrt != 2 && rows_put > 0) { /* Return as much of the image as we could get. */
      /* Attempt to read trailing opts - mightn't work if jpeg has sucked all of file */
      read_trail_opt(image_ops,input_file,image,verbose);
      image->gamma = RETURN_GAMMA;
      return image;
    }
    else {	/* No data loaded into image, or not JPEG error */
      if (image)
        freeImage(image);
      return NULL;
    }
  }

  jselmemmgr(&e_methods);	/* select std memory allocation routines */

  /* Set up default decompression parameters. */
  j_d_defaults(&gcinfo, TRUE);
  gcinfo.output_gamma = RETURN_GAMMA;

  /* Override default methods */
  dc_methods.d_ui_method_selection = d_ui_method_selection;
  dc_methods.read_jpeg_data = read_jpeg_data;

  /* Set up to read a JFIF or baseline-JPEG file. */
  /* This is the only JPEG file format currently supported. */
  jselrjfif(&gcinfo);

  /* Here we go! */
  jpeg_decompress(&gcinfo);

  read_trail_opt(image_ops,input_file,image,verbose);
  zclose(input_file);		/* Done, close the input file */

  image->gamma = RETURN_GAMMA;
  return image;
}


/* Main control routine for identifying JPEG without loading */


int
jpegIdent (fullname, name)
     char *fullname, *name;
{
  struct Decompress_methods_struct dc_methods;
  struct External_methods_struct e_methods;

  CURRFUNC("jpegIdent");
  input_file = zopen(fullname);	/* Open the input file */
  if (input_file == NULL)
    return 0;

  /* Quick check to see if file starts with JPEG SOI marker */
  if (zgetc(input_file) != 0xFF || zgetc(input_file) != 0xD8) {
    zclose(input_file);
    return 0;
  }
  zrewind(input_file);

  /* We want to find and display the image dimensions, and also
   * verify that the header markers are not corrupt.  To do this,
   * we fire up the JPEG decoder as normal, but when d_ui_method_selection
   * is called, we abort the process by longjmp'ing back here.
   * This works nicely since the headers are all read at that point.
   */

  filename = name;		/* copy parms to static vars */
  verbose = 1;
  identify = 1;

  gcinfo.methods = &dc_methods;	/* links to method structs */
  gcinfo.emethods = &e_methods;
  emethods = &e_methods;	/* save struct addr for possible access */
  e_methods.error_exit = error_exit; /* supply error-exit routine */
  e_methods.trace_message = trace_message; /* supply trace-message routine */
  e_methods.trace_level = 0;	/* default = no tracing */
  e_methods.num_warnings = 0;	/* no warnings emitted yet */
  e_methods.first_warning_level = 0; /* display first corrupt-data warning */
  e_methods.more_warning_level = 3; /* but suppress additional ones */

  /* prepare setjmp context for expected exit via longjmp */
  switch (setjmp(setjmp_buffer)) {
  case 0:
    /* First time thru, keep going */
    break;
  case 10:
    /* Successful exit from d_ui_method_selection; return A-OK */
    zclose(input_file);
    return 1;
  default:
    /* If we get here, the JPEG code has signaled an error. */
    /* Return 0 since error in the headers means the image is unloadable. */
    zclose(input_file);
    return 0;
  }

  jselmemmgr(&e_methods);	/* select std memory allocation routines */

  /* Set up default decompression parameters. */
  j_d_defaults(&gcinfo, TRUE);

  /* Override default methods */
  dc_methods.d_ui_method_selection = d_ui_method_selection;
  dc_methods.read_jpeg_data = read_jpeg_data;

  /* Set up to read a JFIF or baseline-JPEG file. */
  /* This is the only JPEG file format currently supported. */
  jselrjfif(&gcinfo);

  /* Here we go! */
  jpeg_decompress(&gcinfo);

  /* Don't expect to get here since d_ui_method_selection should do longjmp */

  zclose(input_file);
  return 0;
}
