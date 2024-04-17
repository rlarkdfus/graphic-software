#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"


class TexShader : public GShader {
public:
  TexShader(const GMatrix& localMatrix, GShader* realShader) : localMatrix(localMatrix), realShader(realShader){}

  bool isOpaque() override {
    return realShader->isOpaque();
  }

  bool setContext(const GMatrix& ctm) override {
    return realShader->setContext(ctm*localMatrix);
  }

  void shadeRow(int x, int y, int count, GPixel row[]) override {
    return realShader->shadeRow(x, y, count, row);
  }

private:
  const GMatrix localMatrix;
  GShader* realShader;
};
