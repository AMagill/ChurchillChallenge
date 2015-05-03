#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
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

  // Initialize the root node
  root = new QuadNode(bounds);

  // Then insert everything
  std::for_each (points_begin, points_end+1, [&](const Point pt)
  {
    root->Insert(pt);
  });

  root->Sort();

  volatile int foo = 42;
}

QuadTree::~QuadTree()
{
  delete root;
}

int32_t QuadTree::Search(const Rect rect, const int32_t count, Point* out_points)
{
  return 0;//root->Search(rect, count, out_points);
}


QuadNode::QuadNode(Rect inBounds)
{
  isLeaf = true;
  bounds = inBounds;
  points = new std::vector<Point>();
  points->reserve(capacity);
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

  if (isLeaf)
  {
    if (points->size() < capacity)
    {
      points->push_back(pt);
    }
    else
    {
      isLeaf = false;
      for (auto pt : *points)
        Insert(pt);
      points->clear();
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

    // Allocate the child quad if necessary
    if (child[quad] == nullptr)
    {
      switch (quad)
      {
      case BOTTOMLEFT:
        child[0] = new QuadNode(Rect{ bounds.lx, bounds.ly, cx, cy });
        break;
      case BOTTOMRIGHT:
        child[1] = new QuadNode(Rect{ cx, bounds.ly, bounds.hx, cy });
        break;
      case TOPLEFT:
        child[2] = new QuadNode(Rect{ bounds.lx, cy, cx, bounds.hy });
        break;
      case TOPRIGHT:
        child[3] = new QuadNode(Rect{ cx, cy, bounds.hx, bounds.hy });
        break;
      }
    }
    child[quad]->Insert(pt);
  }

}

int QuadNode::Count()
{
  if (isLeaf)
    return 1;
  
  int count = 1;  // One for self
  for (int i = 0; i < 4; i++)
  {
    if (child[i])
      count += child[i]->Count();
  }

  return count;
}

void QuadNode::Sort()
{
  if (!isLeaf)
  {
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
      if (child[i])
      {
        child[i]->Sort();
        count++;
      }
    }

    if (count > 1)
    {
      for (int i = 0; i < 4; i++)
      {
        if (child[i])
        {
          int nCopy = std::min(capacity, (int)child[i]->points->size());
          std::copy(child[i]->points->begin(), child[i]->points->begin() + nCopy, std::back_inserter(*points));
        }
      }
    }
  }

  std::sort(points->begin(), points->end(), [](Point a, Point b) { return a.rank < b.rank; });

  if (!isLeaf)
  {
    points->resize(capacity);
    points->shrink_to_fit();
  }
}

std::vector<Point> QuadNode::Search(const Rect rect, const int32_t count)
{
  if (isLeaf)
  {

  }
  else
  {

  }

  return std::vector<Point>();
}