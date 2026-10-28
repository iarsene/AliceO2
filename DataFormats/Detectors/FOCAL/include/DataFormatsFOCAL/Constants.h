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
#ifndef ALICEO2_FOCAL_CONSTANTS_H
#define ALICEO2_FOCAL_CONSTANTS_H

namespace o2::focal::constants
{
// HCal
constexpr int HCAL_NUM_CHANNELS_PER_ROC_HALF = 36;
constexpr int HCAL_NUM_GBT_LINKS = 2;
constexpr int HCAL_NUM_ROCS_PER_LINK = 2;
constexpr int HCAL_NUM_SAMPLES_PER_EVENT = 16;
constexpr int HCAL_NUM_GBT_LINES_PER_LINK = 40;
constexpr int HCAL_NUM_COLUMNS = 16;
constexpr int HCAL_NUM_ROWS = 16;
constexpr double HCAL_TIMEWINDOW = HCAL_NUM_SAMPLES_PER_EVENT * 25.0; // each sample is taken 25ns apart, total time window is dependent on how many samples

// Pads
constexpr int PADLAYER_MODULE_NCHANNELS = 72;
constexpr int PADLAYER_MODULE_NHALVES = 2;
constexpr int PADLAYER_WINDOW_LENGTH = 20;
constexpr int PADS_NLAYERS = 20;

// Pixels
constexpr int PIXELS_NLAYERS = 2;

} // namespace o2::focal::constants

#endif // ALICEO2_FOCAL_CONSTANTS_H