#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>

#define		hex2dec(x)		((x) - (((x) > '9') ? ('A'-10) : '0'))

char *str_toupper(char *str)
{
	char	*c = str;

	while ((*c = toupper(*c)) != '\0')
	{
		c++;
	}

	return str;
}

int main(int argc, char *argv[])
{
	int				bc, la, cs, err = 0;
	int 				dl;
	int				i;
	unsigned int	adr, adr2;
	FILE			*fi, *fo;
	char			line[1024], data[512];

	if (argc < 4) {
		fprintf(stderr, "usage : mot2bin <mot_file> <bin_file> <base_adr>\n");
		return -1;
	}

	sscanf(argv[3], "%x", &adr2);

	if ((fi = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Cannot open file : %s\n", argv[1]);
		return -1;
	}

	if ((fo = fopen(argv[2], "wb")) == NULL) {
		fprintf(stderr, "Cannot create file : %s\n", argv[2]);
		fclose(fi);
		return -1;
	}

	while (fgets(line, sizeof(line), fi) != NULL) {

		str_toupper(line);
		if (line[0] != 'S') {
			fprintf(stderr, "Unknown record type : \'%c\'\n", line[0]);
			break;
		}
		switch (line[1]) {
		case '0': case '1': case '9':
			la = 4;
			break;

		case '2': case '8':
			la = 6;
			break;

		case '3': case '7':
			la = 8;
			break;

		default:
			fprintf(stderr, "Unknown record type : \'S%c\'\n", line[1]);
			fclose(fi); fclose(fo);
			return -1;
		}

		bc = hex2dec(line[2])*16 + hex2dec(line[3]);
		for (adr = i = 0; i < la; i++) {
			adr = adr*16 + hex2dec(line[4+i]);
		}

		/* 僠僃僢僋僒儉寁嶼 */
		for (cs = bc, i = 0; i < bc; i++) {
			cs += data[i] = hex2dec(line[4+2*i])*16 + hex2dec(line[5+2*i]);
		}
		if ((cs & 255) != 255) {
			fprintf(stderr, "Check sum err : 0x%08x\n", adr);
			err = -1;
		}

		if ('1' <= line[1] && line[1] <= '3') {
			if (adr2 != adr) {
				//fprintf(stderr, "Data is not contiguous, fill by 0x00 : 0x%08x-0x%08x.\n", adr2, adr);
				err = 1;

				line[0] = 0x00;
				for (i = 0; i < (int)(adr - adr2); i++) {
					fwrite(line, sizeof(char), 1, fo);
				}
			}
			dl = (la/2);
			adr2 = adr + bc - 1 - dl;

			fwrite(data+dl, sizeof(char), bc - 1 - dl, fo);
		}
	}

	fclose(fi);
	fclose(fo);
	return err;
}
