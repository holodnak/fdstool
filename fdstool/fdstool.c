#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char u8;
typedef unsigned short u16;

//0 intvect.bin DFF6 10 PRG

#define stricmp _stricmp

typedef struct diskfile_s {
	u8 number;
	u8 code;
	char filename[8];
	u16 loadaddr;
	u16 size;
	u8 type;
	u8 data[0x10000];
} diskfile_t;

unsigned char buf[0x10000];
diskfile_t files[64];

void writefile(FILE *fp, diskfile_t *file)
{
	fputc(3, fp);
	fwrite(file, 15, 1, fp);
	fputc(4, fp);
	fwrite(file->data, file->size, 1, fp);
}

int main(int argc, char *argv[])
{
	char header[] = "FDS\x1A\1\0\0\0\0\0\0\0\0\0\0\0";
	FILE *fp;
	FILE *fds;
	FILE *file;
	int n,i;
	diskfile_t *f = files;
	char tmp[16];
	char *in, *out;

	in = "diskinfo.txt";
	out = "disk.fds";

	if (argc < 2) {
		printf("usage: fdstool <outfile.fds> [info.txt]\n\n");
		return(0);
	}
	out = argv[1];
	if (argc > 2) {
		in = argv[2];
	}

	memset(files, 0, sizeof(diskfile_t) * 64);

	fp = fopen(in, "rt");
	fds = fopen(out, "wb");

	if (fp == 0) {
		printf("error opening '%s'\n",in);
		return(1);
	}
	if (out == 0) {
		printf("error opening '%s'\n",out);
		return(1);
	}

	printf("reading '%s'...\n",in);
	n = 0;
	while (feof(fp) == 0) {
		char tmp2[16], *p;
		int code, addr, size, len;

		fscanf(fp,"%d %s %x %d %s\n", &code, tmp, &addr, &size, tmp2);
		printf("  %s \tcode %d\tloadaddr %04X\tsize %d\n", tmp,code,addr,size);

		file = fopen(tmp, "rb");
		if (file == 0) {
			printf("error opening %s\n", tmp);
			break;
		}
		else {
			fseek(file, 0, SEEK_END);
			len = ftell(file);
			fseek(file, 0, SEEK_SET);
			fread(f->data, len, 1, file);
			fclose(file);
		}
		p = strchr(tmp, '.');
		*p = 0;
		p = tmp;
		while (*p) {
			*p = (char)toupper(*p);
			p++;
		}
		f->number = n;
		f->code = (u8)code;
		strncpy(f->filename, tmp, 8);
		f->loadaddr = (u16)addr;
		f->size = (u16)size;
		if (stricmp(tmp2, "PRG") == 0)
			f->type = 0;
		else if (stricmp(tmp2, "CHR") == 0)
			f->type = 1;
		else if (stricmp(tmp2, "NT") == 0)
			f->type = 2;
		else {
			printf("bad file type on line %d\n", n);
			break;
		}
		n++;
		f++;
	}
	fclose(fp);

	printf("writing %s...\n", out);

	fwrite(header, 16, 1, fds);

	file = fopen("block1.bin", "rb");
	if (file == 0) {
		printf("error opening block1.bin\n");
	}
	fread(buf, 56, 1, file);
	fwrite(buf, 56, 1, fds);
	fclose(file);

	fputc(2, fds);
	fputc(n, fds);

	f = files;
	for (i = 0; i < n; i++, f++) {
		strncpy(tmp, f->filename, 8);
		tmp[8] = 0;
		printf("  %s\n", tmp);
		writefile(fds, f);
	}
	printf("padding until eof...");
	while (ftell(fds) < 65516) {
		fputc(0, fds);
	}
	printf("done.\n");
	fclose(fp);
	fclose(fds);
//	system("pause");
	return(0);
}