// Filename: nurbsMatrixVector.cxx
// Created by:  drose (03Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "nurbsMatrixVector.h"

////////////////////////////////////////////////////////////////////
//     Function: NurbsMatrixVector::clear
//       Access: Public
//  Description: Removes all the segments from the curve.
////////////////////////////////////////////////////////////////////
void NurbsMatrixVector::
clear() {
  _segments.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsMatrixVector::append_segment
//       Access: Public
//  Description: Computes a NURBS basis for one segment of the curve
//               and appends it to the set of basis matrices.
////////////////////////////////////////////////////////////////////
void NurbsMatrixVector::
append_segment(int order, int vertex_index, const float knots[]) {
  int i;

  // Scale the supplied knots to the range 0..1.
  float scaled_knots[8];
  float min_k = knots[order - 1];
  float max_k = knots[order];

  nassertv(min_k != max_k);
  for (i = 0; i < order + order; i++) {
    scaled_knots[i] = (knots[i] - min_k) / (max_k - min_k);
  }

  Segment segment;
  segment._vertex_index = vertex_index;
  segment._from = min_k;
  segment._to = max_k;

  for (i = 0; i < order; i++) {
    LVecBase4f b = nurbs_blending_function(order, i, order, scaled_knots);
    segment._matrix.set_col(i, b);
  }

  for (i = order; i < 4; i++) {
    segment._matrix.set_col(i, LVecBase4f::zero());
  }

  _segments.push_back(segment);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsMatrixVector::compose_segment
//       Access: Public
//  Description: Appends a new segment to the vector by composing the
//               indicated geometry matrix with the indicated basis
//               matrix from the given vector.
////////////////////////////////////////////////////////////////////
void NurbsMatrixVector::
compose_segment(const NurbsMatrixVector &basis, int segment, 
                const LMatrix4f &geom) {
  nassertv(segment >= 0 && segment < (int)basis._segments.size());
  const Segment &source = basis._segments[segment];

  Segment dest;
  dest._vertex_index = source._vertex_index;
  dest._from = source._from;
  dest._to = source._to;
  dest._matrix = source._matrix * geom;

  _segments.push_back(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsMatrixVector::nurbs_blending_function
//       Access: Private, Static
//  Description: Recursively computes the appropriate blending
//               function for the indicated knot vector.
////////////////////////////////////////////////////////////////////
LVecBase4f NurbsMatrixVector::
nurbs_blending_function(int order, int i, int j, const float knots[]) {
  // This is doubly recursive.  Ick.
  LVecBase4f r;

  if (j == 1) {
    if (i == order-1 && knots[i] < knots[i+1]) {
      r.set(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
      r.set(0.0f, 0.0f, 0.0f, 0.0f);
    }

  } else {
    LVecBase4f bi0 = nurbs_blending_function(order, i, j - 1, knots);
    LVecBase4f bi1 = nurbs_blending_function(order, i + 1, j - 1, knots);

    float d0 = knots[i + j - 1] - knots[i];
    float d1 = knots[i + j] - knots[i + 1];

    // First term.  Division by zero is defined to equal zero.
    if (d0 != 0.0f) {
      if (d1 != 0.0f) {
        r = bi0 / d0 - bi1 / d1;
      } else {
        r = bi0 / d0;
      }

    } else if (d1 != 0.0f) {
      r = - bi1 / d1;

    } else {
      r.set(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // scale by t.
    r[0] = r[1];
    r[1] = r[2];
    r[2] = r[3];
    r[3] = 0.0f;

    // Second term.
    if (d0 != 0.0f) {
      if (d1 != 0.0f) {
        r += bi0 * (- knots[i] / d0) + bi1 * (knots[i + j] / d1);
      } else {
        r += bi0 * (- knots[i] / d0);
      }

    } else if (d1 != 0.0f) {
      r += bi1 * (knots[i + j] / d1);
    }
  }

  return r;
}
