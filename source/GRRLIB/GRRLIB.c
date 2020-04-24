/*===========================================
        GRRLIB (GX version) 3.0.1 alpha
        Code     : NoNameNo
        GX hints : RedShade
===========================================*/

#include "GRRLIB.h"

/******************************************************************************/
/**** FREETYPE START ****/
/* This is a very rough implementation if freetype using GRRLIB */
#include "fonts/ttf_font.h" /* A truetype font converted with raw2c */
#include <ft2build.h> /* I presume you have freetype for the Wii installed */
#include FT_FREETYPE_H

static FT_Library ftLibrary;
static FT_Face ftFace;

/* Static function prototypes */
static void DrawText(const char *string, unsigned int fontSize, void *buffer);
static bool BlitGlyph(FT_Bitmap *bitmap, int offset, int top, void *buffer);
static void BitmapTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int width, const unsigned int height);

extern void GRRLIB_InitFreetype(void) {
	unsigned int error = FT_Init_FreeType(&ftLibrary);
	if (error) {
		exit(0);
	}

	error = FT_New_Memory_Face(ftLibrary, ttf_font, ttf_font_size, 0, &ftFace);
	/* Note: You could also directly load a font from the SD card like this:
	error = FT_New_Face(ftLibrary, "fat3:/apps/myapp/font.ttf", 0, &ftFace); */
	
	if (error == FT_Err_Unknown_File_Format) {
		exit(0);
	} else if (error) {
		/* Some other error */
		exit(0);
	}
}

/* Render the text string to a 4x4RGBA texture, return a pointer to this texture */
extern void *GRRLIB_TextToTexture(const char *string, unsigned int fontSize, unsigned int fontColour) {
	unsigned int error = FT_Set_Pixel_Sizes(ftFace, 0, fontSize);
	if (error) {
		/* Failed to set the font size to the requested size. 
		 * You probably should set a default size or something. 
		 * I'll leave that up to the reader. */
	}

	/* Set the font colour, alpha is determined when we blit the glyphs */
	fontColour = fontColour << 8;

	/* Create a tempory buffer, 640 pixels wide, for freetype draw the glyphs */
	void *tempTextureBuffer = (void*) malloc(640 * (fontSize*2) * 4);
	if (tempTextureBuffer == NULL) {
		/* Oops! Something went wrong! */
		exit(0);
	}
	
	/* Set the RGB pixels in tempTextureBuffer to the requested colour */
	unsigned int *p = tempTextureBuffer;
	unsigned int loop = 0;
	for (loop = 0; loop < (640 * (fontSize*2)); ++loop) {
		*(p++) = fontColour;	
	}

	/* Render the glyphs on to the temp buffer */
	DrawText(string, fontSize, tempTextureBuffer);

	/* Create a new buffer, this time to hold the final texture 
	 * in a format suitable for the Wii */
	void *texture = memalign(32, 640 * (fontSize*2) * 4);

	/* Convert the RGBA temp buffer to a format usuable by GX */
	BitmapTo4x4RGBA(tempTextureBuffer, texture, 640, (fontSize*2));
	DCFlushRange(texture, 640 * (fontSize*2) * 4);

	/* The temp buffer is no longer required */
	free(tempTextureBuffer);

	return texture;
}

static void DrawText(const char *string, unsigned int fontSize, void *buffer) {
	unsigned int error = 0;
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
	
		error = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_RENDER);
		if (error) {
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
/******************************************************************************/

#define DEFAULT_FIFO_SIZE (256 * 1024)

 u32 fb=0;
 static void *xfb[2] = { NULL, NULL};
 GXRModeObj *rmode;
 void *gp_fifo = NULL;


inline void GRRLIB_FillScreen(u32 color){
	GRRLIB_Rectangle(-40, -40, 680,520, color, 1);
}

inline void GRRLIB_Plot(f32 x,f32 y, u32 color){
   guVector  v[]={{x,y,0.0f}};
   GXColor c[]={GRRLIB_Splitu32(color)};
	
	GRRLIB_NPlot(v,c,1);
}
void GRRLIB_NPlot(guVector v[],GXColor c[],long n){
	GRRLIB_GXEngine(v,c,n,GX_POINTS);
}

inline void GRRLIB_Line(f32 x1, f32 y1, f32 x2, f32 y2, u32 color){
   guVector  v[]={{x1,y1,0.0f},{x2,y2,0.0f}};
   GXColor col = GRRLIB_Splitu32(color);
   GXColor c[]={col,col};

	GRRLIB_NGone(v,c,2);
}

inline void GRRLIB_Rectangle(f32 x, f32 y, f32 width, f32 height, u32 color, u8 filled){
   guVector  v[]={{x,y,0.0f},{x+width,y,0.0f},{x+width,y+height,0.0f},{x,y+height,0.0f},{x,y,0.0f}};
   GXColor col = GRRLIB_Splitu32(color);
   GXColor c[]={col,col,col,col,col};

	if(!filled){
		GRRLIB_NGone(v,c,5);
	}
	else{
		GRRLIB_NGoneFilled(v,c,4);
	}
}
void GRRLIB_NGone(guVector v[],GXColor c[],long n){
	GRRLIB_GXEngine(v,c,n,GX_LINESTRIP);
}
void GRRLIB_NGoneFilled(guVector v[],GXColor c[],long n){
	GRRLIB_GXEngine(v,c,n,GX_TRIANGLEFAN);
}


// By grillo (http://grrlib.santo.fr/forum/viewtopic.php?id=32)
u8 * GRRLIB_LoadTextureFromFile(const char *filename) {

   PNGUPROP imgProp;
   IMGCTX ctx;
   void *my_texture;
	
	ctx = PNGU_SelectImageFromDevice(filename);
        PNGU_GetImageProperties (ctx, &imgProp);
        my_texture = memalign (32, imgProp.imgWidth * imgProp.imgHeight * 4);
        PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, my_texture, 255);
        PNGU_ReleaseImageContext (ctx);
        DCFlushRange (my_texture, imgProp.imgWidth * imgProp.imgHeight * 4);
    return my_texture;
}

u8 * GRRLIB_LoadTexture(const unsigned char my_png[]) {
   PNGUPROP imgProp;
   IMGCTX ctx;
   void *my_texture;

   	ctx = PNGU_SelectImageFromBuffer(my_png);
        PNGU_GetImageProperties (ctx, &imgProp);
        my_texture = memalign (32, imgProp.imgWidth * imgProp.imgHeight * 4);
        PNGU_DecodeTo4x4RGBA8 (ctx, imgProp.imgWidth, imgProp.imgHeight, my_texture, 255);
        PNGU_ReleaseImageContext (ctx);
        DCFlushRange (my_texture, imgProp.imgWidth * imgProp.imgHeight * 4);
	return my_texture;
}

inline void GRRLIB_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], float degrees, float scaleX, f32 scaleY, u8 alpha ){
   GXTexObj texObj;

	
	GX_InitTexObj(&texObj, data, width,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	//GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	Mtx m,m1,m2, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	guVector axis =(guVector) {0 , 0, 1 };
	guMtxRotAxisDeg (m2, &axis, degrees);
	guMtxConcat(m2,m1,m);

	guMtxTransApply(m,m, xpos+width,ypos+height,0);
	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);
	
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
  	GX_Position3f32(-width, -height,  0);
  	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(0, 0);
  
  	GX_Position3f32(width, -height,  0);
 	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(1, 0);
  
  	GX_Position3f32(width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(1, 1);
  
  	GX_Position3f32(-width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(0, 1);
	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

}

inline void GRRLIB_DrawTile(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], float degrees, float scaleX, f32 scaleY, u8 alpha, f32 frame,f32 maxframe ){
GXTexObj texObj;
f32 s1= frame/maxframe;
f32 s2= (frame+1)/maxframe;
f32 t1=0;
f32 t2=1;
	
	GX_InitTexObj(&texObj, data, width*maxframe,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	Mtx m,m1,m2, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	guVector axis =(guVector) {0 , 0, 1 };
	guMtxRotAxisDeg (m2, &axis, degrees);
	guMtxConcat(m2,m1,m);
	guMtxTransApply(m,m, xpos+width,ypos+height,0);
	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
  	GX_Position3f32(-width, -height,  0);
  	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s1, t1);
  
  	GX_Position3f32(width, -height,  0);
 	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s2, t1);
  
  	GX_Position3f32(width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s2, t2);
  
  	GX_Position3f32(-width, height,  0);
	GX_Color4u8(0xFF,0xFF,0xFF,alpha);
  	GX_TexCoord2f32(s1, t2);
	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

}

inline void GRRLIB_DrawChar(f32 xpos, f32 ypos, u16 width, u16 height, u8 data[], float degrees, float scaleX, f32 scaleY, f32 frame,f32 maxframe, GXColor c ){
GXTexObj texObj;
f32 s1= frame/maxframe;
f32 s2= (frame+1)/maxframe;
f32 t1=0;
f32 t2=1;
	
	GX_InitTexObj(&texObj, data, width*maxframe,height, GX_TF_RGBA8,GX_CLAMP, GX_CLAMP,GX_FALSE);
	GX_InitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_MODULATE);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);

	Mtx m,m1,m2, mv;
	width *=.5;
	height*=.5;
	guMtxIdentity (m1);
	guMtxScaleApply(m1,m1,scaleX,scaleY,1.0);
	guVector axis =(guVector) {0 , 0, 1 };
	guMtxRotAxisDeg (m2, &axis, degrees);
	guMtxConcat(m2,m1,m);
	guMtxTransApply(m,m, xpos+width,ypos+height,0);
	guMtxConcat (GXmodelView2D, m, mv);
	GX_LoadPosMtxImm (mv, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0,4);
  	GX_Position3f32(-width, -height,  0);
  	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s1, t1);
  
  	GX_Position3f32(width, -height,  0);
 	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s2, t1);
  
  	GX_Position3f32(width, height,  0);
	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s2, t2);
  
  	GX_Position3f32(-width, height,  0);
	GX_Color4u8(c.r,c.g,c.b,c.a);
  	GX_TexCoord2f32(s1, t2);
	GX_End();
	GX_LoadPosMtxImm (GXmodelView2D, GX_PNMTX0);

	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
  	GX_SetVtxDesc (GX_VA_TEX0, GX_NONE);

}

void GRRLIB_Printf(f32 xpos, f32 ypos, u8 data[], u32 color, f32 zoom, char *text,...){
int i ;
char tmp[1024];
int size=0;

va_list argp;
va_start(argp, text);
vsprintf(tmp, text, argp);
va_end(argp);

size = strlen(tmp);
GXColor col = GRRLIB_Splitu32(color);
	for(i=0;i<strlen(tmp);i++){
		u8 c = tmp[i];
		GRRLIB_DrawChar(xpos+i*8*zoom, ypos, 8, 8, data, 0, zoom, zoom, c,128, col );
	}
}

void GRRLIB_GXEngine(guVector v[], GXColor c[], long n,u8 fmt){
   int i=0;	

	GX_Begin(fmt, GX_VTXFMT0,n);
	for(i=0;i<n;i++){
  		GX_Position3f32(v[i].x, v[i].y,  v[i].z);
  		GX_Color4u8(c[i].r, c[i].g, c[i].b, c[i].a);
  	}
	GX_End();
}

GXColor GRRLIB_Splitu32(u32 color){
   u8 a,r,g,b;

	a = (color >> 24) & 0xFF; 
	r = (color >> 16) & 0xFF; 
	g = (color >> 8) & 0xFF; 
	b = (color) & 0xFF; 

	return (GXColor){r,g,b,a};
}




//********************************************************************************************

void GRRLIB_InitVideo () {

    f32 yscale;
    u32 xfbHeight;
    Mtx perspective;

    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
	
	/* Widescreen patch by CashMan's Productions (http://www.CashMan-Productions.fr.nf) */
	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
		rmode->viWidth = 678;
		rmode->viXOrigin = (VI_MAX_WIDTH_NTSC - 678)/2;
	}
	
    xfb[0] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    xfb[1] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    VIDEO_Configure (rmode);
    VIDEO_SetNextFramebuffer(xfb[fb]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
    fb ^= 1;

    gp_fifo = (u8 *) memalign(32,DEFAULT_FIFO_SIZE);
    memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);
    GX_Init (gp_fifo, DEFAULT_FIFO_SIZE);
    GXColor background = {0x00, 0x00, 0x00, 0xff};
    GX_SetCopyClear(background, 0x00ffffff);

    // other gx setup
    yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
    GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
    GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

    if (rmode->aa) {
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    } else {
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(xfb[0],GX_TRUE);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc (GX_VA_CLR0, GX_DIRECT);
    
    GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_TRUE);
    
    GX_InvVtxCache ();

    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
    
    GX_SetNumTexGens(1);
    GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    
    GX_InvalidateTexAll();

    guMtxIdentity(GXmodelView2D);
    guMtxTransApply (GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -50.0F);
    GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

    guOrtho(perspective,0,479,0,639,0,300);
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_SetCullMode(GX_CULL_NONE);
    
}


/* OLD
void GRRLIB_InitVideo () {

	rmode = VIDEO_GetPreferredMode(NULL);
	VIDEO_Configure (rmode);
	xfb[0] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	xfb[1] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_SetNextFramebuffer(xfb[fb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


	gp_fifo = (u8 *) memalign(32,DEFAULT_FIFO_SIZE);
}

void GRRLIB_Start(){
   
   f32 yscale;
   u32 xfbHeight;
   Mtx perspective;

	GX_Init (gp_fifo, DEFAULT_FIFO_SIZE);

	// clears the bg to color and clears the z buffer
	GXColor background = { 0, 0, 0, 0xff };
	GX_SetCopyClear (background, 0x00ffffff);

	// other gx setup
	yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

	if (rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetDispCopyGamma(GX_GM_1_0);
 

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
		GX_InvVtxCache ();
		GX_InvalidateTexAll();

	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc (GX_VA_CLR0, GX_DIRECT);


		GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GX_SetZMode (GX_FALSE, GX_LEQUAL, GX_TRUE);

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOp (GX_TEVSTAGE0, GX_PASSCLR);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	guMtxIdentity(GXmodelView2D);
	guMtxTransApply (GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -50.0F);
	GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);

	guOrtho(perspective,0,479,0,639,0,300);
	GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	
	GX_SetCullMode(GX_CULL_NONE);
}*/

void GRRLIB_Render () {
        GX_DrawDone ();

	fb ^= 1;		// flip framebuffer
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(xfb[fb],GX_TRUE);
	VIDEO_SetNextFramebuffer(xfb[fb]);
 	VIDEO_Flush();
 	VIDEO_WaitVSync();

}

void GRRLIB_Stop() {

    GX_AbortFrame();
    GX_Flush();

    free(MEM_K1_TO_K0(xfb[0])); xfb[0] = NULL;
    free(MEM_K1_TO_K0(xfb[1])); xfb[1] = NULL;
    free(gp_fifo); gp_fifo = NULL;

}

