// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file    DatDecoderSpec.cxx
/// \author  Andrea Ferrero
///
/// \brief Implementation of a data processor to run the raw decoding
///

#include <random>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <array>
#include <functional>

#include "Framework/CallbackService.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/ControlService.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/Lifetime.h"
#include "Framework/Output.h"
#include "Framework/Task.h"
#include "Framework/WorkflowSpec.h"

#include "Headers/RAWDataHeader.h"
#include "DetectorsRaw/RDHUtils.h"
#include "DPLUtils/DPLRawParser.h"
#include "CommonConstants/LHCConstants.h"
#include "DetectorsRaw/HBFUtils.h"

#include "DataFormatsMCH/Digit.h"
#include "MCHRawCommon/DataFormats.h"
#include "MCHRawDecoder/DataDecoder.h"
#include "MCHRawDecoder/ROFFinder.h"
#include "MCHRawElecMap/Mapper.h"
#include "MCHMappingInterface/Segmentation.h"
#include "MCHWorkflow/DataDecoderSpec.h"

//#define MCH_RAW_DATADECODER_DEBUG_DIGIT_TIME 1

namespace o2
{
namespace mch
{
namespace raw
{

using namespace o2;
using namespace o2::framework;
using namespace o2::mch::mapping;
using RDH = o2::header::RDHAny;

//=======================
// Data decoder
class DataDecoderTask
{
 public:
  DataDecoderTask(std::string spec) : mInputSpec(spec) {}

  //_________________________________________________________________________________________________
  void init(framework::InitContext& ic)
  {
    SampaChannelHandler channelHandler;
    RdhHandler rdhHandler;

    auto ds2manu = ic.options().get<bool>("ds2manu");
    auto sampaBcOffset = ic.options().get<int>("sampa-bc-offset");
    mDebug = ic.options().get<bool>("debug");
    mCheckROFs = ic.options().get<bool>("check-rofs");
    mDummyROFs = ic.options().get<bool>("dummy-rofs");
    auto mapCRUfile = ic.options().get<std::string>("cru-map");
    auto mapFECfile = ic.options().get<std::string>("fec-map");
    auto useDummyElecMap = ic.options().get<bool>("dummy-elecmap");
    mDecoder = new DataDecoder(channelHandler, rdhHandler, sampaBcOffset, mapCRUfile, mapFECfile, ds2manu, mDebug,
                               useDummyElecMap);
  }

  //_________________________________________________________________________________________________
  // the decodeTF() function processes the messages generated by the (sub)TimeFrame builder
  void decodeTF(framework::ProcessingContext& pc)
  {
    const auto* dh = o2::header::get<o2::header::DataHeader*>(pc.inputs().getByPos(0).header);
    mFirstTForbit = dh->firstTForbit;

    if (!mDecoder->getFirstOrbitInRun()) {
      uint32_t firstRunOrbit = mFirstTForbit - dh->tfCounter * o2::raw::HBFUtils::Instance().getNOrbitsPerTF();
      mDecoder->setFirstOrbitInRun(firstRunOrbit);
    }
    mDecoder->setFirstOrbitInTF(mFirstTForbit);

    if (mDebug) {
      std::cout << "[DataDecoderSpec::run] first run orbit is " << mDecoder->getFirstOrbitInRun().value() << std::endl;
      std::cout << "[DataDecoderSpec::run] first TF orbit is " << mFirstTForbit << std::endl;
    }

    // get the input buffer
    auto& inputs = pc.inputs();
    DPLRawParser parser(inputs, o2::framework::select(mInputSpec.c_str()));
    for (auto it = parser.begin(), end = parser.end(); it != end; ++it) {
      auto const* raw = it.raw();
      if (!raw) {
        continue;
      }
      size_t payloadSize = it.size();

      gsl::span<const std::byte> buffer(reinterpret_cast<const std::byte*>(raw), sizeof(RDH) + payloadSize);
      mDecoder->decodeBuffer(buffer);
    }
  }

  //_________________________________________________________________________________________________
  // the decodeReadout() function processes the messages generated by o2-mch-cru-page-reader-workflow
  void decodeReadout(const o2::framework::DataRef& input)
  {
    static int nFrame = 1;
    // get the input buffer
    if (input.spec->binding != "readout") {
      return;
    }

    const auto* header = o2::header::get<header::DataHeader*>(input.header);
    if (!header) {
      return;
    }

    auto const* raw = input.payload;
    // size of payload
    size_t payloadSize = header->payloadSize;

    if (mDebug) {
      std::cout << nFrame << "  payloadSize=" << payloadSize << std::endl;
    }
    nFrame += 1;
    if (payloadSize == 0) {
      return;
    }

    const RDH* rdh = reinterpret_cast<const RDH*>(raw);
    mFirstTForbit = o2::raw::RDHUtils::getHeartBeatOrbit(rdh);
    if (!mDecoder->getFirstOrbitInRun()) {
      mDecoder->setFirstOrbitInRun(mFirstTForbit);
    }
    mDecoder->setFirstOrbitInTF(mFirstTForbit);

    gsl::span<const std::byte> buffer(reinterpret_cast<const std::byte*>(raw), payloadSize);
    mDecoder->decodeBuffer(buffer);
  }

  //_________________________________________________________________________________________________
  void run(framework::ProcessingContext& pc)
  {
    static int32_t deltaMax = 0;

    auto createBuffer = [&](auto& vec, size_t& size) {
      size = vec.empty() ? 0 : sizeof(*(vec.begin())) * vec.size();
      char* buf = nullptr;
      if (size > 0) {
        buf = (char*)malloc(size);
        if (buf) {
          char* p = buf;
          size_t sizeofElement = sizeof(*(vec.begin()));
          for (auto& element : vec) {
            memcpy(p, &element, sizeofElement);
            p += sizeofElement;
          }
        }
      }
      return buf;
    };

    mDecoder->reset();

    for (auto&& input : pc.inputs()) {
      if (input.spec->binding == "readout") {
        decodeReadout(input);
      }
      if (input.spec->binding == "TF") {
        decodeTF(pc);
      }
    }
    mDecoder->computeDigitsTime();

    auto& digits = mDecoder->getDigits();
    auto& orbits = mDecoder->getOrbits();

#ifdef MCH_RAW_DATADECODER_DEBUG_DIGIT_TIME
    constexpr int BCINORBIT = o2::constants::lhc::LHCMaxBunches;
    int32_t bcMax = mNHBPerTF * 3564 - 1;
    for (auto d : digits) {
      if (d.getTime() < 0 || d.getTime() > bcMax) {
        int32_t delta = d.getTime() - bcMax;
        if (delta > deltaMax) {
          deltaMax = delta;
          std::cout << "Max digit time exceeded: TF ORBIT " << mDecoder->getFirstOrbitInTF().value() << "  DE# " << d.getDetID() << " PadId " << d.getPadID() << " ADC " << d.getADC()
                    << " time " << d.getTime() << " (" << bcMax << ", delta=" << delta << ")" << std::endl;
        }
      }
    }
#endif

    ROFFinder rofFinder(digits, mFirstTForbit);
    rofFinder.process(mDummyROFs);

    if (mDebug) {
      rofFinder.dumpOutputDigits();
      rofFinder.dumpOutputROFs();
    }

    if (mCheckROFs) {
      rofFinder.isRofTimeMonotonic();
      rofFinder.isDigitsTimeAligned();
    }

    // send the output buffer via DPL
    size_t digitsSize, rofsSize, orbitsSize;
    char* digitsBuffer = rofFinder.saveDigitsToBuffer(digitsSize);
    char* rofsBuffer = rofFinder.saveROFRsToBuffer(rofsSize);
    char* orbitsBuffer = createBuffer(orbits, orbitsSize);

    if (mDebug) {
      std::cout << "digitsSize " << digitsSize << "  rofsSize " << rofsSize << "  orbitsSize " << orbitsSize << std::endl;
    }

    // create the output message
    auto freefct = [](void* data, void*) { free(data); };
    pc.outputs().adoptChunk(Output{header::gDataOriginMCH, "DIGITS", 0}, digitsBuffer, digitsSize, freefct, nullptr);
    pc.outputs().adoptChunk(Output{header::gDataOriginMCH, "DIGITROFS", 0}, rofsBuffer, rofsSize, freefct, nullptr);
    pc.outputs().adoptChunk(Output{header::gDataOriginMCH, "ORBITS", 0}, orbitsBuffer, orbitsSize, freefct, nullptr);
  }

 private:
  std::string mInputSpec;            /// selection string for the input data
  bool mDebug = {false};             /// flag to enable verbose output
  bool mCheckROFs = {false};         /// flag to enable consistency checks on the output ROFs
  bool mDummyROFs = {false};         /// flag to disable the ROFs finding
  uint32_t mFirstTForbit{0};         /// first orbit of the time frame being processed
  DataDecoder* mDecoder = {nullptr}; /// pointer to the data decoder instance
};

//_________________________________________________________________________________________________
o2::framework::DataProcessorSpec getDecodingSpec(std::string inputSpec)
{
  o2::mch::raw::DataDecoderTask task(inputSpec);
  return DataProcessorSpec{
    "DataDecoder",
    o2::framework::select(inputSpec.c_str()),
    Outputs{OutputSpec{header::gDataOriginMCH, "DIGITS", 0, Lifetime::Timeframe},
            OutputSpec{header::gDataOriginMCH, "DIGITROFS", 0, Lifetime::Timeframe},
            OutputSpec{header::gDataOriginMCH, "ORBITS", 0, Lifetime::Timeframe}},
    AlgorithmSpec{adaptFromTask<DataDecoderTask>(std::move(task))},
    Options{{"debug", VariantType::Bool, false, {"enable verbose output"}},
            {"cru-map", VariantType::String, "", {"custom CRU mapping"}},
            {"fec-map", VariantType::String, "", {"custom FEC mapping"}},
            {"dummy-elecmap", VariantType::Bool, false, {"use dummy electronic mapping (for debug, temporary)"}},
            {"ds2manu", VariantType::Bool, false, {"convert channel numbering from Run3 to Run1-2 order"}},
            {"sampa-bc-offset", VariantType::Int, 339986, {"SAMPA bunch-crossing counter at the beginning of the run"}},
            {"check-rofs", VariantType::Bool, false, {"perform consistency checks on the output ROFs"}},
            {"dummy-rofs", VariantType::Bool, false, {"disable the ROFs finding algorithm"}}}};
}

} // namespace raw
} // namespace mch
} // end namespace o2
