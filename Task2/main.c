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

void readHeader(uint8_t *headerBuff, FILE *inputFilePtr);
void writeHeader(uint8_t *headerBuff, FILE *outputFilePtr);
uint32_t defineDataSize(uint8_t *headerBuff);
void readData(int16_t *dataBuff, int16_t size, FILE *inputFilePtr);
void writeData(int16_t *data, int16_t size, FILE *outputFilePtr);

FILE * openFile(char *fileName, _Bool mode);	//if 0 - read, if 1 - write
void closeFile(FILE *filePtr);

int32_t dBtoGain(float dB);
int16_t processSample(int16_t sample, int32_t gain);
void processBuffer(int16_t *data, int16_t size, int32_t gain);
void processData(FILE *inputFilePtr, FILE *outputFilePtr, uint32_t dataSize, int32_t gain);


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
	processData(inputFilePtr, outputFilePtr, defineDataSize(headerBuff), dBtoGain(atof(argv[3])));
	closeFile(inputFilePtr);
	closeFile(outputFilePtr);

	system("pause");
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

FILE * openFile(char *fileName, _Bool mode)		//if 0 - read, if 1 - write
{
	FILE * filePtr = NULL;
	errno_t err;

	if (mode == 0)
	{
		err = fopen_s(&filePtr, fileName, "rb");
	}
	else
	{
		err = fopen_s(&filePtr, fileName, "wb");
	}

	if (err != 0)
	{
		if (mode == 0)
		{
			printf("Error opening input file\n");
		}
		else
		{
			printf("Error opening output file\n");
		}

		system("pause");
		exit(0);
	}

	return filePtr;
}

void closeFile(FILE *filePtr)
{
	fclose(filePtr);
}

int32_t dBtoGain(float dB)
{
	return floatToFixed32(powf(10, dB / 20.0f));
}

int16_t processSample(int16_t sample, int32_t gain)
{
	return (int16_t)(Mul((int32_t)sample << 16, gain) >> 16);
}

void processBuffer(int16_t *data, int16_t size, int32_t gain)
{
	int16_t i;

	for (i = 0; i < size; i++)
	{
		*(data + i) = processSample(*(data + i), gain);
	}
}

void processData(FILE *inputFilePtr, FILE *outputFilePtr, uint32_t dataSize, int32_t gain)
{
	uint32_t i;
	int16_t bytesLeft;
	int16_t dataBuff[DATA_BUFF_SIZE];

	for (i = 0; i < dataSize / BYTES_PER_SAMPLE / DATA_BUFF_SIZE; i++)
	{
		readData(dataBuff, DATA_BUFF_SIZE, inputFilePtr);
		processBuffer(dataBuff, DATA_BUFF_SIZE, gain);
		writeData(dataBuff, DATA_BUFF_SIZE, outputFilePtr);

		bytesLeft = (dataSize / BYTES_PER_SAMPLE) % DATA_BUFF_SIZE;

		if (bytesLeft != 0)
		{
			readData(dataBuff, bytesLeft, inputFilePtr);
			processBuffer(dataBuff, DATA_BUFF_SIZE, gain);
			writeData(dataBuff, bytesLeft, outputFilePtr);
		}
	}
}