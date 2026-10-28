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

#include <iostream>
#include "FOCALSimulation/HCalLabeledDigit.h"

using namespace o2::focal;

HCalLabeledDigit::HCalLabeledDigit(HCalDigit digit, MCLabel label)
  : mDigit(digit)
{
  mLabels.push_back(label);
}

HCalLabeledDigit::HCalLabeledDigit(Int_t tower, Double_t amplitude, Double_t time, MCLabel label)
  : mDigit(tower, amplitude, time)
{
  mLabels.push_back(label);
}

HCalLabeledDigit& HCalLabeledDigit::operator+=(const HCalLabeledDigit& other)
{
  if (canAdd(other)) {
    double a1 = getAmplitude();
    double a2 = other.getAmplitude();
    double r = ((a1 + a2) != 0) ? 1.0 / (a1 + a2) : 0.0;
    mDigit += other.getDigit();

    for (int j = 0; j < mLabels.size(); j++) {
      mLabels.at(j).setAmplitudeFraction(mLabels.at(j).getAmplitudeFraction() * a1 * r);
    }

    for (const auto& label : other.getLabels()) {
      mLabels.push_back(label);
    }
  }

  return *this;
}

void HCalLabeledDigit::PrintStream(std::ostream& stream) const
{
  stream << "Tower: " << getTower() << ", Amplitude: " << getAmplitude() << ", TimeStamp: " << getTimeStamp() << ", Labels: ";
  for (const auto& label : getLabels()) {
    stream << label << " ";
  }
}

std::ostream& o2::focal::operator<<(std::ostream& stream, HCalLabeledDigit digit)
{
  digit.PrintStream(stream);
  return stream;
}
