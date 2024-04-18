#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"

#include <iostream>
#include <vector>

class TexColorShader : public GShader {
public:
  TexColorShader(GShader* triColorShader, GShader* texShader) : 
    triColorShader(triColorShader),
    texShader(texShader) {}

  bool isOpaque() override {
    return triColorShader->isOpaque() && texShader->isOpaque();
  }

  bool setContext(const GMatrix& ctm) override {
    return triColorShader->setContext(ctm) && texShader->setContext(ctm);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override {
    GPixel crow[count], trow[count];
    triColorShader->shadeRow(x, y, count, crow);
    texShader->shadeRow(x,y,count,trow);

    for(int i = 0; i < count; i++) {
      int a = GPixel_GetA(crow[i])*GPixel_GetA(trow[i]) / 255;
      int r = GPixel_GetR(crow[i])*GPixel_GetR(trow[i]) / 255;
      int g = GPixel_GetG(crow[i])*GPixel_GetG(trow[i]) / 255;
      int b = GPixel_GetB(crow[i])*GPixel_GetB(trow[i]) / 255;

      row[i] = GPixel_PackARGB(a, r, g, b);
    }
  }

private:
  GShader* triColorShader;
  GShader* texShader;
};

