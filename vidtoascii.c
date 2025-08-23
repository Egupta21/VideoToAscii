#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "../stb/stb_image_write.h"

#define SCALEX                   10
#define SCALEY                   SCALEX*0.56

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char *data;
} Image;

Image loadImage(const char *filename, int desiredChannels);
void pixelsToGrayscale(const Image *img, uint8_t *grayscaleData);
void computeAsciiArt(const uint8_t *grayscaleData, int width, int height, int scalex, int scaley, const char *grayRamp, char *characterData);
void printAsciiArt(const char *characterData, int outputCharWidth, int outputCharHeight);
void selectAsciiCharacter(const uint8_t averageDensity, const char *grayRamp, char *characterData, const int charIndex);

Image loadImage(const char *filename, int desiredChannels)
{
   Image img;
   img.data = stbi_load(filename, &img.width, &img.height, &img.channels, 0);
   if (img.data != NULL) 
   {
       img.channels = desiredChannels; // Update channels to desired if loading was successful
   }
   return img;
}

void printAsciiArt(const char *characterData, int outputCharCountXAxis, int outputCharCountYAxis)
{
   // print out all the data in average density for testing
   for(size_t y = 0; y < outputCharCountYAxis; y++)
   {
      for(size_t x = 0; x < outputCharCountXAxis; x++)
      {
         size_t index = x + (y * outputCharCountXAxis);
         printf("%c", characterData[index]);
      }
      printf("\n");
   }
}
void selectAsciiCharacter(const uint8_t averageDensity, const char *grayRamp, char *characterData, const int charIndex)
{
   size_t rampSize = strlen(grayRamp);
   size_t index = (size_t) round((averageDensity / 255.0) * (rampSize - 1));
   characterData[charIndex] = grayRamp[index];
}

void pixelsToGrayscale(const Image *img, uint8_t *grayscaleData)
{
   for(unsigned char *pixelIndex = img->data; pixelIndex < (unsigned char*) (img->data + (img->width * img->height * img->channels)); pixelIndex += img->channels)
   {
      uint8_t R = *(pixelIndex);
      uint8_t G = *(pixelIndex + 1);
      uint8_t B = *(pixelIndex + 2);
      uint8_t GrayScale = (uint8_t) round(0.21*R + 0.72*G + 0.07*B);
      grayscaleData[(pixelIndex - img->data) / img->channels] = GrayScale;
   }
}

void computeAsciiArt(const uint8_t *grayscaleData, int width, int height, int outputCharCountXAxis, int outputCharCountYAxis, const char *grayRamp, char *characterData)
{
   for(size_t blocky = 0; blocky < outputCharCountYAxis; blocky++) {
      size_t startY = blocky * height / outputCharCountYAxis;
      size_t endY   = (blocky+1) * height / outputCharCountYAxis;

      for(size_t blockx = 0; blockx < outputCharCountXAxis; blockx++) {
         size_t startX = blockx * width / outputCharCountXAxis;
         size_t endX   = (blockx+1) * width / outputCharCountXAxis;

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
         
         selectAsciiCharacter(averageDensity, grayRamp, characterData, blocky * outputCharCountXAxis + blockx);
      }
   }
}

int main(void)
{
   
   // channels - number of bytes in each pixel  
   // width - width of the image
   // height - height of the image
   const char *imgName = "media/michcar.jpg";
   int desiredChannels = 3;
   Image img = loadImage(imgName, desiredChannels);
   if(img.data == NULL)
   {
      printf("Error in loading image\n");
      return 1;
   }

   int outputCharCountXAxis = img.width/SCALEX;
   int outputCharCountYAxis = img.height/SCALEY;

   uint8_t *grayscaleData = (uint8_t*) malloc(img.width * img.height * sizeof(uint8_t));
   char *characterData = malloc(outputCharCountXAxis * outputCharCountYAxis * sizeof(char));

   if(grayscaleData == NULL || characterData == NULL)
   {
      free(grayscaleData);
      free(characterData);
      printf("Error allocating memory on the heap\n");
      return 1;
   }

   // for black background
   const char *grayRamp = " .:~+*#$";

   // for white background
   //char *grayRamp = "#$*+~:. ";

   // processing
   pixelsToGrayscale(&img, grayscaleData);
   computeAsciiArt(grayscaleData, img.width, img.height, outputCharCountXAxis, outputCharCountYAxis, grayRamp, characterData);

   // output
   printAsciiArt(characterData, outputCharCountXAxis, outputCharCountYAxis);

   //cleanup
   free(grayscaleData);
   free(characterData);
   stbi_image_free(img.data);
   return 0;

}

// write a function to print hello world

