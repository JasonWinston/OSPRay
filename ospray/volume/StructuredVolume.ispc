// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "ospray/volume/StructuredVolume.ih"
#include "ospray/volume/GridAccelerator.ih"

inline varying float StructuredVolume_computeSample(void *uniform _volume, const varying vec3f &worldCoordinates)
{
  // Cast to the actual Volume subtype.
  StructuredVolume *uniform volume = (StructuredVolume *uniform) _volume;

  // Transform the sample location into the local coordinate system.
  vec3f localCoordinates;
  volume->transformWorldToLocal(volume, worldCoordinates, localCoordinates);

  // Coordinates outside the volume are clamped to the volume bounds.
  const vec3f clampedLocalCoordinates = clamp(localCoordinates, make_vec3f(0.0f),
                                              volume->localCoordinatesUpperBound);

  // Lower and upper corners of the box straddling the voxels to be interpolated.
  const vec3i voxelIndex_0 = integer_cast(clampedLocalCoordinates);
  const vec3i voxelIndex_1 = voxelIndex_0 + 1;

  // Fractional coordinates within the lower corner voxel used during interpolation.
  const vec3f fractionalLocalCoordinates = clampedLocalCoordinates - float_cast(voxelIndex_0);

  // Look up the voxel values to be interpolated.
  float voxelValue_000;
  float voxelValue_001;
  float voxelValue_010;
  float voxelValue_011;
  float voxelValue_100;
  float voxelValue_101;
  float voxelValue_110;
  float voxelValue_111;
  volume->getVoxel(volume, make_vec3i(voxelIndex_0.x, voxelIndex_0.y, voxelIndex_0.z), voxelValue_000);
  volume->getVoxel(volume, make_vec3i(voxelIndex_1.x, voxelIndex_0.y, voxelIndex_0.z), voxelValue_001);
  volume->getVoxel(volume, make_vec3i(voxelIndex_0.x, voxelIndex_1.y, voxelIndex_0.z), voxelValue_010);
  volume->getVoxel(volume, make_vec3i(voxelIndex_1.x, voxelIndex_1.y, voxelIndex_0.z), voxelValue_011);
  volume->getVoxel(volume, make_vec3i(voxelIndex_0.x, voxelIndex_0.y, voxelIndex_1.z), voxelValue_100);
  volume->getVoxel(volume, make_vec3i(voxelIndex_1.x, voxelIndex_0.y, voxelIndex_1.z), voxelValue_101);
  volume->getVoxel(volume, make_vec3i(voxelIndex_0.x, voxelIndex_1.y, voxelIndex_1.z), voxelValue_110);
  volume->getVoxel(volume, make_vec3i(voxelIndex_1.x, voxelIndex_1.y, voxelIndex_1.z), voxelValue_111);

  // Interpolate the voxel values.
  const float voxelValue_00 = voxelValue_000 + fractionalLocalCoordinates.x * (voxelValue_001 - voxelValue_000);
  const float voxelValue_01 = voxelValue_010 + fractionalLocalCoordinates.x * (voxelValue_011 - voxelValue_010);
  const float voxelValue_10 = voxelValue_100 + fractionalLocalCoordinates.x * (voxelValue_101 - voxelValue_100);
  const float voxelValue_11 = voxelValue_110 + fractionalLocalCoordinates.x * (voxelValue_111 - voxelValue_110);
  const float voxelValue_0  = voxelValue_00  + fractionalLocalCoordinates.y * (voxelValue_01  - voxelValue_00 );
  const float voxelValue_1  = voxelValue_10  + fractionalLocalCoordinates.y * (voxelValue_11  - voxelValue_10 );
  const float volumeSample  = voxelValue_0   + fractionalLocalCoordinates.z * (voxelValue_1   - voxelValue_0  );

  return volumeSample;
}

inline varying vec3f StructuredVolume_computeGradient(void *uniform _volume, const varying vec3f &worldCoordinates)
{
  // Cast to the actual Volume subtype.
  StructuredVolume *uniform volume = (StructuredVolume *uniform) _volume;

  // Gradient step in each dimension (world coordinates).
  const uniform vec3f gradientStep = volume->gridSpacing;

  // The gradient will be computed using central differences.
  varying vec3f gradient;

#if 0
  // Central differences.

  // Gradient magnitude in the X direction.
  gradient.x = volume->super.computeSample(volume, location + make_vec3f(gradientStep.x, 0.0f, 0.0f)) - volume->super.computeSample(volume, location + make_vec3f(-gradientStep.x, 0.0f, 0.0f));

  // Gradient magnitude in the Y direction.
  gradient.y = volume->super.computeSample(volume, location + make_vec3f(0.0f, gradientStep.y, 0.0f)) - volume->super.computeSample(volume, location + make_vec3f(0.0f, -gradientStep.y, 0.0f));

  // Gradient magnitude in the Z direction.
  gradient.z = volume->super.computeSample(volume, location + make_vec3f(0.0f, 0.0f, gradientStep.z)) - volume->super.computeSample(volume, location + make_vec3f(0.0f, 0.0f, -gradientStep.z));

  // This approximation may yield image artifacts.
  return(0.5f * gradient / gradientStep);
#else
  // Forward differences.

  // Sample at gradient location.
  float sample = volume->super.computeSample(volume, worldCoordinates);

  // Gradient magnitude in the X direction.
  gradient.x = volume->super.computeSample(volume, worldCoordinates + make_vec3f(gradientStep.x, 0.0f, 0.0f)) - sample;

  // Gradient magnitude in the Y direction.
  gradient.y = volume->super.computeSample(volume, worldCoordinates + make_vec3f(0.0f, gradientStep.y, 0.0f)) - sample;

  // Gradient magnitude in the Z direction.
  gradient.z = volume->super.computeSample(volume, worldCoordinates + make_vec3f(0.0f, 0.0f, gradientStep.z)) - sample;

  // This approximation may yield image artifacts.
  return(gradient / gradientStep);
#endif
}

inline void StructuredVolume_intersect(void *uniform _volume, varying Ray &ray)
{
  // Cast to the actual Volume subtype.
  StructuredVolume *uniform volume = (StructuredVolume *uniform) _volume;

  // The recommended step size for ray casting based volume renderers.
  const uniform float step = volume->super.samplingStep / volume->super.samplingRate;

  // Compute the next hit point using a spatial acceleration structure.
  GridAccelerator_intersect(volume->accelerator, step, ray);

//ray.t0 += step;
}

inline void StructuredVolume_intersectIsosurface(void *uniform _volume, uniform float *uniform isovalues, uniform int numIsovalues, varying Ray &ray)
{
  // Cast to the actual Volume subtype.
  StructuredVolume *uniform volume = (StructuredVolume *uniform) _volume;

  // The nominal step size for ray casting based volume renderers, not considering the sampling rate.
  const uniform float step = volume->super.samplingStep;

  // Compute the next hit point using a spatial acceleration structure.
  GridAccelerator_intersectIsosurface(volume->accelerator, step, isovalues, numIsovalues, ray);
}

inline void StructuredVolume_transformLocalToWorld(StructuredVolume *uniform volume, const varying vec3f &localCoordinates, varying vec3f &worldCoordinates)
{
  worldCoordinates = volume->gridOrigin + localCoordinates * volume->gridSpacing;
}

inline void StructuredVolume_transformWorldToLocal(StructuredVolume *uniform volume, const varying vec3f &worldCoordinates, varying vec3f &localCoordinates)
{
  localCoordinates = rcp(volume->gridSpacing) * (worldCoordinates - volume->gridOrigin);
}

void StructuredVolume_Constructor(StructuredVolume *uniform volume,
                                  /*! pointer to the c++-equivalent class of this entity */
                                  void *uniform cppEquivalent,
                                  const uniform vec3i &dimensions
                                  )
{
  Volume_Constructor(&volume->super,cppEquivalent);

  volume->dimensions = dimensions;
  volume->accelerator = NULL;
  volume->localCoordinatesUpperBound = nextafter(volume->dimensions - 1, make_vec3i(0));
  volume->getVoxel = NULL;
  volume->transformLocalToWorld = StructuredVolume_transformLocalToWorld;
  volume->transformWorldToLocal = StructuredVolume_transformWorldToLocal;

  volume->super.boundingBox = make_box3f(volume->gridOrigin, volume->gridOrigin + make_vec3f(volume->dimensions - 1) * volume->gridSpacing);
  volume->super.computeSample = StructuredVolume_computeSample;
  volume->super.computeGradient = StructuredVolume_computeGradient;
  volume->super.intersect = StructuredVolume_intersect;
  volume->super.intersectIsosurface = StructuredVolume_intersectIsosurface;
}

export void StructuredVolume_setGridOrigin(void *uniform _self, const uniform vec3f &value)
{
  uniform StructuredVolume *uniform self = (uniform StructuredVolume *uniform)_self;
  self->gridOrigin = value;

  // Update bounding box for new grid definition.
  self->super.boundingBox = make_box3f(self->gridOrigin, self->gridOrigin + make_vec3f(self->dimensions - 1) * self->gridSpacing);
}

export void StructuredVolume_setGridSpacing(void *uniform _self, const uniform vec3f &value)
{
  uniform StructuredVolume *uniform self = (uniform StructuredVolume *uniform)_self;
  self->gridSpacing = value;

  // Update nominal sampling step size for new grid definition.
  self->super.samplingStep = reduce_min(self->gridSpacing);

  // Update bounding box for new grid definition.
  self->super.boundingBox = make_box3f(self->gridOrigin, self->gridOrigin + make_vec3f(self->dimensions - 1) * self->gridSpacing);
}

export void *uniform StructuredVolume_createAccelerator(void *uniform _self)
{
  // Cast to the actual Volume type.
  StructuredVolume *uniform self = (StructuredVolume *uniform)_self;

  // Delete the current accelerator if it exists.
  if(self->accelerator) GridAccelerator_destroy(self->accelerator);

  // Set the accelerator structure field.
  self->accelerator = GridAccelerator_createInstance(&self->super);

  return self->accelerator;
}
