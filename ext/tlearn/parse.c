#include <stdio.h>
#ifdef ibmpc
#include "strings.h"
#else
#include <strings.h>
#endif
#include <math.h>
#define MAX_PARSE_BUF 4096


char	pbuf[MAX_PARSE_BUF];	/* retains all strings parsed so far */
char	nbuf[80];	/* shows string in get_nums */
int	nbp;

double	atof();

#ifdef ibmpc
extern char  far *malloc();
#else
#ifndef aix3
extern char  *malloc();
#endif
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

extern	struct	nf	*ninfo;		/* (nn) node info */

extern	FILE	*cfp;

extern	int	*outputs;	/* (no) indices of output nodes */
extern	int	*selects;	/* (nn+1) nodes selected for probe printout */

extern	int	ngroups;	/* number of groups */
extern	int	limits;		/* flag for limited weights */

extern	float	weight_limit;	/* bound for random weight init */

void parse_err()
{
	fprintf(stderr,"\nError in .cf file:\n\n");
	fprintf(stderr,"%s\n\n",pbuf);
	fprintf(stderr,"(%s)\n\n",nbuf);
	exit(1);

}

void get_str(fp,buf,str)
	FILE	*fp;
	char	*buf;
	char	*str;
{
	if (fscanf(fp,"%s",buf) == EOF){
		fprintf(stderr,"Premature EOF detected.\n\n");
		parse_err();
	}
	if (strlen(pbuf) > MAX_PARSE_BUF -512) strcpy(pbuf, pbuf +512);
	strcat(pbuf,buf);
	strcat(pbuf,str);
}

void get_nums(str,nv,offset,vec)
	char	*str;
	int	nv;
	int	offset;
	int	*vec;
{
	int	c, i, j, l, k, m, n;
	int	dash;
	int	input;

	char	tmp[80];

	dash = 0;
	input = 0;
	l = strlen(str);
	nbp = 0;
	for (i = 0; i <= nv; i++)
		vec[i] = 0;
	for (i = 0, j = 0; i < l; j++, i++){
		c = str[i];
		switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':	nbuf[nbp++] = tmp[j] = str[i];
					break;
			case 'i':	input++;
					j--;
					nbuf[nbp++] = str[i];
					break;
			case '-':	if (j == 0)
						parse_err();
					tmp[j] = '\0';
					j = -1;
					nbuf[nbp++] = str[i];
					m = atoi(tmp);
					dash = 1;
					break;
			case ',':	if (j == 0)
						parse_err();
					tmp[j] = '\0';
					j = -1;
					nbuf[nbp++] = str[i];
					if (dash){
						n = atoi(tmp);
						if (input == 1)
							parse_err();
						if (n < m)
							parse_err();
					}
					else{
						m = atoi(tmp);
						n = m;
					}
					if (input == 0){
						m += offset;
						n += offset;
					}
					if (n > nv){
						fprintf(stderr,"ERROR %d > %d\n",n,nv);
						parse_err();
					}
					if ((input) && (n > offset)){
						fprintf(stderr,"ERROR %d > %d\n",n,offset);
						parse_err();
					}
					for (k = m; k <= n; k++){
						if ((input == 0) && (k == offset))
							vec[0] = 1;
						else
							vec[k] = 1;
					}
					input = 0;
					dash = 0;
					break;
					
			default:	parse_err();
		}
	}
	if (j == 0)
		parse_err();
	tmp[j] = '\0';
	nbuf[nbp++] = tmp[j];
	if (dash){
		n = atoi(tmp);
		if (input == 1){
			fprintf(stderr,"Cannot use dash to connect input and noninput\n\n");
			parse_err();
		}
		if (n < m){
			fprintf(stderr,"Upper bound must exceed lower\n\n");
			parse_err();
		}
	}
	else{
		m = atoi(tmp);
		n = m;
	}
	if (input == 0){
		m += offset;
		n += offset;
	}
	if (n > nv){
		fprintf(stderr,"ERROR %d > %d\n",n,nv);
		parse_err();
	}
	if ((input) && (n > offset)){
		fprintf(stderr,"ERROR %d > %d\n",n,offset);
		parse_err();
	}
	for (k = m; k <= n; k++){
		if ((input == 0) && (k == offset))
			vec[0] = 1;
		else
			vec[k] = 1;
	}

	nbp = 0;

}

void get_nodes()
{
	int	i;

	char	buf[80];
	char	tmp[80];

	/* read nn, ni, no */
	nn = ni = no = -1;
	get_str(cfp,buf,"\n");
	/* first line must be "NODES:" */
	if (strcmp(buf, "NODES:") != 0){
		fprintf(stderr,".cf file must begin with NODES:\n");
		exit(1);
	}
	/* next three lines must specify nn, ni, and no in any order */
	for (i = 0; i < 3; i++){
		get_str(cfp,buf," ");
		get_str(cfp,tmp," ");
		if (tmp[0] != '=')
			parse_err();
		get_str(cfp,tmp,"\n");
		if (strcmp(buf, "nodes") == 0)
			nn = atoi(tmp);
		if (strcmp(buf, "inputs") == 0)
			ni = atoi(tmp);
		if (strcmp(buf, "outputs") == 0)
			no = atoi(tmp);
	}
	if ((nn < 1) || (ni < 0) || (no < 0) || (nn < no)){
		fprintf(stderr,"ERROR: Invalid specification\n\n");
		parse_err();
	}
	nt = 1 + ni + nn;
	np = 1 + ni;
}

void get_outputs()
{
	int	i;
	int	j;

	struct	nf	*n;

	char	buf[80];

	get_str(cfp,buf," ");
	/* next line must specify outputs */
	if (strcmp(buf, "output") != 0)
		parse_err();
	get_str(cfp,buf," ");
	if (strcmp(buf, "node") != 0){
		if (strcmp(buf, "nodes") != 0)
			parse_err();
	}
	get_str(cfp,buf," ");
	if (strcmp(buf, "are") != 0){
		if (strcmp(buf, "is") != 0)
		parse_err();
	}
	get_str(cfp,buf,"\n");
	get_nums(buf,nn+ni,ni,selects);
	if (selects[0] == 1){
		fprintf(stderr,"Node 0 cannot be an output\n");
		exit(1);
	}
	for (i = 1; i <= ni; i++){
		if (selects[i] == 1){
			fprintf(stderr,"An input cannot be an output\n");
			exit(1);
		}
	}
	n = ninfo;
	for (i = ni+1, j = -1; i <= nn+ni; i++, n++){
		if (selects[i] > 0){
			if (++j < no){
				outputs[j] = i-ni;
				n->targ = 1;
			}
		}
	}
	if (++j != no){
		fprintf(stderr,"Expecting %d outputs, found %d\n",no,j);
		exit(1);
	}

}

void get_connections()
{
	int	i;
	int	j;
	int	k;

	struct	cf	*ci;

	int	gn;

	float	min;
	float	max;

	char	buf[80];

	int	*tmp;
	int	*iselects;

	/* malloc space for iselects */
	iselects = (int *) malloc(nt * sizeof(int));
	if (iselects == NULL){
		perror("iselects malloc failed");
		exit(1);
	}

	get_str(cfp,buf,"\n");
	/* next line must be "CONNECTIONS:" */
	if (strcmp(buf, "CONNECTIONS:") != 0)
		parse_err();

	get_str(cfp,buf," ");

	/* next line must be "groups = #" */
	if (strcmp(buf, "groups") != 0)
		parse_err();
	get_str(cfp,buf," ");
	if (buf[0] != '=')
		parse_err();
	get_str(cfp,buf,"\n");
	ngroups = atoi(buf);

	/* malloc space for tmp */
	tmp = (int *) malloc((ngroups+1) * sizeof(int));
	if (tmp == NULL){
		perror("tmp malloc failed");
		exit(1);
	}

	get_str(cfp,buf," ");
	while (strcmp(buf, "SPECIAL:") != 0){
		/* a group is identified */
		if (strcmp(buf,"group") == 0){
			get_str(cfp,buf," ");
			get_nums(buf,ngroups,0,tmp);
			get_str(cfp,buf," ");
			if (buf[0] != '=')
				parse_err();
			get_str(cfp,buf," ");
			/* group * = fixed */
			if (strcmp(buf,"fixed") == 0){
				for (i = 0; i < nn; i++){
					ci = *(cinfo + i);
					for (j = 0; j < nt; j++, ci++){
						if (tmp[ci->num])
							ci->fix = 1;
					}
				}
			}
			/* group * = wmin & wmax */
			else {
				min = (float) atof(buf);
				get_str(cfp,buf," ");
				if (buf[0] != '&')
					parse_err();
				get_str(cfp,buf," ");
				max = (float) atof(buf);
				if (max < min){
					fprintf(stderr,"ERROR: %g < %g\n\n",max,min);
					parse_err();
				}
				for (i = 0; i < nn; i++){
					ci = *(cinfo + i);
					for (j = 0; j < nt; j++, ci++){
						if (tmp[ci->num]){
							limits = 1;
							ci->lim = 1;
							ci->min = min;
							ci->max = max;
						}
					}
				}
			}
			strcat(pbuf,"\n");
			get_str(cfp,buf," ");
		}
		/* a connection is specified */
		else {
			get_nums(buf,nn+ni,ni,selects);
			if (selects[0]){
				fprintf(stderr,"Connecting TO a bias\n\n");
				parse_err();
			}
			for (i = 1; i <= ni; i++){
				if (selects[i]){
					fprintf(stderr,"Connecting TO an input\n\n");
					parse_err();
				}
			}
			get_str(cfp,buf," ");
			if (strcmp(buf,"from") != 0)
				parse_err();
			get_str(cfp,buf," ");
			get_nums(buf,nn+ni,ni,iselects);
			for (i = 0; i < nn; i++){
				ci = *(cinfo + i);
				for (j = 0; j < nt; j++, ci++){
					if ((selects[i+ni+1]) && (iselects[j]))
						ci->con += 1;
				}
			}
			strcat(pbuf,"\t");
			get_str(cfp,buf," ");
			if (buf[0] == '='){
				get_str(cfp,buf," ");
				/* connection  = fixed */
				if (strcmp(buf,"fixed") == 0){
					for (i = 0; i < nn; i++){
						ci = *(cinfo + i);
						for (j = 0; j < nt; j++, ci++){
							if ((selects[i+ni+1]) &&
								(iselects[j]))
								ci->fix = 1;
						}
					}
				}
				else {
				    /* connection  = group # */
				    if (strcmp(buf,"group") == 0){
					get_str(cfp,buf," ");
					gn = atoi(buf);
					for (i = 0; i < nn; i++){
					    ci = *(cinfo + i);
					    for (j = 0; j < nt; j++, ci++){
						if ((selects[i+ni+1]) &&
							(iselects[j]))
							ci->num = gn;
					    }
					}
				    }
				    /* connection  = min & max */
				    else {
					min = (float) atof(buf);
					get_str(cfp,buf," ");
					if (buf[0] != '&')
						parse_err();
					get_str(cfp,buf," ");
					max = (float) atof(buf);
					if (max < min){
						fprintf(stderr,"ERROR: %g < %g\n\n",max,min);
						parse_err();
					}
					for (i = 0; i < nn; i++){
						ci = *(cinfo + i);
						for (j = 0; j < nt; j++, ci++){
							if ((selects[i+ni+1]) &&
								(iselects[j])){
								limits = 1;
								ci->lim = 1;
								ci->min = min;
								ci->max = max;
							}
						}
					}
				    }
				}
				get_str(cfp,buf,"\t");
				if (strcmp(buf,"fixed") == 0){
					for (i = 0; i < nn; i++){
						ci = *(cinfo + i);
						for (j = 0; j < nt; j++, ci++){
							if ((selects[i+ni+1]) &&
								(iselects[j]))
								ci->fix = 1;
						}
					}
					get_str(cfp,buf,"\t");
				}
				if (strcmp(buf,"one-to-one") == 0){
					for (k = 0; k < nt; k++){
						if (iselects[k])
							break;
					}
					for (i = 0; i < nn; i++){
						ci = *(cinfo + i);
						for (j = 0; j < nt; j++, ci++){
							if ((selects[i+ni+1]) &&
								(iselects[j])){
								if (ci->con == 1){
									ci->con = 0;
									ci->fix = 0;
									ci->lim = 0;
								}
								else
									ci->con -= 1;
							}
						}
						if (selects[i+np]){
							ci = *(cinfo + i) + k++;
							ci->con = 1;
							ci->fix = 1;
							ci->lim = 1;
						}
					}
					get_str(cfp,buf,"\n");
				}
			}
		}
	}
/*
	for (i = 0; i < nn; i++){
		ci = *(cinfo + i);
		for (j = 0; j < nt; j++, ci++){
			fprintf(stderr,"i: %d  j: %d  c: %d  f: %d  g: %d\n",
					i,j,ci->con,ci->fix,ci->num);
		}
	}
*/

}

void get_special()
{
	char	buf[80];
	char	tmp[80];

	int	i;

	int	*iselects;

	struct	nf	*n;

	/* malloc space for iselects */
	iselects = (int *) malloc(nt * sizeof(int));
	if (iselects == NULL){
		perror("iselects malloc failed");
		exit(1);
	}


	while (fscanf(cfp,"%s",buf) != EOF){
	if (strlen(pbuf) > MAX_PARSE_BUF -512) strcpy(pbuf, pbuf +512);
		strcat(pbuf,buf);
		strcat(pbuf," ");
		get_str(cfp,tmp," ");
		if (tmp[0] != '=')
			parse_err();
		get_str(cfp,tmp,"\n");
		if (strcmp(buf,"weight_limit") == 0)
			weight_limit = (float) atof(tmp);
		if (strcmp(buf,"selected") == 0){
			get_nums(tmp,nn,0,selects);
		}
		if (strcmp(buf,"linear") == 0){
			get_nums(tmp,nn,0,iselects);
			n = ninfo;
			for (i = 1; i <= nn; i++, n++){
				if (iselects[i])
					n->func = 2;
			}
		}
		if (strcmp(buf,"bipolar") == 0){
			get_nums(tmp,nn,0,iselects);
			n = ninfo;
			for (i = 1; i <= nn; i++, n++){
				if (iselects[i])
					n->func = 1;
			}
		}
		if (strcmp(buf,"binary") == 0){
			get_nums(tmp,nn,0,iselects);
			n = ninfo;
			for (i = 1; i <= nn; i++, n++){
				if (iselects[i])
					n->func = 3;
			}
		}
		if (strcmp(buf,"delayed") == 0){
			get_nums(tmp,nn,0,iselects);
			n = ninfo;
			for (i = 1; i <= nn; i++, n++){
				if (iselects[i])
					n->dela = 1;
			}
		}
	}

}
