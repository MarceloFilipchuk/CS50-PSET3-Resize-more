#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{

    // ensures proper usage
    if (argc != 4)
    {
        printf("Usage: ./resize f infile outfile.\n");
        return 1;
    }

    // remembers the values
    float f = atof(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    // ensures that the 'f' factor is major than 0 and less or equal than 100
    if (f <= 0 || f > 100)
    {
        printf("The resize factor should be major than 0 and equal or less than 100.\n");
        return 1;
    }

    // opens infile
    FILE *inptr = fopen(infile, "r");
    // checks if the infile is usable
    if (inptr == NULL)
    {
        fclose(inptr);
        printf("Could not open %s\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        printf("Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        printf("Unsupported file format.\n");
        return 4;
    }

    // define infile padding
    int infile_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // save the data from infile's height and width for later iterations in order to safely modify the outfile's headers
    int infile_width = bi.biWidth;
    int infile_height = bi.biHeight;

    // IF THE 'F' FACTOR IS EQUAL OR MAJOR THAN 1
    if (f >= 1)
    {
        // modifys BITMAPINFOHEADER's width and height by 'f' times
        bi.biHeight = bi.biHeight * f;
        bi.biWidth = bi.biWidth * f;

        // determine outfile's padding for scanlines using the new height and width values
        int outfile_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

        // define the new BITMAPFILEHEADER's bfSizeImage according to the new biHeight and biWidth
        bi.biSizeImage = ((sizeof(RGBTRIPLE) * bi.biWidth) + outfile_padding) * abs(bi.biHeight);

        // define the new BITMAPFILEHEADER's biSize according to the new dimensions
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;

        // write outfile's BITMAPFILEHEADER
        fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

        // write outfile's BITMAPINFOHEADER multiplying the image's height and width 'n' times
        fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

        // iterate over infile's scanlines
        for (int i = 0, biHeight = abs(infile_height); i < biHeight; i++)
        {
            // iterates f times to create the vertical copy
            for (int m = 0; m < (f - 1); m++)
            {
                // iterates over each pixel
                for (int j = 0; j < infile_width; j++)
                {
                    // temporary storage
                    RGBTRIPLE triple;

                    // read RGB triple from infile
                    fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                    // write RGB triple to outfile 'f' times
                    for (int l = 0; l < f; l++)
                    {
                        fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                    }
                }

                // adds padding (if necesary) to outfile
                if (outfile_padding != 0)
                    for (int k = 0; k < outfile_padding; k++)
                    {
                        fputc(0x00, outptr);
                    }

                // skip over padding in infile, if any
                fseek(inptr, infile_padding, SEEK_CUR);

                // returns to the begining of the scanline
                fseek(inptr, -(sizeof(RGBTRIPLE) * infile_width + infile_padding), SEEK_CUR);
            }
            // adds the final scanline
            for (int j = 0; j < infile_width; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // write RGB triple to outfile 'f' times
                for (int l = 0; l < f; l++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }

            // adds padding (if necesary) to outfile
            if (outfile_padding != 0)
                for (int k = 0; k < outfile_padding; k++)
                {
                    fputc(0x00, outptr);
                }

            // skip over padding in infile, if any
            fseek(inptr, infile_padding, SEEK_CUR);
        }
    }

    // IF THE 'F' FACTOR IS LESS THAN 1
    if (f < 1)
    {

        // defines f_factor that will be used in defining the outfile width and height
        int f_factor = 1 / f;
        int skip = f_factor - 1;

        // modifys BITMAPINFOHEADER's width and height by 'f' times
        bi.biHeight = bi.biHeight / f_factor;
        bi.biWidth = bi.biWidth / f_factor;

        // determine outfile's padding for scanlines using the new height and width values
        int outfile_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

        // define the new BITMAPFILEHEADER's bfSizeImage according to the new biHeight and biWidth
        bi.biSizeImage = ((sizeof(RGBTRIPLE) * bi.biWidth) + outfile_padding) * abs(bi.biHeight);

        // define the new BITMAPFILEHEADER's biSize according to the new dimensions
        bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;

        // write outfile's BITMAPFILEHEADER
        fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

        // write outfile's BITMAPINFOHEADER multiplying the image's height and width 'n' times
        fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

        // iterate over infile's scanlines

        for (int i = 0; i < abs(bi.biHeight); i++)
        {
            // iterate over pixels in scanline
            for (int j = 0; j < bi.biWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // writes RGB triple to outfile
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

                // moves the cursor f_factor minus 1 to the next pixel
                fseek(inptr, (sizeof(RGBTRIPLE) * skip), SEEK_CUR);
            }

            // adds padding if necesary
            for (int k = 0; k < outfile_padding; k++)
            {
                fputc(0x00, outptr);
            }

            // skip over padding, if any
            fseek(inptr, infile_padding, SEEK_CUR);

            // skips f_factor minus 1 rows
            for (int y = 0; y < skip; y++)
            {
                fseek(inptr, (sizeof(RGBTRIPLE) * (infile_width) + infile_padding), SEEK_CUR);
            }
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}