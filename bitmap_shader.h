#ifndef _g_shader_h_
#define _g_shader_h_

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"

class BitmapShader : public GShader {
public:
  BitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode) : bm(bitmap), lm(localMatrix), tileMode(tileMode) {}

  bool isOpaque() override;
  bool setContext(const GMatrix& ctm) override;
  void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
  const GBitmap bm;
  const GMatrix lm;
  GMatrix inverted;
  GTileMode tileMode;
};

#endif
