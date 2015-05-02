#include <memory>
#include <vector>
#include <algorithm>
#include <cassert>
#include "point_search.h"
#include "QuadTree.h"


QuadTree::QuadTree()
{

}

void QuadTree::Load(const Point* points_begin, const Point* points_end)
{
  // The bounds always come out to +/-1E10, presumably to frustrate attempts at finding a tight bounding box.
  // Still, best not to assume that.  Sparse quadtrees don't mind much anyway.  :)
  Rect bounds{ 0,0,0,0 };
  
  std::for_each (points_begin, points_end, [&](const Point pt)
  {
    if (pt.x < bounds.lx) bounds.lx = pt.x;
    if (pt.x > bounds.hx) bounds.hx = pt.x;
    if (pt.y < bounds.ly) bounds.ly = pt.y;
    if (pt.y > bounds.hy) bounds.hy = pt.y;
  });
  /*
  for (auto pt = points_begin; pt <= points_end; pt++)
  {
    if (pt->x < bounds.lx) bounds.lx = pt->x;
    if (pt->x > bounds.hx) bounds.hx = pt->x;
    if (pt->y < bounds.ly) bounds.ly = pt->y;
    if (pt->y > bounds.hy) bounds.hy = pt->y;
  }*/

  // Initialize the root node
  root = new QuadNode(bounds);

  // Then insert everything
  std::for_each (points_begin, points_end+1, [&](const Point pt)
  {
    root->Insert(pt);
  });

  volatile int foo = 42;
}

QuadTree::~QuadTree()
{
  delete root;
}

QuadNode::QuadNode(Rect inBounds)
{
  bounds = inBounds;
  size = 0;
  points = new Point[capacity];
}

QuadNode::~QuadNode()
{
  delete child[0];
  delete child[1];
  delete child[2];
  delete child[3];
}

void QuadNode::Insert(Point pt)
{
  assert(pt.x >= bounds.lx);
  assert(pt.x <= bounds.hx);
  assert(pt.y >= bounds.ly);
  assert(pt.y <= bounds.hy);

  float cx = (bounds.lx + bounds.hx) * 0.5f;
  float cy = (bounds.ly + bounds.hy) * 0.5f;

  if (points) // Only leaf nodes have points
  {
    if (size < capacity)
    {
      points[size++] = pt;
    }
    else
    {
      child[0] = new QuadNode(Rect{ bounds.lx, bounds.ly, cx, cy });    // BL
      child[1] = new QuadNode(Rect{ cx, bounds.ly, bounds.hx, cy });    // BR
      child[2] = new QuadNode(Rect{ bounds.lx, cy, cx, bounds.hy });    // TL
      child[3] = new QuadNode(Rect{ cx, cy, bounds.hx, bounds.hy });    // TR
      auto tempPoints = points;
      points = nullptr;
      for (int i = 0; i < size; i++)
        Insert(tempPoints[i]);
      delete tempPoints;
    }
  }
  else
  {
    Quadrant quad;
    if (pt.x >= cx)
    {
      if (pt.y >= cy) quad = TOPRIGHT;
      else            quad = BOTTOMRIGHT;
    }
    else
    {
      if (pt.y >= cy) quad = TOPLEFT;
      else            quad = BOTTOMLEFT;
    }

    child[quad]->Insert(pt);
  }

}

int QuadNode::Count()
{
  if (points)
    return 1;
  
  return 1 +
    child[0]->Count() + 
    child[1]->Count() +
    child[2]->Count() +
    child[3]->Count();
}
