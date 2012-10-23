#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef ibmpc
#include <fcntl.h>
#include <sys/types.h>
#else
#ifdef	EXP_TABLE
#include <sys/file.h>
#include <sys/types.h>
#endif	/* EXP_TABLE */
#endif
#ifdef	EXP_TABLE
#include <sys/stat.h>
#endif	/* EXP_TABLE */

#ifdef ibmpc
extern char  far *malloc();
#define random() rand()
#define srandom(x) srand(x)
#else
extern void  *malloc();
#ifdef THINK_C
#define random() rand()
#define srandom(x) srand(x)
#endif	/* THINK_C  */
#endif

float	*exp_array;	/* table look-up for exp function */
float	exp_mult;
long	exp_add;

extern	int	nn;		/* number of nodes */
extern	int	ni;		/* number of inputs */
extern	int	no;		/* number of outputs */
extern	int	nt;		/* nn + ni + 1 */

extern	int	ce;		/* cross-entropy flag */

extern	int	*outputs;	/* (no) indices of output nodes */
extern	int	*selects;	/* (nn+1) nodes selected for probe printout */

extern	char	root[128];	/* root filename for .cf, .data, .teach, etc.*/

extern	long	sweep;		/* current sweep */
extern	long	rms_report;	/* report error every report sweeps */

extern	float	criterion;	/* exit program when rms error < criterion */

extern int start = 1;


float rans(w)
	float	w;
{
/*	extern	long random(); */

	static	float max = RAND_MAX;
	
	return (((float)random() / max) * 2*w) - w;
}

exp_init()
{
#ifdef	EXP_TABLE
	struct	stat statb;

	int	fd;

	fd = open(EXP_TABLE, O_RDONLY, 0);
	if (fd < 0) {
		perror("exp_table");
		exit(1);
	}
	fstat(fd, &statb);
	exp_add = (statb.st_size / sizeof(float)) / 2;
	exp_mult = (float) (exp_add / 16);
	exp_array = (float *) malloc(statb.st_size);
	if (read(fd, exp_array, statb.st_size) != statb.st_size) {
		perror("read exp array");
		exit(1);
	}
#endif	EXP_TABLE
}

print_nodes(aold)
	float	*aold;
{
	int	i;

	for (i = 1; i <= nn; i++){
		if (selects[i])
			fprintf(stdout,"%7.3f\t",aold[ni+i]);
	}
	fprintf(stdout,"\n");

}

print_output(aold)
	float	*aold;
{
	int	i;

	for (i = 0; i < no; i++){
		fprintf(stdout,"%7.3f\t",aold[ni+outputs[i]]);
	}
	fprintf(stdout,"\n");

}

print_error(e)
	float	*e;
{
	static	FILE	*fp;

	FILE	*fopen();
	char	file[128];

	if (start){
		start = 0;
		sprintf(file, "%s.err", root);
		fp = fopen(file, "w");
		if (fp == NULL) {
			perror("ERROR: Can't open .err file");
			exit(1);
		}
	}

	if (ce != 2) {
		/* report rms error */
		*e = sqrt(*e / rms_report);
	} else if (ce == 2) {
		/* report cross-entropy */
		*e = *e / rms_report;
	}
	fprintf(fp,"%g\n",*e);
	fflush(fp);
	if (ce == 0) {
		if (*e < criterion){
			sweep += 1;
			save_wts();
				exit(0);
		}
	}
	*e = 0.;
}

reset_network(aold,anew,apold,apnew)
	float	*aold;
	float	*anew;
	float	***apold;
	float	***apnew;
{
	register	int	i, j, k;

	register	float	*pn;
	register	float	*po;
	register	float	**pnp;
	register	float	**pop;
	register	float	***pnpp;
	register	float	***popp;

	register	float	*zn;
	register	float	*zo;

	zn = anew + 1;
	zo = aold + 1;
	for (i = 1; i < nt; i++, zn++, zo++)
		*zn = *zo = 0.;

	popp = apold;
	pnpp = apnew;
	for (i = 0; i < nn; i++, popp++, pnpp++){
		pop = *popp;
		pnp = *pnpp;
		for (j = 0; j < nt; j++, pop++, pnp++){
			po = *pop;
			pn = *pnp;
			for (k = 0; k < nn; k++, po++, pn++){
				*po = 0.;
				*pn = 0.;
			}
		}
	}

	return;
}

reset_bp_net(aold,anew)
	float	*aold;
	float	*anew;
{
	register	int	i;

	register	float	*zn;
	register	float	*zo;

	zn = anew + 1;
	zo = aold + 1;
	for (i = 1; i < nt; i++, zn++, zo++)
		*zn = *zo = 0.;

	return;
}

