#include <stdint.h>
#include "point_search.h"

/* Load the provided points into an internal data structure. The pointers follow the STL iterator convention, where
"points_begin" points to the first element, and "points_end" points to one past the last element. The input points are
only guaranteed to be valid for the duration of the call. Return a pointer to the context that can be used for
consecutive searches on the data. */
extern "C" _declspec(dllexport) SearchContext* create(const Point* points_begin, const Point* points_end)
{
  if (points_begin == nullptr || points_end == nullptr) 
    return nullptr;

  return nullptr;
}

/* Search for "count" points with the smallest ranks inside "rect" and copy them ordered by smallest rank first in
"out_points". Return the number of points copied. "out_points" points to a buffer owned by the caller that
can hold "count" number of Points. */
extern "C" _declspec(dllexport) int32_t search(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points)
{
  if (sc == nullptr)
    return 0;

  return 0;
}

/* Release the resources associated with the context. Return nullptr if successful, "sc" otherwise. */
extern "C" _declspec(dllexport) SearchContext* destroy(SearchContext* sc)
{
  if (sc == nullptr)
    return nullptr;

  return nullptr;
}
