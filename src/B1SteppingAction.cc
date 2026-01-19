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
#include <iomanip> 
#include "G4AnalysisManager.hh" 
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include <vector>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1SteppingAction::B1SteppingAction(B1EventAction* eventAction)
: G4UserSteppingAction(),
  fEventAction(eventAction),
  fScoringVolume(nullptr),
  fCSVHeaderWritten(false)
{
    fTrackedPhotonID = -1;
    fTrackedPhotonGeneratedPos = G4ThreeVector(0,0,0);
    fTrackedPhotonGeneratedEnergy = 0.0;
    fCurrentEventID = -1;
    fDetectorHits.clear(); 
    fCSVHits.clear();
    
    // Open CSV file for writing
    csvFile.open("detector_hits.csv");
    if (csvFile.is_open()) {
        std::cout << "CSV file 'detector_hits.csv' opened successfully." << std::endl;
    } else {
        std::cerr << "ERROR: Unable to create CSV file!" << std::endl;
    }

    std::cout << "Photon tracking started - tracking one photon per event" << std::endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1SteppingAction::~B1SteppingAction()
{
    // Write any remaining data
    if (!fCSVHits.empty()) {
        WriteCSVData();
    }
    
    if (csvFile.is_open()) {
        csvFile.close();
        std::cout << "CSV file closed successfully." << std::endl;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1SteppingAction::WriteCSVHeader()
{
    if (!fCSVHeaderWritten && csvFile.is_open()) {
        // SINGLE LINE HEADER - MATCHES CSVHitData struct
        csvFile << "Event_ID,Track_ID,Detector,Pos_X_mm,Pos_Y_mm,Pos_Z_mm,"
                << "Energy_MeV,Deposited_MeV,True_Origin_X_mm,"
                << "True_Origin_Y_mm,True_Origin_Z_mm,Initial_Energy_MeV\n";
        fCSVHeaderWritten = true;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1SteppingAction::WriteCSVData()
{
    WriteCSVHeader();
    
    for (const auto& hit : fCSVHits) {
        csvFile << hit.eventID << ","
                << hit.trackID << ","
                << hit.detector << ","
                << std::fixed << std::setprecision(6)
                << hit.posX << ","
                << hit.posY << ","
                << hit.posZ << ","
                << hit.energy << ","
                << hit.depositedEnergy << ","
                << hit.trueOriginX << ","
                << hit.trueOriginY << ","
                << hit.trueOriginZ << ","
                << hit.initialEnergy << "\n";
    }
    
    csvFile.flush();
    fCSVHits.clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Get current event ID
    G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    
    // Check if new event started
    if (eventID != fCurrentEventID) {
        // New event - reset tracking
        if (!fCSVHits.empty()) {
            WriteCSVData();
        }
        fCurrentEventID = eventID;
        fTrackedPhotonID = -1;
        fTrackedPhotonGeneratedPos = G4ThreeVector(0,0,0);
        fTrackedPhotonGeneratedEnergy = 0.0;
        fDetectorHits.clear();
        std::cout << "\n[Event " << eventID << "] Started new event tracking..." << std::endl;
    }

    // Get volume and particle information
    G4String volume = step->GetPreStepPoint()->GetTouchableHandle()
                     ->GetVolume()->GetLogicalVolume()->GetName();
    G4int particlePID = step->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
    G4int trackID = step->GetTrack()->GetTrackID();
    G4ThreeVector position = step->GetPreStepPoint()->GetPosition();
    G4double kineticEnergy = step->GetPreStepPoint()->GetKineticEnergy();
    
    // ==================== PHOTON TRACKING (ONE PER EVENT) ====================
    if (particlePID == 22) {
        if (fTrackedPhotonID == -1) {
            // This is the FIRST photon in this event - let's track it!
            fTrackedPhotonID = trackID;
            fTrackedPhotonGeneratedPos = step->GetTrack()->GetVertexPosition();
            fTrackedPhotonGeneratedEnergy = step->GetTrack()->GetVertexKineticEnergy();
            
            std::cout << "  Selected Photon ID " << trackID << " for tracking" << std::endl;
            std::cout << "    TRUE ORIGIN: (" 
                      << fTrackedPhotonGeneratedPos.x()/mm << ", "
                      << fTrackedPhotonGeneratedPos.y()/mm << ", "
                      << fTrackedPhotonGeneratedPos.z()/mm << ") mm" << std::endl;
            std::cout << "    Initial energy: " << fTrackedPhotonGeneratedEnergy/MeV << " MeV" << std::endl;
        }
        
        // Track the selected photon's detailed interactions
        if (trackID == fTrackedPhotonID) {
            if (volume == "CylDetector" || volume == "CylDetector2") {
                std::cout << "i am here - Photon hit cylindrical detector: " << volume << std::endl;
                
                G4ThreeVector hitPosition = step->GetPreStepPoint()->GetPosition();
                G4double hitEnergy = step->GetPreStepPoint()->GetKineticEnergy();
                G4double depositedEnergy = step->GetTotalEnergyDeposit();
                
                std::cout << "   DETECTOR HIT: " << volume << std::endl;
                std::cout << "     Position: ("
                          << hitPosition.x()/mm << ", "
                          << hitPosition.y()/mm << ", "
                          << hitPosition.z()/mm << ") mm" << std::endl;
                std::cout << "     Energy before hit: " << hitEnergy/MeV << " MeV" << std::endl;
                std::cout << "     Energy deposited: " << depositedEnergy/MeV << " MeV" << std::endl;
                std::cout << "     Time: " << step->GetPreStepPoint()->GetGlobalTime()/ns << " ns" << std::endl;
                
                // Store hit information for tracked photon
                DetectorHit hit;
                hit.position = hitPosition;
                hit.energy = hitEnergy;
                hit.depositedEnergy = depositedEnergy;
                hit.detectorName = volume;
                hit.time = step->GetPreStepPoint()->GetGlobalTime();
                
                fDetectorHits.push_back(hit);
            }
            
            // Check if tracked photon ended
            if (step->GetPostStepPoint()->GetPhysicalVolume() == nullptr || 
                step->GetTrack()->GetTrackStatus() != fAlive) {
                
                std::cout << "    Tracked photon ended." << std::endl;
                std::cout << "     Total detector hits: " << fDetectorHits.size() << std::endl;
                fTrackedPhotonID = -1;
                fDetectorHits.clear();
            }
        }
    }
    
    // ==================== SAVE ALL PHOTON HITS TO ROOT & CSV ====================
    // For ALL photons (not just tracked ones) hitting either detector
    if (particlePID == 22) {
        if (volume == "CylDetector" || volume == "CylDetector2") {
            // Get THIS photon's true origin (not the tracked photon's origin!)
            G4ThreeVector thisPhotonOrigin = step->GetTrack()->GetVertexPosition();
            G4double thisPhotonInitialEnergy = step->GetTrack()->GetVertexKineticEnergy();
            
            // ============ SAVE TO ROOT NTUPLE ============
            auto analysisManager = G4AnalysisManager::Instance();
            analysisManager->FillNtupleIColumn(0, particlePID);
            analysisManager->FillNtupleDColumn(1, position.x()/mm);
            analysisManager->FillNtupleDColumn(2, position.y()/mm);
            analysisManager->FillNtupleDColumn(3, position.z()/mm);
            analysisManager->FillNtupleDColumn(4, kineticEnergy/MeV);
            analysisManager->FillNtupleSColumn(5, volume);
            analysisManager->AddNtupleRow();
            
            // ============ SAVE TO CSV ============
            CSVHitData csvHit;
            csvHit.eventID = eventID;
            csvHit.trackID = trackID;
            csvHit.detector = volume;
            csvHit.posX = position.x()/mm;
            csvHit.posY = position.y()/mm;
            csvHit.posZ = position.z()/mm;
            csvHit.energy = kineticEnergy/MeV;
            csvHit.depositedEnergy = step->GetTotalEnergyDeposit()/MeV;
            
            // CRITICAL FIX: Use THIS photon's origin, not the tracked photon's!
            csvHit.trueOriginX = thisPhotonOrigin.x()/mm;     
            csvHit.trueOriginY = thisPhotonOrigin.y()/mm;      
            csvHit.trueOriginZ = thisPhotonOrigin.z()/mm;      
            csvHit.initialEnergy = thisPhotonInitialEnergy/MeV;
            
            fCSVHits.push_back(csvHit);
            
            // Write to file periodically (every 100 hits for efficiency)
            if (fCSVHits.size() >= 100) {
                WriteCSVData();
            }
        }
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......