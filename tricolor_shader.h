#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"

class TriColorShader : public GShader {
public:
  TriColorShader(const GMatrix& localMatrix, const GColor colors[]) : localMatrix(localMatrix) {
    for(int i = 0; i < 3; i++) this->colors[i] = colors[i];
  }

bool isOpaque() override {
  for(int i = 0; i < 3; i++) {
    if(colors[i].a < 1) return false;
  }
  return true;
}

bool setContext(const GMatrix& ctm) override {
  std::optional<GMatrix> invLocal = GMatrix::Concat(ctm,localMatrix).invert();
  if(!invLocal.has_value()) {
    return false;
  }
  inv = *invLocal;
  return true;
}

void shadeRow(int x, int y, int count, GPixel row[]) override {
  GPoint init = {x+0.5, y+0.5};
  GPoint coord = inv*init;

  GColor dc1 = colors[1] - colors[0];
  GColor dc2 = colors[2] - colors[0];
  GColor dc = inv[0]*dc1 + inv[1]*dc2;

  GColor c = coord.x*dc1 + coord.y*dc2 + colors[0];

  for(int i = 0; i < count; i++) {
    row[i] = premul(c);
    c += dc;
  }
}

private:
  const GMatrix localMatrix;
  GMatrix inv;
  GColor colors[3];
};
