#include <stdio.h>
#include <math.h>

main() {
	double	f;
	register int i;
	float	x;

	for (i = -64000; i<64000; i++) {
		x = exp((double) i * 0.00025);
		fwrite(&x, sizeof(x), 1, stdout);
	}
}
