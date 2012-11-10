
/* make_arrays() - malloc space for arrays */

#include <stdio.h>

#ifdef ibmpc
extern char  far *malloc();
#else
extern void *malloc();
#endif
extern	int	nn;		/* number of nodes */
extern	int	ni;		/* number of inputs */
extern	int	no;		/* number of outputs */
extern	int	nt;		/* nn + ni + 1 */

struct	cf {
	int	con;	/* connection flag */
	int	fix;	/* fixed-weight flag */
	int	num;	/* group number */
	int	lim;	/* weight limits */
	float	min;	/* weight minimum */
	float	max;	/* weight maximum */
};

struct	nf {
	int	func;	/* activation function type */
	int	dela;	/* delay flag */
	int	targ;	/* target flag */
};

extern	struct	cf	**cinfo;	/* (nn x nt) connection info */
extern	struct	nf	*ninfo;		/* (nn) node activation function info */

extern	int	*outputs;	/* (no) indices of output nodes */
extern	int	*selects;	/* (nn+1) nodes selected for probe printout */
extern	int	*linput;	/* (ni) localist input array */

extern	float	*znew;		/* (nt) inputs and activations at time t+1 */
extern	float	*zold;		/* (nt) inputs and activations at time t */
extern	float	*zmem;		/* (nt) inputs and activations at time t */
extern	float	**wt;		/* (nn x nt) weight TO node i FROM node j*/ 
extern	float	**dwt;		/* (nn x nt) delta weight at time t */
extern	float	**winc;		/* (nn x nt) accumulated weight increment*/
extern	float	*target;	/* (no) output target values */
extern	float	*error_values; /* (nn) error = (output - target) values */
extern	float	***pnew;	/* (nn x nt x nn) p-variable at time t+1 */
extern	float	***pold;	/* (nn x nt x nn) p-variable at time t */

void free_arrays(){
    free(error_values);
    free(target);
    free(zold);
    free(zmem);
    free(znew);
    free(selects);
    free(outputs);
    free(linput);
    free(wt);
    free(dwt);
    free(winc);
    free(cinfo);
    free(ninfo);
}

make_arrays()
{

	int	i;
	int	j;

	struct	cf	*ci;
	struct	nf	*n;

	zold = (float *) malloc(nt * sizeof(float));
	if (zold == NULL){
		perror("zold malloc failed");
		exit(1);
	}
	zmem = (float *) malloc(nt * sizeof(float));
	if (zmem == NULL){
		perror("zmem malloc failed");
		exit(1);
	}
	znew = (float *) malloc(nt * sizeof(float));
	if (znew == NULL){
		perror("znew malloc failed");
		exit(1);
	}
	target = (float *) malloc(no * sizeof(float));
	if (target == NULL){
		perror("target malloc failed");
		exit(1);
	}
	error_values = (float *) malloc(nn * sizeof(float));
	if (error_values == NULL){
		perror("error malloc failed");
		exit(1);
	}
	selects = (int *) malloc(nt * sizeof(int));
	if (selects == NULL){
		perror("selects malloc failed");
		exit(1);
	}
	outputs = (int *) malloc(no * sizeof(int));
	if (outputs == NULL){
		perror("outputs malloc failed");
		exit(1);
	}
	linput = (int *) malloc(ni * sizeof(int));
	if (linput == NULL){
		perror("linput malloc failed");
		exit(1);
	}

	wt = (float **) malloc(nn * sizeof(float *));
	if (wt == NULL){
		printf("wt malloc failed--needed %d bytes for pointers", nn*sizeof(float *));
		exit(1);
	}
	for (i = 0; i < nn; i++){
		*(wt + i) = (float *) malloc(nt * sizeof(float));
		if (*(wt + i) == NULL){
			printf("wt malloc failed");
			printf("--got %d nodes, %d remaining", i, nn);
			exit(1);
		}
	}

	dwt = (float **) malloc(nn * sizeof(float *));
	if (dwt == NULL){
		perror("dwt malloc failed");
		exit(1);
	}
	for (i = 0; i < nn; i++){
		*(dwt + i) = (float *) malloc(nt * sizeof(float));
		if (*(dwt + i) == NULL){
			perror("dwt malloc failed");
			exit(1);
		}
	}

	winc = (float **) malloc(nn * sizeof(float *));
	if (winc == NULL){
		perror("winc malloc failed");
		exit(1);
	}
	for (i = 0; i < nn; i++){
		*(winc + i) = (float *) malloc(nt * sizeof(float));
		if (*(winc + i) == NULL){
			perror("winc malloc failed");
			exit(1);
		}
	}

	cinfo = (struct cf **) malloc(nn * sizeof(struct cf *));
	if (cinfo == NULL){
		perror("cinfo malloc failed");
		exit(1);
	}
	for (i = 0; i < nn; i++){
		*(cinfo + i) = (struct cf *) malloc(nt * sizeof(struct cf));
		if (*(cinfo + i) == NULL){
			perror("cinfo malloc failed");
			exit(1);
		}
	}

	ninfo = (struct nf *) malloc(nn * sizeof(struct nf));
	if (ninfo == NULL){
		perror("ninfo malloc failed");
		exit(1);
	}

	n = ninfo;
	for (i = 0; i < nn; i++, n++){
		n->func = 0;
		n->dela = 0;
		n->targ = 0;
		ci = *(cinfo + i);
		for (j = 0; j < nt; j++, ci++){
			ci->con = 0;
			ci->fix = 0;
			ci->num = 0;
			ci->lim = 0;
			ci->min = 0.;
			ci->max = 0.;
		}
	}

}

free_parrays(){
	free(pnew);
	free(pold);
}

make_parrays()
{

	int	i;
	int	j;

	pold = (float ***) malloc(nn * sizeof(float **));
	if (pold == NULL){
		perror("pold malloc failed");
		exit(1);
	}
	for (i = 0; i < nn; i++){
		*(pold + i) = (float **) malloc(nt * sizeof(float *));
		if (*(pold + i) == NULL){
			perror("pold malloc failed");
			exit(1);
		}
		for (j = 0; j < nt; j++){
			*(*(pold + i) + j) = (float *) malloc(nn * sizeof(float));
			if (*(*(pold + i) + j) == NULL){
				perror("pold malloc failed");
				exit(1);
			}
		}
	}

	pnew = (float ***) malloc(nn * sizeof(float **));
	if (pnew == NULL){
		perror("pnew malloc failed");
		exit(1);
	}
	for (i = 0; i < nn; i++){
		*(pnew + i) = (float **) malloc(nt * sizeof(float *));
		if (*(pnew + i) == NULL){
			perror("pnew malloc failed");
			exit(1);
		}
		for (j = 0; j < nt; j++){
			*(*(pnew + i) + j) = (float *) malloc(nn * sizeof(float));
			if (*(*(pnew + i) + j) == NULL){
				perror("pnew malloc failed");
				exit(1);
			}
		}
	}

}

