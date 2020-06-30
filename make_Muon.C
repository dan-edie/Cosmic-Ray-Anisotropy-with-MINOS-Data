/* NOTE - this is a root6 macro.  root5's way of accessing TTrees is a PITA */

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TTimeStamp.h"
#include "math.h"

using namespace std;

void make_Muon() {

    Double_t lastazi = 0.0;
    Double_t deltaazi = 0.0;
    Double_t deltaT = 0.0;
    Double_t deltaTH = 0.0;
    Double_t cosAngle = 0.0;
    Double_t cosAngleOutput = 0.0;
    Double_t diffx = 0.0;
    Double_t diffy = 0.0;
    Double_t diffz = 0.0;
    auto beginningTimeStamp = new TTimeStamp(0,0);
    auto lastTimeStamp = new TTimeStamp(0,0);
    UInt_t lastRun = 0;
    int muonCount = 0;
    Int_t runCount = 0;
    TDatime T0(2003,7,31,00,00,00);
    int X0 = T0.Convert(true)-36000; //3600 is offset from CST to GMT
    Double_t deltaTempPercent = 0.0;
    Double_t weightedCount = 0.0;


    //Setting the maximum size of bins for the sky histograms
    Int_t maxRaBin = 1081;
    Int_t maxDecBin = 270;
    Int_t maxZenBin = 270;
    Int_t maxAziBin = 1081;

    // use TChain just to get list of files in directory

    TChain chain;
    std::string infilename;
    //infilename = "muons-*.root";
    
    //turn this on when getting the entire muon data
    infilename = "/net/lepton/localhome/data/minos/ntuple/muons-*.root";
    chain.Add(infilename.c_str());
  
    TObjArray* filelist = chain.GetListOfFiles(); 
    TIter iter(filelist);

    //TFile *f = new TFile("muonData.root","RECREATE");
    TFile *f = new TFile("muonData.root","RECREATE");
    //TFile *hists = new TFile("dataHistograms.root","RECREATE");

    //Open up temp histograms from Bryce
    TFile* tempData = TFile::Open("tempData.root"); //open Bryce's histograms

    //read in information from Bryce's root file
    TH1D* muonRate = 0;
    tempData->GetObject("MuonRate", muonRate); //muon rate per day

    TH1D* rateHist = 0;
    tempData->GetObject("RateHist", rateHist); //number of rates. Don't think I need.

    TH1D* deltaTime = 0;
    tempData->GetObject("DeltaT", deltaTime); //time between muons

    TH1D* teff = 0;
    tempData->GetObject("Teff", teff); //effective temperatures

    TH1D* teffPercent = 0;
    tempData->GetObject("TeffPercent", teffPercent); //temp percent correclation of muons/temp

    TH1D* ratePercent = 0;
    tempData->GetObject("RatePercent", ratePercent); //rate percent correclation of muons/temp

    f->cd();

    //output file for data analysis
    ofstream myfile;
    myfile.open("muonRate.dat");

    //create the tree
    TTree *tree= new TTree("muonData","run_data");

    tree->Branch("lastRun",&lastRun,"lastRun/i");
    tree->Branch("beginningTimeStamp", &beginningTimeStamp);
    tree->Branch("deltaT", &deltaT, "deltaT/D");
    tree->Branch("muonCount",&muonCount,"muonCount/I");
    
    // make the histograms
    //These are just checks/examples
    TH1D *ntrackHisto = new TH1D("ntrackHisto","number of tracks per event",20,0.0,20.0);
    TH1D *lengthHisto = new TH1D("lengthHisto","length per track",60,0.0,60.0);
    TH1D *subrunHisto = new TH1D("subrunHisto","subrun",30,0.0,30.0);

    //sky histograms
    TH1D *raHisto = new TH1D("raHisto", "ra", maxRaBin, 0.0, 360.0);
    TH1D *declHisto = new TH1D("declHisto", "sin Dec", maxDecBin, -1.0, 1.0);
    TH1D *zenHisto = new TH1D("zenHisto", "zen", maxZenBin, 0.0, 90.0);
    TH1D *aziHisto = new TH1D("aziHisto", "azi", maxAziBin, 0.0, 360.0);

    //time and angle histograms
    TH1D *delTHisto = new TH1D("delTHisto", "Delta T",300,0.0,70.0);
    TH1D *cosAngleHisto = new TH1D("cosAngleHisto", "angle between muons", 100, 0.0, 5.0);

    //2d sky histograms
    TH2D *zenithAziHisto = new TH2D("zenithAziHisto", "Zenith vs Azimuth", maxZenBin, 0.0, 90.0, maxAziBin, 0.0, 360.0);
    TH2D *raDeclHisto = new TH2D("raDeclHisto", "RA vs sin Decl", maxRaBin, 0.0, 360.0, maxDecBin, -1.0, 1.0);
    TH2D *weightedRaDeclHisto = new TH2D("weightedRaDeclHisto", "Weighted RA and Sin Decl", maxRaBin, 0.0, 360.0, maxDecBin, -1.0, 1.0);

    //histograms for anisotropy
    TH1D* muonRunHisto = new TH1D("muonRunHisto", "Muon Count", 46343, 18143.0, 64486.0);
    TH1D* timeRunHisto = new TH1D("timeRunHisto", "Time Count", 46343, 18143.0, 64486.0);
    TH1D* muonRateHisto = new TH1D("muonRateHisto", "Muon Rate", 46343, 18143.0, 64486.0);

    // Loop over all files in chain element list until done selecting those 
    TChainElement* element = 0; 
    while ((element = dynamic_cast<TChainElement*>(iter.Next())) ) {
        cout<<" new input file "<<element->GetTitle()<<endl; 
        
        TFile* CondenseFile = new TFile(element->GetTitle()); 
        if ( !CondenseFile->IsOpen() || CondenseFile->IsZombie() )  { 
            cerr << "Warning! Failed to open file " << CondenseFile->GetName() << endl; 
            delete CondenseFile; 
            continue; 
        } 

        // make a tree reader instance: get the "data" tree out of CondenseFile
        // This is the root6 magic.  It's gross in root5
        TTreeReader muonReader("data", CondenseFile);

        // Let's look at number of tracks.  Things where there are one value
        // per event are accessed like this
        TTreeReaderValue<Int_t> ntrack(muonReader, "ntrack");

        // Let's look at subrun, which is a UChar_t
        TTreeReaderValue<UChar_t> subrun(muonReader, "subrun");

        TTreeReaderValue<TTimeStamp> timestamp(muonReader, "timestamp");

        TTreeReaderValue<UInt_t> run(muonReader, "run");

        // and the length of all the tracks
        // Things where there's an array per event (in our case, could
        // be a lot of tracks) are accessed like this
        TTreeReaderArray<Double_t> length(muonReader, "length");

        TTreeReaderArray<Double_t> ra(muonReader, "ra");

        TTreeReaderArray<Double_t> decl(muonReader, "decl");

        TTreeReaderArray<Double_t> zen(muonReader, "zen");

        TTreeReaderArray<Double_t> azi(muonReader, "azi");

        TTreeReaderArray<Double_t> dcosx(muonReader, "dcosx");

        TTreeReaderArray<Double_t> dcosy(muonReader, "dcosy");

        TTreeReaderArray<Double_t> dcosz(muonReader, "dcosz");

        TTreeReaderArray<Double_t> Enddcosx(muonReader, "Enddcosx");

        TTreeReaderArray<Double_t> Enddcosy(muonReader, "Enddcosy");

        TTreeReaderArray<Double_t> Enddcosz(muonReader, "Enddcosz");

        lastazi = 0.0;
        deltaazi = 0.0;
        beginningTimeStamp->SetSec(0); beginningTimeStamp->SetNanoSec(0);
        lastTimeStamp->SetSec(0); lastTimeStamp->SetNanoSec(0);
        lastRun = 0;
        muonCount = 0;
        runCount = 0;

        Int_t muonCounter = 0;
        Int_t trackCounter = 0;
        Int_t lengthCounter = 0;
        Int_t zenCounter = 0;

        // Loop over all entries of the TTree
        while (muonReader.Next()) {
            muonCounter ++;
            // TTreeReaderValue's are accessed with the "*" dereference
            ntrackHisto->Fill((Double_t)*ntrack);
            subrunHisto->Fill((Double_t)*subrun);

            // TTreeReaderArray's are accessed just straight up like this
            lengthHisto->Fill(length[0]);

            if( *ntrack == 1) { //cuts made: track length eq or gr than 8 m, only one event track per event, zenith cut at 80 degrees
                trackCounter ++;
                if(length[0] >= 8.0) {
                    lengthCounter ++;
                    if(zen[0] <= 80.0){
                        zenCounter ++;
                        declHisto->Fill(sin(decl[0]*M_PI/180.0)); //convert to radians

                        raHisto->Fill(ra[0]);

                        raDeclHisto->Fill(ra[0],sin(decl[0]*M_PI/180.0)); //convert to radians

                        //weighted radec with temp with data pulled from Bryce
                        int time = timestamp->GetSec() - X0;
                        int tempBin = teffPercent->GetXaxis()->FindBin(time);
                        deltaTempPercent = teffPercent->GetBinContent(tempBin)/100.0;

                        //alphaT = .923324       
                        weightedCount = 1.0 / (1.0 + 0.923324*deltaTempPercent);

                        weightedRaDeclHisto->Fill(ra[0],sin(decl[0]*M_PI/180.0), weightedCount);//fills new histogram with temp data correction

                        zenithAziHisto->Fill(zen[0], azi[0]);

                        zenHisto->Fill(zen[0]);

                        aziHisto->Fill(azi[0]);

                        deltaazi = lastazi - azi[0];

                        lastazi = azi[0];

                        deltaTH =  timestamp->AsDouble() - lastTimeStamp->AsDouble(); //for the histogram

                        deltaT = lastTimeStamp->AsDouble() -beginningTimeStamp->AsDouble(); //gets put into root branch

                        //if the last run is equal to the new run, fill the histo up, and
                        //increment the muonCount
                        if ( lastRun == *run) {

                            delTHisto->Fill(deltaTH);
                            muonCount ++;
                        } // end if lastrun == *run loop

                        else {
                            if (lastRun !=0) {
                                tree->Fill();
                                muonRunHisto->Fill(*run, (float) muonCount);
                                timeRunHisto->Fill(*run, (float) deltaTH);
                            }//end if statement
                        
                            timestamp->Copy(*beginningTimeStamp); //first muon of new run
                            muonCount = 1;
                            }//end else statement

                        lastRun = *run;
                        timestamp->Copy(*lastTimeStamp); //set last time of muon event
                        //any counters output goes here
                        myfile << muonCounter << " " << trackCounter << " " << lengthCounter << " " << zenCounter << endl;
                    }//end zen[0] <= 80.0 loop
                }//end of tracklength >= 8 meters loop
            }//end of ntrack == 1 loop
      
            if(*ntrack == 2) {
            cosAngle = (180/M_PI)*acos(dcosx[0]*dcosx[1]+dcosy[0]*dcosy[1]+dcosz[0]*dcosz[1]);
            cosAngleOutput = cosAngle/sqrt(2.0);

            cosAngleHisto->Fill(cosAngleOutput);

            }//end ntrack == 2
        }//end muonReader

        // free input tree stuff, close input file
        if (CondenseFile) { delete CondenseFile; CondenseFile = 0; 
            } // also destructs input file tree
        }// end of opening while loop
    // draw the histograms

    if (false) {
        TCanvas* aradDeclCanvas = new TCanvas("raDeclCanvas","ra vs sine dec",800,600);
        raDeclHisto->Draw("SURFACE");
        raDeclHisto->SetXTitle("Right Ascension (deg)");
        raDeclHisto->SetYTitle("Sin Declination");
        raDeclHisto->SetStats(kFALSE);

        TCanvas* zenCanvas = new TCanvas("zen", "zenith",800,600);
        zenHisto->Draw();

        TCanvas* aziCanvas = new TCanvas("azi", "azimuth",800,600);
        aziHisto->Draw();

        TCanvas* declCanvas = new TCanvas("declCanvas", "Sin Declination",800,600);
        declHisto->Draw();

        TCanvas* raCanvas = new TCanvas("raCanvas", "RA",800,600);
        raHisto->Draw();

        TCanvas* ntrackCanvas = new TCanvas("ntrackCanvas","# of tracks",640,480);
        gPad->SetLogy();
        ntrackHisto->Draw();

        TCanvas* lengthCanvas = new TCanvas("lengthCanvas","length of tracks",640,480);
        lengthHisto->Draw();

        TCanvas* zenithAziCanvas = new TCanvas("zenithAziCanvas", "zen vs azi",800,600);
        zenithAziHisto->Draw("SURFACE");

        TCanvas* cosAngleCanvas = new TCanvas("cosAngleCanvas", "Cos Angle", 800, 600);
        cosAngleHisto->Draw();
        cosAngleHisto->GetXaxis()->SetTitle("#Delta#theta");
        cosAngleHisto->GetYaxis()->SetTitle("Frequency");

        TCanvas* delTCanvas = new TCanvas("delTCanvas", "Delta T", 800, 600);
        delTHisto->Draw("E");
        gPad->SetLogy();
        delTHisto->Fit("expo");
        delTHisto->GetXaxis()->SetTitle("Time (s)");
        delTHisto->GetYaxis()->SetTitle("Log # events");

        TCanvas* muonRunCanvas = new TCanvas("muonRunCanvas", "muon run", 800, 600);
        muonRunHisto->Draw();

        TCanvas* timeRunCanvas = new TCanvas("timeRunCanvas", "time run", 800, 600);
        timeRunCanvas->Draw();

        TCanvas* muonRateCanvas = new TCanvas("muonRateCanvas", "Muon Rate", 800, 600);
        muonRateHisto->Divide(muonRunHisto, timeRunHisto);
        muonRateHisto->Draw();

        TCanvas* weightedRaDeclCanvas = new TCanvas("weightedRaDeclCanvas", "Weighted RA and Sin Decl", 800, 600);
        weightedRaDeclHisto->Draw();
    }

    f->Write();

    //write histograms to own root file for ease of access this way
    /*
    hists->cd();
    aniHisto->Write();
    zenHisto->Write();
    aziHisto->Write();
    declHisto->Write();
    raHisto->Write();
    raDeclHisto->Write();
    */

    myfile.close();
}
