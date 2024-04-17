#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "bitmap_shader.h"

#include <algorithm>
#include <iostream>

using namespace std;

bool BitmapShader::isOpaque() {
  return bm.isOpaque();
}

bool BitmapShader::setContext(const GMatrix& ctm) {
  std::optional<GMatrix> invertedLocal = GMatrix::Concat(ctm, lm).invert();
  if(invertedLocal.has_value()) {
    inverted = *invertedLocal;
    return true;
  } else {
    return false;
  }
}

int applyTileMode(GTileMode tileMode, float n, int bound) {
  if(tileMode == GTileMode::kClamp) {
    return max(0, min((int)n, bound-1));
  } else if(tileMode == GTileMode::kRepeat) {
    while(n < 0) n += bound;
    return abs((int)(n+0.5))%bound;
  } else if(tileMode == GTileMode::kMirror) {
    int x = abs((int)(n+0.5))%(bound*2);
    if(x > bound-1) x = 2*bound- 1 - x;
    return x;
  }
  return 0;
}

void BitmapShader::shadeRow(int x, int y, int count, GPixel row[]) {
  GPoint init = {x+0.5, y+0.5};
  GPoint coord = inverted*init;

  int width = bm.width();
  int height = bm.height();

  if(inverted[1] == 0) {
    float xx = coord.x;
    int yy = applyTileMode(tileMode, coord.y, height);

    float step = inverted[0];
    for(int i = 0; i < count; i++) {
      row[i] = *bm.getAddr(applyTileMode(tileMode, xx, width), yy);
      xx += step; 

    }
  } else {
    float xx = coord.x;
    float yy = coord.y;

    float stepx = inverted[0];
    float stepy = inverted[1];

    for(int i = 0; i < count; i++) {
      row[i] = *bm.getAddr(applyTileMode(tileMode, xx, width), applyTileMode(tileMode, yy, height));
      xx += stepx;
      yy += stepy;
    }
  }
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode) {
    return std::unique_ptr<GShader>(new BitmapShader(bitmap, localMatrix, tileMode));
}
