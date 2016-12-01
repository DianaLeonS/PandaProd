#include "../interface/MetFiller.h"

#include "DataFormats/PatCandidates/interface/MET.h"

MetFiller::MetFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name)
{
  auto& fillerCfg(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name));

  getToken_(metToken_, fillerCfg, _coll, "met");
  getToken_(candidatesToken_, _cfg, _coll, "pfCandidates");
}

void
MetFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore&)
{
  auto& inMet(getProduct_(_inEvent, metToken_, "met").at(0));
  auto& candidates(getProduct_(_inEvent, candidatesToken_, "pfCandidates"));

  if (dynamic_cast<pat::MET const*>(&inMet)) {
    auto& patMet(static_cast<pat::MET const&>(inMet));

    auto& t1Met(_outEvent.met);

    t1Met.pt = patMet.pt();
    t1Met.phi = patMet.phi();

    t1Met.sumETRaw = patMet.uncorSumEt();

    t1Met.ptCorrUp = patMet.shiftedPt(pat::MET::JetEnUp);
    t1Met.phiCorrUp = patMet.shiftedPhi(pat::MET::JetEnUp);
    t1Met.ptCorrDown = patMet.shiftedPt(pat::MET::JetEnDown);
    t1Met.phiCorrDown = patMet.shiftedPhi(pat::MET::JetEnDown);

    t1Met.ptUnclUp = patMet.shiftedPt(pat::MET::UnclusteredEnUp);
    t1Met.phiUnclUp = patMet.shiftedPhi(pat::MET::UnclusteredEnUp);
    t1Met.ptUnclDown = patMet.shiftedPt(pat::MET::UnclusteredEnDown);
    t1Met.phiUnclDown = patMet.shiftedPhi(pat::MET::UnclusteredEnDown);

    if (!_inEvent.isRealData()) {
      t1Met.ptSmear = patMet.corPt(pat::MET::Type1Smear);
      t1Met.phiSmear = patMet.corPhi(pat::MET::Type1Smear);
      t1Met.ptSmearUp = patMet.shiftedPt(pat::MET::JetResUpSmear, pat::MET::Type1Smear);
      t1Met.phiSmearUp = patMet.shiftedPhi(pat::MET::JetResUpSmear, pat::MET::Type1Smear);
      t1Met.ptSmearDown = patMet.shiftedPt(pat::MET::JetResDownSmear, pat::MET::Type1Smear);
      t1Met.phiSmearDown = patMet.shiftedPhi(pat::MET::JetResDownSmear, pat::MET::Type1Smear);
    }

    _outEvent.rawMet.pt = patMet.uncorPt();
    _outEvent.rawMet.phi = patMet.uncorPhi();

    _outEvent.caloMet.pt = patMet.caloMETPt();
    _outEvent.caloMet.phi = patMet.caloMETPhi();
  }

  double noMuMex(inMet.px());
  double noMuMey(inMet.py());
  double trkMex(0.);
  double trkMey(0.);
  double neutralMex(0.);
  double neutralMey(0.);
  double photonMex(0.);
  double photonMey(0.);
  double hfMex(0.);
  double hfMey(0.);
  for (auto& cand : candidates) {
    if (std::abs(cand.pdgId()) == 13) {
      noMuMex += cand.px();
      noMuMey += cand.py();
    }
    else if (cand.pdgId() == 130) {
      neutralMex -= cand.px();
      neutralMey -= cand.py();
    }
    else if (cand.pdgId() == 22) {
      photonMex -= cand.px();
      photonMey -= cand.py();
    }
    else if (cand.pdgId() == 1 || cand.pdgId() == 2) {
      hfMex -= cand.px();
      hfMey -= cand.py();
    }

    if (cand.charge() != 0) {
      trkMex -= cand.px();
      trkMey -= cand.py();
    }
  }

  _outEvent.noMuMet.setXY(noMuMex, noMuMey);
  _outEvent.trkMet.setXY(trkMex, trkMey);
  _outEvent.neutralMet.setXY(neutralMex, neutralMey);
  _outEvent.photonMet.setXY(photonMex, photonMey);
  _outEvent.hfMet.setXY(hfMex, hfMey);
}

DEFINE_TREEFILLER(MetFiller);