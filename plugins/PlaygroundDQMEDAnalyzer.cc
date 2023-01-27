#include "Validation/PlaygroundDQMEDAnalyzer/interface/PlaygroundDQMEDAnalyzer.h"

PlaygroundDQMEDAnalyzer::PlaygroundDQMEDAnalyzer(const edm::ParameterSet& iConfig)
    : folder_(iConfig.getParameter<std::string>("folder")),
      myTag(iConfig.getParameter<std::string>( "DataType" )),
      calibration_flags(iConfig.getParameter<std::vector<int> >( "CalibrationFlags" ))
{}

PlaygroundDQMEDAnalyzer::~PlaygroundDQMEDAnalyzer() {}

// ------------ method called for each event  ------------
void PlaygroundDQMEDAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  eventCount_++;

  example_->Fill(5);
  example2D_->Fill(eventCount_ / 10, eventCount_ / 10);
  example3D_->Fill(eventCount_ / 10, eventCount_ / 10, eventCount_ / 10.f);
  exampleTProfile_->Fill(eventCount_ / 10, eventCount_ / 10.f);
  exampleTProfile2D_->Fill(eventCount_ / 10, eventCount_ / 10, eventCount_ / 10.f);
}

void PlaygroundDQMEDAnalyzer::bookHistograms(DQMStore::IBooker& ibook, edm::Run const& run, edm::EventSetup const& iSetup) {
  ibook.setCurrentFolder(folder_);

  example_ = ibook.book1D("EXAMPLE", "Example 1D", 20, 0., 10.);
  example2D_ = ibook.book2D("EXAMPLE_2D", "Example 2D", 20, 0, 20, 15, 0, 15);
  example3D_ = ibook.book3D("EXAMPLE_3D", "Example 3D", 20, 0, 20, 15, 0, 15, 25, 0, 25);
  exampleTProfile_ = ibook.bookProfile("EXAMPLE_TPROFILE", "Example TProfile", 20, 0, 20, 15, 0, 15);
  exampleTProfile2D_ = ibook.bookProfile2D("EXAMPLE_TPROFILE2D", "Example TProfile 2D", 20, 0, 20, 15, 0, 15, 0, 100);
}



// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void PlaygroundDQMEDAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("folder", "HGCAL/RecHits");
  desc.add<std::string>("DataType", "beam");
  desc.add<std::vector<int>>("CalibrationFlags", {1, 1, 0, 0, 0, 0, 0, 0, 0, 0});
  descriptions.add("playgrounddqmedanalyzer", desc);
  //---------- Definitions of calibration flags ----------#
  // calibration_flags[0]: pedestal subtraction
  // calibration_flags[1]: cm subtraction
  // calibration_flags[2]: BX-1 correction
  // calibration_flags[3]: gain linearization
  // calibration_flags[4]: charge collection efficiency
  // calibration_flags[5]: MIP scale
  // calibration_flags[6]: EM scale
  // calibration_flags[7]: zero suppression
  // calibration_flags[8]: hit energy calibration
  // calibration_flags[9]: ToA conversion
  //------------------------------------------------------#
}

// define this as a plug-in
DEFINE_FWK_MODULE(PlaygroundDQMEDAnalyzer);
