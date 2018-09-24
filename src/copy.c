// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bmp.h"

FILE *inptr;
FILE *outptr;

int filePreparing(char * infile,char * outfile)
{
    // open input file
    inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 3)
    {
        fprintf(stderr, "Usage: copy infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[1];
    char *outfile = argv[2];

    filePreparing(infile, outfile); //Source and output files are ready

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    int origW = bi.biWidth;
    int origH = bi.biHeight;

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    //resize of file
    bi.biClrUsed = 0;
    bi.biBitCount = 24;
    bi.biCompression = 0x0000;
    bi.biClrImportant = 0;
    int padding = (4 - (bi.biWidth * sizeof(BYTE)) % 4) % 4;
    bi.biSizeImage = 0;
    bf.bfSize = 0;
    
    //https://stackoverflow.com/questions/48065528/color-changing-bmp-in-c?rq=1
    int stride = ((bi.biWidth * bi.biBitCount + 31) / 32) * 4;
    int size = stride * bi.biHeight;

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    for (int i = 0, biHeight = abs(origH); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < stride; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            BYTE mono = ( triple.rgbtBlue + triple.rgbtGreen + triple.rgbtRed ) / 3;

            // write RGB triple to outfile
            fwrite(&mono, sizeof(BYTE), 1, outptr);
        }
        
        //fseek(inptr, padding, SEEK_CUR);
    }

    fclose(inptr);
    fclose(outptr);
    return 0;
}
