# PlaygroundDQMEDAnalyzer

```
# Developing on top of hgcal-condformat-13_2_X branch
cmsrel CMSSW_13_2_X_2023-05-23-2300
cd CMSSW_13_2_X_2023-05-23-2300/src/
cmsenv
git cms-init
git cms-checkout-topic CMS-HGCAL:hgcal-condformat-13_2_X
cd ./CalibCalorimetry

# Import this module
git clone -b eleId git@github.com:ywkao/PlaygroundDQMEDAnalyzer.git
cd PlaygroundEDProducer/
mkdir eos
time scram b -j10
time cmsRun python/ExampleConfig_cfg.py
```

Reference: https://cms-sw.github.io/tutorial-collaborating-with-peers.html
