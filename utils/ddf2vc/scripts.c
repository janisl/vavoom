#include "i_defs.h"

#include "ddf_main.h"
#include "vc.h"

extern int num_disabled_sfx;

void WriteSoundScript(void)
{
	FILE *f;
	char tmp[256];
	int i;

	sprintf(tmp, "%s/sndinfo.txt", scriptsdir);
	f = fopen(tmp, "w");
	if (!f)
	{
		return;
	}
	for (i = num_disabled_sfx; i < numsfx; i++)
	{
		if (!S_sfx[i]->ddf.name[0])
			continue;
		fprintf(f, "\"%s\" %s\n", S_sfx[i]->ddf.name, S_sfx[i]->lump_name);
	}
	fclose(f);
}

