/* #ident	"@(#)x11:contrib/clients/xloadimage/pbm.h 6.7 93/07/23 Labtam" */
/* pbm.h:
 *
 * PBM header file
 *
 * jim frost 10.15.89
 */

typedef struct {
  unsigned char width[2];
  unsigned char height[2];
} PBMCompact;

#define PM_SCALE(a, b, c) (long)((a) * (c))/(b)