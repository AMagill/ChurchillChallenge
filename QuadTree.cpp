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
  // Allocate storage for all the points
  nAllPoints = (int)(points_end - points_begin);
  allPoints = new Point[nAllPoints];

  // The bounds always come out to +/-1E10, presumably to frustrate attempts at finding a tight bounding box.
  // Still, best not to assume that.  Sparse quadtrees don't mind much anyway.  :)
  Rect bounds{ 0,0,0,0 };
  
  // Save points in order.  The rank makes a convenient indexer!
  for (auto pt = points_begin; pt < points_end; pt++)
  {
    assert(pt->rank < nAllPoints);
    allPoints[pt->rank] = *pt;

    if (pt->x < bounds.lx) bounds.lx = pt->x;
    if (pt->x > bounds.hx) bounds.hx = pt->x;
    if (pt->y < bounds.ly) bounds.ly = pt->y;
    if (pt->y > bounds.hy) bounds.hy = pt->y;
  };

  // Initialize the root node
  root = new QuadNode(allPoints, bounds);

  // Then insert everything
  std::for_each (points_begin, points_end, [&](const Point pt)
  {
    root->Insert(pt);
  });

  root->Sort();
}

QuadTree::~QuadTree()
{
  delete root;
}

int32_t QuadTree::Search(const Rect* rect, const int32_t count, Point* out_points)
{
  std::vector<int32_t> results;
  root->Search(rect, count, results);
  std::sort_heap(results.begin(), results.end());

  int nPoints = std::min((int)results.size(), count);
  for (int i = 0; i < nPoints; i++)
    out_points[i] = allPoints[results[i]];
  
  return nPoints;
}


QuadNode::QuadNode(Point* inAllPoints, Rect inBounds)
{
  isLeaf = true;
  allPoints = inAllPoints;
  bounds = inBounds;
  points.reserve(capacity);
}

QuadNode::~QuadNode()
{
  delete child[0];
  delete child[1];
  delete child[2];
  delete child[3];
}

void QuadNode::Insert(Point pt, int depth /* = 0 */)
{
/*  assert(pt.x >= bounds.lx);
  assert(pt.x <= bounds.hx);
  assert(pt.y >= bounds.ly);
  assert(pt.y <= bounds.hy);*/
  
  float cx = (bounds.lx + bounds.hx) * 0.5f;
  float cy = (bounds.ly + bounds.hy) * 0.5f;

  volatile auto foo = pt;

  if (isLeaf)
  {
    if (points.size() < capacity ||
        depth >= max_depth)   // Capacity limit is waived, to prevent stack overflows.  (Tricky, tricky..)
    {
      points.push_back(pt.rank);
    }
    else
    {
      // Split the leaf, reinserting its contents into its children
      isLeaf = false;
      for (auto movePt : points)
        Insert(allPoints[movePt], depth);
      points.clear();
      // Don't forget this one
      Insert(pt, depth);
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
        child[0] = new QuadNode(allPoints, Rect{ bounds.lx, bounds.ly, cx, cy });
        break;
      case BOTTOMRIGHT:
        child[1] = new QuadNode(allPoints, Rect{ cx, bounds.ly, bounds.hx, cy });
        break;
      case TOPLEFT:
        child[2] = new QuadNode(allPoints, Rect{ bounds.lx, cy, cx, bounds.hy });
        break;
      case TOPRIGHT:
        child[3] = new QuadNode(allPoints, Rect{ cx, cy, bounds.hx, bounds.hy });
        break;
      }
    }
    child[quad]->Insert(pt, depth+1);
  }

}

int QuadNode::Count()
{
  if (isLeaf)
    return (int)points.size();
  
  int count = 0;
  for (int i = 0; i < 4; i++)
  {
    if (child[i])
      count += child[i]->Count();
  }

  return count;
}

void QuadNode::Sort()
{
  // Compress the tree; remove nodes with only one child, promoting that child to their place.
  for (int i = 0; i < 4; i++)
  {
    if (child[i] == nullptr)
      continue;

    while (true)
    {
      int count = 0;
      int which;
      for (int j = 0; j < 4; j++)
      {
        if (child[i]->child[j] != nullptr)
        {
          count++;
          which = j;
        }
      }

      if (count == 1) // This child only has one child
      {  // Promote it
        auto temp = child[i]->child[which];
        child[i]->child[which] = nullptr;   // So it won't get deleted by the destructor
        delete child[i];
        child[i] = temp;
      }
      else
        break;
    }
  }

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
          int nCopy = std::min(capacity, (int)child[i]->points.size());
          std::copy(child[i]->points.begin(), child[i]->points.begin() + nCopy, std::back_inserter(points));
        }
      }
    }
  }

  std::sort(points.begin(), points.end());

  if (!isLeaf)
  {
    points.resize(capacity);
    points.shrink_to_fit();
  }
}

void QuadNode::Search(const Rect* rect, const int32_t count, std::vector<int32_t>& results)
{
  if (rect->lx > bounds.hx ||
      rect->hx < bounds.lx ||
      rect->ly > bounds.hy ||
      rect->hy < bounds.ly)
    return;    // No intersection; return nothing
  
  if (rect->lx < bounds.lx &&
      rect->hx > bounds.hx &&
      rect->ly < bounds.ly &&
      rect->hy > bounds.hy)
  { // Fully enclosed; return everything
    for (auto pt : points)
    {
      results.push_back(pt);
      std::push_heap(results.begin(), results.end());
      if (results.size() > count)
      {
        std::pop_heap(results.begin(), results.end());
        results.pop_back();
      }
    }
    return;
  }
  
  // Must be a partial intersection.  Need to recurse or individually test points.
  if (isLeaf)
  {
    int passed = 0;
    for (auto pt : points)
    {
      if (allPoints[pt].x >= rect->lx &&
          allPoints[pt].x <= rect->hx &&
          allPoints[pt].y >= rect->ly &&
          allPoints[pt].y <= rect->hy)
      {
        results.push_back(pt);
        std::push_heap(results.begin(), results.end());
        if (results.size() > count)
        {
          std::pop_heap(results.begin(), results.end());
          results.pop_back();
        }

        passed++;
        if (passed >= count)
          break;
      }
    }
  }
  else
  {
    for (int i = 0; i < 4; i++)
    {
      if (child[i])
        child[i]->Search(rect, count, results);
    }
  }

}
