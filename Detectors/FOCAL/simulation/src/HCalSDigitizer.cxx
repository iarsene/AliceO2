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

#include "FOCALSimulation/HCalSDigitizer.h"
#include "FOCALSimulation/HCalLabeledDigit.h"
#include "DataFormatsFOCAL/HCalDigit.h"
#include "FOCALBase/Geometry.h"
#include "FOCALBase/Hit.h"
#include "MathUtils/Cartesian.h"
#include "SimulationDataFormat/MCCompLabel.h"

#include <climits>
#include <list>
#include <map>
#include <chrono>
#include <numeric>
#include <fairlogger/Logger.h> // for LOG

ClassImp(o2::focal::HCalSDigitizer);

using o2::focal::HCalDigit;
using o2::focal::Hit;

using namespace o2::focal;

// Process hits and convert them to summed digits

std::vector<o2::focal::HCalLabeledDigit> HCalSDigitizer::process(const std::vector<Hit>& hits)
{

  std::map<int, std::map<int, std::vector<o2::focal::Hit>>> hitsPerTowerPerParticleID;

  // will be used to sort digits and labels by tower
  std::unordered_map<Int_t, std::vector<HCalLabeledDigit>> digitsPerTower;

  for (auto hit : hits) {
    if (hit.isHCALHit()) {
      hitsPerTowerPerParticleID[hit.GetDetectorID()][hit.GetTrackID()].push_back(hit);
    }
  }

  std::vector<o2::focal::Hit> SHits;
  for (auto [towerID, hitsParticle] : hitsPerTowerPerParticleID) {
    for (auto [partID, Hits] : hitsParticle) {
      o2::focal::Hit SHit = std::accumulate(std::next(Hits.begin()), Hits.end(), Hits.front());
      SHits.push_back(SHit);
    }
  }

  for (auto hit : SHits) {
    Int_t tower = hit.GetDetectorID();

    if (tower < 0 || tower > mGeometry->getHCALTowersInX() * mGeometry->getHCALTowersInY()) {
      auto [indetector, col, row, layer, segment] = mGeometry->getVirtualInfo(hit.GetX(), hit.GetY(), hit.GetZ());
      int nCol = mGeometry->getHCALTowersInX();
      int recomputedTower = row * nCol + col;
      LOG(warning) << "hit " << hit.GetTrackID() << " in event " << mCurrEvID << " tower index out of range: " << tower
                   << " (indetector=" << indetector << ", segment=" << segment << ", col=" << col << ", row=" << row
                   << ", nCol=" << nCol << ", recomputed tower=" << recomputedTower << ")";
      if (!indetector) {
        // position doesn't map into the detector at all - not a tower-encoding problem
        LOG(warning) << "  hit position (" << hit.GetX() << ", " << hit.GetY() << ", " << hit.GetZ() << ") falls outside the detector volume";
      } else if (recomputedTower != tower) {
        // stored detID disagrees with what the current geometry would assign - encoding mismatch, not a geometry problem
        LOG(warning) << "  stored tower (" << tower << ") != tower recomputed from position (" << recomputedTower << "): detID/column encoding mismatch between Detector::ProcessHitsHCAL and HCalSDigitizer";
      }
      continue;
    }

    Double_t energy = hit.GetEnergyLoss();

    HCalDigit digit(tower, energy, hit.GetTime());

    MCLabel label(hit.GetTrackID(), mCurrEvID, mCurrSrcID, false, 1.0);
    if (digit.getAmplitude() < __DBL_EPSILON__) { // __DBL_EPSILON__ is the smallest possible float for which 1.0 + __DBL_EPSILON__ != 1.0, meaning this is essentially checking for negligible amplitude contributions
      label.setAmplitudeFraction(0);
    }
    HCalLabeledDigit d(digit, label);

    digitsPerTower[tower].push_back(d);
  }
  std::vector<o2::focal::HCalLabeledDigit> digits;

  // Sum all digits in one tower
  for (auto& [tower, labeledDigits] : digitsPerTower) {
    if (labeledDigits.empty()) {
      continue;
    }

    o2::focal::HCalLabeledDigit SDigit = std::accumulate(std::next(labeledDigits.begin()), labeledDigits.end(), labeledDigits.front());

    if (SDigit.getDigit().getAmplitude() < __DBL_EPSILON__) {
      continue;
    }
    digits.push_back(SDigit);
  }

  digitsPerTower.clear();

  return digits;
}

// Setting source and event id for the current MC truth labels
void HCalSDigitizer::setCurrSrcID(int v)
{
  // set current MC source ID
  if (v > MCCompLabel::maxSourceID()) {
    LOG(fatal) << "MC source id " << v << " exceeds max storable in the label " << MCCompLabel::maxSourceID();
  }
  mCurrSrcID = v;
}

void HCalSDigitizer::setCurrEvID(int v)
{
  // set current MC event ID
  if (v > MCCompLabel::maxEventID()) {
    LOG(fatal) << "MC event id " << v << " exceeds max storable in the label " << MCCompLabel::maxEventID();
  }
  mCurrEvID = v;
}