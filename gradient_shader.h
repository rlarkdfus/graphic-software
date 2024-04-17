#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"

#include <iostream>
#include <vector>

class GradientShader : public GShader {
public:
  GradientShader(const GMatrix& localMatrix, const GColor colors[], int count, GTileMode tileMode) : 
    localMatrix(localMatrix),
    cnt(count),
    tileMode(tileMode){
      for(int i = 0; i < count; i++) {
        this->colors.push_back(colors[i]);
      }
    }

  bool isOpaque() override;
  bool setContext(const GMatrix& ctm) override;
  void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
  GMatrix localMatrix;
  GMatrix inv;
  int cnt;
  GTileMode tileMode;
  std::vector<GColor> colors;
};
