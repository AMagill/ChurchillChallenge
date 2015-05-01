#pragma once

#define EMPTY -1

enum Quadrant
{
  BOTTOMLEFT  = 0,
  BOTTOMRIGHT = 1,
  TOPLEFT     = 2,
  TOPRIGHT    = 3,
  // Masks
  RIGHT       = 1,
  TOP         = 2,
};

class QuadNode;
class QuadInternal;

class QuadTree
{
private:
  QuadInternal* root;
  Rect extents;

public:
  QuadTree(const Point* points_begin, const Point* points_end);
  ~QuadTree();
};

class QuadNode 
{
public:
  //virtual ~QuadNode() = 0;
};

class QuadLeaf : public QuadNode
{
  static int allocs;
public:
  Point point;
  QuadLeaf(Point);
};

class QuadInternal : public QuadNode
{
  static int allocs;
  QuadNode* child[4];
  Rect bounds;
  float cx, cy;

public:
  QuadInternal(Rect);
  ~QuadInternal();
  void Insert(Point);
};
