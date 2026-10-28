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

#ifndef ALICEO2_FOCAL_HCALLABELEDDIGIT_H_
#define ALICEO2_FOCAL_HCALLABELEDDIGIT_H_

#include <iosfwd>
#include <cmath>
#include <vector>
#include "Rtypes.h"
#include "CommonDataFormat/TimeStamp.h"
#include "DataFormatsFOCAL/Constants.h"
#include "DataFormatsFOCAL/HCalDigit.h"
#include "DataFormatsFOCAL/MCLabel.h"

#include <boost/serialization/base_object.hpp> // for base_object

namespace o2
{

namespace focal
{
/// \class HCalLabeledDigit
/// \brief FOCAL HCal labeled digit implementation
/// \ingroup FOCALsimulation

class HCalLabeledDigit
{
 public:
  HCalLabeledDigit() = default;
  HCalLabeledDigit(HCalDigit digit, o2::focal::MCLabel label);
  HCalLabeledDigit(Int_t tower, Double_t amplitude, Double_t time, o2::focal::MCLabel label);
  ~HCalLabeledDigit() = default; // override

  void setDigit(HCalDigit digit) { mDigit = digit; }
  HCalDigit getDigit() const { return mDigit; }

  void addLabel(MCLabel l) { mLabels.push_back(l); }
  Int_t getNumberOfLabels() const { return mLabels.size(); }
  std::vector<o2::focal::MCLabel> getLabels() const { return mLabels; }

  void setTower(Int_t tower) { mDigit.setTower(tower); }
  Int_t getTower() const { return mDigit.getTower(); }

  void setAmplitude(Double_t amplitude) { mDigit.setAmplitude(amplitude); }
  Double_t getAmplitude() const { return mDigit.getAmplitude(); } // returns energy of digit with the addition of simulated electronics noise (in EMCAL - still determining if this is how we want to do it here)

  void setEnergy(Double_t energy) { mDigit.setEnergy(energy); }
  Double_t getEnergy() const { return mDigit.getEnergy(); } // returns base energy of the digit

  void setTimeStamp(Double_t time) { mDigit.setTimeStamp(time); }
  Double_t getTimeStamp() const { return mDigit.getTimeStamp(); }

  bool operator<(const HCalLabeledDigit& other) const { return getTimeStamp() < other.getTimeStamp(); }
  bool operator>(const HCalLabeledDigit& other) const { return getTimeStamp() > other.getTimeStamp(); }
  bool operator==(const HCalLabeledDigit& other) const { return (getTimeStamp() == other.getTimeStamp()); }
  bool canAdd(const HCalLabeledDigit& other) // checks if two digits can be added, i.e. if they belong to the same tower in the same time window
  {
    return (getTower() == other.getTower() && std::abs(getTimeStamp() - other.getTimeStamp()) < constants::HCAL_TIMEWINDOW);
  }
  HCalLabeledDigit& operator+=(const HCalLabeledDigit& other);
  friend HCalLabeledDigit operator+(HCalLabeledDigit lhs, const HCalLabeledDigit& rhs) // Adds amplitude of two digits, combines lists of labels
  {
    lhs += rhs;
    return lhs;
  }

  void PrintStream(std::ostream& stream) const;

 private:
  friend class boost::serialization::access;

  HCalDigit mDigit;
  std::vector<o2::focal::MCLabel> mLabels;

  ClassDefNV(HCalLabeledDigit, 1);
};

std::ostream& operator<<(std::ostream& stream, HCalLabeledDigit digit);

} // namespace focal

} // namespace o2

#endif // ALICEO2_FOCAL_HCALLABELEDDIGIT_H_