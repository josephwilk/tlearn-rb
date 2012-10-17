
/* tlearn.c - simulator for arbitrary networks with time-ordered input */

/*------------------------------------------------------------------------

This program simulates learning in a neural network using either
the classical back-propagation learning algorithm or a slightly
modified form derived in Williams and Zipser, "A Learning Algo-
rithm for Continually Running Fully Recurrent Networks."  The
input is a sequence of vectors of (ascii) floating point numbers
contained in a ".data" file.  The target outputs are a set of
time-stamped vectors of (ascii) floating point numbers (including
optional "don't care" values) in a ".teach" file.  The network
configuration is defined in a ".cf" file documented in tlearn.man.

------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <signal.h>
#ifdef ibmpc
#include "strings.h"
#include <fcntl.h>
#else
#ifndef	THINK_C
#include <strings.h>
#include <sys/file.h>
#include <stdlib.h>
#else	/* THINK_C */
#include <console.h>
#include <time.h>
#include <stdlib.h>
#endif	/* THINK_C */
#endif
#ifdef	notdef
#include <sys/types.h>
#include <sys/stat.h>
#endif	/* notdef */

#ifdef ibmpc
#define random(x) rand(x)
#define srandom(x) srand(x)
#endif
#ifdef THINK_C
#define random(x) rand(x)
#define srandom(x) srand(x)
#endif	/* THINK_C */

int	nn;		/* number of nodes */
int	ni;		/* number of inputs */
int	no;		/* number of outputs */
int	nt;		/* nn + ni + 1 */
int	np;		/* ni + 1 */

struct	cf {
	int	con;	/* connection flag */
	int	fix;	/* fixed-weight flag */
	int	num;	/* group number */
	int	lim;	/* weight-limits flag */
	float	min;	/* weight minimum */
	float	max;	/* weight maximum */
};

struct	nf {
	int	func;	/* activation function type */
	int	dela;	/* delay flag */
	int	targ;	/* target flag */
};

struct	cf	**cinfo;	/* (nn x nt) connection info */
struct	nf	*ninfo;		/* (nn) node activation function info */

int	*outputs;	/* (no) indices of output nodes */
int	*selects;	/* (nn+1) nodes selected for probe printout */
int	*linput;	/* (ni) localist input array */

float	*znew;		/* (nt) inputs and activations at time t+1 */
float	*zold;		/* (nt) inputs and activations at time t */
float	*zmem;		/* (nt) inputs and activations at time t */
float	**wt;		/* (nn x nt) weight TO node i FROM node j*/ 
float	**dwt;		/* (nn x nt) delta weight at time t */
float	**winc;		/* (nn x nt) accumulated weight increment*/
float	*target;	/* (no) output target values */
float	*error;		/* (nn) error = (output - target) values */
float	***pnew;	/* (nn x nt x nn) p-variable at time t+1 */
float	***pold;	/* (nn x nt x nn) p-variable at time t */

float	rate = .1;	/* learning rate */
float	momentum = 0.;	/* momentum */
float	weight_limit = 1.;	/* bound for random weight init */
float	criterion = 0.;	/* exit program when rms error is less than this */
float	init_bias = 0.;	/* possible offset for initial output biases */

long	sweep = 0;	/* current sweep */
long	tsweeps = 0;	/* total sweeps to date */
long	rms_report = 0;	/* output rms error every "report" sweeps */

int	ngroups = 0;	/* number of groups */

int	backprop = 1;	/* flag for standard back propagation (the default) */
int	teacher = 0;	/* flag for feeding back targets */
int	localist = 0;	/* flag for speed-up with localist inputs */
int	randomly = 0;	/* flag for presenting inputs in random order */
int	limits = 0;	/* flag for limited weights */
int	ce = 0;		/* flag for cross_entropy */
#ifdef GRAPHICS
int	dsp_type = 0;	/* flag for graphics display */
int	dsp_freq = 0;	/* frequency of graphics display */
int	dsp_delay = 0;	/* delay of graphics display */
int	dsp_print = 0;	/* frequency of graphics hardcopy */
#endif GRAPHICS

char	root[128];	/* root filename for .cf, .data, .teach, etc.*/
char	loadfile[128];	/* filename for weightfile to be read in */

FILE	*cfp;		/* file pointer for .cf file */

void intr();

extern int load_wts();
extern int save_wts();
extern int act_nds();


main(argc,argv)
	int	argc;
	char  **argv;
{

	FILE	*fopen();
	FILE	*fpid;
	extern	char *optarg;
	extern	float rans();
	extern	time_t time();


	long	nsweeps = 0;	/* number of sweeps to run for */
	long	ttime = 0;	/* number of sweeps since time = 0 */
	long	utime = 0;	/* number of sweeps since last update_weights */
	long	tmax = 0;	/* maximum number of sweeps (given in .data) */
	long	umax = 0;	/* update weights every umax sweeps */
	long	rtime = 0;	/* number of sweeps since last rms_report */
	long	check = 0;	/* output weights every "check" sweeps */
	long	ctime = 0;	/* number of sweeps since last check */

	int	c;
	int	i;
	int	j;
	int	k;
	int	nticks = 1;	/* number of internal clock ticks per input */
	int	ticks = 0;	/* counter for ticks */
	int	learning = 1;	/* flag for learning */
	int	reset = 0;	/* flag for resetting net */
	int	verify = 0;	/* flag for printing output values */
	int	probe = 0;	/* flag for printing selected node values */
	int	command = 1;	/* flag for writing to .cmd file */
	int	loadflag = 0;	/* flag for loading initial weights from file */
	int	iflag = 0;	/* flag for -I */
	int	tflag = 0;	/* flag for -T */
	int	rflag = 0;	/* flag for -x */
	int	seed = 0;	/* seed for random() */

	float	err = 0.;	/* cumulative ss error */
	float	ce_err = 0.;	/* cumulate cross_entropy error */

	float	*w;
	float	*wi;
	float	*dw;
	float	*pn;
	float	*po;

	struct	cf	*ci;

	char	cmdfile[128];	/* filename for logging runs of program */
	char	cfile[128];	/* filename for .cf file */

	FILE	*cmdfp;

#ifdef	THINK_C
	argc = ccommand(&argv);
#endif	/* THINK_C */

	signal(SIGINT, intr);
#ifndef ibmpc
#ifndef	THINK_C
	signal(SIGHUP, intr);
	signal(SIGQUIT, intr);
	signal(SIGKILL, intr);
#endif	/* THINK_C */
#endif

#ifndef ibmpc
	exp_init();
#endif

	root[0] = 0;

	while ((c = getopt(argc, argv, "f:hil:m:n:r:s:tC:E:ILM:PpRS:TU:VvXB:H:D:")) != EOF) {
		switch (c) {
			case 'C':
				check = (long) atol(optarg);
				ctime = check;
				break;
			case 'f':
				strcpy(root,optarg);
				break;
			case 'i':
				command = 0;
				break;
			case 'l':
				loadflag = 1;
				strcpy(loadfile,optarg);
				break;
			case 'm':
				momentum = (float) atof(optarg);
				break;
			case 'n':
				nticks = (int) atoi(optarg);
				break;
			case 'P':
				learning = 0;
				/* drop through deliberately */
			case 'p':
				probe = 1;
				break;
			case 'r':
				rate = (double) atof(optarg);
				break;
			case 's':
				nsweeps = (long) atol(optarg);
				break;
			case 't':
				teacher = 1;
				break;
			case 'L':
				backprop = 0;
				break;
			case 'V':
				learning = 0;
				/* drop through deliberately */
			case 'v':
				verify = 1;
				break;
			case 'X':
				rflag = 1;
				break;
			case 'E':
				rms_report = (long) atol(optarg);
				break;
			case 'I':
				iflag = 1;
				break;
			case 'M':
				criterion = (float) atof(optarg);
				break;
			case 'R':
				randomly = 1;
				break;
			case 'S':
				seed = atoi(optarg);
				break;
			case 'T':
				tflag = 1;
				break;
			case 'U':
				umax = atol(optarg);
				break;
			case 'B':
				init_bias = atof(optarg);
				break;
#ifdef GRAPHICS
			/*
			 * graphics display; dsp_type:
			 *	0 = no display (default)
			 *	1 = weights only
			 *	2 = activations only
			 *	3 = weights & activations
			 */
			case 'D':
				switch (optarg[0]) {
					case 'f':
						optarg++;
						dsp_freq = atol(optarg);
						break;
					case 't':
						optarg++;
						dsp_type = atoi(optarg);
						break;
					case 'd':
						dsp_delay = 1;
						break;
					case 'p':
						optarg++;
						dsp_print = atol(optarg);
						break;
				}
				break;
#endif GRAPHICS
			/*
			 * if == 1, use cross-entropy as error;
			 * if == 2, also collect cross-entropy stats.
			 */
			case 'H':
				ce = atoi(optarg);
				break;
			case '?':
			case 'h':
			default:
				usage();
				exit(2);
			break;
		}
	}
	if (nsweeps == 0){
		perror("ERROR: No -s specified");
		exit(1);
	}

	/* open files */

	if (root[0] == 0){
		perror("ERROR: No fileroot specified");
		exit(1);
	}

	if (command){
		sprintf(cmdfile, "%s.cmd", root);
		cmdfp = fopen(cmdfile, "a");
		if (cmdfp == NULL) {
			perror("ERROR: Can't open .cmd file");
			exit(1);
		}
		for (i = 1; i < argc; i++)
			fprintf(cmdfp,"%s ",argv[i]);
		fprintf(cmdfp,"\n");
		fflush(cmdfp);
	}

#ifndef	THINK_C
	sprintf(cmdfile, "%s.pid", root);
	fpid = fopen(cmdfile, "w");
	fprintf(fpid, "%d\n",  getpid());
	fclose(fpid);
#endif	/* THINK_C */

	sprintf(cfile, "%s.cf", root);
	cfp = fopen(cfile, "r");
	if (cfp == NULL) {
		perror("ERROR: Can't open .cf file");
		exit(1);
	}

	get_nodes();
	make_arrays();
	get_outputs();
	get_connections();
	get_special();
#ifdef GRAPHICS
	/*
	 * graphics must be done after other files are opened
	 */
	if (dsp_type != 0)
		init_dsp(root);
#endif GRAPHICS
	if (!seed)
		seed = time((time_t *) NULL);
	srandom(seed);

	if (loadflag)
		load_wts();
	else {
		for (i = 0; i < nn; i++){
			w = *(wt + i);
			dw = *(dwt+ i);
			wi = *(winc+ i);
			ci = *(cinfo+ i);
			for (j = 0; j < nt; j++, ci++, w++, wi++, dw++){
				if (ci->con) 
					*w = rans(weight_limit);
				else
					*w = 0.;
				*wi = 0.;
				*dw = 0.;
			}
		}
		/*
		 * If init_bias, then we want to set initial biases
		 * to (*only*) output units to a random negative number.
		 * We index into the **wt to find the section of receiver
		 * weights for each output node.  The first weight in each
		 * section is for unit 0 (bias), so no further indexing needed.
		 */
		for (i = 0; i < no; i++){
			w = *(wt + outputs[i] - 1);
			ci = *(cinfo + outputs[i] - 1); 
			if (ci->con) 
				*w = init_bias + rans(.1);
			else
				*w = 0.;
		}
	}
	zold[0] = znew[0] = 1.;
	for (i = 1; i < nt; i++)
		zold[i] = znew[i] = 0.;
	if (backprop == 0){
		make_parrays();
		for (i = 0; i < nn; i++){
			for (j = 0; j < nt; j++){
				po = *(*(pold + i) + j);
				pn = *(*(pnew + i) + j);
				for (k = 0; k < nn; k++, po++, pn++){
					*po = 0.;
					*pn = 0.;
				}
			}
		}
	}


	nsweeps += tsweeps;
	for (sweep = tsweeps; sweep < nsweeps; sweep++){

		for (ticks = 0; ticks < nticks; ticks++){

			update_reset(ttime,ticks,rflag,&tmax,&reset);

			if (reset){
				if (backprop == 0)
					reset_network(zold,znew,pold,pnew);
				else
					reset_bp_net(zold,znew);
			}

			update_inputs(zold,ticks,iflag,&tmax,&linput);

			if (learning || teacher || (rms_report != 0))
				update_targets(target,ttime,ticks,tflag,&tmax);

			act_nds(zold,zmem,znew,wt,linput,target);

			comp_errors(zold,target,error,&err,&ce_err);

			if (learning && (backprop == 0))
				comp_deltas(pold,pnew,wt,dwt,zold,znew,error);
			if (learning && (backprop == 1))
				comp_backprop(wt,dwt,zold,zmem,target,error,linput);

			if (probe)
				print_nodes(zold);
		}
#ifdef GRAPHICS
		if ((dsp_type != 0) && (sweep%dsp_freq == 0))
			do_dsp();
#endif GRAPHICS
		if (verify)
			print_output(zold);

		if (rms_report && (++rtime >= rms_report)){
			rtime = 0;
			if (ce == 2)
				print_error(&ce_err);
			else
				print_error(&err);
		}

		if (check && (++ctime >= check)){
			ctime = 0;
			save_wts();
		}

		if (++ttime >= tmax)
			ttime = 0;

		if (learning && (++utime >= umax)){
			utime = 0;
			update_weights(wt,dwt,winc);
		}

	}
	if (learning)
		save_wts();
	exit(0);

}

usage() {
  fprintf(stderr, "\n");
  fprintf(stderr, "-f fileroot:\tspecify fileroot <always required>\n");
  fprintf(stderr, "-l weightfile:\tload in weightfile\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "-s #:\trun for # sweeps <always required>\n");
  fprintf(stderr, "-r #:\tset learning rate to # (between 0. and 1.) [0.1]\n");
  fprintf(stderr, "-m #:\tset momentum to # (between 0. and 1.) [0.0]\n");
  fprintf(stderr, "-n #:\t# of clock ticks per input vector [1]\n");
  fprintf(stderr, "-t:\tfeedback teacher values in place of outputs\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "-S #:\tseed for random number generator [random]\n");
  fprintf(stderr, "-U #:\tupdate weights every # sweeps [1]\n");
  fprintf(stderr, "-E #:\trecord rms error in .err file every # sweeps [0]\n");
  fprintf(stderr, "-C #:\tcheckpoint weights file every # sweeps [0]\n");
  fprintf(stderr, "-M #:\texit program when rms error is less than # [0.0]\n");
  fprintf(stderr, "-X:\tuse auxiliary .reset file\n");
  fprintf(stderr, "-P:\tprobe selected nodes on each sweep (no learning)\n");
  fprintf(stderr, "-V:\tverify outputs on each sweep (no learning)\n");
  fprintf(stderr, "-R:\tpresent input patterns in random order\n");
  fprintf(stderr, "-I:\tignore input values during extra clock ticks\n");
  fprintf(stderr, "-T:\tignore target values during extra clock ticks\n");
  fprintf(stderr, "-L:\tuse RTRL temporally recurrent learning\n");
  fprintf(stderr, "-B #:\toffset for offset biasi initialization\n");
  fprintf(stderr, "-Dt#:\tdisplay type (0=none;1=activations;2=weights;3=both\n");
  fprintf(stderr, "-Df#:\tdisplay frequency (#cycles)");
  fprintf(stderr, "-Dp#:\thardcopy print frequency (#cycles)");
  fprintf(stderr, "-Dd:\tdelay after each display\n");
  fprintf(stderr, "\n");
}

void
intr(sig)
	int	sig;
{
	save_wts();
	exit(sig);
}


