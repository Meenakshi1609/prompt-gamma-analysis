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
/// \file B1SteppingAction.hh
/// \brief Definition of the B1SteppingAction class

#ifndef B1SteppingAction_h
#define B1SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include <fstream>
#include "G4ThreeVector.hh"
#include <vector>
#include <string>

// Forward declaration
class B1EventAction;
class G4LogicalVolume;

// Struct for detailed tracking of one photon per event
struct DetectorHit {
    G4ThreeVector position;
    G4double energy;
    G4double depositedEnergy;
    std::string detectorName;
    G4double time;
};

// CSV hit data structure for ALL photons
struct CSVHitData {
    G4int eventID;
    G4int trackID;
    G4String detector;
    G4double posX;
    G4double posY;
    G4double posZ;
    G4double energy;
    G4double depositedEnergy;
    // True origin tracking
    G4double trueOriginX;
    G4double trueOriginY;
    G4double trueOriginZ;
    G4double initialEnergy;
};

/// Stepping action class
class B1SteppingAction : public G4UserSteppingAction
{
  public:
    B1SteppingAction(B1EventAction* eventAction);
    virtual ~B1SteppingAction();

    // method from the base class
    virtual void UserSteppingAction(const G4Step* step);

  private:
    // Private helper methods for CSV
    void WriteCSVHeader();              
    void WriteCSVData();  

  private:
    // Member variables
    B1EventAction*  fEventAction;
    G4LogicalVolume* fScoringVolume;
    
    // Photon tracking (one photon per event)
    G4int fTrackedPhotonID;
    G4ThreeVector fTrackedPhotonGeneratedPos;
    G4double fTrackedPhotonGeneratedEnergy;
    G4int fCurrentEventID;
    std::vector<DetectorHit> fDetectorHits;
    
    // CSV file handling for ALL photons
    std::ofstream csvFile;
    std::vector<CSVHitData> fCSVHits;
    G4bool fCSVHeaderWritten;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
