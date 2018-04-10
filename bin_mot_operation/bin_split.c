#include	<stdio.h>
#include	<string.h>

#define		min(a,b)		(((a) < (b)) ? (a) : (b))
#define		numof(x)		(sizeof(x) / sizeof((x)[0]))

int main(int argc, char *argv[])
{
	int		l, i, n = 2;
	FILE	*fi, *fo[9];
	char	data[1024];

	if (argc < 2) {
		fprintf(stderr, "usage : bin_split <bin_file> [n]\n");
		return -1;
	}

	if (argc > 2) {
		sscanf(argv[2], "%d", &n);
		n = min(9, n);
	}

	if ((fi = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "Cannot open file : %s\n", argv[1]);
		return -1;
	}

	l = strlen(argv[1]);
	strncpy(data, argv[1], numof(data));
	l = min(l, numof(data)-1);

	data[l+1] = '\0';

	for (i = 0; i < n; i++) {
		data[l] = '0' + i;
		if ((fo[i] = fopen(data, "wb")) == NULL) {
			fprintf(stderr, "Cannot create file : %s\n", data);
			fclose(fi);
			for (l = 0; l < i-1; l++) {
				fclose(fo[l]);
			}
			return -1;
		}
	}

	i = 0;
	while (fread(data, sizeof(char), 1, fi) > 0) {
		fwrite(data, sizeof(char), 1, fo[i++ % n]);
	}

	fclose(fi);
	for (i = 0; i < n; i++) {
		fclose(fo[i]);
	}
	return 0;
}
