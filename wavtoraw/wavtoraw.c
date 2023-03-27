#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define saferead(x, y, z) if (fscanf(x, y, z) != 1) puts("Failed to read " #z)

double *readSample(FILE* file, int chunkSize, int sampleSize, int numChan);

int main(int argc, char *argv[]) {
    char prefix[4];
    char fileFormat[4];
    char ckID[4];
    uint32_t nChunkSize;
    short wFormatTag;
    short nChannels;
    uint32_t nSamplesPerSecond;
    uint32_t nBytesPerSecond;
    uint32_t nOutputSamplesPerSecond;
    short nBlockAlign;
    short nBitsPerSample;

    if (argc < 3) {
        printf("WavToRaw 1.2 - Stephane Dallongeville - copyright 2016\n");
        printf("\n");
        printf("Usage: wav2raw sourceFile destFile <outRate>\n");
        printf("Output rate is given in Hz.\n");
        printf("Success returns errorlevel 0. Error return greater than zero.\n");
        printf("\n");
        printf("Ex: wav2raw input.wav output.raw 11025\n");
        exit(1);
    }
    /* Open source for binary read (will fail if file does not exist) */
    FILE *infile = fopen(argv[1], "rb");
    if (!infile) {
        printf("The source file %s was not opened\n", argv[1]);
        exit(2);
    }
    if (argc > 3) {
        nOutputSamplesPerSecond = atoi(argv[3]);
    } else {
        nOutputSamplesPerSecond = 0;
    }
    /* Read the header bytes. */
    saferead(infile, "%4s", prefix);
    saferead(infile, "%4c", &nChunkSize);
    saferead(infile, "%4c", fileFormat );
    saferead(infile, "%4c", ckID );
    saferead(infile, "%4c", &nChunkSize );
    saferead(infile, "%2c", &wFormatTag );
    saferead(infile, "%2c", &nChannels );
    saferead(infile, "%4c", &nSamplesPerSecond );
    saferead(infile, "%4c", &nBytesPerSecond );
    saferead(infile, "%2c", &nBlockAlign );
    saferead(infile, "%2c", &nBitsPerSample );
    // pass extra bytes in bloc
    for(int i = 0; i < nChunkSize - 0x10; i++) {
        char c;
        saferead(infile, "%1c", &c);
    }
    saferead( infile, "%4c", ckID );
    saferead( infile, "%4c", &nChunkSize );

    if (nOutputSamplesPerSecond == 0) {
        nOutputSamplesPerSecond = nSamplesPerSecond;
    }
    /* Open output for write */
    FILE *outfile = fopen(argv[2], "wb");
    if(!outfile) {
        printf("The output file %s was not opened\n", argv[2]);
        exit(3);
    }

    int nBytesPerSample = nBitsPerSample / 8;
    int size = nChunkSize / (nChannels * nBytesPerSample);
    const double *data = readSample(infile, nChunkSize, nBytesPerSample, nChannels);
    double step = nSamplesPerSecond;
    step /= nOutputSamplesPerSecond;
    double value = 0;
    double lastSample = 0;
    int iOffset = 0;

    for(double offset = 0; offset < size; offset += step) {
        double sample = 0;
        if(step > 1.0) { // extrapolation
            if(value < 0) sample += lastSample * -value;
            value += step;
            while(value > 0) {
                lastSample = data[iOffset++];
                if (value >= 1) {
                    sample += lastSample;
                } else {
                    sample += lastSample * value;
                }
                value--;
            }
            sample /= step;
        } else { // interpolation
            sample = data[(int) offset];
        }
        char byte = round(sample);
        fwrite(&byte, 1, 1, outfile);
    }
    fclose(infile);
    fclose(outfile);
    return 0;
}

double nextSample(FILE* file, int sampleSize, int numChan) {
    double res = 0;
    unsigned char b;
    short w;
    long l;
    for(int i = 0; i < numChan; i++) {
        switch(sampleSize) {
            case 1: {
                fscanf(file, "%1c", &b);
                res += ((int) b) - 0x80;
                break;
            }
            case 2: {
                fscanf(file, "%2c", &w);
                res += w >> 8;
                break;
            }
            case 3: {
                fscanf(file, "%4c", &l);
                res += l >> 16;
                break;
            }
            case 4: {
                fscanf(file, "%4c", &l);
                res += l >> 24;
                break;
            }
        }
    }
    return res / numChan;
}

double *readSample(FILE* file, int chunkSize, int sampleSize, int numChan){
    int size = chunkSize / (numChan * sampleSize);
    double *result = malloc(size * sizeof(double));
    double *dst = result;
    for(int i = 0; i < size; i++) {
        *dst++ = nextSample(file, sampleSize, numChan);
    }
    return result;
}
