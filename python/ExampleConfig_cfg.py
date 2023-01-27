import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1) )

process.source = cms.Source("PoolSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
        'file:/eos/user/y/ykao/www/HGCAL_Geant4_project/testbeam_positron_D86_R80To130_E100/step2.root'
    )
)

process.load("Validation.PlaygroundDQMEDAnalyzer.playgrounddqmedanalyzer_cfi")
process.DQMStore = cms.Service("DQMStore")

process.load("DQMServices.FileIO.DQMFileSaverOnline_cfi")

process.p = cms.Path(process.playgrounddqmedanalyzer + process.dqmSaver)
