#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#define FILE_HEADER_SIZE 44
#define DATA_BUFF_SIZE 10
#define BYTES_PER_SAMPLE 2
#define FRACTIONAL_BITS 31


int32_t floatToFixed32(float x);
int32_t Mul(int32_t x, int32_t y);

FILE * openFile(char *fileName, _Bool mode);	//if 0 - read, if 1 - write
void readHeader(uint8_t *headerBuff, FILE *inputFilePtr);
void writeHeader(uint8_t *headerBuff, FILE *outputFilePtr);

int32_t dBtoGain(float dB);
void run(FILE *inputFilePtr, FILE *outputFilePtr, int32_t gain);


int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Error. The number of arguments should be 3\n");
		system("pause");
		exit(0);
	}

	if (atof(argv[3]) > 0)
	{
		printf("Wrong argument. dB gain can be only < 0\n");
		system("pause");
		exit(0);
	}

	FILE *inputFilePtr = openFile(argv[1], 0);
	FILE *outputFilePtr = openFile(argv[2], 1);
	uint8_t headerBuff[FILE_HEADER_SIZE];

	readHeader(headerBuff, inputFilePtr);
	writeHeader(headerBuff, outputFilePtr);
	run(inputFilePtr, outputFilePtr, dBtoGain(atof(argv[3])));
	fclose(inputFilePtr);
	fclose(outputFilePtr);

	return 0;
}

int32_t floatToFixed32(float x)
{
	if (x >= 1)
	{
		return INT32_MAX;
	}
	else if (x < -1)
	{
		return INT32_MIN;
	}

	return (int32_t)(x * (double)(1LL << FRACTIONAL_BITS));
}

int32_t Mul(int32_t x, int32_t y)
{
	if (x == INT32_MIN && y == INT32_MIN)
	{
		return INT32_MAX;
	}

	return (int32_t)(((int64_t)x * y) >> 31);
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

FILE * openFile(char *fileName, _Bool mode)		//if 0 - read, if 1 - write
{
	FILE *filePtr;

	if (mode == 0)
	{
		if ((filePtr = fopen(fileName, "rb")) == NULL)
		{
			printf("Error opening input file\n");
			system("pause");
			exit(0);
		}
	}
	else
	{
		if ((filePtr = fopen(fileName, "wb")) == NULL)
		{
			printf("Error opening output file\n");
			system("pause");
			exit(0);
		}
	}

	return filePtr;
}

int32_t dBtoGain(float dB)
{
	return floatToFixed32(powf(10, dB / 20.0f));
}

void run(FILE *inputFilePtr, FILE *outputFilePtr, int32_t gain)
{
	int16_t dataBuff[DATA_BUFF_SIZE];
	size_t samplesRead;
	uint32_t i;

	while (1)
	{
		samplesRead = fread(dataBuff, BYTES_PER_SAMPLE, DATA_BUFF_SIZE, inputFilePtr);

		if (!samplesRead)
		{
			break;
		}

		for (i = 0; i < samplesRead; i++)
		{
			dataBuff[i] = (int16_t)(Mul((int32_t)dataBuff[i] << 16, gain) >> 16);
		}

		fwrite(dataBuff, BYTES_PER_SAMPLE, samplesRead, outputFilePtr);
	}
}
