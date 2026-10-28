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

#ifndef ALICEO2_FOCAL_HCALDIGIT_H_
#define ALICEO2_FOCAL_HCALDIGIT_H_

#include <iosfwd>
#include <cmath>
#include "Rtypes.h"
#include "CommonDataFormat/TimeStamp.h"
#include "DataFormatsFOCAL/Constants.h"

#include <boost/serialization/base_object.hpp> // for base_object

namespace o2
{

namespace focal
{
using DigitBase = o2::dataformats::TimeStamp<double>;

/// \class HCalDigit
/// \brief FOCAL HCal digit implementation
/// \ingroup FOCALDataFormat
class HCalDigit : public DigitBase
{
 public:
  HCalDigit() = default;
  HCalDigit(Int_t tower, Double_t amplitude, Double_t time);

  void setTower(Int_t tower) { mTower = tower; }
  Int_t getTower() const { return mTower; }

  void setAmplitude(Double_t amplitude) { mAmplitude = amplitude; }
  Double_t getAmplitude() const; // returns energy of digit with the addition of simulated electronics noise (in EMCAL - still determining if this is how we want to do it here)

  void setEnergy(Double_t energy) { mAmplitude = energy; }
  Double_t getEnergy() const { return mAmplitude; } // returns base energy of the digit

  bool canAdd(const HCalDigit other) // checks if two digits can be added, i.e. if they belong to the same tower in the same time window
  {
    return (mTower == other.getTower() && std::abs(getTimeStamp() - other.getTimeStamp()) < constants::HCAL_TIMEWINDOW);
  }

  HCalDigit& operator+=(const HCalDigit& other);

 private:
  Int_t mTower = -1;
  Double_t mAmplitude = 0;

  ClassDefNV(HCalDigit, 1);
};

std::ostream& operator<<(std::ostream& stream, const HCalDigit& dig);

} // namespace focal
} // namespace o2

#endif // ALICEO2_FOCAL_HCALDIGIT_H_