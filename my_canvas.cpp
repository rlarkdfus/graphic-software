/*
 *  Copyright 2024 Gordon Kim!
 */

#include "include/GMath.h"
#include "include/GPixel.h"
#include "my_canvas.h"
#include "blender.h"
#include "bitmap_shader.h"
#include "edge.h"
#include <algorithm>
#include <iostream>

using namespace std;

// 0: normal 1: Sa = 0 2: Sa = 1;
const int gProcs[][3] = {
  { 0, 0, 0 },    // bClear
  { 1, 0, 1 },    // bSrc
  { 2, 2, 2 },    // bDst
  { 3, 2, 1 },    // bSrcOver
  { 4, 2, 4 },    // bDstOver
  { 5, 0, 5 },    // bSrcIn
  { 6, 0, 6 },    // bDstIn
  { 7, 0, 7 },    // bSrcOut
  { 8, 2, 0 },    // bDstOut
  { 9, 2, 5 },    // bSrcATop
  { 10, 0, 10 },  // bDstATop
  { 11, 2, 7 }    // bXor
};

void MyCanvas::save() {
  ctmStack.push(ctmStack.top());
}

void MyCanvas::restore() {
  ctmStack.pop();
}

void MyCanvas::concat(const GMatrix& matrix) { 
  GMatrix a = ctmStack.top();
  ctmStack.pop();
  ctmStack.push(GMatrix::Concat(a, matrix));
}

template<typename Proc>
void MyCanvas::fillRow(int x, int y, int count, GPixel row[], Proc blend) {
  for(int i = 0; i < count; i++) {
    GPixel* dst = canvas.getAddr(x+i, y);
    blend(row[i], dst);
  }
}

void MyCanvas::optimizeBlend(int x, int y, int count, const GPaint& paint) {
  GPixel row[count];
  GPixel src = premul(paint.getColor());
  GShader* shader_ptr = paint.getShader();

  int blendMode = (int)paint.getBlendMode();
  float sa = paint.getAlpha();
  if(sa == 0 || sa == 1) sa += 1;


  if(shader_ptr) {
    if(!shader_ptr->setContext(ctmStack.top())) return;
    shader_ptr->shadeRow(x, y, count, row);
    sa = (shader_ptr->isOpaque()) ? 2 : 0;
  } else {
    std::fill(row, row+count, src);
  }

  int blendOptimal = gProcs[blendMode][(int)sa];

  switch (blendOptimal) {
    case 0:
      fillRow(x, y, count, row, bClear);
      break;
    case 1:
      fillRow(x, y, count, row, bSrc);
      break;
    case 2:
      break;
    case 3:
      fillRow(x, y, count, row, bSrcOver);
      break;
    case 4:
      fillRow(x, y, count, row, bDstOver);
      break;
    case 5:
      fillRow(x, y, count, row, bSrcIn);
      break;
    case 6:
      fillRow(x, y, count, row, bDstIn);
      break;
    case 7:
      fillRow(x, y, count, row, bSrcOut);
      break;
    case 8:
      fillRow(x, y, count, row, bDstOut);
      break;
    case 9:
      fillRow(x, y, count, row, bSrcATop);
      break;
    case 10:
      fillRow(x, y, count, row, bDstATop);
      break;
    case 11:
      fillRow(x, y, count, row, bXor);
      break;
  }
}

void MyCanvas::clear(const GColor& color) {
  int width = canvas.width();
  int height = canvas.height();

  GPixel c = premul(color);

  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      GPixel* pixel = canvas.getAddr(x, y);
      *pixel = c;
    }
  }
}


void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
  GMatrix ctm = ctmStack.top();

  float x1 = rect.x();
  float y1 = rect.y();
  float x2 = x1 + rect.width();
  float y2 = y1 + rect.height();

  GPoint rectPoints[4] = {
    { x1, y1 },
    { x2, y1 }, 
    { x2, y2 },
    { x1, y2 }
  };

  if(ctm[1] == 0 && ctm[2] == 0) {
    GPoint lt = ctm*rectPoints[0];
    GPoint rb = ctm*rectPoints[2];

    int x = max(0, GRoundToInt(lt.x));
    int right = min(GRoundToInt(rb.x), canvas.width());
    int count = max(0, right - x);

    int top = max(0, GRoundToInt(lt.y));
    int bottom = min(GRoundToInt(rb.y), canvas.height());

    for(int y = top; y < bottom; y++) {
      optimizeBlend(x, y, count, paint);
    }
  } else {
    drawConvexPolygon(rectPoints, 4, paint);
  }
}


void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
  int boundBottom = canvas.height();
  int boundRight  = canvas.width();

  vector<Edge> edges;
  GMatrix ctm = ctmStack.top();

  GPoint transformed[count];
  ctm.mapPoints(transformed, points, count);
  
  for(int i = 0; i < count; i++) {
    GPoint a = transformed[i];
    GPoint b = transformed[(i+1)%count];

    clipEdge(edges, a, b, boundBottom, boundRight); 
  }

  sort(edges.begin(), edges.end());
  if(edges.size() == 0) return;

  int top = edges.front().top;
  int bottom = edges.back().bottom;
  int ptr1 = 0, ptr2 = 1;
  for(int y = top; y < bottom; y++) {
    if(!edges[ptr1].isValid(y)) {
      ptr1 = max(ptr1, ptr2) + 1;
    }
    if(!edges[ptr2].isValid(y)) {
      ptr2 = max(ptr1, ptr2) + 1;
    }

    int edge1x = GRoundToInt(edges[ptr1].x(y+0.5));
    int edge2x = GRoundToInt(edges[ptr2].x(y+0.5));

    int x = min(edge1x, edge2x);
    int cnt = max(edge1x, edge2x) - x;

    optimizeBlend(x, y, cnt, paint);
  }
}

void MyCanvas::drawPath(const GPath& path, const GPaint& paint) {
  int boundBottom = canvas.height();
  int boundRight  = canvas.width();
  vector<Edge> edges;

  GPath cpath = path;
  cpath.transform(ctmStack.top());
  GPoint pts[GPath::kMaxNextPoints];
  GPath::Edger edger(cpath);

  while(auto verb = edger.next(pts)) {
    if(verb.value() == GPath::Verb::kLine) {
        clipEdge(edges, pts[0], pts[1], boundBottom, boundRight);
    } else if(verb.value() == GPath::Verb::kQuad) {
      GPoint E = pts[0]*0.25f - pts[1]*0.5f + pts[2]*0.25f;
      int num_segs = (int)ceil(sqrtf(4.0f*E.length()));
      GPoint points[num_segs];

      float t = 0, tt = 1, step = 1.0f/num_segs;
      for(int i = 0; i < num_segs; i++) {
        points[i] = pts[0]*tt*tt + 2*pts[1]*t*tt + pts[2]*t*t;

        t += step;
        tt -= step;
      }

      for(int i = 0; i < num_segs-1; i++) {
        GPoint a = points[i];
        GPoint b = points[i+1];
        clipEdge(edges, a, b, boundBottom, boundRight); 
      }
      clipEdge(edges, points[num_segs-1], pts[2], boundBottom, boundRight);
    } else if(verb.value() == GPath::Verb::kCubic) {
      GPoint E0 = pts[0] - 2*pts[1] + pts[2];
      GPoint E1 = pts[1] - 2*pts[2] + pts[3];
      GPoint E = {
        max(abs(E0.x), abs(E1.x)),
        max(abs(E0.y), abs(E1.y))
      };
      int num_segs = (int)ceil(sqrtf(3.0f*E.length()));
      GPoint points[num_segs];

      float t = 0, tt = 1, step = 1.0f/num_segs;
      for(int i = 0; i < num_segs; i++) {
        points[i] = pts[0]*tt*tt*tt + 3*pts[1]*tt*tt*t + 3*pts[2]*tt*t*t + pts[3]*t*t*t;

        t += step;
        tt -= step;
      }

      for(int i = 0; i < num_segs-1; i++) {
        GPoint a = points[i];
        GPoint b = points[i+1];
        clipEdge(edges, a, b, boundBottom, boundRight); 
      }
      clipEdge(edges, points[num_segs-1], pts[3], boundBottom, boundRight);
    }

  }

  if(edges.size() == 0) return;
  sort(edges.begin(), edges.end());

  GRect bound = cpath.bounds();
  int top = GRoundToInt(bound.top);
  int bottom = GRoundToInt(bound.bottom);
  for(int y = top; y < bottom; y++) {
    vector<pair<int, int>> xx;
    int i = 0;
    while(i < edges.size()) {
      if(edges[i].isValid(y)) {
        int x = GRoundToInt(edges[i].x(y+0.5));
        xx.push_back({x, edges[i].direction});
      }
      i++;
    }

    if(xx.size() == 0) continue;
    sort(xx.begin(), xx.end());

    int w = 0;
    int L = xx[0].first;
    for(int ii = 0; ii < xx.size(); ii++) {
      w += xx[ii].second;
      if(w == 0) {
        optimizeBlend(L, y, xx[ii].first - L, paint);
        if(ii < xx.size()-1) L = xx[ii+1].first;
      }
    }

  }
}

GShader* initColorShader(GPoint U, GPoint V, int n, GPoint points[], const GColor colors[], const int indices[]) {
  GColor c[3];
  c[0] = colors[indices[n+0]];
  c[1] = colors[indices[n+1]];
  c[2] = colors[indices[n+2]];

  GMatrix localMatrix = GMatrix(
      U.x, V.x, points[0].x,
      U.y, V.y, points[0].y);

  return new TriColorShader(localMatrix, c);
}

GShader* initTexShader(GPoint U, GPoint V, int n, GPoint points[], const GPoint texs[], const int indices[], const GPaint& paint) {
  if(paint.getShader() == nullptr) {
    return nullptr;
  }
  GPoint txts[3];
  txts[0] = texs[indices[n+0]];
  txts[1] = texs[indices[n+1]];
  txts[2] = texs[indices[n+2]];

  GMatrix P = GMatrix(
      U.x, V.x, points[0].x,
      U.y, V.y, points[0].y);
  GPoint TU = txts[1] - txts[0];
  GPoint TV = txts[2] - txts[0];
  GMatrix Tinv, T = GMatrix(
      TU.x, TV.x, txts[0].x,
      TU.y, TV.y, txts[0].y);
  optional<GMatrix> Tt = T.invert();
  if(Tt.has_value()) Tinv = *Tt;
  else return nullptr;

  return new TexShader(P*Tinv, paint.getShader());
}


void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
    int count, const int indices[], const GPaint& paint) {
  GShader* shader;
  int n = 0;
  for(int i = 0; i < count ; i++) {
    GPoint points[3];
    points[0] = verts[indices[n+0]];
    points[1] = verts[indices[n+1]];
    points[2] = verts[indices[n+2]];

    GPoint U = points[1] - points[0];
    GPoint V = points[2] - points[0];


    if(texs == nullptr) {
      shader = initColorShader(U, V, n, points, colors, indices);
    } else if(colors == nullptr) {
      shader = initTexShader(U, V, n, points, texs, indices, paint);
    } else {
      GShader* cShader = initColorShader(U, V, n, points, colors, indices);
      GShader* tShader = initTexShader(U, V, n, points, texs, indices, paint);
      shader = new TexColorShader(cShader, tShader);
    }

    GPaint shaderPaint = paint;
    shaderPaint.setShader(shader);

    drawConvexPolygon(points, 3, shaderPaint);
    
    n += 3;
  }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
    int level, const GPaint& paint) {
  if(!colors) return;

  int num = (2+level)*(2+level);
  int count = (1+level)*(1+level);
  GPoint points[num];
  GColor ncolors[num];
  int indices[6*count];

  float stp = 1.0f / (level+1.0f);

  GPoint v1step = (verts[3] - verts[0])*stp;
  GPoint v2step = (verts[2] - verts[1])*stp;

  GPoint a = verts[0];
  GPoint b = verts[1];

  GColor acolor = colors[0];
  GColor bcolor = colors[1];

  GColor vc1step = (colors[3] - colors[0])*stp;
  GColor vc2step = (colors[2] - colors[1])*stp;

  int idx = 0;
  for(int i = 0; i < level+2; i++) {
    GPoint ustep = (b-a)*stp;
    GPoint pt = a;

    GColor ucstep = (bcolor-acolor)*stp;
    GColor c = acolor;
    for(int j = 0; j < level+2; j++) {
      points[idx] = pt;
      pt += ustep;

      ncolors[idx] = c;
      c += ucstep;

      idx++;
    }
    a += v1step;
    b += v2step;

    acolor += vc1step;
    bcolor += vc2step;
  }

  idx = 0;
  for(int i = 0; i < level+1; i++) {
    for(int j = 0; j < level+1; j++) {
      int lt = (level+2)*i + j;
      int lb = (level+2)*(i+1) + j;
      
      indices[idx+0] = lt;
      indices[idx+1] = lt+1;
      indices[idx+2] = lb;

      indices[idx+3] = lt+1;
      indices[idx+4] = lb;
      indices[idx+5] = lb+1;

      idx += 6;
    }
  }

  drawMesh(points, ncolors, nullptr, 2*count, indices, paint);

}


std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
  return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
  return "";
}

