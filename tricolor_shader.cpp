#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "blender.h"
#include "tricolor_shader.h"

using namespace std;

bool TriColorShader::isOpaque() {
  for(int i = 0; i < 3; i++) {
    if(colors[i].a < 1) return false;
  }
  return true;
}

bool TriColorShader::setContext(const GMatrix& ctm) {
  std::optional<GMatrix> invLocal = GMatrix::Concat(ctm,localMatrix).invert();
  if(!invLocal.has_value()) {
    return false;
  }
  inv = *invLocal;
  return true;
}

void TriColorShader::shadeRow(int x, int y, int count, GPixel row[]) {
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
