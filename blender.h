#include<iostream>
using namespace std;
static inline GPixel premul(const GColor& color) {
  int a = GRoundToInt(255.0f*color.a);
  int r = GRoundToInt(a*color.r);
  int g = GRoundToInt(a*color.g);
  int b = GRoundToInt(a*color.b);
  return GPixel_PackARGB(a, r, g, b);
}

static inline void bClear(GPixel src, GPixel *dst) {
  *dst = GPixel_PackARGB(0, 0, 0, 0);
}

static inline void bSrc(GPixel src, GPixel *dst) {
  *dst = src;
}

static inline void bDst(GPixel src, GPixel *dst) {
  return;
}

static inline void bSrcOver(GPixel src, GPixel *dst) {
  float srcA_inv = (255.0f - GPixel_GetA(src)) / 255.0f;
  int a = GPixel_GetA(src) + (int)(srcA_inv*GPixel_GetA(*dst) + 0.5f);
  int r = GPixel_GetR(src) + (int)(srcA_inv*GPixel_GetR(*dst) + 0.5f);
  int g = GPixel_GetG(src) + (int)(srcA_inv*GPixel_GetG(*dst) + 0.5f);
  int b = GPixel_GetB(src) + (int)(srcA_inv*GPixel_GetB(*dst) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bDstOver(GPixel src, GPixel *dst) {
  float dstA_inv = (255.0f - GPixel_GetA(*dst)) / 255.0f;
  int a = GPixel_GetA(*dst) + (int)(dstA_inv*GPixel_GetA(src) + 0.5f);
  int r = GPixel_GetR(*dst) + (int)(dstA_inv*GPixel_GetR(src) + 0.5f);
  int g = GPixel_GetG(*dst) + (int)(dstA_inv*GPixel_GetG(src) + 0.5f);
  int b = GPixel_GetB(*dst) + (int)(dstA_inv*GPixel_GetB(src) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bSrcIn(GPixel src, GPixel *dst) {
  float dstA = GPixel_GetA(*dst) / 255.0f;
  int a = (int)(dstA*GPixel_GetA(src) + 0.5f);
  int r = (int)(dstA*GPixel_GetR(src) + 0.5f);
  int g = (int)(dstA*GPixel_GetG(src) + 0.5f);
  int b = (int)(dstA*GPixel_GetB(src) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bDstIn(GPixel src, GPixel *dst) {
  float srcA = GPixel_GetA(src) / 255.0f;
  int a = (int)(srcA*GPixel_GetA(*dst) + 0.5f);
  int r = (int)(srcA*GPixel_GetR(*dst) + 0.5f);
  int g = (int)(srcA*GPixel_GetG(*dst) + 0.5f);
  int b = (int)(srcA*GPixel_GetB(*dst) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bSrcOut(GPixel src, GPixel *dst) {
  float dstA_inv = (255.0f - GPixel_GetA(*dst)) / 255.0f;
  int a = (int)(dstA_inv*GPixel_GetA(src) + 0.5f);
  int r = (int)(dstA_inv*GPixel_GetR(src) + 0.5f);
  int g = (int)(dstA_inv*GPixel_GetG(src) + 0.5f);
  int b = (int)(dstA_inv*GPixel_GetB(src) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bDstOut(GPixel src, GPixel *dst) {
  float srcA_inv = (255.0f - GPixel_GetA(src)) / 255.0f;
  int a = (int)(srcA_inv*GPixel_GetA(*dst) + 0.5f);
  int r = (int)(srcA_inv*GPixel_GetR(*dst) + 0.5f);
  int g = (int)(srcA_inv*GPixel_GetG(*dst) + 0.5f);
  int b = (int)(srcA_inv*GPixel_GetB(*dst) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bSrcATop(GPixel src, GPixel *dst) {
  float dstA = GPixel_GetA(*dst) / 255.0f;
  float srcA_inv = 1.0f - GPixel_GetA(src) / 255.0f;
  int a = (int)(dstA * GPixel_GetA(src) + srcA_inv * GPixel_GetA(*dst) + 0.5f);
  int r = (int)(dstA * GPixel_GetR(src) + srcA_inv * GPixel_GetR(*dst) + 0.5f);
  int g = (int)(dstA * GPixel_GetG(src) + srcA_inv * GPixel_GetG(*dst) + 0.5f);
  int b = (int)(dstA * GPixel_GetB(src) + srcA_inv * GPixel_GetB(*dst) + 0.5f);

  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bDstATop(GPixel src, GPixel *dst) {
  float srcA = GPixel_GetA(src) / 255.0f;
  float dstA_inv = (255.0f - GPixel_GetA(*dst)) / 255;
  int a = (int)(srcA*GPixel_GetA(*dst) + dstA_inv*GPixel_GetA(src) + 0.5f);
  int r = (int)(srcA*GPixel_GetR(*dst) + dstA_inv*GPixel_GetR(src) + 0.5f);
  int g = (int)(srcA*GPixel_GetG(*dst) + dstA_inv*GPixel_GetG(src) + 0.5f);
  int b = (int)(srcA*GPixel_GetB(*dst) + dstA_inv*GPixel_GetB(src) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

static inline void bXor(GPixel src, GPixel *dst) {
  float srcA_inv = (255.0f - GPixel_GetA(src)) / 255.0f;
  float dstA_inv = (255.0f - GPixel_GetA(*dst)) / 255.0f;
  int a = (int)(srcA_inv*GPixel_GetA(*dst) + dstA_inv*GPixel_GetA(src) + 0.5f);
  int r = (int)(srcA_inv*GPixel_GetR(*dst) + dstA_inv*GPixel_GetR(src) + 0.5f);
  int g = (int)(srcA_inv*GPixel_GetG(*dst) + dstA_inv*GPixel_GetG(src) + 0.5f);
  int b = (int)(srcA_inv*GPixel_GetB(*dst) + dstA_inv*GPixel_GetB(src) + 0.5f);
  *dst = GPixel_PackARGB(a, r, g, b);
}

