
/*----------------------------------------------------------------------
Full solution of the multi-layer two-stream equation (solar, thermal) 
with maximum-random overlap assumption for partial cloudiness.        
Author: Nina Črnivec, nina.crnivec@physik.uni-muenchen.de             
----------------------------------------------------------------------*/


/*-------------------------------------------------------------------
 * This file is part of libRadtran.
 * Copyright (c) 1997-2018 by Arve Kylling, Bernhard Mayer,
 *                            Claudia Emde, Robert Buras,
 *			   
 * ######### Contact info: http://www.libradtran.org #########
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License   
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.        
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the   
 * GNU General Public License for more details.                    
 * 
 * You should have received a copy of the GNU General Public License          
 * along with this program; if not, write to the Free Software                
 * Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "equation.h"
#include "twostrebe.h"
#include "twomaxrnd.h"
#include "solver.h"


#define NFLUX 4   // number of diffuse fluxes defined at each level (Edn_c, Edn_f, Eup_c, Eup_f)


static int calcp1p2p3p4 (int nlev, double *cf, double *ar_p1, double *ar_p2, double *ar_p3, double *ar_p4);

static int buildMatrixA (int nlev, double Ag, 
                         double *ar_a11_c, double *ar_a11_f, 
                         double *ar_a12_c, double *ar_a12_f, 
                         double *ar_p1, double *ar_p2, double *ar_p3, double *ar_p4, 
                         double **matrixA);

static int buildVectorBsol (int nlev, double Ag, double mu0, 
                            double *ar_a13_c, double *ar_a13_f, 
                            double *ar_a23_c, double *ar_a23_f, 
                            double *ar_S_c, double *ar_S_f, 
                            double *ar_p1, double *ar_p3, 
                            double *vectB);

static int buildVectorBthe (int nlev, double Ag, double Bg, 
                            double *ar_theComp1_c, double *ar_theComp1_f, 
                            double *ar_theComp2_c, double *ar_theComp2_f,
                            double *cf,
                            double *vectB);

static void freeMemory (int nlev, 
                        double *ar_a11_c, double *ar_a11_f, 
                        double *ar_a12_c, double *ar_a12_f, 
                        double *ar_a13_c, double *ar_a13_f, 
                        double *ar_a23_c, double *ar_a23_f, 
                        double *ar_a33_c, double *ar_a33_f, 
                        double *ar_p1, double *ar_p2, double *ar_p3, double *ar_p4, 
                        double *bb_sol, double *bb_the, 
                        double *ar_theComp1_c, double *ar_theComp1_f, 
                        double *ar_theComp2_c, double *ar_theComp2_f, 
                        double *S_c, double *S_f, 
                        double *Edir_c, double *Edir_f, 
                        double *Eup_c, double *Eup_f, 
                        double *Edn_c, double *Edn_f,
                        double *bb, double *xx, double **AA);

static int twostream_maxrand (double *dtau_c, double *omega0_c, double *g_c, //"_c" for cloudy region
                              double *dtau_f, double *omega0_f, double *g_f, //"_f" for cloud-free region
                              double *cf, int nlev, double S0, double mu0, double Ag, 
                              double Bg, double *B, int delta, int flagSolar, int flagThermal,
                              double **Edir, double **Edn, double **Eup, double **Lavg);

int twomaxrnd (float *dtau_org, float *omega0_org, float *g_org,
               float *dtau_clr_org, float *omega0_clr_org, float *g_clr_org, 
               float *cf, int nlev, 
               double S0, double mu0,
               double Ag,
               int planck,
               int delta,
               int nzout,
               float *zd,
               float *temper,
               float btemp,
               float wvnmlo,
               float wvnmhi,
               float *zout, 
               float *fldn, 
               float *flup, 
               float *fldir,
               float *uavg)
{
  int ilev=0;
  int ilyr=0;
  int lu=0;
  int status=0;
    
  double *dtau_clr_org_d   = calloc (nlev-1, sizeof(double));
  double *omega0_clr_org_d = calloc (nlev-1, sizeof(double));
  double *g_clr_org_d      = calloc (nlev-1, sizeof(double));
    
  double *dtau_org_d   = calloc (nlev-1, sizeof(double));
  double *omega0_org_d = calloc (nlev-1, sizeof(double));
  double *g_org_d      = calloc (nlev-1, sizeof(double));
    
  double *cf_d         = calloc (nlev-1, sizeof(double));
  double *B            = malloc (nlev*sizeof(double)); 
  double *Edir=NULL; 
  double *Edn=NULL;
  double *Eup=NULL;
  double *Lavg=NULL;
    
  const double taumax=100.0;
  double Bg=0.0; // Used only for thermal RT, unused (0.0) for solar RT;
  float plkavg;
    
  int flagSolar=0;  
    
  if(S0 > 0.0) flagSolar = 1;
    
  if(planck){
     for(ilev=0;ilev<nlev;ilev++){  // level temperatures and Planck functions
        F77_FUNC (cplkavg, CPLKAVG) (&wvnmlo, &wvnmhi, &temper[ilev], &plkavg);
        B[ilev] = plkavg;
     }//e-for
        
     // surface temperature and Planck function
     F77_FUNC (cplkavg, CPLKAVG) (&wvnmlo, &wvnmhi, &btemp, &plkavg);
     Bg = plkavg;
  }//e-if
      
  // copy float arrays to double arrays 
  for(ilyr=0;ilyr<nlev-1;ilyr++){
     dtau_org_d[ilyr] = dtau_org[ilyr];
     dtau_clr_org_d[ilyr] = dtau_clr_org[ilyr];

     // restrict layer optical thickness to 100 
     if(dtau_org_d[ilyr] > taumax) 
        dtau_org_d[ilyr] = taumax;

     if(dtau_clr_org_d[ilyr] > taumax) 
        dtau_clr_org_d[ilyr] = taumax;
        
     omega0_org_d[ilyr]     = omega0_org[ilyr];
     omega0_clr_org_d[ilyr] = omega0_clr_org[ilyr];
        
     g_org_d[ilyr]     = g_org[ilyr];
     g_clr_org_d[ilyr] = g_clr_org[ilyr];
        
     cf_d[ilyr] = cf[ilyr];
  }//e-for
       
  // Call twostream_maxrand code:
  status = twostream_maxrand (dtau_org_d, omega0_org_d, g_org_d,
                              dtau_clr_org_d, omega0_clr_org_d, g_clr_org_d,
                              cf_d, nlev, 
                              S0, mu0, Ag, 
                              Bg, B, delta, flagSolar, planck, // flagThermal = planck
                              &Edir, &Edn, &Eup, &Lavg);
  if(status!=0){
     fprintf (stderr, "Error %d returned by twostream_maxrand()\n", status);
     return status;
  }//e-if
    
  // Copy results to final fields:
  for (ilev=0;ilev<nlev;ilev++) {
     for (lu=0;lu<nzout;lu++) {
        if (zout[lu] == zd[ilev]) {
           fldn [lu] = Edn [ilev];
           flup [lu] = Eup [ilev];
           fldir[lu] = Edir[ilev];
           uavg [lu] = Lavg[ilev];
        }//e-if
     }//e-for
  }//e-for
            
  // Free memory:
  free (Edir); 
  free (Edn); 
  free (Eup); 
  free (Lavg);
  free (dtau_org_d); 
  free (dtau_clr_org_d);
  free (omega0_org_d); 
  free (omega0_clr_org_d);
  free (g_org_d); 
  free (g_clr_org_d);
  free (B);
        
  return 0;
}




//===========================
// FUNCTION twostream_maxrand
//===========================

/*
OUTPUT:
Edir = direct irradiance;
Edn  = diffuse downward irradiance;
Eup  = diffuse upward irradiance;
Lavg = radiance (set to NaN);
*/

static int twostream_maxrand (double *dtau_org_c, double *omega0_org_c, double *g_org_c, //"_c" for cloudy region
                              double *dtau_org_f, double *omega0_org_f, double *g_org_f, //"_f" for cloud-free region
                              double *cf, int nlev, double S0, double mu0, double Ag, 
                              double Bg, double *B, int delta, int flagSolar, int flagThermal,
                              double **Edir, double **Edn, double **Eup, double **Lavg)
{
  int nlyr=nlev-1;    
  int ilev;
  int ilyr;
  int iStatus=0; 
    
  double dtau_c; 
  double omega0_c;
  double g_c;
    
  double dtau_f;
  double omega0_f;
  double g_f;
       
  /*
  Eddington coefficients:
  a11 = transmission coefficient for diffuse radiation;
  a12 = reflection coefficient for diffuse radiation; 
  a13 = reflection coefficient for the primary scattered parallel solar radiation;
  a23 = transmission coefficient for the primary scattered parallel solar radiation;
  a33 = transmission coefficient for the direct parallel solar radiation;
  */

  double a11_c;
  double a12_c;
  double a13_c;
  double a23_c;
  double a33_c;
   
  double a11_f;
  double a12_f;
  double a13_f;
  double a23_f;
  double a33_f;
       
  double *ar_a11_c=calloc(nlyr,sizeof(double));
  double *ar_a12_c=calloc(nlyr,sizeof(double));
  double *ar_a13_c=calloc(nlyr,sizeof(double));
  double *ar_a23_c=calloc(nlyr,sizeof(double));
  double *ar_a33_c=calloc(nlyr,sizeof(double));
    
  double *ar_a11_f=calloc(nlyr,sizeof(double));
  double *ar_a12_f=calloc(nlyr,sizeof(double));
  double *ar_a13_f=calloc(nlyr,sizeof(double));
  double *ar_a23_f=calloc(nlyr,sizeof(double));
  double *ar_a33_f=calloc(nlyr,sizeof(double));  
    
  // Components of vector B in the thermal spectral range:
  double theComp1_c;
  double theComp1_f;
  double theComp2_c;
  double theComp2_f;
    
  double *ar_theComp1_c=NULL;
  double *ar_theComp1_f=NULL;
  double *ar_theComp2_c=NULL;
  double *ar_theComp2_f=NULL;
     
  // Parameters related to cloud cover of two adjacent layers:
  // in Zdunkowski (pages: 180-183) denoted as: b1,b2,b3,b4;
  // here: p1, p2, p3, p4;   
  double *ar_p1=calloc(nlyr,sizeof(double));
  double *ar_p2=calloc(nlyr,sizeof(double));
  double *ar_p3=calloc(nlyr,sizeof(double));
  double *ar_p4=calloc(nlyr,sizeof(double));
    
  double **AA=NULL;
  double *bb_sol=NULL;
  double *bb_the=NULL; 
  double *bb=NULL;
  double *xx=NULL; // result vector
    
  double *S_c=NULL;  
  double *S_f=NULL;
    
  double *Edir_c=NULL;
  double *Edir_f=NULL;
    
  double *Eup_c;
  double *Eup_f;
    
  double *Edn_c;
  double *Edn_f;
    
  AA=calloc(NFLUX*nlev,sizeof(double*));
    
  for(ilev=0;ilev<NFLUX*nlev;ilev++){
     if((AA[ilev]=calloc(NFLUX*nlev, sizeof(double)))==NULL){
        fprintf (stderr, "Error allocating memory for AA[%d]\n", ilev);
        return -1;
     }//e-if
  }//e-for
    
  if(flagSolar){ 
     bb_sol=calloc(NFLUX*nlev,sizeof(double)); 
  }//e-if
    
  if(flagThermal){
     ar_theComp1_c=calloc(nlev,sizeof(double)); 
     ar_theComp1_f=calloc(nlev,sizeof(double)); 
     ar_theComp2_c=calloc(nlev,sizeof(double));
     ar_theComp2_f=calloc(nlev,sizeof(double)); 
     bb_the=calloc(NFLUX*nlev, sizeof(double));	
  }//e-if
    
  bb=calloc(NFLUX*nlev,sizeof(double)); 
  xx=calloc(NFLUX*nlev,sizeof(double)); 
     
  S_c=calloc(nlev,sizeof(double));
  S_f=calloc(nlev,sizeof(double));
  Edir_c=calloc(nlev,sizeof(double));
  Edir_f=calloc(nlev,sizeof(double));
    
  Eup_c=calloc(nlev,sizeof(double));
  Eup_f=calloc(nlev,sizeof(double));
    
  Edn_c=calloc(nlev,sizeof(double));
  Edn_f=calloc(nlev,sizeof(double));
    
  // Final total irradiances (sum: c + f):
  *Edir=calloc(nlev,sizeof(double));
  *Eup=calloc(nlev,sizeof(double));
  *Edn=calloc(nlev,sizeof(double));
  *Lavg=calloc(nlev,sizeof(double));
    
  // At the moment it is only possible to calculate solar OR thermal RT, but not both simultaneously;
  if ((flagSolar && flagThermal) || (!flagSolar && !flagThermal)){
     fprintf (stderr, "Error - invalid input parameters - use flagSolar and flagThermal alternatingly\n");
     fprintf (stderr, "flagSolar = %d , flagThermal = %d\n", flagSolar, flagThermal);
     freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                 ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                 bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                 S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
     return -2;
  }//e-if
  
  // Calculate vertical profiles of p1, p2, p3 and p4 from vertical profile of cloud fraction:
  iStatus = calcp1p2p3p4(nlev, cf, ar_p1, ar_p2, ar_p3, ar_p4);

  if(iStatus != 0){
     fprintf (stderr, "Error calculating vertical profiles of p1, p2, p3 and p4 from cloud cover; ERROR=%d \n", iStatus);
     freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                 ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                 bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                 S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
     return -3;
  }//e-if
    
    
  // Calculate vertical profiles of Eddington coefficients
  // and vertical profiles of thermal components:
  for(ilyr=0;ilyr<nlyr;ilyr++){
      
     // Delta scaling of optical properties of cloudy regions: 
     if(delta){
        delta_scale_hg (dtau_org_c[ilyr], omega0_org_c[ilyr], g_org_c[ilyr], 
                         &dtau_c, &omega0_c, &g_c);                    
     }else{
        dtau_c   = dtau_org_c[ilyr];
        omega0_c = omega0_org_c[ilyr];
        g_c      = g_org_c[ilyr];
     }//e-if
        
     // Optical properties of cloud-free regions: 
     dtau_f   = dtau_org_f[ilyr];
     omega0_f = omega0_org_f[ilyr];
     g_f      = g_org_f[ilyr];  
        
     // omega0 should not be 1 (avoiding singularity problem), restrict omega0 to 0.999999: 
     if(omega0_c > 0.999999) omega0_c = 0.999999;
     if(omega0_f > 0.999999) omega0_f = 0.999999;
        
     eddington_coeffc (dtau_c, g_c, omega0_c, mu0, &a11_c, &a12_c, &a13_c, &a23_c, &a33_c); 
     eddington_coeffc (dtau_f, g_f, omega0_f, mu0, &a11_f, &a12_f, &a13_f, &a23_f, &a33_f); 
        
     ar_a11_c[ilyr] = a11_c;
     ar_a11_f[ilyr] = a11_f;
        
     ar_a12_c[ilyr] = a12_c;
     ar_a12_f[ilyr] = a12_f;
        
     ar_a13_c[ilyr] = a13_c;
     ar_a13_f[ilyr] = a13_f;
        
     ar_a23_c[ilyr] = a23_c;
     ar_a23_f[ilyr] = a23_f;
        
     ar_a33_c[ilyr] = a33_c;
     ar_a33_f[ilyr] = a33_f;
        
     if(flagThermal){
        calcThermalComponents(ilyr, B, dtau_c, omega0_c, g_c, &theComp1_c, &theComp2_c);
        calcThermalComponents(ilyr, B, dtau_f, omega0_f, g_f, &theComp1_f, &theComp2_f);
            
        ar_theComp1_c[ilyr] = theComp1_c;
        ar_theComp1_f[ilyr] = theComp1_f;
            
        ar_theComp2_c[ilyr] = theComp2_c;
        ar_theComp2_f[ilyr] = theComp2_f;
     }//e-if
  }//e-for
    
  // Initialize vectors S_c and S_f:
  // S[0]= S0 = S_c[0] + S_f[0] = cf[0]*S0 + (1.0-cf[0])*S0;
  if(flagSolar){
     S_c[0]=cf[0]*S0;    
     S_f[0]=(1.0-cf[0])*S0;
        
     for(ilev=1;ilev<nlev;ilev++){
        S_c[ilev]=ar_a33_c[ilev-1]*((1.0-ar_p1[ilev-1])*S_f[ilev-1] + ar_p3[ilev-1]*S_c[ilev-1]); 
        S_f[ilev]=ar_a33_f[ilev-1]*(ar_p1[ilev-1]*S_f[ilev-1] + (1.0-ar_p3[ilev-1])*S_c[ilev-1]); 
     }//e-for
  }//e-if
    
  // Equation system has the following form: xx = AA*xx + bb;
    
  // Build matrix AA:  
  iStatus = buildMatrixA (nlev, Ag, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, 
                          ar_p1, ar_p2, ar_p3, ar_p4, AA);
  if(iStatus != 0){
     fprintf (stderr, "buildMatrixA ERROR=%d \n", iStatus);
     freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                 ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                 bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                 S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
     return -4;
  }//e-if
        
  // Make matrix A2 = AA - IdentityMatrix and save it to the same matrix AA,
  // since the equation system in the form Ax=b must be passed to solve_gauss;
  // xx = AA*xx + bb; 
  // (AA-II)*xx = -bb;

  iStatus = makeMatrixA2 (nlev, NFLUX, AA);

  if(iStatus != 0){
     fprintf (stderr, "makeMatrixA2 ERROR=%d \n", iStatus);
     freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                 ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                 bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                 S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
     return -5;
  }//e-if
    
  // Build vector bb_sol:
  if(flagSolar){
     iStatus = buildVectorBsol (nlev, Ag, mu0, ar_a13_c, ar_a13_f, ar_a23_c, ar_a23_f, S_c, S_f, ar_p1, ar_p3, bb_sol);

     if(iStatus != 0){
        fprintf (stderr, "buildVectorBsol ERROR=%d \n", iStatus);
        freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                    ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                    bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                    S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
        return -6;
     }//e-if
  }//e-if
    
  // Build vector bb_the:
  if(flagThermal){
     iStatus = buildVectorBthe (nlev, Ag, Bg, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, cf, bb_the);

     if(iStatus != 0){
        fprintf (stderr, "buildVectorBthe ERROR=%d \n", iStatus);
        freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                    ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                    bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                    S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
        return -7;
     }//e-if
  }//e-if
    
  // Assign bb_sol or bb_the to the final vector bb:
  for(ilev=0; ilev<NFLUX*nlev; ilev++){
     bb[ilev] = 0;

     if(flagSolar){
        bb[ilev] += bb_sol[ilev];
     }//e-if

     if(flagThermal){
        bb[ilev] += bb_the[ilev];
     }//e-if

  }//e-for
        
  // Make vector -bb and save it to the same vector bb:
  iStatus = makeVectorMinusB (nlev, NFLUX, bb);

  if(iStatus != 0){
     fprintf (stderr, "makeVectorMinusB ERROR=%d \n", iStatus);
     freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                 ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                 bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                 S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
     return -8;
  }//e-if
    
  // Solve AA*xx=bb to obtain xx:
  // AA is a 11-diagonal matrix; calculation could be sped up with a 11
  iStatus = solve_gauss (AA, bb, NFLUX*nlev, &xx);

  if(iStatus != 0){
     fprintf (stderr, "Error %d solving equation system\n", iStatus);
     freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
                 ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
                 bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
                 S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
     return -9;
  }//e-if
           
  for(ilev=0;ilev<nlev;ilev++){
     Eup_f[ilev] = xx[NFLUX*ilev];
     Eup_c[ilev] = xx[NFLUX*ilev+1];
     Edn_f[ilev] = xx[NFLUX*ilev+2];
     Edn_c[ilev] = xx[NFLUX*ilev+3];
  }//e-for
      
  for(ilev=0;ilev<nlev;ilev++){
     Edir_c[ilev] = S_c[ilev]*mu0;  
     Edir_f[ilev] = S_f[ilev]*mu0;  
  }//e-for
      
  // Sum up the irradiances for cloudy and cloud-free regions to obtain the final result:
  for(ilev=0;ilev<nlev;ilev++){
     (*Edir)[ilev] = Edir_c[ilev] + Edir_f[ilev];
     (*Eup)[ilev]  = Eup_c[ilev] + Eup_f[ilev];  
     (*Edn)[ilev]  = Edn_c[ilev] + Edn_f[ilev];  
     (*Lavg)[ilev] = NAN;
  }//e-for  
       
  freeMemory (nlev, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_a13_c, ar_a13_f, 
              ar_a23_c, ar_a23_f, ar_a33_c, ar_a33_f, ar_p1, ar_p2, ar_p3, ar_p4, 
              bb_sol, bb_the, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, 
              S_c, S_f, Edir_c, Edir_f, Eup_c, Eup_f, Edn_c, Edn_f, bb, xx, AA);
       
  return 0;
}//e-twostream_maxrand





//======================
// FUNCTION calcp1p2p3p4
//======================
// INPUT:  nlev, cf;
// OUTPUT: ar_p1, ar_p2, ar_p3, ar_p4;

/*  
* From Zdunkowski (pages 180-183): 
* p1 = (1 - max(cf[i], cf[i-1])) / (1 - cf[i-1])  
* p2 = (1 - max(cf[i], cf[i+1]) / (1 - cf[i+1])  
* p3 = min(cf[i], cf[i-1]) / cf[i-1]  
* p4 = min(cf[i], cf[i+1]) / cf[i+1]

* Remark from Zdunkowski:
* It is noteworthy that a particular coefficient pj (j=1,2,3,4) is set equal to 1 
* if an undetermined expression 0/0 occurs.
* This follows from physical reasoning or from applying l'Hopital's rule.
*/

static int calcp1p2p3p4 (int nlev, double *cf, double *ar_p1, double *ar_p2, double *ar_p3, double *ar_p4)
{
  int ilyr; 
  int nlyr;
    
  double max_p1; // = max(cf[ilyr],cf[ilyr-1]); appears in the expression for p1;
  double min_p3; // = min(cf[ilyr],cf[ilyr-1]); appears in the expression for p3;
  double max_p2; // = max(cf[ilyr],cf[ilyr+1]); appears in the expression for p2;
  double min_p4; // = min(cf[ilyr],cf[ilyr+1]); appears in the expression for p4;
    
  nlyr=nlev-1;
    
  /*
  * FORMULAS:
  * ar_p1[ilyr] = (1.0 - max(cf[ilyr], cf[ilyr-1])) / (1.0 - cf[ilyr-1]);
  * ar_p2[ilyr] = (1.0 - max(cf[ilyr], cf[ilyr+1]) / (1.0 - cf[ilyr+1]); 
  * ar_p3[ilyr] = min(cf[ilyr], cf[ilyr-1]) / cf[ilyr-1]; 
  * ar_p4[ilyr] = min(cf[ilyr], cf[ilyr+1]) / cf[ilyr+1];   
  */
      
  // Calculate vertical profiles of p1, p3:
  // Special case: ilyr=0;
  for(ilyr=0;ilyr<nlyr;ilyr++){
        
     if(ilyr == 0){
        ar_p1[ilyr] = 1.0; 
        ar_p3[ilyr] = 1.0; 
     }else{  
            
        // Find max_p1:
        if(cf[ilyr] > cf[ilyr-1]){
           max_p1 = cf[ilyr];
        }else{
           max_p1 = cf[ilyr-1];
        }//e-if
            
        // Find_min_p3:
        if(cf[ilyr] < cf[ilyr-1]){
           min_p3 = cf[ilyr];
        }else{
           min_p3 = cf[ilyr-1];
        }//e-if
            
        if(cf[ilyr-1] == 1.0) ar_p1[ilyr] = 1.0;
        else ar_p1[ilyr] = (1.0 - max_p1) / (1.0 - cf[ilyr-1]);
            
        if(cf[ilyr-1] == 0.0) ar_p3[ilyr] = 1.0;
        else ar_p3[ilyr] = min_p3 / cf[ilyr-1];  
            
     }//e-if  
  }//e-for over ilyr;
    
  // Calculate vertical profiles of p2 and p4:
  // Special case: ilyr = nlyr-1 = nlev-2;
  for(ilyr=0;ilyr<nlyr;ilyr++){
        
     if(ilyr == (nlyr-1)){ 
        ar_p2[ilyr] = 1.0; 
        ar_p4[ilyr] = 1.0; 
     }else{ 
            
        // Find max_p2:
        if(cf[ilyr] > cf[ilyr+1]){
           max_p2 = cf[ilyr];
        }else{
           max_p2 = cf[ilyr+1];
        }//e-if
            
        // Find_min_p4:
        if(cf[ilyr] < cf[ilyr+1]){
           min_p4 = cf[ilyr];
        }else{
           min_p4 = cf[ilyr+1];
        }//e-if
            
        if(cf[ilyr+1] == 1.0) ar_p2[ilyr] = 1.0;
        else ar_p2[ilyr] = (1.0 - max_p2) / (1.0 - cf[ilyr+1]);  
            
        if(cf[ilyr+1] == 0.0) ar_p4[ilyr] = 1.0;
        else ar_p4[ilyr] = min_p4 / cf[ilyr+1];
     }//e-if
  }//e-for
    
  return 0;
}//e-calcp1p2p3p4



//===========================
// FUNCTION: eddington_coeffc
//===========================

/* 
Calculate Eddington coefficients a11, a12, a13, a23, and a33 
from layer optical thickness dtau, asymmetry parameter g, 
single scattering albedo omega0, and cosine of solar zenith angle mu0.
*/

void eddington_coeffc (double dtau, double g, double omega0, double mu0,
                       double *a11, double *a12, double *a13, double *a23, double *a33)
{
  double alpha1=0, alpha2=0, alpha3=0, alpha4=0, alpha5=0, alpha6=0;
  double lambda=0, b=0, A=0;
  double denom=0;
    
  if(omega0==0){
     alpha1 = 1.75;
     alpha2 = -0.25;
  }
  else{
     alpha1 = (1.0-omega0)+0.75*(1.0-omega0*g);
     alpha2 = -(1.0-omega0)+0.75*(1.0-omega0*g);
  }//e-if
    
  lambda=sqrt(alpha1*alpha1-alpha2*alpha2);
    
  A=1.0/(alpha2/(alpha1-lambda)*exp(lambda*dtau)-alpha2/(alpha1+lambda)*exp(-lambda*dtau));
    
  *a11=A*2.0*lambda/alpha2;
  *a12=A*(exp(lambda*dtau)-exp(-lambda*dtau));
    
  b=0.5-0.75*g*mu0;
  alpha3=-omega0*b; 
  alpha4=omega0*(1-b);
    
  denom = (1.0/mu0/mu0-lambda*lambda);
  alpha5=((alpha1-1.0/mu0)*alpha3-alpha2*alpha4)/denom;
  alpha6=(alpha2*alpha3-(alpha1+1.0/mu0)*alpha4)/denom;
    
  *a33=exp(-dtau/mu0);
    
  *a13=alpha5*(1.0-(*a11)*(*a33))-alpha6*(*a12);
  *a23=-(*a12)*alpha5*(*a33)+alpha6*((*a33)-(*a11));
    
}//e-eddington_coeffc



//================================
// FUNCTION: calcThermalComponents
//================================
// INPUT:  ilyr, B, dtau, omega0, g;
// OUTPUT: theComp1, theComp2;

// theComp1 = Planckian emission term of a given layer (upward direction);
// theComp2 = Planckian emission term of a given layer (downward direction);

void calcThermalComponents (int ilyr, double *B, double dtau, double omega0, double g,
                            double *theComp1, double *theComp2)
{
  double B0;
  double B1;
  double alpha1;
  double alpha2;
  double lambda;
  double A;
  double a11;
  double a12;
  double kappa;
  double alpha7;
  double alpha8;
  double alpha9;
  double alpha10;
    
  // Calculate B0, B1:
  if(dtau>0.01){
     B1 = (B[ilyr+1]-B[ilyr])/dtau;
     B0 = B[ilyr];
  }
  else{
     B1 = 0;
     B0 = (B[ilyr+1]+B[ilyr])*0.5;
  }//e-if

  if(omega0==0){
     alpha1 = 1.75;
     alpha2 = -0.25;
  }
  else{
     alpha1 = (1.0-omega0)+0.75*(1.0-omega0*g);
     alpha2 = -(1.0-omega0)+0.75*(1.0-omega0*g);
  }//e-if
    
  lambda=sqrt(alpha1*alpha1-alpha2*alpha2);
    
  A=1.0/(alpha2/(alpha1-lambda)*exp(lambda*dtau)-alpha2/(alpha1+lambda)*exp(-lambda*dtau));
    
  a11=A*2.0*lambda/alpha2;
  a12=A*(exp(lambda*dtau)-exp(-lambda*dtau));
    
  kappa = 2.0*M_PI*(1.0-omega0);
  alpha7 = kappa*(B0/(alpha1-alpha2) + B1/lambda/lambda);
  alpha9 = kappa*(B0/(alpha1-alpha2) - B1/lambda/lambda);
    
  alpha8 = kappa*B1/(alpha1-alpha2);
  alpha10 = alpha8;
    
  *theComp1 = -a11*(alpha7+alpha8*dtau)-a12*(alpha9)+alpha7;
  *theComp2 = -a12*(alpha7+alpha8*dtau)-a11*(alpha9)+alpha9+alpha10*dtau;
    
}//e-calcThermalComponents



//=======================
// FUNCTION: buildMatrixA
//=======================
// INPUT:  nlev, Ag, ar_a11_c, ar_a11_f, ar_a12_c, ar_a12_f, ar_p1, ar_p2, ar_p3, ar_p4;
// OUTPUT: matrixA;

static int buildMatrixA (int nlev, double Ag, 
                         double *ar_a11_c, double *ar_a11_f, 
                         double *ar_a12_c, double *ar_a12_f, 
                         double *ar_p1, double *ar_p2, double *ar_p3, double *ar_p4, 
                         double **matrixA)
{
  int i;     // position in levels (e.g. for nlev=21, i in range: 0 - 20)
  int iRow;  // row position in matrix A (e.g. for nlev=21, iRow in range: 0 - 83)
    
  int nextColEup; // index of column for next Eup data
  int nextColEdw; // index of column for next Edn data; nextColEdn = nextColEup - 4   
    
  // Set values for initial four rows of matrix A:
  // First row:
  matrixA[0][2] = ar_a12_f[0]*ar_p1[0];
  matrixA[0][3] = ar_a12_f[0]*(1.0-ar_p3[0]);
  matrixA[0][4] = ar_a11_f[0]*ar_p2[0];
  matrixA[0][5] = ar_a11_f[0]*(1.0-ar_p4[0]);

  // Second row: 
  matrixA[1][2] = ar_a12_c[0]*(1.0-ar_p1[0]);
  matrixA[1][3] = ar_a12_c[0]*ar_p3[0];
  matrixA[1][4] = ar_a11_c[0]*(1.0-ar_p2[0]);
  matrixA[1][5] = ar_a11_c[0]*ar_p4[0];
  // Third row is already zero; (needs to be zero due to upper boundary condition);
  // Forth row is already zero; (needs to be zero due to upper boundary condition);  
    
  nextColEup = 6; // index of column for Eup(level1)
  nextColEdw = 2; // index of column for Edn(level1)
    
  for(i=1;i<nlev;i++){	
     iRow = NFLUX*i;
        
     if(i == (nlev-1)){ // lower boundary condition for the forth and third row from bottom up of matrix A;
        matrixA[iRow][nextColEup] = Ag;
        matrixA[iRow+1][nextColEup+1] = Ag;
     }else{
        matrixA[iRow][nextColEup]   = ar_a12_f[i]*ar_p1[i];		
        matrixA[iRow][nextColEup+1] = ar_a12_f[i]*(1.0-ar_p3[i]);
        matrixA[iRow][nextColEup+2] = ar_a11_f[i]*ar_p2[i];
        matrixA[iRow][nextColEup+3] = ar_a11_f[i]*(1.0-ar_p4[i]);
            
        matrixA[iRow+1][nextColEup]   = ar_a12_c[i]*(1.0-ar_p1[i]);
        matrixA[iRow+1][nextColEup+1] = ar_a12_c[i]*ar_p3[i];
        matrixA[iRow+1][nextColEup+2] = ar_a11_c[i]*(1.0-ar_p2[i]);
        matrixA[iRow+1][nextColEup+3] = ar_a11_c[i]*ar_p4[i];
     }//e-if
     nextColEup += NFLUX;
        
     matrixA[iRow+2][nextColEdw]   = ar_a11_f[i-1]*ar_p1[i-1];
     matrixA[iRow+2][nextColEdw+1] = ar_a11_f[i-1]*(1.0-ar_p3[i-1]);
     matrixA[iRow+2][nextColEdw+2] = ar_a12_f[i-1]*ar_p2[i-1];
     matrixA[iRow+2][nextColEdw+3] = ar_a12_f[i-1]*(1.0-ar_p4[i-1]);
        
     matrixA[iRow+3][nextColEdw]   = ar_a11_c[i-1]*(1.0-ar_p1[i-1]);	
     matrixA[iRow+3][nextColEdw+1] = ar_a11_c[i-1]*ar_p3[i-1];
     matrixA[iRow+3][nextColEdw+2] = ar_a12_c[i-1]*(1.0-ar_p2[i-1]);
     matrixA[iRow+3][nextColEdw+3] = ar_a12_c[i-1]*ar_p4[i-1];
        
     nextColEdw += NFLUX;
  }//e-for
    
  return 0;
}//e-buildMatrixA



//======================
// FUNCTION makeMatrixA2 (= write -1 to the diagonal elements of matrix A)
//======================

int makeMatrixA2 (int nlev, int nFluxPerLevel, double **matrixA)
{
  int iRow;

  for(iRow=0; iRow < nFluxPerLevel*nlev; iRow++){
     matrixA[iRow][iRow] = -1.0;

  }//e-for
  return 0;
}//e-makeMatrixA2



//=========================
// FUNCTION buildVectorBsol
//=========================
// INPUT:  nlev, Ag, mu0, ar_a13_c, ar_a13_f, ar_a23_c, ar_a23_f, ar_S_c, ar_S_f, ar_p1, ar_p3;
// OUTPUT: vectB;

static int buildVectorBsol (int nlev, double Ag, double mu0, 
                            double *ar_a13_c, double *ar_a13_f, 
                            double *ar_a23_c, double *ar_a23_f, 
                            double *ar_S_c, double *ar_S_f, 
                            double *ar_p1, double *ar_p3, 
                            double *vectB)
{
  int i;  // position in levels
  int j;  // position in vector B
    
  // Set initial four values: 
  vectB[0] = ar_a13_f[0]*ar_S_f[0]; 
  vectB[1] = ar_a13_c[0]*ar_S_c[0];
  vectB[2] = 0.0; // upper boundary condition
  vectB[3] = 0.0; // upper boundary condition
       
  for(i=1; i<(nlev-1);i++){
     j = NFLUX*i;     
     vectB[j]   = ar_a13_f[i]*(ar_p1[i]*ar_S_f[i] + (1.0-ar_p3[i])*ar_S_c[i]);
     vectB[j+1] = ar_a13_c[i]*((1.0-ar_p1[i])*ar_S_f[i] + ar_p3[i]*ar_S_c[i]);
     vectB[j+2] = ar_a23_f[i-1]*(ar_p1[i-1]*ar_S_f[i-1] + (1.0-ar_p3[i-1])*ar_S_c[i-1]);
     vectB[j+3] = ar_a23_c[i-1]*((1.0-ar_p1[i-1])*ar_S_f[i-1] + ar_p3[i-1]*ar_S_c[i-1]); 
  }//e-for
       
  // Treat last four values seperately:
  j = NFLUX*(nlev-1); 
  vectB[j]   = Ag*mu0*ar_S_f[nlev-1]; // lower boundary condition
  vectB[j+1] = Ag*mu0*ar_S_c[nlev-1]; // lower boundary condition
  vectB[j+2] = ar_a23_f[nlev-2]*(ar_p1[nlev-2]*ar_S_f[nlev-2] + (1.0-ar_p3[nlev-2])*ar_S_c[nlev-2]);
  vectB[j+3] = ar_a23_c[nlev-2]*((1.0-ar_p1[nlev-2])*ar_S_f[nlev-2] + ar_p3[nlev-2]*ar_S_c[nlev-2]); 
    
  return 0;
}//e-buildVectorBsol



//=========================
// FUNCTION buildVectorBthe
//=========================
// INPUT:  nlev, Ag, Bg, ar_theComp1_c, ar_theComp1_f, ar_theComp2_c, ar_theComp2_f, cf;
// OUTPUT: bb_the;

// thermal component 1 = upward emission
// thermal component 2 = downward emission

static int buildVectorBthe (int nlev, double Ag, double Bg, 
                            double *ar_theComp1_c, double *ar_theComp1_f, 
                            double *ar_theComp2_c, double *ar_theComp2_f,
                            double *cf,
                            double *vectB)
{
  int i;  // position in levels
  int j;  // position in vector B
  double constPi = 3.141593;
 
  // Set initial four values: 
  vectB[0] = (1.0-cf[0])*ar_theComp1_f[0]; 
  vectB[1] = cf[0]*ar_theComp1_c[0];
  vectB[2] = 0.0; // upper boundary condition
  vectB[3] = 0.0; // upper boundary condition
        
  for(i=1; i<(nlev-1); i++){
     j = NFLUX*i;     
     vectB[j]   = (1.0-cf[i])*ar_theComp1_f[i];
     vectB[j+1] = cf[i]*ar_theComp1_c[i];
     vectB[j+2] = (1.0-cf[i-1])*ar_theComp2_f[i-1];
     vectB[j+3] = cf[i-1]*ar_theComp2_c[i-1]; 
  }//e-for
   
  // Treat last four values separately:
  // i=nlev-1; // bottom (ground) level;
  j = NFLUX*(nlev-1); 
  vectB[j]   = (1.0-cf[nlev-2])*(1.0-Ag)*constPi*Bg; // lower boundary condition 
  vectB[j+1] = cf[nlev-2]*(1.0-Ag)*constPi*Bg;       // lower boundary condition 
  vectB[j+2] = (1.0-cf[nlev-2])*ar_theComp2_f[nlev-2]; 
  vectB[j+3] = cf[nlev-2]*ar_theComp2_c[nlev-2]; 
    
  return 0;
}//e-buildVectorBthe



//==========================
// FUNCTION makeVectorMinusB
//==========================

int makeVectorMinusB (int nlev, int nFluxPerLevel, double *vectB)
{
  int iRow;

  for(iRow=0; iRow < nFluxPerLevel*nlev; iRow++){
     vectB[iRow] = -vectB[iRow];
  }//e-for

  return 0;
}//e-makeVectorMinusB 



//=====================
// FUNCTION: freeMemory
//=====================
// Function to free all allocated memory

static void freeMemory (int nlev, 
                        double *ar_a11_c, double *ar_a11_f, 
                        double *ar_a12_c, double *ar_a12_f, 
                        double *ar_a13_c, double *ar_a13_f, 
                        double *ar_a23_c, double *ar_a23_f, 
                        double *ar_a33_c, double *ar_a33_f, 
                        double *ar_p1, double *ar_p2, double *ar_p3, double *ar_p4, 
                        double *bb_sol, double *bb_the, 
                        double *ar_theComp1_c, double *ar_theComp1_f, 
                        double *ar_theComp2_c, double *ar_theComp2_f, 
                        double *S_c, double *S_f, 
                        double *Edir_c, double *Edir_f, 
                        double *Eup_c, double *Eup_f, 
                        double *Edn_c, double *Edn_f,
                        double *bb, double *xx, double **AA)
{
  int i;
    
  free(ar_a11_c);
  free(ar_a11_f);
  free(ar_a12_c);
  free(ar_a12_f);
  free(ar_a13_c);
  free(ar_a13_f);
  free(ar_a23_c);
  free(ar_a23_f);
  free(ar_a33_c);
  free(ar_a33_f);
  free(ar_p1);
  free(ar_p2);
  free(ar_p3);
  free(ar_p4);
    
  if(ar_theComp1_c != 0) free(ar_theComp1_c);
  if(ar_theComp1_f != 0) free(ar_theComp1_f);         
  if(ar_theComp2_c != 0) free(ar_theComp2_c);
  if(ar_theComp2_f != 0) free(ar_theComp2_f); 
  if(bb_sol != 0)        free(bb_sol);
  if(bb_the != 0)        free(bb_the); 
    
  free(S_c);
  free(S_f); 
  free(Edir_c);
  free(Edir_f);    
  free(Eup_c);
  free(Eup_f);
  free(Edn_c);
  free(Edn_f);
  free(bb);
  free(xx);
      
  for(i=0; i < NFLUX*nlev; i++) free(AA[i]);    
  
}//e-freeMemory

