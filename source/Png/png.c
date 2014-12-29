/*---------------------------------------------------------------------------------
 libBAG  Copyright (C) 2010 - 2013
  BassAceGold - <BassAceGold@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.

  3. Restricting the usage of or portions of code in this library
     from other software developers and or content creators
     is strictly prohibited in applications with software creation capabilities
     and or with the intent to profit from said restrictions.

  4. This notice may not be removed or altered from any source
     distribution.
---------------------------------------------------------------------------------*/

#include "lodepng.h"

/*
static inline u32 fourbyte_Array_mult(int x, int y, int width){
	return ( (y<<2) * width +  (x << 2));
}

static inline void _png_fillBuffer(GFXObj_t *object, const u8 *image){
	u32 x=0, y=0,r=0,g=0,b=0,a=0;
	u16 *buf = BAG_Display_GetGfxBuf(object);
	do{
		x = 0;
		do{
			u32 pos = fourbyte_Array_mult(x, y, VAL(BAG_Display_GetGfxWidth(object)));
			r = (image[ pos + 0]>>3); //red
			g = (image[ pos + 1]>>3) ;//green
			b = (image[ pos+ 2]>>3);  //blue
			buf[ x + (y * VAL(BAG_Display_GetGfxWidth(object))) ] = RGB15(r,g,b);

			a = (image[ pos + 3]);  //alpha
			VAL(BAG_Display_GetGfxAlphaPix(object, x, y)) = a;
		}while(++x < VAL(BAG_Display_GetGfxWidth(object)));
	}while(++y < VAL(BAG_Display_GetGfxHeight(object)));
}*/

u8 *BAG_Display_LoadPng(const char *filename, int *wd, int *ht){
	u8* buffer = NULL;
	u8* image = NULL;
	size_t buffersize = 0, imagesize = 0;

	LodePNG_Decoder pngDecoder;
	memset(&pngDecoder, 0, sizeof(LodePNG_Decoder));

	if(!LodePNG_loadFile ( &buffer, &buffersize, filename )){
		LodePNG_Decoder_init (&pngDecoder);
		LodePNG_decode( &pngDecoder, &image, &imagesize, buffer, buffersize ); /*decode the png*/

    *wd = pngDecoder.infoPng.width;
    *ht = pngDecoder.infoPng.height;

		//_png_fillBuffer(object, image);
	}
	else{
    //BAG_DBG_LibMsg(OPEN_FAIL_MSG, filename);
    printf("Error opening png file: %s\n", filename);
    if(image) free(image);
    image = NULL;
  }

	end:
    LodePNG_Decoder_cleanup(&pngDecoder);
    if(buffer) free(buffer);

	return image;
}

/*
int BAG_Display_ReloadPng(const char *filename, GFXObj_t *object){
    int tempWidth = VAL(BAG_Display_GetGfxFrameWd(object));
    int tempHeight = VAL(BAG_Display_GetGfxFrameHt(object));

	int err = BAG_Display_LoadPng(filename, object);
	BAG_Display_SetGfxFrameDim(object, tempWidth, tempHeight);
	return err;
}
*/

static void flipEndianness_integer(int *data){
    char temp[4];
    int input = *data;
    char *ptr = (char*)&input;

    int i = 0;
    //copy old endianness
    while(i < 4){
      temp[i] = ptr[i];
      i++;
    }

    //copy reveresed endianness
    while(i-- >= 0){
      ptr[3 - i] = temp[i];
    }
    (*data) = input;
}

void BAG_Display_ReadPngSize(const char *filename, int *width, int *height){
    FILE *png = fopen(filename, "rb");
    if(!png){
        printf("Error opening file: %s\n", filename);
        return;
    }

    fseek(png, 16, SEEK_SET);
    fread(width, 1, sizeof(int), png);
    fread(height, 1, sizeof(int), png);
    fclose(png);
    flipEndianness_integer(width);
    flipEndianness_integer(height);
}

