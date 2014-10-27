/********************************************************************* *\
 * INTEL CORPORATION PROPRIETARY INFORMATION                            
 * This software is supplied under the terms of a license agreement or  
 * nondisclosure agreement with Intel Corporation and may not be copied 
 * or disclosed except in accordance with the terms of that agreement.  
 * Copyright (C) 2014 Intel Corporation. All Rights Reserved.           
 ********************************************************************* */

#pragma once

// viewer
#include "HelperGeometry.h"

namespace ospray {
  namespace viewer {

    void HelperGeometry::Mesh::addQuad(const affine3f &xfm, const Quad &quad)
    {
      vec3i idx0 = vec3i(vertex.size());
      for (int i=0;i<4;i++) {
        vertex.push_back(vec3fa(xfmPoint(xfm,quad.vtx[i])));
        normal.push_back(vec3fa(normalize(xfmVector(xfm,quad.nor[i]))));
      }
      index.push_back(idx0+vec3i(0,1,2));
      index.push_back(idx0+vec3i(0,2,3));
    }
      
    CoordFrameGeometry::CoordFrameGeometry()
      : shaftThickness(.1), headLength(.2f), numSegments(16)
    {
      makeArrow(arrow[0],vec3f(1,0,0));
      makeArrow(arrow[1],vec3f(0,1,0));
      makeArrow(arrow[2],vec3f(0,0,1));
    }
    void CoordFrameGeometry::makeArrow(Mesh &mesh,const vec3f &axis) 
    {
      mesh.color = axis;
      affine3f xfm = embree::frame(axis);
      std::swap(xfm.l.vx,xfm.l.vz);
      Quad quad; 
      for (int i=0;i<numSegments;i++) {
        const float f0 = (i+0.f)/numSegments * 2.f * M_PI;
        const float f1 = (i+1.f)/numSegments * 2.f * M_PI;
        const float u0 = cosf(f0), u1 = cosf(f1);
        const float v0 = sinf(f0), v1 = sinf(f1);
        
        quad.vtx[0] = vec3f(0,shaftThickness*u0,shaftThickness*v0);
        quad.vtx[1] = vec3f(1.f-headLength,shaftThickness*u0,shaftThickness*v0);
        quad.vtx[2] = vec3f(1.f-headLength,shaftThickness*u1,shaftThickness*v1);
        quad.vtx[3] = vec3f(0,shaftThickness*u1,shaftThickness*v1);
        quad.nor[0] = quad.nor[1] = vec3f(0,u0,v0);
        quad.nor[2] = quad.nor[3] = vec3f(0,u1,v1);
        mesh.addQuad(xfm,quad);

        quad.vtx[0] = vec3f(1.f-headLength,headLength*u0,headLength*v0);
        quad.vtx[3] = vec3f(1.f-headLength,headLength*u1,headLength*v1);
        quad.nor[0] = quad.nor[1] = quad.nor[2] = quad.nor[3] = vec3f(-1,0,0);
        mesh.addQuad(xfm,quad);

        quad.vtx[1] = quad.vtx[2] = vec3f(1.f,0,0);
        quad.nor[0] = quad.nor[1] = vec3f(+1,u0,v0);
        quad.nor[2] = quad.nor[3] = vec3f(+1,u1,v1);
        mesh.addQuad(xfm,quad);
      }
    }
  } // ::viewer
} // ::ospray

