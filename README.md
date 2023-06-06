# PlaygroundDQMEDAnalyzer

Backup branch from 81d26b2e5ad3d9694258e3a0789ea6fb039d8d92 in https://github.com/ywkao/cmssw/commits/CMSSW_13_1_0_pre2_hgcal_dqm_gui

```
# Need to manually create a directory (or softlink) for output files
$ mkdir ./eos

# Setup environment and compile
$ cmsenv
$ time scram b -j10

# Execution
$ time cmsRun python/ExampleConfig_cfg.py

# Make plots from DQM_V0001_*.root
$ ./getPlots.py
```
