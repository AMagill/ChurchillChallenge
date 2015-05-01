#include <memory>
#include <vector>
#include <algorithm>
#include <cassert>
#include "point_search.h"
#include "QuadTree.h"


QuadTree::QuadTree(const Point* points_begin, const Point* points_end)
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

  // Initialize the root node
  root = new QuadInternal(bounds);

  // Then insert everything
  for (const Point* ptr = points_begin; ptr <= points_end; ptr++)
  {
    auto foo = *ptr;
    auto bar = 32;
  }

  std::for_each (points_begin, points_end+1, [&](const Point pt)
  {
    //auto foo(pt);
    auto bar = 42;
    root->Insert(pt);
  });

}

QuadTree::~QuadTree()
{

}

//QuadNode::~QuadNode() {}

int QuadInternal::allocs;
QuadInternal::QuadInternal(Rect inBounds)
{
  allocs++;
  bounds = inBounds;
  cx = (bounds.lx + bounds.hx) * 0.5f;
  cy = (bounds.ly + bounds.hy) * 0.5f;

  child[0] = nullptr;
  child[1] = nullptr;
  child[2] = nullptr;
  child[3] = nullptr;
}

QuadInternal::~QuadInternal()
{
  delete child[0], child[1], child[2], child[3];
}

void QuadInternal::Insert(Point pt)
{
  assert(pt.x >= bounds.lx);
  assert(pt.x <= bounds.hx);
  assert(pt.y >= bounds.ly);
  assert(pt.y <= bounds.hy);

  Quadrant quad;
  Rect subBounds = Rect(bounds);
  float x = pt.x;
  float y = pt.y;

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

  if (child[quad] == nullptr)
  { // Empty leaf; construct it
    child[quad] = new QuadLeaf(pt);
  }
  else if (typeid(*child[quad]) == typeid(QuadLeaf))
  { // Full leaf; split it
    Rect subBounds(bounds);
    if (quad & RIGHT) subBounds.lx = cx;
    else              subBounds.hx = cx;
    if (quad & TOP)   subBounds.ly = cy;
    else              subBounds.hy = cy;
    QuadInternal* newNode = new QuadInternal(subBounds);
    newNode->Insert(static_cast<QuadLeaf*>(child[quad])->point);
    delete child[quad];
    child[quad] = newNode;
  }
  else
  { // Internal node; recurse
    assert((typeid(*child[quad]) == typeid(QuadInternal)));
    static_cast<QuadInternal*>(child[quad])->Insert(pt);
  }

}

int QuadLeaf::allocs;
QuadLeaf::QuadLeaf(Point pt)
{
  allocs++;
  point = pt;
}
