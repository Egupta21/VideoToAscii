#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "stb/stb_image.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

// #define SCALEX                   5
// #define SCALEY                   SCALEX*0.56

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

int hasImageExtension(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    ext++;
    return (strcasecmp(ext, "jpg") == 0 ||
            strcasecmp(ext, "jpeg") == 0 ||
            strcasecmp(ext, "png") == 0 ||
            strcasecmp(ext, "bmp") == 0);
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

int main(int argc, char *argv[])
{
   if (argc < 3) {
        printf("Usage: %s <video_file>\n", argv[0]);
        return -1;
    }

       // for black background
   const char *grayRamp = " .:~+*#$";

   // for white background
   //char *grayRamp = "#$*+~:. ";

   const char *filename = argv[1];
   const int SCALEY = atoi(argv[2]);
   const double SCALEX = (double)(SCALEY*0.50);
   

   if(hasImageExtension(filename))
   {
      // channels - number of bytes in each pixel  
      // width - width of the image
      // height - height of the image
      //updateTerminalZoom(1, 1);
      int desiredChannels = 3;
      Image img = loadImage(filename, desiredChannels);
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
   else
   {
      // used AI from here
      AVFormatContext *fmt_ctx = NULL;
      if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
         fprintf(stderr, "Could not open source file %s\n", filename);
         return -1;
      }
      if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
         fprintf(stderr, "Could not find stream information\n");
         return -1;
      }

      
      // Find video stream
      int video_stream_index = -1;
      AVCodecParameters *codecpar = NULL;
      for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
         if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
               video_stream_index = i;
               codecpar = fmt_ctx->streams[i]->codecpar;
               break;
         }
      }
      if (video_stream_index == -1) {
         fprintf(stderr, "No video stream found\n");
         return -1;
      }

      int videoStreamIndex = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
      if (videoStreamIndex < 0) {
         fprintf(stderr, "Could not find video stream\n");
         return -1;
      }

      AVStream *videoStream = fmt_ctx->streams[videoStreamIndex];
      AVRational framerate = av_guess_frame_rate(fmt_ctx, videoStream, NULL);
      double fps = av_q2d(framerate);
      int delay_us = (int)(1000000.0 / fps);
      // Open codec
      const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
      if (!codec) {
         fprintf(stderr, "Codec not found\n");
         return -1;
      }
      AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
      avcodec_parameters_to_context(codec_ctx, codecpar);
      if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
         fprintf(stderr, "Could not open codec\n");
         return -1;
      }

      // Prepare scaling (YUV → RGB)
      struct SwsContext *sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height,
                                                   codec_ctx->pix_fmt,
                                                   codec_ctx->width, codec_ctx->height,
                                                   AV_PIX_FMT_RGB24,
                                                   SWS_BILINEAR, NULL, NULL, NULL);

      AVFrame *frame = av_frame_alloc();
      AVFrame *rgb_frame = av_frame_alloc();
      int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                             codec_ctx->width,
                                             codec_ctx->height, 1);
      uint8_t *rgb_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
      av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize,
                           rgb_buffer, AV_PIX_FMT_RGB24,
                           codec_ctx->width, codec_ctx->height, 1);

      AVPacket *packet = av_packet_alloc();

      // ASCII buffer sizes
      int outputCharCountXAxis = codec_ctx->width / SCALEX;
      int outputCharCountYAxis = codec_ctx->height / SCALEY;
      if (outputCharCountXAxis < 1) outputCharCountXAxis = 1;
      if (outputCharCountYAxis < 1) outputCharCountYAxis = 1;
      char *characterData = malloc(outputCharCountXAxis * outputCharCountYAxis);


      // Allocate grayscale buffer
      uint8_t *grayscaleData = malloc(codec_ctx->width * codec_ctx->height);
      struct timeval wall_start, end;
      gettimeofday(&wall_start, NULL);
      double wall_start_sec = wall_start.tv_sec + wall_start.tv_usec / 1000000.0;
      // Decode loop
      // Decode loop
      while (av_read_frame(fmt_ctx, packet) >= 0) {
         if (packet->stream_index == video_stream_index) {
            if (avcodec_send_packet(codec_ctx, packet) == 0) {
                  while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                     // Convert frame to RGB
                     sws_scale(sws_ctx, (const uint8_t * const *)frame->data,
                              frame->linesize, 0, codec_ctx->height,
                              rgb_frame->data, rgb_frame->linesize);

                     // Wrap in Image struct
                     Image img;
                     img.width = codec_ctx->width;
                     img.height = codec_ctx->height;
                     img.channels = 3;
                     img.data = rgb_frame->data[0];

                     // Grayscale → ASCII
                     pixelsToGrayscale(&img, grayscaleData);
                     computeAsciiArt(grayscaleData,
                                    img.width, img.height,
                                    outputCharCountXAxis, outputCharCountYAxis,
                                    grayRamp, characterData);

                     // PTS-based timing
                     double pts_seconds = (frame->pts != AV_NOPTS_VALUE) ?
                                          frame->pts * av_q2d(videoStream->time_base) :
                                          frame->best_effort_timestamp * av_q2d(videoStream->time_base);

                     struct timeval now;
                     gettimeofday(&now, NULL);
                     double now_sec = now.tv_sec + now.tv_usec / 1000000.0;
                     double target_sec = wall_start_sec + pts_seconds;
                     double delay_sec = target_sec - now_sec;
                     if (delay_sec > 0) {
                        usleep((unsigned int)(delay_sec * 1000000.0));
                     }

                     // Print ASCII frame
                     printf("\033[H");
                     printAsciiArt(characterData, outputCharCountXAxis, outputCharCountYAxis);
                     fflush(stdout);
                  }
            }
         }
         av_packet_unref(packet);
      }


      // cleanup
      free(grayscaleData);
      free(characterData);
      av_free(rgb_buffer);
      av_frame_free(&frame);
      av_frame_free(&rgb_frame);
      av_packet_free(&packet);
      sws_freeContext(sws_ctx);
      avcodec_free_context(&codec_ctx);
      avformat_close_input(&fmt_ctx);

      return 0;
   }
}

