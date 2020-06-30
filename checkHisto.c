#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TRandom1.h"
#include "TTimeStamp.h"
#include "math.h"

//open and compare histograms file from data and simulated sky
//Double_t getBinContent = readZenithAzi->GetBinContent(i,j); produces value from bins i, j

void checkHisto(){ 

    ofstream myfile;
    myfile.open("binValues.dat");

    //create the files
    TFile* muon_Data = TFile::Open("muonData.root");
    TFile* simMuon_Data = TFile::Open("simMuonData.root");
    
    //read the .root files in
    TTreeReader muonReader("muonData", muon_Data);
    TTreeReader simMuonReader("simMuonData", simMuon_Data);
    
    //get from the .root file trees
    TTreeReaderValue<TTimeStamp> beginningTimeStamp(muonReader, "beginningTimeStamp");
    TTreeReaderValue<Int_t> simMuonCount(simMuonReader, "simMuonCount");

    //read and recreate the saved histograms in the .root file
    TH2D* readZenithAzi = 0;
    muon_Data->GetObject("zenithAziHisto", readZenithAzi);
    TH2D* readSimZenAzi = 0;
    simMuon_Data->GetObject("simZenAziHisto", readSimZenAzi);

    TH2D* readRADecl = 0;
    muon_Data->GetObject("raDeclHisto", readRADecl);
    TH2D* readSimRADecl = 0;
    simMuon_Data->GetObject("simRADeclHisto", readSimRADecl);

    TH1D* readRA = 0;
    muon_Data->GetObject("raHisto", readRA);
    TH1D* readDecl = 0;
    muon_Data->GetObject("declHisto", readDecl);

    //create the histograms from above
    if (false){
        TCanvas* zenithAziHistoCanvas = new TCanvas("zenithAziHisto", "Zenith & Azimuth",800, 600);
        readZenithAzi->Draw("SURFACE");
        TCanvas* simZenithAziHistoCanvas = new TCanvas("simZenithAziHistoCanvas", "sim Zen & Azi",800,600);
        readSimZenAzi->Draw("SURFACE");
        
        TCanvas* raSinDeclCanvas = new TCanvas("raSineDeclHisto", "RA/Sine Decl Histo", 800, 600);
        readRADecl->Draw("SURFACE");
        TCanvas* simRaDeclCanvas = new TCanvas("simRaDeclHisto", "Sim RA/Sine Decl Histo", 800, 600);
        readSimRADecl->Draw("SURFACE");
    }

    TH1D *deviationHisto = new TH1D("deviationHisto", "Deviation Histogram", 300, -5.0, 5.0);

    Double_t numObs = 0.0;
    Double_t numBack = 0.0;
    Double_t totObs = 0.0;
    Double_t sigmaD = 0.0;

    Int_t k = 0;
    Int_t l = 0;
    Int_t greater = 0;
    Int_t lower = 0;
    Int_t highX = 0;
    Int_t highY = 0;
    
    for(Int_t i = 1; i <=800; i++){
        for(Int_t j = 1; j <= 255; j++){

            numObs = readRADecl->GetBinContent(i,j);
            numBack = readSimRADecl->GetBinContent(i,j) * 0.01;

            if(numBack > 24.0){
                sigmaD = (numObs-numBack)/sqrt(numBack);
                deviationHisto->Fill(sigmaD);
                myfile << sigmaD << endl;
            }                  
        } 
    }

    cout << readRADecl->GetBinContent(252,260) << " " << readSimRADecl->GetBinContent(252,260) * 0.01 << endl;
    cout << readRADecl->GetBinContent(201,242) << " " << readSimRADecl->GetBinContent(201,242) * 0.01 << endl;

    TCanvas* deviationCanvas = new TCanvas("deviationCanvas", "Deviation", 800, 600);
    deviationHisto->Draw();
    deviationHisto->Fit("gausn");

    
    myfile.close();
}