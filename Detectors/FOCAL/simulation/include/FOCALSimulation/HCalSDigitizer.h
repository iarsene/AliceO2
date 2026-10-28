// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ALICEO2_FOCAL_HCALSDIGITIZER_H
#define ALICEO2_FOCAL_HCALSDIGITIZER_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <list>

#include "Rtypes.h"  // for SDigitizer::Class, Double_t, ClassDef, etc
#include "TObject.h" // for TObject

#include "DataFormatsFOCAL/HCalDigit.h"
#include "FOCALBase/Geometry.h"
#include "FOCALBase/Hit.h"
#include "FOCALSimulation/HCalLabeledDigit.h"

#include "SimulationDataFormat/MCTruthContainer.h"

namespace o2
{
namespace focal
{

/// \class SDigitizer
/// \brief FOCAL HCal summed digitizer
/// \ingroup FOCALsimulation

class HCalSDigitizer
{
 public:
  HCalSDigitizer() = default;
  ~HCalSDigitizer() = default;
  HCalSDigitizer(const HCalSDigitizer&) = delete;
  HCalSDigitizer& operator=(const HCalSDigitizer&) = delete;

  /// Steer conversion of hits to digits
  std::vector<o2::focal::HCalLabeledDigit> process(const std::vector<Hit>& hits);

  void setCurrSrcID(int v);
  int getCurrSrcID() const { return mCurrSrcID; }

  void setCurrEvID(int v);
  int getCurrEvID() const { return mCurrEvID; }

  void setGeometry(const o2::focal::Geometry* gm) { mGeometry = gm; }

 private:
  const Geometry* mGeometry = nullptr; // FOCAL geometry
  int mCurrSrcID = 0;                  // current MC source from the manager
  int mCurrEvID = 0;                   // current event ID from the manager

  ClassDefNV(HCalSDigitizer, 1);
};
} // namespace focal
} // namespace o2

#endif /* ALICEO2_FOCAL_HCALSDIGITIZER_H */
