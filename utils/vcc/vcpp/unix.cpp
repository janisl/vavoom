#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

void cpp_add_include(char *path)
{
	int i;

	for (i = NINCLUDE - 2; i >= 0; i--)
	{
		if (includelist[i].file == NULL)
		{
			includelist[i].always = 1;
			includelist[i].file = path;
			break;
		}
	}
	if (i < 0)
		error(FATAL, "Too many -I directives");
}

void cpp_add_define(int c, char *text)
{
	Tokenrow tr;

	setsource("<cmdarg>", -1, text);
	maketokenrow(3, &tr);
	gettokens(&tr, 1);
	doadefine(&tr, c);
	unsetsource();
}

void setup(char *srcf, char *dstf)
{
	int fd;
	char *fp, *dp;
	extern void setup_kwtab(void);

	setup_kwtab();
	dp = ".";
	fd = 0;

	if ((fp = strrchr(srcf, '/')) != NULL)
	{
		int len = fp - srcf;

		dp = (char *)newstring((uchar *) srcf, len + 1, 0);
		dp[len] = '\0';
	}
	fp = (char *)newstring((uchar *) srcf, strlen(srcf), 0);
	if ((fd = open(fp, 0)) <= 0)
		error(FATAL, "Can't open input file %s", fp);

	output_file = creat(dstf, 0666);
	if (output_file < 0)
		error(FATAL, "Can't open output file %s", dstf);

	includelist[NINCLUDE - 1].always = 0;
	includelist[NINCLUDE - 1].file = dp;
	setsource(fp, fd, NULL);
}

/* memmove is defined here because some vendors don't provide it at
   all and others do a terrible job (like calling malloc) */
void *memmove(void *dp, const void *sp, size_t n)
{
	unsigned char *cdp, *csp;

	if (n <= 0)
		return 0;
	cdp = (unsigned char *)dp;
	csp = (unsigned char *)sp;
	if (cdp < csp)
	{
		do
		{
			*cdp++ = *csp++;
		}
		while (--n);
	}
	else
	{
		cdp += n;
		csp += n;
		do
		{
			*--cdp = *--csp;
		}
		while (--n);
	}
	return 0;
}
