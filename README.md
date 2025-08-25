# Description

C program that donverts images and videos into ASCII art displayed in the terminal

- Allows for adjustable scaling to control the amount of characters displyed in an axis

# Requirements

## Setup

Clone the repo 
`git clone https://github.com/Egupta21/VideoToAscii.git`
`cd VideoToAscii`


### Install dependencies

ffmpeg setup
`sudo apt update`
`sudo apt install build-essential pkg-config libavcodec-dev libavformat-dev libavutil-dev libswscale-dev`

stb setup
`mkdir stb`
`wget -O stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h`

# Build

`gcc vidtoascii.c -o vidtoascii -I./stb -lavformat -lavcodec -lswscale -lavutil -lm`

# Usage

`./vidtoascii <file> <scaleY>`
<file> → Path to an image (.jpg, .png, etc.) or video (.mp4, etc.)

<scaleY> → Vertical scale factor (higher = fewer rows, lower = more detail)

## Examples

Convert an image:
`./vidtoascii media/test.png 10`

Convert a video:
`./vidtoascii media/demo.mp4 15`

# Project Structure

ascii/
├── media/             # put test images and videos here
├── stb/               # stb_image.h (downloaded via setup step)
├── vidtoascii.c       # main source file
└── vidtoascii         # compiled binary (after build)



