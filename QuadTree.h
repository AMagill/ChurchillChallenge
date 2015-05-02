#pragma once

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

class QuadTree
{
private:
  QuadNode* root;
  Rect extents;

public:
  QuadTree();
  ~QuadTree();
  void Load(const Point* points_begin, const Point* points_end);
};

class QuadNode 
{
private:
  static const int capacity = 20; // Limit 255
  uint8_t size;
  Point* points;
  QuadNode* child[4];
  Rect bounds;

public:
  QuadNode(Rect);
  ~QuadNode();
  void Insert(Point);
  int Count();
};
