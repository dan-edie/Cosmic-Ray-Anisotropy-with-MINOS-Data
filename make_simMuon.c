#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TRandom1.h"
#include "TTimeStamp.h"
#include "math.h"
#include "time.h"

//Creating the functions. Have to tell it what you want, but don't need specific names
Double_t rando();
void timeCalc(Double_t x, TTimeStamp *currentTime);
Double_t declination(Double_t zen, Double_t azi, Double_t geoLat);
Double_t hourAngle(Double_t x, Double_t y, Double_t z, Double_t a, Double_t b);

void make_simMuon() {
        
    //Setting the maximum size of bins for the sky histograms
    Int_t maxRaBin = 1081;
    Int_t maxDecBin = 270;
    Int_t maxZenBin = 270;
    Int_t maxAziBin = 1081;

//create an output .root file
    TFile *f = new TFile("simMuonData.root","RECREATE");

    //create the histograms
    TH1D *deltaTHisto = new TH1D("deltaTHisto", "Delta T", 60, 0.0, 100.0);
    TH1D *simRAHisto = new TH1D("simRAHisto", "Sim RA", maxRaBin, 0.0, 360.0);
    TH1D *simZenHisto = new TH1D("simZenHisto", "Sim Zen", maxZenBin, 0.0, 90.0);
    TH1D *simAziHisto = new TH1D("simAziHisto", "Sim Azi", maxAziBin, 0.0, 360.0);
    TH1D *simSinDecHisto = new TH1D("simSinDecHisto", "Sim Sin Decl", maxDecBin, -1.0, 1.0);
    TH1D *timeCheckHisto = new TH1D("timeCheckHisto", "Time Check", 300, -5000.0, 5000.0);

    TH2D *simZenAziHisto = new TH2D("simZenAziHisto", "Sim Zenith vs Azimuth", maxZenBin, 0.0, 90.0, maxAziBin, 0.0, 360.0);
    TH2D *simRADeclHisto = new TH2D("simRADeclHisto", "Sim RA vs Sim Sin Decl", maxRaBin, 0.0, 360.0, maxDecBin, -1.0, 1.0);

    gRandom = new TRandom3(1983); //set system-wide RNG to be the seed
    
    Int_t numSimSky = 1;

    for(Int_t k = 1; k <= numSimSky; k++){ //main loop runs for whatever value I set
        clock_t begin = clock(); //Times the loop

        //open the .root file
        //TFile *muon_simFile = TFile::Open("muonDatatest.root");
        TFile *muon_File = TFile::Open("muonData.root");

        //read the .root file in
        TTreeReader muonReader("muonData", muon_File);

        //from the .root file, read the trees
        TTreeReaderValue<TTimeStamp> beginningTimeStamp(muonReader, "beginningTimeStamp");

        TTreeReaderValue<Double_t> deltaT(muonReader, "deltaT");

        TTreeReaderValue<Int_t> muonCount(muonReader, "muonCount");

        //read and recreate saved histograms in .root file
        TH2D* readZenAzi = 0;
        muon_File->GetObject("zenithAziHisto", readZenAzi);
        
        while(muonReader.Next()){//loops for all runs in the MINOS root file

            Double_t muonRate = (Double_t) *muonCount / *deltaT;
            auto currentTime = new TTimeStamp(0,0); //set the Timestamp
            beginningTimeStamp-> Copy(*currentTime);
            Double_t timeCheck = 0.0;

            if(*muonCount > 0.0){
                for(Int_t i = 1; i <= *muonCount; i++){//loop for total number of muons in the MINOS root file

                    Double_t azi;
                    Double_t zen;
                    Double_t geoLat = 47.82027; // Soudan Mine Latitude in decimal degrees
                    Double_t randomTime;
                    Double_t simHourAngle;
                    Double_t hourTime;
                    Double_t simDeclination;

                    //zenith and azimuth creation
                    readZenAzi->GetRandom2(zen, azi); // gets random zenith and azimuth from saved histogram
                    simAziHisto->Fill(azi);
                    simZenHisto->Fill(zen);
                    simZenAziHisto->Fill(zen, azi);

                    //random time and delta T creation
                    randomTime = rando();
                    Double_t simTime = randomTime / muonRate;
                    deltaTHisto->Fill(simTime);
                    //timeCalc sets seconds and nanoseconds
                    timeCalc(simTime, currentTime);

                    //declination creation
                    simDeclination = declination(zen, azi, geoLat);
                    simSinDecHisto->Fill(simDeclination);

                    //right acension creation
                    hourTime = currentTime->AsLMST(-92.24141);
                    simHourAngle = hourAngle(zen, azi, geoLat, simDeclination, hourTime);
                    simRAHisto->Fill(simHourAngle);
                    simRADeclHisto->Fill(simHourAngle, simDeclination);

                }//end loop over muons
            }
        timeCheck = (beginningTimeStamp->AsDouble() + *deltaT) - currentTime->AsDouble();
        //plop this into a histogram
        timeCheckHisto->Fill(timeCheck);

        delete currentTime;

        }//end muonReader loop

        //this bit of code calculates the time it took to run.
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        cout << "Finished Run Number: " << k << " " <<  "Time in loop: " << time_spent/60.0 << " min" << endl;

        delete readZenAzi;

        muon_File->Close("");

    }//program ends

f->Write();
/*
TCanvas* simAziCanvas = new TCanvas("simAziCanvas", "Sim Azi", 800, 600);
simAziHisto->Draw();

TCanvas* simZenCanvas = new TCanvas("simZenCanvas", "Sim Zenith", 800, 600);
simZenHisto->Draw();

TCanvas* simZenAziCanvas = new TCanvas("simZenAziCanvas", "Sim Zenith/Azi", 800, 600);
simZenAziHisto->Draw();

TCanvas* simSinDecCanvas = new TCanvas("simSinDecCanvas", "Sim Sin Declination", 800, 600);
simSinDecHisto->Draw();

TCanvas* simRACanvas = new TCanvas("simRACanvas", "Sim Right Acension", 800, 600);
simRAHisto->Draw();

TCanvas* simRADecCanvas = new TCanvas("simRADecCanvas", "Sim RA and Dec", 800, 600);
simRADeclHisto->Draw();

TCanvas* timeCheckCanvas = new TCanvas("timeCheckCanvas", "Time Check", 800, 600);
timeCheckHisto->Draw();

TCanvas* deltaTCanvas = new TCanvas("deltaTCanvas", "Sim DeltaT", 800, 600);
deltaTHisto->Draw("E");
gPad->SetLogy();
deltaTHisto->Fit("expo");
deltaTHisto->GetXaxis()->SetTitle("Time (s)");
deltaTHisto->GetYaxis()->SetTitle("Log # events");
*/    
}
//Functions go here
Double_t rando(){

    Double_t random = 0.0; 

    while(random == 0.0){
        random = gRandom-> Rndm();
    }
    Double_t randT = -log(random);//exponential deviate
    return randT;
}

void timeCalc(Double_t simTime, TTimeStamp *currentTime){
    //Get the times out of the timestamp
    Int_t getSimSec = currentTime->GetSec();
    Int_t getSimNSec = currentTime->GetNanoSec();
    Double_t totalTime;
    /* Carrying logic: if currentTime = 3.1415, getSimSec pulls off the 3, getSimNSec takes the
    0.1415 and multiplies it by a billion. If the result is OVER 1 billion, add a second to
    getSimSec, and subtract a billion from getSimNSec */

    getSimSec += (int)floor(simTime);
    getSimNSec += (int)(fmod(simTime, 1.0) * 1000000000.0);
    if(getSimNSec > 1000000000){
        getSimNSec -= 1000000000;
        getSimSec += 1;
    }
    //Put the carried time into the time stamp
    currentTime->SetSec(getSimSec);
    currentTime->SetNanoSec(getSimNSec);
}

//Create a simulated declination
Double_t declination(Double_t zen, Double_t azi, Double_t geoLat){
    Double_t aziDecimal = azi;
    Double_t zenDecimal = zen;
    Double_t altitude = 90.0 - zen;
    
    //From Section 26 in Practical Astronomy
    Double_t declination = sin(altitude * M_PI/180.0)*sin(geoLat * M_PI/180.0) +
    cos(altitude * M_PI/180.0)*cos(geoLat * M_PI/180.0)*cos(aziDecimal * M_PI/180.0);

    return declination;
}

//Return simulated Right Acension
Double_t hourAngle(Double_t zen, Double_t azi, Double_t geoLat, Double_t simDeclination, Double_t currentTime){
    Double_t aziDecimal = azi;
    Double_t zenDecimal = zen;
    Double_t altitude = 90.0 - zenDecimal;
    Double_t decl = asin(simDeclination) * 180/M_PI;
    Double_t trueHour;
    Double_t localSideTime = currentTime;

    Double_t hourAngle = (sin(altitude * M_PI/180.0)-(sin(geoLat * M_PI/180.0)*sin(decl * M_PI/180.0)))/
    (cos(geoLat * M_PI/180.0)*cos(decl * M_PI/180.0)); //in radians

    //fixes weird math glitches
    if(hourAngle > 1.0){
        hourAngle = 1.0;
    }
    if(hourAngle < -1.0){
        hourAngle = -1.0;
    }

    Double_t invHourAngle = acos(hourAngle) * 180.0/M_PI;

    if(sin(aziDecimal * M_PI/180.0) < 0.0){
        trueHour = invHourAngle/15;
    }
    else
        trueHour = (360.0 - invHourAngle)/15;

    localSideTime = localSideTime - trueHour;
    if(localSideTime > 24.0){
        localSideTime -= 24.0;
    }
    if(localSideTime < 0.0){
        localSideTime += 24.0;
    }

    Double_t rightAcension = localSideTime - trueHour;
    if(rightAcension < 0.0){
        rightAcension += 24.0;
    }

    rightAcension = rightAcension * 15.0;

    return rightAcension;

}