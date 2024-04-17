#include "include/GPath.h"

#include <algorithm>
#include <iostream>

using namespace std;

void GPath::addRect(const GRect& rect, Direction dir) {
  this->moveTo(rect.left, rect.top);

  if(dir == kCW_Direction) {
    this->lineTo(rect.right, rect.top);
    this->lineTo(rect.right, rect.bottom);
    this->lineTo(rect.left, rect.bottom);
  } else {
    this->lineTo(rect.left, rect.bottom);
    this->lineTo(rect.right, rect.bottom);
    this->lineTo(rect.right, rect.top);
  }
}

void GPath::addPolygon(const GPoint pts[], int count) {
  if(count < 1) return;

  this-> moveTo(pts[0]);
  for(int i = 1; i < count; i++) {
    this->lineTo(pts[i]);
  }
}

void GPath::addCircle(GPoint center, float radius, Direction dir) {
  float a = 0.551915;

  vector<GPoint> pts;
  pts.push_back({1,0});
  pts.push_back({1,a});
  pts.push_back({a,1});
  pts.push_back({0,1});
  pts.push_back({-a,1});
  pts.push_back({-1,a});
  pts.push_back({-1,0});
  pts.push_back({-1,-a});
  pts.push_back({-a,-1});
  pts.push_back({0,-1});
  pts.push_back({a,-1});
  pts.push_back({1,-a});
  pts.push_back({1,0});
  GMatrix mx = GMatrix::Translate(center.x, center.y)*GMatrix::Scale(radius, radius);

  this->moveTo(center.x + radius, center.y);
  if(dir == kCW_Direction) {
    for(int i = 0; i < 4; i++) {
      this->cubicTo(
          mx*pts[3*i + 1],
          mx*pts[3*i + 2],
          mx*pts[3*i + 3]);
    }
  } else {
    for(int i = 3; i >= 0; i--) {
      this->cubicTo(
          mx*pts[3*i + 2],
          mx*pts[3*i + 1],
          mx*pts[3*i + 0]);
    }
  }
}

float solve2(float A, float B, float C) {
  float numerator = A - B;
  float denominator = A + C - 2*B;
  if(denominator == 0) {
    return -1;
  }

  return numerator / denominator;
}

vector<float> solve3(float A, float B, float C, float D) {
  vector<float> roots;
  float a = 3.0f*D - 3.0f*A + 9.0f*B - 9.0f*C;
  float b = 6.0f*A - 12.0f*B + 6.0f*C;
  float c = -3.0f*A + 3.0f*B;

  float d = b*b - a*c*4.0f;

  if(a == 0) {
    if(b != 0) {
      roots.push_back(-c/b);
    }
  } else {
    if(d == 0) {
      roots.push_back(-b/(a*2.0f));
    }
    else if(d > 0) {
      float t1 = (-b + sqrtf(d))/(2.0f*a);
      float t2 = (-b - sqrtf(d))/(2.0f*a);
      roots.push_back(t1);
      roots.push_back(t2);
    }
  }

  return roots;
}

float calc2(float A, float B, float C, float t) {
  float tt = 1 - t;
  return A*tt*tt + 2*B*tt*t + C*t*t;
}

float calc3(float A, float B, float C, float D, float t) {
  float tt = 1-t;
  return A*tt*tt*tt + B*t*tt*tt*3 + C*t*t*tt*3 + D*t*t*t;
}

GRect GPath::bounds() const {
  if(countPoints() == 0) {
    return GRect::LTRB(0, 0, 0, 0);
  }

  float left = 1e9, right = 0, top = 1e9, bottom = 0;

  GPoint pts[kMaxNextPoints];
  Edger edger(*this);
  while(auto v = edger.next(pts)) {
    if(v.value() == Verb::kLine) {
      for(int i = 0; i < 2; i++) {
        left = min(left, pts[i].x);
        top = min(top, pts[i].y);
        right = max(right, pts[i].x);
        bottom = max(bottom, pts[i].y);
      }
    } else if(v.value() == Verb::kQuad) {
      left = min(left, min(pts[0].x, pts[2].x));
      right = max(right, max(pts[0].x, pts[2].x));
      top = min(top, min(pts[0].y, pts[2].y));
      bottom = max(bottom , max(pts[0].y, pts[2].y));

      float xt = solve2(pts[0].x, pts[1].x, pts[2].x);
      float yt = solve2(pts[0].y, pts[1].y, pts[2].y);

      if(xt > 0 && xt < 1) {
        float x = calc2(pts[0].x, pts[1].x, pts[2].x, xt);
        left = min(left, x);
        right = max(right, x);
      }
      if(yt > 0 && yt < 1) {
        float y = calc2(pts[0].y, pts[1].y, pts[2].y, yt);
        top = min(top, y);
        bottom = max(bottom, y);
      }

    } else if(v.value() == Verb::kCubic) {
      vector<float> xt = solve3(pts[0].x, pts[1].x, pts[2].x, pts[3].x);
      vector<float> yt = solve3(pts[0].y, pts[1].y, pts[2].y, pts[3].y);

      left = min(left, min(pts[0].x, pts[3].x));
      top = min(top, min(pts[0].y, pts[3].y));
      right = max(right, max(pts[0].x, pts[3].x));
      bottom = max(bottom, max(pts[0].y, pts[3].y));

      for(auto t : xt) {
        if(t < 0 || t > 1) continue;
        float x = calc3(pts[0].x, pts[1].x, pts[2].x, pts[3].x, t);
        left = min(left, x);
        right = max(right, x);
      }
      for(auto t : yt) {
        if(t < 0 || t > 1) continue;
        float y = calc3(pts[0].y, pts[1].y, pts[2].y, pts[3].y, t);
        top = min(top, y);
        bottom = max(bottom, y);
      }
    }

  }

  return GRect::LTRB(left, top, right, bottom);
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
  GPoint a = src[0];
  GPoint b = src[1];
  GPoint c = src[2];

  GPoint ab = (1-t)*a + t*b;
  GPoint bc = (1-t)*b + t*c;
  GPoint p = (1-t)*ab + t*bc;

  dst[0] = a;
  dst[1] = ab;
  dst[2] = p;
  dst[3] = bc;
  dst[4] = c;
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
  float tt = 1-t;
  GPoint a = src[0];
  GPoint b = src[1];
  GPoint c = src[2];
  GPoint d = src[3];

  GPoint ab = tt*a + t*b;
  GPoint bc = tt*b + t*c;
  GPoint cd = tt*c + t*d;

  GPoint p1 = tt*ab + t*bc;
  GPoint p2 = tt*bc + t*cd;
  GPoint pm = tt*p1 + t*p2;

  dst[0] = a;
  dst[1] = ab;
  dst[2] = p1;
  dst[3] = pm;
  dst[4] = p2;
  dst[5] = cd;
  dst[6] = d;
}

void GPath::transform(const GMatrix& matrix) {
  for(int i = 0; i < fPts.size(); i++) {
    fPts[i] = matrix*fPts[i];
  }
}

