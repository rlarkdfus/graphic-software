#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"

class TriColorShader : public GShader {
public:
  TriColorShader(const GMatrix& localMatrix, const GColor colors[]) : localMatrix(localMatrix) {
    for(int i = 0; i < 3; i++) this->colors[i] = colors[i];
  }

  bool isOpaque() override;
  bool setContext(const GMatrix& ctm) override;
  void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
  const GMatrix localMatrix;
  GMatrix inv;
  GColor colors[3];
};
