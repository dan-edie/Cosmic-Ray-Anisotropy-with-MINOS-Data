#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TRandom1.h"
#include "TTimeStamp.h"
#include "math.h"

//Little program to open any histograms I want without having to run all the data again.

void openHistogram() {

    //open the ROOT files
    TFile* muon_Data = TFile::Open("muonData.root");
    TFile* simMuon_Data = TFile::Open("simMuonData.root");

    //read the .root files in
    TTreeReader muonReader("muonData", muon_Data);
    TTreeReader simMuonReader("simMuonData", simMuon_Data);

    //read and recreate the saved histograms in the .root file
    //1D Histograms
    TH1D* readCosAngle = 0;
    muon_Data->GetObject("cosAngleHisto", readCosAngle);

    TH1D* readZenith = 0;
    muon_Data->GetObject("zenHisto", readZenith);

    TH1D* readSimZenith = 0;
    simMuon_Data->GetObject("simZenHisto", readSimZenith);

    TH1D* readAzimuth = 0;
    muon_Data->GetObject("aziHisto", readAzimuth);

    TH1D* readSimAzimuth = 0;
    simMuon_Data->GetObject("simAziHisto", readSimAzimuth);

    TH1D* readDecl = 0;
    muon_Data->GetObject("declHisto", readDecl);

    TH1D* readSimDecl = 0;
    simMuon_Data->GetObject("simSinDecHisto", readSimDecl);

    TH1D* readRAHisto = 0;
    muon_Data->GetObject("raHisto", readRAHisto);

    TH1D* readSimRAHisto = 0;
    simMuon_Data->GetObject("simRAHisto", readSimRAHisto);


    //2D Histograms
    TH2D* readZenithAzi = 0;
    muon_Data->GetObject("zenithAziHisto", readZenithAzi);
    TH2D* readSimZenAzi = 0;
    simMuon_Data->GetObject("simZenAziHisto", readSimZenAzi);

    TH2D* readRADecl = 0;
    muon_Data->GetObject("raDeclHisto", readRADecl);
    TH2D* readSimRADecl = 0;
    simMuon_Data->GetObject("simRADeclHisto", readSimRADecl);
    
    //print out the histograms
    TCanvas* cosAngleCanvas = new TCanvas("cosAngleCanvas", "Angle Bewteen Muons",800, 600);
    readCosAngle->Draw();
    readCosAngle->SetStats(0);
    readCosAngle->SetTitle("Angle Between Muons/#sqrt{N}");
    
    TCanvas* declCanvas = new TCanvas("declCanvas", "Sin Declination", 800, 600);
    readDecl->Draw();
    readDecl->SetStats(0);
    readSimDecl->Scale(0.05);
    readSimDecl->Draw("SAME");
    readDecl->SetLineColor(kRed);
    readSimDecl->SetLineColor(kBlue);
    
    
    //TCanvas* simDeclCanvas = new TCanvas("simDeclCanvas", "Sim Sin Declination", 800, 600);
    //readSimDecl->Draw();

    TCanvas* zenithCanvas = new TCanvas("zenithCanvas", "Zenith", 800,600);
    readZenith->Draw();
    readZenith->SetLineColor(kRed);
    readSimZenith->Scale(0.05);
    readSimZenith->Draw("SAME");
    readSimZenith->SetLineColor(kBlue);
    readZenith->SetStats(0);
    readZenith->SetXTitle("Degrees");
    readZenith->SetYTitle("Muon Count");

    TCanvas* azimuthCanvas = new TCanvas("azimuthCanvas", "Azimuth", 800,600);
    readAzimuth->Draw();
    readAzimuth->SetStats(0);
    readAzimuth->SetLineColor(kRed);
    readSimAzimuth->Scale(0.05);
    readSimAzimuth->Draw("SAME");
    readSimAzimuth->SetLineColor(kBlue);
    
    TCanvas* raHistoCanvas = new TCanvas("raHistoCanvas", "RA Histogram", 800, 600);
    readRAHisto->Draw("E");
    readRAHisto->SetLineColor(kRed);
    readSimRAHisto->Scale(0.05);
    readSimRAHisto->Draw("SAME");
    readSimRAHisto->SetLineColor(kBlue);

    //subtract initial timestamp and compare to deltaT saved in root files
}