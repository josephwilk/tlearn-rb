
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

extern int  nn;    /* number of nodes */
extern int  ni;    /* number of inputs */
extern int  no;    /* number of outputs */
extern int  nt;    /* nn + ni + 1 */
extern int  np;    /* ni + 1 */

extern struct  cf {
   int  con;  /* connection flag */
   int  fix;  /* fixed-weight flag */
   int  num;  /* group number */
   int  lim;  /* weight-limits flag */
   float  min;  /* weight minimum */
   float  max;  /* weight maximum */
};

extern struct  nf {
   int  func;  /* activation function type */
   int  dela;  /* delay flag */
   int  targ;  /* target flag */
 };

extern struct  cf  **cinfo;  /* (nn x nt) connection info */
extern struct  nf  *ninfo;   /* (nn) node activation function info */

extern int  *outputs;    /* (no) indices of output nodes */

extern int  *selects;    /* (nn+1) nodes selected for probe printout */
extern int  *linput;     /* (ni) localist input array */

extern float  *znew;     /* (nt) inputs and activations at time t+1 */
extern float  *zold;     /* (nt) inputs and activations at time t */
extern float  *zmem;     /* (nt) inputs and activations at time t */
extern float  **wt;      /* (nn x nt) weight TO node i FROM node j*/ 
extern float  **dwt;     /* (nn x nt) delta weight at time t */
extern float  **winc;    /* (nn x nt) accumulated weight increment*/
extern float  *target;   /* (no) output target values */
extern float  *error;    /* (nn) error = (output - target) values */
extern float  ***pnew;   /* (nn x nt x nn) p-variable at time t+1 */
extern float  ***pold;   /* (nn x nt x nn) p-variable at time t */

extern float  rate;         /* learning rate */
extern float  momentum;     /* momentum */
extern float  weight_limit; /* bound for random weight init */
extern float  criterion;    /* exit program when rms error is less than this */
extern float  init_bias;    /* possible offset for initial output biases */

extern float	*data;        /* Required to reset the .data file */

extern long  sweep;       /* current sweep */
extern long  tsweeps;     /* total sweeps to date */
extern long  rms_report;  /* output rms error every "report" sweeps */

extern int  ngroups;   /* number of groups */

extern int  backprop;  /* flag for standard back propagation (the default) */
extern int  teacher;   /* flag for feeding back targets */
extern int  localist;  /* flag for speed-up with localist inputs */
extern int  randomly;  /* flag for presenting inputs in random order */
extern int  limits;    /* flag for limited weights */
extern int  ce;        /* flag for cross_entropy */

extern char  root[128];  /* root filename for .cf, .data, .teach, etc.*/
extern char  loadfile[128];  /* filename for weightfile to be read in */

extern FILE  *cfp;    /* file pointer for .cf file */

extern void intr();

extern int load_wts();
extern int save_wts();
extern int act_nds();

extern int optind;


int run_training(nsweeps, file_path, current_weights_output)
  long nsweeps;
  char *file_path;
  float *current_weights_output;
{
  int  argc = 1;
  char *argv[argc];
  argv[0] = "tlearn";
  int status;

  backprop = 0;
  status = run(argc,argv, nsweeps, file_path, backprop, current_weights_output);

  return(status);
}

int run_fitness(argc,argv, nsweeps, file_path, current_weights_output)
  int  argc;
  char  **argv;
  long nsweeps;
  char *file_path;
  float *current_weights_output;
{
  int status;
  backprop = 1;
  status = run(argc,argv, nsweeps, file_path, backprop, current_weights_output);

  return(status);
}

int run(argc,argv, nsweeps, file_path, backprop, current_weights_output)
  int  argc;
  char  **argv;
  long nsweeps;
  char *file_path;
  int backprop;
  float *current_weights_output;
{
  //Reset getopts
  optind  = 1;
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

  FILE  *fopen();
  FILE  *fpid;
  extern  char *optarg;
  extern  float rans();
  extern  time_t time();

  long  ttime = 0;  /* number of sweeps since time = 0 */
  long  utime = 0;  /* number of sweeps since last update_weights */
  long  tmax = 0;  /* maximum number of sweeps (given in .data) */
  long  umax = 0;  /* update weights every umax sweeps */
  long  rtime = 0;  /* number of sweeps since last rms_report */
  long  check = 0;  /* output weights every "check" sweeps */
  long  ctime = 0;  /* number of sweeps since last check */

  int  c;
  int  i;
  int  j;
  int  k;
  int  nticks = 1;  /* number of internal clock ticks per input */
  int  ticks = 0;  /* counter for ticks */
  int  learning = 1;  /* flag for learning */
  int  reset = 0;  /* flag for resetting net */
  int  verify = 0;  /* flag for printing output values */
  int  probe = 0;  /* flag for printing selected node values */
  int  command = 1;  /* flag for writing to .cmd file */
  int  loadflag = 0;  /* flag for loading initial weights from file */
  int  iflag = 0;  /* flag for -I */
  int  tflag = 0;  /* flag for -T */
  int  rflag = 0;  /* flag for -x */
  int  seed = 0;  /* seed for random() */

  float  err = 0.;  /* cumulative ss error */
  float  ce_err = 0.;  /* cumulate cross_entropy error */

  float  *w;
  float  *wi;
  float  *dw;
  float  *pn;
  float  *po;

  struct  cf  *ci;

  char  cmdfile[128];  /* filename for logging runs of program */
  char  cfile[128];  /* filename for .cf file */

  FILE  *cmdfp;

#ifdef  THINK_C
  argc = ccommand(&argv);
#endif  /* THINK_C */

  signal(SIGINT, intr);
#ifndef ibmpc
#ifndef  THINK_C
  signal(SIGHUP, intr);
  signal(SIGQUIT, intr);
  signal(SIGKILL, intr);
#endif  /* THINK_C */
#endif

#ifndef ibmpc
  exp_init();
#endif

  root[0] = 0;
  strcpy(root, file_path);

  while ((c = getopt(argc, argv, "f:hil:m:n:r:s:tC:E:ILM:PpRS:TU:VvXB:H:D:")) != EOF) {
    switch (c) {
      case 'C':
        check = (long) atol(optarg);
        ctime = check;
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
      case 't':
        teacher = 1;
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
        return(2);
      break;
    }
  }
  if (nsweeps == 0){
    perror("ERROR: No -s specified");
    return(1);
  }

  /* open files */

  if (root[0] == 0){
    perror("ERROR: No fileroot specified");
    return(1);
  }

  if (command){
    sprintf(cmdfile, "%s.cmd", root);
    cmdfp = fopen(cmdfile, "a");
    if (cmdfp == NULL) {
      perror("ERROR: Can't open .cmd file");
      return(1);
    }
    for (i = 1; i < argc; i++)
      fprintf(cmdfp,"%s ",argv[i]);
    fprintf(cmdfp,"\n");
    fflush(cmdfp);
  }

#ifndef  THINK_C
  sprintf(cmdfile, "%s.pid", root);
  fpid = fopen(cmdfile, "w");
  fprintf(fpid, "%d\n",  getpid());
  fclose(fpid);
#endif  /* THINK_C */

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

  data = 0;

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
    if (verify){
      for (i = 0; i < no; i++){
        current_weights_output[i] = zold[ni+outputs[i]];
      }

      //print_output(zold);
    }
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
  
  return(0);

}

/* -- Ruby interface -- */

int do_print(VALUE key, VALUE val, VALUE in) {
  fprintf(stderr, "Input data is %s\n", StringValueCStr(in));

  fprintf(stderr, "Key %s=>Value %s\n", StringValueCStr(key),
         StringValueCStr(val));

  return ST_CONTINUE;
}

static VALUE tlearn_train(VALUE self, VALUE config) {
  VALUE sweeps_value = rb_hash_aref(config, ID2SYM(rb_intern("sweeps")));
  long nsweeps       = NUM2DBL(sweeps_value);

  VALUE file_root_value  = rb_hash_aref(config, ID2SYM(rb_intern("file_root")));
  char *file_root        = StringValueCStr(file_root_value);

  float current_weights_output[6];
  
  int result = run_training(nsweeps, file_root, current_weights_output);
  return rb_int_new(result);
}

static VALUE tlearn_fitness(VALUE self, VALUE config) {
  int  tlearn_args_count = 4;
  char *tlearn_args[tlearn_args_count];




  VALUE ruby_array       = rb_ary_new();
  VALUE file_root_value  = rb_hash_aref(config, ID2SYM(rb_intern("file_root")));

  VALUE sweeps_value = rb_hash_aref(config, ID2SYM(rb_intern("sweeps")));
  long nsweeps       = NUM2DBL(sweeps_value);

  char *file_root        = StringValueCStr(file_root_value);
  char weights[strlen(file_root) + strlen(".wts")];

  float *result_weights; 

  strcpy(weights, file_root);

  tlearn_args[0] = "tlearn_fitness";
  tlearn_args[1] = "-l";
  tlearn_args[2] = strcat(weights, ".wts");
  tlearn_args[3] = "-V";

  float current_weights_output[6];

  int failure = run_fitness(tlearn_args_count, tlearn_args, nsweeps, file_root, current_weights_output);

  if(failure == 0){
    float weight;
    int result_index;
    for(result_index = 0; result_index < 6; result_index++){
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