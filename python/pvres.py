# save to python script in same folder as ssc.py 
import sys
import ssc
data = ssc.Data()

# pvsamv1 input variables
data.set_string( 'weather_file', '../../examples/AZ Phoenix.tm2') 
data.set_number( 'use_wf_albedo', 1 )
data.set_number( 'albedo', 0.2 )
data.set_number( 'irrad_mode', 0 )
data.set_number( 'sky_model', 2 )
data.set_number( 'ac_derate', 0.99 )
data.set_number( 'modules_per_string', 9 )
data.set_number( 'strings_in_parallel', 2 )
data.set_number( 'inverter_count', 1 )
data.set_number( 'enable_mismatch_vmax_calc', 0 )
data.set_number( 'subarray1_tilt', 20 )
data.set_number( 'subarray1_tilt_eq_lat', 0 )
data.set_number( 'subarray1_azimuth', 180 )
data.set_number( 'subarray1_track_mode', 0 )
data.set_number( 'subarray1_rotlim', 45 )
data.set_number( 'subarray1_enable_backtracking', 0 )
data.set_number( 'subarray1_btwidth', 2 )
data.set_number( 'subarray1_btspacing', 1 )
data.set_array( 'subarray1_soiling', [ 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95 ] )
data.set_number( 'subarray1_derate', 0.955598 )
data.set_number( 'subarray2_enable', 0 )
data.set_number( 'subarray2_nstrings', 0 )
data.set_number( 'subarray2_tilt', 20 )
data.set_number( 'subarray2_tilt_eq_lat', 0 )
data.set_number( 'subarray2_azimuth', 180 )
data.set_number( 'subarray2_track_mode', 0 )
data.set_number( 'subarray2_rotlim', 45 )
data.set_number( 'subarray2_enable_backtracking', 0 )
data.set_number( 'subarray2_btwidth', 2 )
data.set_number( 'subarray2_btspacing', 1 )
data.set_array( 'subarray2_soiling', [ 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95 ] )
data.set_number( 'subarray2_derate', 0.955598 )
data.set_number( 'subarray3_enable', 0 )
data.set_number( 'subarray3_nstrings', 0 )
data.set_number( 'subarray3_tilt', 20 )
data.set_number( 'subarray3_tilt_eq_lat', 0 )
data.set_number( 'subarray3_azimuth', 180 )
data.set_number( 'subarray3_track_mode', 0 )
data.set_number( 'subarray3_rotlim', 45 )
data.set_number( 'subarray3_enable_backtracking', 0 )
data.set_number( 'subarray3_btwidth', 2 )
data.set_number( 'subarray3_btspacing', 1 )
data.set_array( 'subarray3_soiling', [ 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95 ] )
data.set_number( 'subarray3_derate', 0.955598 )
data.set_number( 'subarray4_enable', 0 )
data.set_number( 'subarray4_nstrings', 0 )
data.set_number( 'subarray4_tilt', 20 )
data.set_number( 'subarray4_tilt_eq_lat', 0 )
data.set_number( 'subarray4_azimuth', 180 )
data.set_number( 'subarray4_track_mode', 0 )
data.set_number( 'subarray4_rotlim', 45 )
data.set_number( 'subarray4_enable_backtracking', 0 )
data.set_number( 'subarray4_btwidth', 2 )
data.set_number( 'subarray4_btspacing', 1 )
data.set_array( 'subarray4_soiling', [ 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95, 0.95 ] )
data.set_number( 'subarray4_derate', 0.955598 )
data.set_number( 'module_model', 1 )
data.set_number( 'spe_area', 0.74074 )
data.set_number( 'spe_rad0', 200 )
data.set_number( 'spe_rad1', 400 )
data.set_number( 'spe_rad2', 600 )
data.set_number( 'spe_rad3', 800 )
data.set_number( 'spe_rad4', 1000 )
data.set_number( 'spe_eff0', 13.5 )
data.set_number( 'spe_eff1', 13.5 )
data.set_number( 'spe_eff2', 13.5 )
data.set_number( 'spe_eff3', 13.5 )
data.set_number( 'spe_eff4', 13.5 )
data.set_number( 'spe_reference', 4 )
data.set_number( 'spe_module_structure', 0 )
data.set_number( 'spe_a', -3.56 )
data.set_number( 'spe_b', -0.075 )
data.set_number( 'spe_dT', 3 )
data.set_number( 'spe_temp_coeff', -0.5 )
data.set_number( 'spe_fd', 1 )
data.set_number( 'cec_area', 1.244 )
data.set_number( 'cec_a_ref', 1.9816 )
data.set_number( 'cec_adjust', 20.8 )
data.set_number( 'cec_alpha_sc', 0.002651 )
data.set_number( 'cec_beta_oc', -0.14234 )
data.set_number( 'cec_gamma_r', -0.407 )
data.set_number( 'cec_i_l_ref', 5.754 )
data.set_number( 'cec_i_mp_ref', 5.25 )
data.set_number( 'cec_i_o_ref', 1.919e-010 )
data.set_number( 'cec_i_sc_ref', 5.75 )
data.set_number( 'cec_n_s', 72 )
data.set_number( 'cec_r_s', 0.105 )
data.set_number( 'cec_r_sh_ref', 160.48 )
data.set_number( 'cec_t_noct', 49.2 )
data.set_number( 'cec_v_mp_ref', 41 )
data.set_number( 'cec_v_oc_ref', 47.7 )
data.set_number( 'cec_temp_corr_mode', 0 )
data.set_number( 'cec_standoff', 6 )
data.set_number( 'cec_height', 0 )
data.set_number( 'cec_mounting_config', 0 )
data.set_number( 'cec_heat_transfer', 0 )
data.set_number( 'cec_mounting_orientation', 0 )
data.set_number( 'cec_gap_spacing', 0.05 )
data.set_number( 'cec_module_width', 1 )
data.set_number( 'cec_module_length', 1.244 )
data.set_number( 'cec_array_rows', 1 )
data.set_number( 'cec_array_cols', 10 )
data.set_number( 'cec_backside_temp', 20 )
data.set_number( '6par_celltech', 1 )
data.set_number( '6par_vmp', 30 )
data.set_number( '6par_imp', 6 )
data.set_number( '6par_voc', 37 )
data.set_number( '6par_isc', 7 )
data.set_number( '6par_bvoc', -0.11 )
data.set_number( '6par_aisc', 0.004 )
data.set_number( '6par_gpmp', -0.41 )
data.set_number( '6par_nser', 60 )
data.set_number( '6par_area', 1.3 )
data.set_number( '6par_tnoct', 46 )
data.set_number( '6par_standoff', 6 )
data.set_number( '6par_mounting', 0 )
data.set_number( 'snl_module_structure', 0 )
data.set_number( 'snl_a', -3.62 )
data.set_number( 'snl_b', -0.075 )
data.set_number( 'snl_dtc', 3 )
data.set_number( 'snl_ref_a', -3.62 )
data.set_number( 'snl_ref_b', -0.075 )
data.set_number( 'snl_ref_dT', 3 )
data.set_number( 'snl_fd', 1 )
data.set_number( 'snl_a0', 0.94045 )
data.set_number( 'snl_a1', 0.052641 )
data.set_number( 'snl_a2', -0.0093897 )
data.set_number( 'snl_a3', 0.00072623 )
data.set_number( 'snl_a4', -1.9938e-005 )
data.set_number( 'snl_aimp', -0.00038 )
data.set_number( 'snl_aisc', 0.00061 )
data.set_number( 'snl_area', 1.244 )
data.set_number( 'snl_b0', 1 )
data.set_number( 'snl_b1', -0.002438 )
data.set_number( 'snl_b2', 0.0003103 )
data.set_number( 'snl_b3', -1.246e-005 )
data.set_number( 'snl_b4', 2.112e-007 )
data.set_number( 'snl_b5', -1.359e-009 )
data.set_number( 'snl_bvmpo', -0.139 )
data.set_number( 'snl_bvoco', -0.136 )
data.set_number( 'snl_c0', 1.0039 )
data.set_number( 'snl_c1', -0.0039 )
data.set_number( 'snl_c2', 0.291066 )
data.set_number( 'snl_c3', -4.73546 )
data.set_number( 'snl_c4', 0.9942 )
data.set_number( 'snl_c5', 0.0058 )
data.set_number( 'snl_c6', 1.0723 )
data.set_number( 'snl_c7', -0.0723 )
data.set_number( 'snl_impo', 5.25 )
data.set_number( 'snl_isco', 5.75 )
data.set_number( 'snl_ixo', 5.65 )
data.set_number( 'snl_ixxo', 3.85 )
data.set_number( 'snl_mbvmp', 0 )
data.set_number( 'snl_mbvoc', 0 )
data.set_number( 'snl_n', 1.221 )
data.set_number( 'snl_series_cells', 72 )
data.set_number( 'snl_vmpo', 40 )
data.set_number( 'snl_voco', 47.7 )
data.set_number( 'inverter_model', 1 )
data.set_number( 'inv_spe_efficiency', 95 )
data.set_number( 'inv_spe_power_ac', 4000 )
data.set_number( 'inv_snl_c0', -6.57929e-006 )
data.set_number( 'inv_snl_c1', 4.72925e-005 )
data.set_number( 'inv_snl_c2', 0.00202195 )
data.set_number( 'inv_snl_c3', 0.000285321 )
data.set_number( 'inv_snl_paco', 4000 )
data.set_number( 'inv_snl_pdco', 4186 )
data.set_number( 'inv_snl_pnt', 0.17 )
data.set_number( 'inv_snl_pso', 19.7391 )
data.set_number( 'inv_snl_vdco', 310.67 )
data.set_number( 'inv_snl_vdcmax', 0 )
data.set_number( 'self_shading_enabled', 0 )
data.set_number( 'self_shading_length', 1.84844 )
data.set_number( 'self_shading_width', 0.673 )
data.set_number( 'self_shading_mod_orient', 1 )
data.set_number( 'self_shading_str_orient', 0 )
data.set_number( 'self_shading_ncellx', 6 )
data.set_number( 'self_shading_ncelly', 12 )
data.set_number( 'self_shading_ndiode', 3 )
data.set_number( 'self_shading_nmodx', 2 )
data.set_number( 'self_shading_nstrx', 1 )
data.set_number( 'self_shading_nmody', 3 )
data.set_number( 'self_shading_nrows', 3 )
data.set_number( 'self_shading_rowspace', 5 )


module = ssc.Module('pvsamv1')

if (module.exec_(data)):
# return the relevant outputs desired
	ac_hourly = data.get_array('hourly_ac_net');
	ac_monthly = data.get_array('monthly_ac_net');
	ac_annual = data.get_number('annual_ac_net');

	for i in range(len(ac_monthly)):
		print 'ac_monthly [' , i, '](kWh) = ' , ac_monthly[i]
	print 'ac_annual (kWh) = ' , ac_annual
	
else:
	idx = 0
	msg = module.log(idx)
	while (msg is not None):
		print "Error [", idx," ]: " , msg
		idx += 1
		msg = module.log(idx)
	sys.exit("pvsamv1 example failed")


del module

# annualoutput input variables
data.set_number( 'analysis_years', 25 )
data.set_array( 'energy_availability', [ 100 ] )
data.set_array( 'energy_degradation', [ 0.5 ] )
data.set_matrix( 'energy_curtailment', 
[ [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ], 
[ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ] ] )
data.set_number( 'system_use_lifetime_output', 0 )
data.set_array( 'energy_net_hourly', ac_hourly )


module = ssc.Module('annualoutput')

if (module.exec_(data)):
# return the relevant outputs desired
	net_hourly = data.get_array('hourly_e_net_delivered');
	net_annual = data.get_array('annual_e_net_delivered');
	
else:
	idx = 0
	msg = module.log(idx)
	while (msg is not None):
		print "Error [", idx," ]: " , msg
		idx += 1
		msg = module.log(idx)
	sys.exit("annualoutput example failed")

del module





# utilityrate input variables
data.set_number( 'analysis_years', 25 )
data.set_array( 'e_with_system', net_hourly )
data.set_array( 'system_availability', [ 100 ] )
data.set_array( 'system_degradation', [ 0.5 ] )
data.set_array( 'load_escalation', [ 2.5 ] )
data.set_array( 'rate_escalation', [ 2.5 ] )
data.set_number( 'ur_sell_eq_buy', 1 )
data.set_number( 'ur_monthly_fixed_charge', 0 )
data.set_number( 'ur_flat_buy_rate', 0.12 )
data.set_number( 'ur_flat_sell_rate', 0 )
data.set_number( 'ur_tou_enable', 0 )
data.set_number( 'ur_tou_p1_buy_rate', 0.12 )
data.set_number( 'ur_tou_p1_sell_rate', 0 )
data.set_number( 'ur_tou_p2_buy_rate', 0.12 )
data.set_number( 'ur_tou_p2_sell_rate', 0 )
data.set_number( 'ur_tou_p3_buy_rate', 0.12 )
data.set_number( 'ur_tou_p3_sell_rate', 0 )
data.set_number( 'ur_tou_p4_buy_rate', 0.12 )
data.set_number( 'ur_tou_p4_sell_rate', 0 )
data.set_number( 'ur_tou_p5_buy_rate', 0.12 )
data.set_number( 'ur_tou_p5_sell_rate', 0 )
data.set_number( 'ur_tou_p6_buy_rate', 0.12 )
data.set_number( 'ur_tou_p6_sell_rate', 0 )
data.set_number( 'ur_tou_p7_buy_rate', 0.12 )
data.set_number( 'ur_tou_p7_sell_rate', 0 )
data.set_number( 'ur_tou_p8_buy_rate', 0.12 )
data.set_number( 'ur_tou_p8_sell_rate', 0 )
data.set_number( 'ur_tou_p9_buy_rate', 0.12 )
data.set_number( 'ur_tou_p9_sell_rate', 0 )
data.set_string( 'ur_tou_sched_weekday', '111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111' )
data.set_string( 'ur_tou_sched_weekend', '111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111' )
data.set_number( 'ur_dc_enable', 0 )
data.set_number( 'ur_dc_fixed_m1', 0 )
data.set_number( 'ur_dc_fixed_m2', 0 )
data.set_number( 'ur_dc_fixed_m3', 0 )
data.set_number( 'ur_dc_fixed_m4', 0 )
data.set_number( 'ur_dc_fixed_m5', 0 )
data.set_number( 'ur_dc_fixed_m6', 0 )
data.set_number( 'ur_dc_fixed_m7', 0 )
data.set_number( 'ur_dc_fixed_m8', 0 )
data.set_number( 'ur_dc_fixed_m9', 0 )
data.set_number( 'ur_dc_fixed_m10', 0 )
data.set_number( 'ur_dc_fixed_m11', 0 )
data.set_number( 'ur_dc_fixed_m12', 0 )
data.set_number( 'ur_dc_p1', 0 )
data.set_number( 'ur_dc_p2', 0 )
data.set_number( 'ur_dc_p3', 0 )
data.set_number( 'ur_dc_p4', 0 )
data.set_number( 'ur_dc_p5', 0 )
data.set_number( 'ur_dc_p6', 0 )
data.set_number( 'ur_dc_p7', 0 )
data.set_number( 'ur_dc_p8', 0 )
data.set_number( 'ur_dc_p9', 0 )
data.set_string( 'ur_dc_sched_weekday', '444444443333333333334444444444443333333333334444444444443333333333334444444444443333333333334444222222221111111111112222222222221111111111112222222222221111111111112222222222221111111111112222222222221111111111112222222222221111111111112222444444443333333333334444444444443333333333334444' )
data.set_string( 'ur_dc_sched_weekend', '444444443333333333334444444444443333333333334444444444443333333333334444444444443333333333334444222222221111111111112222222222221111111111112222222222221111111111112222222222221111111111112222222222221111111111112222222222221111111111112222444444443333333333334444444444443333333333334444' )
data.set_number( 'ur_tr_enable', 0 )
data.set_number( 'ur_tr_sell_mode', 1 )
data.set_number( 'ur_tr_sell_rate', 0 )
data.set_number( 'ur_tr_s1_energy_ub1', 1e+099 )
data.set_number( 'ur_tr_s1_energy_ub2', 1e+099 )
data.set_number( 'ur_tr_s1_energy_ub3', 1e+099 )
data.set_number( 'ur_tr_s1_energy_ub4', 1e+099 )
data.set_number( 'ur_tr_s1_energy_ub5', 1e+099 )
data.set_number( 'ur_tr_s1_energy_ub6', 1e+099 )
data.set_number( 'ur_tr_s1_rate1', 0 )
data.set_number( 'ur_tr_s1_rate2', 0 )
data.set_number( 'ur_tr_s1_rate3', 0 )
data.set_number( 'ur_tr_s1_rate4', 0 )
data.set_number( 'ur_tr_s1_rate5', 0 )
data.set_number( 'ur_tr_s1_rate6', 0 )
data.set_number( 'ur_tr_s2_energy_ub1', 1e+099 )
data.set_number( 'ur_tr_s2_energy_ub2', 1e+099 )
data.set_number( 'ur_tr_s2_energy_ub3', 1e+099 )
data.set_number( 'ur_tr_s2_energy_ub4', 1e+099 )
data.set_number( 'ur_tr_s2_energy_ub5', 1e+099 )
data.set_number( 'ur_tr_s2_energy_ub6', 1e+099 )
data.set_number( 'ur_tr_s2_rate1', 0 )
data.set_number( 'ur_tr_s2_rate2', 0 )
data.set_number( 'ur_tr_s2_rate3', 0 )
data.set_number( 'ur_tr_s2_rate4', 0 )
data.set_number( 'ur_tr_s2_rate5', 0 )
data.set_number( 'ur_tr_s2_rate6', 0 )
data.set_number( 'ur_tr_s3_energy_ub1', 1e+099 )
data.set_number( 'ur_tr_s3_energy_ub2', 1e+099 )
data.set_number( 'ur_tr_s3_energy_ub3', 1e+099 )
data.set_number( 'ur_tr_s3_energy_ub4', 1e+099 )
data.set_number( 'ur_tr_s3_energy_ub5', 1e+099 )
data.set_number( 'ur_tr_s3_energy_ub6', 1e+099 )
data.set_number( 'ur_tr_s3_rate1', 0 )
data.set_number( 'ur_tr_s3_rate2', 0 )
data.set_number( 'ur_tr_s3_rate3', 0 )
data.set_number( 'ur_tr_s3_rate4', 0 )
data.set_number( 'ur_tr_s3_rate5', 0 )
data.set_number( 'ur_tr_s3_rate6', 0 )
data.set_number( 'ur_tr_s4_energy_ub1', 1e+099 )
data.set_number( 'ur_tr_s4_energy_ub2', 1e+099 )
data.set_number( 'ur_tr_s4_energy_ub3', 1e+099 )
data.set_number( 'ur_tr_s4_energy_ub4', 1e+099 )
data.set_number( 'ur_tr_s4_energy_ub5', 1e+099 )
data.set_number( 'ur_tr_s4_energy_ub6', 1e+099 )
data.set_number( 'ur_tr_s4_rate1', 0 )
data.set_number( 'ur_tr_s4_rate2', 0 )
data.set_number( 'ur_tr_s4_rate3', 0 )
data.set_number( 'ur_tr_s4_rate4', 0 )
data.set_number( 'ur_tr_s4_rate5', 0 )
data.set_number( 'ur_tr_s4_rate6', 0 )
data.set_number( 'ur_tr_s5_energy_ub1', 1e+099 )
data.set_number( 'ur_tr_s5_energy_ub2', 1e+099 )
data.set_number( 'ur_tr_s5_energy_ub3', 1e+099 )
data.set_number( 'ur_tr_s5_energy_ub4', 1e+099 )
data.set_number( 'ur_tr_s5_energy_ub5', 1e+099 )
data.set_number( 'ur_tr_s5_energy_ub6', 1e+099 )
data.set_number( 'ur_tr_s5_rate1', 0 )
data.set_number( 'ur_tr_s5_rate2', 0 )
data.set_number( 'ur_tr_s5_rate3', 0 )
data.set_number( 'ur_tr_s5_rate4', 0 )
data.set_number( 'ur_tr_s5_rate5', 0 )
data.set_number( 'ur_tr_s5_rate6', 0 )
data.set_number( 'ur_tr_s6_energy_ub1', 1e+099 )
data.set_number( 'ur_tr_s6_energy_ub2', 1e+099 )
data.set_number( 'ur_tr_s6_energy_ub3', 1e+099 )
data.set_number( 'ur_tr_s6_energy_ub4', 1e+099 )
data.set_number( 'ur_tr_s6_energy_ub5', 1e+099 )
data.set_number( 'ur_tr_s6_energy_ub6', 1e+099 )
data.set_number( 'ur_tr_s6_rate1', 0 )
data.set_number( 'ur_tr_s6_rate2', 0 )
data.set_number( 'ur_tr_s6_rate3', 0 )
data.set_number( 'ur_tr_s6_rate4', 0 )
data.set_number( 'ur_tr_s6_rate5', 0 )
data.set_number( 'ur_tr_s6_rate6', 0 )
data.set_number( 'ur_tr_sched_m1', 0 )
data.set_number( 'ur_tr_sched_m2', 0 )
data.set_number( 'ur_tr_sched_m3', 0 )
data.set_number( 'ur_tr_sched_m4', 0 )
data.set_number( 'ur_tr_sched_m5', 0 )
data.set_number( 'ur_tr_sched_m6', 0 )
data.set_number( 'ur_tr_sched_m7', 0 )
data.set_number( 'ur_tr_sched_m8', 0 )
data.set_number( 'ur_tr_sched_m9', 0 )
data.set_number( 'ur_tr_sched_m10', 0 )
data.set_number( 'ur_tr_sched_m11', 0 )
data.set_number( 'ur_tr_sched_m12', 0 )


module = ssc.Module('utilityrate')

if (module.exec_(data)):
# return the relevant outputs desired
	energy_value = data.get_array("energy_value")
	
else:
	idx = 0
	msg = module.log(idx)
	while (msg is not None):
		print "Error [", idx," ]: " , msg
		idx += 1
		msg = module.log(idx)
	
	sys.exit("utilityrate example failed")

del module




# cashloan input variables
data.set_number( 'analysis_years', 25 )
data.set_number( 'federal_tax_rate', 28 )
data.set_number( 'state_tax_rate', 7 )
data.set_number( 'property_tax_rate', 0 )
data.set_number( 'prop_tax_cost_assessed_percent', 100 )
data.set_number( 'prop_tax_assessed_decline', 0 )
data.set_number( 'sales_tax_rate', 5 )
data.set_number( 'real_discount_rate', 8 )
data.set_number( 'inflation_rate', 2.5 )
data.set_number( 'insurance_rate', 0 )
data.set_number( 'system_capacity', 3.8745 )
data.set_number( 'system_heat_rate', 0 )
data.set_number( 'loan_term', 25 )
data.set_number( 'loan_rate', 7.5 )
data.set_number( 'loan_debt', 100 )
data.set_array( 'om_fixed', [ 0 ] )
data.set_number( 'om_fixed_escal', 0 )
data.set_array( 'om_production', [ 0 ] )
data.set_number( 'om_production_escal', 0 )
data.set_array( 'om_capacity', [ 20 ] )
data.set_number( 'om_capacity_escal', 0 )
data.set_array( 'om_fuel_cost', [ 0 ] )
data.set_number( 'om_fuel_cost_escal', 0 )
data.set_number( 'annual_fuel_usage', 0 )
data.set_number( 'itc_fed_amount', 0 )
data.set_number( 'itc_fed_amount_deprbas_fed', 0 )
data.set_number( 'itc_fed_amount_deprbas_sta', 0 )
data.set_number( 'itc_sta_amount', 0 )
data.set_number( 'itc_sta_amount_deprbas_fed', 0 )
data.set_number( 'itc_sta_amount_deprbas_sta', 0 )
data.set_number( 'itc_fed_percent', 30 )
data.set_number( 'itc_fed_percent_maxvalue', 1e+099 )
data.set_number( 'itc_fed_percent_deprbas_fed', 0 )
data.set_number( 'itc_fed_percent_deprbas_sta', 0 )
data.set_number( 'itc_sta_percent', 0 )
data.set_number( 'itc_sta_percent_maxvalue', 1e+099 )
data.set_number( 'itc_sta_percent_deprbas_fed', 0 )
data.set_number( 'itc_sta_percent_deprbas_sta', 0 )
data.set_array( 'ptc_fed_amount', [ 0 ] )
data.set_number( 'ptc_fed_term', 10 )
data.set_number( 'ptc_fed_escal', 2.5 )
data.set_array( 'ptc_sta_amount', [ 0 ] )
data.set_number( 'ptc_sta_term', 10 )
data.set_number( 'ptc_sta_escal', 2.5 )
data.set_number( 'ibi_fed_amount', 0 )
data.set_number( 'ibi_fed_amount_tax_fed', 1 )
data.set_number( 'ibi_fed_amount_tax_sta', 1 )
data.set_number( 'ibi_fed_amount_deprbas_fed', 0 )
data.set_number( 'ibi_fed_amount_deprbas_sta', 0 )
data.set_number( 'ibi_sta_amount', 0 )
data.set_number( 'ibi_sta_amount_tax_fed', 1 )
data.set_number( 'ibi_sta_amount_tax_sta', 1 )
data.set_number( 'ibi_sta_amount_deprbas_fed', 0 )
data.set_number( 'ibi_sta_amount_deprbas_sta', 0 )
data.set_number( 'ibi_uti_amount', 0 )
data.set_number( 'ibi_uti_amount_tax_fed', 1 )
data.set_number( 'ibi_uti_amount_tax_sta', 1 )
data.set_number( 'ibi_uti_amount_deprbas_fed', 0 )
data.set_number( 'ibi_uti_amount_deprbas_sta', 0 )
data.set_number( 'ibi_oth_amount', 0 )
data.set_number( 'ibi_oth_amount_tax_fed', 1 )
data.set_number( 'ibi_oth_amount_tax_sta', 1 )
data.set_number( 'ibi_oth_amount_deprbas_fed', 0 )
data.set_number( 'ibi_oth_amount_deprbas_sta', 0 )
data.set_number( 'ibi_fed_percent', 0 )
data.set_number( 'ibi_fed_percent_maxvalue', 1e+099 )
data.set_number( 'ibi_fed_percent_tax_fed', 1 )
data.set_number( 'ibi_fed_percent_tax_sta', 1 )
data.set_number( 'ibi_fed_percent_deprbas_fed', 0 )
data.set_number( 'ibi_fed_percent_deprbas_sta', 0 )
data.set_number( 'ibi_sta_percent', 0 )
data.set_number( 'ibi_sta_percent_maxvalue', 1e+099 )
data.set_number( 'ibi_sta_percent_tax_fed', 1 )
data.set_number( 'ibi_sta_percent_tax_sta', 1 )
data.set_number( 'ibi_sta_percent_deprbas_fed', 0 )
data.set_number( 'ibi_sta_percent_deprbas_sta', 0 )
data.set_number( 'ibi_uti_percent', 0 )
data.set_number( 'ibi_uti_percent_maxvalue', 1e+099 )
data.set_number( 'ibi_uti_percent_tax_fed', 1 )
data.set_number( 'ibi_uti_percent_tax_sta', 1 )
data.set_number( 'ibi_uti_percent_deprbas_fed', 0 )
data.set_number( 'ibi_uti_percent_deprbas_sta', 0 )
data.set_number( 'ibi_oth_percent', 0 )
data.set_number( 'ibi_oth_percent_maxvalue', 1e+099 )
data.set_number( 'ibi_oth_percent_tax_fed', 1 )
data.set_number( 'ibi_oth_percent_tax_sta', 1 )
data.set_number( 'ibi_oth_percent_deprbas_fed', 0 )
data.set_number( 'ibi_oth_percent_deprbas_sta', 0 )
data.set_number( 'cbi_fed_amount', 0 )
data.set_number( 'cbi_fed_maxvalue', 1e+099 )
data.set_number( 'cbi_fed_tax_fed', 1 )
data.set_number( 'cbi_fed_tax_sta', 1 )
data.set_number( 'cbi_fed_deprbas_fed', 0 )
data.set_number( 'cbi_fed_deprbas_sta', 0 )
data.set_number( 'cbi_sta_amount', 0 )
data.set_number( 'cbi_sta_maxvalue', 1e+099 )
data.set_number( 'cbi_sta_tax_fed', 1 )
data.set_number( 'cbi_sta_tax_sta', 1 )
data.set_number( 'cbi_sta_deprbas_fed', 0 )
data.set_number( 'cbi_sta_deprbas_sta', 0 )
data.set_number( 'cbi_uti_amount', 0 )
data.set_number( 'cbi_uti_maxvalue', 1e+099 )
data.set_number( 'cbi_uti_tax_fed', 1 )
data.set_number( 'cbi_uti_tax_sta', 1 )
data.set_number( 'cbi_uti_deprbas_fed', 0 )
data.set_number( 'cbi_uti_deprbas_sta', 0 )
data.set_number( 'cbi_oth_amount', 0 )
data.set_number( 'cbi_oth_maxvalue', 1e+099 )
data.set_number( 'cbi_oth_tax_fed', 1 )
data.set_number( 'cbi_oth_tax_sta', 1 )
data.set_number( 'cbi_oth_deprbas_fed', 0 )
data.set_number( 'cbi_oth_deprbas_sta', 0 )
data.set_array( 'pbi_fed_amount', [ 0 ] )
data.set_number( 'pbi_fed_term', 0 )
data.set_number( 'pbi_fed_escal', 0 )
data.set_number( 'pbi_fed_tax_fed', 1 )
data.set_number( 'pbi_fed_tax_sta', 1 )
data.set_array( 'pbi_sta_amount', [ 0 ] )
data.set_number( 'pbi_sta_term', 0 )
data.set_number( 'pbi_sta_escal', 0 )
data.set_number( 'pbi_sta_tax_fed', 1 )
data.set_number( 'pbi_sta_tax_sta', 1 )
data.set_array( 'pbi_uti_amount', [ 0 ] )
data.set_number( 'pbi_uti_term', 0 )
data.set_number( 'pbi_uti_escal', 0 )
data.set_number( 'pbi_uti_tax_fed', 1 )
data.set_number( 'pbi_uti_tax_sta', 1 )
data.set_array( 'pbi_oth_amount', [ 0 ] )
data.set_number( 'pbi_oth_term', 0 )
data.set_number( 'pbi_oth_escal', 0 )
data.set_number( 'pbi_oth_tax_fed', 1 )
data.set_number( 'pbi_oth_tax_sta', 1 )
data.set_number( 'market', 0 )
data.set_number( 'mortgage', 0 )
data.set_number( 'total_installed_cost', 22194.2 )
data.set_number( 'salvage_percentage', 0 )


module = ssc.Module('cashloan')

if (module.exec_(data)):
# return the relevant outputs desired
	lcoe_real = data.get_number('lcoe_real');
	lcoe_nom = data.get_number('lcoe_nom');
	npv = data.get_number('npv');
	print 'LCOE real (cents/kWh) = ' , lcoe_real
	print 'LCOE nominal (cents/kWh) = ' , lcoe_nom
	print 'NPV = $' , npv

	
else:
	idx = 0
	msg = module.log(idx)
	while (msg is not None):
		print "Error [", idx," ]: " , msg
		idx += 1
		msg = module.log(idx)
	sys.exit("cashloan example failed")



