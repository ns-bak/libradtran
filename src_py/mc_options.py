"""--------------------------------------------------------------------
 * $Id: mc_options.py 3523 2019-12-20 13:14:51Z Claudia.Emde $
 * 
 * This file is part of libRadtran.
 * Copyright (c) 1997-2012 by Arve Kylling, Bernhard Mayer,
 *                            Claudia Emde, Robert Buras
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
 *--------------------------------------------------------------------"""

from option_definition import *

class setup_mc_group():
	
	group_name = 'Monte Carlo'

	def __init__(self):
		documentation = get_mc_documentation()

		mc_escape = not_yet_lex2py_option(  #TODO: bad implementation in lex_starter.l 
			name='mc_escape',
			group='mc',
			helpstr='Calculate MYSTIC radiances via escape probabilities.', 
			documentation=documentation['mc_escape'],
			gui_inputs=(ListInput(name='on/off',valid_range=['on','off']),),
			tokens=addToken(name="", datatype=str),
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True
		)
		mc_core = option(
			name='mc_core',
			group='mc',
			helpstr='Activate massive delta scaling in the core of thick clouds.', 
			documentation=documentation['mc_core'],
			# gui_inputs=(ListInput(name='on/off',valid_range=['on','off'],optional=True),),
			tokens= [ addSetting(name="Input.rte.mc.core_isactive", setting=1),
																addSetting(name='isp', setting='get_caoth_index(&Input.caoth,&Input.n_caoth,"hiddencore_dummy",0)'),
																addSetting(name='iwc', setting='get_caoth_index(&Input.caoth,&Input.n_caoth,"wc",0)'),
																addSetting(name='Input.caoth[isp].source', setting='CAOTH_FROM_3D'),
																addSetting(name='Input.caoth[isp].filename', setting='Input.caoth[iwc].filename'),
																addSetting(name='Input.caoth[isp].properties', setting='PROP_HU'),
																addToken(name='Input.rte.mc.core_inputfile', datatype=str, optional=True, default="NULL"),
			],
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
                        developer = True,
		)

		mc_core_threshold = option(
			name='mc_core_threshold',
			group='mc',
			documentation=documentation['mc_core_threshold'],
			tokens=[addToken(name='Input.rte.mc.core_threshold', datatype=float,valid_range=[0,10000],default=3)],
                        developer=True,
		)

		mc_core_scale = option(
			name='mc_core_scale',
			group='mc',
			documentation=documentation['mc_core_scale'],
			tokens=[addToken(name='Input.rte.mc.core_scale', datatype=float,valid_range=[-100000,0.9999999999],default=-1)],
                        developer=True,
		)

		mc_core_savefile = option(
			name='mc_core_savefile',
			group='mc',
			documentation=documentation['mc_core_savefile'],
			tokens=[addToken(name='Input.rte.mc.core_savefile', datatype=str, optional=True, default="NULL")],
                        developer=True,
		)

		mc_vroom = option(
			name='mc_vroom',
			group='mc',
			helpstr='Use Variance Reduction Methods in MYSTIC.', 
			documentation=documentation['mc_vroom'],
			gui_inputs=(ListInput(name='on/off',valid_range=['on','off'],optional=True),),
			tokens= [ addSetting(name="Input.rte.mc.vroom", setting=1),
				addLogical(name='Input.rte.mc.vroom', logicals=['on','off'], setting='SWITCH_', optional=True) ],
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True
		)
				
		mc_visualize = option(
			name='mc_visualize',
			group='mc',
			helpstr='Switch on OpenGL visualization for MYSTIC.', 
			documentation=documentation['mc_visualize'],
			gui_inputs=(ListInput(name='hiddenline',valid_range = ['hiddenline'],optional=True),),
			tokens= [ addSetting(name="Input.rte.mc.visualize", setting=1),
				addLogical(name="GLmystic_hiddenline",logicals=['hiddenline'], optional=True) ],
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			threedmystic =True
		)
				
		mc_truncate = option(  
			name='mc_truncate',
			group='mc',
			helpstr='Truncate phase function at the specified polar angle mu.', 
			documentation=documentation['mc_truncate'],
			tokens = [ addSetting(name="Input.rte.mc.truncate", setting=0.99),
				addToken(name="Input.rte.mc.truncate", datatype=float, optional=True) ], 
			gui_inputs=( FloatInput(name='Polar angle mu',optional=True, default=0.99), ) ,
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			threedmystic =True,
			showInGui =False,
                        islidar = True
		)
				
		mc_backward = option(
			name='mc_backward',
			group='mc',
			helpstr='Backward tracing of photons.', 
			documentation=documentation['mc_backward'],
			gui_inputs=(IntegerInput(name='mc_backward_islower',optional=True),IntegerInput(name='mc_backward_jslower',optional=True),IntegerInput(name='mc_backward_isupper',optional=True),IntegerInput(name='mc_backward_jsupper',optional=True),),
			tokens = [ addSetting( name="Input.rte.mc.backward.yes", setting=1),
				addToken(name="Input.rte.mc.backward.islower", datatype=int, optional=True, default='NOT_DEFINED_INTEGER'),
				addToken(name="Input.rte.mc.backward.jslower", datatype=int, optional=True, default='NOT_DEFINED_INTEGER'),
				addToken(name="Input.rte.mc.backward.isupper", datatype=int, optional=True, default='NOT_DEFINED_INTEGER'),
				addToken(name="Input.rte.mc.backward.jsupper", datatype=int, optional=True, default='NOT_DEFINED_INTEGER') ] ,
			parents=['uvspec'],
			non_parents= ['mc_forward_output' ],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			childs=['mc_backward_output', 'mc_backward_writeallpixels',
				'mc_backward_writeback','mc_panorama_view', 'mc_sensordirection',
				'mc_sensorposition', 'mc_reference_to_nn','mc_bw_umu_file',
				]
		)

		mc_sample_cldprp = option(
			name='mc_sample_cldprp', 
			group='mc',  
			helpstr='Turns on sampling of cloud properties by photons.',
			documentation=documentation['mc_sample_cldprp'],
			tokens=[ addSetting(name='Input.rte.mc.backward.yes', setting=1, default=0),
				   addSetting(name='Input.rte.mc.sample_cldprp', setting=1) ], 
			parents=['mc_backward'],  
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			threedmystic =True,
			showInGui = False
		)

		mc_surface_reflectalways = option(
			name='mc_surface_reflectalways',
			group='mc',  
			helpstr='The photon is reflected and the albedo is considered by reducing the photon weight.',
			documentation=documentation['mc_surface_reflectalways'],
			tokens=[ addSetting(name='Input.rte.mc.reflectalways', setting=1, default=0)], 
			parents=['uvspec'],  
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui=False,
		)
		
		mc_DoLE = option(
			name='mc_DoLE',
			group='mc',  
			helpstr='Double local estimate of specular ocean reflection to speed up convergence.',
			documentation=documentation['mc_DoLE'],
			tokens=[ addSetting(name='Input.rte.mc.DoLE', setting=1, default=0)], 
			parents=['uvspec'],  
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui=False,
                        islidar=True
		)
		
		mc_spectral_is = option(  
			name='mc_spectral_is', 
			group='mc', 
			helpstr='Turns on spectral importance sampling.', 
			documentation=documentation['mc_spectral_is'],     
			gui_inputs = (FloatInput(name='wavelength',optional=True,  valid_range=[0, 1000000.0]),),  
			tokens=[addSetting(name='Input.rte.mc.spectral_is', setting=True, default=False),
				addToken(name='Input.rte.mc.spectral_is_wvl[0]', datatype=float, valid_range=[0, 1e6],optional=True) ], 
			parents=['uvspec'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
                        showInGui=False
                )

		mc_aerosol_is = option(  
			name='mc_aerosol_is',
			group='mc',
			helpstr='Enable aerosol concentration importance sampling.', 
			documentation=documentation['mc_aerosol_is'], 
			tokens=[ addToken(name='Input.rte.mc.filename[FN_MC_AERIS]', datatype=file),
				 addSetting(name='Input.rte.mc.concentration_is', setting=True, default=False) ],
			parents=['uvspec'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui = False,
                        islidar=True
		)

		mc_boxairmass = option(  
			name='mc_boxairmass',
			group='mc',
			helpstr='Calculate box-air-mass factors', 
			documentation=documentation['mc_boxairmass'], 
			tokens=[addLogical(name='id', logicals=['1D','3D'],
                                           setting='DIM_'),
                                addSetting(name='Input.rte.mc.boxairmass[id]', setting=1)],
			parents=['mc_backward'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui = False,
                        islidar = False
			)
		
		mc_albedo_spectral = option( # TODO: Documentation missing!
			name='mc_albedo_spectral', 
			group='mc', 
			helpstr='',
			documentation=documentation['mc_albedo_spectral'], 
			tokens=addSetting(name='Input.rte.mc.spectral', setting=1, default=0), 
			parents=['uvspec'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui = False,
                        islidar = False

		)

		mc_azimuth_old = option(
			name='mc_azimuth_old',
			group='mc',
			helpstr='Use old MYSTIC azimuth convention.',
			documentation=documentation['mc_azimuth_old'], 
			tokens=addSetting(name='Input.rte.mc.azimuth', setting='MCAZIMUTH_OLD', default ='MCAZIMUTH_NEW'),	
			parents=['uvspec'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui = False

		)

		mc_backward_heat = option(
			name='mc_backward_heat',
			group='mc',
			helpstr='Define method for thermal backward heating rate calculation.',
			documentation=documentation['mc_backward_heat'],
			tokens=addLogical(name='Input.rte.mc.backward.thermal_heating_method', logicals=['HYBRID','EMABS','EMABSOPT','DENET'], setting='MCBACKWARD_HEAT_',default = 'MCBACKWARD_HEAT_HYBRID'),
			parents=['uvspec'],
			enable_values=("mystic","montecarlo"),
			threedmystic =True,
			showInGui = False
		)

		mc_backward_sunshape_file = option(
			name='mc_backward_sunshape_file', 
			group='mc', 
			helpstr='Path to a 2-coloumned text file containing the extraterrestrial sun shape.', 
			documentation=documentation['mc_backward_sunshape_file'],
			tokens=addToken(name='Input.rte.mc.filename[FN_MC_SUNSHAPE_FILE]', datatype=file),
			parents=['mc_backward'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui = False,
                        islidar = True

		)

		mc_backward_writeback = option(
			name='mc_backward_writeback',
			group='mc',
			helpstr='Write extra photon information',
			documentation=documentation['mc_backward_writeback'],
			tokens=addSetting(name='Input.rte.mc.backward.writeback', setting=1),
			parents=['mc_backward'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui = False

		)

		mc_coherent_backscatter = option(
			name='mc_coherent_backscatter',
			group='mc', 
			helpstr='Switches on coherent backscattering for lidar and radar.',
			documentation=documentation['mc_coherent_backscatter'], 
			tokens=[addSetting(name='Input.rte.mc.coherent_backscatter',setting=1,default=0),
                                addToken(name='Input.rte.mc.coherent_backscatter_lmode',datatype=int, optional=True,default=1),
                                addToken(name='Input.rte.mc.coherent_backscatter_off',datatype=int, optional=True,default=0) ],
			parents=[], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			islidar = False,
			showInGui=False,
                        developer=True,
		)

		mc_delta_scaling = option(
			name='mc_delta_scaling',
			group='mc', 
			helpstr='Truncate phase function in MYSTIC.',
			documentation=documentation['mc_delta_scaling'],
			tokens=[ addSetting(name='Input.rte.mc.delta_scaling_mucut', setting=0.99),	
				addSetting(name='Input.rte.mc.delta_scaling_start', setting=0),
				addToken(name='Input.rte.mc.delta_scaling_mucut', datatype=float, optional=True),	
				addToken(name='Input.rte.mc.delta_scaling_start', datatype=int, optional =True) ], 
			parents=['uvspec'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			threedmystic =True,
			showInGui =False
		)

		mc_maxscatters = option( 
			name='mc_maxscatters',
			group='mc',
			helpstr='Photons are destroyed after n scatters. For testing only.',
			documentation=documentation['mc_maxscatters'], 
			tokens=addToken(name='Input.rte.mc.maxscatters', datatype=int, default = 'NOT_DEFINED_INTEGER', valid_range=[0,1e6]), # 1e6 is float, not integer!??  
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui =False

		)

		mc_minscatters = option( 
			name='mc_minscatters',
			group='mc',
			helpstr='Local estimates are not performed before n scatters. For testing only',
			documentation=documentation['mc_minscatters'],
			tokens=addToken(name='Input.rte.mc.minscatters', datatype=int, default=0, valid_range=[0,1e6]),
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True,
			showInGui =False

		)

		mc_minphotons = option(
			name='mc_minphotons',
			group='mc',
			helpstr='Minimum value of photons to be used per simulation.',	
			documentation=documentation['mc_minphotons'],
			gui_inputs=(IntegerInput(name='Input.rte.mc.minphotons', default=0, valid_range=[0, 1000000]),),
			tokens=addToken(name='Input.rte.mc.minphotons', datatype=int, default=0, valid_range=[0,1e6]),
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic = True
		)

		mc_photons = option(
			name='mc_photons', 
			group='mc', 
			helpstr='Total number of photons to be traced by the Monte Carlo solver.', 
			documentation=documentation['mc_photons'], 
			gui_inputs=(IntegerInput(name='Input.rte.mc.photons', default=10000, valid_range=[0, 10000000000]),),
			tokens=addToken(name='Input.rte.mc.photons', datatype=long, valid_range=[0,1e10]),
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True
		)

		mc_photons_file = option(  
			name='mc_photons_file',
			group='mc', 
			helpstr='Define a MYSTIC spectral photon file.',
			documentation=documentation['mc_photons_file'],
			gui_inputs=(FileInput(name='filename'),),
			tokens=addToken( name='Input.rte.mc.filename[FN_MC_PHOTONS]', datatype=file ),	
			parents=['uvspec'], 
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True
		)

		mc_polarisation = option(
			name='mc_polarisation', 
			group='mc', 
			helpstr='Turn on polarisation for Monte Carlo solver.', 
			documentation=documentation['mc_polarisation'], 
			tokens = [ addSetting(name='Input.rte.mc.polarisation', setting=1, default=0),
				 addSetting(name='Input.rte.mc.reflectalways', setting=1,default=0),
                                 addToken(name='Input.rte.mc.polarisation_state', datatype=int,valid_range=[-3,4],default=0,optional=True)],
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True
		)
		
		mc_rad_alpha =  option(
			name='mc_rad_alpha', 
			group='mc', 
			helpstr='Define opening angle for radiance simulations.', 
			documentation=documentation['mc_rad_alpha'], 
			gui_inputs=(IntegerInput(name='Input.rte.mc.alpha',
						 default=5, valid_range=[0, 90]),),
			tokens=addToken(name='Input.rte.mc.alpha', datatype=float, default=5,
					valid_range=[0,90]),
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			mystic =True
		)
		
		mc_radial_pathlength = option(
			name='mc_radial_pathlength', 
			group='mc', 
			helpstr='I3RC case 7',
			documentation=documentation['mc_radial_pathlength'],
			tokens=[ addToken(name='Input.rte.mc.Nr', datatype=int, default=0,valid_range=[0,1e6]),
				 addToken(name='Input.rte.mc.Nt', datatype=int, default=0,valid_range=[0,1e6]) ],
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic","montecarlo"),
			threedmystic =True,
			showInGui = False
		)

		mc_radial_pathlength_dt = option(
			name='mc_radial_pathlength_dt',
			group='mc', 
			helpstr='Time increment for mc_radial_pathlength',
			documentation=documentation['mc_radial_pathlength_dt'],
			tokens=addToken(name='Input.rte.mc.dt', datatype=float, default=1e-7, valid_range=[0,1e6]),
			parents=['mc_radial_pathlength'],
			speaker='rte_solver',
			enable_values=("mystic",),
			threedmystic =True,
			showInGui = False
		)

		mc_readrandomstatus = option(  # TODO: Documentation wrong! filename is not implemented!
			name='mc_readrandomstatus', 
			group='mc', 
			helpstr='Read from file the random status for the random number generator.',  #Documentation wrong!
			documentation=documentation['mc_readrandomstatus'], 
			tokens=[addSetting(name='Input.rte.mc.readrandomstatus',setting=1),
				addSetting(name='Input.rte.mc.readrandomseed',setting=1) ],
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic",),
			threedmystic =True,
			showInGui=False,
		)

		mc_randomseed = option(
			name='mc_randomseed', 
			group='mc', 
			helpstr='Set random seed for MYSTIC.',
			documentation=documentation['mc_randomseed'], 
			gui_inputs=(IntegerInput(name='Input.rte.mc.randomseed', default=0, valid_range=[0, 1000000000000000]),),
			tokens=addToken(name='Input.rte.mc.randomseed', datatype=int, valid_range=[0,1e15]),
			parents=['uvspec'],
			speaker='rte_solver',
			enable_values=("mystic",),
		)

		mc_refraction = option(
			name='mc_refraction',
			group='mc',
			helpstr='Enable refraction.', 
			documentation=documentation['mc_refraction'],
			tokens=addSetting(name='Input.rte.mc.refraction', setting=1, default=0), 
			parents=['mc_spherical'], 
			speaker='rte_solver',
			enable_values=("mystic",),
			mystic =True,
			showInGui=False,
			developer=True,
		)
	
		mc_ris = option(
			name='mc_ris',
			group='mc',
			helpstr='Use with mc_radar to get good statistics, artificially enhances the optical depth of the clouds for microwave wavelengths.',
			documentation=documentation['mc_ris'],
			tokens= [addLogical(name='id', logicals=['factor','optical_depth'], setting='MC_RIS_'),
				addToken(name='Input.rte.mc.ris[id]', datatype=Double ) ], 
			parents=[],
			speaker='rte_solver',
			enable_values=("mystic",),
			threedmystic =True, 
			islidar = True,
			showInGui = False
		)

		self.options = [mc_polarisation, mc_refraction, mc_coherent_backscatter,
				mc_backward, 
				mc_escape, mc_core, mc_core_threshold, mc_core_scale, mc_core_savefile,mc_vroom,
				mc_surface_reflectalways,
				mc_delta_scaling, mc_truncate,
				mc_spectral_is, 
				mc_aerosol_is,
				mc_boxairmass,
				mc_ris,
			        mc_photons, mc_minphotons, mc_photons_file, 
				mc_visualize, 
				mc_maxscatters, mc_minscatters,
				mc_randomseed, 
				mc_readrandomstatus,
                         	mc_backward_sunshape_file, 
				mc_sample_cldprp, 
				mc_albedo_spectral, 
				mc_azimuth_old, 
				mc_backward_heat,
				mc_backward_writeback, 
				mc_rad_alpha,
				mc_radial_pathlength, 
				mc_radial_pathlength_dt,
				mc_DoLE
				]

	def __iter__(self):
		return iter(self.options)


def get_mc_documentation():
	return {
		'mc_escape'  : r'''
	Calculate MYSTIC radiances via escape probabilities; slows down the tracing 
	but usually speeds up the computation considerably since it reduces
	noise. Switched on per default since it should basically be used always 
	when calculating radiances. Only meaningful with \code{rte_solver montecarlo}. The syntax is
	\fcode{
	  mc_escape on/off
	} 
  		''',
		'mc_rad_alpha'  : r'''
	Define opening angle for radiance calculations without local
	estimate. Ths oprion is useful for all-sky simulations.	
                ''', 
		'mc_vroom' : r'''
	Variance Reduction Optimal Options Method (VROOM). Options are "on"
	and "off". Needs to be specified if you are calculating radiances and
	spiky phase functions are present in your atmosphere. If you are using
	VROOM, please cite: \cite{buras2011a}.
       		''',
		'mc_backward'  : r'''
	Backward tracing of photons. \code{mc_backward} takes either zero, two or four coordinates:
	\fcode{
	  mc_backward [ix_start iy_start] [ix_end iy_end]
	}
	where \code{ix_start}, \code{iy_start} is the index of the sample pixel to be calculated
	or the pixel area from \code{ix_start} to \code{ix_end} and \code{iy_start} to \code{iy_end}.
	All x-indices must be in the range of 0 ... (\code{Nx}-1) and y-indices the range of 0 ... (\code{Ny}-1).
	If no coordinates are specified, all sample pixels will be calculated.
	\code{mc_backward} computes radiances and downward diffuse irradiances. If 
	a different quantity is required, please use \code{mc_backward_output}.
       		''',
		'mc_sample_cldprp'  : r'''
	Switch on sampling of cloud properties (reff/tau/dlwc) by photons. 
	This option needs \code{mc_backward}. The optical properties contributing to 
	the result for each pixel are integrated using the photon weight 
	$p_{weight,n}$ in the following way:
	\begin{equation*}
	{\langle r_e \rangle}_\textrm{wc}^\textrm{tau}\,=\,\frac{
	\sum\limits_{n=0}^{photons}\,p_{weight,n}\,r_{\textrm{wc},n}}
	{\sum\limits_{n=0}^{photons}\,p_{weight,n}}\qquad
	r_{\textrm{wc}} \,=\, 
	\frac{\int_0^\tau\ \tau_{\textrm{w}}r_{\textrm{ew}}\,\mathrm{d}\tau}
	{\int_0^\tau \tau_{\textrm{w}}\,\mathrm{d}\tau}
	\end{equation*}
	For each internal wavelength, one [basename]\_[wvl].cldprp file with 
	the following format is created where values marked with top are taken at 
	first cloud contact:
	\fcode{
	ix iy 
	${\langle r_e \rangle}^\textrm{tau}_\textrm{wc}$
	${\langle r_e \rangle}^\textrm{tau}_\textrm{ic}$
	${\langle r_e \rangle}^\textrm{hit}_\textrm{wc}$
	${\langle r_e \rangle}^\textrm{hit}_\textrm{ic}$
	${\langle \tau \rangle}_\textrm{wc}$
	${\langle \tau \rangle}_\textrm{mc}$
	$\nabla LWC$ $\nabla IWC$
	}	
	Values denoted with \textit{hit} are taken at the last cloud contact 
	before hitting the sensor, while the the water content gradient is
	taken at the last cloud scattering before hitting the sensor (information
	about cloud surface orientation).
		''',
		'mc_spectral_is': r'''
	Calculate a spectrum with high spectral
	resolution using an importance sampling
	method.  
	\fcode{
	mc_spectral_is [wvl]
	}
	The spectral variation of Rayleigh
	scattering and molecular absorption is fully
	included as well the spectral variation of
	aerosol and cloud scattering and absorption
	coefficients.  The spectral variation of the
	phase matrix is neglected, for all wavelengths
	the phase matrix of the calculation wavelength
	is assumed. 
	
	Use the optional argument \code{wvl} to specify calculation wavelength. 
	If not set, the central wavelength of the selected spectral interval is used.

	This method is very efficient for
	calculations with very high spectral
	resolution, e.g.  simulations for trace gas
	retrievals, in particular when polarization is required.
        For more details refer to
	\citet{emde2011}.
	        ''',
		'mc_boxairmass': r'''
	Calculate box-airmass-factors based on the pathlength
	distribution as described by \citet{deutschmann2011}.
	This option is currently only included for 1D
	atmospheres. The ouput is a two-column file (basename+\".amf\")
	including the altitude levels and the box-airmass-factors
	corresponding to the layer above the given altitude level. 
        The altitude levels in the output file correspond to the once
        provided with \code{atmosphere_file} and the boxairmass factors
        are for the layers above the given altitude levels.
	The method works for trace gases which do not absorb a
	significant amount of radiation, i.e. which do not influence
	the pathlength distribution significantly.
	        ''',
		'mc_surface_reflectalways': r'''
	Usually, a photon is either absorbed or reflected at the surface, 
	with a probability defined by the surface albedo. If 
	\code{mc_surface_reflectalways} is specified, each photon is reflected and 
	the albedo is considered by reducing the photon weight. In case of BRDF,
	\code{mc_surface_reflectalways} is switched on automatically because the 
	other method is no longer implemented for non-Lambertian BRDFs, due to 
	implementation and numerical problems. For small albedos, the computational 
	time is increased if \code{mc_surface_reflectalways} is used; however, 
	the accuracy of the upward radiance (reflected by the surface) is increased 
	considerably. In case of clouds, however, computational time might be 
	increased considerably without gaining accuracy. 
       		''',
		'mc_DoLE': r'''
        Variance reduction method, use double local estimate and specular
        reflection instead of Cox and Munk ocean BRDF. Status is very
        experimental. USE ONLY IF YOU REALLY KNOW WHAT YOU ARE DOING!
       		''',
		'mc_truncate': r'''
	Truncate phase function at the specified polar angle mu. 
	USE ONLY IF YOU REALLY KNOW WHAT YOU ARE DOING!
       	 	''',
		'mc_aerosol_is' : r'''
	Enable aerosol concentration importance sampling. The input file
	includes one column containing the desired scaling factors: 
	\fcode{
	mc_aerosol_is file
	}
	USE ONLY IF YOU REALLY KNOW WHAT YOU ARE DOING!
  		''',
		'mc_albedo_spectral' : r'''
	To be done!
		''',
		'mc_azimuth_old' : r'''
	Use old MYSTIC azimuth convention (0 degree = looking from the
	direction of the sun; 180 degree = looking into the direction of the
	sun; that is, exactly opposite to the disort convention). The MYSTIC
	azimuth was changed March 1, 2004 - hence this option was introduced
	for compatibility reasons.
		''',
		'mc_backward_heat' : r'''
        Customize calculation of thermal heating rates with backward
	tracing (selected with \code{mc_backward} and \code{mc_backward_output}).
	\fcode{
	  mc_backward_heat [option]
	}
        where option is one of 
	\begin{description}
	\parameter{emabs} 
	  straightforward backward calculation of emission and absorption where the photons are started randomly in a grid box
	\parameter{emabsopt} 
          optimized version of \code{emabs}: photons are started
	  closer to the grid box edges using a variance reduction
          technique. 
	\parameter{denet} 
          calculate heating rates as the difference of net fluxes at
          the grid box faces
	\parameter{hybrid} 
          optimum combination of the above (default)
	\end{description}
        \code{emabs} and \code{emabsopt} calculate the heating rates as the difference of the emission and absorption of the grid
	box, while with \code{denet} photons are started directly at
	the grid box's faces. \code{denet} has been shown to perform
	better for grid box optical thicknesses larger than 5, while for optical thicknesses smaller than 5 \code{emabsopt} performes better. 
        "hybrid" is the combination of "emabsopt" and "denet". The
	respective method is selected automatically depending on the
	grid box optical thickness.
        The optimized methods (\code{emabsopt}, \code{denet}, and \code{hybrid}) only work if the sample grid and the cloud grid
        are identical. If not, \code{emabs} will be used for the
	calculation. For details see \citet{klinger2013}.
		''',
		'mc_backward_sunshape_file' : r'''
	Path to a 2-coloumned text file containing the extraterrestrial sun shape. 1. column:
	A probability density or if you want to a radiance distribution. Does not need
	to be normalized 2. column: Relative angular distance to center from sun inside sun disc,
	values from 0 to 1 (0: center, 1: limb).  Use \code{mc_sun_angular_size} to manually 
	set the extent of the sun. If filename is set to "default", or if only sun radius is given
	 a spectrally resolved sun shape as in \citet{koepke2001} is used.
		''',
		'mc_backward_writeback' : r'''
	If set, the distribution of photons contributing to the result is written 
	to a file with extension .bac which may be useful for some interpretations
	(it basically tells you where the photons come from whichn contribute to the result). 
		''',
		'mc_coherent_backscatter' : r'''
	Switches on coherent backscattering, use only with \code{mc_polarisation}.
	\fcode{
	  mc_coherent_backscatter [option1] [option2]
	}
        \begin{itemize}
        \item If used without \code{mc_lidar} or \code{mc_radar} it will calculate enhancement values directly 
        from averaged scattering matrices (\citet{mishchenko2006}) and output them 
        into a .mish.cb file. This method is only correct in the exact backscattering
        direction, its accuracy is around $5\%$ compared to \citet{Mishchenko1992}.
        \item If used with \code{mc_lidar} or \code{mc_radar} it will use the Stokes Vector method which explicitely
        simulates backward photon paths, calculate the enhancement using the Stokes vector-method
        (\citet{Muinonen2004}) and enhance the result accordingly.
        To get enhancement values repeat the calculation without coherent backscattering and
        calculate the quotients from the results. This method is not limited to the exact
        backscattering direction, but the accuracy especially in backscattering direction can be
        problematic, so it should be viewed as an
        experimental option. Additional output like the value of $0.75/(kl^*)$ which gives the cone 
        halfwidth in radian (\citet{fiebig2010_phd}) is written into a .lid.cb file.
        \end{itemize}
        Description of options:
        \begin{description}
        \parameter{option1}
          Sets configuration for CB with Lidar. Takes the values 0, 1, 2 which correspond to the
          three cases A, B, C in fig. 3.8 in \citet{pause16}. Optional, default is 0, use 1 or 2 to get 
          a significant CB signal.
        \parameter{option2}
          If set to 1 omits adding the CB signal to the full signal, that means the result is just
          the mean of the diffuse signals of both scattering paths. Optional, default is 0 (off).
        \end{description}
		''',
		'mc_delta_scaling' : r'''
	Truncate phase function in MYSTIC: The phasefunctions are set to zero
	for mu>mucut. The extinction coefficient etc. are scaled
	accordingly. Optional settings are \code{mc_delta_scaling mucut} and
	\code{mc_delta_scaling mucut n_start}, where mucut is 0.99 by
	default. The value n\_start defines after which scattering order
	delta-scaling is applied, before that scattering order, the real phase
	function is applied. Default is 0, i.e. the delta-scaled phase
	function is applied from the beginning.
		''',
		'mc_maxscatters' : r'''
	If set, photons are destroyed after n scatters. Please note that this
	is only for testing, debugging and process understanding! The result
	will certainly be wrong because photons are lost.
		''',
		'mc_minscatters' :  r'''
	If set, local estimates are not performed before n scatters ( e.g. if
	set to 1, single scattering is not taken into account). Please note
	that this is only for testing, debugging and process understanding!
	The result will certainly be wrong.
		''',
		'mc_minphotons' : r'''
	If set, this is the minimum value of photons to be used per
	simulation. The default value is \code{MIN_MCPHOTONS}, set in
	uvspec.h.
	
	\fcode{
	mc_maxscatters value
	}
		''',
		'mc_photons' : r'''
	Total number of photons to be traced by the Monte Carlo solver, MYSTIC.
	\fcode{
	mc_photons value
	}
	Only meaningful with \code{rte_solver montecarlo}.
		''',
		'mc_photons_file' : r'''
	Distribution of photons over wavelength bands; to be used with \code{mol_abs_param}.
	\fcode{
	mc_photons_file file
	}
	For an example see \file{data/correlated_k/kato2/x_solar.dat}. No error checking!
	Do only use if you are absolutely sure what you are doing.
	Only meaningful with \code{rte_solver montecarlo}.
		''',
		'mc_polarisation' : r'''
	Switch on polarisation for \code{rte_solver montecarlo}. By default uses unpolarized
        sunlight (light starting with an initial Stokes Vector (1,0,0,0). Optionally you may provide 
        one of the following numbers to specify one of the following initial Stokes Vectors:
        \begin{itemize}
        \item 0 : (1,0,0,0) (default)
        \item 1 : (1,1,0,0)
        \item 2 : (1,0,1,0)
        \item 3 : (1,0,0,1)
        \item -1 : (1,-1,0,0)
        \item -2 : (1,0,-1,0)
        \item -3 : (1,0,0,-1)
        \item 4 : Each photon's Stokes Vector is randomly determined fulfilling the condition 
        $I^2=Q^2+U^2+V^2$, use this e.g. if you need unpolarized radiation with 
        \code{mc_coherent_backscatter} using \code{mc_lidar} or \code{mc_radar}. Averaging over 
        a large number of these random Stokes Vectors leads again to unpolarized radiation (1,0,0,0).
        \end{itemize}
	Details about the implementation of polarisation are described in \citet{emde2010}.
		''',
		'mc_radial_pathlength' : r'''
	I3RC case 7, laser beam experiment. Radiances are sampled in radial 
	and pathlength elements. Specify the number of radial (Nr) and time (Nt) intervals. 
	The radius increment is calculated from Nr and the domain size. The time interval 
	may  be specified with \code{mc_radial_pathlength_dt}. 
		''',
		'mc_radial_pathlength_dt' : r'''
	Specify time increment for \code{mc_radial_pathlength}. Time is converted to pathlength 
	assuming a speed of light of 3$\cdot$10$^8$~m/s
		''',
                'mc_randomseed' : r'''
       	Provide your own random seed (positive integer) for the random number generator. 
       	\fcode{
       	mc_randomseed value
       	}
       	Usually a random seed is determined from current time plus process id. 
       	This option is useful to re-run a simulation for debugging.
       		''',
		'mc_refraction' : '''
	Enable refraction for \code{rte_solver montecarlo}. Works only 
	in 1D spherical geometry (with option \code{mc_spherical}).
		''',
		'mc_visualize' : r'''
	Switch on OpenGL visualization for MYSTIC. 
	\fcode{
	mc_visualize [hiddenline]
	}
	Use the optional argument \code{hiddenline} to switch on hidden line removal for 
	the MYSTIC online visualization. Good for topography, not so good for clouds because 
	the latter look much more realistic when the hidden layers are plotted in transparent mode.

	Try the following keys in the graphics window:
	\begin{description}
	  \item[a]  clouds darker
	  \item[A]  clouds brighter
	  \item[h]  decrease displayed domain height
	  \item[H]  increase displayed domain height
	  \item[r]  turn right
	  \item[l]  turn left
	  \item[u]  turn up
	  \item[d]  turn down
	  \item[k]  "Karussell" (rotate continuously)
	  \item[i]  zoom in
	  \item[o]  zoom out
	  \item[f]  fullscreen
	  \item[q]  quit
	  \item[w]  wireframe (frame cloudy pixels)
	  \item[s]  store and release photon paths; press once to start displaying every 10000th photon pass; press again to save last photon path; press again to release photon
	  \item[b]  step through photon path; store a photon with 's'; press 'b'; use '+' and '-' to step through the photon path forward and backward
	\end{description}
		''',
		'mc_ris' : r'''
        Use with \code{mc_radar} or \code{mc_lidar} to get good statistics. Artificially enhances
        the scattering optical depth by a constant factor.
        \fcode{
        mc_ris factor value
        }
        Set the ris-factor manually.
        \fcode{
        mc_ris optical_depth value
        }        Automatically determine a ris-factor to reach the provided scattering optical
        depth value along a straight line from the transmitter to the border of the domain.
        In case of Lidar/Radar four additional paths at the rim of the transmitter cone are
        simulated and if needed an average over all paths is calculated.
		''',
		'mc_readrandomstatus' : r'''
	Read from file the random status for the random number generator. 
	\fcode{
	mc_readrandomstatus file
	}
	This option is useful to re-run a simulation for debugging, especially if
	the buggy photon appears only late in a long simulation. This option automatically
	toggles on \code{mc_readrandomseed}.
		''',
	'mc_core' : r'''
	\fcode{
			mc_core [Optionally: Corefile]
	}
	Activate delta scaling in the core of thick clouds.
	If set, in the inner part of all clouds specified by "wc\_file 3D" the scattering phase function is delta-scaled, i.e. it is assumed to be almost isotropic,
	while simultaneously reducing the optical thickness. Depending on the specific setup, this can substantially decrease the number of scatterings
	inside the cloud core and therefore simulation time. Deviations in the outcome are kept small by use of the original phase function in the cloud boundary. However, the option should not be used if the sensor points closely towards the sun. Which gridboxes belong to the cloud core is determined by an algorithm. Its result can be seen by setting "mc\_core\_savefile", the thickness of the cloud boundary can be set via "mc\_core\_threshold". Because "mc\_core" does not refine the cloud grid, the given cloud gridboxes should have a resolution finer than "mc\_core\_threshold". Alternatively, the user can specify the core pixels manually by providing a file similar to a 3D cloud file. In this case, the algorithm is not applied and "mc\_core\_threshold" has no effect.
	Corefile structure: 
	\fcode{
    Nx  Ny  Nz 4 \\
    dx  dy  z(1) z(2) ... z(nlev) \\
    ix  iy  iz  core  (core=1 for voxel belongs to the core, 0 otherwise)\\
    }
	\code{Nx}, \code{Ny} and \code{Nz} are the number or gridboxes in each direction, \code{dx}, \code{dy} and \code{z()} specify the box sizes. \code{4} is the flag to indicate corefiles. See also the description for "wc\_file 3D".
		''',
	'mc_core_threshold' : r'''
	\fcode{
			mc_core_threshold value
	}
	Distance of the hidden core from the cloud boundary in tau units. Default is 3.0.
	''',
	'mc_core_scale' : r'''
	\fcode{
			mc_core_scale value
	}
	Delta scaling factor f inside the hidden core. If mc\_core\_scale is smaller than zero, f=g,  with g the assymmetry parameter is used for scaling
	(which implies isotropic scattering in the core). Default -1.0.
	''',
	'mc_core_savefile' : r'''
	\fcode{
			mc_core_savefile file
	}
	If set, the shape of the detected hidden core is written to the given file. Format is: \fcode{indexX indexY indexZ core distance}
	"core" is 0 for voxel not in core, 1 for in core. "distance" is the minimal optical distance to the observer in the cloud boundary. Value is "nan" inside the core or if the user specified a custom core.
	''',
	}
