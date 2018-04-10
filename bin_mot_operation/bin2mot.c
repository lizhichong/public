#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>

#define		min(a,b)		(((a) < (b)) ? (a) : (b))

int main(int argc, char *argv[])
{
	int				bc, cs;
	int				i;
	unsigned int	adr, base = 0x20000000;
	FILE			*fi, *fo;
	unsigned char	line[33];

	if (argc < 3) {
		fprintf(stderr, "usage : bin2mot <bin_file> <mot_file> [base_adr:hex]\n");
		return -1;
	}

	if ((fi = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "Cannot open file : %s\n", argv[1]);
		return -1;
	}

	if ((fo = fopen(argv[2], "w")) == NULL) {
		fprintf(stderr, "Cannot create file : %s\n", argv[2]);
		fclose(fi);
		return -1;
	}

	if (argc >= 4) {
		sscanf(argv[3], "%x", &base);
	}
	adr = base;

	/* 僗僞乕僩儗僐乕僪惗惉 */
	bc = strlen(argv[2]);
	bc = min(bc, 32);
	fprintf(fo, "S0%02X0000", cs = bc+3);
	for (i = 0; i < bc; i++) {
		fprintf(fo, "%02X", argv[2][i]);
		cs += argv[2][i];
	}
	fprintf(fo, "%02X\n", 255 - (255&cs));

	/* 僨乕僞儗僐乕僪惗惉 */
	while ((bc = fread(line, sizeof(char), 32, fi)) > 0) {
		fprintf(fo, "S3%02X%08X", bc+5, adr);
		cs = (bc+5) + (adr>>24) + (adr>>16) + (adr>>8) + adr;
		adr += 32;
		for (i = 0; i < bc; i++) {
			fprintf(fo, "%02X", line[i]);
			cs += line[i];
		}
		fprintf(fo, "%02X\n", 255-(cs&255));
	}

	/* 僄儞僪儗僐乕僪惗惉 */
	fprintf(fo, "S705%08X%02X\n", base, 255-((5+(base>>24)+(base>>16)+(base>>8)+base)&255));

	fclose(fi);
	fclose(fo);
	return 0;
}
