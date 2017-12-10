int main(int argc, const char **argv)
{
	const char *source_file = "default.fs";
	if (argc == 2) {
		source_file = argv[1];
	}

	FILE *input_buffut_file = fopen(source_file, "r");
	char input_buff[17] = { 0 };
	uint8_t vmem[32] = { 0 };

	printf("Enter flag: ", "r");
	fgets(input_buff, 17, stdin);
	memcpy(vmem + 16, input_buff, 16);

	FILE *null_file = fopen("/dev/null", "w");
	char line[1024];
	while (fgets(line, 1024, input_buffut_file)) {
		fprintf(null_file, line,
			&vmem[0], &vmem[1], &vmem[2], &vmem[3], &vmem[4], &vmem[5], &vmem[6], &vmem[7],
			&vmem[8], &vmem[9], &vmem[10], &vmem[11], &vmem[12], &vmem[13], &vmem[14], &vmem[15],
			&vmem[16], &vmem[17], &vmem[18], &vmem[19], &vmem[20], &vmem[21], &vmem[22], &vmem[23], &vmem[24],
			&vmem[25], &vmem[26], &vmem[27], &vmem[28], &vmem[29], &vmem[30], &vmem[31],

			vmem[0], vmem[1], vmem[2], vmem[3], vmem[4], vmem[5], vmem[6], vmem[7],
			vmem[8], vmem[9], vmem[10], vmem[11], vmem[12], vmem[13], vmem[14], vmem[15],
			vmem[16], vmem[17], vmem[18], vmem[19], vmem[20], vmem[21], vmem[22], vmem[23], vmem[24],
			vmem[25], vmem[26], vmem[27], vmem[28], vmem[29], vmem[30], vmem[31]
	   );
	}

	if ( vmem[15] )
		puts("Invalid flag");
	else
		printf("Congrats! The flag is SECCON{%s}.\n", input_buff);

	return 0;
}
