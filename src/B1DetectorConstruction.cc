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
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class

#include "B1DetectorConstruction.hh"
#include "G4PVReplica.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::~B1DetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B1DetectorConstruction::Construct()
{  
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
   // Envelope parameters
  G4double env_sizeXY = 8.5*cm, env_sizeZ = 55*cm;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* det_mat   = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* env_mat = nist->FindOrBuildMaterial("G4_WATER");
  
  // Option to switch on/off checking of volumes overlaps
  G4bool checkOverlaps = true;
 // WORLD - Make it a LARGE BOX (40cm cube)
  G4double world_size = 2.2*env_sizeXY;
  G4double world_sizeZ  = 5.2*env_sizeZ;
 // G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
  G4Box* solidWorld =      
    new G4Box("World",                       //its name
       0.5*world_size, 0.5*world_size, 2.5*world_size);     //its size

  //G4Tubs* solidWorld =    
    //new G4Tubs("World", 0, world_radius, world_halfZ, 0, 360*deg);
      
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld, world_mat, "World");
                                   
  G4VPhysicalVolume* physWorld = 
    new G4PVPlacement(0, G4ThreeVector(), logicWorld,
                      "World", 0, false, 0, checkOverlaps);

  // CYLINDRICAL DETECTOR SHELL
  G4double det_innerRadius   = 6.5*cm;
  G4double det_outerRadius   = 6.55*cm;
  G4double det_halfHeight    = 0.55*env_sizeZ;
  
  G4Tubs* solidDetector =
    new G4Tubs("CylDetector", det_innerRadius, det_outerRadius,
               det_halfHeight, 0.*deg, 360.*deg);

  G4LogicalVolume* logicDetector =
    new G4LogicalVolume(solidDetector, det_mat, "CylDetector");

  new G4PVPlacement(0, G4ThreeVector(), logicDetector,
                    "CylDetector", logicWorld, false, 0, checkOverlaps);
                    
           // ============================================
  // DETECTOR 2: OUTER CYLINDRICAL DETECTOR
  // (Surrounds Detector1 with a gap)
  // ============================================
  G4double gap_thickness = 0.5*cm;  // Gap between the two detectors
  
  // Outer detector starts where inner detector ends + gap
  G4double det_innerRadius2 = det_outerRadius + gap_thickness;
  G4double det_outerRadius2 = det_innerRadius2 + 0.05*cm;  // Same thickness as inner detector
  
  G4Tubs* solidDetector2 =
    new G4Tubs("CylDetector2", det_innerRadius2, det_outerRadius2,
               det_halfHeight, 0.*deg, 360.*deg);

  G4LogicalVolume* logicDetector2 =
    new G4LogicalVolume(solidDetector2, det_mat, "CylDetector2");

  // Position Detector2 at same center (concentric)
  new G4PVPlacement(0, G4ThreeVector(), logicDetector2,
                    "CylDetector2", logicWorld, false, 0, checkOverlaps);         
  

  // WATER PHANTOM (Box) inside Cylinder
  G4Box* waterTub =
    new G4Box("WaterTub", 0.5*env_sizeXY, 0.5*env_sizeXY, 0.5*env_sizeZ);
      
  G4LogicalVolume* logicWater =
    new G4LogicalVolume(waterTub, env_mat, "WaterTub");
               
  new G4PVPlacement(0, G4ThreeVector(), logicWater,
                    "WaterTub", logicWorld, false, 0, checkOverlaps);

                     // ---------------------------------
  // Slice the Water Phantom along Z
  // ---------------------------------
  // G4int nSlices = 600;   // number of slices
  // G4double sliceThickness = env_sizeZ / nSlices;

  // G4Box* solidSlice =
  //   new G4Box("Slice", 0.5*env_sizeXY, 0.5*env_sizeXY, 0.5*sliceThickness);

  // G4LogicalVolume* logicSlice =
  //   new G4LogicalVolume(solidSlice, env_mat, "Slice");

  // new G4PVReplica("Slice",
  //                 logicSlice,     // logical volume of slice
  //                 logicWater,     // mother volume = WaterTub
  //                 kZAxis,         // replicate along Z
  //                 nSlices,        // number of slices
  //                 sliceThickness);// thickness of each slice
                  // ---------------------------------
  // Slice the Cylindrical Detector along Z
  // ---------------------------------
  // G4int nDetSlices = 600;   // number of detector slices
  // G4double detSliceThickness = (2.0 * det_halfHeight) / nDetSlices;

  // G4Tubs* solidDetSlice =
  //   new G4Tubs("DetSlice",
  //              det_innerRadius,
  //              det_outerRadius,
  //              0.5 * detSliceThickness,  // half-thickness
  //              0.*deg,
  //              360.*deg);

  // G4LogicalVolume* logicDetSlice =
  //   new G4LogicalVolume(solidDetSlice, det_mat, "DetSlice");

  // new G4PVReplica("DetSlice",
  //                 logicDetSlice,     // logical volume of slice
  //                 logicDetector,     // mother volume = CylDetector
  //                 kZAxis,            // replicate along Z
  //                 nDetSlices,        // number of slices
  //                 detSliceThickness);// thickness of each slice

  // ---------------------------------
  // Scoring volume
  // ---------------------------------
  fScoringVolume = logicWater;  // scoring individual slices
 
  //     
  // Shape 1
  //
  //G4Material* shape1_mat = nist->FindOrBuildMaterial("G4_A-150_TISSUE");
//  G4ThreeVector pos1 = G4ThreeVector(0, 2*cm, -7*cm);
        
//  // Conical section shape
//  G4double shape1_rmina =  0.*cm, shape1_rmaxa = 2.*cm;
//  G4double shape1_rminb =  0.*cm, shape1_rmaxb = 4.*cm;
//  G4double shape1_hz = 3.*cm;
//  G4double shape1_phimin = 0.*deg, shape1_phimax = 360.*deg;
//  G4Cons* solidShape1 =
//    new G4Cons("Shape1",
//    shape1_rmina, shape1_rmaxa, shape1_rminb, shape1_rmaxb, shape1_hz,
//    shape1_phimin, shape1_phimax);
                      
//  G4LogicalVolume* logicShape1 =
//    new G4LogicalVolume(solidShape1,         //its solid
//                        shape1_mat,          //its material
//                        "Shape1");           //its name
               
//  new G4PVPlacement(0,                       //no rotation
//                    pos1,                    //at position
//                    logicShape1,             //its logical volume
//                    "Shape1",                //its name
//                    logicEnv,                //its mother  volume
//                    false,                   //no boolean operation
//                    0,                       //copy number
//                    checkOverlaps);          //overlaps checking

//  //
//  // Shape 2
//  //
//  G4Material* shape2_mat = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
//  G4ThreeVector pos2 = G4ThreeVector(0, -1*cm, 7*cm);

//  // Trapezoid shape
//  G4double shape2_dxa = 12*cm, shape2_dxb = 12*cm;
//  G4double shape2_dya = 10*cm, shape2_dyb = 16*cm;
//  G4double shape2_dz  = 6*cm;
//  G4Trd* solidShape2 =
//    new G4Trd("Shape2",                      //its name
//              0.5*shape2_dxa, 0.5*shape2_dxb,
//              0.5*shape2_dya, 0.5*shape2_dyb, 0.5*shape2_dz); //its size
                
//  G4LogicalVolume* logicShape2 =
//    new G4LogicalVolume(solidShape2,         //its solid
//                        shape2_mat,          //its material
//                        "Shape2");           //its name
               
//  new G4PVPlacement(0,                       //no rotation
//                    pos2,                    //at position
//                    logicShape2,             //its logical volume
//                    "Shape2",                //its name
//                    logicEnv,                //its mother  volume
//                    false,                   //no boolean operation
//                    0,                       //copy number
//                    checkOverlaps);          //overlaps checking
                
  // Set Shape2 as scoring volume
  //
  //fScoringVolume = logicShape2;

  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
