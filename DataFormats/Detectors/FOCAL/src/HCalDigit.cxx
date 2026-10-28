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

#include "DataFormatsFOCAL/HCalDigit.h"
#include <iostream>

using namespace o2::focal;

HCalDigit::HCalDigit(Int_t tower, Double_t amplitude, Double_t time)
  : DigitBase(time), mTower(tower), mAmplitude(amplitude)
{
}

Double_t HCalDigit::getAmplitude() const
{
  // To be added: electronics noise
  return mAmplitude;
}

HCalDigit& HCalDigit::operator+=(const HCalDigit& other)
{
  if (canAdd(other)) {
    mAmplitude += other.mAmplitude;
  }
  return *this;
}

std::ostream& operator<<(std::ostream& stream, const HCalDigit& dig)
{
  stream << "HCalDigit(Tower: " << dig.getTower() << ", Amplitude: " << dig.getAmplitude() << ", Time: " << dig.getTimeStamp() << ")";
  return stream;
}
