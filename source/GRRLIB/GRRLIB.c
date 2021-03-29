#include "GRRLIB.h"

#include "ttf_font_ttf.h"
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library ftLibrary;
static FT_Face ftFace;

/* Static function prototypes */
static void DrawText(const char *string, unsigned int fontSize, void *buffer);
static bool BlitGlyph(FT_Bitmap *bitmap, int offset, int top, void *buffer);
static void BitmapTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int width, const unsigned int height);

void GRRLIB_InitFreetype(void) {
	if (FT_Init_FreeType(&ftLibrary)) {
		exit(0);
	}

	if (FT_New_Memory_Face(ftLibrary, ttf_font_ttf, ttf_font_ttf_size, 0, &ftFace)) {
		exit(0);
	}
}

// despite the fact that this function allocates to the heap most calls are never freed
/* Render the text string to a 4x4RGBA texture, return a pointer to this texture */
GRRLIB_texImg *GRRLIB_TextToTexture(const char *string, unsigned int fontSize, unsigned int fontColour) {
	FT_Set_Pixel_Sizes(ftFace, 0, fontSize);

	GRRLIB_texImg *texture = calloc(1, sizeof(GRRLIB_texImg));
	texture->w = 640;
	texture->h = fontSize*2;

	/* Set the font colour, alpha is determined when we blit the glyphs */
	fontColour = fontColour << 8;

	/* Create a tempory buffer, 640 pixels wide, for freetype draw the glyphs */
	void *tempTextureBuffer = (void*) malloc(texture->w * (fontSize*2) * 4);
	if (tempTextureBuffer == NULL) {
		/* Oops! Something went wrong! */
		exit(0);
	}
	
	/* Set the RGB pixels in tempTextureBuffer to the requested colour */
	unsigned int *p = tempTextureBuffer;
	unsigned int loop = 0;
	for (loop = 0; loop < (texture->w * (fontSize*2)); ++loop) {
		*(p++) = fontColour;	
	}

	/* Render the glyphs on to the temp buffer */
	DrawText(string, fontSize, tempTextureBuffer);

	/* Create a new buffer, this time to hold the final texture 
	 * in a format suitable for the Wii */
	texture->data = memalign(32, texture->w * (fontSize*2) * 4);

	/* Convert the RGBA temp buffer to a format usuable by GX */
	BitmapTo4x4RGBA(tempTextureBuffer, texture->data, texture->w, (fontSize*2));
	DCFlushRange(texture->data, texture->w * (fontSize*2) * 4);

	/* The temp buffer is no longer required */
	free(tempTextureBuffer);

	return texture;
}

static void DrawText(const char *string, unsigned int fontSize, void *buffer) {
	int penX = 0;
	int penY = fontSize;
	FT_GlyphSlot slot = ftFace->glyph;
	FT_UInt glyphIndex = 0;
	FT_UInt previousGlyph = 0;
	FT_Bool hasKerning = FT_HAS_KERNING(ftFace);

	/* Convert the string to UTF32 */
	size_t length = strlen(string);
	wchar_t *utf32 = (wchar_t*)malloc(length * sizeof(wchar_t)); 
	length = mbstowcs(utf32, string, length);
	
	/* Loop over each character, drawing it on to the buffer, until the 
	 * end of the string is reached, or until the pixel width is too wide */
	unsigned int loop = 0;
	for (loop = 0; loop < length; ++loop) {
		glyphIndex = FT_Get_Char_Index(ftFace, utf32[ loop ]);
		
		/* To the best of my knowledge, none of the other freetype 
		 * implementations use kerning, so my method ends up looking
		 * slightly better :) */
		if (hasKerning && previousGlyph && glyphIndex) {
			FT_Vector delta;
			FT_Get_Kerning(ftFace, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
			penX += delta.x >> 6;
		}

		if (FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_RENDER)) {
			/* Whoops, something went wrong trying to load the glyph 
			 * for this character... you should handle this better */
			continue;
		}
	
		if (BlitGlyph(&slot->bitmap, penX + slot->bitmap_left, penY - slot->bitmap_top, buffer) == true) {
			/* The glyph was successfully blitted to the buffer, move the pen forwards */
			penX += slot->advance.x >> 6;
			previousGlyph = glyphIndex;
		} else {
			/* BlitGlyph returned false, the line must be full */
			free(utf32);
			return;
		}
	}

	free(utf32);
}

/* Returns true if the character was draw on to the buffer, false if otherwise */
static bool BlitGlyph(FT_Bitmap *bitmap, int offset, int top, void *buffer) {
	int bitmapWidth = bitmap->width;
	int bitmapHeight = bitmap->rows;

	if (offset + bitmapWidth > 640) {
		/* Drawing this character would over run the buffer, so don't draw it */
		return false;
	}

	/* Draw the glyph onto the buffer, blitting from the bottom up */
	/* CREDIT: Derived from a function by DragonMinded */
	unsigned char *p = buffer;
	unsigned int y = 0;
	for (y = 0; y < bitmapHeight; ++y) {
		int sywidth = y * bitmapWidth;
		int dywidth = (y + top) * 640;

		unsigned int column = 0;
		for (column = 0; column < bitmapWidth; ++column) {
			unsigned int srcloc = column + sywidth;
			unsigned int dstloc = ((column + offset) + dywidth) << 2;
			/* Copy the alpha value for this pixel into the texture buffer */
			p[ dstloc + 3 ] = bitmap->buffer[ srcloc ];
		}
	}
	
	return true;
}

static void BitmapTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int width, const unsigned int height) {
	unsigned int block = 0;
	unsigned int i = 0;
	unsigned int c = 0;
	unsigned int ar = 0;
	unsigned int gb = 0;
	unsigned char *p = (unsigned char*)dst;

	for (block = 0; block < height; block += 4) {
		for (i = 0; i < width; i += 4) {
			/* Alpha and Red */
			for (c = 0; c < 4; ++c) {
				for (ar = 0; ar < 4; ++ar) {
					/* Alpha pixels */
					*p++ = src[(((i + ar) + ((block + c) * width)) * 4) + 3];
					/* Red pixels */	
					*p++ = src[((i + ar) + ((block + c) * width)) * 4];
				}
			}
			
			/* Green and Blue */
			for (c = 0; c < 4; ++c) {
				for (gb = 0; gb < 4; ++gb) {
					/* Green pixels */
					*p++ = src[(((i + gb) + ((block + c) * width)) * 4) + 1];
					/* Blue pixels */
					*p++ = src[(((i + gb) + ((block + c) * width)) * 4) + 2];
				}
			}
		} /* i */
	} /* block */
}

/**** FREETYPE END ****/
