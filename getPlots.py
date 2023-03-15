#!/usr/bin/env python
import ROOT
ROOT.gROOT.SetBatch(True)

fin = ROOT.TFile("./eos/DQM_V0001_TEST_R000000001.root", "R")

d = fin.GetDirectory("DQMData/Run 1/HGCAL/Run summary/RecHits", True)

h2d = d.Get("h2d_adc_channel_7")
p2d = d.Get("p2d_adc_channel_7")

#--------------------------------------------------
# Linear fit
#--------------------------------------------------
''' Consider to add linear fit based on h2d from DQM plots '''

#--------------------------------------------------
# Running statistics
#--------------------------------------------------
''' To add the information on the plot, we need to load values from a parameter table produced from DQM module '''

#--------------------------------------------------
# plotting
#--------------------------------------------------
ROOT.gStyle.SetOptStat("nemrou");
ROOT.gStyle.SetPalette(ROOT.kCherry)
ROOT.TColor.InvertPalette()

c1 = ROOT.TCanvas("c1", "", 800, 600)
c1.SetRightMargin(0.15)

h2d.SetStats(0)
h2d.Draw("colz")

p2d.SetStats(0)
p2d.SetMarkerStyle(20)
p2d.SetMarkerSize(0.5)
p2d.SetLineWidth(2)
p2d.Draw("p,same")

output = "./eos/h2d_adc_channel_7"
c1.SaveAs(output + ".png")
c1.SaveAs(output + ".pdf")
