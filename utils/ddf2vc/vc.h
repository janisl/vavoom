extern char *progsdir;
extern char *scriptsdir;
extern FILE *cur_file;

void VC_WriteMobjs(void);
void VC_WriteWeapons(void);
void WriteSoundScript(void);

const char *SFX(sfx_t *s);
void PrintAttack(FILE *f, const attacktype_t *a, const char *Name);
char *GetAmmoName(int num);
void WriteInitialBenefits(FILE *f, benefit_t *list);

