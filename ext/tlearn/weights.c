#include <stdio.h>

extern	int	nn;	/* number of nodes */
extern	int	ni;	/* number of inputs */
extern	int	nt;	/* nn + ni + 1 */

extern	long	tsweeps;	/* total sweeps */
extern	long	sweep;		/* current sweep */

extern	float	**wt;		/* (nn x nt): weights */
extern	float	**dwt;		/* (nn x nt) delta weight at time t */
extern	float	**winc;		/* (nn x nt) accumulated weight increment*/

extern	char	loadfile[];	/* .wts file to start with */
extern	char	root[];		/* root file name */


save_wts()
{

	FILE	*fp;
	FILE	*fopen();

	register	int	i;
	register	int	j;

	float	*w;
	float	**wp;

	char	file[128];

#ifdef ibmpc
	/* 
	 * if running under DOS, probably can't have filenames
	 * with more than 8 chars total, or multiple "."s, so
	 * "fileroot.nnnnn.wts" becomes "fileroot_nnnnn.wts"
	 */
	sprintf(file, "%s_%ld.wts", root, sweep);
#else
	sprintf(file, "%s.%ld.wts", root, sweep);
#endif
	if ((fp=fopen(file, "w+")) == NULL) {
		perror("Can't open .wts file\n");
		exit(1);
	}
	fprintf(fp, "NETWORK CONFIGURED BY TLEARN\n");
	fprintf(fp, "# weights after %ld sweeps\n", sweep);
	fprintf(fp, "# WEIGHTS\n");

	/* to each node */
	wp = wt;
	for (i = 0; i < nn; i++, wp++){
		fprintf(fp, "# TO NODE %d\n",i+1);
		w = *wp;
		/* from each bias, input, and node */
		for (j = 0; j < nt; j++,w++){
			fprintf(fp,"%f\n",*w);
		}
	}

	fflush(fp);
	fclose(fp);

	return;
}

load_wts()
{

	FILE	*fp;
	FILE	*fopen();

	register	int	i;
	register	int	j;

	register	float	*w;
	register	float	*wi;
	register	float	*dw;
	register	float	**wp;

	int	tmp;

	char	mode[10];

	if ((fp=fopen(loadfile, "r")) == NULL) {
		perror(loadfile);
		exit(1);
	}
	fscanf(fp, "NETWORK CONFIGURED BY %s\n", mode);
	if (strcmp(mode, "TLEARN") != 0) {
		printf("Saved weights not for tlearn-configured network\n");
		exit(1);
	}
	fscanf(fp, "# weights after %ld sweeps\n", &tsweeps);
	fscanf(fp, "# WEIGHTS\n");

	/* to each of nn nodes */
	wp = wt;
	for (i = 0; i < nn; i++, wp++){
		fscanf(fp, "# TO NODE %d\n",&tmp);
		w = *wp;
		dw = *(dwt+i);
		wi = *(winc+i);
		/* from each bias, input, and node */
		for (j = 0; j < nt; j++, w++){
			fscanf(fp,"%f\n",w);
			*dw = 0.;
			*wi = 0.;
		}
	}

	return;

}


