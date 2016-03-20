#include <stdio.h>

#ifdef ibmpc
#define random() rand()
#define srandom(x) srand(x)
#endif
#ifdef THINK_C
#define random() rand()
#define srandom(x) srand(x)
#endif	/* THINK_C */

double	atof();

#ifdef ibmpc
extern char  far *malloc();
#else
extern void  *malloc();
#endif
extern	int	nn;		/* number of nodes */
extern	int	ni;		/* number of inputs */
extern	int	no;		/* number of outputs */
extern	int	nt;		/* nn + ni + 1 */
extern	int	np;		/* ni + 1 */

extern	struct	cf {
	int	con;	/* connection flag */
	int	fix;	/* fixed-weight flag */
	int	num;	/* group number */
	int	lim;	/* weight limits */
	float	min;	/* weight minimum */
	float	max;	/* weight maximum */
};

extern	struct	nf {
	int	func;	/* activation function type */
	int	dela;	/* delay flag */
	int	targ;	/* target flag */
};

extern	struct	cf	**cinfo;	/* (nn x nt) connection info */

extern	int	ngroups;	/* number of groups */

extern	char	root[128];	/* root filename for .data, .teach, etc. files */

extern	float	rate;		/* learning rate */
extern	float	momentum;	/* momentum */

extern	int	randomly;	/* flag for presenting inputs in random order */
extern	int	localist;	/* flag for localist inputs */
extern	int	limits;		/* flag for limited weights */

extern	float *otarget = 0;	/* (no) back-up copy of target values */


long	dc = 0;
int	*ldata = 0;

float	*data = 0;

update_inputs(aold,tick,flag,maxtime,local)
	float	*aold;
	int	tick;
	int	flag;
	long	*maxtime;
	int	**local;
{
/*	extern	long	random(); */

	register	int	i;

	int	j;

	long	ii;

	static	long	dn;
	static	long	ds;

	int	*idata;
	int	*id;
	int	*lld;

	static	FILE	*fp;

	char	buf[512];
	char	file[512];

	static	float	*dm;
	static	int	*ld;

	register	float	*d;
	register	float	*zo;


	if ((data == 0) && (ldata == 0)){
		/* get .data file */
		sprintf(file, "%s.data", root);
		fp = fopen(file, "r");
		if (fp == NULL) {
		     perror("ERROR: Empty data file");
		     exit(1);
		}
		/* determine format of .data file */
		fscanf(fp,"%s",buf);
		if (strcmp(buf, "localist") == 0){
			localist = 1;
		}
		else if (strcmp(buf, "distributed") != 0){
			perror("ERROR: .data file must be localist or distributed\n");
			exit(1);
		}
		/* determine size of .data file */
		if (fscanf(fp,"%ld",maxtime) != 1){
			perror("ERROR: how many items in .data file?");
			exit(1);
		}
		/* malloc space for data */
		if (localist){
			dn = *maxtime;
			ds = *maxtime * ni;
			ldata = (int *) malloc(ds * sizeof(int));
			if (ldata == NULL){
				perror("ldata malloc failed");
				exit(1);
			}
			idata = (int *) malloc((ni+1) * sizeof(int));
			if (idata == NULL){
				perror("idata malloc failed");
				exit(1);
			}
			/* read data */
			ld = ldata;
			for (ii = 0; ii < dn; ii++){
				fscanf(fp,"%s",buf);
				get_nums(buf,ni,0,idata);
				id = idata + 1;
				lld = ld;
				for (j = 1; j <= ni; j++, id++){
					if (*id)
						*lld++ = j;
				}
				*lld = 0;
				ld += ni;
			}
		}
		else {
			dn = *maxtime;
			ds = *maxtime * ni;
			data = (float *) malloc(ds * sizeof(float));
			if (data == NULL){
				perror("data malloc failed");
				exit(1);
			}
			/* read data */
			d = dm = data;
			for (ii = 0; ii < ds; ii++, d++){
				fscanf(fp,"%s",buf);
				*d = atof(buf);
				if ((*d == 0.) && (buf[0] != '0') && (buf[1] != '0')){
					fprintf(stderr,"error reading .data file on or around line %ld of input\n",ii+1);
					exit(1);
				}
			}
		}

		fclose(fp);
	}


	/* update input (only at major time increments) */
	if (tick == 0){
		/* read next ni inputs from .data file */
		if (localist){
			if (randomly){
				dc = (random() >> 8) % dn;
				if (dc < 0)
					dc = -dc;
				*local = (int *) (ldata + dc * ni);
			}
			else {
				*local = (int *) (ldata + dc * ni);
				if (++dc >= dn)
					dc = 0; 
			}
		}
		else {
			if (randomly){
				dc = (random() >> 8) % dn;
				if (dc < 0)
					dc = -dc;
				d = (float *) (data + dc * ni);
				zo = aold + 1;
				for (i = 0; i < ni; i++, zo++, d++){
					*zo = *d;
				}
			}
			else {
				d = dm;
				zo = aold + 1;
				for (i = 0; i < ni; i++, zo++, d++, dc++){
					if (dc >= ds){
						dc = 0;
						d = data;
					}
					*zo = *d;
				}
				dm = d;
			}
		}
	}
	else {
		/* turn off input during extra ticks with -I */
		if (flag){
			zo = aold + 1;
			for (i = 0; i < ni; i++, zo++)
				*zo = 0.;
		}
	}
	
}

void update_targets(atarget,time,tick,flag,maxtime)
	float	*atarget;
	long	time;
	int	tick;
	int	flag;
	long	*maxtime;
{
	long	i;

	int	k;

	register	int	j;

	register	float	*ta;
	register	float	*t;
	register	long	*n;
	register	float	*to;

	static	long	*ntimes;
	static	float	*teach;

	static	int	local = 0;	/* flag for localist output */

	static	long	*nm;
	static	float	*tm;
	static	long	nc = 0;

	static	long	len;
	static	long	ts;

	static	long	next;		/* next time tag in .teach file */

	static	FILE	*fp;

	char	buf[128];

	if (otarget == 0){
		/* get .teach file */
		sprintf(buf, "%s.teach", root);
		fp = fopen(buf, "r");
		if (fp == NULL) {
		     perror("ERROR: Empty target file");
		     exit(1);
		}
		/* malloc space for back-up copy of targets */
		otarget = (float *) malloc(no * sizeof(float));
		if (otarget == NULL){
			perror("otarget malloc failed");
			exit(1);
		}
		/* determine format of .teach file */
		fscanf(fp,"%s",buf);
		if (strcmp(buf, "localist") == 0){
			local = 1;
		}
		else if (strcmp(buf, "distributed") != 0){
			perror("ERROR: .teach file must be localist or distributed\n");
			exit(1);
		}
		/* determine size of teach array */
		if (fscanf(fp,"%ld",&len) != 1){
			perror("ERROR: how many items in .teach file?");
			exit(1);
		}
		/* malloc space for teach and ntimes buffers */
		ts = len * no;
		teach = (float *) malloc(ts * sizeof(float));
		if (teach == NULL){
			perror("teach malloc failed");
			exit(1);
		}
		ntimes = (long *) malloc(len * sizeof(long));
		if (ntimes == NULL){
			perror("ntimes malloc failed");
			exit(1);
		}
		/* read teach info */
		t = tm = teach;
		n = nm = ntimes;
		for (i = 0; i < len; i++, n++){
			fscanf(fp,"%ld",n);
			if (local){
				fscanf(fp,"%s",buf);
				k = atoi(buf) - 1;
				if (k < 0){
					fprintf(stderr,"error reading .teach file on or around line %ld of input\n",i+1);
					exit(1);
				}
				for (j = 0; j < no; j++, t++){
					if (j == k)
						*t = 1.;
					else
						*t = 0.;
				}
			}
			else {
				for (j = 0; j < no; j++, t++){
					fscanf(fp,"%s",buf);
					/* asterick is don't care sign */
					if (buf[0] == '*')
						*t = -9999.0;
					else {
						*t = atof(buf);
						if ((*t == 0.) && (buf[0] != '0') && (buf[1] != '0')){
							fprintf(stderr,"error reading .teach file on or around line %ld of input\n",i+1);
							exit(1);
						}
					}
				}
			}
		}
	}

	/* check for new target values (only at major time increments) */
	if (tick == 0){
		t = tm;
		n = nm;
		/* restore previous values if destroyed by -T */
		if (flag){
			ta = atarget;
			to = otarget;
			for (j = 0; j < no; j++, ta++, to++)
				*ta = *to;
		}

		/* if inputs are selected randomly, time-tags are
		   assumed to run sequentially, and targets are
		   selected to match input */

		if (randomly){
			if (dc >= len){
				perror("ERROR: a target line is required for every input line with -R");
				exit(1);
			}
			ta = atarget;
			t = (float *) (teach + no * dc);
			for (j = 0; j < no; j++, ta++, t++){
				*ta = *t;
			}
			return;
		}

		/* rewind whenever .data begins again at time 0 */
		if (time == 0){
			nc = 0;
			t = teach;
			n = ntimes;
			next = *n;
			ta = atarget;
			for (j = 0; j < no; j++, ta++)
				*ta = -9999.0;
		}
		/* get new target values when time matches next */
		if (time >= next){
			/* read next no targets */
			ta = atarget;
			for (j = 0; j < no; j++, t++, ta++){
				*ta = *t;
			}
			/* final target persists till end of input */
			n++;
			if (++nc >= len)
				next = *maxtime;
			else
				next = *n;
		}
		tm = t;
		nm = n;
		/* remember target values if -T will destroy them */
		if (flag){
			ta = atarget;
			to = otarget;
			for (j = 0; j < no; j++, ta++, to++)
				*to = *ta;
		}
	}
	else {
		/* turn off target during extra ticks with -T */
		if (flag){
			ta = atarget;
			for (j = 0; j < no; j++, ta++)
				*ta = -9999.0;
		}
	}

}

void update_reset(time,tick,flag,maxtime,now)
	long	time;
	int	tick;
	int	flag;
	long	*maxtime;
	int	*now;
{
	long	i;

	static	int	start = 1;	/* flag for initialization */
	static	long	next;		/* next time tag in .teach file */

	static	long	*rtimes;

	static	long	*nm;
	static	long	nc = 0;

	static	long	l;

	static	FILE	*fp;

	char	buf[128];

	*now = 0;

	if (flag == 0)
		return;

	if (start){
		start = 0;
		/* get .reset file */
		sprintf(buf, "%s.reset", root);
		fp = fopen(buf, "r");
		if (fp == NULL) {
		     perror("ERROR: Empty reset file");
		     exit(1);
		}
		/* determine size of .reset file */
		if (fscanf(fp,"%ld",&l) != 1){
			perror("error reading .reset file");
			exit(1);
		}
		/* malloc space for rtimes buffer */
		rtimes = (long *) malloc(l * sizeof(long));
		if (rtimes == NULL){
			perror("rtimes malloc failed");
			exit(1);
		}
		/* read reset info */
		nm = rtimes;
		for (i = 0; i < l; i++, nm++)
			fscanf(fp,"%ld",nm);
		nm = rtimes;
	}

	/* check for new resets (only at major time increments) */
	if (tick == 0){
		/* rewind whenever .data begins again at time 0 */
		if (time == 0){
			nc = 0;
			nm = rtimes;
			next = *nm;
		}
		if (time >= next){
			*now = 1;
			nm++;
			if (++nc >= l)
				next = *maxtime;
			else
				next = *nm;
		}
	}

}

void update_weights(awt,adwt,awinc)
	float	**awt;
	float	**adwt;
	float	**awinc;
{
	register	int	i;
	register	int	j;

	register	struct	cf	*ci;

	register	float	*w;
	register	float	*dw;
	register	float	*wi;
	register	float	**wip;
	register	float	**wp;
	register	float	**dwp;

	register	struct	cf	**cp;

	register	int	k;
	register	int	n;
	register	float	*sum;

	float	asum;

	/* update weights if they are not fixed */
	sum = &asum;
	cp = cinfo;
	wp = awt;
	dwp = adwt;
	wip = awinc;
	for (i = 0; i < nn; i++, cp++, wp++, dwp++, wip++){
		ci = *cp;
		w = *wp;
		dw = *dwp;
		wi = *wip;
		for (j = 0; j < nt; j++, dw++, wi++, w++, ci++){
			if ((ci->con) && !(ci->fix)){
				*wi = rate * *dw + momentum * *wi;
				*w += *wi;
				*dw = 0.;
			}
		}
	}
	/* look for weights in the same group and average them together */
	for (k = 1; k <= ngroups; k++){
		*sum = 0.;
		n = 0;
		cp = cinfo;
		wp = awt;
		/* calculate average */
		for (i = 0; i < nn; i++, cp++, wp++){
			ci = *cp;
			w = *wp;
			for (j = 0; j < nt; j++, w++, ci++){
				if (ci->num == k){
					*sum += *w;
					n++;
				}
			}
		}
		if (n > 0)
			*sum /= n;
		/* replace weight with average */
		cp = cinfo;
		wp = awt;
		for (i = 0; i < nn; i++, cp++, wp++){
			ci = *cp;
			w = *wp;
			for (j = 0; j < nt; j++, w++, ci++){
				if (ci->num == k)
					*w = *sum;
			}
		}
	}
	/* look for limited weights and enforce limits */
	if (limits == 0)
		return;
	cp = cinfo;
	wp = awt;
	for (i = 0; i < nn; i++, cp++, wp++){
		ci = *cp;
		w = *wp;
		for (j = 0; j < nt; j++, w++, ci++){
			if (ci->lim){
				if (*w < ci->min)
					*w = ci->min;
				if (*w > ci->max)
					*w = ci->max;
			}
		}
	}

	return;
}
