/* #ident	"@(#)x11:contrib/clients/xloadimage/jpeglib.c 1.3 93/07/23 Labtam" */
/*
 * The Independent JPEG Group's JPEG software
 * ==========================================
 * 
 * The JPEG routines in this program are taken from the Independent JPEG Group's
 * free JPEG software.  The files named j*.h, j*.c are from the IJG code, except
 * for jpeg.c, which provides an interface between the IJG code and the rest of
 * the program.
 * 
 * Only a subset of the IJG code is included in this program.  You can find the
 * complete IJG distribution at the archive sites listed below.  This file also
 * provides the text of the IJG copyright and terms of distribution, plus some
 * notes on adapting the IJG code for use in xloadimage.
 * 
 * This code is from IJG release 4 of 10-Dec-92.
 * 
 * 
 * LEGAL ISSUES
 * ============
 * 
 * The authors make NO WARRANTY or representation, either express or implied,
 * with respect to this software, its quality, accuracy, merchantability, or
 * fitness for a particular purpose.  This software is provided "AS IS", and you,
 * its user, assume the entire risk as to its quality and accuracy.
 * 
 * This software is copyright (C) 1991, 1992, 1993, Thomas G. Lane.
 * All Rights Reserved except as specified below.
 * 
 * Permission is hereby granted to use, copy, modify, and distribute this
 * software (or portions thereof) for any purpose, without fee, subject to these
 * conditions:
 * (1) If any part of the source code for this software is distributed, then this
 * README file must be included, with this copyright and no-warranty notice
 * unaltered; and any additions, deletions, or changes to the original files
 * must be clearly indicated in accompanying documentation.
 * (2) If only executable code is distributed, then the accompanying
 * documentation must state that "this software is based in part on the work of
 * the Independent JPEG Group".
 * (3) Permission for use of this software is granted only if the user accepts
 * full responsibility for any undesirable consequences; the authors accept
 * NO LIABILITY for damages of any kind.
 * 
 * Permission is NOT granted for the use of any IJG author's name or company name
 * in advertising or publicity relating to this software or products derived from
 * it.  This software may be referred to only as "the Independent JPEG Group's
 * software".
 * 
 * We specifically permit and encourage the use of this software as the basis of
 * commercial products, provided that all warranty or liability claims are
 * assumed by the product vendor.
 * 
 * 
 * ARCHIVE LOCATIONS
 * =================
 * 
 * The "official" archive site for this software is ftp.uu.net (Internet
 * address 137.39.1.9 or 192.48.96.9).  The most recent released version can
 * always be found there in directory graphics/jpeg.  This particular version
 * will be archived as jpegsrc.v4.tar.Z.  If you are on the Internet, you can
 * retrieve files from UUNET by anonymous FTP.  If you don't have FTP access,
 * UUNET's archives are also available via UUCP; contact postmaster@uunet.uu.net
 * for information on retrieving files that way.
 * 
 * Numerous Internet sites maintain copies of the UUNET files; in particular,
 * you can probably find a copy at any site that archives comp.sources.misc
 * submissions.  However, only ftp.uu.net is guaranteed to have the latest
 * official version.
 * 
 * You can also obtain this software from CompuServe, in the GRAPHSUPPORT forum
 * (GO PICS), library 15; this version will be file jpsrc4.zip.  Again,
 * CompuServe is not guaranteed to have the very latest version.
 * 
 * The JPEG FAQ (Frequently Asked Questions) article is a useful source of
 * general information about JPEG.  It is updated constantly and therefore
 * is not included in this distribution.  The FAQ is posted every two weeks
 * to Usenet newsgroups comp.graphics, news.answers, and other groups.  You
 * can always obtain the latest version from the news.answers archive at
 * rtfm.mit.edu (18.172.1.27).  By FTP, fetch /pub/usenet/news.answers/jpeg-faq.
 * If you don't have FTP, send e-mail to mail-server@rtfm.mit.edu with body
 * "send usenet/news.answers/jpeg-faq".
 * 
 * 
 * CONVERSION NOTES
 * ================
 * 
 * These notes are directed towards xloadimage/xli, but might also be of use for
 * other applications that need to use the free JPEG code.
 * 
 * The objective here is to provide a JPEG library which is of minimal size and
 * is pre-configured for easy installation on typical Unix machines.  The
 * standard IJG distribution is not suitable because it provides many functions
 * not needed by xloadimage/xli, it requires some thought to install, and it is
 * larger than the whole of xloadimage/xli :-(.
 * 
 * What we want is a minimal library which is easily generated from the standard
 * IJG files, so that new versions of the IJG code can be dropped in with little
 * work.  We must keep application-specific interfacing routines separate from
 * the files that are derived from the IJG code.
 * 
 * xloadimage/xli only require the core JPEG decoder and JFIF-format file input
 * functions from the IJG library.  The JPEG compressor is not required, nor are
 * the non-JPEG file I/O modules, nor the color quantization modules (since
 * xloadimage/xli have their own quantization code).  Only Unix systems need be
 * supported.  I would further recommend that "block smoothing" not be supported,
 * since it is of little use at typical JPEG quality settings.  This leaves us
 * with the following IJG source files to be included:
 * 
 * 	jdcolor.c jddeflts.c jdhuff.c jdmaster.c jdmcu.c jdpipe.c jdsample.c
 * 	jrevdct.c jutils.c jrdjfif.c jmemmgr.c jmemnobs.c jinclude.h
 * 	jconfig.h jpegdata.h jmemsys.h
 * 
 * These files amount to about 230K in the standard IJG v4 distribution.  There
 * is some unused code that we could remove to pare down the files; for example,
 * compressor-only declarations could be removed from jpegdata.h.  But it's not
 * clear that this is worth the effort.  By my count only about 20K could be
 * saved without careful thought.  Similarly, it would be possible to save a
 * dozen or so K by combining all the files into one .c and one .h file, but this
 * would make it more difficult to generate new versions (hand editing would be
 * needed, and I think there are some duplicate static variables to be worried
 * about).  I'm inclined to say that leaving the files unmodified is worth that
 * much bloat.
 * 
 * Since the surrounding xloadimage code is written in non-ANSI C, we may as well
 * preprocess all the IJG code through ansi2knr before distribution; this way
 * we don't need to include ansi2knr.c.
 * 
 * We will edit jconfig.h and jinclude.h to pre-configure the code for Unix
 * installation.  jconfig.h will be set up as follows:
 *   Delete HAVE_STDC symbol.
 *   #undef PROTO			necessary since we de-ANSIfied the code.
 *   #define HAVE_UNSIGNED_CHAR	widely assumed by rest of xloadimage, anyway.
 *   #define HAVE_UNSIGNED_SHORT	ditto.
 *   CHAR_IS_UNSIGNED		noncritical; leave define commented out.
 *   RIGHT_SHIFT_IS_UNSIGNED	leave define commented out, mention in README.
 *   #define void char		rest of xloadimage doesn't use void*; but we
 * 				will leave this commented out unless necessary.
 *   #define const			rest of xloadimage doesn't use const.
 *   #undef NEED_FAR_POINTERS	we're not MSDOS, thank god.
 *   Delete TWO_FILE_COMMANDLINE, NEED_SIGNAL_CATCHER, DONT_USE_B_MODE.
 * 				only apply to the cjpeg/djpeg main programs.
 *   Delete encoder capability options, file format options except JFIF.
 *   #undef BLOCK_SMOOTHING_SUPPORTED
 *   #undef QUANT_1PASS_SUPPORTED
 *   #undef QUANT_2PASS_SUPPORTED
 * 
 * In jinclude.h: delete most of the autoconfiguring cruft.  Instead we will
 * unconditionally include sys/types.h (so do some other xloadimage files),
 * and will select string.h/strings.h, bcopy/memcpy the same way as in image.h.
 * This will make the jpeg modules behave the same as the rest of the code
 * in regards to system header files.
 * 
 * The above changes are the only ones that have been made to the files derived
 * from the IJG code; they should be easy to re-apply to new versions.
 * 
 * The interface code found in jpeg.c is derived from the IJG "example.c" file.
 * This code should require little or no change when new IJG versions are
 * installed; but check the IJG CHANGELOG for any changes that may have been made
 * to the subroutine library interface.
 */
#line 1 "jdcolor.c"
/*
 * jdcolor.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains output colorspace conversion routines.
 * These routines are invoked via the methods color_convert
 * and colorout_init/term.
 */

#include "jpeg.h"


/**************** YCbCr -> RGB conversion: most common case **************/

/*
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *	R = Y                + 1.40200 * Cr
 *	G = Y - 0.34414 * Cb - 0.71414 * Cr
 *	B = Y + 1.77200 * Cb
 * where Cb and Cr represent the incoming values less MAXJSAMPLE/2.
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 *
 * To avoid floating-point arithmetic, we represent the fractional constants
 * as integers scaled up by 2^16 (about 4 digits precision); we have to divide
 * the products by 2^16, with appropriate rounding, to get the correct answer.
 * Notice that Y, being an integral input, does not contribute any fraction
 * so it need not participate in the rounding.
 *
 * For even more speed, we avoid doing any multiplications in the inner loop
 * by precalculating the constants times Cb and Cr for all possible values.
 * For 8-bit JSAMPLEs this is very reasonable (only 256 entries per table);
 * for 12-bit samples it is still acceptable.  It's not very reasonable for
 * 16-bit samples, but if you want lossless storage you shouldn't be changing
 * colorspace anyway.
 * The Cr=>R and Cb=>B values can be rounded to integers in advance; the
 * values for the G calculation are left scaled up, since we must add them
 * together before rounding.
 */

#ifdef SIXTEEN_BIT_SAMPLES
#define SCALEBITS	14	/* avoid overflow */
#else
#define SCALEBITS	16	/* speedier right-shift on some machines */
#endif
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))

static int * Cr_r_tab;		/* => table for Cr to R conversion */
static int * Cb_b_tab;		/* => table for Cb to B conversion */
static INT32 * Cr_g_tab;	/* => table for Cr to G conversion */
static INT32 * Cb_g_tab;	/* => table for Cb to G conversion */


/*
 * Initialize for colorspace conversion.
 */

METHODDEF void
ycc_rgb_init (cinfo)  decompress_info_ptr cinfo;
{
  INT32 i, x2;
  SHIFT_TEMPS

  Cr_r_tab = (int *) (*cinfo->emethods->alloc_small)
				((MAXJSAMPLE+1) * SIZEOF(int));
  Cb_b_tab = (int *) (*cinfo->emethods->alloc_small)
				((MAXJSAMPLE+1) * SIZEOF(int));
  Cr_g_tab = (INT32 *) (*cinfo->emethods->alloc_small)
				((MAXJSAMPLE+1) * SIZEOF(INT32));
  Cb_g_tab = (INT32 *) (*cinfo->emethods->alloc_small)
				((MAXJSAMPLE+1) * SIZEOF(INT32));

  for (i = 0; i <= MAXJSAMPLE; i++) {
    /* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
    /* The Cb or Cr value we are thinking of is x = i - MAXJSAMPLE/2 */
    x2 = 2*i - MAXJSAMPLE;	/* twice x */
    /* Cr=>R value is nearest int to 1.40200 * x */
    Cr_r_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.40200/2) * x2 + ONE_HALF, SCALEBITS);
    /* Cb=>B value is nearest int to 1.77200 * x */
    Cb_b_tab[i] = (int)
		    RIGHT_SHIFT(FIX(1.77200/2) * x2 + ONE_HALF, SCALEBITS);
    /* Cr=>G value is scaled-up -0.71414 * x */
    Cr_g_tab[i] = (- FIX(0.71414/2)) * x2;
    /* Cb=>G value is scaled-up -0.34414 * x */
    /* We also add in ONE_HALF so that need not do it in inner loop */
    Cb_g_tab[i] = (- FIX(0.34414/2)) * x2 + ONE_HALF;
  }
}


/*
 * Convert some rows of samples to the output colorspace.
 */

METHODDEF void
ycc_rgb_convert (cinfo, num_rows, num_cols, input_data, output_data)  decompress_info_ptr cinfo; int num_rows; long num_cols;
		 JSAMPIMAGE input_data; JSAMPIMAGE output_data;
{
#ifdef SIXTEEN_BIT_SAMPLES
  register INT32 y;
  register UINT16 cb, cr;
#else
  register int y, cb, cr;
#endif
  register JSAMPROW inptr0, inptr1, inptr2;
  register JSAMPROW outptr0, outptr1, outptr2;
  register long col;
  /* copy these pointers into registers if possible */
  register JSAMPLE * range_limit = cinfo->sample_range_limit;
  register int * Crrtab = Cr_r_tab;
  register int * Cbbtab = Cb_b_tab;
  register INT32 * Crgtab = Cr_g_tab;
  register INT32 * Cbgtab = Cb_g_tab;
  int row;
  SHIFT_TEMPS
  
  for (row = 0; row < num_rows; row++) {
    inptr0 = input_data[0][row];
    inptr1 = input_data[1][row];
    inptr2 = input_data[2][row];
    outptr0 = output_data[0][row];
    outptr1 = output_data[1][row];
    outptr2 = output_data[2][row];
    for (col = 0; col < num_cols; col++) {
      y  = GETJSAMPLE(inptr0[col]);
      cb = GETJSAMPLE(inptr1[col]);
      cr = GETJSAMPLE(inptr2[col]);
      /* Note: if the inputs were computed directly from RGB values,
       * range-limiting would be unnecessary here; but due to possible
       * noise in the DCT/IDCT phase, we do need to apply range limits.
       */
      outptr0[col] = range_limit[y + Crrtab[cr]];	/* red */
      outptr1[col] = range_limit[y +			/* green */
				 ((int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr],
						    SCALEBITS))];
      outptr2[col] = range_limit[y + Cbbtab[cb]];	/* blue */
    }
  }
}


/*
 * Finish up at the end of the file.
 */

METHODDEF void
ycc_rgb_term (cinfo)  decompress_info_ptr cinfo;
{
  /* no work (we let free_all release the workspace) */
}


/**************** Cases other than YCbCr -> RGB **************/


/*
 * Initialize for colorspace conversion.
 */

METHODDEF void
null_init (cinfo)  decompress_info_ptr cinfo;
/* colorout_init for cases where no setup is needed */
{
  /* no work needed */
}


/*
 * Color conversion for no colorspace change: just copy the data.
 */

METHODDEF void
null_convert (cinfo, num_rows, num_cols, input_data, output_data)  decompress_info_ptr cinfo; int num_rows; long num_cols;
	      JSAMPIMAGE input_data; JSAMPIMAGE output_data;
{
  short ci;

  for (ci = 0; ci < cinfo->num_components; ci++) {
    jcopy_sample_rows(input_data[ci], 0, output_data[ci], 0,
		      num_rows, num_cols);
  }
}


/*
 * Color conversion for grayscale: just copy the data.
 * This also works for YCbCr/YIQ -> grayscale conversion, in which
 * we just copy the Y (luminance) component and ignore chrominance.
 */

METHODDEF void
grayscale_convert (cinfo, num_rows, num_cols, input_data, output_data)  decompress_info_ptr cinfo; int num_rows; long num_cols;
		   JSAMPIMAGE input_data; JSAMPIMAGE output_data;
{
  jcopy_sample_rows(input_data[0], 0, output_data[0], 0,
		    num_rows, num_cols);
}


/*
 * Finish up at the end of the file.
 */

METHODDEF void
null_term (cinfo)  decompress_info_ptr cinfo;
/* colorout_term for cases where no teardown is needed */
{
  /* no work needed */
}



/*
 * The method selection routine for output colorspace conversion.
 */

GLOBAL void
jseldcolor (cinfo)  decompress_info_ptr cinfo;
{
  /* Make sure num_components agrees with jpeg_color_space */
  switch (cinfo->jpeg_color_space) {
  case CS_GRAYSCALE:
    if (cinfo->num_components != 1)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    break;

  case CS_RGB:
  case CS_YCbCr:
  case CS_YIQ:
    if (cinfo->num_components != 3)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    break;

  case CS_CMYK:
    if (cinfo->num_components != 4)
      ERREXIT(cinfo->emethods, "Bogus JPEG colorspace");
    break;

  default:
    ERREXIT(cinfo->emethods, "Unsupported JPEG colorspace");
    break;
  }

  /* Set color_out_comps and conversion method based on requested space */
  switch (cinfo->out_color_space) {
  case CS_GRAYSCALE:
    cinfo->color_out_comps = 1;
    if (cinfo->jpeg_color_space == CS_GRAYSCALE ||
	cinfo->jpeg_color_space == CS_YCbCr ||
	cinfo->jpeg_color_space == CS_YIQ) {
      cinfo->methods->color_convert = grayscale_convert;
      cinfo->methods->colorout_init = null_init;
      cinfo->methods->colorout_term = null_term;
    } else
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  case CS_RGB:
    cinfo->color_out_comps = 3;
    if (cinfo->jpeg_color_space == CS_YCbCr) {
      cinfo->methods->color_convert = ycc_rgb_convert;
      cinfo->methods->colorout_init = ycc_rgb_init;
      cinfo->methods->colorout_term = ycc_rgb_term;
    } else if (cinfo->jpeg_color_space == CS_RGB) {
      cinfo->methods->color_convert = null_convert;
      cinfo->methods->colorout_init = null_init;
      cinfo->methods->colorout_term = null_term;
    } else
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;

  default:
    /* Permit null conversion from CMYK or YCbCr to same output space */
    if (cinfo->out_color_space == cinfo->jpeg_color_space) {
      cinfo->color_out_comps = cinfo->num_components;
      cinfo->methods->color_convert = null_convert;
      cinfo->methods->colorout_init = null_init;
      cinfo->methods->colorout_term = null_term;
    } else			/* unsupported non-null conversion */
      ERREXIT(cinfo->emethods, "Unsupported color conversion request");
    break;
  }

  if (cinfo->quantize_colors)
    cinfo->final_out_comps = 1;	/* single colormapped output component */
  else
    cinfo->final_out_comps = cinfo->color_out_comps;
}
#line 1 "jddeflts.c"
/*
 * jddeflts.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains optional default-setting code for the JPEG decompressor.
 * User interfaces do not have to use this file, but those that don't use it
 * must know more about the innards of the JPEG code.
 */

#include "jpeg.h"


/* Default do-nothing progress monitoring routine.
 * This can be overridden by a user interface that wishes to
 * provide progress monitoring; just set methods->progress_monitor
 * after j_d_defaults is done.  The routine will be called periodically
 * during the decompression process.
 *
 * During any one pass, loopcounter increases from 0 up to (not including)
 * looplimit; the step size is not necessarily 1.  Both the step size and
 * the limit may differ between passes.  The expected total number of passes
 * is in cinfo->total_passes, and the number of passes already completed is
 * in cinfo->completed_passes.  Thus the fraction of work completed may be
 * estimated as
 *		completed_passes + (loopcounter/looplimit)
 *		------------------------------------------
 *				total_passes
 * ignoring the fact that the passes may not be equal amounts of work.
 *
 * When decompressing, the total_passes figure is an estimate that may be
 * on the high side; completed_passes will jump by more than one if some
 * passes are skipped.
 */

METHODDEF void
progress_monitor (cinfo, loopcounter, looplimit)  decompress_info_ptr cinfo; long loopcounter; long looplimit;
{
  /* do nothing */
}


/*
 * Reload the input buffer after it's been emptied, and return the next byte.
 * See the JGETC macro for calling conditions.  Note in particular that
 * read_jpeg_data may NOT return EOF.  If no more data is available, it must
 * exit via ERREXIT, or perhaps synthesize fake data (such as an RST marker).
 * In the present implementation, we insert an EOI marker; this might not be
 * appropriate for non-JFIF file formats, but it usually allows us to handle
 * a truncated JFIF file.
 *
 * This routine can be overridden by the system-dependent user interface,
 * in case the data source is not a stdio stream or some other special
 * condition applies.  Note, however, that this capability only applies for
 * JFIF or similar serial-access JPEG file formats.  The input file control
 * module for a random-access format such as TIFF/JPEG would most likely
 * override the read_jpeg_data method with its own routine.
 */

METHODDEF int
read_jpeg_data (cinfo)  decompress_info_ptr cinfo;
{
  cinfo->next_input_byte = cinfo->input_buffer + MIN_UNGET;

  cinfo->bytes_in_buffer = (int) JFREAD(cinfo->input_file,
					cinfo->next_input_byte,
					JPEG_BUF_SIZE);
  
  if (cinfo->bytes_in_buffer <= 0) {
    WARNMS(cinfo->emethods, "Premature EOF in JPEG file");
    cinfo->next_input_byte[0] = (char) 0xFF;
    cinfo->next_input_byte[1] = (char) 0xD9; /* EOI marker */
    cinfo->bytes_in_buffer = 2;
  }

  return JGETC(cinfo);
}



/* Default parameter setup for decompression.
 *
 * User interfaces that don't choose to use this routine must do their
 * own setup of all these parameters.  Alternately, you can call this
 * to establish defaults and then alter parameters selectively.  This
 * is the recommended approach since, if we add any new parameters,
 * your code will still work (they'll be set to reasonable defaults).
 *
 * standard_buffering should be TRUE to cause an input buffer to be allocated
 * (the normal case); if FALSE, the user interface must provide a buffer.
 * This option is most useful in the case that the buffer must not be freed
 * at the end of an image.  (For example, when reading a sequence of images
 * from a single file, the remaining data in the buffer represents the
 * start of the next image and mustn't be discarded.)  To handle this,
 * allocate the input buffer yourself at startup, WITHOUT using alloc_small
 * (probably a direct call to malloc() instead).  Then pass FALSE on each
 * call to j_d_defaults to ensure the buffer state is not modified.
 *
 * If the source of the JPEG data is not a stdio stream, override the
 * read_jpeg_data method with your own routine after calling j_d_defaults.
 * You can still use the standard buffer if it's appropriate.
 *
 * CAUTION: if you want to decompress multiple images per run, it's necessary
 * to call j_d_defaults before *each* call to jpeg_decompress, since subsidiary
 * structures like the quantization tables are automatically freed during
 * cleanup.
 */

GLOBAL void
j_d_defaults (cinfo, standard_buffering)  decompress_info_ptr cinfo; boolean standard_buffering;
/* NB: the external methods must already be set up. */
{
  short i;

  /* Initialize pointers as needed to mark stuff unallocated. */
  /* Outer application may fill in default tables for abbreviated files... */
  cinfo->comp_info = NULL;
  for (i = 0; i < NUM_QUANT_TBLS; i++)
    cinfo->quant_tbl_ptrs[i] = NULL;
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    cinfo->dc_huff_tbl_ptrs[i] = NULL;
    cinfo->ac_huff_tbl_ptrs[i] = NULL;
  }
  cinfo->colormap = NULL;

  /* Default to RGB output */
  /* UI can override by changing out_color_space */
  cinfo->out_color_space = CS_RGB;
  cinfo->jpeg_color_space = CS_UNKNOWN;
  /* Setting any other value in jpeg_color_space overrides heuristics in */
  /* jrdjfif.c.  That might be useful when reading non-JFIF JPEG files, */
  /* but ordinarily the UI shouldn't change it. */
  
  /* Default to no gamma correction of output */
  cinfo->output_gamma = 1.0;
  
  /* Default to no color quantization */
  cinfo->quantize_colors = FALSE;
  /* but set reasonable default parameters for quantization, */
  /* so that turning on quantize_colors is sufficient to do something useful */
  cinfo->two_pass_quantize = TRUE;
  cinfo->use_dithering = TRUE;
  cinfo->desired_number_of_colors = 256;
  
  /* Default to no smoothing */
  cinfo->do_block_smoothing = FALSE;
  cinfo->do_pixel_smoothing = FALSE;
  
  /* Allocate memory for input buffer, unless outer application provides it. */
  if (standard_buffering) {
    cinfo->input_buffer = (char *) (*cinfo->emethods->alloc_small)
					((size_t) (JPEG_BUF_SIZE + MIN_UNGET));
    cinfo->bytes_in_buffer = 0;	/* initialize buffer to empty */
  }

  /* Install standard buffer-reloading method (outer code may override). */
  cinfo->methods->read_jpeg_data = read_jpeg_data;

  /* Install default do-nothing progress monitoring method. */
  cinfo->methods->progress_monitor = progress_monitor;
}
#line 1 "jdhuff.c"
/*
 * jdhuff.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains Huffman entropy decoding routines.
 * These routines are invoked via the methods entropy_decode
 * and entropy_decode_init/term.
 */

#include "jpeg.h"


/* Static variables to avoid passing 'round extra parameters */

static decompress_info_ptr dcinfo;

static INT32 get_buffer;	/* current bit-extraction buffer */
static int bits_left;		/* # of unused bits in it */
static boolean printed_eod;	/* flag to suppress multiple end-of-data msgs */

LOCAL void
fix_huff_tbl (htbl)  HUFF_TBL * htbl;
/* Compute derived values for a Huffman table */
{
  int p, i, l, si;
  char huffsize[257];
  UINT16 huffcode[257];
  UINT16 code;
  
  /* Figure C.1: make table of Huffman code length for each symbol */
  /* Note that this is in code-length order. */

  p = 0;
  for (l = 1; l <= 16; l++) {
    for (i = 1; i <= (int) htbl->bits[l]; i++)
      huffsize[p++] = (char) l;
  }
  huffsize[p] = 0;
  
  /* Figure C.2: generate the codes themselves */
  /* Note that this is in code-length order. */
  
  code = 0;
  si = huffsize[0];
  p = 0;
  while (huffsize[p]) {
    while (((int) huffsize[p]) == si) {
      huffcode[p++] = code;
      code++;
    }
    code <<= 1;
    si++;
  }

  /* We don't bother to fill in the encoding tables ehufco[] and ehufsi[], */
  /* since they are not used for decoding. */

  /* Figure F.15: generate decoding tables */

  p = 0;
  for (l = 1; l <= 16; l++) {
    if (htbl->bits[l]) {
      htbl->valptr[l] = p;	/* huffval[] index of 1st sym of code len l */
      htbl->mincode[l] = huffcode[p]; /* minimum code of length l */
      p += htbl->bits[l];
      htbl->maxcode[l] = huffcode[p-1];	/* maximum code of length l */
    } else {
      htbl->maxcode[l] = -1;
    }
  }
  htbl->maxcode[17] = 0xFFFFFL;	/* ensures huff_DECODE terminates */
}


/*
 * Code for extracting the next N bits from the input stream.
 * (N never exceeds 15 for JPEG data.)
 * This needs to go as fast as possible!
 *
 * We read source bytes into get_buffer and dole out bits as needed.
 * If get_buffer already contains enough bits, they are fetched in-line
 * by the macros get_bits() and get_bit().  When there aren't enough bits,
 * fill_bit_buffer is called; it will attempt to fill get_buffer to the
 * "high water mark", then extract the desired number of bits.  The idea,
 * of course, is to minimize the function-call overhead cost of entering
 * fill_bit_buffer.
 * On most machines MIN_GET_BITS should be 25 to allow the full 32-bit width
 * of get_buffer to be used.  (On machines with wider words, an even larger
 * buffer could be used.)  However, on some machines 32-bit shifts are
 * relatively slow and take time proportional to the number of places shifted.
 * (This is true with most PC compilers, for instance.)  In this case it may
 * be a win to set MIN_GET_BITS to the minimum value of 15.  This reduces the
 * average shift distance at the cost of more calls to fill_bit_buffer.
 */

#ifdef SLOW_SHIFT_32
#define MIN_GET_BITS  15	/* minimum allowable value */
#else
#define MIN_GET_BITS  25	/* max value for 32-bit get_buffer */
#endif

static const int bmask[16] =	/* bmask[n] is mask for n rightmost bits */
  { 0, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
    0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF };


LOCAL int
fill_bit_buffer (nbits)  int nbits;
/* Load up the bit buffer and do get_bits(nbits) */
{
  /* Attempt to load at least MIN_GET_BITS bits into get_buffer. */
  while (bits_left < MIN_GET_BITS) {
    register int c = JGETC(dcinfo);
    
    /* If it's 0xFF, check and discard stuffed zero byte */
    if (c == 0xFF) {
      int c2 = JGETC(dcinfo);
      if (c2 != 0) {
	/* Oops, it's actually a marker indicating end of compressed data. */
	/* Better put it back for use later */
	JUNGETC(c2,dcinfo);
	JUNGETC(c,dcinfo);
	/* There should be enough bits still left in the data segment; */
	/* if so, just break out of the while loop. */
	if (bits_left >= nbits)
	  break;
	/* Uh-oh.  Report corrupted data to user and stuff zeroes into
	 * the data stream, so we can produce some kind of image.
	 * Note that this will be repeated for each byte demanded for the
	 * rest of the segment; this is a bit slow but not unreasonably so.
	 * The main thing is to avoid getting a zillion warnings, hence:
	 */
	if (! printed_eod) {
	  WARNMS(dcinfo->emethods, "Corrupt JPEG data: premature end of data segment");
	  printed_eod = TRUE;
	}
	c = 0;			/* insert a zero byte into bit buffer */
      }
    }

    /* OK, load c into get_buffer */
    get_buffer = (get_buffer << 8) | c;
    bits_left += 8;
  }

  /* Having filled get_buffer, extract desired bits (this simplifies macros) */
  bits_left -= nbits;
  return ((int) (get_buffer >> bits_left)) & bmask[nbits];
}


/* Macros to make things go at some speed! */
/* NB: parameter to get_bits should be simple variable, not expression */

#define get_bits(nbits) \
	(bits_left >= (nbits) ? \
	 ((int) (get_buffer >> (bits_left -= (nbits)))) & bmask[nbits] : \
	 fill_bit_buffer(nbits))

#define get_bit() \
	(bits_left ? \
	 ((int) (get_buffer >> (--bits_left))) & 1 : \
	 fill_bit_buffer(1))


/* Figure F.16: extract next coded symbol from input stream */
  
INLINE
LOCAL int
huff_DECODE (htbl)  HUFF_TBL * htbl;
{
  register int l;
  register INT32 code;
  
  code = get_bit();
  l = 1;
  while (code > htbl->maxcode[l]) {
    code = (code << 1) | get_bit();
    l++;
  }

  /* With garbage input we may reach the sentinel value l = 17. */

  if (l > 16) {
    WARNMS(dcinfo->emethods, "Corrupt JPEG data: bad Huffman code");
    return 0;			/* fake a zero as the safest result */
  }

  return htbl->huffval[ htbl->valptr[l] + ((int) (code - htbl->mincode[l])) ];
}


/* Figure F.12: extend sign bit */

#define huff_EXTEND(x,s)  ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

static const int extend_test[16] =   /* entry n is 2**(n-1) */
  { 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };

static const int extend_offset[16] = /* entry n is (-1 << n) + 1 */
  { 0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
    ((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
    ((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
    ((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1 };


/*
 * Initialize for a Huffman-compressed scan.
 * This is invoked after reading the SOS marker.
 */

METHODDEF void
huff_decoder_init (cinfo)  decompress_info_ptr cinfo;
{
  short ci;
  jpeg_component_info * compptr;

  /* Initialize static variables */
  dcinfo = cinfo;
  bits_left = 0;
  printed_eod = FALSE;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* Make sure requested tables are present */
    if (cinfo->dc_huff_tbl_ptrs[compptr->dc_tbl_no] == NULL ||
	cinfo->ac_huff_tbl_ptrs[compptr->ac_tbl_no] == NULL)
      ERREXIT(cinfo->emethods, "Use of undefined Huffman table");
    /* Compute derived values for Huffman tables */
    /* We may do this more than once for same table, but it's not a big deal */
    fix_huff_tbl(cinfo->dc_huff_tbl_ptrs[compptr->dc_tbl_no]);
    fix_huff_tbl(cinfo->ac_huff_tbl_ptrs[compptr->ac_tbl_no]);
    /* Initialize DC predictions to 0 */
    cinfo->last_dc_val[ci] = 0;
  }

  /* Initialize restart stuff */
  cinfo->restarts_to_go = cinfo->restart_interval;
  cinfo->next_restart_num = 0;
}


/*
 * Check for a restart marker & resynchronize decoder.
 */

LOCAL void
process_restart (cinfo)  decompress_info_ptr cinfo;
{
  int c, nbytes;
  short ci;

  /* Throw away any unused bits remaining in bit buffer */
  nbytes = bits_left / 8;	/* count any full bytes loaded into buffer */
  bits_left = 0;
  printed_eod = FALSE;		/* next segment can get another warning */

  /* Scan for next JPEG marker */
  do {
    do {			/* skip any non-FF bytes */
      nbytes++;
      c = JGETC(cinfo);
    } while (c != 0xFF);
    do {			/* skip any duplicate FFs */
      /* we don't increment nbytes here since extra FFs are legal */
      c = JGETC(cinfo);
    } while (c == 0xFF);
  } while (c == 0);		/* repeat if it was a stuffed FF/00 */

  if (nbytes != 1)
    WARNMS2(cinfo->emethods,
	    "Corrupt JPEG data: %d extraneous bytes before marker 0x%02x",
	    nbytes-1, c);

  if (c != (RST0 + cinfo->next_restart_num)) {
    /* Uh-oh, the restart markers have been messed up too. */
    /* Let the file-format module try to figure out how to resync. */
    (*cinfo->methods->resync_to_restart) (cinfo, c);
  } else
    TRACEMS1(cinfo->emethods, 2, "RST%d", cinfo->next_restart_num);

  /* Re-initialize DC predictions to 0 */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++)
    cinfo->last_dc_val[ci] = 0;

  /* Update restart state */
  cinfo->restarts_to_go = cinfo->restart_interval;
  cinfo->next_restart_num = (cinfo->next_restart_num + 1) & 7;
}


/* ZAG[i] is the natural-order position of the i'th element of zigzag order.
 * If the incoming data is corrupted, huff_decode_mcu could attempt to
 * reference values beyond the end of the array.  To avoid a wild store,
 * we put some extra zeroes after the real entries.
 */

static const short ZAG[DCTSIZE2+16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
  0,  0,  0,  0,  0,  0,  0,  0, /* extra entries in case k>63 below */
  0,  0,  0,  0,  0,  0,  0,  0
};


/*
 * Decode and return one MCU's worth of Huffman-compressed coefficients.
 * This routine also handles quantization descaling and zigzag reordering
 * of coefficient values.
 *
 * The i'th block of the MCU is stored into the block pointed to by
 * MCU_data[i].  WE ASSUME THIS AREA HAS BEEN ZEROED BY THE CALLER.
 * (Wholesale zeroing is usually a little faster than retail...)
 */

METHODDEF void
huff_decode_mcu (cinfo, MCU_data)  decompress_info_ptr cinfo; JBLOCKROW *MCU_data;
{
  register int s, k, r;
  short blkn, ci;
  register JBLOCKROW block;
  register QUANT_TBL_PTR quanttbl;
  HUFF_TBL *dctbl;
  HUFF_TBL *actbl;
  jpeg_component_info * compptr;

  /* Account for restart interval, process restart marker if needed */
  if (cinfo->restart_interval) {
    if (cinfo->restarts_to_go == 0)
      process_restart(cinfo);
    cinfo->restarts_to_go--;
  }

  /* Outer loop handles each block in the MCU */

  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];
    quanttbl = cinfo->quant_tbl_ptrs[compptr->quant_tbl_no];
    actbl = cinfo->ac_huff_tbl_ptrs[compptr->ac_tbl_no];
    dctbl = cinfo->dc_huff_tbl_ptrs[compptr->dc_tbl_no];

    /* Decode a single block's worth of coefficients */

    /* Section F.2.2.1: decode the DC coefficient difference */
    s = huff_DECODE(dctbl);
    if (s) {
      r = get_bits(s);
      s = huff_EXTEND(r, s);
    }

    /* Convert DC difference to actual value, update last_dc_val */
    s += cinfo->last_dc_val[ci];
    cinfo->last_dc_val[ci] = (JCOEF) s;
    /* Descale and output the DC coefficient (assumes ZAG[0] = 0) */
    (*block)[0] = (JCOEF) (((JCOEF) s) * quanttbl[0]);
    
    /* Section F.2.2.2: decode the AC coefficients */
    /* Since zero values are skipped, output area must be zeroed beforehand */
    for (k = 1; k < DCTSIZE2; k++) {
      r = huff_DECODE(actbl);
      
      s = r & 15;
      r = r >> 4;
      
      if (s) {
	k += r;
	r = get_bits(s);
	s = huff_EXTEND(r, s);
	/* Descale coefficient and output in natural (dezigzagged) order */
	(*block)[ZAG[k]] = (JCOEF) (((JCOEF) s) * quanttbl[k]);
      } else {
	if (r != 15)
	  break;
	k += 15;
      }
    }
  }
}

/*
 * Finish up at the end of a Huffman-compressed scan.
 */

METHODDEF void
huff_decoder_term (cinfo)  decompress_info_ptr cinfo;
{
  /* No work needed */
}


/*
 * The method selection routine for Huffman entropy decoding.
 */

GLOBAL void
jseldhuffman (cinfo)  decompress_info_ptr cinfo;
{
  if (! cinfo->arith_code) {
    cinfo->methods->entropy_decode_init = huff_decoder_init;
    cinfo->methods->entropy_decode = huff_decode_mcu;
    cinfo->methods->entropy_decode_term = huff_decoder_term;
  }
}
#line 1 "jdmaster.c"
/*
 * jdmaster.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the main control for the JPEG decompressor.
 * The system-dependent (user interface) code should call jpeg_decompress()
 * after doing appropriate setup of the decompress_info_struct parameter.
 */

#include "jpeg.h"


METHODDEF void
d_per_scan_method_selection (cinfo)  decompress_info_ptr cinfo;
/* Central point for per-scan method selection */
{
  /* MCU disassembly */
  jseldmcu(cinfo);
  /* Upsampling of pixels */
  jselupsample(cinfo);
}


LOCAL void
d_initial_method_selection (cinfo)  decompress_info_ptr cinfo;
/* Central point for initial method selection (after reading file header) */
{
  /* JPEG file scanning method selection is already done. */
  /* So is output file format selection (both are done by user interface). */

  /* Entropy decoding: either Huffman or arithmetic coding. */
#ifdef D_ARITH_CODING_SUPPORTED
  jseldarithmetic(cinfo);
#else
  if (cinfo->arith_code) {
    ERREXIT(cinfo->emethods, "Arithmetic coding not supported");
  }
#endif
  jseldhuffman(cinfo);
  /* Cross-block smoothing */
#ifdef BLOCK_SMOOTHING_SUPPORTED
  jselbsmooth(cinfo);
#else
  cinfo->do_block_smoothing = FALSE;
#endif
  /* Gamma and color space conversion */
  jseldcolor(cinfo);

  /* Color quantization selection rules */
#ifdef QUANT_1PASS_SUPPORTED
#ifdef QUANT_2PASS_SUPPORTED
  /* We have both, check for conditions in which 1-pass should be used */
  if (cinfo->num_components != 3 || cinfo->jpeg_color_space != CS_YCbCr)
    cinfo->two_pass_quantize = FALSE; /* 2-pass only handles YCbCr input */
  if (cinfo->out_color_space == CS_GRAYSCALE)
    cinfo->two_pass_quantize = FALSE; /* Should use 1-pass for grayscale out */
#else /* not QUANT_2PASS_SUPPORTED */
  cinfo->two_pass_quantize = FALSE; /* only have 1-pass */
#endif
#else /* not QUANT_1PASS_SUPPORTED */
#ifdef QUANT_2PASS_SUPPORTED
  cinfo->two_pass_quantize = TRUE; /* only have 2-pass */
#else /* not QUANT_2PASS_SUPPORTED */
  if (cinfo->quantize_colors) {
    ERREXIT(cinfo->emethods, "Color quantization was not compiled");
  }
#endif
#endif

#ifdef QUANT_1PASS_SUPPORTED
  jsel1quantize(cinfo);
#endif
#ifdef QUANT_2PASS_SUPPORTED
  jsel2quantize(cinfo);
#endif

  /* Pipeline control */
  jseldpipeline(cinfo);
  /* Overall control (that's me!) */
  cinfo->methods->d_per_scan_method_selection = d_per_scan_method_selection;
}


LOCAL void
initial_setup (cinfo)  decompress_info_ptr cinfo;
/* Do computations that are needed before initial method selection */
{
  short ci;
  jpeg_component_info *compptr;

  /* Compute maximum sampling factors; check factor validity */
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    if (compptr->h_samp_factor<=0 || compptr->h_samp_factor>MAX_SAMP_FACTOR ||
	compptr->v_samp_factor<=0 || compptr->v_samp_factor>MAX_SAMP_FACTOR)
      ERREXIT(cinfo->emethods, "Bogus sampling factors");
    cinfo->max_h_samp_factor = MAX(cinfo->max_h_samp_factor,
				   compptr->h_samp_factor);
    cinfo->max_v_samp_factor = MAX(cinfo->max_v_samp_factor,
				   compptr->v_samp_factor);

  }

  /* Compute logical downsampled dimensions of components */
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    compptr->true_comp_width = (cinfo->image_width * compptr->h_samp_factor
				+ cinfo->max_h_samp_factor - 1)
				/ cinfo->max_h_samp_factor;
    compptr->true_comp_height = (cinfo->image_height * compptr->v_samp_factor
				 + cinfo->max_v_samp_factor - 1)
				 / cinfo->max_v_samp_factor;
  }
}


/*
 * This is the main entry point to the JPEG decompressor.
 */


GLOBAL void
jpeg_decompress (cinfo)  decompress_info_ptr cinfo;
{
  /* Init pass counts to 0 --- total_passes is adjusted in method selection */
  cinfo->total_passes = 0;
  cinfo->completed_passes = 0;

  /* Read the JPEG file header markers; everything up through the first SOS
   * marker is read now.  NOTE: the user interface must have initialized the
   * read_file_header method pointer (eg, by calling jselrjfif or jselrtiff).
   * The other file reading methods (read_scan_header etc.) were probably
   * set at the same time, but could be set up by read_file_header itself.
   */
  (*cinfo->methods->read_file_header) (cinfo);
  if (! ((*cinfo->methods->read_scan_header) (cinfo)))
    ERREXIT(cinfo->emethods, "Empty JPEG file");

  /* Give UI a chance to adjust decompression parameters and select */
  /* output file format based on info from file header. */
  (*cinfo->methods->d_ui_method_selection) (cinfo);

  /* Now select methods for decompression steps. */
  initial_setup(cinfo);
  d_initial_method_selection(cinfo);

  /* Initialize the output file & other modules as needed */
  /* (modules needing per-scan init are called by pipeline controller) */

  (*cinfo->methods->output_init) (cinfo);
  (*cinfo->methods->colorout_init) (cinfo);
  if (cinfo->quantize_colors)
    (*cinfo->methods->color_quant_init) (cinfo);

  /* And let the pipeline controller do the rest. */
  (*cinfo->methods->d_pipeline_controller) (cinfo);

  /* Finish output file, release working storage, etc */
  if (cinfo->quantize_colors)
    (*cinfo->methods->color_quant_term) (cinfo);
  (*cinfo->methods->colorout_term) (cinfo);
  (*cinfo->methods->output_term) (cinfo);
  (*cinfo->methods->read_file_trailer) (cinfo);

  (*cinfo->emethods->free_all) ();

  /* My, that was easy, wasn't it? */
}
#line 1 "jdmcu.c"
/*
 * jdmcu.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains MCU disassembly and IDCT control routines.
 * These routines are invoked via the disassemble_MCU, reverse_DCT, and
 * disassemble_init/term methods.
 */

#include "jpeg.h"


/*
 * Fetch one MCU row from entropy_decode, build coefficient array.
 * This version is used for noninterleaved (single-component) scans.
 */

METHODDEF void
disassemble_noninterleaved_MCU (cinfo, image_data)  decompress_info_ptr cinfo;
				JBLOCKIMAGE image_data;
{
  JBLOCKROW MCU_data[1];
  long mcuindex;

  /* this is pretty easy since there is one component and one block per MCU */

  /* Pre-zero the target area to speed up entropy decoder */
  /* (we assume wholesale zeroing is faster than retail) */
  jzero_far((void FAR *) image_data[0][0],
	    (size_t) (cinfo->MCUs_per_row * SIZEOF(JBLOCK)));

  for (mcuindex = 0; mcuindex < cinfo->MCUs_per_row; mcuindex++) {
    /* Point to the proper spot in the image array for this MCU */
    MCU_data[0] = image_data[0][0] + mcuindex;
    /* Fetch the coefficient data */
    (*cinfo->methods->entropy_decode) (cinfo, MCU_data);
  }
}


/*
 * Fetch one MCU row from entropy_decode, build coefficient array.
 * This version is used for interleaved (multi-component) scans.
 */

METHODDEF void
disassemble_interleaved_MCU (cinfo, image_data)  decompress_info_ptr cinfo;
			     JBLOCKIMAGE image_data;
{
  JBLOCKROW MCU_data[MAX_BLOCKS_IN_MCU];
  long mcuindex;
  short blkn, ci, xpos, ypos;
  jpeg_component_info * compptr;
  JBLOCKROW image_ptr;

  /* Pre-zero the target area to speed up entropy decoder */
  /* (we assume wholesale zeroing is faster than retail) */
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
      jzero_far((void FAR *) image_data[ci][ypos],
		(size_t) (cinfo->MCUs_per_row * compptr->MCU_width * SIZEOF(JBLOCK)));
    }
  }

  for (mcuindex = 0; mcuindex < cinfo->MCUs_per_row; mcuindex++) {
    /* Point to the proper spots in the image array for this MCU */
    blkn = 0;
    for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
      compptr = cinfo->cur_comp_info[ci];
      for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
	image_ptr = image_data[ci][ypos] + (mcuindex * compptr->MCU_width);
	for (xpos = 0; xpos < compptr->MCU_width; xpos++) {
	  MCU_data[blkn] = image_ptr;
	  image_ptr++;
	  blkn++;
	}
      }
    }
    /* Fetch the coefficient data */
    (*cinfo->methods->entropy_decode) (cinfo, MCU_data);
  }
}


/*
 * Perform inverse DCT on each block in an MCU row's worth of data;
 * output the results into a sample array starting at row start_row.
 * NB: start_row can only be nonzero when dealing with a single-component
 * scan; otherwise we'd have to pass different offsets for different
 * components, since the heights of interleaved MCU rows can vary.
 * But the pipeline controller logic is such that this is not necessary.
 */

METHODDEF void
reverse_DCT (cinfo, coeff_data, output_data, start_row)  decompress_info_ptr cinfo;
	     JBLOCKIMAGE coeff_data; JSAMPIMAGE output_data; int start_row;
{
  DCTBLOCK block;
  JBLOCKROW browptr;
  JSAMPARRAY srowptr;
  long blocksperrow, bi;
  short numrows, ri;
  short ci;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    /* calculate size of an MCU row in this component */
    blocksperrow = cinfo->cur_comp_info[ci]->downsampled_width / DCTSIZE;
    numrows = cinfo->cur_comp_info[ci]->MCU_height;
    /* iterate through all blocks in MCU row */
    for (ri = 0; ri < numrows; ri++) {
      browptr = coeff_data[ci][ri];
      srowptr = output_data[ci] + (ri * DCTSIZE + start_row);
      for (bi = 0; bi < blocksperrow; bi++) {
	/* copy the data into a local DCTBLOCK.  This allows for change of
	 * representation (if DCTELEM != JCOEF).  On 80x86 machines it also
	 * brings the data back from FAR storage to NEAR storage.
	 */
	{ register JCOEFPTR elemptr = browptr[bi];
	  register DCTELEM *localblkptr = block;
	  register int elem = DCTSIZE2;

	  while (--elem >= 0)
	    *localblkptr++ = (DCTELEM) *elemptr++;
	}

	j_rev_dct(block);	/* perform inverse DCT */

	/* Output the data into the sample array.
	 * Note change from signed to unsigned representation:
	 * DCT calculation works with values +-CENTERJSAMPLE,
	 * but sample arrays always hold 0..MAXJSAMPLE.
	 * We have to do range-limiting because of quantization errors in the
	 * DCT/IDCT phase.  We use the sample_range_limit[] table to do this
	 * quickly; the CENTERJSAMPLE offset is folded into table indexing.
	 */
	{ register JSAMPROW elemptr;
	  register DCTELEM *localblkptr = block;
	  register JSAMPLE *range_limit = cinfo->sample_range_limit +
						CENTERJSAMPLE;
#if DCTSIZE != 8
	  register int elemc;
#endif
	  register int elemr;

	  for (elemr = 0; elemr < DCTSIZE; elemr++) {
	    elemptr = srowptr[elemr] + (bi * DCTSIZE);
#if DCTSIZE == 8		/* unroll the inner loop */
	    *elemptr++ = range_limit[*localblkptr++];
	    *elemptr++ = range_limit[*localblkptr++];
	    *elemptr++ = range_limit[*localblkptr++];
	    *elemptr++ = range_limit[*localblkptr++];
	    *elemptr++ = range_limit[*localblkptr++];
	    *elemptr++ = range_limit[*localblkptr++];
	    *elemptr++ = range_limit[*localblkptr++];
	    *elemptr++ = range_limit[*localblkptr++];
#else
	    for (elemc = DCTSIZE; elemc > 0; elemc--) {
	      *elemptr++ = range_limit[*localblkptr++];
	    }
#endif
	  }
	}
      }
    }
  }
}


/*
 * Initialize for processing a scan.
 */

METHODDEF void
disassemble_init (cinfo)  decompress_info_ptr cinfo;
{
  /* no work for now */
}


/*
 * Clean up after a scan.
 */

METHODDEF void
disassemble_term (cinfo)  decompress_info_ptr cinfo;
{
  /* no work for now */
}



/*
 * The method selection routine for MCU disassembly.
 */

GLOBAL void
jseldmcu (cinfo)  decompress_info_ptr cinfo;
{
  if (cinfo->comps_in_scan == 1)
    cinfo->methods->disassemble_MCU = disassemble_noninterleaved_MCU;
  else
    cinfo->methods->disassemble_MCU = disassemble_interleaved_MCU;
  cinfo->methods->reverse_DCT = reverse_DCT;
  cinfo->methods->disassemble_init = disassemble_init;
  cinfo->methods->disassemble_term = disassemble_term;
}
#line 1 "jdpipe.c"
/*
 * jdpipe.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression pipeline controllers.
 * These routines are invoked via the d_pipeline_controller method.
 *
 * There are two basic pipeline controllers.  The simpler one handles a
 * single-scan JPEG file (single component or fully interleaved) with no
 * color quantization or 1-pass quantization.  In this case, the file can
 * be processed in one top-to-bottom pass.  The more complex controller is
 * used when 2-pass color quantization is requested and/or the JPEG file
 * has multiple scans (noninterleaved or partially interleaved).  In this
 * case, the entire image must be buffered up in a "big" array.
 *
 * If you need to make a minimal implementation, the more complex controller
 * can be compiled out by disabling the appropriate configuration options.
 * We don't recommend this, since then you can't handle all legal JPEG files.
 */

#include "jpeg.h"


#ifdef D_MULTISCAN_FILES_SUPPORTED /* wish we could assume ANSI's defined() */
#define NEED_COMPLEX_CONTROLLER
#else
#ifdef QUANT_2PASS_SUPPORTED
#define NEED_COMPLEX_CONTROLLER
#endif
#endif


/*
 * About the data structures:
 *
 * The processing chunk size for upsampling is referred to in this file as
 * a "row group": a row group is defined as Vk (v_samp_factor) sample rows of
 * any component while downsampled, or Vmax (max_v_samp_factor) unsubsampled
 * rows.  In an interleaved scan each MCU row contains exactly DCTSIZE row
 * groups of each component in the scan.  In a noninterleaved scan an MCU row
 * is one row of blocks, which might not be an integral number of row groups;
 * therefore, we read in Vk MCU rows to obtain the same amount of data as we'd
 * have in an interleaved scan.
 * To provide context for the upsampling step, we have to retain the last
 * two row groups of the previous MCU row while reading in the next MCU row
 * (or set of Vk MCU rows).  To do this without copying data about, we create
 * a rather strange data structure.  Exactly DCTSIZE+2 row groups of samples
 * are allocated, but we create two different sets of pointers to this array.
 * The second set swaps the last two pairs of row groups.  By working
 * alternately with the two sets of pointers, we can access the data in the
 * desired order.
 *
 * Cross-block smoothing also needs context above and below the "current" row.
 * Since this is an optional feature, I've implemented it in a way that is
 * much simpler but requires more than the minimum amount of memory.  We
 * simply allocate three extra MCU rows worth of coefficient blocks and use
 * them to "read ahead" one MCU row in the file.  For a typical 1000-pixel-wide
 * image with 2x2,1x1,1x1 sampling, each MCU row is about 50Kb; an 80x86
 * machine may be unable to apply cross-block smoothing to wider images.
 */


/*
 * These variables are logically local to the pipeline controller,
 * but we make them static so that scan_big_image can use them
 * without having to pass them through the quantization routines.
 */

static int rows_in_mem;		/* # of sample rows in full-size buffers */
/* Work buffer for data being passed to output module. */
/* This has color_out_comps components if not quantizing, */
/* but only one component when quantizing. */
static JSAMPIMAGE output_workspace;

#ifdef NEED_COMPLEX_CONTROLLER
/* Full-size image array holding upsampled, but not color-processed data. */
static big_sarray_ptr *fullsize_image;
static JSAMPIMAGE fullsize_ptrs; /* workspace for access_big_sarray() result */
#endif


/*
 * Utility routines: common code for pipeline controllers
 */

LOCAL void
interleaved_scan_setup (cinfo)  decompress_info_ptr cinfo;
/* Compute all derived info for an interleaved (multi-component) scan */
/* On entry, cinfo->comps_in_scan and cinfo->cur_comp_info[] are set up */
{
  short ci, mcublks;
  jpeg_component_info *compptr;

  if (cinfo->comps_in_scan > MAX_COMPS_IN_SCAN)
    ERREXIT(cinfo->emethods, "Too many components for interleaved scan");

  cinfo->MCUs_per_row = (cinfo->image_width
			 + cinfo->max_h_samp_factor*DCTSIZE - 1)
			/ (cinfo->max_h_samp_factor*DCTSIZE);

  cinfo->MCU_rows_in_scan = (cinfo->image_height
			     + cinfo->max_v_samp_factor*DCTSIZE - 1)
			    / (cinfo->max_v_samp_factor*DCTSIZE);
  
  cinfo->blocks_in_MCU = 0;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    /* for interleaved scan, sampling factors give # of blocks per component */
    compptr->MCU_width = compptr->h_samp_factor;
    compptr->MCU_height = compptr->v_samp_factor;
    compptr->MCU_blocks = compptr->MCU_width * compptr->MCU_height;
    /* compute physical dimensions of component */
    compptr->downsampled_width = jround_up(compptr->true_comp_width,
					   (long) (compptr->MCU_width*DCTSIZE));
    compptr->downsampled_height = jround_up(compptr->true_comp_height,
					    (long) (compptr->MCU_height*DCTSIZE));
    /* Sanity check */
    if (compptr->downsampled_width !=
	(cinfo->MCUs_per_row * (compptr->MCU_width*DCTSIZE)))
      ERREXIT(cinfo->emethods, "I'm confused about the image width");
    /* Prepare array describing MCU composition */
    mcublks = compptr->MCU_blocks;
    if (cinfo->blocks_in_MCU + mcublks > MAX_BLOCKS_IN_MCU)
      ERREXIT(cinfo->emethods, "Sampling factors too large for interleaved scan");
    while (mcublks-- > 0) {
      cinfo->MCU_membership[cinfo->blocks_in_MCU++] = ci;
    }
  }

  (*cinfo->methods->d_per_scan_method_selection) (cinfo);
}


LOCAL void
noninterleaved_scan_setup (cinfo)  decompress_info_ptr cinfo;
/* Compute all derived info for a noninterleaved (single-component) scan */
/* On entry, cinfo->comps_in_scan = 1 and cinfo->cur_comp_info[0] is set up */
{
  jpeg_component_info *compptr = cinfo->cur_comp_info[0];

  /* for noninterleaved scan, always one block per MCU */
  compptr->MCU_width = 1;
  compptr->MCU_height = 1;
  compptr->MCU_blocks = 1;
  /* compute physical dimensions of component */
  compptr->downsampled_width = jround_up(compptr->true_comp_width,
					 (long) DCTSIZE);
  compptr->downsampled_height = jround_up(compptr->true_comp_height,
					  (long) DCTSIZE);

  cinfo->MCUs_per_row = compptr->downsampled_width / DCTSIZE;
  cinfo->MCU_rows_in_scan = compptr->downsampled_height / DCTSIZE;

  /* Prepare array describing MCU composition */
  cinfo->blocks_in_MCU = 1;
  cinfo->MCU_membership[0] = 0;

  (*cinfo->methods->d_per_scan_method_selection) (cinfo);
}



LOCAL JSAMPIMAGE
alloc_sampimage (cinfo, num_comps, num_rows, num_cols)  decompress_info_ptr cinfo;
		 int num_comps; long num_rows; long num_cols;
/* Allocate an in-memory sample image (all components same size) */
{
  JSAMPIMAGE image;
  int ci;

  image = (JSAMPIMAGE) (*cinfo->emethods->alloc_small)
				(num_comps * SIZEOF(JSAMPARRAY));
  for (ci = 0; ci < num_comps; ci++) {
    image[ci] = (*cinfo->emethods->alloc_small_sarray) (num_cols, num_rows);
  }
  return image;
}


#if 0				/* this routine not currently needed */

LOCAL void
free_sampimage (cinfo, image, num_comps)  decompress_info_ptr cinfo; JSAMPIMAGE image; int num_comps;
/* Release a sample image created by alloc_sampimage */
{
  int ci;

  for (ci = 0; ci < num_comps; ci++) {
      (*cinfo->emethods->free_small_sarray) (image[ci]);
  }
  (*cinfo->emethods->free_small) ((void *) image);
}

#endif


LOCAL JBLOCKIMAGE
alloc_MCU_row (cinfo)  decompress_info_ptr cinfo;
/* Allocate one MCU row's worth of coefficient blocks */
{
  JBLOCKIMAGE image;
  int ci;

  image = (JBLOCKIMAGE) (*cinfo->emethods->alloc_small)
				(cinfo->comps_in_scan * SIZEOF(JBLOCKARRAY));
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    image[ci] = (*cinfo->emethods->alloc_small_barray)
			(cinfo->cur_comp_info[ci]->downsampled_width / DCTSIZE,
			 (long) cinfo->cur_comp_info[ci]->MCU_height);
  }
  return image;
}


#ifdef NEED_COMPLEX_CONTROLLER	/* not used by simple controller */

LOCAL void
free_MCU_row (cinfo, image)  decompress_info_ptr cinfo; JBLOCKIMAGE image;
/* Release a coefficient block array created by alloc_MCU_row */
{
  int ci;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    (*cinfo->emethods->free_small_barray) (image[ci]);
  }
  (*cinfo->emethods->free_small) ((void *) image);
}

#endif


LOCAL void
alloc_sampling_buffer (cinfo, sampled_data)  decompress_info_ptr cinfo; JSAMPIMAGE sampled_data[2];
/* Create a downsampled-data buffer having the desired structure */
/* (see comments at head of file) */
{
  short ci, vs, i;

  /* Get top-level space for array pointers */
  sampled_data[0] = (JSAMPIMAGE) (*cinfo->emethods->alloc_small)
				(cinfo->comps_in_scan * SIZEOF(JSAMPARRAY));
  sampled_data[1] = (JSAMPIMAGE) (*cinfo->emethods->alloc_small)
				(cinfo->comps_in_scan * SIZEOF(JSAMPARRAY));

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    vs = cinfo->cur_comp_info[ci]->v_samp_factor; /* row group height */
    /* Allocate the real storage */
    sampled_data[0][ci] = (*cinfo->emethods->alloc_small_sarray)
				(cinfo->cur_comp_info[ci]->downsampled_width,
				(long) (vs * (DCTSIZE+2)));
    /* Create space for the scrambled-order pointers */
    sampled_data[1][ci] = (JSAMPARRAY) (*cinfo->emethods->alloc_small)
				(vs * (DCTSIZE+2) * SIZEOF(JSAMPROW));
    /* Duplicate the first DCTSIZE-2 row groups */
    for (i = 0; i < vs * (DCTSIZE-2); i++) {
      sampled_data[1][ci][i] = sampled_data[0][ci][i];
    }
    /* Copy the last four row groups in swapped order */
    for (i = 0; i < vs * 2; i++) {
      sampled_data[1][ci][vs*DCTSIZE + i] = sampled_data[0][ci][vs*(DCTSIZE-2) + i];
      sampled_data[1][ci][vs*(DCTSIZE-2) + i] = sampled_data[0][ci][vs*DCTSIZE + i];
    }
  }
}


#ifdef NEED_COMPLEX_CONTROLLER	/* not used by simple controller */

LOCAL void
free_sampling_buffer (cinfo, sampled_data)  decompress_info_ptr cinfo; JSAMPIMAGE sampled_data[2];
/* Release a sampling buffer created by alloc_sampling_buffer */
{
  short ci;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    /* Free the real storage */
    (*cinfo->emethods->free_small_sarray) (sampled_data[0][ci]);
    /* Free the scrambled-order pointers */
    (*cinfo->emethods->free_small) ((void *) sampled_data[1][ci]);
  }

  /* Free the top-level space */
  (*cinfo->emethods->free_small) ((void *) sampled_data[0]);
  (*cinfo->emethods->free_small) ((void *) sampled_data[1]);
}

#endif


/*
 * Several decompression processes need to range-limit values to the range
 * 0..MAXJSAMPLE; the input value may fall somewhat outside this range
 * due to noise introduced by quantization, roundoff error, etc.  These
 * processes are inner loops and need to be as fast as possible.  On most
 * machines, particularly CPUs with pipelines or instruction prefetch,
 * a (range-check-less) C table lookup
 *		x = sample_range_limit[x];
 * is faster than explicit tests
 *		if (x < 0)  x = 0;
 *		else if (x > MAXJSAMPLE)  x = MAXJSAMPLE;
 * These processes all use a common table prepared by the routine below.
 *
 * The table will work correctly for x within MAXJSAMPLE+1 of the legal
 * range.  This is a much wider range than is needed for most cases,
 * but the wide range is handy for color quantization.
 * Note that the table is allocated in near data space on PCs; it's small
 * enough and used often enough to justify this.
 */

LOCAL void
prepare_range_limit_table (cinfo)  decompress_info_ptr cinfo;
/* Allocate and fill in the sample_range_limit table */
{
  JSAMPLE * table;
  int i;

  table = (JSAMPLE *) (*cinfo->emethods->alloc_small)
			(3 * (MAXJSAMPLE+1) * SIZEOF(JSAMPLE));
  cinfo->sample_range_limit = table + (MAXJSAMPLE+1);
  for (i = 0; i <= MAXJSAMPLE; i++) {
    table[i] = 0;			/* sample_range_limit[x] = 0 for x<0 */
    table[i+(MAXJSAMPLE+1)] = (JSAMPLE) i;	/* sample_range_limit[x] = x */
    table[i+(MAXJSAMPLE+1)*2] = MAXJSAMPLE;	/* x beyond MAXJSAMPLE */
  }
}


LOCAL void
duplicate_row (image_data, num_cols, source_row, num_rows)  JSAMPARRAY image_data;
	       long num_cols; int source_row; int num_rows;
/* Duplicate the source_row at source_row+1 .. source_row+num_rows */
/* This happens only at the bottom of the image, */
/* so it needn't be super-efficient */
{
  register int row;

  for (row = 1; row <= num_rows; row++) {
    jcopy_sample_rows(image_data, source_row, image_data, source_row + row,
		      1, num_cols);
  }
}


LOCAL void
expand (cinfo, sampled_data, fullsize_data, fullsize_width, above, current, below, out)  decompress_info_ptr cinfo;
	JSAMPIMAGE sampled_data; JSAMPIMAGE fullsize_data;
	long fullsize_width;
	short above; short current; short below; short out;
/* Do upsampling expansion of a single row group (of each component). */
/* above, current, below are indexes of row groups in sampled_data;       */
/* out is the index of the target row group in fullsize_data.             */
/* Special case: above, below can be -1 to indicate top, bottom of image. */
{
  jpeg_component_info *compptr;
  JSAMPARRAY above_ptr, below_ptr;
  JSAMPROW dummy[MAX_SAMP_FACTOR]; /* for downsample expansion at top/bottom */
  short ci, vs, i;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    vs = compptr->v_samp_factor; /* row group height */

    if (above >= 0)
      above_ptr = sampled_data[ci] + above * vs;
    else {
      /* Top of image: make a dummy above-context with copies of 1st row */
      /* We assume current=0 in this case */
      for (i = 0; i < vs; i++)
	dummy[i] = sampled_data[ci][0];
      above_ptr = (JSAMPARRAY) dummy; /* possible near->far pointer conv */
    }

    if (below >= 0)
      below_ptr = sampled_data[ci] + below * vs;
    else {
      /* Bot of image: make a dummy below-context with copies of last row */
      for (i = 0; i < vs; i++)
	dummy[i] = sampled_data[ci][(current+1)*vs-1];
      below_ptr = (JSAMPARRAY) dummy; /* possible near->far pointer conv */
    }

    (*cinfo->methods->upsample[ci])
		(cinfo, (int) ci,
		 compptr->downsampled_width, (int) vs,
		 fullsize_width, (int) cinfo->max_v_samp_factor,
		 above_ptr,
		 sampled_data[ci] + current * vs,
		 below_ptr,
		 fullsize_data[ci] + out * cinfo->max_v_samp_factor);
  }
}


LOCAL void
emit_1pass (cinfo, num_rows, fullsize_data, dummy)  decompress_info_ptr cinfo; int num_rows; JSAMPIMAGE fullsize_data;
	    JSAMPARRAY dummy;
/* Do color processing and output of num_rows full-size rows. */
/* This is not used when doing 2-pass color quantization. */
/* The dummy argument simply lets this be called via scan_big_image. */
{
  if (cinfo->quantize_colors) {
    (*cinfo->methods->color_quantize) (cinfo, num_rows, fullsize_data,
				       output_workspace[0]);
  } else {
    (*cinfo->methods->color_convert) (cinfo, num_rows, cinfo->image_width,
				      fullsize_data, output_workspace);
  }
    
  (*cinfo->methods->put_pixel_rows) (cinfo, num_rows, output_workspace);
}


/*
 * Support routines for complex controller.
 */

#ifdef NEED_COMPLEX_CONTROLLER

METHODDEF void
scan_big_image (cinfo, quantize_method)  decompress_info_ptr cinfo; quantize_method_ptr quantize_method;
/* Apply quantize_method to entire image stored in fullsize_image[]. */
/* This is the "iterator" routine used by the 2-pass color quantizer. */
/* We also use it directly in some cases. */
{
  long pixel_rows_output;
  short ci;

  for (pixel_rows_output = 0; pixel_rows_output < cinfo->image_height;
       pixel_rows_output += rows_in_mem) {
    (*cinfo->methods->progress_monitor) (cinfo, pixel_rows_output,
					 cinfo->image_height);
    /* Realign the big buffers */
    for (ci = 0; ci < cinfo->num_components; ci++) {
      fullsize_ptrs[ci] = (*cinfo->emethods->access_big_sarray)
	(fullsize_image[ci], pixel_rows_output, FALSE);
    }
    /* Let the quantizer have its way with the data.
     * Note that output_workspace is simply workspace for the quantizer;
     * when it's ready to output, it must call put_pixel_rows itself.
     */
    (*quantize_method) (cinfo,
			(int) MIN((long) rows_in_mem,
				  cinfo->image_height - pixel_rows_output),
			fullsize_ptrs, output_workspace[0]);
  }

  cinfo->completed_passes++;
}

#endif /* NEED_COMPLEX_CONTROLLER */


/*
 * Support routines for cross-block smoothing.
 */

#ifdef BLOCK_SMOOTHING_SUPPORTED


LOCAL void
smooth_mcu_row (cinfo, above, input, below, output)  decompress_info_ptr cinfo;
		JBLOCKIMAGE above; JBLOCKIMAGE input; JBLOCKIMAGE below;
		JBLOCKIMAGE output;
/* Apply cross-block smoothing to one MCU row's worth of coefficient blocks. */
/* above,below are NULL if at top/bottom of image. */
{
  jpeg_component_info *compptr;
  short ci, ri, last;
  JBLOCKROW prev;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    last = compptr->MCU_height - 1;

    if (above == NULL)
      prev = NULL;
    else
      prev = above[ci][last];

    for (ri = 0; ri < last; ri++) {
      (*cinfo->methods->smooth_coefficients) (cinfo, compptr,
				prev, input[ci][ri], input[ci][ri+1],
				output[ci][ri]);
      prev = input[ci][ri];
    }

    if (below == NULL)
      (*cinfo->methods->smooth_coefficients) (cinfo, compptr,
				prev, input[ci][last], (JBLOCKROW) NULL,
				output[ci][last]);
    else
      (*cinfo->methods->smooth_coefficients) (cinfo, compptr,
				prev, input[ci][last], below[ci][0],
				output[ci][last]);
  }
}


LOCAL void
get_smoothed_row (cinfo, coeff_data, bsmooth, whichb, cur_mcu_row)  decompress_info_ptr cinfo; JBLOCKIMAGE coeff_data;
		  JBLOCKIMAGE bsmooth[3]; int * whichb; long cur_mcu_row;
/* Get an MCU row of coefficients, applying cross-block smoothing. */
/* The output row is placed in coeff_data.  bsmooth and whichb hold */
/* working state, and cur_row is needed to check for image top/bottom. */
/* This routine just takes care of the buffering logic. */
{
  int prev, cur, next;
  
  /* Special case for top of image: need to pre-fetch a row & init whichb */
  if (cur_mcu_row == 0) {
    (*cinfo->methods->disassemble_MCU) (cinfo, bsmooth[0]);
    if (cinfo->MCU_rows_in_scan > 1) {
      (*cinfo->methods->disassemble_MCU) (cinfo, bsmooth[1]);
      smooth_mcu_row(cinfo, (JBLOCKIMAGE) NULL, bsmooth[0], bsmooth[1],
		     coeff_data);
    } else {
      smooth_mcu_row(cinfo, (JBLOCKIMAGE) NULL, bsmooth[0], (JBLOCKIMAGE) NULL,
		     coeff_data);
    }
    *whichb = 1;		/* points to next bsmooth[] element to use */
    return;
  }
  
  cur = *whichb;		/* set up references */
  prev = (cur == 0 ? 2 : cur - 1);
  next = (cur == 2 ? 0 : cur + 1);
  *whichb = next;		/* advance whichb for next time */
  
  /* Special case for bottom of image: don't read another row */
  if (cur_mcu_row >= cinfo->MCU_rows_in_scan - 1) {
    smooth_mcu_row(cinfo, bsmooth[prev], bsmooth[cur], (JBLOCKIMAGE) NULL,
		   coeff_data);
    return;
  }
  
  /* Normal case: read ahead a new row, smooth the one I got before */
  (*cinfo->methods->disassemble_MCU) (cinfo, bsmooth[next]);
  smooth_mcu_row(cinfo, bsmooth[prev], bsmooth[cur], bsmooth[next],
		 coeff_data);
}


#endif /* BLOCK_SMOOTHING_SUPPORTED */



/*
 * Decompression pipeline controller used for single-scan files
 * without 2-pass color quantization.
 */

METHODDEF void
simple_dcontroller (cinfo)  decompress_info_ptr cinfo;
{
  long fullsize_width;		/* # of samples per row in full-size buffers */
  long cur_mcu_row;		/* counts # of MCU rows processed */
  long pixel_rows_output;	/* # of pixel rows actually emitted */
  int mcu_rows_per_loop;	/* # of MCU rows processed per outer loop */
  /* Work buffer for dequantized coefficients (IDCT input) */
  JBLOCKIMAGE coeff_data;
  /* Work buffer for cross-block smoothing input */
#ifdef BLOCK_SMOOTHING_SUPPORTED
  JBLOCKIMAGE bsmooth[3];	/* this is optional */
  int whichb;
#endif
  /* Work buffer for downsampled image data (see comments at head of file) */
  JSAMPIMAGE sampled_data[2];
  /* Work buffer for upsampled data */
  JSAMPIMAGE fullsize_data;
  int whichss, ri;
  short i;

  /* Compute dimensions of full-size pixel buffers */
  /* Note these are the same whether interleaved or not. */
  rows_in_mem = cinfo->max_v_samp_factor * DCTSIZE;
  fullsize_width = jround_up(cinfo->image_width,
			     (long) (cinfo->max_h_samp_factor * DCTSIZE));

  /* Prepare for single scan containing all components */
  if (cinfo->comps_in_scan == 1) {
    noninterleaved_scan_setup(cinfo);
    /* Need to read Vk MCU rows to obtain Vk block rows */
    mcu_rows_per_loop = cinfo->cur_comp_info[0]->v_samp_factor;
  } else {
    interleaved_scan_setup(cinfo);
    /* in an interleaved scan, one MCU row provides Vk block rows */
    mcu_rows_per_loop = 1;
  }
  cinfo->total_passes++;

  /* Allocate working memory: */
  prepare_range_limit_table(cinfo);
  /* coeff_data holds a single MCU row of coefficient blocks */
  coeff_data = alloc_MCU_row(cinfo);
  /* if doing cross-block smoothing, need extra space for its input */
#ifdef BLOCK_SMOOTHING_SUPPORTED
  if (cinfo->do_block_smoothing) {
    bsmooth[0] = alloc_MCU_row(cinfo);
    bsmooth[1] = alloc_MCU_row(cinfo);
    bsmooth[2] = alloc_MCU_row(cinfo);
  }
#endif
  /* sampled_data is sample data before upsampling */
  alloc_sampling_buffer(cinfo, sampled_data);
  /* fullsize_data is sample data after upsampling */
  fullsize_data = alloc_sampimage(cinfo, (int) cinfo->num_components,
				  (long) rows_in_mem, fullsize_width);
  /* output_workspace is the color-processed data */
  output_workspace = alloc_sampimage(cinfo, (int) cinfo->final_out_comps,
				     (long) rows_in_mem, fullsize_width);

  /* Tell the memory manager to instantiate big arrays.
   * We don't need any big arrays in this controller,
   * but some other module (like the output file writer) may need one.
   */
  (*cinfo->emethods->alloc_big_arrays)
	((long) 0,				/* no more small sarrays */
	 (long) 0,				/* no more small barrays */
	 (long) 0);				/* no more "medium" objects */
  /* NB: if quantizer needs any "medium" size objects, it must get them */
  /* at color_quant_init time */

  /* Initialize to read scan data */

  (*cinfo->methods->entropy_decode_init) (cinfo);
  (*cinfo->methods->upsample_init) (cinfo);
  (*cinfo->methods->disassemble_init) (cinfo);

  /* Loop over scan's data: rows_in_mem pixel rows are processed per loop */

  pixel_rows_output = 0;
  whichss = 1;			/* arrange to start with sampled_data[0] */

  for (cur_mcu_row = 0; cur_mcu_row < cinfo->MCU_rows_in_scan;
       cur_mcu_row += mcu_rows_per_loop) {
    (*cinfo->methods->progress_monitor) (cinfo, cur_mcu_row,
					 cinfo->MCU_rows_in_scan);

    whichss ^= 1;		/* switch to other downsampled-data buffer */

    /* Obtain v_samp_factor block rows of each component in the scan. */
    /* This is a single MCU row if interleaved, multiple MCU rows if not. */
    /* In the noninterleaved case there might be fewer than v_samp_factor */
    /* block rows remaining; if so, pad with copies of the last pixel row */
    /* so that upsampling doesn't have to treat it as a special case. */

    for (ri = 0; ri < mcu_rows_per_loop; ri++) {
      if (cur_mcu_row + ri < cinfo->MCU_rows_in_scan) {
	/* OK to actually read an MCU row. */
#ifdef BLOCK_SMOOTHING_SUPPORTED
	if (cinfo->do_block_smoothing)
	  get_smoothed_row(cinfo, coeff_data,
			   bsmooth, &whichb, cur_mcu_row + ri);
	else
#endif
	  (*cinfo->methods->disassemble_MCU) (cinfo, coeff_data);
      
	(*cinfo->methods->reverse_DCT) (cinfo, coeff_data,
					sampled_data[whichss],
					ri * DCTSIZE);
      } else {
	/* Need to pad out with copies of the last downsampled row. */
	/* This can only happen if there is just one component. */
	duplicate_row(sampled_data[whichss][0],
		      cinfo->cur_comp_info[0]->downsampled_width,
		      ri * DCTSIZE - 1, DCTSIZE);
      }
    }

    /* Upsample the data */
    /* First time through is a special case */

    if (cur_mcu_row) {
      /* Expand last row group of previous set */
      expand(cinfo, sampled_data[whichss], fullsize_data, fullsize_width,
	     (short) DCTSIZE, (short) (DCTSIZE+1), (short) 0,
	     (short) (DCTSIZE-1));
      /* and dump the previous set's expanded data */
      emit_1pass (cinfo, rows_in_mem, fullsize_data, (JSAMPARRAY) NULL);
      pixel_rows_output += rows_in_mem;
      /* Expand first row group of this set */
      expand(cinfo, sampled_data[whichss], fullsize_data, fullsize_width,
	     (short) (DCTSIZE+1), (short) 0, (short) 1,
	     (short) 0);
    } else {
      /* Expand first row group with dummy above-context */
      expand(cinfo, sampled_data[whichss], fullsize_data, fullsize_width,
	     (short) (-1), (short) 0, (short) 1,
	     (short) 0);
    }
    /* Expand second through next-to-last row groups of this set */
    for (i = 1; i <= DCTSIZE-2; i++) {
      expand(cinfo, sampled_data[whichss], fullsize_data, fullsize_width,
	     (short) (i-1), (short) i, (short) (i+1),
	     (short) i);
    }
  } /* end of outer loop */

  /* Expand the last row group with dummy below-context */
  /* Note whichss points to last buffer side used */
  expand(cinfo, sampled_data[whichss], fullsize_data, fullsize_width,
	 (short) (DCTSIZE-2), (short) (DCTSIZE-1), (short) (-1),
	 (short) (DCTSIZE-1));
  /* and dump the remaining data (may be less than full height) */
  emit_1pass (cinfo, (int) (cinfo->image_height - pixel_rows_output),
	      fullsize_data, (JSAMPARRAY) NULL);

  /* Clean up after the scan */
  (*cinfo->methods->disassemble_term) (cinfo);
  (*cinfo->methods->upsample_term) (cinfo);
  (*cinfo->methods->entropy_decode_term) (cinfo);
  (*cinfo->methods->read_scan_trailer) (cinfo);
  cinfo->completed_passes++;

  /* Verify that we've seen the whole input file */
  if ((*cinfo->methods->read_scan_header) (cinfo))
    WARNMS(cinfo->emethods, "Didn't expect more than one scan");

  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */
}


/*
 * Decompression pipeline controller used for multiple-scan files
 * and/or 2-pass color quantization.
 *
 * The current implementation places the "big" buffer at the stage of
 * upsampled, non-color-processed data.  This is the only place that
 * makes sense when doing 2-pass quantization.  For processing multiple-scan
 * files without 2-pass quantization, it would be possible to develop another
 * controller that buffers the downsampled data instead, thus reducing the size
 * of the temp files (by about a factor of 2 in typical cases).  However,
 * our present upsampling logic is dependent on the assumption that
 * upsampling occurs during a scan, so it's much easier to do the
 * enlargement as the JPEG file is read.  This also simplifies life for the
 * memory manager, which would otherwise have to deal with overlapping
 * access_big_sarray() requests.
 * At present it appears that most JPEG files will be single-scan,
 * so it doesn't seem worthwhile to worry about this optimization.
 */

#ifdef NEED_COMPLEX_CONTROLLER

METHODDEF void
complex_dcontroller (cinfo)  decompress_info_ptr cinfo;
{
  long fullsize_width;		/* # of samples per row in full-size buffers */
  long cur_mcu_row;		/* counts # of MCU rows processed */
  long pixel_rows_output;	/* # of pixel rows actually emitted */
  int mcu_rows_per_loop;	/* # of MCU rows processed per outer loop */
  /* Work buffer for dequantized coefficients (IDCT input) */
  JBLOCKIMAGE coeff_data;
  /* Work buffer for cross-block smoothing input */
#ifdef BLOCK_SMOOTHING_SUPPORTED
  JBLOCKIMAGE bsmooth[3];	/* this is optional */
  int whichb;
#endif
  /* Work buffer for downsampled image data (see comments at head of file) */
  JSAMPIMAGE sampled_data[2];
  int whichss, ri;
  short ci, i;
  boolean single_scan;

  /* Compute dimensions of full-size pixel buffers */
  /* Note these are the same whether interleaved or not. */
  rows_in_mem = cinfo->max_v_samp_factor * DCTSIZE;
  fullsize_width = jround_up(cinfo->image_width,
			     (long) (cinfo->max_h_samp_factor * DCTSIZE));

  /* Allocate all working memory that doesn't depend on scan info */
  prepare_range_limit_table(cinfo);
  /* output_workspace is the color-processed data */
  output_workspace = alloc_sampimage(cinfo, (int) cinfo->final_out_comps,
				     (long) rows_in_mem, fullsize_width);

  /* Get a big image: fullsize_image is sample data after upsampling. */
  fullsize_image = (big_sarray_ptr *) (*cinfo->emethods->alloc_small)
			(cinfo->num_components * SIZEOF(big_sarray_ptr));
  for (ci = 0; ci < cinfo->num_components; ci++) {
    fullsize_image[ci] = (*cinfo->emethods->request_big_sarray)
			(fullsize_width,
			 jround_up(cinfo->image_height, (long) rows_in_mem),
			 (long) rows_in_mem);
  }
  /* Also get an area for pointers to currently accessible chunks */
  fullsize_ptrs = (JSAMPIMAGE) (*cinfo->emethods->alloc_small)
				(cinfo->num_components * SIZEOF(JSAMPARRAY));

  /* Tell the memory manager to instantiate big arrays */
  (*cinfo->emethods->alloc_big_arrays)
	 /* extra sarray space is for downsampled-data buffers: */
	((long) (fullsize_width			/* max width in samples */
	 * cinfo->max_v_samp_factor*(DCTSIZE+2)	/* max height */
	 * cinfo->num_components),		/* max components per scan */
	 /* extra barray space is for MCU-row buffers: */
	 (long) ((fullsize_width / DCTSIZE)	/* max width in blocks */
	 * cinfo->max_v_samp_factor		/* max height */
	 * cinfo->num_components		/* max components per scan */
	 * (cinfo->do_block_smoothing ? 4 : 1)),/* how many of these we need */
	 /* no extra "medium"-object space */
	 (long) 0);
  /* NB: if quantizer needs any "medium" size objects, it must get them */
  /* at color_quant_init time */

  /* If file is single-scan, we can do color quantization prescan on-the-fly
   * during the scan (we must be doing 2-pass quantization, else this method
   * would not have been selected).  If it is multiple scans, we have to make
   * a separate pass after we've collected all the components.  (We could save
   * some I/O by doing CQ prescan during the last scan, but the extra logic
   * doesn't seem worth the trouble.)
   */

  single_scan = (cinfo->comps_in_scan == cinfo->num_components);

  /* Account for passes needed (color quantizer adds its passes separately).
   * If multiscan file, we guess that each component has its own scan,
   * and increment completed_passes by the number of components in the scan.
   */

  if (single_scan)
    cinfo->total_passes++;	/* the single scan */
  else {
    cinfo->total_passes += cinfo->num_components; /* guessed # of scans */
    if (cinfo->two_pass_quantize)
      cinfo->total_passes++;	/* account for separate CQ prescan pass */
  }
  if (! cinfo->two_pass_quantize)
    cinfo->total_passes++;	/* count output pass unless quantizer does it */

  /* Loop over scans in file */

  do {
    
    /* Prepare for this scan */
    if (cinfo->comps_in_scan == 1) {
      noninterleaved_scan_setup(cinfo);
      /* Need to read Vk MCU rows to obtain Vk block rows */
      mcu_rows_per_loop = cinfo->cur_comp_info[0]->v_samp_factor;
    } else {
      interleaved_scan_setup(cinfo);
      /* in an interleaved scan, one MCU row provides Vk block rows */
      mcu_rows_per_loop = 1;
    }
    
    /* Allocate scan-local working memory */
    /* coeff_data holds a single MCU row of coefficient blocks */
    coeff_data = alloc_MCU_row(cinfo);
    /* if doing cross-block smoothing, need extra space for its input */
#ifdef BLOCK_SMOOTHING_SUPPORTED
    if (cinfo->do_block_smoothing) {
      bsmooth[0] = alloc_MCU_row(cinfo);
      bsmooth[1] = alloc_MCU_row(cinfo);
      bsmooth[2] = alloc_MCU_row(cinfo);
    }
#endif
    /* sampled_data is sample data before upsampling */
    alloc_sampling_buffer(cinfo, sampled_data);

    /* line up the big buffers for components in this scan */
    for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
      fullsize_ptrs[ci] = (*cinfo->emethods->access_big_sarray)
	(fullsize_image[cinfo->cur_comp_info[ci]->component_index],
	 (long) 0, TRUE);
    }
    
    /* Initialize to read scan data */
    
    (*cinfo->methods->entropy_decode_init) (cinfo);
    (*cinfo->methods->upsample_init) (cinfo);
    (*cinfo->methods->disassemble_init) (cinfo);
    
    /* Loop over scan's data: rows_in_mem pixel rows are processed per loop */
    
    pixel_rows_output = 0;
    whichss = 1;		/* arrange to start with sampled_data[0] */
    
    for (cur_mcu_row = 0; cur_mcu_row < cinfo->MCU_rows_in_scan;
	 cur_mcu_row += mcu_rows_per_loop) {
      (*cinfo->methods->progress_monitor) (cinfo, cur_mcu_row,
					   cinfo->MCU_rows_in_scan);

      whichss ^= 1;		/* switch to other downsampled-data buffer */

      /* Obtain v_samp_factor block rows of each component in the scan. */
      /* This is a single MCU row if interleaved, multiple MCU rows if not. */
      /* In the noninterleaved case there might be fewer than v_samp_factor */
      /* block rows remaining; if so, pad with copies of the last pixel row */
      /* so that upsampling doesn't have to treat it as a special case. */
      
      for (ri = 0; ri < mcu_rows_per_loop; ri++) {
	if (cur_mcu_row + ri < cinfo->MCU_rows_in_scan) {
	  /* OK to actually read an MCU row. */
#ifdef BLOCK_SMOOTHING_SUPPORTED
	  if (cinfo->do_block_smoothing)
	    get_smoothed_row(cinfo, coeff_data,
			     bsmooth, &whichb, cur_mcu_row + ri);
	  else
#endif
	    (*cinfo->methods->disassemble_MCU) (cinfo, coeff_data);
	  
	  (*cinfo->methods->reverse_DCT) (cinfo, coeff_data,
					  sampled_data[whichss],
					  ri * DCTSIZE);
	} else {
	  /* Need to pad out with copies of the last downsampled row. */
	  /* This can only happen if there is just one component. */
	  duplicate_row(sampled_data[whichss][0],
			cinfo->cur_comp_info[0]->downsampled_width,
			ri * DCTSIZE - 1, DCTSIZE);
	}
      }
      
      /* Upsample the data */
      /* First time through is a special case */
      
      if (cur_mcu_row) {
	/* Expand last row group of previous set */
	expand(cinfo, sampled_data[whichss], fullsize_ptrs, fullsize_width,
	       (short) DCTSIZE, (short) (DCTSIZE+1), (short) 0,
	       (short) (DCTSIZE-1));
	/* If single scan, can do color quantization prescan on-the-fly */
	if (single_scan)
	  (*cinfo->methods->color_quant_prescan) (cinfo, rows_in_mem,
						  fullsize_ptrs,
						  output_workspace[0]);
	/* Realign the big buffers */
	pixel_rows_output += rows_in_mem;
	for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	  fullsize_ptrs[ci] = (*cinfo->emethods->access_big_sarray)
	    (fullsize_image[cinfo->cur_comp_info[ci]->component_index],
	     pixel_rows_output, TRUE);
	}
	/* Expand first row group of this set */
	expand(cinfo, sampled_data[whichss], fullsize_ptrs, fullsize_width,
	       (short) (DCTSIZE+1), (short) 0, (short) 1,
	       (short) 0);
      } else {
	/* Expand first row group with dummy above-context */
	expand(cinfo, sampled_data[whichss], fullsize_ptrs, fullsize_width,
	       (short) (-1), (short) 0, (short) 1,
	       (short) 0);
      }
      /* Expand second through next-to-last row groups of this set */
      for (i = 1; i <= DCTSIZE-2; i++) {
	expand(cinfo, sampled_data[whichss], fullsize_ptrs, fullsize_width,
	       (short) (i-1), (short) i, (short) (i+1),
	       (short) i);
      }
    } /* end of loop over scan's data */
    
    /* Expand the last row group with dummy below-context */
    /* Note whichss points to last buffer side used */
    expand(cinfo, sampled_data[whichss], fullsize_ptrs, fullsize_width,
	   (short) (DCTSIZE-2), (short) (DCTSIZE-1), (short) (-1),
	   (short) (DCTSIZE-1));
    /* If single scan, finish on-the-fly color quantization prescan */
    if (single_scan)
      (*cinfo->methods->color_quant_prescan) (cinfo,
			(int) (cinfo->image_height - pixel_rows_output),
			fullsize_ptrs, output_workspace[0]);
    
    /* Clean up after the scan */
    (*cinfo->methods->disassemble_term) (cinfo);
    (*cinfo->methods->upsample_term) (cinfo);
    (*cinfo->methods->entropy_decode_term) (cinfo);
    (*cinfo->methods->read_scan_trailer) (cinfo);
    if (single_scan)
      cinfo->completed_passes++;
    else
      cinfo->completed_passes += cinfo->comps_in_scan;

    /* Release scan-local working memory */
    free_MCU_row(cinfo, coeff_data);
#ifdef BLOCK_SMOOTHING_SUPPORTED
    if (cinfo->do_block_smoothing) {
      free_MCU_row(cinfo, bsmooth[0]);
      free_MCU_row(cinfo, bsmooth[1]);
      free_MCU_row(cinfo, bsmooth[2]);
    }
#endif
    free_sampling_buffer(cinfo, sampled_data);
    
    /* Repeat if there is another scan */
  } while ((!single_scan) && (*cinfo->methods->read_scan_header) (cinfo));

  if (single_scan) {
    /* If we expected just one scan, make SURE there's just one */
    if ((*cinfo->methods->read_scan_header) (cinfo))
      WARNMS(cinfo->emethods, "Didn't expect more than one scan");
    /* We did the CQ prescan on-the-fly, so we are all set. */
  } else {
    /* For multiple-scan file, do the CQ prescan as a separate pass. */
    /* The main reason why prescan is passed the output_workspace is */
    /* so that we can use scan_big_image to call it... */
    if (cinfo->two_pass_quantize)
      scan_big_image(cinfo, cinfo->methods->color_quant_prescan);
  }

  /* Now that we've collected the data, do color processing and output */
  if (cinfo->two_pass_quantize)
    (*cinfo->methods->color_quant_doit) (cinfo, scan_big_image);
  else
    scan_big_image(cinfo, emit_1pass);

  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */
}

#endif /* NEED_COMPLEX_CONTROLLER */


/*
 * The method selection routine for decompression pipeline controllers.
 * Note that at this point we've already read the JPEG header and first SOS,
 * so we can tell whether the input is one scan or not.
 */

GLOBAL void
jseldpipeline (cinfo)  decompress_info_ptr cinfo;
{
  /* simplify subsequent tests on color quantization */
  if (! cinfo->quantize_colors)
    cinfo->two_pass_quantize = FALSE;
  
  if (cinfo->comps_in_scan == cinfo->num_components) {
    /* It's a single-scan file */
    if (cinfo->two_pass_quantize) {
#ifdef NEED_COMPLEX_CONTROLLER
      cinfo->methods->d_pipeline_controller = complex_dcontroller;
#else
      ERREXIT(cinfo->emethods, "2-pass quantization support was not compiled");
#endif
    } else
      cinfo->methods->d_pipeline_controller = simple_dcontroller;
  } else {
    /* It's a multiple-scan file */
#ifdef NEED_COMPLEX_CONTROLLER
    cinfo->methods->d_pipeline_controller = complex_dcontroller;
#else
    ERREXIT(cinfo->emethods, "Multiple-scan support was not compiled");
#endif
  }
}
#line 1 "jdsample.c"
/*
 * jdsample.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains upsampling routines.
 * These routines are invoked via the upsample and
 * upsample_init/term methods.
 *
 * An excellent reference for image resampling is
 *   Digital Image Warping, George Wolberg, 1990.
 *   Pub. by IEEE Computer Society Press, Los Alamitos, CA. ISBN 0-8186-8944-7.
 */

#include "jpeg.h"


/*
 * Initialize for upsampling a scan.
 */

METHODDEF void
upsample_init (cinfo)  decompress_info_ptr cinfo;
{
  /* no work for now */
}


/*
 * Upsample pixel values of a single component.
 * This version handles any integral sampling ratios.
 *
 * This is not used for typical JPEG files, so it need not be fast.
 * Nor, for that matter, is it particularly accurate: the algorithm is
 * simple replication of the input pixel onto the corresponding output
 * pixels.  The hi-falutin sampling literature refers to this as a
 * "box filter".  A box filter tends to introduce visible artifacts,
 * so if you are actually going to use 3:1 or 4:1 sampling ratios
 * you would be well advised to improve this code.
 */

METHODDEF void
int_upsample (cinfo, which_component, input_cols, input_rows, output_cols, output_rows, above, input_data, below, output_data)  decompress_info_ptr cinfo; int which_component;
	      long input_cols; int input_rows;
	      long output_cols; int output_rows;
	      JSAMPARRAY above; JSAMPARRAY input_data; JSAMPARRAY below;
	      JSAMPARRAY output_data;
{
  jpeg_component_info * compptr = cinfo->cur_comp_info[which_component];
  register JSAMPROW inptr, outptr;
  register JSAMPLE invalue;
  register short h_expand, h;
  short v_expand, v;
  int inrow, outrow;
  register long incol;

#ifdef DEBUG			/* for debugging pipeline controller */
  if (input_rows != compptr->v_samp_factor ||
      output_rows != cinfo->max_v_samp_factor ||
      (input_cols % compptr->h_samp_factor) != 0 ||
      (output_cols % cinfo->max_h_samp_factor) != 0 ||
      output_cols*compptr->h_samp_factor != input_cols*cinfo->max_h_samp_factor)
    ERREXIT(cinfo->emethods, "Bogus upsample parameters");
#endif

  h_expand = cinfo->max_h_samp_factor / compptr->h_samp_factor;
  v_expand = cinfo->max_v_samp_factor / compptr->v_samp_factor;

  outrow = 0;
  for (inrow = 0; inrow < input_rows; inrow++) {
    for (v = 0; v < v_expand; v++) {
      inptr = input_data[inrow];
      outptr = output_data[outrow++];
      for (incol = 0; incol < input_cols; incol++) {
	invalue = GETJSAMPLE(*inptr++);
	for (h = 0; h < h_expand; h++) {
	  *outptr++ = invalue;
	}
      }
    }
  }
}


/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 1:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */

METHODDEF void
h2v1_upsample (cinfo, which_component, input_cols, input_rows, output_cols, output_rows, above, input_data, below, output_data)  decompress_info_ptr cinfo; int which_component;
	       long input_cols; int input_rows;
	       long output_cols; int output_rows;
	       JSAMPARRAY above; JSAMPARRAY input_data; JSAMPARRAY below;
	       JSAMPARRAY output_data;
{
  register JSAMPROW inptr, outptr;
  register int invalue;
  int inrow;
  register long colctr;

#ifdef DEBUG			/* for debugging pipeline controller */
  jpeg_component_info * compptr = cinfo->cur_comp_info[which_component];
  if (input_rows != compptr->v_samp_factor ||
      output_rows != cinfo->max_v_samp_factor ||
      (input_cols % compptr->h_samp_factor) != 0 ||
      (output_cols % cinfo->max_h_samp_factor) != 0 ||
      output_cols*compptr->h_samp_factor != input_cols*cinfo->max_h_samp_factor)
    ERREXIT(cinfo->emethods, "Bogus upsample parameters");
#endif

  for (inrow = 0; inrow < input_rows; inrow++) {
    inptr = input_data[inrow];
    outptr = output_data[inrow];
    /* Special case for first column */
    invalue = GETJSAMPLE(*inptr++);
    *outptr++ = (JSAMPLE) invalue;
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(*inptr) + 2) >> 2);

    for (colctr = input_cols - 2; colctr > 0; colctr--) {
      /* General case: 3/4 * nearer pixel + 1/4 * further pixel */
      invalue = GETJSAMPLE(*inptr++) * 3;
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(inptr[-2]) + 2) >> 2);
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(*inptr) + 2) >> 2);
    }

    /* Special case for last column */
    invalue = GETJSAMPLE(*inptr);
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(inptr[-1]) + 2) >> 2);
    *outptr++ = (JSAMPLE) invalue;
  }
}


/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 2:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */

METHODDEF void
h2v2_upsample (cinfo, which_component, input_cols, input_rows, output_cols, output_rows, above, input_data, below, output_data)  decompress_info_ptr cinfo; int which_component;
	       long input_cols; int input_rows;
	       long output_cols; int output_rows;
	       JSAMPARRAY above; JSAMPARRAY input_data; JSAMPARRAY below;
	       JSAMPARRAY output_data;
{
  register JSAMPROW inptr0, inptr1, outptr;
#ifdef EIGHT_BIT_SAMPLES
  register int thiscolsum, lastcolsum, nextcolsum;
#else
  register INT32 thiscolsum, lastcolsum, nextcolsum;
#endif
  int inrow, outrow, v;
  register long colctr;

#ifdef DEBUG			/* for debugging pipeline controller */
  jpeg_component_info * compptr = cinfo->cur_comp_info[which_component];
  if (input_rows != compptr->v_samp_factor ||
      output_rows != cinfo->max_v_samp_factor ||
      (input_cols % compptr->h_samp_factor) != 0 ||
      (output_cols % cinfo->max_h_samp_factor) != 0 ||
      output_cols*compptr->h_samp_factor != input_cols*cinfo->max_h_samp_factor)
    ERREXIT(cinfo->emethods, "Bogus upsample parameters");
#endif

  outrow = 0;
  for (inrow = 0; inrow < input_rows; inrow++) {
    for (v = 0; v < 2; v++) {
      /* inptr0 points to nearest input row, inptr1 points to next nearest */
      inptr0 = input_data[inrow];
      if (v == 0) {		/* next nearest is row above */
	if (inrow == 0)
	  inptr1 = above[input_rows-1];
	else
	  inptr1 = input_data[inrow-1];
      } else {			/* next nearest is row below */
	if (inrow == input_rows-1)
	  inptr1 = below[0];
	else
	  inptr1 = input_data[inrow+1];
      }
      outptr = output_data[outrow++];

      /* Special case for first column */
      thiscolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 8) >> 4);
      lastcolsum = thiscolsum; thiscolsum = nextcolsum;

      for (colctr = input_cols - 2; colctr > 0; colctr--) {
	/* General case: 3/4 * nearer pixel + 1/4 * further pixel in each */
	/* dimension, thus 9/16, 3/16, 3/16, 1/16 overall */
	nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
	*outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
	*outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 8) >> 4);
	lastcolsum = thiscolsum; thiscolsum = nextcolsum;
      }

      /* Special case for last column */
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 8) >> 4);
    }
  }
}


/*
 * Upsample pixel values of a single component.
 * This version handles the special case of a full-size component.
 */

METHODDEF void
fullsize_upsample (cinfo, which_component, input_cols, input_rows, output_cols, output_rows, above, input_data, below, output_data)  decompress_info_ptr cinfo; int which_component;
		   long input_cols; int input_rows;
		   long output_cols; int output_rows;
		   JSAMPARRAY above; JSAMPARRAY input_data; JSAMPARRAY below;
		   JSAMPARRAY output_data;
{
#ifdef DEBUG			/* for debugging pipeline controller */
  if (input_cols != output_cols || input_rows != output_rows)
    ERREXIT(cinfo->emethods, "Pipeline controller messed up");
#endif

  jcopy_sample_rows(input_data, 0, output_data, 0, output_rows, output_cols);
}



/*
 * Clean up after a scan.
 */

METHODDEF void
upsample_term (cinfo)  decompress_info_ptr cinfo;
{
  /* no work for now */
}



/*
 * The method selection routine for upsampling.
 * Note that we must select a routine for each component.
 */

GLOBAL void
jselupsample (cinfo)  decompress_info_ptr cinfo;
{
  short ci;
  jpeg_component_info * compptr;

  if (cinfo->CCIR601_sampling)
    ERREXIT(cinfo->emethods, "CCIR601 upsampling not implemented yet");

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    if (compptr->h_samp_factor == cinfo->max_h_samp_factor &&
	compptr->v_samp_factor == cinfo->max_v_samp_factor)
      cinfo->methods->upsample[ci] = fullsize_upsample;
    else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	     compptr->v_samp_factor == cinfo->max_v_samp_factor)
      cinfo->methods->upsample[ci] = h2v1_upsample;
    else if (compptr->h_samp_factor * 2 == cinfo->max_h_samp_factor &&
	     compptr->v_samp_factor * 2 == cinfo->max_v_samp_factor)
      cinfo->methods->upsample[ci] = h2v2_upsample;
    else if ((cinfo->max_h_samp_factor % compptr->h_samp_factor) == 0 &&
	     (cinfo->max_v_samp_factor % compptr->v_samp_factor) == 0)
      cinfo->methods->upsample[ci] = int_upsample;
    else
      ERREXIT(cinfo->emethods, "Fractional upsampling not implemented yet");
  }

  cinfo->methods->upsample_init = upsample_init;
  cinfo->methods->upsample_term = upsample_term;
}
#line 1 "jmemmgr.c"
/*
 * jmemmgr.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides the standard system-independent memory management
 * routines.  This code is usable across a wide variety of machines; most
 * of the system dependencies have been isolated in a separate file.
 * The major functions provided here are:
 *   * bookkeeping to allow all allocated memory to be freed upon exit;
 *   * policy decisions about how to divide available memory among the
 *     various large arrays;
 *   * control logic for swapping virtual arrays between main memory and
 *     backing storage.
 * The separate system-dependent file provides the actual backing-storage
 * access code, and it contains the policy decision about how much total
 * main memory to use.
 * This file is system-dependent in the sense that some of its functions
 * are unnecessary in some systems.  For example, if there is enough virtual
 * memory so that backing storage will never be used, much of the big-array
 * control logic could be removed.  (Of course, if you have that much memory
 * then you shouldn't care about a little bit of unused code...)
 *
 * These routines are invoked via the methods alloc_small, free_small,
 * alloc_medium, free_medium, alloc_small_sarray, free_small_sarray,
 * alloc_small_barray, free_small_barray, request_big_sarray,
 * request_big_barray, alloc_big_arrays, access_big_sarray, access_big_barray,
 * free_big_sarray, free_big_barray, and free_all.
 */

#define AM_MEMORY_MANAGER	/* we define big_Xarray_control structs */

#include "jpeg.h"
#include "jpeg.h"		/* import the system-dependent declarations */

#ifndef NO_GETENV
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare getenv() */
#else
extern char * getenv PP((const char * name));
#endif
#endif


/*
 * On many systems it is not necessary to distinguish alloc_small from
 * alloc_medium; the main case where they must be distinguished is when
 * FAR pointers are distinct from regular pointers.  However, you might
 * want to keep them separate if you have different system-dependent logic
 * for small and large memory requests (i.e., jget_small and jget_large
 * do different things).
 */

#ifdef NEED_FAR_POINTERS
#define NEED_ALLOC_MEDIUM	/* flags alloc_medium really exists */
#endif


/*
 * Many machines require storage alignment: longs must start on 4-byte
 * boundaries, doubles on 8-byte boundaries, etc.  On such machines, malloc()
 * always returns pointers that are multiples of the worst-case alignment
 * requirement, and we had better do so too.  This means the headers that
 * we tack onto allocated structures had better have length a multiple of
 * the alignment requirement.
 * There isn't any really portable way to determine the worst-case alignment
 * requirement.  In this code we assume that the alignment requirement is
 * multiples of sizeof(align_type).  Here we define align_type as double;
 * with this definition, the code will run on all machines known to me.
 * If your machine has lesser alignment needs, you can save a few bytes
 * by making align_type smaller.
 */

typedef double align_type;


/*
 * Some important notes:
 *   The allocation routines provided here must never return NULL.
 *   They should exit to error_exit if unsuccessful.
 *
 *   It's not a good idea to try to merge the sarray and barray routines,
 *   even though they are textually almost the same, because samples are
 *   usually stored as bytes while coefficients are shorts.  Thus, in machines
 *   where byte pointers have a different representation from word pointers,
 *   the resulting machine code could not be the same.
 */


static external_methods_ptr methods; /* saved for access to error_exit */


#ifdef MEM_STATS		/* optional extra stuff for statistics */

/* These macros are the assumed overhead per block for malloc().
 * They don't have to be accurate, but the printed statistics will be
 * off a little bit if they are not.
 */
#define MALLOC_OVERHEAD  (SIZEOF(void *)) /* overhead for jget_small() */
#define MALLOC_FAR_OVERHEAD  (SIZEOF(void FAR *)) /* for jget_large() */

static long total_num_small = 0;	/* total # of small objects alloced */
static long total_bytes_small = 0;	/* total bytes requested */
static long cur_num_small = 0;		/* # currently alloced */
static long max_num_small = 0;		/* max simultaneously alloced */

#ifdef NEED_ALLOC_MEDIUM
static long total_num_medium = 0;	/* total # of medium objects alloced */
static long total_bytes_medium = 0;	/* total bytes requested */
static long cur_num_medium = 0;		/* # currently alloced */
static long max_num_medium = 0;		/* max simultaneously alloced */
#endif

static long total_num_sarray = 0;	/* total # of sarray objects alloced */
static long total_bytes_sarray = 0;	/* total bytes requested */
static long cur_num_sarray = 0;		/* # currently alloced */
static long max_num_sarray = 0;		/* max simultaneously alloced */

static long total_num_barray = 0;	/* total # of barray objects alloced */
static long total_bytes_barray = 0;	/* total bytes requested */
static long cur_num_barray = 0;		/* # currently alloced */
static long max_num_barray = 0;		/* max simultaneously alloced */


LOCAL void
print_mem_stats ()       
{
  /* since this is only a debugging stub, we can cheat a little on the
   * trace message mechanism... helpful 'cuz trace_message can't handle longs.
   */
  fprintf(stderr, "total_num_small = %ld\n", total_num_small);
  fprintf(stderr, "total_bytes_small = %ld\n", total_bytes_small);
  if (cur_num_small)
    fprintf(stderr, "cur_num_small = %ld\n", cur_num_small);
  fprintf(stderr, "max_num_small = %ld\n", max_num_small);
  
#ifdef NEED_ALLOC_MEDIUM
  fprintf(stderr, "total_num_medium = %ld\n", total_num_medium);
  fprintf(stderr, "total_bytes_medium = %ld\n", total_bytes_medium);
  if (cur_num_medium)
    fprintf(stderr, "cur_num_medium = %ld\n", cur_num_medium);
  fprintf(stderr, "max_num_medium = %ld\n", max_num_medium);
#endif
  
  fprintf(stderr, "total_num_sarray = %ld\n", total_num_sarray);
  fprintf(stderr, "total_bytes_sarray = %ld\n", total_bytes_sarray);
  if (cur_num_sarray)
    fprintf(stderr, "cur_num_sarray = %ld\n", cur_num_sarray);
  fprintf(stderr, "max_num_sarray = %ld\n", max_num_sarray);
  
  fprintf(stderr, "total_num_barray = %ld\n", total_num_barray);
  fprintf(stderr, "total_bytes_barray = %ld\n", total_bytes_barray);
  if (cur_num_barray)
    fprintf(stderr, "cur_num_barray = %ld\n", cur_num_barray);
  fprintf(stderr, "max_num_barray = %ld\n", max_num_barray);
}

#endif /* MEM_STATS */


LOCAL void
out_of_memory (which)  int which;
/* Report an out-of-memory error and stop execution */
/* If we compiled MEM_STATS support, report alloc requests before dying */
{
#ifdef MEM_STATS
  if (methods->trace_level <= 0) /* don't do it if free_all() will */
    print_mem_stats();		/* print optional memory usage statistics */
#endif
  ERREXIT1(methods, "Insufficient memory (case %d)", which);
}


/*
 * Management of "small" objects.
 * These are all-in-memory, and are in near-heap space on an 80x86.
 */

typedef union small_struct * small_ptr;

typedef union small_struct {
	small_ptr next;		/* next in list of allocated objects */
	align_type dummy;	/* ensures alignment of following storage */
      } small_hdr;

static small_ptr small_list;	/* head of list */


METHODDEF void *
alloc_small (sizeofobject)  size_t sizeofobject;
/* Allocate a "small" object */
{
  small_ptr result;

  sizeofobject += SIZEOF(small_hdr); /* add space for header */

#ifdef MEM_STATS
  total_num_small++;
  total_bytes_small += sizeofobject + MALLOC_OVERHEAD;
  cur_num_small++;
  if (cur_num_small > max_num_small) max_num_small = cur_num_small;
#endif

  result = (small_ptr) jget_small(sizeofobject);
  if (result == NULL)
    out_of_memory(1);

  result->next = small_list;
  small_list = result;
  result++;			/* advance past header */

  return (void *) result;
}


METHODDEF void
free_small (ptr)  void *ptr;
/* Free a "small" object */
{
  small_ptr hdr;
  small_ptr * llink;

  hdr = (small_ptr) ptr;
  hdr--;			/* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = &small_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      ERREXIT(methods, "Bogus free_small request");
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  jfree_small((void *) hdr);

#ifdef MEM_STATS
  cur_num_small--;
#endif
}


/*
 * Management of "medium-size" objects.
 * These are just like small objects except they are in the FAR heap.
 */

#ifdef NEED_ALLOC_MEDIUM

typedef union medium_struct FAR * medium_ptr;

typedef union medium_struct {
	medium_ptr next;	/* next in list of allocated objects */
	align_type dummy;	/* ensures alignment of following storage */
      } medium_hdr;

static medium_ptr medium_list;	/* head of list */


METHODDEF void FAR *
alloc_medium (sizeofobject)  size_t sizeofobject;
/* Allocate a "medium-size" object */
{
  medium_ptr result;

  sizeofobject += SIZEOF(medium_hdr); /* add space for header */

#ifdef MEM_STATS
  total_num_medium++;
  total_bytes_medium += sizeofobject + MALLOC_FAR_OVERHEAD;
  cur_num_medium++;
  if (cur_num_medium > max_num_medium) max_num_medium = cur_num_medium;
#endif

  result = (medium_ptr) jget_large(sizeofobject);
  if (result == NULL)
    out_of_memory(2);

  result->next = medium_list;
  medium_list = result;
  result++;			/* advance past header */

  return (void FAR *) result;
}


METHODDEF void
free_medium (ptr)  void FAR *ptr;
/* Free a "medium-size" object */
{
  medium_ptr hdr;
  medium_ptr FAR * llink;

  hdr = (medium_ptr) ptr;
  hdr--;			/* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = (medium_ptr FAR *) &medium_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      ERREXIT(methods, "Bogus free_medium request");
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  jfree_large((void FAR *) hdr);

#ifdef MEM_STATS
  cur_num_medium--;
#endif
}

#endif /* NEED_ALLOC_MEDIUM */


/*
 * Management of "small" (all-in-memory) 2-D sample arrays.
 * The pointers are in near heap, the samples themselves in FAR heap.
 * The header structure is adjacent to the row pointers.
 * To minimize allocation overhead and to allow I/O of large contiguous
 * blocks, we allocate the sample rows in groups of as many rows as possible
 * without exceeding MAX_ALLOC_CHUNK total bytes per allocation request.
 * Note that the big-array control routines, later in this file, know about
 * this chunking of rows ... and also how to get the rowsperchunk value!
 */

typedef struct small_sarray_struct * small_sarray_ptr;

typedef struct small_sarray_struct {
	small_sarray_ptr next;	/* next in list of allocated sarrays */
	long numrows;		/* # of rows in this array */
	long rowsperchunk;	/* max # of rows per allocation chunk */
	JSAMPROW dummy;		/* ensures alignment of following storage */
      } small_sarray_hdr;

static small_sarray_ptr small_sarray_list; /* head of list */


METHODDEF JSAMPARRAY
alloc_small_sarray (samplesperrow, numrows)  long samplesperrow; long numrows;
/* Allocate a "small" (all-in-memory) 2-D sample array */
{
  small_sarray_ptr hdr;
  JSAMPARRAY result;
  JSAMPROW workspace;
  long rowsperchunk, currow, i;

#ifdef MEM_STATS
  total_num_sarray++;
  cur_num_sarray++;
  if (cur_num_sarray > max_num_sarray) max_num_sarray = cur_num_sarray;
#endif

  /* Calculate max # of rows allowed in one allocation chunk */
  rowsperchunk = MAX_ALLOC_CHUNK / (samplesperrow * SIZEOF(JSAMPLE));
  if (rowsperchunk <= 0)
      ERREXIT(methods, "Image too wide for this implementation");

  /* Get space for header and row pointers; this is always "near" on 80x86 */
  hdr = (small_sarray_ptr) alloc_small((size_t) (numrows * SIZEOF(JSAMPROW)
						 + SIZEOF(small_sarray_hdr)));

  result = (JSAMPARRAY) (hdr+1); /* advance past header */

  /* Insert into list now so free_all does right thing if I fail */
  /* after allocating only some of the rows... */
  hdr->next = small_sarray_list;
  hdr->numrows = 0;
  hdr->rowsperchunk = rowsperchunk;
  small_sarray_list = hdr;

  /* Get the rows themselves; on 80x86 these are "far" */
  currow = 0;
  while (currow < numrows) {
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
#ifdef MEM_STATS
    total_bytes_sarray += rowsperchunk * samplesperrow * SIZEOF(JSAMPLE)
			  + MALLOC_FAR_OVERHEAD;
#endif
    workspace = (JSAMPROW) jget_large((size_t) (rowsperchunk * samplesperrow
						* SIZEOF(JSAMPLE)));
    if (workspace == NULL)
      out_of_memory(3);
    for (i = rowsperchunk; i > 0; i--) {
      result[currow++] = workspace;
      workspace += samplesperrow;
    }
    hdr->numrows = currow;
  }

  return result;
}


METHODDEF void
free_small_sarray (ptr)  JSAMPARRAY ptr;
/* Free a "small" (all-in-memory) 2-D sample array */
{
  small_sarray_ptr hdr;
  small_sarray_ptr * llink;
  long i;

  hdr = (small_sarray_ptr) ptr;
  hdr--;			/* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = &small_sarray_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      ERREXIT(methods, "Bogus free_small_sarray request");
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  /* Free the rows themselves; on 80x86 these are "far" */
  /* Note we only free the row-group headers! */
  for (i = 0; i < hdr->numrows; i += hdr->rowsperchunk) {
    jfree_large((void FAR *) ptr[i]);
  }

  /* Free header and row pointers */
  free_small((void *) hdr);

#ifdef MEM_STATS
  cur_num_sarray--;
#endif
}


/*
 * Management of "small" (all-in-memory) 2-D coefficient-block arrays.
 * This is essentially the same as the code for sample arrays, above.
 */

typedef struct small_barray_struct * small_barray_ptr;

typedef struct small_barray_struct {
	small_barray_ptr next;	/* next in list of allocated barrays */
	long numrows;		/* # of rows in this array */
	long rowsperchunk;	/* max # of rows per allocation chunk */
	JBLOCKROW dummy;	/* ensures alignment of following storage */
      } small_barray_hdr;

static small_barray_ptr small_barray_list; /* head of list */


METHODDEF JBLOCKARRAY
alloc_small_barray (blocksperrow, numrows)  long blocksperrow; long numrows;
/* Allocate a "small" (all-in-memory) 2-D coefficient-block array */
{
  small_barray_ptr hdr;
  JBLOCKARRAY result;
  JBLOCKROW workspace;
  long rowsperchunk, currow, i;

#ifdef MEM_STATS
  total_num_barray++;
  cur_num_barray++;
  if (cur_num_barray > max_num_barray) max_num_barray = cur_num_barray;
#endif

  /* Calculate max # of rows allowed in one allocation chunk */
  rowsperchunk = MAX_ALLOC_CHUNK / (blocksperrow * SIZEOF(JBLOCK));
  if (rowsperchunk <= 0)
      ERREXIT(methods, "Image too wide for this implementation");

  /* Get space for header and row pointers; this is always "near" on 80x86 */
  hdr = (small_barray_ptr) alloc_small((size_t) (numrows * SIZEOF(JBLOCKROW)
						 + SIZEOF(small_barray_hdr)));

  result = (JBLOCKARRAY) (hdr+1); /* advance past header */

  /* Insert into list now so free_all does right thing if I fail */
  /* after allocating only some of the rows... */
  hdr->next = small_barray_list;
  hdr->numrows = 0;
  hdr->rowsperchunk = rowsperchunk;
  small_barray_list = hdr;

  /* Get the rows themselves; on 80x86 these are "far" */
  currow = 0;
  while (currow < numrows) {
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
#ifdef MEM_STATS
    total_bytes_barray += rowsperchunk * blocksperrow * SIZEOF(JBLOCK)
			  + MALLOC_FAR_OVERHEAD;
#endif
    workspace = (JBLOCKROW) jget_large((size_t) (rowsperchunk * blocksperrow
						 * SIZEOF(JBLOCK)));
    if (workspace == NULL)
      out_of_memory(4);
    for (i = rowsperchunk; i > 0; i--) {
      result[currow++] = workspace;
      workspace += blocksperrow;
    }
    hdr->numrows = currow;
  }

  return result;
}


METHODDEF void
free_small_barray (ptr)  JBLOCKARRAY ptr;
/* Free a "small" (all-in-memory) 2-D coefficient-block array */
{
  small_barray_ptr hdr;
  small_barray_ptr * llink;
  long i;

  hdr = (small_barray_ptr) ptr;
  hdr--;			/* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = &small_barray_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      ERREXIT(methods, "Bogus free_small_barray request");
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  /* Free the rows themselves; on 80x86 these are "far" */
  /* Note we only free the row-group headers! */
  for (i = 0; i < hdr->numrows; i += hdr->rowsperchunk) {
    jfree_large((void FAR *) ptr[i]);
  }

  /* Free header and row pointers */
  free_small((void *) hdr);

#ifdef MEM_STATS
  cur_num_barray--;
#endif
}



/*
 * About "big" array management:
 *
 * To allow machines with limited memory to handle large images,
 * all processing in the JPEG system is done a few pixel or block rows
 * at a time.  The above "small" array routines are only used to allocate
 * strip buffers (as wide as the image, but just a few rows high).
 * In some cases multiple passes must be made over the data.  In these
 * cases the "big" array routines are used.  The array is still accessed
 * a strip at a time, but the memory manager must save the whole array
 * for repeated accesses.  The intended implementation is that there is
 * a strip buffer in memory (as high as is possible given the desired memory
 * limit), plus a backing file that holds the rest of the array.
 *
 * The request_big_array routines are told the total size of the image (in case
 * it is useful to know the total file size that will be needed).  They are
 * also given the unit height, which is the number of rows that will be
 * accessed at once; the in-memory buffer should be made a multiple of
 * this height for best efficiency.
 *
 * The request routines create control blocks (and may open backing files),
 * but they don't create the in-memory buffers.  This is postponed until
 * alloc_big_arrays is called.  At that time the total amount of space needed
 * is known (approximately, anyway), so free memory can be divided up fairly.
 *
 * The access_big_array routines are responsible for making a specific strip
 * area accessible (after reading or writing the backing file, if necessary).
 * Note that the access routines are told whether the caller intends to modify
 * the accessed strip; during a read-only pass this saves having to rewrite
 * data to disk.
 *
 * The typical access pattern is one top-to-bottom pass to write the data,
 * followed by one or more read-only top-to-bottom passes.  However, other
 * access patterns may occur while reading.  For example, translation of image
 * formats that use bottom-to-top scan order will require bottom-to-top read
 * passes.  The memory manager need not support multiple write passes nor
 * funny write orders (meaning that rearranging rows must be handled while
 * reading data out of the big array, not while putting it in).
 *
 * In current usage, the access requests are always for nonoverlapping strips;
 * that is, successive access start_row numbers always differ by exactly the
 * unitheight.  This allows fairly simple buffer dump/reload logic if the
 * in-memory buffer is made a multiple of the unitheight.  It would be
 * possible to keep downsampled rather than fullsize data in the "big" arrays,
 * thus reducing temp file size, if we supported overlapping strip access
 * (access requests differing by less than the unitheight).  At the moment
 * I don't believe this is worth the extra complexity.
 */



/* The control blocks for virtual arrays.
 * System-dependent info for the associated backing store is hidden inside
 * the backing_store_info struct.
 */

struct big_sarray_control {
	long rows_in_array;	/* total virtual array height */
	long samplesperrow;	/* width of array (and of memory buffer) */
	long unitheight;	/* # of rows accessed by access_big_sarray() */
	JSAMPARRAY mem_buffer;	/* the in-memory buffer */
	long rows_in_mem;	/* height of memory buffer */
	long rowsperchunk;	/* allocation chunk size in mem_buffer */
	long cur_start_row;	/* first logical row # in the buffer */
	boolean dirty;		/* do current buffer contents need written? */
	boolean b_s_open;	/* is backing-store data valid? */
	big_sarray_ptr next;	/* link to next big sarray control block */
	backing_store_info b_s_info; /* System-dependent control info */
};

static big_sarray_ptr big_sarray_list; /* head of list */

struct big_barray_control {
	long rows_in_array;	/* total virtual array height */
	long blocksperrow;	/* width of array (and of memory buffer) */
	long unitheight;	/* # of rows accessed by access_big_barray() */
	JBLOCKARRAY mem_buffer;	/* the in-memory buffer */
	long rows_in_mem;	/* height of memory buffer */
	long rowsperchunk;	/* allocation chunk size in mem_buffer */
	long cur_start_row;	/* first logical row # in the buffer */
	boolean dirty;		/* do current buffer contents need written? */
	boolean b_s_open;	/* is backing-store data valid? */
	big_barray_ptr next;	/* link to next big barray control block */
	backing_store_info b_s_info; /* System-dependent control info */
};

static big_barray_ptr big_barray_list; /* head of list */


METHODDEF big_sarray_ptr
request_big_sarray (samplesperrow, numrows, unitheight)  long samplesperrow; long numrows; long unitheight;
/* Request a "big" (virtual-memory) 2-D sample array */
{
  big_sarray_ptr result;

  /* get control block */
  result = (big_sarray_ptr) alloc_small(SIZEOF(struct big_sarray_control));

  result->rows_in_array = numrows;
  result->samplesperrow = samplesperrow;
  result->unitheight = unitheight;
  result->mem_buffer = NULL;	/* marks array not yet realized */
  result->b_s_open = FALSE;	/* no associated backing-store object */
  result->next = big_sarray_list; /* add to list of big arrays */
  big_sarray_list = result;

  return result;
}


METHODDEF big_barray_ptr
request_big_barray (blocksperrow, numrows, unitheight)  long blocksperrow; long numrows; long unitheight;
/* Request a "big" (virtual-memory) 2-D coefficient-block array */
{
  big_barray_ptr result;

  /* get control block */
  result = (big_barray_ptr) alloc_small(SIZEOF(struct big_barray_control));

  result->rows_in_array = numrows;
  result->blocksperrow = blocksperrow;
  result->unitheight = unitheight;
  result->mem_buffer = NULL;	/* marks array not yet realized */
  result->b_s_open = FALSE;	/* no associated backing-store object */
  result->next = big_barray_list; /* add to list of big arrays */
  big_barray_list = result;

  return result;
}


METHODDEF void
alloc_big_arrays (extra_small_samples, extra_small_blocks, extra_medium_space)  long extra_small_samples; long extra_small_blocks;
		  long extra_medium_space;
/* Allocate the in-memory buffers for any unrealized "big" arrays */
/* 'extra' values are upper bounds for total future small-array requests */
/* and far-heap requests */
{
  long total_extra_space = extra_small_samples * SIZEOF(JSAMPLE)
			   + extra_small_blocks * SIZEOF(JBLOCK)
			   + extra_medium_space;
  long space_per_unitheight, maximum_space, avail_mem;
  long unitheights, max_unitheights;
  big_sarray_ptr sptr;
  big_barray_ptr bptr;

  /* Compute the minimum space needed (unitheight rows in each buffer)
   * and the maximum space needed (full image height in each buffer).
   * These may be of use to the system-dependent jmem_available routine.
   */
  space_per_unitheight = 0;
  maximum_space = total_extra_space;
  for (sptr = big_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) { /* if not realized yet */
      space_per_unitheight += sptr->unitheight *
			      sptr->samplesperrow * SIZEOF(JSAMPLE);
      maximum_space += sptr->rows_in_array *
		       sptr->samplesperrow * SIZEOF(JSAMPLE);
    }
  }
  for (bptr = big_barray_list; bptr != NULL; bptr = bptr->next) {
    if (bptr->mem_buffer == NULL) { /* if not realized yet */
      space_per_unitheight += bptr->unitheight *
			      bptr->blocksperrow * SIZEOF(JBLOCK);
      maximum_space += bptr->rows_in_array *
		       bptr->blocksperrow * SIZEOF(JBLOCK);
    }
  }

  if (space_per_unitheight <= 0)
    return;			/* no unrealized arrays, no work */

  /* Determine amount of memory to actually use; this is system-dependent. */
  avail_mem = jmem_available(space_per_unitheight + total_extra_space,
			     maximum_space);

  /* If the maximum space needed is available, make all the buffers full
   * height; otherwise parcel it out with the same number of unitheights
   * in each buffer.
   */
  if (avail_mem >= maximum_space)
    max_unitheights = 1000000000L;
  else {
    max_unitheights = (avail_mem - total_extra_space) / space_per_unitheight;
    /* If there doesn't seem to be enough space, try to get the minimum
     * anyway.  This allows a "stub" implementation of jmem_available().
     */
    if (max_unitheights <= 0)
      max_unitheights = 1;
  }

  /* Allocate the in-memory buffers and initialize backing store as needed. */

  for (sptr = big_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) { /* if not realized yet */
      unitheights = (sptr->rows_in_array + sptr->unitheight - 1L)
		    / sptr->unitheight;
      if (unitheights <= max_unitheights) {
	/* This buffer fits in memory */
	sptr->rows_in_mem = sptr->rows_in_array;
      } else {
	/* It doesn't fit in memory, create backing store. */
	sptr->rows_in_mem = max_unitheights * sptr->unitheight;
	jopen_backing_store(& sptr->b_s_info,
			    (long) (sptr->rows_in_array *
				    sptr->samplesperrow * SIZEOF(JSAMPLE)));
	sptr->b_s_open = TRUE;
      }
      sptr->mem_buffer = alloc_small_sarray(sptr->samplesperrow,
					    sptr->rows_in_mem);
      /* Reach into the small_sarray header and get the rowsperchunk field.
       * Yes, I know, this is horrible coding practice.
       */
      sptr->rowsperchunk =
	((small_sarray_ptr) sptr->mem_buffer)[-1].rowsperchunk;
      sptr->cur_start_row = 0;
      sptr->dirty = FALSE;
    }
  }

  for (bptr = big_barray_list; bptr != NULL; bptr = bptr->next) {
    if (bptr->mem_buffer == NULL) { /* if not realized yet */
      unitheights = (bptr->rows_in_array + bptr->unitheight - 1L)
		    / bptr->unitheight;
      if (unitheights <= max_unitheights) {
	/* This buffer fits in memory */
	bptr->rows_in_mem = bptr->rows_in_array;
      } else {
	/* It doesn't fit in memory, create backing store. */
	bptr->rows_in_mem = max_unitheights * bptr->unitheight;
	jopen_backing_store(& bptr->b_s_info,
			    (long) (bptr->rows_in_array *
				    bptr->blocksperrow * SIZEOF(JBLOCK)));
	bptr->b_s_open = TRUE;
      }
      bptr->mem_buffer = alloc_small_barray(bptr->blocksperrow,
					    bptr->rows_in_mem);
      /* Reach into the small_barray header and get the rowsperchunk field. */
      bptr->rowsperchunk =
	((small_barray_ptr) bptr->mem_buffer)[-1].rowsperchunk;
      bptr->cur_start_row = 0;
      bptr->dirty = FALSE;
    }
  }
}


LOCAL void
do_sarray_io (ptr, writing)  big_sarray_ptr ptr; boolean writing;
/* Do backing store read or write of a "big" sample array */
{
  long bytesperrow, file_offset, byte_count, rows, i;

  bytesperrow = ptr->samplesperrow * SIZEOF(JSAMPLE);
  file_offset = ptr->cur_start_row * bytesperrow;
  /* Loop to read or write each allocation chunk in mem_buffer */
  for (i = 0; i < ptr->rows_in_mem; i += ptr->rowsperchunk) {
    /* One chunk, but check for short chunk at end of buffer */
    rows = MIN(ptr->rowsperchunk, ptr->rows_in_mem - i);
    /* Transfer no more than fits in file */
    rows = MIN(rows, ptr->rows_in_array - (ptr->cur_start_row + i));
    if (rows <= 0)		/* this chunk might be past end of file! */
      break;
    byte_count = rows * bytesperrow;
    if (writing)
      (*ptr->b_s_info.write_backing_store) (& ptr->b_s_info,
					    (void FAR *) ptr->mem_buffer[i],
					    file_offset, byte_count);
    else
      (*ptr->b_s_info.read_backing_store) (& ptr->b_s_info,
					   (void FAR *) ptr->mem_buffer[i],
					   file_offset, byte_count);
    file_offset += byte_count;
  }
}


LOCAL void
do_barray_io (ptr, writing)  big_barray_ptr ptr; boolean writing;
/* Do backing store read or write of a "big" coefficient-block array */
{
  long bytesperrow, file_offset, byte_count, rows, i;

  bytesperrow = ptr->blocksperrow * SIZEOF(JBLOCK);
  file_offset = ptr->cur_start_row * bytesperrow;
  /* Loop to read or write each allocation chunk in mem_buffer */
  for (i = 0; i < ptr->rows_in_mem; i += ptr->rowsperchunk) {
    /* One chunk, but check for short chunk at end of buffer */
    rows = MIN(ptr->rowsperchunk, ptr->rows_in_mem - i);
    /* Transfer no more than fits in file */
    rows = MIN(rows, ptr->rows_in_array - (ptr->cur_start_row + i));
    if (rows <= 0)		/* this chunk might be past end of file! */
      break;
    byte_count = rows * bytesperrow;
    if (writing)
      (*ptr->b_s_info.write_backing_store) (& ptr->b_s_info,
					    (void FAR *) ptr->mem_buffer[i],
					    file_offset, byte_count);
    else
      (*ptr->b_s_info.read_backing_store) (& ptr->b_s_info,
					   (void FAR *) ptr->mem_buffer[i],
					   file_offset, byte_count);
    file_offset += byte_count;
  }
}


METHODDEF JSAMPARRAY
access_big_sarray (ptr, start_row, writable)  big_sarray_ptr ptr; long start_row; boolean writable;
/* Access the part of a "big" sample array starting at start_row */
/* and extending for ptr->unitheight rows.  writable is true if  */
/* caller intends to modify the accessed area. */
{
  /* debugging check */
  if (start_row < 0 || start_row+ptr->unitheight > ptr->rows_in_array ||
      ptr->mem_buffer == NULL)
    ERREXIT(methods, "Bogus access_big_sarray request");

  /* Make the desired part of the virtual array accessible */
  if (start_row < ptr->cur_start_row ||
      start_row+ptr->unitheight > ptr->cur_start_row+ptr->rows_in_mem) {
    if (! ptr->b_s_open)
      ERREXIT(methods, "Virtual array controller messed up");
    /* Flush old buffer contents if necessary */
    if (ptr->dirty) {
      do_sarray_io(ptr, TRUE);
      ptr->dirty = FALSE;
    }
    /* Decide what part of virtual array to access.
     * Algorithm: if target address > current window, assume forward scan,
     * load starting at target address.  If target address < current window,
     * assume backward scan, load so that target address is top of window.
     * Note that when switching from forward write to forward read, will have
     * start_row = 0, so the limiting case applies and we load from 0 anyway.
     */
    if (start_row > ptr->cur_start_row) {
      ptr->cur_start_row = start_row;
    } else {
      ptr->cur_start_row = start_row + ptr->unitheight - ptr->rows_in_mem;
      if (ptr->cur_start_row < 0)
	ptr->cur_start_row = 0;	/* don't fall off front end of file */
    }
    /* If reading, read in the selected part of the array. 
     * If we are writing, we need not pre-read the selected portion,
     * since the access sequence constraints ensure it would be garbage.
     */
    if (! writable) {
      do_sarray_io(ptr, FALSE);
    }
  }
  /* Flag the buffer dirty if caller will write in it */
  if (writable)
    ptr->dirty = TRUE;
  /* Return address of proper part of the buffer */
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}


METHODDEF JBLOCKARRAY
access_big_barray (ptr, start_row, writable)  big_barray_ptr ptr; long start_row; boolean writable;
/* Access the part of a "big" coefficient-block array starting at start_row */
/* and extending for ptr->unitheight rows.  writable is true if  */
/* caller intends to modify the accessed area. */
{
  /* debugging check */
  if (start_row < 0 || start_row+ptr->unitheight > ptr->rows_in_array ||
      ptr->mem_buffer == NULL)
    ERREXIT(methods, "Bogus access_big_barray request");

  /* Make the desired part of the virtual array accessible */
  if (start_row < ptr->cur_start_row ||
      start_row+ptr->unitheight > ptr->cur_start_row+ptr->rows_in_mem) {
    if (! ptr->b_s_open)
      ERREXIT(methods, "Virtual array controller messed up");
    /* Flush old buffer contents if necessary */
    if (ptr->dirty) {
      do_barray_io(ptr, TRUE);
      ptr->dirty = FALSE;
    }
    /* Decide what part of virtual array to access.
     * Algorithm: if target address > current window, assume forward scan,
     * load starting at target address.  If target address < current window,
     * assume backward scan, load so that target address is top of window.
     * Note that when switching from forward write to forward read, will have
     * start_row = 0, so the limiting case applies and we load from 0 anyway.
     */
    if (start_row > ptr->cur_start_row) {
      ptr->cur_start_row = start_row;
    } else {
      ptr->cur_start_row = start_row + ptr->unitheight - ptr->rows_in_mem;
      if (ptr->cur_start_row < 0)
	ptr->cur_start_row = 0;	/* don't fall off front end of file */
    }
    /* If reading, read in the selected part of the array. 
     * If we are writing, we need not pre-read the selected portion,
     * since the access sequence constraints ensure it would be garbage.
     */
    if (! writable) {
      do_barray_io(ptr, FALSE);
    }
  }
  /* Flag the buffer dirty if caller will write in it */
  if (writable)
    ptr->dirty = TRUE;
  /* Return address of proper part of the buffer */
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}


METHODDEF void
free_big_sarray (ptr)  big_sarray_ptr ptr;
/* Free a "big" (virtual-memory) 2-D sample array */
{
  big_sarray_ptr * llink;

  /* Remove item from list -- linear search is fast enough */
  llink = &big_sarray_list;
  while (*llink != ptr) {
    if (*llink == NULL)
      ERREXIT(methods, "Bogus free_big_sarray request");
    llink = &( (*llink)->next );
  }
  *llink = ptr->next;

  if (ptr->b_s_open)		/* there may be no backing store */
    (*ptr->b_s_info.close_backing_store) (& ptr->b_s_info);

  if (ptr->mem_buffer != NULL)	/* just in case never realized */
    free_small_sarray(ptr->mem_buffer);

  free_small((void *) ptr);	/* free the control block too */
}


METHODDEF void
free_big_barray (ptr)  big_barray_ptr ptr;
/* Free a "big" (virtual-memory) 2-D coefficient-block array */
{
  big_barray_ptr * llink;

  /* Remove item from list -- linear search is fast enough */
  llink = &big_barray_list;
  while (*llink != ptr) {
    if (*llink == NULL)
      ERREXIT(methods, "Bogus free_big_barray request");
    llink = &( (*llink)->next );
  }
  *llink = ptr->next;

  if (ptr->b_s_open)		/* there may be no backing store */
    (*ptr->b_s_info.close_backing_store) (& ptr->b_s_info);

  if (ptr->mem_buffer != NULL)	/* just in case never realized */
    free_small_barray(ptr->mem_buffer);

  free_small((void *) ptr);	/* free the control block too */
}


/*
 * Cleanup: free anything that's been allocated since jselmemmgr().
 */

METHODDEF void
free_all ()       
{
  /* First free any open "big" arrays -- these may release small arrays */
  while (big_sarray_list != NULL)
    free_big_sarray(big_sarray_list);
  while (big_barray_list != NULL)
    free_big_barray(big_barray_list);
  /* Free any open small arrays -- these may release small objects */
  /* +1's are because we must pass a pointer to the data, not the header */
  while (small_sarray_list != NULL)
    free_small_sarray((JSAMPARRAY) (small_sarray_list + 1));
  while (small_barray_list != NULL)
    free_small_barray((JBLOCKARRAY) (small_barray_list + 1));
  /* Free any remaining small objects */
  while (small_list != NULL)
    free_small((void *) (small_list + 1));
#ifdef NEED_ALLOC_MEDIUM
  while (medium_list != NULL)
    free_medium((void FAR *) (medium_list + 1));
#endif

  jmem_term();			/* system-dependent cleanup */

#ifdef MEM_STATS
  if (methods->trace_level > 0)
    print_mem_stats();		/* print optional memory usage statistics */
#endif
}


/*
 * The method selection routine for virtual memory systems.
 * The system-dependent setup routine should call this routine
 * to install the necessary method pointers in the supplied struct.
 */

GLOBAL void
jselmemmgr (emethods)  external_methods_ptr emethods;
{
  methods = emethods;		/* save struct addr for error exit access */

  emethods->alloc_small = alloc_small;
  emethods->free_small = free_small;
#ifdef NEED_ALLOC_MEDIUM
  emethods->alloc_medium = alloc_medium;
  emethods->free_medium = free_medium;
#else
  emethods->alloc_medium = alloc_small;
  emethods->free_medium = free_small;
#endif
  emethods->alloc_small_sarray = alloc_small_sarray;
  emethods->free_small_sarray = free_small_sarray;
  emethods->alloc_small_barray = alloc_small_barray;
  emethods->free_small_barray = free_small_barray;
  emethods->request_big_sarray = request_big_sarray;
  emethods->request_big_barray = request_big_barray;
  emethods->alloc_big_arrays = alloc_big_arrays;
  emethods->access_big_sarray = access_big_sarray;
  emethods->access_big_barray = access_big_barray;
  emethods->free_big_sarray = free_big_sarray;
  emethods->free_big_barray = free_big_barray;
  emethods->free_all = free_all;

  /* Initialize list headers to empty */
  small_list = NULL;
#ifdef NEED_ALLOC_MEDIUM
  medium_list = NULL;
#endif
  small_sarray_list = NULL;
  small_barray_list = NULL;
  big_sarray_list = NULL;
  big_barray_list = NULL;

  jmem_init(emethods);		/* system-dependent initialization */

  /* Check for an environment variable JPEGMEM; if found, override the
   * default max_memory setting from jmem_init.  Note that a command line
   * -m argument may again override this value.
   * If your system doesn't support getenv(), define NO_GETENV to disable
   * this feature.
   */
#ifndef NO_GETENV
  { char * memenv;

    if ((memenv = getenv("JPEGMEM")) != NULL) {
      long lval;
      char ch = 'x';

      if (sscanf(memenv, "%ld%c", &lval, &ch) > 0) {
	if (ch == 'm' || ch == 'M')
	  lval *= 1000L;
	emethods->max_memory_to_use = lval * 1000L;
      }
    }
  }
#endif

}
#line 1 "jmemnobs.c"
/*
 * jmemnobs.c  (jmemsys.c)
 *
 * Copyright (C) 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a really simple implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that no backing-store files are needed: all required space
 * can be obtained from malloc().
 * This is very portable in the sense that it'll compile on almost anything,
 * but you'd better have lots of main memory (or virtual memory) if you want
 * to process big images.
 * Note that the max_memory_to_use option is ignored by this implementation.
 */

#include "jpeg.h"
#include "jpeg.h"

#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare malloc(), free() */
#else
extern void * malloc PP((size_t size));
extern void free PP((void *ptr));
#endif


static external_methods_ptr smethods; /* saved for access to error_exit */


/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */

GLOBAL void *
jget_small (sizeofobject)  size_t sizeofobject;
{
  return (void *) malloc(sizeofobject);
}

GLOBAL void
jfree_small (object)  void * object;
{
  free(object);
}

/*
 * We assume NEED_FAR_POINTERS is not defined and so the separate entry points
 * jget_large, jfree_large are not needed.
 */


/*
 * This routine computes the total memory space available for allocation.
 * Here we always say, "we got all you want bud!"
 */

GLOBAL long
jmem_available (min_bytes_needed, max_bytes_needed)  long min_bytes_needed; long max_bytes_needed;
{
  return max_bytes_needed;
}


/*
 * Backing store (temporary file) management.
 * This should never be called and we just error out.
 */

GLOBAL void
jopen_backing_store (info, total_bytes_needed)  backing_store_ptr info; long total_bytes_needed;
{
  ERREXIT(smethods, "Backing store not supported");
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Keep in mind that jmem_term may be called more than
 * once.
 */

GLOBAL void
jmem_init (emethods)  external_methods_ptr emethods;
{
  smethods = emethods;		/* save struct addr for error exit access */
  emethods->max_memory_to_use = 0;
}

GLOBAL void
jmem_term ()       
{
  /* no work */
}
#line 1 "jrdjfif.c"
/*
 * jrdjfif.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to decode standard JPEG file headers/markers.
 * This code will handle "raw JPEG" and JFIF-convention JPEG files.
 *
 * You can also use this module to decode a raw-JPEG or JFIF-standard data
 * stream that is embedded within a larger file.  To do that, you must
 * position the file to the JPEG SOI marker (0xFF/0xD8) that begins the
 * data sequence to be decoded.  If nothing better is possible, you can scan
 * the file until you see the SOI marker, then use JUNGETC to push it back.
 *
 * This module relies on the JGETC macro and the read_jpeg_data method (which
 * is provided by the user interface) to read from the JPEG data stream.
 * Therefore, this module is not dependent on any particular assumption about
 * the data source; it need not be a stdio stream at all.  (This fact does
 * NOT carry over to more complex JPEG file formats such as JPEG-in-TIFF;
 * those format control modules may well need to assume stdio input.)
 *
 * These routines are invoked via the methods read_file_header,
 * read_scan_header, read_jpeg_data, read_scan_trailer, and read_file_trailer.
 */

#include "jpeg.h"

#ifdef JFIF_SUPPORTED


typedef enum {			/* JPEG marker codes */
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,
  
  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,
  
  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,
  
  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,
  
  M_DHT   = 0xc4,
  
  M_DAC   = 0xcc,
  
  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,
  
  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,
  
  M_APP0  = 0xe0,
  M_APP15 = 0xef,
  
  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,
  
  M_TEM   = 0x01,
  
  M_ERROR = 0x100
} JPEG_MARKER;


/*
 * Reload the input buffer after it's been emptied, and return the next byte.
 * This is exported for direct use by the entropy decoder.
 * See the JGETC macro for calling conditions.  Note in particular that
 * read_jpeg_data may NOT return EOF.  If no more data is available, it must
 * exit via ERREXIT, or perhaps synthesize fake data (such as an RST marker).
 * For error recovery purposes, synthesizing an EOI marker is probably best.
 *
 * For this header control module, read_jpeg_data is supplied by the
 * user interface.  However, header formats that require random access
 * to the input file would need to supply their own code.  This code is
 * left here to indicate what is required.
 */

#if 0				/* not needed in this module */

METHODDEF int
read_jpeg_data (cinfo)  decompress_info_ptr cinfo;
{
  cinfo->next_input_byte = cinfo->input_buffer + MIN_UNGET;

  cinfo->bytes_in_buffer = (int) JFREAD(cinfo->input_file,
					cinfo->next_input_byte,
					JPEG_BUF_SIZE);
  
  if (cinfo->bytes_in_buffer <= 0) {
    WARNMS(cinfo->emethods, "Premature EOF in JPEG file");
    cinfo->next_input_byte[0] = (char) 0xFF;
    cinfo->next_input_byte[1] = (char) M_EOI;
    cinfo->bytes_in_buffer = 2;
  }

  return JGETC(cinfo);
}

#endif


/*
 * Routines to parse JPEG markers & save away the useful info.
 */


LOCAL INT32
get_2bytes (cinfo)  decompress_info_ptr cinfo;
/* Get a 2-byte unsigned integer (e.g., a marker parameter length field) */
{
  INT32 a;
  
  a = JGETC(cinfo);
  return (a << 8) + JGETC(cinfo);
}


LOCAL void
skip_variable (cinfo, code)  decompress_info_ptr cinfo; int code;
/* Skip over an unknown or uninteresting variable-length marker */
{
  INT32 length;
  
  length = get_2bytes(cinfo);
  
  TRACEMS2(cinfo->emethods, 1,
	   "Skipping marker 0x%02x, length %u", code, (int) length);
  
  for (length -= 2; length > 0; length--)
    (void) JGETC(cinfo);
}


LOCAL void
get_dht (cinfo)  decompress_info_ptr cinfo;
/* Process a DHT marker */
{
  INT32 length;
  UINT8 bits[17];
  UINT8 huffval[256];
  int i, index, count;
  HUFF_TBL **htblptr;
  
  length = get_2bytes(cinfo)-2;
  
  while (length > 0) {
    index = JGETC(cinfo);

    TRACEMS1(cinfo->emethods, 1, "Define Huffman Table 0x%02x", index);
      
    bits[0] = 0;
    count = 0;
    for (i = 1; i <= 16; i++) {
      bits[i] = (UINT8) JGETC(cinfo);
      count += bits[i];
    }

    length -= 1 + 16;

    TRACEMS8(cinfo->emethods, 2, "        %3d %3d %3d %3d %3d %3d %3d %3d",
	     bits[1], bits[2], bits[3], bits[4],
	     bits[5], bits[6], bits[7], bits[8]);
    TRACEMS8(cinfo->emethods, 2, "        %3d %3d %3d %3d %3d %3d %3d %3d",
	     bits[9], bits[10], bits[11], bits[12],
	     bits[13], bits[14], bits[15], bits[16]);

    if (count > 256 || ((INT32) count) > length)
      ERREXIT(cinfo->emethods, "Bogus DHT counts");

    for (i = 0; i < count; i++)
      huffval[i] = (UINT8) JGETC(cinfo);

    length -= count;

    if (index & 0x10) {		/* AC table definition */
      index -= 0x10;
      htblptr = &cinfo->ac_huff_tbl_ptrs[index];
    } else {			/* DC table definition */
      htblptr = &cinfo->dc_huff_tbl_ptrs[index];
    }

    if (index < 0 || index >= NUM_HUFF_TBLS)
      ERREXIT1(cinfo->emethods, "Bogus DHT index %d", index);

    if (*htblptr == NULL)
      *htblptr = (HUFF_TBL *) (*cinfo->emethods->alloc_small) (SIZEOF(HUFF_TBL));
  
    MEMCOPY((*htblptr)->bits, bits, SIZEOF((*htblptr)->bits));
    MEMCOPY((*htblptr)->huffval, huffval, SIZEOF((*htblptr)->huffval));
    }
}


LOCAL void
get_dac (cinfo)  decompress_info_ptr cinfo;
/* Process a DAC marker */
{
  INT32 length;
  int index, val;

  length = get_2bytes(cinfo)-2;
  
  while (length > 0) {
    index = JGETC(cinfo);
    val = JGETC(cinfo);

    TRACEMS2(cinfo->emethods, 1,
	     "Define Arithmetic Table 0x%02x: 0x%02x", index, val);

    if (index < 0 || index >= (2*NUM_ARITH_TBLS))
      ERREXIT1(cinfo->emethods, "Bogus DAC index %d", index);

    if (index >= NUM_ARITH_TBLS) { /* define AC table */
      cinfo->arith_ac_K[index-NUM_ARITH_TBLS] = (UINT8) val;
    } else {			/* define DC table */
      cinfo->arith_dc_L[index] = (UINT8) (val & 0x0F);
      cinfo->arith_dc_U[index] = (UINT8) (val >> 4);
      if (cinfo->arith_dc_L[index] > cinfo->arith_dc_U[index])
	ERREXIT1(cinfo->emethods, "Bogus DAC value 0x%x", val);
    }

    length -= 2;
  }
}


LOCAL void
get_dqt (cinfo)  decompress_info_ptr cinfo;
/* Process a DQT marker */
{
  INT32 length;
  int n, i, prec;
  UINT16 tmp;
  QUANT_TBL_PTR quant_ptr;
  
  length = get_2bytes(cinfo) - 2;
  
  while (length > 0) {
    n = JGETC(cinfo);
    prec = n >> 4;
    n &= 0x0F;

    TRACEMS2(cinfo->emethods, 1,
	     "Define Quantization Table %d  precision %d", n, prec);

    if (n >= NUM_QUANT_TBLS)
      ERREXIT1(cinfo->emethods, "Bogus table number %d", n);
      
    if (cinfo->quant_tbl_ptrs[n] == NULL)
      cinfo->quant_tbl_ptrs[n] = (QUANT_TBL_PTR)
	(*cinfo->emethods->alloc_small) (SIZEOF(QUANT_TBL));
    quant_ptr = cinfo->quant_tbl_ptrs[n];

    for (i = 0; i < DCTSIZE2; i++) {
      tmp = JGETC(cinfo);
      if (prec)
	tmp = (tmp<<8) + JGETC(cinfo);
      quant_ptr[i] = tmp;
    }

    for (i = 0; i < DCTSIZE2; i += 8) {
      TRACEMS8(cinfo->emethods, 2, "        %4u %4u %4u %4u %4u %4u %4u %4u",
	       quant_ptr[i  ], quant_ptr[i+1], quant_ptr[i+2], quant_ptr[i+3],
	       quant_ptr[i+4], quant_ptr[i+5], quant_ptr[i+6], quant_ptr[i+7]);
    }

    length -= DCTSIZE2+1;
    if (prec) length -= DCTSIZE2;
  }
}


LOCAL void
get_dri (cinfo)  decompress_info_ptr cinfo;
/* Process a DRI marker */
{
  if (get_2bytes(cinfo) != 4)
    ERREXIT(cinfo->emethods, "Bogus length in DRI");

  cinfo->restart_interval = (UINT16) get_2bytes(cinfo);

  TRACEMS1(cinfo->emethods, 1,
	   "Define Restart Interval %u", cinfo->restart_interval);
}


LOCAL void
get_app0 (cinfo)  decompress_info_ptr cinfo;
/* Process an APP0 marker */
{
#define JFIF_LEN 14
  INT32 length;
  UINT8 b[JFIF_LEN];
  int buffp;

  length = get_2bytes(cinfo) - 2;

  /* See if a JFIF APP0 marker is present */

  if (length >= JFIF_LEN) {
    for (buffp = 0; buffp < JFIF_LEN; buffp++)
      b[buffp] = (UINT8) JGETC(cinfo);
    length -= JFIF_LEN;

    if (b[0]==0x4A && b[1]==0x46 && b[2]==0x49 && b[3]==0x46 && b[4]==0) {
      /* Found JFIF APP0 marker: check version */
      /* Major version must be 1 */
      if (b[5] != 1)
	ERREXIT2(cinfo->emethods, "Unsupported JFIF revision number %d.%02d",
		 b[5], b[6]);
      /* Minor version should be 0..2, but try to process anyway if newer */
      if (b[6] > 2)
	TRACEMS2(cinfo->emethods, 1, "Warning: unknown JFIF revision number %d.%02d",
		 b[5], b[6]);
      /* Save info */
      cinfo->density_unit = b[7];
      cinfo->X_density = (b[8] << 8) + b[9];
      cinfo->Y_density = (b[10] << 8) + b[11];
      /* Assume colorspace is YCbCr, unless UI has overridden me */
      if (cinfo->jpeg_color_space == CS_UNKNOWN)
	cinfo->jpeg_color_space = CS_YCbCr;
      TRACEMS3(cinfo->emethods, 1, "JFIF APP0 marker, density %dx%d  %d",
	       cinfo->X_density, cinfo->Y_density, cinfo->density_unit);
      if (b[12] | b[13])
	TRACEMS2(cinfo->emethods, 1, "    with %d x %d thumbnail image",
		 b[12], b[13]);
      if (length != ((INT32) b[12] * (INT32) b[13] * (INT32) 3))
	TRACEMS1(cinfo->emethods, 1,
		 "Warning: thumbnail image size does not match data length %u",
		 (int) length);
    } else {
      TRACEMS1(cinfo->emethods, 1, "Unknown APP0 marker (not JFIF), length %u",
	       (int) length + JFIF_LEN);
    }
  } else {
    TRACEMS1(cinfo->emethods, 1, "Short APP0 marker, length %u", (int) length);
  }

  while (length-- > 0)		/* skip any remaining data */
    (void) JGETC(cinfo);
}


LOCAL void
get_sof (cinfo, code)  decompress_info_ptr cinfo; int code;
/* Process a SOFn marker */
{
  INT32 length;
  short ci;
  int c;
  jpeg_component_info * compptr;
  
  length = get_2bytes(cinfo);
  
  cinfo->data_precision = JGETC(cinfo);
  cinfo->image_height   = get_2bytes(cinfo);
  cinfo->image_width    = get_2bytes(cinfo);
  cinfo->num_components = JGETC(cinfo);

  TRACEMS4(cinfo->emethods, 1,
	   "Start Of Frame 0x%02x: width=%u, height=%u, components=%d",
	   code, (int) cinfo->image_width, (int) cinfo->image_height,
	   cinfo->num_components);

  /* We don't support files in which the image height is initially specified */
  /* as 0 and is later redefined by DNL.  As long as we have to check that,  */
  /* might as well have a general sanity check. */
  if (cinfo->image_height <= 0 || cinfo->image_width <= 0
      || cinfo->num_components <= 0)
    ERREXIT(cinfo->emethods, "Empty JPEG image (DNL not supported)");

#ifdef EIGHT_BIT_SAMPLES
  if (cinfo->data_precision != 8)
    ERREXIT(cinfo->emethods, "Unsupported JPEG data precision");
#endif
#ifdef TWELVE_BIT_SAMPLES
  if (cinfo->data_precision != 12) /* this needs more thought?? */
    ERREXIT(cinfo->emethods, "Unsupported JPEG data precision");
#endif
#ifdef SIXTEEN_BIT_SAMPLES
  if (cinfo->data_precision != 16) /* this needs more thought?? */
    ERREXIT(cinfo->emethods, "Unsupported JPEG data precision");
#endif

  if (length != (cinfo->num_components * 3 + 8))
    ERREXIT(cinfo->emethods, "Bogus SOF length");

  cinfo->comp_info = (jpeg_component_info *) (*cinfo->emethods->alloc_small)
			(cinfo->num_components * SIZEOF(jpeg_component_info));
  
  for (ci = 0; ci < cinfo->num_components; ci++) {
    compptr = &cinfo->comp_info[ci];
    compptr->component_index = ci;
    compptr->component_id = JGETC(cinfo);
    c = JGETC(cinfo);
    compptr->h_samp_factor = (c >> 4) & 15;
    compptr->v_samp_factor = (c     ) & 15;
    compptr->quant_tbl_no  = JGETC(cinfo);
      
    TRACEMS4(cinfo->emethods, 1, "    Component %d: %dhx%dv q=%d",
	     compptr->component_id, compptr->h_samp_factor,
	     compptr->v_samp_factor, compptr->quant_tbl_no);
  }
}


LOCAL void
get_sos (cinfo)  decompress_info_ptr cinfo;
/* Process a SOS marker */
{
  INT32 length;
  int i, ci, n, c, cc;
  jpeg_component_info * compptr;
  
  length = get_2bytes(cinfo);
  
  n = JGETC(cinfo);  /* Number of components */
  cinfo->comps_in_scan = n;
  length -= 3;
  
  if (length != (n * 2 + 3) || n < 1 || n > MAX_COMPS_IN_SCAN)
    ERREXIT(cinfo->emethods, "Bogus SOS length");

  TRACEMS1(cinfo->emethods, 1, "Start Of Scan: %d components", n);
  
  for (i = 0; i < n; i++) {
    cc = JGETC(cinfo);
    c = JGETC(cinfo);
    length -= 2;
    
    for (ci = 0; ci < cinfo->num_components; ci++)
      if (cc == cinfo->comp_info[ci].component_id)
	break;
    
    if (ci >= cinfo->num_components)
      ERREXIT(cinfo->emethods, "Invalid component number in SOS");
    
    compptr = &cinfo->comp_info[ci];
    cinfo->cur_comp_info[i] = compptr;
    compptr->dc_tbl_no = (c >> 4) & 15;
    compptr->ac_tbl_no = (c     ) & 15;
    
    TRACEMS3(cinfo->emethods, 1, "    c%d: [dc=%d ac=%d]", cc,
	     compptr->dc_tbl_no, compptr->ac_tbl_no);
  }
  
  while (length > 0) {
    (void) JGETC(cinfo);
    length--;
  }
}


LOCAL void
get_soi (cinfo)  decompress_info_ptr cinfo;
/* Process an SOI marker */
{
  int i;
  
  TRACEMS(cinfo->emethods, 1, "Start of Image");

  /* Reset all parameters that are defined to be reset by SOI */

  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    cinfo->arith_dc_L[i] = 0;
    cinfo->arith_dc_U[i] = 1;
    cinfo->arith_ac_K[i] = 5;
  }
  cinfo->restart_interval = 0;

  cinfo->density_unit = 0;	/* set default JFIF APP0 values */
  cinfo->X_density = 1;
  cinfo->Y_density = 1;

  cinfo->CCIR601_sampling = FALSE; /* Assume non-CCIR sampling */
}


LOCAL int
next_marker (cinfo)  decompress_info_ptr cinfo;
/* Find the next JPEG marker */
/* Note that the output might not be a valid marker code, */
/* but it will never be 0 or FF */
{
  int c, nbytes;

  nbytes = 0;
  do {
    do {			/* skip any non-FF bytes */
      nbytes++;
      c = JGETC(cinfo);
    } while (c != 0xFF);
    do {			/* skip any duplicate FFs */
      /* we don't increment nbytes here since extra FFs are legal */
      c = JGETC(cinfo);
    } while (c == 0xFF);
  } while (c == 0);		/* repeat if it was a stuffed FF/00 */

  if (nbytes != 1)
    WARNMS2(cinfo->emethods,
	    "Corrupt JPEG data: %d extraneous bytes before marker 0x%02x",
	    nbytes-1, c);

  return c;
}


LOCAL JPEG_MARKER
process_tables (cinfo)  decompress_info_ptr cinfo;
/* Scan and process JPEG markers that can appear in any order */
/* Return when an SOI, EOI, SOFn, or SOS is found */
{
  int c;

  while (TRUE) {
    c = next_marker(cinfo);
      
    switch (c) {
    case M_SOF0:
    case M_SOF1:
    case M_SOF2:
    case M_SOF3:
    case M_SOF5:
    case M_SOF6:
    case M_SOF7:
    case M_JPG:
    case M_SOF9:
    case M_SOF10:
    case M_SOF11:
    case M_SOF13:
    case M_SOF14:
    case M_SOF15:
    case M_SOI:
    case M_EOI:
    case M_SOS:
      return ((JPEG_MARKER) c);
      
    case M_DHT:
      get_dht(cinfo);
      break;
      
    case M_DAC:
      get_dac(cinfo);
      break;
      
    case M_DQT:
      get_dqt(cinfo);
      break;
      
    case M_DRI:
      get_dri(cinfo);
      break;
      
    case M_APP0:
      get_app0(cinfo);
      break;

    case M_RST0:		/* these are all parameterless */
    case M_RST1:
    case M_RST2:
    case M_RST3:
    case M_RST4:
    case M_RST5:
    case M_RST6:
    case M_RST7:
    case M_TEM:
      TRACEMS1(cinfo->emethods, 1, "Unexpected marker 0x%02x", c);
      break;

    default:	/* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn */
      skip_variable(cinfo, c);
      break;
    }
  }
}



/*
 * Initialize and read the file header (everything through the SOF marker).
 */

METHODDEF void
read_file_header (cinfo)  decompress_info_ptr cinfo;
{
  int c;

  /* Demand an SOI marker at the start of the file --- otherwise it's
   * probably not a JPEG file at all.  If the user interface wants to support
   * nonstandard headers in front of the SOI, it must skip over them itself
   * before calling jpeg_decompress().
   */
  if (JGETC(cinfo) != 0xFF  ||  JGETC(cinfo) != M_SOI)
    ERREXIT(cinfo->emethods, "Not a JPEG file");

  get_soi(cinfo);		/* OK, process SOI */

  /* Process markers until SOF */
  c = process_tables(cinfo);

  switch (c) {
  case M_SOF0:
  case M_SOF1:
    get_sof(cinfo, c);
    cinfo->arith_code = FALSE;
    break;
      
  case M_SOF9:
    get_sof(cinfo, c);
    cinfo->arith_code = TRUE;
    break;

  default:
    ERREXIT1(cinfo->emethods, "Unsupported SOF marker type 0x%02x", c);
    break;
  }

  /* Figure out what colorspace we have */
  /* (too bad the JPEG committee didn't provide a real way to specify this) */

  switch (cinfo->num_components) {
  case 1:
    cinfo->jpeg_color_space = CS_GRAYSCALE;
    break;

  case 3:
    /* if we saw a JFIF marker, leave it set to YCbCr; */
    /* also leave it alone if UI has provided a value */
    if (cinfo->jpeg_color_space == CS_UNKNOWN) {
      short cid0 = cinfo->comp_info[0].component_id;
      short cid1 = cinfo->comp_info[1].component_id;
      short cid2 = cinfo->comp_info[2].component_id;

      if (cid0 == 1 && cid1 == 2 && cid2 == 3)
	cinfo->jpeg_color_space = CS_YCbCr; /* assume it's JFIF w/out marker */
      else if (cid0 == 1 && cid1 == 4 && cid2 == 5)
	cinfo->jpeg_color_space = CS_YIQ; /* prototype's YIQ matrix */
      else {
	TRACEMS3(cinfo->emethods, 1,
		 "Unrecognized component IDs %d %d %d, assuming YCbCr",
		 cid0, cid1, cid2);
	cinfo->jpeg_color_space = CS_YCbCr;
      }
    }
    break;

  case 4:
    cinfo->jpeg_color_space = CS_CMYK;
    break;

  default:
    cinfo->jpeg_color_space = CS_UNKNOWN;
    break;
  }
}


/*
 * Read the start of a scan (everything through the SOS marker).
 * Return TRUE if find SOS, FALSE if find EOI.
 */

METHODDEF boolean
read_scan_header (cinfo)  decompress_info_ptr cinfo;
{
  int c;
  
  /* Process markers until SOS or EOI */
  c = process_tables(cinfo);
  
  switch (c) {
  case M_SOS:
    get_sos(cinfo);
    return TRUE;
    
  case M_EOI:
    TRACEMS(cinfo->emethods, 1, "End Of Image");
    return FALSE;

  default:
    ERREXIT1(cinfo->emethods, "Unexpected marker 0x%02x", c);
    break;
  }
  return FALSE;			/* keeps lint happy */
}


/*
 * The entropy decoder calls this routine if it finds a marker other than
 * the restart marker it was expecting.  (This code is *not* used unless
 * a nonzero restart interval has been declared.)  The passed parameter is
 * the marker code actually found (might be anything, except 0 or FF).
 * The desired restart marker is that indicated by cinfo->next_restart_num.
 * This routine is supposed to apply whatever error recovery strategy seems
 * appropriate in order to position the input stream to the next data segment.
 * For some file formats (eg, TIFF) extra information such as tile boundary
 * pointers may be available to help in this decision.
 *
 * This implementation is substantially constrained by wanting to treat the
 * input as a data stream; this means we can't back up.  (For instance, we
 * generally can't fseek() if the input is a Unix pipe.)  Therefore, we have
 * only the following actions to work with:
 *   1. Do nothing, let the entropy decoder resume at next byte of file.
 *   2. Read forward until we find another marker, discarding intervening
 *      data.  (In theory we could look ahead within the current bufferload,
 *      without having to discard data if we don't find the desired marker.
 *      This idea is not implemented here, in part because it makes behavior
 *      dependent on buffer size and chance buffer-boundary positions.)
 *   3. Push back the passed marker (with JUNGETC).  This will cause the
 *      entropy decoder to process an empty data segment, inserting dummy
 *      zeroes, and then re-read the marker we pushed back.
 * #2 is appropriate if we think the desired marker lies ahead, while #3 is
 * appropriate if the found marker is a future restart marker (indicating
 * that we have missed the desired restart marker, probably because it got
 * corrupted).

 * We apply #2 or #3 if the found marker is a restart marker no more than
 * two counts behind or ahead of the expected one.  We also apply #2 if the
 * found marker is not a legal JPEG marker code (it's certainly bogus data).
 * If the found marker is a restart marker more than 2 counts away, we do #1
 * (too much risk that the marker is erroneous; with luck we will be able to
 * resync at some future point).
 * For any valid non-restart JPEG marker, we apply #3.  This keeps us from
 * overrunning the end of a scan.  An implementation limited to single-scan
 * files might find it better to apply #2 for markers other than EOI, since
 * any other marker would have to be bogus data in that case.
 */

METHODDEF void
resync_to_restart (cinfo, marker)  decompress_info_ptr cinfo; int marker;
{
  int desired = cinfo->next_restart_num;
  int action = 1;

  /* Always put up a warning. */
  WARNMS2(cinfo->emethods,
	  "Corrupt JPEG data: found 0x%02x marker instead of RST%d",
	  marker, desired);
  /* Outer loop handles repeated decision after scanning forward. */
  for (;;) {
    if (marker < M_SOF0)
      action = 2;		/* invalid marker */
    else if (marker < M_RST0 || marker > M_RST7)
      action = 3;		/* valid non-restart marker */
    else {
      if (marker == (M_RST0 + ((desired+1) & 7)) ||
	  marker == (M_RST0 + ((desired+2) & 7)))
	action = 3;		/* one of the next two expected restarts */
      else if (marker == (M_RST0 + ((desired-1) & 7)) ||
	       marker == (M_RST0 + ((desired-2) & 7)))
	action = 2;		/* a prior restart, so advance */
      else
	action = 1;		/* desired restart or too far away */
    }
    TRACEMS2(cinfo->emethods, 4,
	     "At marker 0x%02x, recovery action %d", marker, action);
    switch (action) {
    case 1:
      /* Let entropy decoder resume processing. */
      return;
    case 2:
      /* Scan to the next marker, and repeat the decision loop. */
      marker = next_marker(cinfo);
      break;
    case 3:
      /* Put back this marker & return. */
      /* Entropy decoder will be forced to process an empty segment. */
      JUNGETC(marker, cinfo);
      JUNGETC(0xFF, cinfo);
      return;
    }
  }
}


/*
 * Finish up after a compressed scan (series of read_jpeg_data calls);
 * prepare for another read_scan_header call.
 */

METHODDEF void
read_scan_trailer (cinfo)  decompress_info_ptr cinfo;
{
  /* no work needed */
}


/*
 * Finish up at the end of the file.
 */

METHODDEF void
read_file_trailer (cinfo)  decompress_info_ptr cinfo;
{
  /* no work needed */
}


/*
 * The method selection routine for standard JPEG header reading.
 * Note that this must be called by the user interface before calling
 * jpeg_decompress.  When a non-JFIF file is to be decompressed (TIFF,
 * perhaps), the user interface must discover the file type and call
 * the appropriate method selection routine.
 */

GLOBAL void
jselrjfif (cinfo)  decompress_info_ptr cinfo;
{
  cinfo->methods->read_file_header = read_file_header;
  cinfo->methods->read_scan_header = read_scan_header;
  /* For JFIF/raw-JPEG format, the user interface supplies read_jpeg_data. */
#if 0
  cinfo->methods->read_jpeg_data = read_jpeg_data;
#endif
  cinfo->methods->resync_to_restart = resync_to_restart;
  cinfo->methods->read_scan_trailer = read_scan_trailer;
  cinfo->methods->read_file_trailer = read_file_trailer;
}

#endif /* JFIF_SUPPORTED */
#line 1 "jrevdct.c"
/*
 * jrevdct.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the basic inverse-DCT transformation subroutine.
 *
 * This implementation is based on an algorithm described in
 *   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 * The primary algorithm described there uses 11 multiplies and 29 adds.
 * We use their alternate method with 12 multiplies and 32 adds.
 * The advantage of this method is that no data path contains more than one
 * multiplication; this allows a very simple and accurate implementation in
 * scaled fixed-point arithmetic, with a minimal number of shifts.
 */

#include "jpeg.h"

/*
 * This routine is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif


/*
 * A 2-D IDCT can be done by 1-D IDCT on each row followed by 1-D IDCT
 * on each column.  Direct algorithms are also available, but they are
 * much more complex and seem not to be any faster when reduced to code.
 *
 * The poop on this scaling stuff is as follows:
 *
 * Each 1-D IDCT step produces outputs which are a factor of sqrt(N)
 * larger than the true IDCT outputs.  The final outputs are therefore
 * a factor of N larger than desired; since N=8 this can be cured by
 * a simple right shift at the end of the algorithm.  The advantage of
 * this arrangement is that we save two multiplications per 1-D IDCT,
 * because the y0 and y4 inputs need not be divided by sqrt(N).
 *
 * We have to do addition and subtraction of the integer inputs, which
 * is no problem, and multiplication by fractional constants, which is
 * a problem to do in integer arithmetic.  We multiply all the constants
 * by CONST_SCALE and convert them to integer constants (thus retaining
 * CONST_BITS bits of precision in the constants).  After doing a
 * multiplication we have to divide the product by CONST_SCALE, with proper
 * rounding, to produce the correct output.  This division can be done
 * cheaply as a right shift of CONST_BITS bits.  We postpone shifting
 * as long as possible so that partial sums can be added together with
 * full fractional precision.
 *
 * The outputs of the first pass are scaled up by PASS1_BITS bits so that
 * they are represented to better-than-integral precision.  These outputs
 * require BITS_IN_JSAMPLE + PASS1_BITS + 3 bits; this fits in a 16-bit word
 * with the recommended scaling.  (To scale up 12-bit sample data further, an
 * intermediate INT32 array would be needed.)
 *
 * To avoid overflow of the 32-bit intermediate results in pass 2, we must
 * have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
 * shows that the values given below are the most effective.
 */

#ifdef EIGHT_BIT_SAMPLES
#define CONST_BITS  13
#define PASS1_BITS  2
#else
#define CONST_BITS  13
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

#define ONE	((INT32) 1)

#define CONST_SCALE (ONE << CONST_BITS)

/* Convert a positive real constant to an integer scaled by CONST_SCALE. */

#define FIX(x)	((INT32) ((x) * CONST_SCALE + 0.5))

/* Some C compilers fail to reduce "FIX(constant)" at compile time, thus
 * causing a lot of useless floating-point operations at run time.
 * To get around this we use the following pre-calculated constants.
 * If you change CONST_BITS you may want to add appropriate values.
 * (With a reasonable C compiler, you can just rely on the FIX() macro...)
 */

#if CONST_BITS == 13
#define FIX_0_298631336  ((INT32)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((INT32)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((INT32)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((INT32)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((INT32)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((INT32)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((INT32)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((INT32)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((INT32)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((INT32)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((INT32)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((INT32)  25172)	/* FIX(3.072711026) */
#else
#define FIX_0_298631336  FIX(0.298631336)
#define FIX_0_390180644  FIX(0.390180644)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_765366865  FIX(0.765366865)
#define FIX_0_899976223  FIX(0.899976223)
#define FIX_1_175875602  FIX(1.175875602)
#define FIX_1_501321110  FIX(1.501321110)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_1_961570560  FIX(1.961570560)
#define FIX_2_053119869  FIX(2.053119869)
#define FIX_2_562915447  FIX(2.562915447)
#define FIX_3_072711026  FIX(3.072711026)
#endif


/* Descale and correctly round an INT32 value that's scaled by N bits.
 * We assume RIGHT_SHIFT rounds towards minus infinity, so adding
 * the fudge factor is correct for either sign of X.
 */

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

/* Multiply an INT32 variable by an INT32 constant to yield an INT32 result.
 * For 8-bit samples with the recommended scaling, all the variable
 * and constant values involved are no more than 16 bits wide, so a
 * 16x16->32 bit multiply can be used instead of a full 32x32 multiply;
 * this provides a useful speedup on many machines.
 * There is no way to specify a 16x16->32 multiply in portable C, but
 * some C compilers will do the right thing if you provide the correct
 * combination of casts.
 * NB: for 12-bit samples, a full 32-bit multiplication will be needed.
 */

#ifdef EIGHT_BIT_SAMPLES
#ifdef SHORTxSHORT_32		/* may work if 'int' is 32 bits */
#define MULTIPLY(var,const)  (((INT16) (var)) * ((INT16) (const)))
#endif
#ifdef SHORTxLCONST_32		/* known to work with Microsoft C 6.0 */
#define MULTIPLY(var,const)  (((INT16) (var)) * ((INT32) (const)))
#endif
#endif

#ifndef MULTIPLY		/* default definition */
#define MULTIPLY(var,const)  ((var) * (const))
#endif


/*
 * Perform the inverse DCT on one block of coefficients.
 */

GLOBAL void
j_rev_dct (data)  DCTBLOCK data;
{
  INT32 tmp0, tmp1, tmp2, tmp3;
  INT32 tmp10, tmp11, tmp12, tmp13;
  INT32 z1, z2, z3, z4, z5;
  register DCTELEM *dataptr;
  int rowctr;
  SHIFT_TEMPS

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  dataptr = data;
  for (rowctr = DCTSIZE-1; rowctr >= 0; rowctr--) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any row in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * row DCT calculations can be simplified this way.
     */

    if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
	 dataptr[5] | dataptr[6] | dataptr[7]) == 0) {
      /* AC terms all zero */
      DCTELEM dcval = (DCTELEM) (dataptr[0] << PASS1_BITS);
      
      dataptr[0] = dcval;
      dataptr[1] = dcval;
      dataptr[2] = dcval;
      dataptr[3] = dcval;
      dataptr[4] = dcval;
      dataptr[5] = dcval;
      dataptr[6] = dcval;
      dataptr[7] = dcval;
      
      dataptr += DCTSIZE;	/* advance pointer to next row */
      continue;
    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = (INT32) dataptr[2];
    z3 = (INT32) dataptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((INT32) dataptr[0] + (INT32) dataptr[4]) << CONST_BITS;
    tmp1 = ((INT32) dataptr[0] - (INT32) dataptr[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = (INT32) dataptr[7];
    tmp1 = (INT32) dataptr[5];
    tmp2 = (INT32) dataptr[3];
    tmp3 = (INT32) dataptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
    
    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
    
    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    dataptr[0] = (DCTELEM) DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[7] = (DCTELEM) DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    dataptr[1] = (DCTELEM) DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    dataptr[6] = (DCTELEM) DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = (DCTELEM) DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[5] = (DCTELEM) DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    dataptr[3] = (DCTELEM) DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    dataptr[4] = (DCTELEM) DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += DCTSIZE;		/* advance pointer to next row */
  }

  /* Pass 2: process columns. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  dataptr = data;
  for (rowctr = DCTSIZE-1; rowctr >= 0; rowctr--) {
    /* Columns of zeroes can be exploited in the same way as we did with rows.
     * However, the row calculation has created many nonzero AC terms, so the
     * simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */

#ifndef NO_ZERO_COLUMN_TEST
    if ((dataptr[DCTSIZE*1] | dataptr[DCTSIZE*2] | dataptr[DCTSIZE*3] |
	 dataptr[DCTSIZE*4] | dataptr[DCTSIZE*5] | dataptr[DCTSIZE*6] |
	 dataptr[DCTSIZE*7]) == 0) {
      /* AC terms all zero */
      DCTELEM dcval = (DCTELEM) DESCALE((INT32) dataptr[0], PASS1_BITS+3);
      
      dataptr[DCTSIZE*0] = dcval;
      dataptr[DCTSIZE*1] = dcval;
      dataptr[DCTSIZE*2] = dcval;
      dataptr[DCTSIZE*3] = dcval;
      dataptr[DCTSIZE*4] = dcval;
      dataptr[DCTSIZE*5] = dcval;
      dataptr[DCTSIZE*6] = dcval;
      dataptr[DCTSIZE*7] = dcval;
      
      dataptr++;		/* advance pointer to next column */
      continue;
    }
#endif

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = (INT32) dataptr[DCTSIZE*2];
    z3 = (INT32) dataptr[DCTSIZE*6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((INT32) dataptr[DCTSIZE*0] + (INT32) dataptr[DCTSIZE*4]) << CONST_BITS;
    tmp1 = ((INT32) dataptr[DCTSIZE*0] - (INT32) dataptr[DCTSIZE*4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = (INT32) dataptr[DCTSIZE*7];
    tmp1 = (INT32) dataptr[DCTSIZE*5];
    tmp2 = (INT32) dataptr[DCTSIZE*3];
    tmp3 = (INT32) dataptr[DCTSIZE*1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
    
    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
    
    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    dataptr[DCTSIZE*0] = (DCTELEM) DESCALE(tmp10 + tmp3,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*7] = (DCTELEM) DESCALE(tmp10 - tmp3,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*1] = (DCTELEM) DESCALE(tmp11 + tmp2,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*6] = (DCTELEM) DESCALE(tmp11 - tmp2,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*2] = (DCTELEM) DESCALE(tmp12 + tmp1,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*5] = (DCTELEM) DESCALE(tmp12 - tmp1,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*3] = (DCTELEM) DESCALE(tmp13 + tmp0,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*4] = (DCTELEM) DESCALE(tmp13 - tmp0,
					   CONST_BITS+PASS1_BITS+3);
    
    dataptr++;			/* advance pointer to next column */
  }
}
#line 1 "jutils.c"
/*
 * jutils.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains miscellaneous utility routines needed for both
 * compression and decompression.
 * Note we prefix all global names with "j" to minimize conflicts with
 * a surrounding application.
 */

#include "jpeg.h"


GLOBAL long
jround_up (a, b)  long a; long b;
/* Compute a rounded up to next multiple of b; a >= 0, b > 0 */
{
  a += b-1;
  return a - (a % b);
}


/* On normal machines we can apply MEMCOPY() and MEMZERO() to sample arrays
 * and coefficient-block arrays.  This won't work on 80x86 because the arrays
 * are FAR and we're assuming a small-pointer memory model.  However, some
 * DOS compilers provide far-pointer versions of memcpy() and memset() even
 * in the small-model libraries.  These will be used if USE_FMEM is defined.
 * Otherwise, the routines below do it the hard way.  (The performance cost
 * is not all that great, because these routines aren't very heavily used.)
 */

#ifndef NEED_FAR_POINTERS	/* normal case, same as regular macros */
#define FMEMCOPY(dest,src,size)	MEMCOPY(dest,src,size)
#define FMEMZERO(target,size)	MEMZERO(target,size)
#else				/* 80x86 case, define if we can */
#ifdef USE_FMEM
#define FMEMCOPY(dest,src,size)	_fmemcpy((void FAR *)(dest), (const void FAR *)(src), (size_t)(size))
#define FMEMZERO(target,size)	_fmemset((void FAR *)(target), 0, (size_t)(size))
#endif
#endif


GLOBAL void
jcopy_sample_rows (input_array, source_row, output_array, dest_row, num_rows, num_cols)  JSAMPARRAY input_array; int source_row;
		   JSAMPARRAY output_array; int dest_row;
		   int num_rows; long num_cols;
/* Copy some rows of samples from one place to another.
 * num_rows rows are copied from input_array[source_row++]
 * to output_array[dest_row++]; these areas should not overlap.
 * The source and destination arrays must be at least as wide as num_cols.
 */
{
  register JSAMPROW inptr, outptr;
#ifdef FMEMCOPY
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));
#else
  register long count;
#endif
  register int row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) {
    inptr = *input_array++;
    outptr = *output_array++;
#ifdef FMEMCOPY
    FMEMCOPY(outptr, inptr, count);
#else
    for (count = num_cols; count > 0; count--)
      *outptr++ = *inptr++;	/* needn't bother with GETJSAMPLE() here */
#endif
  }
}


GLOBAL void
jcopy_block_row (input_row, output_row, num_blocks)  JBLOCKROW input_row; JBLOCKROW output_row; long num_blocks;
/* Copy a row of coefficient blocks from one place to another. */
{
#ifdef FMEMCOPY
  FMEMCOPY(output_row, input_row, num_blocks * (DCTSIZE2 * SIZEOF(JCOEF)));
#else
  register JCOEFPTR inptr, outptr;
  register long count;

  inptr = (JCOEFPTR) input_row;
  outptr = (JCOEFPTR) output_row;
  for (count = num_blocks * DCTSIZE2; count > 0; count--) {
    *outptr++ = *inptr++;
  }
#endif
}


GLOBAL void
jzero_far (target, bytestozero)  void FAR * target; size_t bytestozero;
/* Zero out a chunk of FAR memory. */
/* This might be sample-array data, block-array data, or alloc_medium data. */
{
#ifdef FMEMZERO
  FMEMZERO(target, bytestozero);
#else
  register char FAR * ptr = (char FAR *) target;
  register size_t count;

  for (count = bytestozero; count > 0; count--) {
    *ptr++ = 0;
  }
#endif
}
