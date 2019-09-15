/* #ident	"@(#)x11:contrib/clients/xloadimage/imagetypes.c 6.11 93/07/23 Labtam" */
/* imagetypes.c:
 *
 * this contains things which reference the global ImageTypes array
 *
 * jim frost 09.27.89
 *
 * Copyright 1989, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "xli.h"
#include "imagetypes.h"
#include <errno.h>

extern int errno;

/* load a named image
 */

Image *loadImage(image_ops, verbose)
     ImageOptions *image_ops;
     boolean       verbose;
{ char   fullname[BUFSIZ];
  Image *image;
  int    a;

  if (findImage(image_ops->name, fullname) < 0) {
    if (errno == ENOENT)
      printf("%s: image not found\n", image_ops->name);
    else if (errno == EISDIR)
      printf("%s: directory\n", image_ops->name);
    else
      perror(fullname);
    return(NULL);
  }
  if(image_ops->loader_idx != -1) {	/* We've done this before !! */
    if (image= ImageTypes[image_ops->loader_idx].loader(fullname, image_ops, verbose)) {
      zreset(NULL);
      return(image);
    }
  } else {
    for (a= 0; ImageTypes[a].loader; a++)
      if (image= ImageTypes[a].loader(fullname, image_ops, verbose)) {
        zreset(NULL);
        return(image);
      }
  }
  printf("%s: unknown or unsupported image type\n", fullname);
  zreset(NULL);
  return(NULL);
}

/* identify what kind of image a named image is
 */

void identifyImage(name)
     char *name;
{ char fullname[BUFSIZ];
  int  a;

  if (findImage(name, fullname) < 0) {
    if (errno == ENOENT)
      printf("%s: image not found\n", name);
    else if (errno == EISDIR)
      printf("%s: directory\n", name);
    else
      perror(fullname);
    return;
  }
  for (a= 0; ImageTypes[a].identifier; a++) {
    if (ImageTypes[a].identifier(fullname, name)) {
      zreset(NULL);
      return;
    }
}
  zreset(NULL);
  printf("%s: unknown or unsupported image type\n", fullname);
}

/* tell user what image types we support
 */

void supportedImageTypes()
{ int a;

  printf("Image types supported:\n");
  for (a= 0; ImageTypes[a].name; a++)
    printf("  %s\n", ImageTypes[a].name);
}
