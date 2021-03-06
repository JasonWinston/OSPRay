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

#pragma once

#include <ospray/ospray.h>
#include "common/box.h"
#include <QtGui>
#include <vector>

class VolumeViewer;

class PreferencesDialog : public QDialog
{
Q_OBJECT

public:

  PreferencesDialog(VolumeViewer *volumeViewer, ospcommon::box3f boundingBox);

protected slots:

  void updateVolumeClippingBox();

protected:

  VolumeViewer *volumeViewer;

  std::vector<QDoubleSpinBox *> volumeClippingBoxSpinBoxes;
};
