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
      row[i] = crow[i]*trow[i];
    }
  }

private:
  GShader* triColorShader;
  GShader* texShader;
};

