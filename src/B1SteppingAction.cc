//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1SteppingAction.cc
/// \brief Implementation of the B1SteppingAction class

#include "B1SteppingAction.hh"
#include "B1EventAction.hh"
#include "B1DetectorConstruction.hh"
#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include <fstream>
#include <iostream>
#include "G4AnalysisManager.hh" 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1SteppingAction::B1SteppingAction(B1EventAction* eventAction)
: G4UserSteppingAction(),
  fEventAction(eventAction),
  fScoringVolume(0)
{
    //open while stepping action is created
    // outputFile.open("photondat.txt");
    // if (outputFile.is_open()) {
    //     outputFile << "PID X[mm] Y[mm] Z[mm] KineticEnergy[MeV]" << std::endl;
    //     std::cout << "File photondat.txt opened successfully." << std::endl;
    // } else {
    //     std::cerr << "Error: Unable to open photondat.txt" << std::endl;
    // }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1SteppingAction::~B1SteppingAction()
{
    // //close the file 
    // if (outputFile.is_open()) {
    //     outputFile.close();
    //     std::cout << "File photondat.txt closed successfully." << std::endl;
    // }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1SteppingAction::UserSteppingAction(const G4Step* step)
{
//  if (!fScoringVolume) {
//    const B1DetectorConstruction* detectorConstruction
//      = static_cast<const B1DetectorConstruction*>
//        (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
//    fScoringVolume = detectorConstruction->GetScoringVolume();
//  }

//  // get volume of the current step
//  G4LogicalVolume* volume
//    = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
      
//  // check if we are in scoring volume
//  if (volume != fScoringVolume) return;

//  // collect energy deposited in this step
//  G4double edepStep = step->GetTotalEnergyDeposit();
//  fEventAction->AddEdep(edepStep);
// std::cout<<"I am inside stepping action"<<std::endl;
// std::cout<<step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume()->GetName()<<std::endl;
 G4String volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume()->GetName();
    
    if (volume == "CylDetector") {
        // Check if it's a photon with PID 22
       
        G4int particlePID = step->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
        if (particlePID == 22) {
        //    std::cout << "i am here" << std::endl;
            // Get position and kinetic energy
            G4ThreeVector position = step->GetPreStepPoint()->GetPosition();
            G4double kineticEnergy = step->GetPreStepPoint()->GetKineticEnergy();
            // Fill the ROOT ntuple 
            auto analysisManager = G4AnalysisManager::Instance();
            analysisManager->FillNtupleIColumn(0, particlePID);        // PID (column 0)
            analysisManager->FillNtupleDColumn(1, position.x());       // PosX (column 1)
            analysisManager->FillNtupleDColumn(2, position.y());       // PosY (column 2)
            analysisManager->FillNtupleDColumn(3, position.z());       // PosZ (column 3)
            analysisManager->FillNtupleDColumn(4, kineticEnergy);      // KineticEnergy (column 4)
            analysisManager->AddNtupleRow();                           // Save the row
            

            //write data to file if its open
               
        //    if (outputFile.is_open()) {
            
        //         outputFile << particlePID << " " << position.x() << " " << position.y() << " " << position.z() << " " << kineticEnergy << std::endl;
        //     }
            
            // std::cout << particlePID << " " << position.x() << " " << position.y() << " " << position.z() << " " << kineticEnergy << std::endl;
        }
    }
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

