#include "include/GPoint.h"
#include "include/GMath.h"
#include <vector>

#include <algorithm>

using namespace std;

struct Edge {

  float m, b;
  int top, bottom;
  int direction;

  float x(float y) { 
    return m*y + b; 
  }

  float y(float x) { 
    return (x - b) / m; 
  }

  void projectTo(int x) {
    m = 0;
    b = x;
  }

  bool isValid(int y) {
    return (y >= top && y < bottom);
  }

  bool operator<(const Edge& other) const {
    if (top != other.top) {
      return top < other.top;
    } else {
      return bottom < other.bottom;
    }
  }

};

static inline Edge createEdge(GPoint p0, GPoint p1, int direction) {
  float m = (p1.x - p0.x) / (p1.y - p0.y);
  float b = p0.x - m*p0.y; 

  int p0y = (int)(p0.y+0.5);
  int p1y = (int)(p1.y+0.5);

  return { m, b, min(p0y, p1y), max(p0y, p1y), direction };
}

void clipEdge(vector<Edge>& edges, GPoint p0, GPoint p1, int boundBottom, int boundRight) {
  int boundTop = 0;
  int boundLeft = 0;

  int direction = p0.y < p1.y ? 1 : -1;
  Edge edge = createEdge(p0, p1, direction);

  // top bottom
  if(edge.top == edge.bottom) return;

  if(p0.y > p1.y) swap(p0, p1);
  if(p1.y < boundTop || p0.y > boundBottom) return;

  if(edge.top < boundTop) {
    edge.top = boundTop;
    p0.x = edge.x(boundTop);
    p0.y = boundTop;
  }
  if(edge.bottom > boundBottom) {
    edge.bottom = boundBottom;
    p1.x = edge.x(boundBottom);
    p1.y = boundBottom;
  }

  // left right
  if(p0.x > p1.x) swap(p0, p1);
  if(p1.x < boundLeft) {
    edge.projectTo(0);
    edges.push_back(edge);
    return;
  } else if(p0.x >= boundRight) {
    edge.projectTo(boundRight);
    edges.push_back(edge);
    return;
  }

  if(p0.x < boundLeft) {
    GPoint intersection = { boundLeft, edge.y(boundLeft) };
    GPoint projection = { boundLeft, p0.y };

    Edge projected = createEdge(intersection, projection, direction);
    edges.push_back(projected);

    if(edge.m < 0) edge.bottom = projected.top;
    else edge.top = projected.bottom;
  }

  if(p1.x > boundRight) {
    GPoint intersection = { boundRight, edge.y(boundRight) };
    GPoint projection = { boundRight, p1.y };

    Edge projected = createEdge(intersection, projection, direction);
    edges.push_back(projected);

    if(edge.m > 0) edge.bottom = projected.top;
    else edge.top = projected.bottom;
  }

  edges.push_back(edge);
}

