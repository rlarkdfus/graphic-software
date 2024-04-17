#include "include/GMatrix.h"
#include <math.h>

GMatrix::GMatrix() {
  fMat[0] = 1;
  fMat[1] = 0;
  fMat[2] = 0;
  fMat[3] = 1;
  fMat[4] = 0;
  fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty) {
  return GMatrix(
      1, 0, tx, 
      0, 1, ty
      );
}

GMatrix GMatrix::Scale(float sx, float sy) {
  return GMatrix(
      sx, 0, 0, 
      0, sy, 0
      );
}

GMatrix GMatrix::Rotate(float radians) {
  double c = cos(radians);
  double s = sin(radians);
  return GMatrix(
      c, -s, 0, 
      s, c, 0
      );
}

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
  float a0 = a[0]*b[0] + a[2]*b[1];
  float b0 = a[1]*b[0] + a[3]*b[1];
  float c0 = a[0]*b[2] + a[2]*b[3];
  float d0 = a[1]*b[2] + a[3]*b[3];
  float e0 = a[0]*b[4] + a[2]*b[5] + a[4];
  float f0 = a[1]*b[4] + a[3]*b[5] + a[5];

  return GMatrix(
      a0, c0, e0, 
      b0, d0, f0
      );
}

std::optional<GMatrix> GMatrix::invert() const {
  float a = fMat[0];
  float b = fMat[1];
  float c = fMat[2];
  float d = fMat[3];
  float e = fMat[4];
  float f = fMat[5];
  float det = a*d - b*c;

  if(det == 0) {
    return {};
  }

  det = 1/det;

  float inv[6];
  inv[0] = d*det;
  inv[1] = -b*det;
  inv[2] = -c*det;
  inv[3] = a*det;
  inv[4] = (c*f-d*e)*det;
  inv[5] = (b*e-a*f)*det;

  return GMatrix(
      inv[0], inv[2], inv[4], 
      inv[1], inv[3], inv[5]
      );
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
  for(int i = 0; i < count; i++) {
    float x = src[i].x;
    float y = src[i].y;

    dst[i].x = fMat[0]*x + fMat[2]*y + fMat[4];
    dst[i].y = fMat[1]*x + fMat[3]*y + fMat[5];
  }
}


