/*
/* * citrix_rle.c
 *
 *  Created on: May 8, 2017
 *      Author: bindus
 */

/*
Compression

Write a program that takes two arguments: an input filename and an output filename.

Write the input file’s contents to the output file, compressing any repeated sequences of characters.

Examples:
Input: abc            Output: abc
Input: aaabbbccc        Output: a3b3c3



Compression (cont.)

Cases to consider:
Very large input file
¡Feliz cumpleaños!


Decompression

Write a program that takes two arguments: an input filename and an output filename.

The input file is the output of the compression algorithm from the previous problem.

Write the input file’s contents to the output file, expanding any compressed sequences of characters.

Examples:
Input: abc            Output: abc
Input: a3b3c3        Output: aaabbbccc



Decompression (bonus)

Consider this case:

Original input:        x3y3z3
Compressed output:    x3y3z3
Decompressed output:     xxxyyyzzz

The compression algorithm corrupted the file!

How would you fix the algorithm?

*/

#define SIZE 1024
#define SCNT 2

char ibuf[SIZE];
char obuf[SIZE];

int rle_encode(char input_buff[], char output_buff[]){

	char * ps = input_buff;
	//char * output_buff = malloc(sizeof(char)* BUFSIZE);
	char * pcur_seq = ps;
	int cnt = 0;
	int out_indx = 0;
	char last_seq;
	while(*ps){

		pcur_seq++;
		cnt = 0;
		while(*ps++ == *pcur_seq++){
			cnt++;

		}
		last_seq = *ps;
		if(out_indx <=  SIZE - SCNT) {

			if(cnt)
				sprintf(output_buff[out_indx],"%c%d", last_seq, cnt );
			else
				sprintf(output_buff[out_indx],"%c", last_seq);
			out_indx += SCNT;
		}
		ps++;
	}
	return out_indx;
}

/*
 * #optimize for large files
 * #sequence continues on next page/bufeer
 *
 *
 *
 * */

main(int argc, char** argv){

	char * inputfile = argv[1];
	char * outputfile = argv[2];

	size_t  nread = 0;
	int fd_input = open(argv[1],"O_READONLY");
	int fd_output = open(argv[2], "O_WRITEONLY");
	int out_indx = 0, nwrit = 0;
	nread = read(fd_input, ibuf, SIZE);
	if(fd_input != -1){
		while(nread){
			memset(obuf, 0,SIZE);
			out_indx = rle_encode(ibuf, obuf);
			nwrit = write(fd_output, obuf, out_indx);
			memset(ibuf, 0,SIZE);
			nread = read(fd_input, ibuf, SIZE);
		}
	}


}
