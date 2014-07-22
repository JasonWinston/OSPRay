/********************************************************************* *\
 * INTEL CORPORATION PROPRIETARY INFORMATION                            
 * This software is supplied under the terms of a license agreement or  
 * nondisclosure agreement with Intel Corporation and may not be copied 
 * or disclosed except in accordance with the terms of that agreement.  
 * Copyright (C) 2014 Intel Corporation. All Rights Reserved.           
 ********************************************************************* */



#pragma once

#include "geometry.h"

/*! @{ \ingroup ospray_module_streamlines */
namespace ospray {

  /*! \defgroup geometry_streamlines Stream Lines ("streamlines") 

    \ingroup ospray_supported_geometries

    \brief Geometry representing stream lines of a fixed radius

    Implements a geometry consisinting of stream lines
    consisting of connected (and rounded) cylinder segments. Each
    stream line is specified by a set of vec3fa control points; all
    vertices belonging to to the same logiical stream line are
    connected via cylinders of a fixed radius, with additional spheres
    at each vertex to make for a smooth transition between the
    cylinders. A "streamline" geometry can contain multiple disjoint
    stream lines; all such stream lines use the same radius that is
    specified for this geometry.

    A stream line geometry is created via \ref ospNewGeometry with
     type string "streamlines".

    Each streamline is specified as a list of linear segments, where
    each segment is pretty much a cylinder with rounded ends. The
    "streamline" geometry is built over those links (not over the
    curves), so the input to the geometry object are a) a list of
    vertices, plus b) a list of ints, where each int specifies the
    first vertex of a link (the second one being "index+1").

    For example, two streamlines of vertices (A-B-C-D) and (E-F-G),
    respectively, would internally correspond to 7 links (A-B, B-C,
    C-D, E-F, and F-G), and could be specified via an array of
    vertices "A,B,C,D,E,F,G", plus an array of link offsets
    "0,1,2,4,5"

    Vertices are stored in vec3fa form; the user is free to use the
    'w' value of each vertex to store additional value (such as curve
    ID, or an attribute to be visualized); ospray will not touch this
    value

    Parameters:
    <dl>
    <dt><code>float        radius</code></dt><dd> Radius to be used for all stream lines</dd>
    <dt><li><code>Data<vec3fa> vertex</code></dt><dd> Array of all vertices for *all* curves in this geometry, one curve's vertices stored after another.</dd>
    <dt><li><code>Data<int32>  index </code></dt><dd> index[i] specifies the index of the first vertex of the i'th curve. The curve then uses all following vertices in the 'vertex' array until either the next curve starts, or the array's end is reached.</dd>
    </dl>

    The functionality for this geometry is implemented via the
    \ref ospray::TriangleMesh class.

    \image html sl2.jpg

  */



  /*! \brief A geometry for stream line curves

    Implements the \ref geometry_streamlines geometry

  */
  struct StreamLines : public Geometry {
    //! \brief common function to help printf-debugging 
    virtual std::string toString() const { return "ospray::StreamLines"; }
    /*! \brief integrates this geometry's primitives into the respective
        model's acceleration structure */
    virtual void finalize(Model *model);

    Ref<Data> vertexData;  //!< refcounted data array for vertex data
    Ref<Data> indexData; //!< refcounted data array for segment data

    const vec3fa *vertex;
    size_t        numVertices;
    const uint32 *index;
    size_t        numSegments;
    float         radius;

    StreamLines();
  };
}
/*! @} */
