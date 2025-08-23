#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "../stb/stb_image_write.h"

#define SCALEX                   10
#define SCALEY                   SCALEX*0.56

// Grayscale conversion weights for R, G, B channels
#define GRAYSCALE_WEIGHT_R       0.21
#define GRAYSCALE_WEIGHT_G       0.72
#define GRAYSCALE_WEIGHT_B       0.07

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

/**
 * outputs the ascii art to the terminal in the right layout
 * 
 * @param characterData - pointer to characterData array
 * @param outputCharCountXAxis - count of characters in the x axis
 * @param outputCharCountYAxis - count of characters in the y axis

 */
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

/**
 * selects ascii character based on grayscale pixel value
 * 
 * @param averageDensitiy - density to base the character off of
 * @param grayRamp - pointer to string of characters to pick from
 * @param characterData - pointer to array of characters to store the characters in
 * @param charIndex - location to store each character in characterData array
 */
void selectAsciiCharacter(const uint8_t averageDensity, const char *grayRamp, char *characterData, const int charIndex)
{
   size_t rampSize = strlen(grayRamp);
   size_t index = (size_t) round((averageDensity / 255.0) * (rampSize - 1));
   characterData[charIndex] = grayRamp[index];
}

/**
 * converts 3 channel color RGB image to grayscale
 * 
 * @param img - pointer to struct of type Image contataining all image information 
 * @param grayscaleData - pointer to the grayscale pixel data
 */
void pixelsToGrayscale(const Image *img, uint8_t *grayscaleData)
{
   for(unsigned char *pixelIndex = img->data; pixelIndex < (unsigned char*) (img->data + (img->width * img->height * img->channels)); pixelIndex += img->channels)
   {
      uint8_t R = *(pixelIndex);
      uint8_t G = *(pixelIndex + 1);
      uint8_t B = *(pixelIndex + 2);
      uint8_t grayScaleVal = (uint8_t) round(GRAYSCALE_WEIGHT_R * R + GRAYSCALE_WEIGHT_G * G + GRAYSCALE_WEIGHT_B * B);
      grayscaleData[(pixelIndex - img->data) / img->channels] = grayScaleVal;
   }
}

/**
 * Converts a grayscale image into ASCII art.
 * 
 * @param grayscaleData - Pointer to the grayscale pixel data.
 * @param width - Width of the input image in pixels.
 * @param height - Height of the input image in pixels.
 * @param outputCharCountXAxis - Number of ASCII characters per row in the output.
 * @param outputCharCountYAxis - Number of ASCII characters per column in the output.
 * @param grayRamp - String representing the ramp of ASCII characters.
 * @param characterData - Output buffer to store the resulting ASCII characters.
 */
void computeAsciiArt(const uint8_t *grayscaleData, int width, int height, int outputCharCountXAxis, int outputCharCountYAxis, const char *grayRamp, char *characterData)
{
   // iterate over block count and y and x axis
   // use output character count in each axis as limiter for block count, as each block is used for 1 charcter
   for(size_t blocky = 0; blocky < outputCharCountYAxis; blocky++) {
      size_t startY = blocky * height / outputCharCountYAxis; // sets starty to top left y pixel for current block
      size_t endY = (blocky+1) * height / outputCharCountYAxis; // sets endy to one past the bottom pixel for the current block

      for(size_t blockx = 0; blockx < outputCharCountXAxis; blockx++) {
         size_t startX = blockx * width / outputCharCountXAxis; // sets startx to top left x pixel for current block
         size_t endX = (blockx+1) * width / outputCharCountXAxis; // sets endX to one past the rightmost pixel for the current block

         uint32_t currGridTotal = 0; // sum of all pixel grayscale values in current block
         size_t pixelcount = 0; // count of number of pixels checked in current block

         // iterate over all pixels in current block
         for(size_t y = startY; y < endY; y++) {
            for(size_t x = startX; x < endX; x++) {
               currGridTotal += grayscaleData[y*width + x]; // add to currGridTotal
               pixelcount++; // update pixel count
            }
         }

         // calculate average density of block
         uint8_t averageDensity = 0;
         if (pixelcount)
         {
            averageDensity = (uint8_t)(currGridTotal / pixelcount);
         }
         
         // Map 2D block coordinates (blocky, blockx) to 1D characterData array index
         // Index = blocky * outputCharCountXAxis + blockx
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

   // calculate the amount of characters in the x and y axis based on the scaling factor
   int outputCharCountXAxis = img.width/SCALEX;
   int outputCharCountYAxis = img.height/SCALEY;

   // allocating memory to store the grayscale image data
   uint8_t *grayscaleData = (uint8_t*) malloc(img.width * img.height * sizeof(uint8_t));
   // allocation memory to store the character data
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

