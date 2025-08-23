#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "../stb/stb_image_write.h"

#define SCALEX                   10
#define SCALEY                   SCALEX*0.56

// typedef struct {
//     int width;
//     int height;
//     int channels;
//     unsigned char *data;
// } Image;

// Image loadImage(const char *filename, int desiredChannels);
// void convertToGrayscale(const Image *img, uint8_t *grayscaleData);
// void computeAsciiArt(const uint8_t *grayscaleData, int width, int height, int scalex, int scaley, const char *grayRamp, char *characterData);
// void printAsciiArt(const char *characterData, int outputCharWidth, int outputCharHeight);

void grayscaleCharacter(uint8_t averageDensity, char *grayRamp, char *characterData, size_t charIndex)
{
   size_t rampSize = strlen(grayRamp);
   size_t index = (size_t) round((averageDensity / 255.0) * (rampSize - 1));
   characterData[charIndex] = grayRamp[index];
}

void pixelsToGrayscale(unsigned char *img, size_t totalImageBytes, int channels, uint8_t grayscaleData[])
{
   for(unsigned char *pixelIndex = img; pixelIndex < (img + totalImageBytes); pixelIndex += channels)
   {
      uint8_t R = *(pixelIndex);
      uint8_t G = *(pixelIndex + 1);
      uint8_t B = *(pixelIndex + 2);
      uint8_t GrayScale = (uint8_t) round(0.21*R + 0.72*G + 0.07*B);
      grayscaleData[(pixelIndex - img) / channels] = GrayScale;
   }
}



int main(void)
{
   
   // channels - number of bytes in each pixel
   // width - width of the image
   // height - height of the image
   int width, height, channels;
   char *imageName = "IMG_9444.jpg";
   int desiredChannels = 3;

   unsigned char *img = stbi_load(imageName, &width, &height, &channels, desiredChannels);
   if(img == NULL)
   {
      printf("Error in loading image\n");
      return 1;
   }

   // number of horizontal characters to output
   // same as the number of blocks the image wants to be broken into, since each block represents a char
   size_t outputCharXct = width/SCALEX;
   // number of vertical characters to output
   size_t outputCharYct = height/SCALEY;

   printf("Loaded image with width of %dpx, height of %dpx, and %d channels\n", width, height, channels);
   
   size_t grayscaleDataBuffer = width * height;
   uint8_t *grayscaleData = malloc(grayscaleDataBuffer * sizeof(uint8_t));
   if(!grayscaleData)
   {
      printf("Error allocating memory for grayscale data\n");
      stbi_image_free(img);
      return 1;
   }

   // for black background
   //char *grayRamp = " .:~+*#$";

   // for white background
   char *grayRamp = "#$*+~:. ";
   size_t rampSize = strlen(grayRamp);

   char *characterData = (char *)malloc(outputCharXct * outputCharYct * sizeof(char));
   char *pchar = characterData;

   // convert the RGB data into grayscale data and then store into 1D array.
   // channels - number of bytes in each pixel
   // iterating over channels (p += channels) lets us jump to the next pixel
   // iterating upto (img + totalImageBytes) lets us iterate upto the memory address of the last pixel in the image
   size_t totalImageBytes = (size_t) (width * height * channels);
   pixelsToGrayscale(img, totalImageBytes, channels, grayscaleData);

   for(size_t blocky = 0; blocky < outputCharYct; blocky++) {
      size_t startY = blocky * height / outputCharYct;
      size_t endY   = (blocky+1) * height / outputCharYct;

      for(size_t blockx = 0; blockx < outputCharXct; blockx++) {
         size_t startX = blockx * width / outputCharXct;
         size_t endX   = (blockx+1) * width / outputCharXct;

         uint32_t currGridTotal = 0;
         size_t pixelcount = 0;

         for(size_t y = startY; y < endY; y++) {
            for(size_t x = startX; x < endX; x++) {
               currGridTotal += grayscaleData[y*width + x];
               pixelcount++;
            }
         }
         uint8_t averageDensity = 0;
         if (pixelcount)
         {
            averageDensity = (uint8_t)(currGridTotal / pixelcount);
         }
         
         grayscaleCharacter(averageDensity, grayRamp, characterData, blocky * outputCharXct + blockx);
         //printf("%i\n", averageDensity);
      }
   }
   // print out all the data in average density for testing
   for(size_t y = 0; y < outputCharYct; y++)
   {
      for(size_t x = 0; x < outputCharXct; x++)
      {
         size_t index = x + (y * outputCharXct);
         printf("%c", characterData[index]);
      }
      printf("\n");
   }

   //print the char data to terminal
   // for(size_t i = 0; i < width*height; i++)
   // {
   //    printf("%d\n", grayscaleData[i]);
   // }
   return 0;
}

// write a function to print hello world

