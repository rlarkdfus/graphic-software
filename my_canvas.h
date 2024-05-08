/*
 *  Copyright 2024 <me>
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GPaint.h"
#include "include/GMatrix.h"
#include "include/GPath.h"
#include "include/GMath.h"

#include "blender.h"
#include "bitmap_shader.h"
#include "gradient_shader.h"
#include "tricolor_shader.h"
#include "tex_shader.h"
#include "texcolor_shader.h"

#include <vector>
#include <stack>
#include <iostream>

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : canvas(device) {
      ctmStack.push(GMatrix(1,0,0,0,1,0));
    }

    void save() override;
    void restore() override;
    void concat(const GMatrix& matrix) override;
    void clear(const GColor& color) override;
    void drawRect(const GRect& rect, const GPaint& paint) override;
    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override;
    void drawPath(const GPath&, const GPaint&) override;
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
        int count, const int indices[], const GPaint& paint) override;
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
        int level, const GPaint& paint) override;


    template<typename Proc> 
      void fillRow(int x, int y, int count, GPixel row[], Proc blend);
    void optimizeBlend(int x, int y, int count, const GPaint& paint);


private:
    const GBitmap canvas;
    std::stack<GMatrix> ctmStack;
};

#endif
