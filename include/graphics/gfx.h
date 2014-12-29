#pragma once
#include <3ds.h>
#include "font.h"

typedef float (*gfxWaveCallback)(void* p, u16 x);

typedef struct GfxObj_s {
	u8 *buffer;
	u16 wd, ht;

} GfxObj_t;



//rendering stuff
void gfxFadeScreen(gfxScreen_t screen, gfx3dSide_t side, u32 f);
void gfxDrawSprite(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawDualSprite(u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlpha(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlphaBlend(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlphaBlendFade(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y, u8 fadeValue);
void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, font_s* f, char* str, s16 x, s16 y);
void gfxDrawTextN(gfxScreen_t screen, gfx3dSide_t side, font_s* f, char* str, u16 length, s16 x, s16 y);
void gfxFillColor(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3]);
void gfxFillColorGradient(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColorStart[3], u8 rgbColorEnd[3]);
void gfxDrawRectangle(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3], s16 x, s16 y, u16 width, u16 height);
void gfxDrawWave(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColorStart[3], u8 rgbColorEnd[3], u16 level, u16 amplitude, u16 width, gfxWaveCallback cb, void* p);


//better api
extern void GfxObj_Render(GfxObj_t *dest, GfxObj_t *gfx, s16 x, s16 y);
extern void GfxObj_RenderAlpha(GfxObj_t *dest, GfxObj_t *gfx, s16 x, s16 y);
extern void GfxObj_Fill(GfxObj_t *gfx, u8 rgbColor[3]);
extern void GfxObj_DrawPix(GfxObj_t *gfx, s16 x, s16 y, u8 rgbColor[3]);
extern void GfxObj_Draw(gfxScreen_t screen, gfx3dSide_t side, GfxObj_t *gfx, s16 x, s16 y);
extern u8 GfxObj_CreateBlank(GfxObj_t *gfx, s16 width, s16 height);
extern void GfxObj_Delete(GfxObj_t *gfx);
