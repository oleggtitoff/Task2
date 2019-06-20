#include <stdio.h>
#include <stdint.h>

#define INPUT_FILE_NAME "Input.wav"		//TODO: from cmd
#define OUTPUT_FILE_NAME "Output.wav"	//TODO: from cmd
#define FILE_HEADER_SIZE 44
#define DATA_BUFF_SIZE 10
#define BYTES_PER_SAMPLE 2

void readHeader(uint8_t *headerBuff, FILE *inputFilePtr);
void writeHeader(uint8_t *headerBuff, FILE *outputFilePtr);
uint32_t defineDataSize(uint8_t *headerBuff);
void readData(int16_t *dataBuff, int16_t size, FILE *inputFilePtr);
void writeData(int16_t *data, int16_t size, FILE *outputFilePtr);
void swap_int16(int16_t *data, int16_t size);

int main()
{


	return 0;
}

void readHeader(uint8_t *headerBuff, FILE *inputFilePtr)
{
	if (fread(headerBuff, FILE_HEADER_SIZE, 1, inputFilePtr) != 1)
	{
		printf("Error reading input file (header)\n");
		system("pause");
		exit(0);
	}
}

void writeHeader(uint8_t *headerBuff, FILE *outputFilePtr)
{
	if (fwrite(headerBuff, FILE_HEADER_SIZE, 1, outputFilePtr) != 1)
	{
		printf("Error writing output file (header)\n");
		system("pause");
		exit(0);
	}
}

uint32_t defineDataSize(uint8_t *headerBuff)
{
	return (*(headerBuff + FILE_HEADER_SIZE - 4)) |
		(*(headerBuff + FILE_HEADER_SIZE - 3) << 8) |
		(*(headerBuff + FILE_HEADER_SIZE - 2) << 16) |
		(*(headerBuff + FILE_HEADER_SIZE - 1) << 24);
}

void readData(int16_t *dataBuff, int16_t size, FILE *inputFilePtr)
{
	if (fread(dataBuff, BYTES_PER_SAMPLE, size, inputFilePtr) != size)
	{
		printf("Error reading input file (data)\n");
		system("pause");
		exit(0);
	}
}

void writeData(int16_t *data, int16_t size, FILE *outputFilePtr)
{
	if (fwrite(data, BYTES_PER_SAMPLE, size, outputFilePtr) != size)
	{
		printf("Error writing output file (data)\n");
		system("pause");
		exit(0);
	}
}

void swap_int16(int16_t *data, int16_t size)
{
	int16_t i;

	for (i = 0; i < size; i++)
	{
		*(data + i) = ((*(data + i)) << 8) | (((*(data + i)) >> 8) & 0xFF);
	}
}