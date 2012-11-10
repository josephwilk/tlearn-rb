
/* tlearn_ext.c - simulator for arbitrary networks with time-ordered input */

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
#include <ruby.h>

#include <math.h>
#include <stdio.h>
#include <signal.h>
#ifdef ibmpc
#include "strings.h"
#include <fcntl.h>
#else
#ifndef  THINK_C
#include <strings.h>
#include <sys/file.h>
#include <stdlib.h>
#else  /* THINK_C */
#include <console.h>
#include <time.h>
#include <stdlib.h>
#endif  /* THINK_C */
#endif
#ifdef  notdef
#include <sys/types.h>
#include <sys/stat.h>
#endif  /* notdef */

#ifdef ibmpc
#define random(x) rand(x)
#define srandom(x) srand(x)
#endif
#ifdef THINK_C
#define random(x) rand(x)
#define srandom(x) srand(x)
#endif  /* THINK_C */



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

extern float   *otarget;

float	*znew;		/* (nt) inputs and activations at time t+1 */
float	*zold;		/* (nt) inputs and activations at time t */
float	*zmem;		/* (nt) inputs and activations at time t */
float	**wt;		/* (nn x nt) weight TO node i FROM node j*/ 
float	**dwt;		/* (nn x nt) delta weight at time t */
float	**winc;		/* (nn x nt) accumulated weight increment*/
float	*target;	/* (no) output target values */
float	*error_values; /* (nn) error = (output - target) values */
float	***pnew;	/* (nn x nt x nn) p-variable at time t+1 */
float	***pold;	/* (nn x nt x nn) p-variable at time t */

extern float	*data;        /* Required to reset the .data file */


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

extern int start;

char	root[128];	/* root filename for .cf, .data, .teach, etc.*/
char	loadfile[128];	/* filename for weightfile to be read in */

FILE	*cfp;		/* file pointer for .cf file */

void intr();

extern int load_wts();
extern int save_wts();
extern int act_nds();

int run_training(nsweeps, file_path, current_weights_output)
  long nsweeps;
  char *file_path;
  float *current_weights_output;
{
  int status;
  int learning = 1;
  int loadflag = 0;
  
  backprop = 0;
  status = run(learning, loadflag, nsweeps, file_path, backprop, current_weights_output);

  return(status);
}

int run_fitness(nsweeps,file_path, current_weights_output)
  long nsweeps;
  char *file_path;
  float *current_weights_output;
{
  int status;
  backprop = 1;
  char weights[255];
  
  strcpy(weights, file_path);
  strcat(weights, ".wts");
  strcpy(loadfile,weights);
  
  int learning = 0;
  int loadflag = 1; 
  
  status = run(learning, loadflag, nsweeps, file_path, backprop, current_weights_output);

  return(status);
}

void cleanup_horrid_globals(){
    sweep = 0;
    tsweeps = 0;
    rate = .1;
    momentum = 0.;
    weight_limit = 1.;
    criterion = 0.;
    init_bias = 0.;
    rms_report = 0;
    ngroups = 0;
    teacher = 0;
    localist = 0;
    randomly = 0;
    limits = 0;
    ce = 0;

    pnew = 0;
    pold = 0;
    wt = 0;
    dwt = 0;
    winc = 0;

    ngroups = 0;
    start = 1;
}

int run(learning, loadflag, nsweeps, file_path, backprop, current_weights_output)
  int learning;
  int loadflag;
  long nsweeps;
  char *file_path;
  int backprop;
  float *current_weights_output;
{

  cleanup_horrid_globals();

  FILE  *fopen();
  extern  char *optarg;
  extern  float rans();
  extern  time_t time();

  long  ttime = 0;  /* number of sweeps since time = 0 */
  long  utime = 0;  /* number of sweeps since last update_weights */
  long  tmax = 0;   /* maximum number of sweeps (given in .data) */
  long  umax = 0;   /* update weights every umax sweeps */
  long  rtime = 0;  /* number of sweeps since last rms_report */
  long  check = 0;  /* output weights every "check" sweeps */
  long  ctime = 0;  /* number of sweeps since last check */

  int  c;
  int  i;
  int  j;
  int  k;
  int  nticks = 1;  /* number of internal clock ticks per input */
  int  ticks = 0;   /* counter for ticks */
  int  reset = 0;   /* flag for resetting net */

  int  iflag = 0;    /* flag for -I */
  int  tflag = 0;    /* flag for -T */
  int  rflag = 0;    /* flag for -x */
  int  seed = 0;     /* seed for random() */

  float  err = 0.;     /* cumulative ss error */
  float  ce_err = 0.;  /* cumulate cross_entropy error */

  float  *w;
  float  *wi;
  float  *dw;
  float  *pn;
  float  *po;

  struct  cf  *ci;

  char  cfile[128];  /* filename for .cf file */

  root[0] = 0;
  strcpy(root, file_path);

  if (nsweeps == 0){
    perror("ERROR: No -s specified");
    return(1);
  }

  /* open files */

  if (root[0] == 0){
    perror("ERROR: No fileroot specified");
    return(1);
  }

  sprintf(cfile, "%s.cf", root);
  cfp = fopen(cfile, "r");
  if (cfp == NULL) {
    perror("ERROR: Can't open .cf file");
    return(1);
  }

  get_nodes();
  make_arrays();
  get_outputs();
  get_connections();
  get_special();
  
  fclose(cfp);

  if (!seed){
    seed = time((time_t *) NULL);
  }
  srandom(seed);

  if (loadflag){
    load_wts();
  }
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

  data = 0;

  nsweeps += tsweeps;
  for (sweep = tsweeps; sweep < nsweeps; sweep++){
    for (ticks = 0; ticks < nticks; ticks++){

      update_reset(ttime,ticks,rflag,&tmax,&reset);

      if (reset){
        if (backprop == 0){
          reset_network(zold,znew,pold,pnew);
        }
		else{
          reset_bp_net(zold,znew);
	    }
      }

      update_inputs(zold,ticks,iflag,&tmax,&linput);

      if (learning || teacher || (rms_report != 0)){
        update_targets(target,ttime,ticks,tflag,&tmax);
	  }

      act_nds(zold,zmem,znew,wt,linput,target);

      comp_errors(zold,target,error_values,&err,&ce_err);

      if (learning && (backprop == 0))
        comp_deltas(pold,pnew,wt,dwt,zold,znew,error_values);
      if (learning && (backprop == 1))
        comp_backprop(wt,dwt,zold,zmem,target,error_values,linput);
    }
    if (learning == 0){
      for (i = 0; i < no; i++){
        current_weights_output[i] = zold[ni+outputs[i]];
      }
    }

    if (++ttime >= tmax){
      ttime = 0;
    }
    if (learning && (++utime >= umax)){
      utime = 0;
      update_weights(wt,dwt,winc);
    }

  }
  if (learning){
    save_wts();
	if(otarget != 0){
	  free(otarget);
	}
	otarget = 0;
  }
  
  if(backprop == 0){
    free_parrays();
  }

  free(data);

  free_arrays();

  return(0);
}

/* -- Ruby interface -- */

static VALUE tlearn_train(VALUE self, VALUE config) {
  VALUE sweeps_value = rb_hash_aref(config, ID2SYM(rb_intern("sweeps")));
  long nsweeps       = NUM2DBL(sweeps_value);

  VALUE file_root_value  = rb_hash_aref(config, ID2SYM(rb_intern("file_root")));
  char *file_root        = StringValueCStr(file_root_value);

  float current_weights_output[255];
  
  int result = run_training(nsweeps, file_root, current_weights_output);

  return rb_int_new(result);
}

static VALUE tlearn_fitness(VALUE self, VALUE config) {
  VALUE ruby_array       = rb_ary_new();
  VALUE file_root_value  = rb_hash_aref(config, ID2SYM(rb_intern("file_root")));

  VALUE sweeps_value = rb_hash_aref(config, ID2SYM(rb_intern("sweeps")));
  long nsweeps       = NUM2DBL(sweeps_value);

  char *file_root        = StringValueCStr(file_root_value);
  float *result_weights; 

  float current_weights_output[255];

  int failure = run_fitness(nsweeps, file_root, current_weights_output);

  if(failure == 0){
    float weight;
    int result_index;
    for(result_index = 0; result_index < no; result_index++){
      weight = current_weights_output[result_index];
      rb_ary_store(ruby_array, result_index, rb_float_new(weight));
    }
    return(ruby_array);
  }
  else{
    return(rb_int_new(failure));
  }
}

void Init_tlearn(void) {
  VALUE klass = rb_define_class("TLearnExt",
       rb_cObject);

 rb_define_singleton_method(klass,
     "train", tlearn_train, 1);

 rb_define_singleton_method(klass,
     "fitness", tlearn_fitness, 1);
}