/* #ident	"@(#)x11:contrib/clients/xloadimage/doslib.c 1.4 94/07/29 Labtam" */
/* 
 * Library of support functions for MSDOS version
 *
 * Author: Graeme Gill
 *
 * Date: 23/2/93
 */ 
/*
 * Copyright 1993 Graeme W. Gill
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * for any purpose is hereby granted without fee, provided that the
 * above copyright notice and this permission notice appear in all
 * copies and that the copyright notice appears in supporting
 * documentation.  The author makes no representations about the
 * suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 */

#include "xli.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#ifdef MSDOS

/**********************************************************/
/* Lowest level graphics support */

/* Fill a rectangular block with the value */
Rfillcolor(base,span,width,height,color)
unsigned char *base;
int span;
int width,height;
unsigned long color;
	{
	for(;height > 0;height--,base += span)
		setpix(base,color,width);
	}

extern int _GrCanBcopyInBlit;

/* Copy a rectangular block from left to right */
Rcopyblt(dstbase,dstspan,width,height,srcbase,srcspan)
unsigned char *dstbase, *srcbase;
int dstspan,srcspan;
int width,height;
	{
	if ((unsigned int)srcbase >= (unsigned int)0xd0000000)
		srcbase = (unsigned char *)(((int)srcbase) | 0x100000);	/* read space */
	if ((unsigned int)dstbase >= (unsigned int)0xd0000000)
		dstbase = (unsigned char *)(((int)dstbase) | 0x200000);	/* write space */
	if (_GrCanBcopyInBlit)
		for(;height > 0;height--,dstbase += dstspan,srcbase+= srcspan)
			cpypixfwd(dstbase,srcbase,width);
	else {
		unsigned char *temp = (unsigned char *)malloc(width);
		for(;height > 0;height--,dstbase += dstspan,srcbase+= srcspan)
			{
			cpypixfwd(temp,srcbase,width);
			cpypixfwd(dstbase,temp,width);
			}
		free(temp);
		}
	}

/* Copy a rectangular block from right to left */
Rcopyblt1(dstbase,dstspan,width,height,srcbase,srcspan)
unsigned char *dstbase, *srcbase;
int dstspan,srcspan;
int width,height;
	{
	if ((unsigned int)srcbase >= (unsigned int)0xd0000000)
		srcbase = (unsigned char *)(((int)srcbase) | 0x100000);	/* read space */
	if ((unsigned int)dstbase >= (unsigned int)0xd0000000)
		dstbase = (unsigned char *)(((int)dstbase) | 0x200000);	/* write space */
	if (_GrCanBcopyInBlit)
		for(;height > 0;height--,dstbase += dstspan,srcbase+= srcspan)
			cpypixbwd(dstbase,srcbase,width);
	else {
		unsigned char *temp = (unsigned char *)malloc(width);
		for(;height > 0;height--,dstbase += dstspan,srcbase+= srcspan)
			{
			cpypixbwd(temp,srcbase,width);
			cpypixbwd(dstbase,temp,width);
			}
		free(temp);
		}
	}

/* Copy and translate a rectangular block (from left to right) */
Rxlateblt(dstbase,dstspan,width,height,srcbase,srcspan,table)
unsigned char *dstbase, *srcbase;
int dstspan,srcspan;
int width,height;
unsigned short *table;
	{
	if ((unsigned int)srcbase >= (unsigned int)0xd0000000)
		srcbase = (unsigned char *)(((int)srcbase) | 0x100000);	/* read space */
	if ((unsigned int)dstbase >= (unsigned int)0xd0000000)
		dstbase = (unsigned char *)(((int)dstbase) | 0x200000);	/* write space */
	if (_GrCanBcopyInBlit)
		for(;height > 0;height--,dstbase += dstspan,srcbase+= srcspan)
			xlatepix(dstbase,srcbase,width,table);
	else {
		unsigned char *temp = (unsigned char *)malloc(width);
		for(;height > 0;height--,dstbase += dstspan,srcbase+= srcspan)
			{
			cpypixfwd(temp,srcbase,width);
			xlatepix(dstbase,temp,width,table);
			}
		free(temp);
		}
	}


/* stipple a rectangular block */
Rstipple1to8(base,span,width,height,patbase,patoff,patspan,dum,color)
unsigned char *base, *patbase;
int width,height;
int span,patspan,patoff;
int dum;	/* dummy parameter */
unsigned long color;
	{
	unsigned char *d,*s;
	int w,pat,bit;
	for(;height > 0;height--,base += span,patbase+=patspan)
		{
		d = base;
		s = patbase;
		bit = 1 << patoff;
		pat = *s;
		for(w=width;w > 0;w--,d++)
			{
			if (pat & bit)
				*d = color;
			bit <<= 1;
			if (bit & 256)
				{
				bit = 1;
				s++;
				pat = *s;
				}
			}
		}
	}

/* reference big endian 4 bits to 4 bytes lookup */
static unsigned int reflu[16] =
	{
	0x00000000,	/* 0000 */
	0xff000000,	/* 0001 */
	0x00ff0000,	/* 0010 */
	0xffff0000,	/* 0011 */
	0x0000ff00,	/* 0100 */
	0xff00ff00,	/* 0101 */
	0x00ffff00,	/* 0110 */
	0xffffff00,	/* 0111 */
	0x000000ff,	/* 1000 */
	0xff0000ff,	/* 1001 */
	0x00ff00ff,	/* 1010 */
	0xffff00ff,	/* 1011 */
	0x0000ffff,	/* 1100 */
	0xff00ffff,	/* 1101 */
	0x00ffffff,	/* 1110 */
	0xffffffff	/* 1111 */
	};

/* opaque stipple a big endian rectangular block */
Rbigendian_copy1to8(base,span,width,height,patbase,patoff,patspan,fg,bg)
unsigned char *base, *patbase;
int width,height;
int span,patspan,patoff;
unsigned long fg,bg;
	{
	int fp;			/* bit feed point */
	unsigned int lu[16];	/* 4 bits to 4 bytes lookup */
	int lrag=0, mid=0, rrag=0;

	for(fp=0;fp<16;fp++)		/* create lookup table */
 		lu[fp] = ((reflu[fp] & (fg ^ bg)) ^ bg);

	lrag = (0-((int)base)) & 3;		/* left ragged */
	if (lrag > width)
		lrag = width;
	else
		{
		width -= lrag;
		mid = width >> 2;		/* number of words */
		rrag = width & 3;		/* right ragged */
		}
	
	/* (This isn't super fast for small areas) */
	for(;height > 0;height--,base += span,patbase+=patspan)
		{
		unsigned int bytes;
		unsigned char *d,*s;
		unsigned int bits;
		d = base;
		s = patbase;
		bits = (*s << 8) | *(++s);
		fp = 12 - patoff;
		if(lrag)
			{
			bytes = lu[(bits >> fp)&15];
			switch(lrag & 3)
				{
				case 0:
				case 1:
					*d = bytes;
					break;
				case 2:
					*((short *)d) = bytes;
					break;
				case 3:
					*d = bytes;
					*((short *)(d+1)) = (bytes >> 8);
					break;
				d = (unsigned char *)(((unsigned int)d) & ~3);
				fp -= lrag;
				if (fp < 4)	/* if need more bits */
					{
					bits = (bits << 8) | *(++s);
					fp += 8;
					}
				}
			}
		if(mid)
			{
			int w;
			for (w = mid; w > 0; w--, d+=4)
				{
				*((unsigned int *)d) = lu[(bits >> fp)&15];
				fp -= 4;
				if (fp < 4)	/* if need more bits */
					{
					bits = (bits << 8) | *(++s);
					fp += 8;
					}
				}
			}
		if(rrag)
			{
			bytes = lu[(bits >> fp)&15];
			switch(rrag & 3)
				{
				case 0:
				case 1:
					*d = bytes;
					break;
				case 2:
					*((short *)d) = bytes;
					break;
				case 3:
					*((short *)d) = bytes;
					*(d+2) = (bytes >> 16);
					break;
				}
			}
		}
	}


/* Render a bunch of text */
Rglblt(text,tlen,minmax,gsize,gbase,gspan,fw,fh,dbase,dspan,color)
char *text;		/* zero terminated code string */
int minmax;		/* low 8 bits is minimum code value, top 8 bits is max code value */
int gsize;		/* number of bytes per glyph bitmap */
unsigned char *gbase;	/* glyph array base address */
int gspan;		/* glyph span */
int fw,fh;		/* width, height of a character */
unsigned char *dbase;	/* destination base address */
int dspan;		/* destination span */
unsigned long color;	/* color of characters */
	{
	int min,max;
	int code;
	unsigned char *dpnt,*d,*s;
	int w,h;
	unsigned int pat,bit;

	min = minmax & 0xff;
	max = (minmax >> 8) - min;
	for(;tlen > 0;tlen--,text++,dbase += fw)
		{
		code = *text - min;
		if (code < 0 || code > max)
			continue;
		dpnt = dbase;
		s = gbase + code * gsize;
		for(h = fh;h > 0;h--,dpnt += dspan,s += gspan)
			{
			d = dpnt;
			bit = 1;
			pat = *s;	/* assume fonts are <= 32 wide */
			for(w=fw;w > 0;w--,d++,bit<<=1)
				{
				if (pat & bit)
					*d = color;
				}
			}
		}
	}

/* 32 bit pixel fill and copy routines */
asm(".text\n\
	.align 4\n\
	.globl _setpix\n\
_setpix:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %ebx\n\
	pushl %edi\n\
	movl 8(%ebp),%edi	# dest\n\
	movl 12(%ebp),%eax	# value\n\
	movl 16(%ebp),%ecx	# count\n\
	cmpl $6,%ecx\n\
	jbe L_setpix_2\n\
	movl %ecx,%ebx	# running count\n\
	movl %edi,%ecx\n\
	negl %ecx\n\
	andl $3,%ecx\n\
	subl %ecx,%ebx	# remaining count \n\
\n\
	jecxz L_setpix_3\n\
	rep\n\
	stosb\n\
L_setpix_3:\n\
\n\
	movl %ebx,%ecx\n\
	sarl $2,%ecx\n\
\n\
	je L_setpix_7\n\
	rep\n\
	stosl\n\
L_setpix_7:\n\
	movl %ebx,%ecx\n\
	andl $3,%ecx	# remaining count\n\
L_setpix_2:\n\
	jecxz L_setpix_11\n\
	rep\n\
	stosb\n\
L_setpix_11:\n\
	popl %edi\n\
	popl %ebx\n\
	leave\n\
	ret\n");

asm(".align 4\n\
	.globl _cpypixfwd\n\
_cpypixfwd:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %esi\n\
	pushl %edi\n\
	movl 8(%ebp),%edi	# dest\n\
	movl 12(%ebp),%esi	# source\n\
	movl 16(%ebp),%ecx	# count\n\
	cld\n\
	cmpl $6,%ecx\n\
	jbe L_cpypixfwd_2\n\
	movl %ecx,%eax	# running count\n\
	movl %edi,%ecx\n\
	negl %ecx\n\
	andl $3,%ecx	# bytes to align\n\
	subl %ecx,%eax	# remaining count \n\
\n\
	jecxz L_cpypixfwd_3\n\
	rep\n\
	movsb\n\
L_cpypixfwd_3:\n\
\n\
	movl %eax,%ecx\n\
	sarl $2,%ecx\n\
\n\
	je L_cpypixfwd_7\n\
	rep\n\
	movsl\n\
L_cpypixfwd_7:\n\
	movl %eax,%ecx\n\
	andl $3,%ecx	# remaining count\n\
L_cpypixfwd_2:\n\
	jecxz L_cpypixfwd_11\n\
	rep\n\
	movsb\n\
L_cpypixfwd_11:\n\
	popl %edi\n\
	popl %esi\n\
	leave\n\
	ret\n");

asm(".align 4\n\
	.globl _cpypixbwd\n\
_cpypixbwd:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %esi\n\
	pushl %edi\n\
	movl 8(%ebp),%edi	# dest\n\
	movl 12(%ebp),%esi	# source\n\
	movl 16(%ebp),%ecx	# count\n\
	addl	%ecx,%esi	# point to last byte\n\
	addl	%ecx,%edi\n\
	decl	%esi\n\
	decl	%edi\n\
	std					# set bwd direction\n\
	cmpl $6,%ecx\n\
	jbe L_cpypixbwd_2\n\
	movl %ecx,%eax	# running count\n\
	movl %edi,%ecx\n\
	addl $1,%ecx\n\
	andl $3,%ecx	# bytes to align\n\
	subl %ecx,%eax	# remaining count \n\
\n\
	jecxz L_cpypixbwd_3\n\
	rep\n\
	movsb\n\
L_cpypixbwd_3:\n\
\n\
	movl %eax,%ecx\n\
	sarl $2,%ecx\n\
\n\
	je L_cpypixbwd_7\n\
\n\
	subl $3,%esi	# point to word address\n\
	subl $3,%edi\n\
	rep\n\
	movsl\n\
	addl $3,%esi\n\
	addl $3,%edi\n\
\n\
L_cpypixbwd_7:\n\
	movl %eax,%ecx\n\
	andl $3,%ecx	# remaining count\n\
L_cpypixbwd_2:\n\
	jecxz L_cpypixbwd_11\n\
	rep\n\
	movsb\n\
L_cpypixbwd_11:\n\
	cld\n\
	popl %edi\n\
	popl %esi\n\
	leave\n\
	ret\n");

	/* translate pixels via 16 bit table */
asm(".align 4\n\
	.globl _xlatepix\n\
_xlatepix:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %ebx\n\
	pushl %esi\n\
	pushl %edi\n\
	movl 8(%ebp),%edi	# dest\n\
	movl 12(%ebp),%esi	# source\n\
	movl 16(%ebp),%ecx	# count\n\
	movl 20(%ebp),%edx	# table\n\
	cld\n\
	cmpl $6,%ecx\n\
	jbe L_xlatepix_2\n\
	movl %ecx,%ebx	# running count\n\
	movl %edi,%ecx\n\
	negl %ecx\n\
	andl $3,%ecx	# bytes to align\n\
	subl %ecx,%ebx	# remaining count \n\
\n\
	jecxz L_xlatepix_3\n\
L_xlatepix_4:\n\
	lodsb\n\
	andl $255,%eax\n\
	movzwl (%edx,%eax,2),%eax\n\
	stosb\n\
	loop	L_xlatepix_4\n\
L_xlatepix_3:\n\
\n\
	movl %ebx,%ecx\n\
	sarl $2,%ecx\n\
\n\
	je L_xlatepix_7\n\
	push %ebx\n\
L_xlatepix_6:\n\
	lodsl\n\
	movl %eax,%ebx\n\
	shrl $16,%eax\n\
	movzwl (%edx,%eax,2),%eax\n\
	andl $65535,%ebx\n\
	movzwl (%edx,%ebx,2),%ebx\n\
	shll $16,%eax\n\
	orl %ebx,%eax\n\
	stosl\n\
	loop	L_xlatepix_6\n\
	pop %ebx\n\
L_xlatepix_7:\n\
	movl %ebx,%ecx\n\
	andl $3,%ecx	# remaining count\n\
L_xlatepix_2:\n\
	jecxz L_xlatepix_11\n\
L_xlatepix_8:\n\
	lodsb\n\
	andl $255,%eax\n\
	movzwl (%edx,%eax,2),%eax\n\
	stosb\n\
	loop	L_xlatepix_8\n\
L_xlatepix_11:\n\
	popl %edi\n\
	popl %esi\n\
	popl %ebx\n\
	leave\n\
	ret\n");

#ifdef DJGCC_BUGS

/*
 * The library that comes with djgcc 1.09 has rather slow
 * memory routines. These are more optimal replacements.
 */

	asm("/* 32 bit bzero/memset functions */\n\
	.text\n\
	.align 4\n\
	.globl _bzero\n\
_bzero:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %ebx\n\
	pushl %edi\n\
	movl 8(%ebp),%edi	# dest\n\
	movl 12(%ebp),%ecx	# count\n\
	movl $0,%eax		# fill value of zero\n\
	jmp L_memset\n\
\n\
	.align 4\n\
	.globl _memset\n\
_memset:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %ebx\n\
	pushl %edi\n\
	movl 8(%ebp),%edi	# dest\n\
	movl 12(%ebp),%eax	# value\n\
	movl 16(%ebp),%ecx	# count\n\
\n\
L_memset:\n\
	cmpl $16,%ecx	# do small counts byte at a time\n\
	jbe L_memset_2\n\
\n\
	mov %al,%ah		# make value into 32 bit const.\n\
	movl %eax,%ebx\n\
	shll $16,%ebx\n\
	orl %ebx,%eax\n\
\n\
	movl %ecx,%ebx	# running count\n\
	movl %edi,%ecx\n\
	negl %ecx\n\
	andl $3,%ecx	# bytes to align\n\
	subl %ecx,%ebx	# remaining count \n\
\n\
	jecxz L_memset_3\n\
	rep\n\
	stosb			# align dest\n\
L_memset_3:\n\
\n\
	movl %ebx,%ecx\n\
	sarl $2,%ecx\n\
\n\
	je L_memset_7\n\
	rep\n\
	stosl			# do 32 bit fill\n\
L_memset_7:\n\
	movl %ebx,%ecx\n\
	andl $3,%ecx	# remaining count\n\
L_memset_2:\n\
	jecxz L_memset_11\n\
	rep\n\
	stosb			# finish off\n\
L_memset_11:\n\
	popl %edi\n\
	popl %ebx\n\
	leave\n\
	ret\n");

	asm("/* 32 bit bcopy/memcpy/memmove functions */\n\
	.text\n\
	.align 4\n\
	.globl _bcopy\n\
_bcopy:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %esi\n\
	pushl %edi\n\
	movl 8(%ebp),%esi	# source\n\
	movl 12(%ebp),%edi	# dest\n\
	movl 16(%ebp),%ecx	# count\n\
\n\
	cmp	%esi,%edi\n\
	jb	L_memcpyfwd\n\
	jmp	L_memcpybwd\n\
\n\
	.align 4\n\
	.globl _memcpy\n\
	.globl _memmove\n\
_memcpy:\n\
_memmove:\n\
	pushl %ebp\n\
	movl %esp,%ebp\n\
	pushl %esi\n\
	pushl %edi\n\
	movl 8(%ebp),%edi	# dest\n\
	movl 12(%ebp),%esi	# source\n\
	movl 16(%ebp),%ecx	# count\n\
\n\
	cmp	%esi,%edi\n\
	jb	L_memcpyfwd\n\
\n\
L_memcpybwd:\n\
	addl	%ecx,%esi	# point to last byte\n\
	addl	%ecx,%edi\n\
	decl	%esi\n\
	decl	%edi\n\
	std					# set bwd direction\n\
	cmpl $16,%ecx	# do small counts byte at a time\n\
	jbe L_memcpybwd_2\n\
	movl %ecx,%eax	# running count\n\
	movl %edi,%ecx\n\
	addl $1,%ecx\n\
	andl $3,%ecx	# bytes to align\n\
	subl %ecx,%eax	# remaining count \n\
\n\
	jecxz L_memcpybwd_3\n\
	rep\n\
	movsb\n\
L_memcpybwd_3:\n\
\n\
	movl %eax,%ecx\n\
	sarl $2,%ecx\n\
\n\
	je L_memcpybwd_7\n\
\n\
	subl $3,%esi	# point to word address\n\
	subl $3,%edi\n\
	rep\n\
	movsl			# do 32 bit copy\n\
	addl $3,%esi\n\
	addl $3,%edi\n\
\n\
L_memcpybwd_7:\n\
	movl %eax,%ecx\n\
	andl $3,%ecx	# remaining count\n\
L_memcpybwd_2:\n\
	jecxz L_memcpybwd_11\n\
	rep\n\
	movsb			# finish off\n\
L_memcpybwd_11:\n\
	cld\n\
	popl %edi\n\
	popl %esi\n\
	leave\n\
	ret\n");

	asm(".align 4\n\
L_memcpyfwd:\n\
	cld\n\
	cmpl $16,%ecx	# do small counts byte at a time\n\
	jbe L_memcpyfwd_2\n\
	movl %ecx,%eax	# running count\n\
	movl %edi,%ecx\n\
	negl %ecx\n\
	andl $3,%ecx	# bytes to align\n\
	subl %ecx,%eax	# remaining count \n\
\n\
	jecxz L_memcpyfwd_3\n\
	rep\n\
	movsb\n\
L_memcpyfwd_3:\n\
\n\
	movl %eax,%ecx\n\
	sarl $2,%ecx\n\
\n\
	je L_memcpyfwd_7\n\
	rep\n\
	movsl			# do 32 bit copy\n\
L_memcpyfwd_7:\n\
	movl %eax,%ecx\n\
	andl $3,%ecx	# remaining count\n\
L_memcpyfwd_2:\n\
	jecxz L_memcpyfwd_11\n\
	rep\n\
	movsb			# finish off\n\
L_memcpyfwd_11:\n\
	popl %edi\n\
	popl %esi\n\
	leave\n\
	ret\n");

	/* not sure what these two are for */
	asm(".text\n\
	.align	4\n\
	.globl	__bcopy\n\
__bcopy:\n\
	pushl	%ebp\n\
	movl	%esp,%ebp\n\
	pushl	%esi\n\
	pushl	%edi\n\
	movl	8(%ebp),%esi\n\
	movl	12(%ebp),%edi\n\
	movl	16(%ebp),%ecx\n\
	jecxz	L_bcopy2\n\
	cld\n\
	cmp	%esi,%edi\n\
	jb	L_bcopy1\n\
L_bcopy3:\n\
	std\n\
	addl	%ecx,%esi\n\
	addl	%ecx,%edi\n\
	decl	%esi\n\
	decl	%edi\n\
L_bcopy1:\n\
	lodsb\n\
	stosb\n\
	loop	L_bcopy1\n\
L_bcopy2:\n\
	cld\n\
	popl	%edi\n\
	popl	%esi\n\
	leave\n\
	ret\n");

	asm(".text\n\
	.align	4\n\
	.globl	__memcpy\n\
__memcpy:\n\
	pushl	%ebp\n\
	movl	%esp,%ebp\n\
	pushl	%esi\n\
	pushl	%edi\n\
	movl	8(%ebp),%edi\n\
	movl	12(%ebp),%esi\n\
	movl	16(%ebp),%ecx\n\
	jecxz	L_memcpy2\n\
	cld\n\
	cmp	%esi,%edi\n\
	jb	L_memcpy1\n\
L_memcpy3:\n\
	std\n\
	addl	%ecx,%esi\n\
	addl	%ecx,%edi\n\
	decl	%esi\n\
	decl	%edi\n\
L_memcpy1:\n\
	lodsb\n\
	stosb\n\
	loop	L_memcpy1\n\
L_memcpy2:\n\
	cld\n\
	popl	%edi\n\
	popl	%esi\n\
	leave\n\
	ret\n");

#endif	/* DJGCC_BUGS */

#ifdef DO_ARG_EXP

/* Argument wildcard expansion functions 1.3 */

/*
 * This emulates the functionality of the standard UNIX shell
 * parameter processing. Note that since MSDOS uses the '\'
 * character as a directory delimiter, it can't be used here
 * to escape metacharacters. Use '...' instead. Note also that
 * command.com gobles up " characters, so they can't be used
 * to protect metacharacters either.
 *
 *  In the pattern string:
 *       `*' matches any sequence of characters (zero or more)
 *       `?' matches any character
 *       [SET] matches any character in the specified set,
 *       [!SET] or [^SET] matches any character not in the specified set.
 *       SET can be a list eg. 'abcd'
 *       SET can be a range eg. 'a-z', '0-9', 'A-Z'
 *       Metacharacters characters can be protected with single quotes, eg. '[*'
 *       A single quote can be specified as ''      
 *       The directory separator is either '/' or '\'
 *       The matching is case sensitive.
 *
 *  Also included are two other useful functions:
 * 
 *  dos_name_stuff(spec,edp,udp,np,esp) - split up a dos file/directory
 *           specification up so it can be worked with.
 *    it splits it into:
 * 
 *    ed: effective directory, ie. a directory that can be opened.
 *    ud: user pretty version of the above.
 *    n: name - any part of the path left after directory extraction.
 *    es: effective specification - a spec that can be opened.
 *
 *    for example:
 *    in        ed          ud         n          es
 *
 *    fred      .                      fred       ./fred
 *    b:fred    b:.         b:         fred       b:./fred
 *    b:./fred  b:.         b:./       fred       b:./fred
 *    b:/fred   b:/         b:/        fred       b:/fred
 *    /fred     /           /          fred       /fred
 *    ./fred    .           ./         fred       ./fred
 *    .         .           .                     .
 *    /         /           /                     /
 *    b:        b:.         b:                    b:.
 *    b:/       b:/         b:/                   b:/
 *    b:./      b:.         b:./                  b:.
 *
 *  strcatpath(s1,s2)  - concatenate strings to make up a path.
 *           This function takes care of not doubling '/' seperators,
 *           and not creating leading '/'s
 *
 * Author: Graeme W. Gill
 * Date:   93/2/28
 */
/*
 * Copyright 1993 Graeme W. Gill
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * for any purpose is hereby granted without fee, provided that the
 * above copyright notice and this permission notice appear in all
 * copies and that the copyright notice appears in supporting
 * documentation.  The author makes no representations about the
 * suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

void argexp();

#ifndef HAVE_BOOLEAN
#define HAVE_BOOLEAN
typedef int boolean;
#endif

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define FLAG_MASK   3	/* mask for type flag */
#define FLAG_EXACT  0	/* default - exact match with low char */
#define FLAG_RANGE  1	/* in range of low to high char */
#define FLAG_ANYONE 2	/* any single character */
#define FLAG_ANYANY 3	/* zero or more any characters */

#define FLAG_INVSET 4	/* inverted matching set */
#define FLAG_SETEND 8	/* end of a set */
#define FLAG_DIREND 16	/* directory after pattern */
#define FLAG_PATEND 32	/* end of pattern */

typedef struct {
			char flags;	/* flags */
			char low;	/* low or exact match */
			char high;	/* high (if any) */
		} pattern;

static void searchdir();
static void addarg();
static boolean makepattern();
static boolean match();
static void remove_prot();

#ifdef _NEVER_
/* echo like test program */
main(argc,argv)
int argc;
char **argv;
	{
	int i;
	argexp(&argc,&argv);
	for (i = 1; i < argc; i++)
		{
		printf("%s ",argv[i]);
		}
	free_argexp(argc,argv);
	}
#endif /* _NEVER_ */

/* free space allocated by argexp() */
free_argexp(argc,argv)
int argc;
char **argv;
	{
	int i;
	for (i = 0; i < argc; i++)
		free(argv[i]);
	free(argv);
	}

/* Do wildcard expansion. Replace argc and argv with values */
/* allocated here. */
void
argexp(argcp,argvp)
int *argcp;
char ***argvp;
	{
	int nargc=0;		/* new argc value */
	char **nargv=NULL;	/* new argv */
	int argsp = 0;	/* space allocated for nargv */
	int i;
	
	addarg(&nargc,&nargv,&argsp,(*argvp)[0]);	/* argv[0] */

	for (i = 1; i < *argcp; i++)
		{
		char *startdir;
		pattern *p;
		int curargc;
		char *ed;		/* effective directory (+ drive) */
		char *ud;		/* user specified directory (+ drive) */
		char *n;		/* name left after user directory is extracted */
		char *es;		/* effective specification */

		dos_name_stuff((*argvp)[i],&ed,&ud,&n,&es);

		curargc = nargc;

		p = (pattern *)malloc(strlen(n) * sizeof(pattern));
		if (p == NULL)
			{
			fprintf(stderr,"Memory exhausted\n");
			exit(-1);
			}
		if (makepattern(n,p))
			{
			searchdir(ed,ud,p,&nargc,&nargv,&argsp);
			}
		free(p);
		free(ed);
		free(ud);
		free(n);
		free(es);

		if (curargc == nargc)	/* expansion didn't add an argument */
			{
			remove_prot((*argvp)[i]);	/* do protection processing */
			addarg(&nargc,&nargv,&argsp,(*argvp)[i]);	/* add input argument */
			}
		}
	/* Use a NULL as the last argv */
	addarg(&nargc,&nargv,&argsp,NULL);

	/* replace original arguments with expanded ones */
	*argcp = nargc;
	*argvp = nargv;
	}

/* add an argument to the new argument list */
static void
addarg(nargcp,nargvp,nargsp,string)
int *nargcp;	/* pointer to new arguments count */
char ***nargvp;	/* pointer to new arguments values */
int *nargsp;	/* pointer to new arguments size */
char *string;	/* string to add (NULL if add NULL pointer) */
	{
	char *d;
	if (*nargsp < (*nargcp + 1))
		{
		*nargsp = (*nargcp + 20);
		if (*nargvp == 0)	/* some reallocs can't handle null */
			*nargvp = (char **)malloc(*nargsp * sizeof(char *));
		else
			*nargvp = (char **)realloc(*nargvp,*nargsp * sizeof(char *));
		if (*nargvp == NULL)
			{
			fprintf(stderr,"Memory exhausted\n");
			exit(-1);
			}
		}
	if (string != NULL)
		{
		(*nargvp)[*nargcp] = (char *)malloc(strlen(string) + 1);
		if ((*nargvp)[*nargcp] == NULL)
			{
			fprintf(stderr,"Memory exhausted\n");
			exit(-1);
			}
  		strcpy((*nargvp)[*nargcp], string);
		(*nargcp)++;
		}
	else
		(*nargvp)[*nargcp] = NULL;
	}

static void
searchdir(dir,udir,p,nargcp,nargvp,nargsp)
char *dir;	/* effective directory */
char *udir;	/* user specified directory */
pattern *p;
int *nargcp;
char ***nargvp;
int *nargsp;
	{
	DIR *dirp;
	struct dirent *dp;
	struct stat buf;		/* for finding out what a file is */
	pattern *pe;			/* pattern end */
	
	for(pe = p;;pe++)
		{
		if((pe->flags & FLAG_DIREND) || (pe->flags & FLAG_PATEND))
			break;
		}

	if ((dirp = opendir(dir))==NULL)
		return;
	while((dp = readdir(dirp)) != NULL)
		{
		if (match(p,dp->d_name))
			{
			char *fullname,*ufullname;
			fullname = (char *)malloc(strlen(dir) + strlen(dp->d_name)+3);
			ufullname = (char *)malloc(strlen(udir) + strlen(dp->d_name)+3);
			if (fullname == NULL || ufullname == NULL)
				{
				fprintf(stderr,"Memory exhausted\n");
				exit(-1);
				}
			strcpy(fullname,dir);
			strcatpath(fullname,dp->d_name);
			strcpy(ufullname,udir);
			strcatpath(ufullname,dp->d_name);
			if (stat(fullname, &buf))	/* stat failed */
				{
				free(fullname);
				free(ufullname);
				continue;
				}
			if ((buf.st_mode&S_IFMT) == S_IFDIR)
				{			
				if (pe->flags & FLAG_PATEND)
					{
					addarg(nargcp,nargvp,nargsp,ufullname);
					}
				else if (pe->flags & FLAG_DIREND) /* go into sub-directory */
					searchdir(fullname,ufullname,pe+1,nargcp,nargvp,nargsp);
				}
			else
				{
				if (pe->flags & FLAG_PATEND)
					addarg(nargcp,nargvp,nargsp,ufullname);
				}
			free(fullname);
			free(ufullname);
			}
		}
	closedir(dirp);
	}

/* Given a pointer to the raw pattern, parse it into directory and match elements. */
/* Return FALSE if no pattern was found */
static boolean
makepattern(ip,op)
char *ip;	/* raw input pattern */
pattern *op;	/* space for processed output pattern */
	{
	char c;
	boolean somepat;	/* set if there is at least one pattern */
#define UNPROT 0	/* in unprotected text */
#define PROT1 1		/* in protected text - protection with ' just started */
#define PROT2 2		/* in protected text - protection with ' continuing */
#define DONE 5	
	int state;
#define NOTSET 0	/* not defining a set */
#define SETSTART 1	/* started a set */
#define INSET 2		/* in a set definition */
	int sstate;
	boolean invset;		/* set if set match is inverted */
	boolean rangefound;	/* set if a range character was found */
	boolean charinset;	/* set if there is at least one char in the set */

	/* Find the first part of a path */
	state = UNPROT;
	sstate = NOTSET;
	somepat = FALSE;
	for(op--;;ip++)	
		{
		c = *ip;		/* fetch a character */
		switch (c)
			{
			case '\'':					/* possible protection character */
				if (state == PROT1)
					{					/* ' after ' */
					state = UNPROT;
					goto literal_char;	/* use ' char as literal */
					}
				else if (state == PROT2)	/* ended protection with ' */
					state = UNPROT;
				else					/* started protection with '...' */
					state = PROT1;
				break;

			case '\\':					/* possible pattern delimeter */
			case '/':
			case '?':					/* possible single char wildcard */
			case '*':					/* possible zero or more wildcard */
			case '[':					/* possible start of matching set */
			case '!':					/* possible not in set char */
			case '^':
			case '-':					/* possible set range character */
			case ']':					/* possible end of matching set */
				/* but not if we are protecting special characters */
				if (state == PROT1 || state == PROT2)
					{					/* protected by '...' */
					state = PROT2;
					goto literal_char;	/* use literal char */
					}
				switch (c)
					{
					case '?':					/* possible single char wildcard */
						/* we have a ? wildcard */
						if (sstate != NOTSET)		/* but we are defining a set */
							goto literal_char;		/* so assume literal */
						op++;
						op->low = c;
						op->flags = FLAG_ANYONE | FLAG_SETEND;	/* use SETEND to count matches */
						somepat = TRUE;
						break;
		
					case '*':					/* possible zero or more wildcard */
						/* we have a * wildcard */
						if (sstate != NOTSET)		/* but we are defining a set */
							goto literal_char;		/* so assume literal */
						if (somepat && ((op->flags & FLAG_MASK) == FLAG_ANYANY)
						            && !(op->flags & FLAG_DIREND))
							break;			/* compress multple *'s */
						op++;
						op->low = c;
						op->flags = FLAG_ANYANY;
						somepat = TRUE;
						break;
		
					case '!':
					case '^':
						if (sstate == SETSTART)	/* first char after [ */
							{
							sstate = INSET;
							invset = TRUE;
							break;
							}
						else
							goto literal_char;		/* treat as ordinary char. */
		
					case '[':					/* possible start of matching set */
						if (sstate != NOTSET)		/* but we are defining a set */
							goto literal_char;		/* so assume literal */
						/* we have the start of a matching set */
						rangefound = FALSE;
						charinset = FALSE;
						invset = FALSE;
						sstate = SETSTART;			/* start of a set */
						break;
		
					case ']':					/* possible end of matching set */
						if (sstate == NOTSET)		/* but we are not defining a set */
							goto literal_char;		/* so assume literal */
						sstate = NOTSET;			/* end of a set */
						if (somepat && charinset)	/* if something in set was defined */
							op->flags |= FLAG_SETEND;
						charinset = FALSE;
						rangefound = FALSE;
						invset = FALSE;
						break;
		
					case '-':					/* possible range char of matching set */
						if (sstate == NOTSET)		/* but we are not defining a set */
							goto literal_char;		/* so assume literal */
						rangefound = TRUE;
						break;
		
					case '\\':				/* we have a pattern delimeter */
					case '/':
						if (somepat)
							{
							op->flags |= FLAG_SETEND;
							op->flags |= FLAG_DIREND;
							}
						state = UNPROT;
						sstate = NOTSET;
						break;
					}
				break;

			case '\000':
				if (somepat)
					op->flags |= FLAG_PATEND;
				return somepat;

			default:
				if (state == PROT1)	/* got past opening protection */
					state = PROT2;	
			literal_char:			/* we have a single char for exact match  or set def. */
				if (sstate == NOTSET)
					{ 		/* exact match character */
					op++;
					op->low = c;
					op->flags = FLAG_EXACT | FLAG_SETEND;	/* a single char is a set of 1 */
					somepat = TRUE;
					}
				else	/* we are doing a set */
					{
					if (sstate == SETSTART)	/* first char after [ */
						sstate = INSET;		/* past start of set now */
					if (charinset && rangefound)
						{
						if (c > op->low)	/* make sure low <= high */
							op->high = c;
						else
							{
							op->high = op->low;
							op->low = c;
							}
						op->flags |= FLAG_RANGE;
						rangefound = FALSE;
						break;
						}
					op++;		/* add another to the set */
					op->low = c;
					op->flags = FLAG_EXACT;
					somepat = TRUE;
					if (invset)
						op->flags |= FLAG_INVSET;	/* inverted set */
					charinset = TRUE;		/* at least one char in set now */
					}
				break;
			}
		}
	}

/* Given a pattern and a string, determine whether they match. */
static boolean
match(p,s)
pattern *p;	/* pattern is assumed to be non-empty */
char *s;
	{
	int prem;	/* pattern remaining */
	int srem;	/* string remaining */
	pattern *tp;
	srem = strlen(s);
	if (srem == 0)
		{
		return FALSE;	/* can't match null string */
		}
	for(tp = p,prem = 0;;tp++)
		{
		if (tp->flags & FLAG_SETEND)
			prem++;		/* count space needed for match */
		if((tp->flags & FLAG_DIREND) || (tp->flags & FLAG_PATEND))
			break;
		}
	/* Filter out strings starting with . unless specified exactly */
	if (*s == '.' && ((p->flags & FLAG_MASK) != FLAG_EXACT || p->low != '.'))
		return FALSE;
	for(;;)
		{
		switch (p->flags & FLAG_MASK)
			{
			case FLAG_EXACT:
			case FLAG_RANGE:
				if (p->flags & FLAG_INVSET)	/* match fails if any in set match */
					{
					for(;;p++)
						{
						if ((p->flags & FLAG_MASK) == FLAG_RANGE)
							{
							if (*s >= p->low && *s <= p->high)
								{
								return FALSE;
								}
							}
						else
							{
							if (*s == p->low)
								{
								return FALSE;
								}
							}
						if (p->flags & FLAG_SETEND)	/* run out of possibilities */
							break;
						}
					}
				else	/* match succeeds if any in set match */
					{
					for(;;p++)
						{
						if ((p->flags & FLAG_MASK) == FLAG_RANGE)
							{
							if (*s >= p->low && *s <= p->high)
								break;
							}
						else
							{
							if (*s == p->low)
								break;
							}
						if (p->flags & FLAG_SETEND)	/* run out of possibilities */
							{
							return FALSE;
							}
						}
					for(;;p++)	/* skip to end of set */
						{
						if (p->flags & FLAG_SETEND)
							break;
						}
					}
				prem--;
				break;

			case FLAG_ANYONE:
				prem--;
				break;			/* always succeeds */

			case FLAG_ANYANY:
				{
				int i;
				if (prem == 0)	/* if this is the last bit of the pattern */
					{
					return TRUE;	/* we match everything else */
					}
				/* Try all reasonable skips in the string */
				for (i = 0; i <= (srem - prem); i++)
					if (match(p+1,s+i))		/* recurse */
						{
						return TRUE;
						}
				return FALSE;
				}
			}
		s++;
		srem--;
		/* if pattern and string have finished together */
		if(((p->flags & FLAG_DIREND) || (p->flags & FLAG_PATEND)) && srem == 0)
			{
			return TRUE;
			}
		/* if pattern or string have finished before the other */
		if((p->flags & FLAG_DIREND) || (p->flags & FLAG_PATEND) || srem == 0)
			{
			return FALSE;	/* pattern and string didn't finish together */
			}
		p++;
		}
	}

/* remove '..' protection from a string */
static void
remove_prot(s)
char *s;
	{
	char *d;
	boolean lwq = FALSE;	/* set if last was a quote */
	for(d = s; *s != '\000'; s++)
		{
		if (*s == '\'')
			{
			if (lwq)		/* double '' */
				{
				*d++ = *s;
				lwq = FALSE;
				}
			else
				lwq = TRUE;
			}
		else
			{
			*d++ = *s;
			lwq = FALSE;
			}
		}
	*d = '\000';
	}

/* munge a dos file/directory specification up */
/* so we can work with it. */
/* All returned strings must be freed */

dos_name_stuff(spec,edp,udp,np,esp)
char *spec;		/* file/directory specification */
char **edp;		/* (return) effective directory (+ drive) */
char **udp;		/* (return) user specified directory (+ drive) */
char **np;		/* (return) name left after user directory is extracted */
char **esp;		/* (return) effective specification */
	{
	char *ed;
	char *ud;
	char *n;
	char *es;
	char *s;
	int i;
	if ((ed = (char *)malloc(strlen(spec)+3))==NULL)
		{
		fprintf(stderr,"Memory exhausted\n");
		exit(-1);
		}
	if ((ud = (char *)malloc(strlen(spec)+3))==NULL)
		{
		fprintf(stderr,"Memory exhausted\n");
		exit(-1);
		}
	if ((n = (char *)malloc(strlen(spec)+3))==NULL)
		{
		fprintf(stderr,"Memory exhausted\n");
		exit(-1);
		}
	if ((es = (char *)malloc(strlen(spec)+3))==NULL)
		{
		fprintf(stderr,"Memory exhausted\n");
		exit(-1);
		}
	/* first seperate the user drive/directory from the name */
	s = spec;
	if(   strlen(s) >= 2
	   && (   (s[0] >= 'A' && s[0] <= 'Z')
	       || (s[0] >= 'a' && s[0] <= 'z'))
	   && s[1] == ':')	/* we have a drive spec */
		s += 2;
	while(*s != '\000' && (*s == '.' || *s == '/' || *s == '\\'))
		s++;
	if(s > spec && s[-1] == '.' && s[0] !='\000')	/* spec is ???.name */
		s--;
	strcpy(n,s);
	strncpy(ud,spec,s-spec);	/* ud + n = spec */
	ud[s-spec] = '\000';

	/* effective directory */
	strcpy(ed,ud);				/* start with user dir */
	if(strlen(ed) >= 1)			/* if room for /, drive spec or ./ */
		{
		s = ed + strlen(ed)-1;	/* look at last char */
		if(strlen(ed) >= 2)			/* if room for drive spec or ./ */
			{
			if(s[-1] == '.' && (s[0] == '/' || s[0] == '\\'))
				s[0] = '\000';		/* remove / of trailing ./ */
			else if(s[0] == ':')	/* just a drive spec */
				{
				s[1] = '.';			/* add a . */
				s[2] = '\000';
				}
			}
		/* else leave it alone */
		}
	else						/* if no directory */
		strcpy(ed,".");			/* assume default */
	/* now do effective name */
	strcpy(es,ed);
	strcatpath(es,n);			/* make up path */

	*edp = ed;
	*udp = ud;
	*np = n;
	*esp = es;
	}


/* string cat a path together - ie. add '/' if it needs it */
/* and make sure a '/' is not added inapropriately */
strcatpath(s1,s2)
char *s1;
char *s2;
	{
	char *se;
	int len;
	len = strlen(s1);
	se = &s1[len-1];
	if(   strlen(s1)>0
	   && strlen(s2)>0
	   && se[0] != '/'
	   && se[0] != '\\'
	   && !(len==2 && ((s1[0] >= 'A' && s1[0] <= 'Z')
	                || (s1[0] >= 'a' && s1[0] <= 'z'))
	        && s1[1] == ':'))
		strcat(s1,"/");	/* don't double, nor make leading one / */
	strcat(s1,s2);
	}

#endif /* DO_ARG_EXP */

#endif	/* MSDOS */
