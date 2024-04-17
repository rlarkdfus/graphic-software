#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "blender.h"
#include "gradient_shader.h"

#include <algorithm>
#include <iostream>

using namespace std;

bool GradientShader::isOpaque() {
  for(int i = 0; i < cnt; i++) {
    if(colors[i].a < 1) return false;
  }
  return true;
}

bool GradientShader::setContext(const GMatrix& ctm) {
  std::optional<GMatrix> invLocal = GMatrix::Concat(ctm,localMatrix).invert();
  if(!invLocal.has_value()) {
    return false;
  }
  inv = *invLocal;
  return true;
}

float applyTileMode(GTileMode tileMode, float n) {
  if(tileMode == GTileMode::kClamp) {
    return max(0.0f, min(0.99999999f, n));
  } else if(tileMode == GTileMode::kRepeat) {
    return n - (int)n;
  } else if(tileMode == GTileMode::kMirror) {
    float x = n - (int)n;
    if((int)n%2 == 0) return x;
    else return 1.0f-x;
  }
  return 0;
}
  

void GradientShader::shadeRow(int x, int y, int count, GPixel row[]) {
  GPoint init = {x+0.5, y+0.5};
  GPoint coord = inv*init;

  if(cnt == 1) {
    for(int i = 0; i < count; i++) {
      row[i] = premul(colors[0]);
    }
  } else if(cnt == 2) {
    float xx = coord.x;
    for(int i = 0; i < count; i++) {
      float t = applyTileMode(tileMode, xx)*(cnt-1.0f);
      GColor c = (1.0f-t)*colors[0] + t*colors[1];
      row[i] = premul(c);
      xx += inv[0];
    }
  } else {
    float xx = coord.x;
    for(int i = 0; i < count; i++) {
      float xxx = applyTileMode(tileMode, xx);
      int k = (int)(xxx*(cnt-1.0f));
      float t = xxx*(cnt-1.0f) - k;

      GColor c = (1.0f-t)*colors[k] + t*colors[k+1];
      row[i] = premul(c);
      xx += inv[0];
    }
  }
  
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode) {
  if(count < 1) return nullptr;

  float dx = p1.x - p0.x;
  float dy = p1.y - p0.y;
  GMatrix localMatrix = GMatrix(
      dx, -dy, p0.x,
      dy,  dx, p0.y);

  return std::unique_ptr<GShader>(new GradientShader(localMatrix, colors, count, tileMode));
}
