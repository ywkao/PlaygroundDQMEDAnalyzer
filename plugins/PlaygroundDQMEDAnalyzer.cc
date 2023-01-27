#include "Validation/PlaygroundDQMEDAnalyzer/interface/PlaygroundDQMEDAnalyzer.h"

PlaygroundDQMEDAnalyzer::PlaygroundDQMEDAnalyzer(const edm::ParameterSet& iConfig)
    : folder_(iConfig.getParameter<std::string>("folder")),
    myTag(iConfig.getParameter<std::string>( "DataType" )),
    calibration_flags(iConfig.getParameter<std::vector<int> >( "CalibrationFlags" ))
{
    // load trees from beam data / pedestal run
    TString root_beamRun  = "/eos/cms/store/group/dpg_hgcal/tb_hgcal/2022/sps_oct2022/pion_beam_150_320fC/beam_run/run_20221007_191926/beam_run0.root";
    TString root_pedestal = "/eos/cms/store/group/dpg_hgcal/tb_hgcal/2022/sps_oct2022/pedestals/pedestal_320fC/pedestal_run/run_20221008_192720/pedestal_run0.root";
    TString input = (myTag=="beam") ? root_beamRun : root_pedestal;
    printf("[INFO] Input rootfile: %s\n", input.Data());

    f1 = new TFile(input, "R");
    t1 = (TTree*) f1->Get("unpacker_data/hgcroc");
    Init(t1); // SetBranchAddress, init variables, etc.

    // determine which calibrations to perform
    if(calibration_flags[0]) enable_pedestal_subtraction();
    if(calibration_flags[1]) enable_cm_subtraction();

    Load_metaData();
}

PlaygroundDQMEDAnalyzer::~PlaygroundDQMEDAnalyzer() {

    // The following code will cause exception messages
    // terminate called after throwing an instance of 'cms::Exception'
    //   what():  An exception of category 'LogicError' occurred.
    //   Exception Message:
    //   MonitorElement h_correlation not backed by any data!

    /*
    // summary for running statistics
    std::vector<RunningStatistics> mRs = myRunStatCollection.get_vector_running_statistics();
    for(int channelId=0; channelId<234; ++channelId) {
        h_correlation -> setBinContent( channelId+1, mRs[channelId].get_correlation() );
        h_slope       -> setBinContent( channelId+1, mRs[channelId].get_slope()       );
        h_intercept   -> setBinContent( channelId+1, mRs[channelId].get_intercept()   );
    }
    */
}

// ------------ method called for each event  ------------
void PlaygroundDQMEDAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;

    eventCount_++;

    example_->Fill(5);
    example2D_->Fill(eventCount_ / 10, eventCount_ / 10);
    example3D_->Fill(eventCount_ / 10, eventCount_ / 10, eventCount_ / 10.f);
    exampleTProfile_->Fill(eventCount_ / 10, eventCount_ / 10.f);
    exampleTProfile2D_->Fill(eventCount_ / 10, eventCount_ / 10, eventCount_ / 10.f);

    //--------------------------------------------------
    // migrate from a standalone c++ code
    //--------------------------------------------------
    Long64_t nentries = fChain->GetEntriesFast();
    int nevent = nentries / 78;

    std::cout << ">>> nentries = " << nentries << std::endl;
    std::cout << ">>> nevent = " << nevent << std::endl;

    // loop over hit
    std::vector<Hit> hits[nevent];
    int current_half      = 0;
    int recorded_half     = 0;
    double adc_channel_37 = 0.;

    Long64_t nbytes = 0, nb = 0;
    for (Long64_t jentry=0; jentry<nentries;jentry++) {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        nb = fChain->GetEntry(jentry); nbytes += nb;

        // reset cm adc when reaching another ROC half
        current_half = half;
        if(current_half != recorded_half) {
            recorded_half = current_half;
            adc_channel_CM = 0.;
        }

        // get detId & mask bad channel
        auto detid = DetectorId( FromRawData(), chip, half, channel );
        globalChannelId = detid.id(); // chip*78+half*39+channel;
        bool is_bad_channel = globalChannelId==146 || globalChannelId==171;
        if(is_bad_channel) continue;

        // convert adc to double
        adc_double = (double) adc;

        // perform pedestal subtraction
        if(flag_perform_pedestal_subtraction) {
            double pedestal = map_pedestals[globalChannelId];
            adc_double -= pedestal;
        }

        // handle cm information after pedestal subtraction
        bool is_cm_channel = (globalChannelId % 39 == 37 || globalChannelId % 39 == 38);
        if(is_cm_channel) {
            // take average of two cm channels in a half
            adc_channel_CM += adc_double / 2.;
        }

        // record adc of ch37 & fill info of ch37 when processing ch38
        if(globalChannelId % 39 == 37) {
            adc_channel_37 = adc_double;
            continue;

        } else if(globalChannelId % 39 == 38) {
            // CM subtraction for channel 37
            if(flag_perform_cm_subtraction) {
                std::vector<double> parameters = map_cm_parameters[globalChannelId-1];
                double slope = parameters[0];
                double intercept = parameters[1];
                double correction = adc_channel_CM*slope + intercept;
                adc_channel_37 -= correction;
            }

            fill_profiles(globalChannelId-1, adc_channel_37);
        }

        // perform common mode subtraction
        if(flag_perform_cm_subtraction) {
            std::vector<double> parameters = map_cm_parameters[globalChannelId];
            double slope = parameters[0];
            double intercept = parameters[1];
            double correction = adc_channel_CM*slope + intercept;
            adc_double -= correction;
        }

        fill_profiles(globalChannelId, adc_double);

        if(globalChannelId==7) fill_histograms();
    }
}

void PlaygroundDQMEDAnalyzer::bookHistograms(DQMStore::IBooker& ibook, edm::Run const& run, edm::EventSetup const& iSetup) {
    ibook.setCurrentFolder(folder_);

    example_ = ibook.book1D("EXAMPLE", "Example 1D", 20, 0., 10.);
    example2D_ = ibook.book2D("EXAMPLE_2D", "Example 2D", 20, 0, 20, 15, 0, 15);
    example3D_ = ibook.book3D("EXAMPLE_3D", "Example 3D", 20, 0, 20, 15, 0, 15, 25, 0, 25);
    exampleTProfile_ = ibook.bookProfile("EXAMPLE_TPROFILE", "Example TProfile", 20, 0, 20, 15, 0, 15);
    exampleTProfile2D_ = ibook.bookProfile2D("EXAMPLE_TPROFILE2D", "Example TProfile 2D", 20, 0, 20, 15, 0, 15, 0, 100);

    // an instance of distributions
    h_adc       = ibook.book1D("h_adc"      + tag_channelId , ";ADC;Entries"      , 175 , -25 , 150 );
    h_adcm      = ibook.book1D("h_adcm"     + tag_channelId , ";ADC-1;Entries"    , 550 , -50 , 500 );
    h_tot       = ibook.book1D("h_tot"      + tag_channelId , ";ToT;Entries"      , 100 , -2  , 2   );
    h_toa       = ibook.book1D("h_toa"      + tag_channelId , ";ToA;Entries"      , 500 , 0   , 500 );
    h_trigtime  = ibook.book1D("h_trigtime" + tag_channelId , ";trigtime;Entries" , 500 , 0   , 500 );

    // summary of physical quantities
    p_adc       = ibook.bookProfile("p_adc"      , ";channel;ADC"      , 234 , 0 , 234 , 175 , -25 , 150 );
    p_adcm      = ibook.bookProfile("p_adcm"     , ";channel;ADC-1"    , 234 , 0 , 234 , 550 , -50 , 500 );
    p_tot       = ibook.bookProfile("p_tot"      , ";channel;ToT"      , 234 , 0 , 234 , 100 , -2  , 2   );
    p_toa       = ibook.bookProfile("p_toa"      , ";channel;ToA"      , 234 , 0 , 234 , 500 , 0   , 500 );
    p_trigtime  = ibook.bookProfile("p_trigtime" , ";channel;trigtime" , 234 , 0 , 234 , 500 , 0   , 500 );
    p_status    = ibook.bookProfile("p_status"   , ";channel;status"   , 234 , 0 , 234 , 3   , -1  , 1   );

    // summary of running statistics
    h_correlation = ibook.book1D("h_correlation" , ";channel;Correlation" , 234 , -0.5 , 233.5);
    h_slope       = ibook.book1D("h_slope"       , ";channel;Slope"       , 234 , -0.5 , 233.5);
    h_intercept   = ibook.book1D("h_intercept"   , ";channel;Intercept"   , 234 , -0.5 , 233.5);
}

// ------------ auxilliary methods  ------------
Long64_t PlaygroundDQMEDAnalyzer::LoadTree(Long64_t entry)
{
    // Set the environment to read one entry
    if (!fChain) return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0) return centry;
    if (fChain->GetTreeNumber() != fCurrent) {
        fCurrent = fChain->GetTreeNumber();
    }
    return centry;
}

void PlaygroundDQMEDAnalyzer::Init(TTree *tree)
{
    if (tree == 0) printf("[ERROR] something goes wrong with input tree\n");

    if (!tree) return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);

    fChain->SetBranchAddress("event", &event, &b_event);
    fChain->SetBranchAddress("chip", &chip, &b_chip);
    fChain->SetBranchAddress("half", &half, &b_half);
    fChain->SetBranchAddress("channel", &channel, &b_channel);
    fChain->SetBranchAddress("adc", &adc, &b_adc);
    fChain->SetBranchAddress("adcm", &adcm, &b_adcm);
    fChain->SetBranchAddress("toa", &toa, &b_toa);
    fChain->SetBranchAddress("tot", &tot, &b_tot);
    fChain->SetBranchAddress("totflag", &totflag, &b_totflag);
    fChain->SetBranchAddress("trigtime", &trigtime, &b_trigtime);
    fChain->SetBranchAddress("trigwidth", &trigwidth, &b_trigwidth);
    fChain->SetBranchAddress("corruption", &corruption, &b_corruption);
    fChain->SetBranchAddress("bxcounter", &bxcounter, &b_bxcounter);
    fChain->SetBranchAddress("eventcounter", &eventcounter, &b_eventcounter);
    fChain->SetBranchAddress("orbitcounter", &orbitcounter, &b_orbitcounter);

    flag_perform_pedestal_subtraction = false;
    flag_perform_cm_subtraction = false;

    tag_calibration = "";
    tag_channelId = "_channel_7";
}

void PlaygroundDQMEDAnalyzer::enable_pedestal_subtraction() { flag_perform_pedestal_subtraction = true; tag_calibration = "_ped_subtracted"; }

void PlaygroundDQMEDAnalyzer::enable_cm_subtraction() { flag_perform_cm_subtraction = true; tag_calibration = "_cm_subtracted"; }

void PlaygroundDQMEDAnalyzer::Load_metaData()
{
    TString csv_file_name = "./meta_conditions/calibration_parameters.csv";
    printf("[INFO] Load calibration parameters: %s\n", csv_file_name.Data());

    std::string line;
    std::ifstream loaded_csv_file(csv_file_name.Data());

    if(loaded_csv_file.is_open()) {
        while(getline(loaded_csv_file, line)) {
            // skip comments
            if(line.find("#")!=std::string::npos) continue;

            std::size_t found_1st_index = line.find(",");
            std::size_t found_2nd_index = line.find(",", found_1st_index+1, 1);
            std::size_t found_3rd_index = line.find(",", found_2nd_index+1, 1);
            std::size_t found_4th_index = line.find(",", found_3rd_index+1, 1);

            int channel_id   = std::stoi( line.substr(0,found_1st_index) );
            double pedestal  = std::stod( line.substr(found_1st_index+1, found_2nd_index) );
            double slope     = std::stod( line.substr(found_2nd_index+1, found_3rd_index) );
            double intercept = std::stod( line.substr(found_3rd_index+1, found_4th_index) );

            std::vector<double> v = {slope, intercept};
            map_pedestals[channel_id] = pedestal;
            map_cm_parameters[channel_id] = v;

            printf("channel_id = %d, pedestal = %.3f, slope = %.3f, intercept = %6.3f\n",
                    channel_id, map_pedestals[channel_id], map_cm_parameters[channel_id][0], map_cm_parameters[channel_id][1] );
        }
        loaded_csv_file.close();
    } else {
        std::cout << "[ERROR] unable to open " << csv_file_name.Data() << std::endl;
    }
}

void PlaygroundDQMEDAnalyzer::fill_histograms()
{
    myRecorder.add_entry(adc_channel_CM, adc_double);

    h_adc      -> Fill(adc_double);
    h_adcm     -> Fill(adcm);
    h_tot      -> Fill(tot);
    h_toa      -> Fill(toa);
    h_trigtime -> Fill(trigtime);
}

void PlaygroundDQMEDAnalyzer::fill_profiles(int globalChannelId_, double adc_double_)
{
    myRunStatCollection.add_entry(globalChannelId_, adc_double, adc_channel_CM);

    p_adc      -> Fill(globalChannelId_ , adc_double);
    p_adcm     -> Fill(globalChannelId_ , adcm      );
    p_tot      -> Fill(globalChannelId_ , tot       );
    p_toa      -> Fill(globalChannelId_ , toa       );
    p_trigtime -> Fill(globalChannelId_ , trigtime  );
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
