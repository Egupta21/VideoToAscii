# Description

C program that donverts images and videos into ASCII art displayed in the terminal

- Adjustable scaling lets you control the number of ASCII characters used  

# Requirements

## Setup

Clone the repo <br>
`git clone https://github.com/Egupta21/VideoToAscii.git`<br>
`cd VideoToAscii`

### Install dependencies

ffmpeg setup<br>
`sudo apt update`<br>
`sudo apt install build-essential pkg-config libavcodec-dev libavformat-dev libavutil-dev libswscale-dev`

stb setup<br>
`mkdir stb`<br>
`wget -O stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h`

create media folder to store input media<br>
`mkdir media`

# Build

`gcc vidtoascii.c -o vidtoascii -I./stb -lavformat -lavcodec -lswscale -lavutil -lm`

# Usage

`./vidtoascii <file> <scaleY>`<br>
<file> → Path to an image (.jpg, .png, etc.) or video (.mp4, etc.)<br>
<scaleY> → Vertical scale factor (higher = fewer rows, lower = more detail)

## Examples

Convert an image:<br>
`./vidtoascii media/test.png 10`

Convert a video:<br>
`./vidtoascii media/demo.mp4 15`

# Project Structure

VideoToAscii/<br>
├── media/          # put test images and videos here<br>
├── stb/            # stb_image.h (downloaded via setup step)<br>
├── vidtoascii.c    # main source file<br>
└── vidtoascii      # compiled binary (after build)<br>




