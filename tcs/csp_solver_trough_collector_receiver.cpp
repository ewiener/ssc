#include "csp_solver_trough_collector_receiver.h"

#include "tcstype.h"

using namespace std;

C_csp_trough_collector_receiver::C_csp_trough_collector_receiver()
{ 
	//Commonly used values, conversions, etc...
	m_r2d = 180. / CSP::pi;
	m_d2r = CSP::pi / 180.;
	m_mtoinch = 39.3700787;	//[m] -> [in]
	m_T_htf_prop_min = 275.0;	//[K]
	
	m_step_recirc = std::numeric_limits<double>::quiet_NaN();

	// set initial values for all parameters to prevent possible misuse
	m_nSCA = -1;
	m_nHCEt = -1;
	m_nColt = -1;
	m_nHCEVar = -1;
	m_eta_pump = std::numeric_limits<double>::quiet_NaN();
	m_HDR_rough = std::numeric_limits<double>::quiet_NaN();
	m_theta_stow = std::numeric_limits<double>::quiet_NaN();
	m_theta_dep = std::numeric_limits<double>::quiet_NaN();
	m_Row_Distance = std::numeric_limits<double>::quiet_NaN();
	m_FieldConfig = -1;
	m_T_startup = std::numeric_limits<double>::quiet_NaN();
	m_m_dot_htfmin = std::numeric_limits<double>::quiet_NaN();
	m_m_dot_htfmax = std::numeric_limits<double>::quiet_NaN();
	m_T_loop_in_des = std::numeric_limits<double>::quiet_NaN();
	m_T_loop_out = std::numeric_limits<double>::quiet_NaN();
	m_Fluid = -1;

	m_T_fp = std::numeric_limits<double>::quiet_NaN();
	m_I_bn_des = std::numeric_limits<double>::quiet_NaN();
	m_V_hdr_max = std::numeric_limits<double>::quiet_NaN();
	m_V_hdr_min = std::numeric_limits<double>::quiet_NaN();
	m_Pipe_hl_coef = std::numeric_limits<double>::quiet_NaN();
	m_SCA_drives_elec = std::numeric_limits<double>::quiet_NaN();
	m_fthrok = -1;
	m_fthrctrl = -1;
	m_ColTilt = std::numeric_limits<double>::quiet_NaN();
	m_ColAz = std::numeric_limits<double>::quiet_NaN();

	m_accept_mode = -1;
	m_accept_init = false;
	m_accept_loc = -1;
	m_is_using_input_gen = false;

	m_solar_mult = std::numeric_limits<double>::quiet_NaN();
	m_mc_bal_hot = std::numeric_limits<double>::quiet_NaN();
	m_mc_bal_cold = std::numeric_limits<double>::quiet_NaN();
	m_mc_bal_sca = std::numeric_limits<double>::quiet_NaN();

	m_defocus = std::numeric_limits<double>::quiet_NaN();
	m_latitude = std::numeric_limits<double>::quiet_NaN();
	m_longitude = std::numeric_limits<double>::quiet_NaN();
	m_T_sys_h = std::numeric_limits<double>::quiet_NaN();
	m_T_sys_c = std::numeric_limits<double>::quiet_NaN();
	m_T_sys_h_converged = std::numeric_limits<double>::quiet_NaN();
	m_T_sys_c_converged = std::numeric_limits<double>::quiet_NaN();
	m_EqOpteff = std::numeric_limits<double>::quiet_NaN();
	m_m_dot_htf_tot = std::numeric_limits<double>::quiet_NaN();
	m_Theta_ave = std::numeric_limits<double>::quiet_NaN();
	m_CosTh_ave = std::numeric_limits<double>::quiet_NaN();
	m_IAM_ave = std::numeric_limits<double>::quiet_NaN();
	m_RowShadow_ave = std::numeric_limits<double>::quiet_NaN();
	m_EndLoss_ave = std::numeric_limits<double>::quiet_NaN();
	m_c_htf_ave = std::numeric_limits<double>::quiet_NaN();

	for (int i = 0; i < 5; i++)
		m_T_save[i] = std::numeric_limits<double>::quiet_NaN();

	for (int i = 0; i < 3; i++)
		m_reguess_args[i] = std::numeric_limits<double>::quiet_NaN();

	m_AnnulusGasMat.fill(NULL);
	m_AbsorberPropMat.fill(NULL);

}

void C_csp_trough_collector_receiver::init(const C_csp_collector_receiver::S_csp_cr_init_inputs init_inputs, 
				C_csp_collector_receiver::S_csp_cr_solved_params & solved_params)
{
	
	// double some_calc = m_nSCA + m_nHCEt;
	/*
	--Initialization call--

	Do any setup required here.
	Get the values of the inputs and parameters
	*/

	//Initialize air properties -- used in reeiver calcs
	m_airProps.SetFluid(HTFProperties::Air);

	// Save init_inputs to member data
	m_latitude = init_inputs.m_latitude;	//[deg]
	m_longitude = init_inputs.m_longitude;	//[deg]
	m_shift = init_inputs.m_shift;			//[deg]
	m_latitude *= m_d2r;		//[rad] convert from [deg]
	m_longitude *= m_d2r;		//[rad] convert from [deg]
	m_shift *= m_d2r;			//[rad] convert from [deg]

	// Set trough HTF properties
	if (m_Fluid != HTFProperties::User_defined)
	{
		if (!m_htfProps.SetFluid(m_Fluid))
		{
			throw(C_csp_exception("Field HTF code is not recognized", "Trough Collector Solver"));
		}
	}
	else if (m_Fluid == HTFProperties::User_defined)
	{
		int n_rows = m_field_fl_props.nrows();
		int n_cols = m_field_fl_props.ncols();
		if (n_rows > 2 && n_cols == 7)
		{
			if (!m_htfProps.SetUserDefinedFluid(m_field_fl_props))
			{
				m_error_msg = util::format(m_htfProps.UserFluidErrMessage(), n_rows, n_cols);
				throw(C_csp_exception(m_error_msg, "Trough Collector Solver"));
			}
		}
		else
		{
			m_error_msg = util::format("The user defined field HTF table must contain at least 3 rows and exactly 7 columns. The current table contains %d row(s) and %d column(s)", n_rows, n_cols);
			throw(C_csp_exception(m_error_msg, "Trough Collector Solver"));
		}
	}
	else
	{
		throw(C_csp_exception("Receiver HTF code is not recognized", "Trough Collector Solver"));
	}

	// Adjust parameters
	m_ColTilt = m_ColTilt*m_d2r;	//[rad] Collector tilt angle (0 is horizontal, 90deg is vertical), convert from [deg]
	m_ColAz = m_ColAz*m_d2r;		//[rad] Collector azimuth angle, convert from [deg]

	// Check m_IAM matrix against number of collectors: m_nColt
	m_n_r_iam_matrix = m_IAM_matrix.nrows();
	m_n_c_iam_matrix = m_IAM_matrix.ncols();

	if (m_n_c_iam_matrix < 3)
	{
		throw(C_csp_exception("There must be at least 3 incident angle modifier coefficients", "Trough collector solver"));
	}

	if (m_n_r_iam_matrix < m_nColt)
	{
		m_error_msg = util::format("The number of groups of m_IAM coefficients (%d) is less than the number of collector types in this simulation (%d)", m_n_r_iam_matrix, m_nColt);
		throw(C_csp_exception(m_error_msg, "Trough collector solver"));
	}

	// Check that for each collector, at least 3 coefficients are != 0.0
	for (int i = 0; i < m_nColt; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (m_IAM_matrix(i, j) == 0.0)
			{

				m_error_msg = util::format("For %d collectors and groups of m_IAM coefficients, each group of m_IAM coefficients must begin with at least 3 non-zero values. There are only %d non-zero coefficients for collector %d", m_nColt, j, i + 1);
				throw(C_csp_exception(m_error_msg, "Trough collector solver"));
				//message(TCS_ERROR, "For %d collectors and groups of m_IAM coefficients, each group of m_IAM coefficients must begin with at least 3 non-zero values. There are only %d non-zero coefficients for collector %d", m_nColt, j, i + 1);
				//return -1;
			}
		}
	}
	// ******************************************************************

	//Organize the emittance tables here
	m_epsilon_3.init(4, 4);
	m_epsilon_3.addTable(&m_epsilon_3_11);	//HCE #1
	m_epsilon_3.addTable(&m_epsilon_3_12);
	m_epsilon_3.addTable(&m_epsilon_3_13);
	m_epsilon_3.addTable(&m_epsilon_3_14);
	m_epsilon_3.addTable(&m_epsilon_3_21);	//HCE #2
	m_epsilon_3.addTable(&m_epsilon_3_22);
	m_epsilon_3.addTable(&m_epsilon_3_23);
	m_epsilon_3.addTable(&m_epsilon_3_24);
	m_epsilon_3.addTable(&m_epsilon_3_31);	//HCE #3
	m_epsilon_3.addTable(&m_epsilon_3_32);
	m_epsilon_3.addTable(&m_epsilon_3_33);
	m_epsilon_3.addTable(&m_epsilon_3_34);
	m_epsilon_3.addTable(&m_epsilon_3_41);	//HCE #4
	m_epsilon_3.addTable(&m_epsilon_3_42);
	m_epsilon_3.addTable(&m_epsilon_3_43);
	m_epsilon_3.addTable(&m_epsilon_3_44);

	//Unit conversions
	m_theta_stow *= m_d2r;
	m_theta_stow = max(m_theta_stow, 1.e-6);
	m_theta_dep *= m_d2r;
	m_theta_dep = max(m_theta_dep, 1.e-6);
	m_T_startup += 273.15;			//[K] convert from C
	m_T_loop_in_des += 273.15;		//[K] convert from C
	m_T_loop_out += 273.15;			//[K] convert from C
	m_T_fp += 273.15;				//[K] convert from C
	m_mc_bal_sca *= 3.6e3;			//[Wht/K-m] -> [J/K-m]


	/*--- Do any initialization calculations here ---- */
	//Allocate space for the loop simulation objects
	m_T_htf_in.resize(m_nSCA);
	m_T_htf_out.resize(m_nSCA);
	m_T_htf_ave.resize(m_nSCA);
	m_q_loss.resize(m_nHCEVar);
	m_q_abs.resize(m_nHCEVar);
	m_DP_tube.resize(m_nSCA);
	m_E_int_loop.resize(m_nSCA);
	m_E_accum.resize(m_nSCA);
	m_E_avail.resize(m_nSCA);
	m_q_loss_SCAtot.resize(m_nSCA);
	m_q_abs_SCAtot.resize(m_nSCA);
	m_q_SCA.resize(m_nSCA);
	m_q_1abs_tot.resize(m_nSCA);
	m_q_1abs.resize(m_nHCEVar);
	m_q_i.resize(m_nColt);
	m_IAM.resize(m_nColt);
	m_ColOptEff.resize(m_nColt, m_nSCA);
	m_EndGain.resize(m_nColt, m_nSCA);
	m_EndLoss.resize(m_nColt, m_nSCA);
	m_RowShadow.resize(m_nColt);
	//Allocate space for transient variables
	m_T_htf_in_last.resize(m_nSCA);
	m_T_htf_out_last.resize(m_nSCA);
	m_T_htf_ave_last.resize(m_nSCA);
	m_T_htf_in_converged.resize(m_nSCA);
	m_T_htf_out_converged.resize(m_nSCA);
	m_T_htf_ave_converged.resize(m_nSCA);

	//Set up annulus gas and absorber property matrices
	m_AnnulusGasMat.resize(m_nHCEt, m_nHCEVar);
	m_AbsorberPropMat.resize(m_nHCEt, m_nHCEVar);
	for (int i = 0; i<m_nHCEt; i++){
		for (int j = 0; j<m_nHCEVar; j++){
			//Set up a matrix of annulus gas properties
			m_AnnulusGasMat.at(i, j) = new HTFProperties();
			m_AnnulusGasMat.at(i, j)->SetFluid((int)m_AnnulusGas.at(i, j));
			//Set up a matrix of absorber prop materials
			m_AbsorberPropMat(i, j) = new AbsorberProps();
			m_AbsorberPropMat(i, j)->setMaterial((int)m_AbsorberMaterial.at(i, j));
		}
	}

	//Initialize values
	m_defocus_old = 0.;

	m_ncall = -1;

	// for test start
	init_fieldgeom();
	// for test end

	// Set solved parameters
	solved_params.m_T_htf_cold_des = m_T_loop_in_des;	//[K]
	solved_params.m_q_dot_rec_on_min = 0.0;				//[W/m^2] Not sure that this is super important for the trough
	solved_params.m_q_dot_rec_des = m_q_design/1.E6;	//[MWt]
	solved_params.m_A_aper_total = m_Ap_tot;			//[m^2]

	return;
}


bool C_csp_trough_collector_receiver::init_fieldgeom()
{

	/*
	Call this method once when call() is first invoked. The calculations require location information that
	is provided by the weatherreader class and not set until after init() and before the first call().
	*/
	//Calculate the actual length of the SCA's based on the aperture length and the collectors per SCA
	m_L_actSCA.resize(m_nColt);
	for (int i = 0; i<m_nColt; i++)
	{
		m_L_actSCA[i] = m_L_aperture[i] * m_ColperSCA[i];
	}

	//Calculate the total field aperture area
	m_Ap_tot = 0.;
	for (int i = 0; i<m_nSCA; i++)
	{
		int ct = (int)m_SCAInfoArray.at(i, 1);
		m_Ap_tot += m_A_aperture[ct - 1];
	}

	//Calculate the cross-sectional flow area of the receiver piping
	m_D_h.resize(m_nHCEt, m_nHCEVar);
	m_A_cs.resize(m_nHCEt, m_nHCEVar);
	for (int i = 0; i < m_nHCEt; i++)
	{
		for (int j = 0; j < m_nHCEVar; j++)
		{
			if (m_Flow_type.at(i, j) == 2)
			{
				m_D_h.at(i, j) = m_D_2.at(i, j) - m_D_p.at(i, j);
			}
			else
			{
				m_D_h.at(i, j) = m_D_2.at(i, j);
				m_D_p.at(i, j) = 0.;
			}
			m_A_cs.at(i, j) = CSP::pi* (m_D_2.at(i, j)*m_D_2.at(i, j) - m_D_p.at(i, j)*m_D_p.at(i, j)) / 4.;  //[m2] The cross-sectional flow area
		}
	}

	m_L_tot = 0.0;
	for (int i = 0; i < m_nSCA; i++)
	{
		int ct = (int)m_SCAInfoArray.at(i, 1);
		m_L_tot += m_L_actSCA[ct - 1];
	}

	if (m_accept_loc == 1)
	{
		m_Ap_tot *= float(m_nLoops);

		//Calculate header diameters here based on min/max velocities
		//output file with calculated header diameter "header_diam.out"
		m_nfsec = m_FieldConfig;  //MJW 1.12.11 allow the user to specify the number of field sections
		//Check to make sure the number of field sections is an even number
		if (m_nfsec % 2 != 0)
		{

			m_error_msg = util::format("Number of field subsections must equal an even number");
			throw(C_csp_exception(m_error_msg, "Trough collector solver"));
			//message(TCS_ERROR, "Number of field subsections must equal an even number");
			//return false;
		}

		/*
		The number of header sections per field section is equal to the total number of loops divided
		by the number of distinct headers. Since two loops are connected to the same header section,
		the total number of header sections is then divided by 2.
		*/
		m_nhdrsec = (int)ceil(float(m_nLoops) / float(m_nfsec * 2));

		//Allocate space for the m_D_hdr array
		m_D_hdr.resize(m_nhdrsec);

		//We need to determine design information about the field for purposes of header sizing ONLY
		m_c_htf_ave = m_htfProps.Cp((m_T_loop_out + m_T_loop_in_des) / 2.0)*1000.;    //[J/kg-K] Specific heat

		//Need to loop through to calculate the weighted average optical efficiency at design
		//Start by initializing sensitive variables
		double x1 = 0.0, x2 = 0.0, loss_tot = 0.0;
		m_opteff_des = 0.0;
		m_m_dot_design = 0.0;

		for (int i = 0; i < m_nSCA; i++)
		{
			int CT = (int)m_SCAInfoArray.at(i, 1);    //Collector type    
			//Calculate the CosTheta value at summer solstice noon
			//x1 = sqrt(1. - pow(cos((m_latitude - 23.5/180.*m_pi)-m_ColTilt) - cos(m_ColTilt) * cos((m_latitude - 23.5/180.*m_pi)) * (1. - cos(0. -m_ColAz)), 2)); //costheta
			//Calculate end gain factor
			//x2 = max((m_Ave_Focal_Length[CT-1]*tan(acos(x1))-m_Distance_SCA[CT-1]),0.0);  //end gain
			//calculate end loss
			//el_des =  1. - (m_Ave_Focal_Length[CT-1] * tan(acos(x1)) - (float(m_nSCA) - 1.) /float(m_nSCA)* x2) / m_L_actSCA[CT-1];

			for (int j = 0; j < m_nHCEVar; j++)
			{
				int HT = (int)m_SCAInfoArray.at(i, 0);    //HCE type
				//Calculate optical efficiency approximating use of the first collector only
				m_opteff_des += m_Shadowing.at(HT - 1, j)*m_TrackingError[CT - 1] * m_GeomEffects[CT - 1] * m_Rho_mirror_clean[CT - 1] * m_Dirt_mirror[CT - 1] *
					m_Dirt_HCE.at(HT - 1, j)*m_Error[CT - 1] * (m_L_actSCA[CT - 1] / m_L_tot)*m_HCE_FieldFrac.at(HT - 1, j);
				loss_tot += m_Design_loss.at(HT - 1, j)*m_L_actSCA[CT - 1] * m_HCE_FieldFrac.at(HT - 1, j);
			}
		}
		//the estimated mass flow rate at design
		m_m_dot_design = (m_Ap_tot*m_I_bn_des*m_opteff_des - loss_tot*float(m_nLoops)) / (m_c_htf_ave*(m_T_loop_out - m_T_loop_in_des));  //tn 4.25.11 using m_Ap_tot instead of A_loop. Change location of m_opteff_des
		//mjw 1.16.2011 Design field thermal power 
		m_q_design = m_m_dot_design * m_c_htf_ave * (m_T_loop_out - m_T_loop_in_des); //[Wt]
		//mjw 1.16.2011 Convert the thermal inertia terms here
		m_mc_bal_hot = m_mc_bal_hot * 3.6 * m_q_design;    //[J/K]
		m_mc_bal_cold = m_mc_bal_cold * 3.6 * m_q_design;  //[J/K]

		//need to provide fluid density
		double rho_ave = m_htfProps.dens((m_T_loop_out + m_T_loop_in_des) / 2.0, 0.0); //kg/m3
		//Calculate the header design
		m_nrunsec = (int)floor(float(m_nfsec) / 4.0) + 1;  //The number of unique runner diameters
		m_D_runner.resize(m_nrunsec);
		m_L_runner.resize(m_nrunsec);
		m_D_hdr.resize(m_nhdrsec);

		std::string summary;
		header_design(m_nhdrsec, m_nfsec, m_nrunsec, rho_ave, m_V_hdr_max, m_V_hdr_min, m_m_dot_design, m_D_hdr, m_D_runner, &summary);
		//if(ErrorFound()) return

		/*
		Do one-time calculations for system geometry. Calculate all HTF volume, set runner piping length
		Assume there are two field subsections per span, then if there's an even number of spans in the field,
		we count the first header section as half-length. I.e., if a field looks like this:
		(1)        (2)
		|||||||   |||||||
		-----------------
		||||||| : |||||||
		:
		[P]
		:
		||||||| : |||||||
		-----------------
		|||||||   |||||||
		(3)        (4)
		Then the field has 4 subfields and two spans. The runner pipe (:) is half the distance between the two spans.
		If the number of subfields were 6 (3 spans), the two runner pipe segments would both be equal to the full
		distance between spans.
		*/
		if (m_nfsec / 2 % 2 == 1)
		{
			x1 = 2.;     //the first runners are normal
		}
		else
		{
			x1 = 1.;     //the first runners are short
		}
		m_L_runner[0] = 50.;  //Assume 50 [m] of runner piping in and around the power block before it heads out to the field in the main runners
		if (m_nrunsec > 1)
		{
			for (int i = 1; i < m_nrunsec; i++)
			{
				int j = (int)m_SCAInfoArray.at(0, 1) - 1;
				m_L_runner[i] = x1 * (2 * m_Row_Distance + (m_L_SCA[j] + m_Distance_SCA[j])*float(m_nSCA) / 2.);
				x1 = 2.;   //tn 4.25.11 Default to 2 for subsequent runners
			}
		}
		double v_tofrom_sgs = 0.0;
		for (int i = 0; i < m_nrunsec; i++)
		{
			v_tofrom_sgs = v_tofrom_sgs + 2.*m_L_runner[i] * CSP::pi*pow(m_D_runner[i], 2) / 4.;  //This is the volume of the runner in 1 direction.
		}

		//6/14/12, TN: Multiplier for runner heat loss. In main section of code, are only calculating loss for one path.
		//Since there will be two symmetric paths (when m_nrunsec > 1), need to calculate multiplier for heat loss, considering
		//that the first 50 meters of runner is assumed shared.
		double lsum = 0.;
		for (int i = 0; i < m_nrunsec; i++){
			lsum += m_L_runner[i];
		}
		m_N_run_mult = 1.0 + (1.0 - 50.0 / lsum);

		//-------piping from header into and out of the HCE's
		double v_loop_tot = 0.;
		for (int j = 0; j < m_nHCEVar; j++)
		{
			for (int i = 0; i < m_nSCA; i++)
			{
				int CT = (int)m_SCAInfoArray.at(i, 1) - 1;   //Collector type    
				int HT = (int)m_SCAInfoArray.at(i, 0) - 1;    //HCE type
				//v_loop_bal = v_loop_bal + m_Distance_SCA(CT)*m_A_cs(HT,j)*m_HCE_FieldFrac(HT,j)*float(m_nLoops)
				v_loop_tot += (m_L_SCA[CT] + m_Distance_SCA[CT])*m_A_cs(HT, j)*m_HCE_FieldFrac(HT, j)*float(m_nLoops);
			}
		}

		//mjw 1.13.2011 Add on volume for the crossover piping 
		//v_loop_tot = v_loop_tot + m_Row_Distance*m_A_cs(m_SCAInfoArray(m_nSCA/2,1),1)*float(m_nLoops)
		v_loop_tot += m_Row_Distance*m_A_cs((int)m_SCAInfoArray(max(2, m_nSCA) / 2 - 1, 0), 0)*float(m_nLoops);      //TN 6/20: need to solve for m_nSCA = 1


		//-------field header loop
		double v_header = 0.0;
		for (int i = 0; i < m_nhdrsec; i++)
		{
			//Also calculate the hot and cold header volume for later use. 4.25 is for header expansion bends
			v_header += m_D_hdr[i] * m_D_hdr[i] / 4.*CSP::pi*(m_Row_Distance + 4.275)*float(m_nfsec)*2.0;  //tn 4.25.11 The header distance should be multiplied by 2 row spacings
		}
		//Add on inlet/outlet from the header to the loop. Assume header to loop inlet ~= 10 [m] (Kelley/Kearney)
		v_header = v_header + 20.*m_A_cs(0, 0)*float(m_nLoops);

		//Calculate the HTF volume associated with pumps and the SGS
		double v_sgs = Pump_SGS(rho_ave, m_m_dot_design, m_solar_mult);

		//Calculate the hot and cold balance-of-plant volumes
		m_v_hot = v_header + v_tofrom_sgs;
		m_v_cold = m_v_hot;

		//Write the volume totals to the piping diameter file
		summary.append("\n----------------------------------------------\n"
			"Plant HTF volume information:\n"
			"----------------------------------------------\n");
#ifdef _MSC_VER
#define MySnprintf _snprintf
#else
#define MySnprintf snprintf
#endif
#define TSTRLEN 512

		char tstr[TSTRLEN];
		MySnprintf(tstr, TSTRLEN,
			"Cold header pipe volume:   %10.4le m3\n"
			"Hot header pipe volume:    %10.4le m3\n"
			"Volume per loop:           %10.4le m3\n"
			"Total volume in all loops: %10.4le m3\n"
			"Total solar field volume:  %10.4le m3\n"
			"Pump / SGS system volume:  %10.4le m3\n"
			"---------------------------\n"
			"Total plant HTF volume:    %10.4le m3\n",

			m_v_cold, m_v_hot, v_loop_tot / double(m_nLoops), v_loop_tot,
			(m_v_hot*2. + v_loop_tot), v_sgs, (m_v_hot*2. + v_loop_tot + v_sgs));

		summary.append(tstr);

		// Can uncomment this when other code is updated to write/display more than ~700 characters
		mc_csp_messages.add_message(C_csp_messages::NOTICE, summary.c_str());
		//message(TCS_NOTICE, summary.c_str());

		//if ( FILE *file = fopen( "C:/Users/mwagner/Documents/NREL/SAM/Code conversion/header_diam.out", "w") )
		//{
		//	fprintf(file, summary.c_str());
		//	fclose(file);
		//}

		//Include the pump/SGS volume with the header
		m_v_hot = m_v_hot + v_sgs / 2.;
		m_v_cold = m_v_cold + v_sgs / 2.;
	}

	/* ----- Set initial storage values ------ */
	double T_field_ini = 0.5*(m_T_fp + m_T_loop_in_des);	//[K]
	m_T_sys_c_converged = m_T_sys_c_last = T_field_ini;		//[K]
	m_T_sys_h_converged = m_T_sys_h_last = T_field_ini;		//[K]
	//cc--> Note that stored(3) -> Iter is no longer used in the TRNSYS code. It is omitted here.
	for (int i = 0; i < m_nSCA; i++)
	{
		m_T_htf_in_converged[i] = m_T_htf_in_last[i] = T_field_ini;		//[K]
		m_T_htf_out_converged[i] = m_T_htf_out_last[i] = T_field_ini;	//[K]
		m_T_htf_ave_converged[i] = m_T_htf_ave_last[i] = T_field_ini;	//[K]
	}

	if (m_accept_init)
		m_ss_init_complete = false;
	else
		m_ss_init_complete = true;

	return true;
}


int C_csp_trough_collector_receiver::get_operating_state()
{
	return -1;
}


double C_csp_trough_collector_receiver::get_startup_time()
{
	return std::numeric_limits<double>::quiet_NaN();
}
double C_csp_trough_collector_receiver::get_startup_energy(double step /*sec*/)
{
	return std::numeric_limits<double>::quiet_NaN();
}
double C_csp_trough_collector_receiver::get_pumping_parasitic_coef()
{
	return std::numeric_limits<double>::quiet_NaN();
}
double C_csp_trough_collector_receiver::get_min_power_delivery()
{
	return std::numeric_limits<double>::quiet_NaN();
}

void C_csp_trough_collector_receiver::get_design_parameters(C_csp_collector_receiver::S_csp_cr_solved_params & solved_params)
{
	return;
}

int C_csp_trough_collector_receiver::loop_energy_balance(const C_csp_weatherreader::S_outputs &weather, 
									double T_htf_cold_in /*C*/, double m_dot_htf_loop /*kg/s*/,
									const C_csp_solver_sim_info &sim_info)
{
	//First calculate the cold header temperature, which will serve as the loop inlet temperature 
	double rho_hdr_cold = m_htfProps.dens(m_T_sys_c_last, 1.);
	double rho_hdr_hot = m_htfProps.dens(m_T_sys_h_last, 1.);
	double c_hdr_cold_last = m_htfProps.Cp(m_T_sys_c_last)*1000.0;	//mjw 1.6.2011 Adding mc_bal to the cold header inertia

	double T_db = weather.m_tdry+273.15;		//[K] Dry bulb temperature, convert from C
	double T_dp = weather.m_twet+273.15;		//[K] Dew point temperature, convert from C

	// Calculate effective sky temperature
	double hour = fmod(sim_info.m_time/3600.0, 24.0);		//[hr] Hour of day
	double T_sky;	//[K] Effective sky temperature
	if(T_dp > -300.0)
		T_sky = CSP::skytemp(T_db, T_dp, hour);				//[K] Effective sky temperature 
	else
		T_sky = T_db - 20.0;

	if( m_accept_loc ==  E_piping_config::FIELD )
	{
		m_T_sys_c = (m_T_sys_c_last - T_htf_cold_in)*exp(-(m_dot_htf_loop*float(m_nLoops)) / (m_v_cold*rho_hdr_cold + m_mc_bal_cold / c_hdr_cold_last)*sim_info.m_step) + T_htf_cold_in;
		m_c_hdr_cold = m_htfProps.Cp(m_T_sys_c)*1000.0; //mjw 1.6.2011 Adding mc_bal to the cold header inertia
			//Consider heat loss from cold piping
		m_Header_hl_cold = 0.0;
		m_Runner_hl_cold = 0.0;
			//Header
		for( int i = 0; i<m_nhdrsec; i++ )
		{
			m_Header_hl_cold += m_Row_Distance*m_D_hdr[i]*CSP::pi*m_Pipe_hl_coef*(m_T_sys_c - T_db);  //[W]
		}
			//Runner
		for( int i = 0; i<m_nrunsec; i++ )
		{
			m_Runner_hl_cold += m_L_runner[i]*CSP::pi*m_D_runner[i] * m_Pipe_hl_coef*(m_T_sys_c - T_db);  //[W]
		}
		double m_Pipe_hl_cold = m_Header_hl_cold + m_Runner_hl_cold;	//[W]

		m_T_htf_in[0] = m_T_sys_c - m_Pipe_hl_cold / (m_dot_htf_loop*float(m_nLoops)*m_c_hdr_cold);	//[C]
	}
	else		// m_accept_loc == 2, only modeling loop
	{
		m_T_htf_in[0] = T_htf_cold_in;		//[C]
		m_T_sys_c = m_T_htf_in[0];			//[C]
	}

	// Reset vectors that are populated in following for(i..nSCA) loop
	m_q_abs_SCAtot.assign(m_q_abs_SCAtot.size(), 0.0);
	m_q_loss_SCAtot.assign(m_q_loss_SCAtot.size(), 0.0);
	m_q_1abs_tot.assign(m_q_1abs_tot.size(), 0.0);
	m_E_avail.assign(m_E_avail.size(), 0.0);
	m_E_accum.assign(m_E_accum.size(), 0.0);
	m_E_int_loop.assign(m_E_int_loop.size(), 0.0);	
	// And single values...
	m_EqOpteff = 0.0;

	//---------------------
	for( int i = 0; i<m_nSCA; i++ )
	{
		m_q_loss.assign(m_q_loss.size(), 0.0);		//[W/m]
		m_q_abs.assign(m_q_abs.size(), 0.0);		//[W/m]
		m_q_1abs.assign(m_q_1abs.size(), 0.0);		//[W/m]

		int HT = (int)m_SCAInfoArray(i, 0) - 1;    //[-] HCE type
		int CT = (int)m_SCAInfoArray(i, 1) - 1;    //[-] Collector type

		double c_htf_i = 0.0;
		double rho_htf_i = 0.0;

		for( int j = 0; j<m_nHCEVar; j++ )
		{
			//Check to see if the field fraction for this HCE is zero.  if so, don't bother calculating for this variation
			if( m_HCE_FieldFrac(HT, j) == 0.0 ) 
				continue;

			double c_htf_j, rho_htf_j;
			c_htf_j = rho_htf_j = std::numeric_limits<double>::quiet_NaN();

			EvacReceiver(m_T_htf_in[i], m_dot_htf_loop, T_db, T_sky, weather.m_wspd, weather.m_pres*100.0, m_q_SCA[i], HT, j, CT, i, false, m_ncall, sim_info.m_time/3600.0,
				//outputs
				m_q_loss[j], m_q_abs[j], m_q_1abs[j], c_htf_j, rho_htf_j);

			// Check for NaN
			if( m_q_abs[j] != m_q_abs[j] )	
			{
				return E_loop_energy_balance_exit::NaN;
			}

			m_q_abs_SCAtot[i] += m_q_abs[j] * m_L_actSCA[CT] * m_HCE_FieldFrac(HT, j);		//[W] Heat absorbed by HTF, weighted, for SCA
			m_q_loss_SCAtot[i] += m_q_loss[j] * m_L_actSCA[CT] * m_HCE_FieldFrac(HT, j);	//[W] Total heat losses, weighted, for SCA
			m_q_1abs_tot[i] += m_q_1abs[j] * m_HCE_FieldFrac(HT, j);  //[W/m] Thermal losses from the absorber surface
			c_htf_i += c_htf_j*m_HCE_FieldFrac(HT, j);				//[kJ/kg-K]
			rho_htf_i += rho_htf_j*m_HCE_FieldFrac(HT, j);

			//keep track of the total equivalent optical efficiency
			m_EqOpteff += m_ColOptEff(CT, i)*m_Shadowing(HT, j)*m_Dirt_HCE(HT, j)*m_alpha_abs(HT, j)*m_Tau_envelope(HT, j)*(m_L_actSCA[CT] / m_L_tot)*m_HCE_FieldFrac(HT, j);;
		}  //m_nHCEVar loop

		//Calculate the specific heat for the node
		c_htf_i *= 1000.0;	//[J/kg-K]
		//Calculate the average node outlet temperature, including transient effects
		double m_node = rho_htf_i * m_A_cs(HT, 1)*m_L_actSCA[CT];

		//MJW 12.14.2010 The first term should represent the difference between the previous average temperature and the new 
		//average temperature. Thus, the heat addition in the first term should be divided by 2 rather than include the whole magnitude
		//of the heat addition.
		//mjw & tn 5.1.11: There was an error in the assumption about average and outlet temperature      
		m_T_htf_out[i] = m_q_abs_SCAtot[i] / (m_dot_htf_loop*c_htf_i) + m_T_htf_in[i] +
			2.0 * (m_T_htf_ave_last[i] - m_T_htf_in[i] - m_q_abs_SCAtot[i] / (2.0 * m_dot_htf_loop * c_htf_i)) *
			exp(-2. * m_dot_htf_loop * c_htf_i * sim_info.m_step / (m_node * c_htf_i + m_mc_bal_sca * m_L_actSCA[CT]));
		//Recalculate the average temperature for the SCA
		m_T_htf_ave[i] = (m_T_htf_in[i] + m_T_htf_out[i]) / 2.0;


		//Calculate the actual amount of energy absorbed by the field that doesn't go into changing the SCA's average temperature
		//MJW 1.16.2011 Include the thermal inertia term
		if( !m_is_using_input_gen )
		{
			if( m_q_abs_SCAtot[i] > 0.0 )
			{
				//m_E_avail[i] = max(m_q_abs_SCAtot[i]*m_dt*3600. - m_A_cs(HT,1)*m_L_actSCA[CT]*m_rho_htf[i]*m_c_htf[i]*(m_T_htf_ave[i]- m_T_htf_ave0[i]),0.0)
				double x1 = (m_A_cs(HT, 1)*m_L_actSCA[CT] * rho_htf_i * c_htf_i + m_L_actSCA[CT] * m_mc_bal_sca);  //mjw 4.29.11 removed m_c_htf[i] -> it doesn't make sense on the m_mc_bal_sca term
				m_E_accum[i] = x1*(m_T_htf_ave[i] - m_T_htf_ave_last[i]);
				m_E_int_loop[i] = x1*(m_T_htf_ave[i] - 298.15);  //mjw 1.18.2011 energy relative to ambient 
				m_E_avail[i] = max(m_q_abs_SCAtot[i] * sim_info.m_step - m_E_accum[i], 0.0);      //[J/s]*[hr]*[s/hr]: [J]

				//Equation: m_m_dot_avail*m_c_htf[i]*(T_hft_out - m_T_htf_in) = m_E_avail/(m_dt*3600)
				//m_m_dot_avail = (m_E_avail[i]/(m_dt*3600.))/(m_c_htf[i]*(m_T_htf_out[i] - m_T_htf_in[i]))   //[J/s]*[kg-K/J]*[K]: 
			}
		}
		else
		{
			//m_E_avail[i] = max(m_q_abs_SCAtot[i]*m_dt*3600. - m_A_cs(HT,1)*m_L_actSCA[CT]*m_rho_htf[i]*m_c_htf[i]*(m_T_htf_ave[i]- m_T_htf_ave0[i]),0.0)
			double x1 = (m_A_cs(HT, 1)*m_L_actSCA[CT] * rho_htf_i * c_htf_i + m_L_actSCA[CT] * m_mc_bal_sca);  //mjw 4.29.11 removed m_c_htf[i] -> it doesn't make sense on the m_mc_bal_sca term
			m_E_accum[i] = x1*(m_T_htf_ave[i] - m_T_htf_ave_last[i]);
			m_E_int_loop[i] = x1*(m_T_htf_ave[i] - 298.15);  //mjw 1.18.2011 energy relative to ambient 
			//m_E_avail[i] = max(m_q_abs_SCAtot[i] * m_dt - m_E_accum[i], 0.0);      //[J/s]*[hr]*[s/hr]: [J]
			m_E_avail[i] = (m_q_abs_SCAtot[i] * sim_info.m_step - m_E_accum[i]);      //[J/s]*[hr]*[s/hr]: [J]

			//Equation: m_m_dot_avail*m_c_htf[i]*(T_hft_out - m_T_htf_in) = m_E_avail/(m_dt*3600)
			//m_m_dot_avail = (m_E_avail[i]/(m_dt*3600.))/(m_c_htf[i]*(m_T_htf_out[i] - m_T_htf_in[i]))   //[J/s]*[kg-K/J]*[K]: 
		}

		//Set the inlet temperature of the next SCA equal to the outlet temperature of the current SCA
		//minus the heat losses in intermediate piping
		if( i < m_nSCA - 1 )
		{
			//Determine the length between SCA's to use.  if halfway down the loop, use the row distance.
			double L_int;
			if( i == m_nSCA / 2 - 1 )
			{
				L_int = 2. + m_Row_Distance;
			}
			else
			{
				L_int = m_Distance_SCA[CT];
			}

			//Calculate inlet temperature of the next SCA
			m_T_htf_in[i + 1] = m_T_htf_out[i] - m_Pipe_hl_coef*m_D_3(HT, 0)*CSP::pi*L_int*(m_T_htf_out[i] - T_db) / (m_dot_htf_loop*c_htf_i);
			//mjw 1.18.2011 Add the internal energy of the crossover piping
			m_E_int_loop[i] = m_E_int_loop[i] + L_int*(pow(m_D_3(HT, 0), 2) / 4.*CSP::pi + m_mc_bal_sca / c_htf_i)*(m_T_htf_out[i] - 298.150);
		}

	}

	if( m_accept_loc == 1 )
	{
		//Calculation for heat losses from hot header and runner pipe
		m_Runner_hl_hot = 0.0;  
		m_Header_hl_hot = 0.0;  
		for( int i = 0; i < m_nhdrsec; i++ )
		{
			m_Header_hl_hot = m_Header_hl_hot + m_Row_Distance*m_D_hdr[i] * CSP::pi*m_Pipe_hl_coef*(m_T_htf_out[m_nSCA-1] - T_db);	//[W]
		}

		//Add the runner length
		for( int i = 0; i < m_nrunsec; i++ )
		{
			m_Runner_hl_hot = m_Runner_hl_hot + m_L_runner[i] * CSP::pi*m_D_runner[i] * m_Pipe_hl_coef*(m_T_htf_out[m_nSCA-1] - T_db);	//[W]
		}
		
		double m_Pipe_hl_hot = m_Header_hl_hot + m_Runner_hl_hot;	//[W]

		m_c_hdr_hot = m_htfProps.Cp(m_T_htf_out[m_nSCA-1])* 1000.;		//[kJ/kg-K]

		//Adjust the loop outlet temperature to account for thermal losses incurred in the hot header and the runner pipe
		m_T_sys_h = m_T_htf_out[m_nSCA - 1] - m_Pipe_hl_hot / (m_dot_htf_loop*float(m_nLoops)*m_c_hdr_hot);	//[C]

		//Calculate the system temperature of the hot portion of the collector field. 
		//This will serve as the fluid outlet temperature
		m_T_sys_h = (m_T_sys_h_last - m_T_sys_h)*exp(-m_dot_htf_loop*float(m_nLoops) / (m_v_hot*rho_hdr_hot + m_mc_bal_hot / m_c_hdr_hot)*sim_info.m_step) + m_T_sys_h;	//[C]
	}
	else
	{
		m_T_sys_h = m_T_htf_out[m_nSCA-1];	//[C]
	}

	return E_loop_energy_balance_exit::SOLVED;
}

void C_csp_trough_collector_receiver::loop_optical_eta_off()
{
	// If trough is not absorbing any sunlight (night or 100% defocus), then set member data as necessary

	m_ftrack = 0.0;		//[-] Fraction of timestep that solar field is deployed
	m_costh = 0.0;		//[-] Cosine of the incident angle between the sun and trough aperture

	m_q_i.assign(m_q_i.size(),0.0);		//[W/m] DNI * A_aper / L_sca
	m_IAM.assign(m_IAM.size(),0.0);		//[-] Incidence angle modifiers
	m_ColOptEff.fill(0.0);				//[-] tracking * geom * rho * dirt * error * IAM * row shadow * end loss * ftrack
	m_EndGain.fill(0.0);				//[-] Light from different collector hitting receiver
	m_EndLoss.fill(0.0);				//[-] Light missing receiver due to length + end gain
	m_RowShadow.assign(m_RowShadow.size(),0.0);	//[-] Row-to-row shadowing losses
	m_q_SCA.assign(m_q_SCA.size(),0.0);			//[W/m] Total incident irradiation on the receiver (q"*A_aper/L_sca*cos(theta))

	m_Theta_ave = 0.0; 
	m_CosTh_ave = 0.0; 
	m_IAM_ave = 0.0; 
	m_RowShadow_ave = 0.0; 
	m_EndLoss_ave = 0.0;

	return;
}

void C_csp_trough_collector_receiver::loop_optical_eta(const C_csp_weatherreader::S_outputs &weather,
	const C_csp_solver_sim_info &sim_info)
{
	// First, clear all the values calculated below
	loop_optical_eta_off();

	//calculate the m_hour of the day
	double time_hr = sim_info.m_time / 3600.;		//[hr]
	double dt_hr = sim_info.m_step / 3600.;			//[hr]
	double hour = fmod(time_hr, 24.);				//[hr]

	// Convert other input data as necessary
	double SolarAz = weather.m_solazi;		//[deg] Solar azimuth angle
	SolarAz = (SolarAz - 180.0) * m_d2r;	//[rad] convert from [deg]
	
	//Time calculations
	int day_of_year = (int)ceil(time_hr / 24.);  //Day of the year
	// Duffie & Beckman 1.5.3b
	double B = (day_of_year - 1)*360.0 / 365.0*CSP::pi / 180.0;
	// Eqn of time in minutes
	double EOT = 229.2 * (0.000075 + 0.001868 * cos(B) - 0.032077 * sin(B) - 0.014615 * cos(B*2.0) - 0.04089 * sin(B*2.0));
	// Declination in radians (Duffie & Beckman 1.6.1)
	double Dec = 23.45 * sin(360.0*(284.0 + day_of_year) / 365.0*CSP::pi / 180.0) *CSP::pi / 180.0;
	// Solar Noon and time in hours
	double SolarNoon = 12. - ((m_shift)*180.0 / CSP::pi) / 15.0 - EOT / 60.0;

	// Deploy & stow times in hours
	// Calculations modified by MJW 11/13/2009 to correct bug
	double DepHr1 = cos(m_latitude) / tan(m_theta_dep);
	double DepHr2 = -tan(Dec) * sin(m_latitude) / tan(m_theta_dep);
	double DepHr3 = CSP::sign(tan(CSP::pi - m_theta_dep)) * acos((DepHr1*DepHr2 + sqrt(DepHr1*DepHr1 - DepHr2*DepHr2 + 1.0)) / (DepHr1 * DepHr1 + 1.0)) * 180.0 / CSP::pi / 15.0;
	double DepTime = SolarNoon + DepHr3;

	double StwHr1 = cos(m_latitude) / tan(m_theta_stow);
	double StwHr2 = -tan(Dec) * sin(m_latitude) / tan(m_theta_stow);
	double StwHr3 = CSP::sign(tan(CSP::pi - m_theta_stow))*acos((StwHr1*StwHr2 + sqrt(StwHr1*StwHr1 - StwHr2*StwHr2 + 1.0)) / (StwHr1 * StwHr1 + 1.0)) * 180.0 / CSP::pi / 15.0;
	double StwTime = SolarNoon + StwHr3;

	// m_ftrack is the fraction of the time period that the field is tracking. MidTrack is time at midpoint of operation
	double HrA = hour - dt_hr;
	double HrB = hour;

	double  MidTrack;
	// Solar field operates
	if( (HrB > DepTime) && (HrA < StwTime) )
	{
		// solar field deploys during time period
		if( HrA < DepTime )
		{
			m_ftrack = (HrB - DepTime) / dt_hr;
			MidTrack = HrB - m_ftrack * 0.5 *dt_hr;

			// Solar field stows during time period
		}
		else if( HrB > StwTime )
		{
			m_ftrack = (StwTime - HrA) / dt_hr;
			MidTrack = HrA + m_ftrack * 0.5 *dt_hr;
			// solar field operates during entire period
		}
		else
		{
			m_ftrack = 1.0;
			MidTrack = HrA + 0.5 *dt_hr;
		}
		// solar field doesn't operate
	}
	else
	{
		m_ftrack = 0.0;
		MidTrack = HrA + 0.5 *dt_hr;
	}

	double StdTime = MidTrack;
	double SolarTime = StdTime + ((m_shift)*180.0 / CSP::pi) / 15.0 + EOT / 60.0;
	// m_hour angle (arc of sun) in radians
	double omega = (SolarTime - 12.0)*15.0*CSP::pi / 180.0;
	// B. Stine equation for Solar Altitude angle in radians
	double SolarAlt = asin(sin(Dec)*sin(m_latitude) + cos(m_latitude)*cos(Dec)*cos(omega));

	// Calculation of Tracking Angle for Trough. Stine Reference
	double TrackAngle = atan(cos(SolarAlt) * sin(SolarAz - m_ColAz) /
		(sin(SolarAlt - m_ColTilt) + sin(m_ColTilt)*cos(SolarAlt)*(1 - cos(SolarAz - m_ColAz))));
	// Calculation of solar incidence angle for trough.. Stine reference
	if( m_ftrack == 0.0 )
	{
		m_costh = 1.0;
	}
	else
	{
		m_costh = sqrt(1.0 - pow(cos(SolarAlt - m_ColTilt) - cos(m_ColTilt) * cos(SolarAlt) * (1.0 - cos(SolarAz - m_ColAz)), 2));
	}

	// m_theta in radians
	double theta = acos(m_costh);		//[rad] Incidene angle

	for( int i = 0; i<m_nColt; i++ )
	{
		m_q_i[i] = weather.m_beam*m_A_aperture[i] / m_L_actSCA[i]; //[W/m] The incoming solar irradiation per aperture length

		m_IAM[i] = m_IAM_matrix(i, 0);
		for( int j = 1; j < m_n_c_iam_matrix; j++ )
			m_IAM[i] += m_IAM_matrix(i, j)*pow(theta, j) / m_costh;

		m_IAM[i] = fmax(0.0, fmin(m_IAM[i], 1.0));

		//Calculate the Optical efficiency of the collector
		for( int j = 0; j<m_nSCA; j++ )
		{
			m_ColOptEff(i, j) = m_TrackingError[i] * m_GeomEffects[i] * m_Rho_mirror_clean[i] * m_Dirt_mirror[i] * m_Error[i] * m_IAM[i];
		}

		//Account for light reflecting off the collector and missing the receiver, also light from other 
		//collectors hitting a different receiver
		//mjw 4.21.11 - rescope this to be for each specific collector j=1,m_nSCA
		for( int j = 0; j<m_nSCA; j++ )
		{
			if( fabs(SolarAz) <= 90.0 )
			{  //mjw 5.1.11 The sun is in the southern sky (towards equator)
				if( j == 0 || j == m_nSCA - 1 )
				{
					m_EndGain(i, j) = 0.0; //No gain for the first or last collector
				}
				else
				{
					m_EndGain(i, j) = max(m_Ave_Focal_Length[i] * tan(theta) - m_Distance_SCA[i], 0.0) / m_L_actSCA[i];
				}
			}
			else
			{  //mjw 5.1.11 The sun is in the northern sky (away from equator)
				if( (j == floor(float(m_nSCA) / 2.) - 1) || (j == floor(float(m_nSCA) / 2.)) )
				{
					m_EndGain(i, j) = 0.0; //No gain for the loops at the ends of the rows
				}
				else
				{
					m_EndGain(i, j) = max(m_Ave_Focal_Length[i] * tan(theta) - m_Distance_SCA[i], 0.0) / m_L_actSCA[i];
				}
			}
			m_EndLoss(i, j) = 1.0 - m_Ave_Focal_Length[i] * tan(theta) / m_L_actSCA[i] + m_EndGain(i, j);
		}

		// Row to Row m_Shadowing Lossess
		//PH = m_pi / 2.0 - TrackAngle[i]
		m_RowShadow[i] = fabs(cos(TrackAngle)) * m_Row_Distance / m_W_aperture[i];
		if( m_RowShadow[i] < 0.5 || SolarAlt < 0. )
		{
			m_RowShadow[i] = 0.;
		}
		else if( m_RowShadow[i] > 1. )
		{
			m_RowShadow[i] = 1.;
		}

		//Finally correct for these losses on the collector optical efficiency value
		for( int j = 0; j<m_nSCA; j++ )
		{
			m_ColOptEff(i, j) = m_ColOptEff(i, j)*m_RowShadow[i] * m_EndLoss(i, j)*m_ftrack;  //mjw 4.21.11 now a separate value for each SCA
		}
	}

	//Calculate the flux level associated with each SCA
	//but only calculate for the first call of the timestep<----[NO// messes with defocusing control: mjw 11.4.2010]
	m_Theta_ave = 0.0; m_CosTh_ave = 0.0; m_IAM_ave = 0.0; m_RowShadow_ave = 0.0; m_EndLoss_ave = 0.0;
	for( int i = 0; i<m_nSCA; i++ )
	{
		int CT = (int)m_SCAInfoArray(i, 1) - 1;    //Collector type
		m_q_SCA[i] = m_q_i[CT] * m_costh;        //The flux corresponding with the collector type
		//Also use this chance to calculate average optical values
		m_Theta_ave = m_Theta_ave + theta*m_L_actSCA[CT] / m_L_tot;
		m_CosTh_ave = m_CosTh_ave + m_costh*m_L_actSCA[CT] / m_L_tot;
		m_IAM_ave = m_IAM_ave + m_IAM[CT] * m_L_actSCA[CT] / m_L_tot;
		m_RowShadow_ave = m_RowShadow_ave + m_RowShadow[CT] * m_L_actSCA[CT] / m_L_tot;
		m_EndLoss_ave = m_EndLoss_ave + m_EndLoss(CT, i)*m_L_actSCA[CT] / m_L_tot;
	}

}

void C_csp_trough_collector_receiver::off(const C_csp_weatherreader::S_outputs &weather,
	const C_csp_solver_htf_1state &htf_state_in,
	C_csp_collector_receiver::S_csp_cr_out_solver &cr_out_solver,
	C_csp_collector_receiver::S_csp_cr_out_report &cr_out_report,
	const C_csp_solver_sim_info &sim_info)
{
	// Always reset last temps
	reset_last_temps();
	
	// Get optical properties
		// Should reflect that the collector is not tracking and probably (but not necessarily) DNI = 0
	loop_optical_eta_off();

	// Set mass flow rate to minimum allowable
	double m_dot_htf_loop = m_m_dot_htfmin;		//[kg/s]

	// Set duration for recirculation timestep
	if(m_step_recirc != m_step_recirc)
		m_step_recirc = 10.0*60.0;	//[s]

	// Calculate number of steps required given timestep from solver and recirculation step
	int n_steps_recirc = std::ceil(sim_info.m_step / m_step_recirc);	//[-] Number of recirculation steps required
	
	// Define a copy of the sim_info structure
	double time_start = sim_info.m_time - sim_info.m_step;	//[s]
	double step_local = sim_info.m_step / (double)n_steps_recirc;	//[s]
	C_csp_solver_sim_info sim_info_temp = sim_info;
	sim_info_temp.m_step = step_local;		//[s]

	for(int i = 0; i < n_steps_recirc; i++)
	{
		sim_info_temp.m_time = time_start + step_local*(i+1);	//[s]

		// Set inlet temperature to previous timestep outlet temperature
		double T_cold_in = m_T_sys_h_last;			//[K]

		// Call energy balance with updated info
		loop_energy_balance(weather, T_cold_in, m_dot_htf_loop, sim_info_temp);

		update_last_temps();
	}
	
	cr_out_solver.m_W_dot_col_tracking = 0.0;			//[MWe]
	cr_out_solver.m_T_salt_hot = m_T_sys_h - 273.15;	//[C]

	m_operating_mode = C_csp_collector_receiver::OFF;

	return;
}

void C_csp_trough_collector_receiver::startup(const C_csp_weatherreader::S_outputs &weather,
	const C_csp_solver_htf_1state &htf_state_in,
	C_csp_collector_receiver::S_csp_cr_out_solver &cr_out_solver,
	C_csp_collector_receiver::S_csp_cr_out_report &cr_out_report,
	const C_csp_solver_sim_info &sim_info)
{
	// Always reset last temps

	// Get optical performance
	loop_optical_eta(weather, sim_info);


}

void C_csp_trough_collector_receiver::update_last_temps()
{
	// Update "_last" temperatures
	m_T_sys_c_last = m_T_sys_c;		//[K]
	m_T_sys_h_last = m_T_sys_h;		//[K]
	for( int i = 0; i<m_nSCA; i++ )
	{
		m_T_htf_in_last[i] = m_T_htf_in[i];		//[K]
		m_T_htf_out_last[i] = m_T_htf_out[i];	//[K]
		m_T_htf_ave_last[i] = (m_T_htf_in_last[i] + m_T_htf_out_last[i]) / 2.0;	//[K]
	}

	return;
}

void C_csp_trough_collector_receiver::reset_last_temps()
{
	// Reset "_last" temperatures to the "_converged" temps
	m_T_sys_c_last = m_T_sys_c_converged;		//[K]
	m_T_sys_h_last = m_T_sys_h_converged;		//[K]
	for( int i = 0; i<m_nSCA; i++ )
	{
		m_T_htf_in_last[i] = m_T_htf_in_converged[i];		//[K]
		m_T_htf_out_last[i] = m_T_htf_out_converged[i];		//[K]
		m_T_htf_ave_last[i] = (m_T_htf_in_last[i] + m_T_htf_out_last[i]) / 2.0;	//[K]
	}

	return;
}

void C_csp_trough_collector_receiver::call(const C_csp_weatherreader::S_outputs &weather,
	const C_csp_solver_htf_1state &htf_state_in,
	const C_csp_collector_receiver::S_csp_cr_inputs &inputs,
	C_csp_collector_receiver::S_csp_cr_out_solver &cr_out_solver,
	C_csp_collector_receiver::S_csp_cr_out_report &cr_out_report,
	const C_csp_solver_sim_info &sim_info)
{
	// Increase call-per-timestep counter
	// Converge() sets it to -1, so on first call this line will adjust it = 0
	m_ncall++;

	//reset m_defocus counter
	int dfcount = 0;

	double time = sim_info.m_time;		//[hr]
	double dt = sim_info.m_step;		//[s]
	//******************************************************************************************************************************
	//               Time-dependent conditions
	//******************************************************************************************************************************
	double I_b = weather.m_beam;			//[W/m^2] DNI 	
	double T_db = weather.m_tdry;			//[C] Dry bulb air temperature 
	double V_wind = weather.m_wspd;			//[m/s] Ambient windspeed 
	double P_amb = weather.m_pres;			//[mbar] Ambient pressure 
	double T_dp = weather.m_tdew;			//[C] The dewpoint temperature 
	double T_cold_in = htf_state_in.m_temp;	//[C] HTF return temperature 
	double m_dot_in = htf_state_in.m_m_dot;	//[kg/hr] HTF mass flow rate at the inlet  
	m_defocus_new = inputs.m_field_control;	//[none] Defocus control 
	double SolarAz = weather.m_solazi;		//[deg] Solar azimuth angle

	//Unit conversions
	T_db += 273.15;				//[K] convert from C
	T_dp += 273.15;				//[K] convert from C
	P_amb *= 100.0;				//[mbar] -> Pa
	T_cold_in += 273.15;		//[K] convert from C
	m_dot_in *= 1 / 3600.;		//[kg/s] convert from kg/hr
	SolarAz = (SolarAz - 180.0) * m_d2r;	//[rad] convert from [deg]

	//If no change in inputs between calls, then no need to go through calculations.  Sometimes an extra call was run.....
	double E_fp_tot = std::numeric_limits<double>::quiet_NaN();
	if (m_ncall>0)
	{
		if ((!m_no_fp) && (m_T_cold_in_1 > T_cold_in))
		{
			E_fp_tot = m_m_dot_htf_tot * m_c_hdr_cold * (m_T_cold_in_1 - T_cold_in);       //[kg/s]*[J/kg-K]*[K] = [W]  Freeze protection energy required
			goto set_outputs_and_return;
		}
	}
	//**********************************************************************************************************************

	double rho_hdr_cold, rho_hdr_hot;
	double c_hdr_cold_last, m_dot_lower, m_dot_upper;
	bool upflag, lowflag, fp_lowflag, fp_upflag;
	double T_in_lower, y_fp_lower, T_in_upper, y_fp_upper;
	double y_upper, y_lower;
	double upmult, t_tol, err;
	double DP_IOCOP, DP_loop, m_dot_run_in, x3, m_dot_temp;
	double DP_toField, DP_fromField;
	double m_dot_header_in, m_dot_header, DP_hdr_cold, DP_hdr_hot;
	double E_avail_tot, rho_ave, E_int_sum;
	double q_abs_maxOT;
	q_abs_maxOT = 0;

	double SCAs_def = 1.0;
	if (m_ncall == 0)  //mjw 3.5.11 We only need to calculate these values once per timestep..
	{
		// Call optical efficiency method
		if(I_b > 0.0)
			loop_optical_eta(weather, sim_info);
		else
			loop_optical_eta_off();

		if (m_accept_mode)
		{
			if (m_accept_loc == 1)
				m_m_dot_htfX = m_dot_in / float(m_nLoops);
			else
				m_m_dot_htfX = m_dot_in;
		}
		else
		{
			if (I_b > 25.)
				m_m_dot_htfX = max(min(10. / 950.*I_b, m_m_dot_htfmax), m_m_dot_htfmin);   //*m_defocus[kg/s] guess Heat transfer fluid mass flow rate through one loop
			else
				m_m_dot_htfX = m_m_dot_htfmin;
		}

	}
	else  //mjw 3.5.11
	{
		for (int i = 0; i<m_nSCA; i++)
		{
			m_q_SCA[i] = m_q_i[(int)m_SCAInfoArray(i, 1) - 1] * m_costh;
		}
	}  //mjw 3.5.11 ---------- } of the items that only need to be calculated once per time step

	double q_SCA_tot = 0.;
	for (int i = 0; i<m_nSCA; i++)
	{ 
		q_SCA_tot += m_q_SCA[i];	//[W/m]
	} 

	//9-27-12, TWN: This model uses relative m_defocus. Changed controller to provide absolute m_defocus, so now convert to relative here
	m_defocus = m_defocus_new / m_defocus_old;
	m_defocus_old = m_defocus_new;

	//TWN 6/14/11  if defous is < 1 { calculate m_defocus from previous call's m_q_abs and { come back to temperature loop.
	if (m_defocus<1.)
	{
		dfcount = 1;	//cc--> Not sure why this counts.. the m_dfcount variable is always set to 0 when the simulation is called.. Ask Ty.
		goto post_convergence_flag; // goto 11;
	}

	// 12.29.2014, twn: want this before 'overtemp_iter_flag' so temp doesn't reset when flag is called
	m_T_cold_in_1 = T_cold_in; //Inlet temperature may change due to new freeze protection model

overtemp_iter_flag: //10 continue     //Return loop for over-temp conditions

	//First calculate the cold header temperature, which will serve as the loop inlet temperature
	rho_hdr_cold = m_htfProps.dens(m_T_sys_c_last, 1.);
	rho_hdr_hot = m_htfProps.dens(m_T_sys_h_last, 1.);
	c_hdr_cold_last = m_htfProps.Cp(m_T_sys_c_last)*1000.0; //mjw 1.6.2011 Adding mc_bal to the cold header inertia

	dfcount++;   //The m_defocus iteration counter

	// ******* Initial values *******
	//if(m_T_loop_in == m_T_loop_out) m_T_loop_in = m_T_loop_out - 1. //Don't allow equal temperatures
	//m_T_htf_in(1) = m_T_loop_in

	//mode for solving the field 
	//1=constrain temperature
	//2=defocusing array
	//3=constrain mass flow above min
	int SolveMode = 1;

	int qq = 0;                  //Set iteration counter

	m_m_dot_htfX = max(min(m_m_dot_htfmax, m_m_dot_htfX), m_m_dot_htfmin);
	double m_dot_htf = m_m_dot_htfX;

	//Set ends of iteration bracket.  Make bracket greater than bracket defined on system limits so a value less than system limit can be tried earlier in iteration
	m_dot_lower = 0.7*m_m_dot_htfmin;
	m_dot_upper = 1.3*m_m_dot_htfmax;

	upflag = false;        //Set logic to switch from bisection to false position mode
	lowflag = false;       //Set logic to switch from bisection to false position mode
	fp_lowflag = false;   //Set logic to switch from bisection to false position mode
	fp_upflag = false;   //Set logic to switch from bisection to false position mode
	m_no_fp = true;          //Set logic that specify that SCA loop is not solving for freeze protection temperature
	T_in_lower = std::numeric_limits<double>::quiet_NaN();
	y_fp_lower = std::numeric_limits<double>::quiet_NaN();
	T_in_upper = std::numeric_limits<double>::quiet_NaN();
	y_fp_upper = std::numeric_limits<double>::quiet_NaN();
	y_upper = std::numeric_limits<double>::quiet_NaN();
	y_lower = std::numeric_limits<double>::quiet_NaN();
	upmult = std::numeric_limits<double>::quiet_NaN();

	t_tol = 1.5e-3;          //Tolerance for m_T_loop_out

	//Decreasing the tolerance helps get out of repeating m_defocus iterations
	if (m_ncall>8)
	{
		t_tol = 1.5e-4;
	}

	err = t_tol * 10.;       //Set beginning error > tolerance
	// ******************************************************************************************************************************
	//                   Iterative section
	// ******************************************************************************************************************************
	while ((fabs(err) > t_tol) && (qq < 30))
	{

		qq++; //Iteration counter

		m_dot_htf = m_m_dot_htfX;

		if (m_accept_loc == 1)
			m_m_dot_htf_tot = m_dot_htf*float(m_nLoops);
		else
			m_m_dot_htf_tot = m_dot_htf;

		int loop_energy_bal_exit = loop_energy_balance(weather, m_T_cold_in_1, m_dot_htf, sim_info);

		// Check that we found a solution
		if( loop_energy_bal_exit != E_loop_energy_balance_exit::SOLVED )	//cc--> Check for NaN
		{
			m_m_dot_htfX = m_m_dot_htfmax;
			if( dfcount > 20 )
			{
				m_error_msg = "The solution encountered an unresolvable NaN error in the heat loss calculations. Continuing calculations...";
				mc_csp_messages.add_message(C_csp_messages::WARNING, m_error_msg);

				// Set values to something that won't crash TCS solver
				cr_out_solver.m_q_startup = 0.0;			//[MWt-hr]
				cr_out_solver.m_time_required_su = 0.0;		//[s]
				cr_out_solver.m_m_dot_salt_tot = 0.0;		//[kg/hr]
				cr_out_solver.m_q_thermal = 0.0;			//[MWt]
				cr_out_solver.m_T_salt_hot = m_T_loop_in_des - 273.15;	//[C] Reset to loop inlet temperature, I guess?

				cr_out_solver.m_E_fp_total = 0.0;
				cr_out_solver.m_W_dot_col_tracking = 0.0;
				cr_out_solver.m_W_dot_htf_pump = 0.0;

				cr_out_report.m_q_dot_field_inc = 0.0;
				cr_out_report.m_eta_field = 0.0;
				cr_out_report.m_q_dot_rec_inc = 0.0;
				cr_out_report.m_eta_thermal = 0.0;
				cr_out_report.m_q_dot_piping_loss = 0.0;

				return;
			}
			goto overtemp_iter_flag;
		}

		if (!m_ss_init_complete && m_accept_init)
		{
			// Check if solution has reached steady state
			double ss_diff = 0.0;
			ss_diff += fabs(m_T_sys_c - m_T_sys_c_last) + fabs(m_T_sys_h_last - m_T_sys_h);
			for (int i = 0; i < m_nSCA; i++)
			{
				ss_diff += fabs(m_T_htf_in_last[i] - m_T_htf_in[i]) + fabs(m_T_htf_out_last[i] - m_T_htf_out[i]) + fabs(m_T_htf_ave_last[i] - (m_T_htf_in[i] + m_T_htf_out[i]) / 2.0);
			}

			if (ss_diff / 300.0 > 0.001)	// If not in steady state, updated previous temperatures and re-run energy balances
			{			// Should be similar to Converged call

				m_T_sys_c_last = m_T_sys_c;		// Get T_sys from the last timestep
				m_T_sys_h_last = m_T_sys_h;

				for (int i = 0; i<m_nSCA; i++)
				{
					m_T_htf_in_last[i] = m_T_htf_in[i];
					m_T_htf_out_last[i] = m_T_htf_out[i];
					m_T_htf_ave_last[i] = (m_T_htf_in_last[i] + m_T_htf_out_last[i]) / 2.0;
				}

				rho_hdr_cold = m_htfProps.dens(m_T_sys_c_last, 1.);
				rho_hdr_hot = m_htfProps.dens(m_T_sys_h_last, 1.);
				c_hdr_cold_last = m_htfProps.Cp(m_T_sys_c_last)*1000.0; //mjw 1.6.2011 Adding mc_bal to the cold header inertia

				//goto overtemp_iter_flag;
				qq = 0;
				continue;
			}
		}

		if (m_accept_mode)
		{
			goto calc_final_metrics_goto;
		}

	freeze_prot_flag: //7   continue    

		if (SolveMode == 4)
		{
			// assumptions...
			// 1) solver won't enter both modes 3 & 4 in the SAME CALL. so, don't need unique flags or to reset flags

			t_tol = 1.5e-4;

			m_no_fp = false;

			// Energy gain in HTF by boosting the inlet temperature
			double E_fp_field = std::numeric_limits<double>::quiet_NaN();

			if (m_is_using_input_gen)
				E_fp_field = m_m_dot_htf_tot*m_c_hdr_cold*(m_T_cold_in_1 - m_T_sys_h)*dt;       //[kg/s]*[J/kg-K]*[K] = [W]*[s] = [J]  Freeze protection energy required
			else
				E_fp_field = m_m_dot_htf_tot*m_c_hdr_cold*(m_T_cold_in_1 - T_cold_in)*dt;       //[kg/s]*[J/kg-K]*[K] = [W]*[s] = [J]  Freeze protection energy required


			// Energy losses in field
			double E_field_loss_tot = 0.;

			for (int i = 0; i<m_nSCA; i++)
			{
				E_field_loss_tot += m_q_loss_SCAtot[i] * 1.e-6;             //*float(m_nLoops);
			}

			E_field_loss_tot *= 1.e-6*dt;

			double E_field_pipe_hl = m_N_run_mult*m_Runner_hl_hot + float(m_nfsec)*m_Header_hl_hot + m_N_run_mult*m_Runner_hl_cold + float(m_nfsec)*m_Header_hl_cold;

			E_field_pipe_hl *= dt;		//[J]

			E_field_loss_tot += E_field_pipe_hl;

			err = (E_field_loss_tot - E_fp_field) / E_field_loss_tot;

			if (fabs(err) <= t_tol)
				goto freeze_prot_ok;

			if ((fp_lowflag) && (fp_upflag))
			{
				if (err > 0.0)      //Outlet too low, set lower limit of inlet temperature
				{
					T_in_lower = m_T_cold_in_1;
					y_fp_lower = err;
				}
				else                     //Outlet too high, set upper limit of inlet temperature
				{
					T_in_upper = m_T_cold_in_1;
					y_fp_upper = err;
				}
				m_T_cold_in_1 = (y_fp_upper) / (y_fp_upper - y_fp_lower)*(T_in_lower - T_in_upper) + T_in_upper;
			}
			else
			{
				if (m_is_using_input_gen && T_in_lower != T_in_lower && T_in_upper != T_in_upper)
				{
					m_T_cold_in_1 = m_T_fp;
					T_in_lower = m_T_cold_in_1;
				}
				else
				{
					if (err > 0.0)      //Outlet too low, set lower limit of inlet temperature
					{
						T_in_lower = m_T_cold_in_1;
						y_fp_lower = err;
						fp_lowflag = true;
						upmult = 0.50;
					}
					else                    //Outlet too high, set upper limit of inlet temperature
					{
						T_in_upper = m_T_cold_in_1;
						y_fp_upper = err;
						fp_upflag = true;
						upmult = 0.50;
					}

					if ((fp_lowflag) && (fp_upflag))    //if results of bracket are defined, use false position
					{
						m_T_cold_in_1 = (y_fp_upper) / (y_fp_upper - y_fp_lower)*(T_in_lower - T_in_upper) + T_in_upper;
					}
					else                             //if not, recalculate value based on approximate energy balance
					{
						m_T_cold_in_1 = m_T_cold_in_1 + 40.0;   //Will always start low, so fp_lowflag = true so until fp_upflag = true { increase inlet temperature
						qq = 0;
					}
				}
			}
		}
		else if (SolveMode == 3)  //Solve to find (increased) inlet temperature that keeps outlet temperature above freeze protection temp
		{
			t_tol = 1.5e-4;		//12.29.2014, twn: decreases oscillation in freeze protection energy because a smaller deltaT governs it

			if( (m_no_fp) && ((m_T_htf_out[m_nSCA-1] > m_T_fp) || fabs(m_T_fp - m_T_htf_out[m_nSCA-1]) / m_T_fp <= t_tol) )
				goto freeze_prot_ok; //goto 9

			m_no_fp = false;

			err = (m_T_fp - m_T_htf_out[m_nSCA-1]) / m_T_fp;

			if (fabs(err) <= t_tol)
				goto freeze_prot_ok; //goto 9

			if ((fp_lowflag) && (fp_upflag))
			{
				if (err > 0.0)      //Outlet too low, set lower limit of inlet temperature
				{
					T_in_lower = m_T_cold_in_1;
					y_fp_lower = err;
				}
				else                     //Outlet too high, set upper limit of inlet temperature
				{
					T_in_upper = m_T_cold_in_1;
					y_fp_upper = err;
				}
				m_T_cold_in_1 = (y_fp_upper) / (y_fp_upper - y_fp_lower)*(T_in_lower - T_in_upper) + T_in_upper;
			}
			else
			{
				if (m_is_using_input_gen && T_in_lower != T_in_lower && T_in_upper != T_in_upper)
				{
					m_T_cold_in_1 = m_T_fp;
					T_in_lower = m_T_cold_in_1;
				}
				else
				{
					if (err > 0.0)      //Outlet too low, set lower limit of inlet temperature
					{
						T_in_lower = m_T_cold_in_1;
						y_fp_lower = err;
						fp_lowflag = true;
						upmult = 0.50;
					}
					else                    //Outlet too high, set upper limit of inlet temperature
					{
						T_in_upper = m_T_cold_in_1;
						y_fp_upper = err;
						fp_upflag = true;
						upmult = 0.50;
					}

					if ((fp_lowflag) && (fp_upflag))    //if results of bracket are defined, use false position
					{
						m_T_cold_in_1 = (y_fp_upper) / (y_fp_upper - y_fp_lower)*(T_in_lower - T_in_upper) + T_in_upper;
					}
					else                             //if not, recalculate value based on approximate energy balance
					{
						m_T_cold_in_1 = m_T_cold_in_1 + 40.0;   //Will always start low, so fp_lowflag = true so until fp_upflag = true { increase inlet temperature  
						qq = 0;
					}
				}
			}

		}
		else    //Solve to find mass flow that results in target outlet temperature
		{
			err = (m_T_loop_out - m_T_htf_out[m_nSCA-1]) / m_T_loop_out;

			if (m_dot_htf == m_m_dot_htfmin)
			{
				if (m_T_htf_out[m_nSCA-1] < m_T_fp + 10.0)         //freeze protection mode
				{
					if (m_T_htf_out[m_nSCA-1] < m_T_fp)
					{
						SolveMode = 3;
						goto freeze_prot_flag; //goto 7
					}
					else
					{
						SolveMode = 4;
						goto freeze_prot_flag;
					}
				}
				else if (err>0.0)
				{
					goto freeze_prot_ok; //goto 9      //M_dot may already equal m_dot_min while the temperature is still too low, this saves 1 more iteration    
				}
			}

			//if((m_m_dot_htf==m_m_dot_htfmin) && (err>0.0))    goto 9      //M_dot may already equal m_dot_min while the temperature is still too low, this saves 1 more iteration    

			// ****************************************************   
			// ***** Hybrid False Position Iteration Method********  
			// ****************************************************
			//Determine next guess for mass flow rate.  Want to use false position method, but it requires that the *results* (err in this case) at both ends of the bracket are known.  We have
			//defined a bracket but not the results.  Use the initial mass flow rate guess to get the results at one } of a new bracket.  { calculate a new mass flow rate based on the appoximation 
			//We eventually want a result for undefined result in the bracket.  After 2 iterations using approximation without defining bracket, switch to bisection.
			//Once results for both sides are { defined, "lowflag" and "upflag" will be true, and false position method will be applied.
			if ((lowflag) && (upflag))
			{
				if (err > 0.0)
				{
					m_dot_upper = m_dot_htf;
					y_upper = err;
				}
				else
				{
					m_dot_lower = m_dot_htf;
					y_lower = err;
				}
				m_m_dot_htfX = (y_upper) / (y_upper - y_lower)*(m_dot_lower - m_dot_upper) + m_dot_upper;
			}
			else
			{
				if (err > 0.0)      //Prescribed is greater than calculated, so decrease mass flow, so set upper limit
				{
					m_dot_upper = m_dot_htf;
					y_upper = err;
					upflag = true;
					upmult = 0.50;
				}
				else                    //Presribed is less than calculated, so increase mass flow, so set lower limit
				{
					m_dot_lower = m_dot_htf;
					y_lower = err;
					lowflag = true;
					upmult = 0.50;
				}

				if ((lowflag) && (upflag))  //if results of bracket are defined, use false position
				{
					m_m_dot_htfX = (y_upper) / (y_upper - y_lower)*(m_dot_lower - m_dot_upper) + m_dot_upper;
				}
				else                            //if not, recalculate value based on approximate energy balance
				{
					if (qq<3)
					{
						m_c_htf_ave = m_htfProps.Cp((m_T_loop_out + m_T_htf_in[0]) / 2.0)*1000.;    //Specific heat
						double qsum = 0.;
						for (int i = 0; i<m_nSCA; i++){ qsum += m_q_abs_SCAtot[i]; }
						m_m_dot_htfX = qsum / (m_c_htf_ave*(m_T_loop_out - m_T_htf_in[0]));
						m_m_dot_htfX = max(m_m_dot_htfmin, min(m_m_dot_htfX, m_m_dot_htfmax));
					}
					else
					{
						m_m_dot_htfX = 0.5*m_dot_upper + 0.5*m_dot_lower;
					}
				}
			}

			// ***************************************************************************
			// ****** } Hyrbid False Position Iteration Method *************************
			// ***************************************************************************

			if (m_dot_lower > m_m_dot_htfmax)      //Once the minimum possible m_dot to solve energy balance is greater than maximum allowable, exit loop and go to m_defocus
			{
				break;
			}

			if (m_dot_upper < m_m_dot_htfmin)      //Once the maximum possible m_dot to solve energy balance is less than minimum allowable, set to min value and get final T_out
			{
				m_m_dot_htfX = m_m_dot_htfmin;
				SolveMode = 3;
			}

		}

		//    if(ncall > 10){
		//        exit
		//    }

	}

freeze_prot_ok:		//9 continue

	if (m_is_using_input_gen && !m_accept_mode)
	{
		// 12.29.2014 twn:
		// If the trough is model is called as a stand-alone, but is solving for mass flow rate, then need a method to enter recirculation when outlet temperature and useful energy are too low
		// Choosing here to enter recirculation when the outlet temperature is less than the user specified inlet temperature. Then, if there is no freeze-protection adjustment of the inlet temperature
		// the code should iterate until the inlet and outlet temperatures are roughly equal.
		// Could consider implementing a faster, more robust method than successive substitution to solve for the inlet temperature.
		if (m_T_sys_h < T_cold_in)
		{
			if (m_no_fp && fabs(m_T_cold_in_1 - m_T_sys_h) / m_T_sys_h > 0.001)
			{
				m_T_cold_in_1 = m_T_sys_h;
				goto overtemp_iter_flag;
			}
		}
	}

	E_fp_tot = 0.0;
	if (!m_no_fp)
	{
		if (m_is_using_input_gen)
			E_fp_tot = m_m_dot_htf_tot * m_c_hdr_cold * (m_T_cold_in_1 - m_T_sys_h);       //[kg/s]*[J/kg-K]*[K] = [W]  Freeze protection energy required
		else
			E_fp_tot = m_m_dot_htf_tot * m_c_hdr_cold * (m_T_cold_in_1 - T_cold_in);       //[kg/s]*[J/kg-K]*[K] = [W]  Freeze protection energy required
	}

	if (qq>29)
	{
		m_error_msg = util::format("Mass Flow Rate Convergence Error");
		throw(C_csp_exception(m_error_msg, "Trough Collector Solver"));
	}

	m_m_dot_htfX = m_dot_htf;              //for calls back to the temperature loop during the same run, this ensures that the first mass flow rate used will be the last mass flow rate

	// ******************************************************************
	// Special condition checks
	// ******************************************************************

	//After convergence, check to see if the HTF flow rate is over the maximum level

post_convergence_flag: //11 continue
	if (((m_dot_htf > m_m_dot_htfmax) && (dfcount<5)) || ((m_defocus<1.0) && (dfcount == 1)))
	{

		//if so, the field m_defocus control must be engaged. for(int this by calculating the maximum
		//amount of absorbed energy that corresponds to the maximum mass flow rate

		//Now determine the number of SCA's that must be defocused to meet this absorption level
		double qsum = 0.;
		for (int i = 0; i<m_nSCA; i++){ qsum += m_q_abs_SCAtot[i]; }
		double q_check = max(qsum, 0.0); //limit to positive
		//mjw 11.4.2010: the max Q amount is based on the m_defocus control    
		if (dfcount == 1) q_abs_maxOT = min(q_check*m_defocus, m_c_htf_ave*m_m_dot_htfmax*(m_T_loop_out - m_T_loop_in_des));  //mjw 11.30.2010

		//Select the method of defocusing used for this system
		if (m_fthrctrl == 0)
		{
			//Standard defocusing.. 1 SCA at a time completely defocuses
			int j;
			for (j = 0; j<m_nSCA; j++)
			{
				//Incrementally subtract the losses, but limit to positive absorption values to avoid 
				//accounting for losses from previously defocused SCA's
				//q_check = q_check - max(m_q_abs_SCAtot(m_SCADefocusArray[j])-m_q_loss_SCAtot(m_SCADefocusArray[j]), 0.0)
				//4/9/11, TN: Don't need to subtract losses: see equation for q_check above
				q_check += -max(m_q_abs_SCAtot[(int)m_SCADefocusArray[j] - 1], 0.0);

				if (q_check <= q_abs_maxOT) break;	//[TO DO] check that j doesn't increment 
			}

			//Reassign the flux on each SCA
			for (int i = 0; i<j; i++)
			{
				m_q_SCA[(int)m_SCADefocusArray[i] - 1] = 0.0;
			}
		}
		else if (m_fthrctrl == 1)
		{
			//Partial defocusing in the sequence specified by the m_SCADefocusArray
			int j;
			for (j = 0; j<m_nSCA; j++)
			{
				//Incrementally subtract the losses, but limit to positive absorption values to avoid 
				//accounting for losses from previously defocused SCA's
				//q_check = q_check - min(max(m_q_abs_SCAtot(m_SCADefocusArray[j])-m_q_loss_SCAtot(m_SCADefocusArray[j]), 0.0),q_check-q_abs_maxOT)
				//4/9/11, TN: Don't need to subtract losses: see equation for q_check above
				q_check += -max(m_q_abs_SCAtot[(int)m_SCADefocusArray[j] - 1], 0.0);
				if (q_check <= q_abs_maxOT)
					break;
			}

			//Reassign the flux on each SCA
			for (int i = 0; i<j; i++)
			{
				m_q_SCA[(int)m_SCADefocusArray[i] - 1] = 0.0;
			}

			double tsum = 0.;
			for (int k = j + 1; k<m_nSCA; k++){ tsum += m_q_abs_SCAtot[(int)m_SCADefocusArray[k] - 1]; }

			m_q_SCA[(int)m_SCADefocusArray[j] - 1] *= (q_check - tsum) / m_q_abs_SCAtot[(int)m_SCADefocusArray[j] - 1];
		}
		else if (m_fthrctrl == 2)
		{
			//Partial defocusing of each SCA in the loop simultaneously. Specified m_defocus order is disregarded.
			for (int k = 0; k<m_nSCA; k++)
			{
				m_q_SCA[k] *= min(q_abs_maxOT / max(q_check, 1.e-6), 1.0);  //MJW 7.23.2010::Limit fraction to 1
			}
		}

		if (q_SCA_tot>0.)
		{
			double tsum = 0.;
			for (int k = 0; k<m_nSCA; k++){ tsum += m_q_SCA[k]; }
			SCAs_def = min(tsum / q_SCA_tot, 1.0);  //MJW 7.23.2010::Limit fraction to 1
		}
		else
		{
			SCAs_def = 1.;
		}

		SolveMode = 2;  //Mode 2 -> defocusing
		//mjw 11.4.2010 added conditional statement
		if (dfcount<5)
		{
			m_T_cold_in_1 = T_cold_in;		//12.29.14 twn: Shouldn't need to worry about freeze protection AND m_defocus, but, if both occur and m_defocus is adjusted, should try inlet temperature
			goto overtemp_iter_flag; //Return to recalculate with new m_defocus arrangement
		}
	}
	//Calculate the amount of defocusing
	if (q_SCA_tot>0.)
	{
		double tsum = 0.;
		for (int k = 0; k<m_nSCA; k++){ tsum += m_q_SCA[k]; }

		SCAs_def = min(tsum / q_SCA_tot, 1.0);  //MJW 7.23.2010::Limit fraction to 1
	}
	else
	{
		SCAs_def = 1.;
	}

calc_final_metrics_goto:

	// ******************************************************************************************************************************
	// Calculate the pressure drop across the piping system
	// ******************************************************************************************************************************
	//m_m_dot_htf = 8.673
	//------Inlet, Outlet, and COP
	DP_IOCOP = PressureDrop(m_dot_htf, (m_T_htf_in[0] + m_T_htf_out[m_nSCA-1]) / 2.0, 1.0, m_D_h((int)m_SCAInfoArray(0, 0), 0),
		m_HDR_rough, (40. + m_Row_Distance), 0.0, 0.0, 2.0, 0.0, 0.0, 2.0, 0.0, 0.0, 2.0, 1.0, 0.0);
	//if(ErrorFound()) return 1
	//-------HCE's
	m_DP_tube.assign(m_DP_tube.size(),0.0);
	for (int j = 0; j<m_nHCEVar; j++)
	{
		for (int i = 0; i<m_nSCA; i++)
		{
			int CT = (int)m_SCAInfoArray(i, 1) - 1;    //Collector type    
			int HT = (int)m_SCAInfoArray(i, 0) - 1;    //HCE type

			//Account for extra fittings on the first HCE
			double x1, x2;
			if (i == 0)
			{
				x1 = 10.0;
				x2 = 3.0;
			}
			else
			{
				x1 = 0.0;
				x2 = 1.0;
			}
			m_DP_tube[i] = m_DP_tube[i] + PressureDrop(m_dot_htf, m_T_htf_ave[i], 1.0, m_D_h(HT, j), (m_Rough(HT, j)*m_D_h(HT, j)),
				(m_L_SCA[CT] + m_Distance_SCA[CT]), 0.0, 0.0, x1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, x2)*m_HCE_FieldFrac(HT, j);
			//if(ErrorFound()) return 1
		}
	}
	//The pressure drop only across the loop
	DP_loop = 0.;
	for (int j = 0; j<m_nSCA; j++){ DP_loop += m_DP_tube[j]; }

	if (m_accept_loc == 1)
		m_m_dot_htf_tot = m_dot_htf*float(m_nLoops);
	else
		m_m_dot_htf_tot = m_dot_htf;

	//-------SGS to field section
	//if(m_FieldConfig==1.) { //"H" type
	//    x1 = 1.  
	//    //MJW changed length calculation 6-8-2010
	//    x2 = (40.+m_Row_Distance+(m_L_SCA(m_SCAInfoArray(1,2))+m_Distance_SCA(m_SCAInfoArray(1,2)))*float(m_nSCA))  //x2 is calculation for piping length
	//    x3 = 1. //number of expansions
	//} else { //"I" type
	//    x1 = 2.
	//    x2 = 50.
	//    x3 = 0.
	//}

	if (m_accept_loc == 1)
	{
		m_dot_run_in = std::numeric_limits<double>::quiet_NaN();

		if (m_nfsec > 2)  //mjw 5.4.11 Correct the mass flow for situations where m_nfsec/2==odd
		{
			m_dot_run_in = m_m_dot_htf_tot / 2.0 * (1. - float(m_nfsec % 4) / float(m_nfsec));
		}
		else
		{
			m_dot_run_in = m_m_dot_htf_tot / 2.0;
		}

		x3 = float(m_nrunsec) - 1.0;  //Number of contractions/expansions
		m_dot_temp = m_dot_run_in;
		DP_toField = 0.0;
		DP_fromField = 0.0;
		for (int i = 0; i<m_nrunsec; i++)
		{
			DP_toField = DP_toField + PressureDrop(m_dot_temp, m_T_htf_in[0], 1.0, m_D_runner[i], m_HDR_rough, m_L_runner[i], 0.0, x3, 0.0, 0.0,
				max(float(CSP::nint(m_L_runner[i] / 70.))*4., 8.), 1.0, 0.0, 1.0, 0.0, 0.0, 0.0);   //*m_dot_temp/m_dot_run_in  //mjw 5.11.11 Correct for less than all mass flow passing through each section
			//if(ErrorFound()) return 1                  
			//-------SGS from field section
			DP_fromField = DP_fromField + PressureDrop(m_dot_temp, m_T_htf_out[m_nSCA-1], 1.0, m_D_runner[i], m_HDR_rough, m_L_runner[i], x3, 0.0, 0.0, 0.0,
				max(float(CSP::nint(m_L_runner[i] / 70.))*4., 8.), 1.0, 0.0, 0.0, 0.0, 0.0, 0.0);   //*m_dot_temp/m_dot_run_in  //mjw 5.11.11 Correct for less than all mass flow passing through each section
			//if(ErrorFound()) return 1
			if (i>1)
				m_dot_temp = max(m_dot_temp - 2.*m_m_dot_htf_tot / float(m_nfsec), 0.0);
		}

		m_dot_header_in = m_m_dot_htf_tot / float(m_nfsec);
		m_dot_header = m_dot_header_in;
		DP_hdr_cold = 0.0;
		DP_hdr_hot = 0.0;
		for (int i = 0; i<m_nhdrsec; i++)
		{
			//Determine whether the particular section has an expansion valve
			double x2 = 0.0;
			if (i>0)
			{
				if (m_D_hdr[i] != m_D_hdr[i - 1])
					x2 = 1.;
			}

			//Calculate pressure drop in cold header and hot header sections.. both use similar information
			DP_hdr_cold = DP_hdr_cold + PressureDrop(m_dot_header, m_T_htf_in[0], 1.0, m_D_hdr[i], m_HDR_rough,
				(m_Row_Distance + 4.275)*2., 0.0, x2, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0); //*m_dot_header/m_dot_header_in  //mjw/tn 1.25.12 already account for m_dot_header in function call //mjw 5.11.11 scale by mass flow passing though
			//if(ErrorFound()) return 1
			DP_hdr_hot = DP_hdr_hot + PressureDrop(m_dot_header, m_T_htf_out[m_nSCA-1], 1.0, m_D_hdr[i], m_HDR_rough,
				(m_Row_Distance + 4.275)*2., x2, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0); //*m_dot_header/m_dot_header_in  //mjw 5.11.11
			//if(ErrorFound()) return 1
			//Siphon off header mass flow rate at each loop.  Multiply by 2 because there are 2 loops per hdr section
			m_dot_header = max(m_dot_header - 2.0*m_dot_htf, 0.0);
		}
	}

	double DP_tot = std::numeric_limits<double>::quiet_NaN();
	double W_dot_pump = std::numeric_limits<double>::quiet_NaN();
	double SCA_par_tot = std::numeric_limits<double>::quiet_NaN();
	if (m_accept_loc == 1)
	{
		// The total pressure drop in all of the piping
		DP_tot = (DP_loop + DP_hdr_cold + DP_hdr_hot + DP_fromField + DP_toField + DP_IOCOP);
		// The total pumping power consumption
		W_dot_pump = DP_tot*m_m_dot_htf_tot / (rho_hdr_cold*m_eta_pump) / 1000.;  //[kW]

		//The parasitic power consumed by electronics and SCA drives
		if (m_EqOpteff>0.0)
		{
			SCA_par_tot = m_SCA_drives_elec*SCAs_def*float(m_nSCA*m_nLoops);
		}
		else
		{
			SCA_par_tot = 0.0;
		}
	}
	else
	{
		// The total pressure drop in all of the piping
		DP_tot = (DP_loop + DP_IOCOP);
		// The total pumping power consumption
		W_dot_pump = DP_tot*m_dot_htf / (rho_hdr_cold*m_eta_pump) / 1000.;  //[kW]

		//The parasitic power consumed by electronics and SCA drives 
		if (m_EqOpteff>0.0)
		{
			SCA_par_tot = m_SCA_drives_elec*SCAs_def*float(m_nSCA);
		}
		else
		{
			SCA_par_tot = 0.0;
		}
	}

	// ******************************************************************
	// Calculate the system transient temperature for the next time step
	// ******************************************************************
	//First, calculate the amount of energy absorbed during the time step that didn't contribute to 
	//heating up the solar field
	E_avail_tot = 0.;
	double E_loop_accum = 0.0;
	E_int_sum = 0.;
	for (int i = 0; i<m_nSCA; i++)
	{
		E_avail_tot += m_E_avail[i];  //[J]
		E_loop_accum += m_E_accum[i]; //[J]
		E_int_sum += m_E_int_loop[i]; //[J]
	}

	double E_field = E_int_sum;

	// Average properties
	rho_ave = m_htfProps.dens((m_T_htf_out[m_nSCA-1] + m_T_sys_c) / 2.0, 0.0); //kg/m3
	m_c_htf_ave = m_htfProps.Cp((m_T_sys_h + m_T_cold_in_1) / 2.0)*1000.0;  //MJW 12.7.2010

	// Other calculated outputs
	double piping_hl_total = 0.0;

	double E_bal_startup = 0.0;
	double E_hdr_accum = 0.0;
	if (m_accept_loc == 1)		// Consider entire internal energy of entire field, not just the loop
	{
		E_avail_tot *= float(m_nLoops);
		E_loop_accum *= float(m_nLoops);
		E_field *= float(m_nLoops);

		//Calculate the HTF mass in the header, balance of field piping, piping to&from the steam generator (SGS) 
		//The mass of HTF in the system will be calculated based on the design loop inlet temperature
		//v_tot = m_v_hot + m_v_cold	//cc--> not used
		m_c_hdr_cold = m_htfProps.Cp(m_T_htf_in[0])* 1000.;
		m_c_hdr_hot = m_htfProps.Cp(m_T_htf_out[m_nSCA-1])* 1000.;

		//Half of the plant thermal mass must be heated up to the startup temperature (think hot header, hot half of heat
		//exchangers) and the other half must be heated up to the design loop inlet temperature
		//MJW 12.8.2010 modified startup temp calc to be based on previous system temperature
		//MJW 12.14.2010 Limit to positive to avoid step-to-step oscillation introduced by using previous step. 
		//.. This may cause a minor underestimation of annual energy output (<<.5%).
		E_hdr_accum = (m_v_hot*rho_hdr_hot*m_c_hdr_hot + m_mc_bal_hot)*(m_T_sys_h - m_T_sys_h_last) + //Hot half
			(m_v_cold*rho_hdr_cold*m_c_hdr_cold + m_mc_bal_cold)*(m_T_sys_c - m_T_sys_c_last);   //cold half

		if (!m_is_using_input_gen)
			E_bal_startup = max(E_hdr_accum, 0.0); //cold half
		else
			E_bal_startup = E_hdr_accum; //cold half

		//mjw 1.17.2011 Calculate the total energy content of the solar field relative to a standard ambient temp. of 25[C]			
		E_field += ((m_v_hot*rho_hdr_hot*m_c_hdr_hot + m_mc_bal_hot)*(m_T_sys_h - 298.150) +       //hot header and piping
			(m_v_cold*rho_hdr_cold*m_c_hdr_cold + m_mc_bal_cold)*(m_T_sys_c - 298.150));   //cold header and piping

		//6/14/12, TN: Redefine pipe heat losses with header and runner components to get total system losses
		double m_Pipe_hl_hot = m_N_run_mult*m_Runner_hl_hot + float(m_nfsec)*m_Header_hl_hot;
		double m_Pipe_hl_cold = m_N_run_mult*m_Runner_hl_cold + float(m_nfsec)*m_Header_hl_cold;

		piping_hl_total = m_Pipe_hl_hot + m_Pipe_hl_cold;

		if (!m_is_using_input_gen)
			E_avail_tot = max(E_avail_tot - piping_hl_total*dt, 0.0);		//[J] 11/1/11 TN: Include hot and cold piping losses in available energy calculation
		else
			E_avail_tot = E_avail_tot - piping_hl_total*dt;				//[J] 11/1/11 TN: Include hot and cold piping losses in available energy calculation

		E_avail_tot = max(E_avail_tot - E_bal_startup, 0.0);  //[J]
	}

	// ******************************************************************
	// Calculate final output values
	// ******************************************************************
	DP_tot = DP_tot * 1.e-5; //[bar]

	double m_dot_avail = std::numeric_limits<double>::quiet_NaN();
	double q_dot_avail = std::numeric_limits<double>::quiet_NaN();
	if (!m_is_using_input_gen)
	{
		//Calculate the thermal power produced by the field
		if (m_T_sys_h >= m_T_startup)   // MJW 12.14.2010 Limit field production to above startup temps. Otherwise we get strange results during startup. Does this affect turbine startup?
		{
			q_dot_avail = E_avail_tot / (dt)*1.e-6;  //[MW]
			//Calculate the available mass flow of HTF
			m_dot_avail = max(q_dot_avail*1.e6 / (m_c_htf_ave*(m_T_sys_h - m_T_cold_in_1)), 0.0); //[kg/s]     
		}
		else
		{
			q_dot_avail = 0.0;
			m_dot_avail = 0.0;
		}
	}
	else
	{
		q_dot_avail = E_avail_tot / (dt)*1.e-6;  //[MW]
		//Calculate the available mass flow of HTF
		m_dot_avail = max(q_dot_avail*1.e6 / (m_c_htf_ave*(m_T_sys_h - m_T_cold_in_1)), 0.0); //[kg/s]     
	}

	//Dumped energy
	double q_dump = m_Ap_tot*I_b*m_EqOpteff*(1.0 - SCAs_def) / 1.e6;  //[MW]

	//Total field performance
	double q_field_delivered = m_m_dot_htf_tot * m_c_htf_ave * (m_T_sys_h - m_T_cold_in_1) / 1.e6; //MJW 1.11.11 [MWt]

	double eta_thermal = std::numeric_limits<double>::quiet_NaN();
	if (I_b*m_CosTh_ave == 0.)	//cc--> Adding case for zero output. Was reporting -Infinity in original version
	{
		eta_thermal = 0.;
	}
	else
	{
		eta_thermal = q_field_delivered / (I_b*m_CosTh_ave*m_Ap_tot / 1.e6);  //MJW 1.11.11	
	}

set_outputs_and_return:

	//---------Do unit conversions and final calculations-------------

	double T_sys_h_out = m_T_sys_h - 273.15;			//[C] from K
	double m_dot_avail_out = m_dot_avail*3600.;			//[kg/hr] from kg/s
	double W_dot_pump_out = W_dot_pump / 1000.;			//[MW] from kW
	double E_fp_tot_out = E_fp_tot*1.e-6;				//[MW] from W
	double T_sys_c_out = m_T_sys_c - 273.15;			//[C] from K
	double EqOpteff_out = m_EqOpteff*m_CosTh_ave;
	double m_dot_htf_tot_out = m_m_dot_htf_tot *3600.;	//[kg/hr] from kg/s
	double E_bal_startup_out = E_bal_startup / (dt*1.e6);	//[MW] from J
	double q_inc_sf_tot = m_Ap_tot*I_b / 1.e6;
	double q_abs_tot = 0.;
	double q_loss_tot = 0.;
	double q_loss_spec_tot = 0.;

	for (int i = 0; i<m_nSCA; i++)
	{
		q_abs_tot += m_q_abs_SCAtot[i] * 1.e-6;               //*float(m_nLoops);
		q_loss_tot += m_q_loss_SCAtot[i] * 1.e-6;             //*float(m_nLoops);
		q_loss_spec_tot += m_q_1abs_tot[i] / float(m_nSCA);
	}

	if (m_accept_loc == 1)
	{
		q_abs_tot *= float(m_nLoops);
		q_loss_tot *= float(m_nLoops);
	}

	double
		SCA_par_tot_out = SCA_par_tot * 1.e-6,
		Pipe_hl_out = piping_hl_total * 1.e-6,		//[MW] convert from W
		Theta_ave_out = m_Theta_ave / m_d2r,
		CosTh_ave_out = m_CosTh_ave;

	double dni_costh = I_b*m_CosTh_ave;
	double qinc_costh = dni_costh * m_Ap_tot / 1.e6;
	double T_loop_outlet = m_T_htf_out[m_nSCA-1] - 273.15;

	double E_loop_accum_out = E_loop_accum * 3.6e-9;
	double E_hdr_accum_out = E_hdr_accum * 3.6e-9;

	double E_tot_accum = E_loop_accum_out + E_hdr_accum_out;

	double E_field_out = E_field*3.6e-9;
	//------------------------------------------------------------------

	//Set outputs
	cr_out_solver.m_T_salt_hot = T_sys_h_out;		//[C] Solar field HTF outlet temperature
	//value(O_T_SYS_H, T_sys_h_out);				//[C] Solar field HTF outlet temperature
	cr_out_solver.m_m_dot_salt_tot = m_dot_avail_out;	//[kg/hr] HTF mass flow rate from the field
	//value(O_M_DOT_AVAIL, m_dot_avail_out);			//[kg/hr] HTF mass flow rate from the field
	
	//value(O_Q_AVAIL, m_q_avail);					//[MWt] Thermal power produced by the field
	//value(O_DP_TOT, m_DP_tot);					//[bar] Total HTF pressure drop
	
	cr_out_solver.m_W_dot_htf_pump = W_dot_pump_out;	//[MWe] Required solar field pumping power
	//value(O_W_DOT_PUMP, W_dot_pump_out);		//[MWe] Required solar field pumping power
	cr_out_solver.m_E_fp_total = E_fp_tot_out;	//[MW] Freeze protection energy
	//value(O_E_FP_TOT, E_fp_tot_out);			//[MW] Freeze protection energy
	
	//value(O_QQ, m_qq);							//[none] Number of iterations required to solve
	//value(O_T_SYS_C, T_sys_c_out);				//[C] Collector inlet temperature
	//value(O_EQOPTEFF, EqOpteff_out);			//[none] Collector equivalent optical efficiency
	//value(O_SCAS_DEF, m_SCAs_def);				//[none] The fraction of focused SCA's
	//value(O_M_DOT_HTF_TOT, m_dot_htf_tot_out);	//[kg/hr] The actual flow rate through the field..
	//value(O_E_BAL_STARTUP, E_bal_startup_out);	//[MWt] Startup energy consumed
	//value(O_Q_INC_SF_TOT, m_q_inc_sf_tot);		//[MWt] Total power incident on the field
	//value(O_Q_ABS_TOT, m_q_abs_tot);				//[MWt] Total absorbed energy
	//value(O_Q_LOSS_TOT, m_q_loss_tot);			//[MWt] Total receiver thermal and optical losses
	//value(O_M_DOT_HTF, m_m_dot_htf);				//[kg/s] Flow rate in a single loop
	//value(O_Q_LOSS_SPEC_TOT, m_q_loss_spec_tot);	//[W/m] Field-average receiver thermal losses (convection and radiation)
	
	cr_out_solver.m_W_dot_col_tracking = SCA_par_tot_out;	//[MWe] Parasitic electric power consumed by the SC
	//value(O_SCA_PAR_TOT, SCA_par_tot_out);		//[MWe] Parasitic electric power consumed by the SC
	
	//value(O_PIPE_HL, Pipe_hl_out);				//[MWt] Pipe heat loss in the header and the hot runner
	//value(O_Q_DUMP, m_q_dump);					//[MWt] Dumped thermal energy
	//value(O_THETA_AVE, Theta_ave_out);			//[deg] Field average m_theta value
	//value(O_COSTH_AVE, CosTh_ave_out);			//[none] Field average costheta value
	//value(O_IAM_AVE, m_IAM_ave);					//[none] Field average incidence angle modifier
	//value(O_ROWSHADOW_AVE, m_RowShadow_ave);		//[none] Field average row shadowing loss
	//value(O_ENDLOSS_AVE, m_EndLoss_ave);			//[none] Field average end loss
	//value(O_DNI_COSTH, m_dni_costh);				//[W/m2] DNI_x_CosTh
	//value(O_QINC_COSTH, m_qinc_costh);			//[MWt] Q_inc_x_CosTh
	//value(O_T_LOOP_OUTLET, m_t_loop_outlet);		//[C] HTF temperature immediately subsequent to the loop outlet
	//value(O_C_HTF_AVE, m_c_htf_ave);				//[J/kg-K] Average solar field specific heat
	//value(O_Q_FIELD_DELIVERED, m_q_field_delivered);		//[MWt] Total solar field thermal power delivered
	//value(O_ETA_THERMAL, m_eta_thermal);			//[none] Solar field thermal efficiency (power out/ANI)
	//value(O_E_LOOP_ACCUM, E_loop_accum_out);	//[MWht] Accumulated internal energy change rate in the loops ONLY
	//value(O_E_HDR_ACCUM, E_hdr_accum_out);		//[MWht] Accumulated internal energy change rate in the headers/SGS
	//value(O_E_TOT_ACCUM, m_E_tot_accum);			//[MWht] Total accumulated internal energy change rate
	//value(O_E_FIELD, E_field_out);				//[MWht] Accumulated internal energy in the entire solar field
	//value(O_T_C_IN_CALC, m_T_cold_in_1 - 273.15);	//[C] Calculated cold HTF inlet temperature - used in freeze protection and for stand-alone model in recirculation

	return;
}

void C_csp_trough_collector_receiver::converged()
{
	/*
	-- Post-convergence call --

	Update values that should be transferred to the next time step
	*/

	m_ss_init_complete = true;

	m_T_sys_c_converged = m_T_sys_c_last = m_T_sys_c;		//[K]
	m_T_sys_h_converged = m_T_sys_h_last = m_T_sys_h;		//[K]
	for (int i = 0; i<m_nSCA; i++)
	{
		m_T_htf_in_converged[i] = m_T_htf_in_last[i] = m_T_htf_in[i];		//[K]
		m_T_htf_out_converged[i] = m_T_htf_out_last[i] = m_T_htf_out[i];	//[K]
		m_T_htf_ave[i] = m_T_htf_ave_last[i] = (m_T_htf_in_last[i] + m_T_htf_out_last[i]) / 2.0;	//[K]
	}

	m_ncall = -1;	//[-]

	if( m_operating_mode == C_csp_collector_receiver::STEADY_STATE )
	{
		throw(C_csp_exception("Receiver should only be run at STEADY STATE mode for estimating output. It must be run at a different mode before exiting a timestep",
			"Trough converged method"));
	}

	m_operating_mode_converged = m_operating_mode;	//[-]

	// Always reset the m_defocus control at the first call of a timestep
	m_defocus_new = 1.0;	//[-]
	m_defocus_old = 1.0;	//[-]
	m_defocus = 1.0;		//[-]

	// Reset the optical efficiency member data
	loop_optical_eta_off();

	return;
}

double C_csp_trough_collector_receiver::calculate_optical_efficiency(const C_csp_weatherreader::S_outputs &weather, const C_csp_solver_sim_info &sim)
{
	return std::numeric_limits<double>::quiet_NaN();
}

double C_csp_trough_collector_receiver::calculate_thermal_efficiency_approx(const C_csp_weatherreader::S_outputs &weather, double q_incident /*MW*/)
{
	return std::numeric_limits<double>::quiet_NaN();
}

double C_csp_trough_collector_receiver::get_collector_area()
{
	return std::numeric_limits<double>::quiet_NaN();
}

// ------------------------------------------ supplemental methods -----------------------------------------------------------


/*
This subroutine contains the trough detailed plant model.  The collector field is modeled
using an iterative solver.
This code was written for the National Renewable Energy Laboratory
Copyright 2009-2010
Author: Mike Wagner

Subroutine Inputs (and parameters)
----------------------------------------------------------------------------------------------------------------------
Nb | Variable             | Description                                             | Input  Units   | Internal Units
---|----------------------|---------------------------------------------------------|----------------|----------------
1  | T_1_in               | Receiver inlet temperature                              |                |
2  | m_dot                | Heat transfer fluid mass flow rate                      |                |
3  | T_amb                | Ambient dry-bulb temperature                            |                |
4  | m_T_sky                | Sky temperature                                         |                |
5  | v_6                  | Ambient wind velocity                                   |                |
6  | P_6                  | Ambient atmospheric pressure                            |                |
7  | m_q_i                  | Total incident irradiation on the receiver              |                |
8  | m_A_cs                 | Internal absorber tube cross-sectional area             |                |
9  | m_D_2                  | Internal absorber tube diameter                         |                |
10 | m_D_3                  | External absorber tube diameter                         |                |
11 | m_D_4                  | Internal glass envelope diameter                        |                |
12 | m_D_5                  | External glass envelope diameter                        |                |
13 | m_D_p                  | (optional) Plug diameter                                |                |
14 | m_D_h                  | Absorber tube hydraulic diameter                        |                |
15 | eps_mode             | Interpolation mode for the emissivity (1=table,2=fixed) |                |
16 | xx                   | Array of temperature values for emissivity table        |                |
17 | yy                   | Array of emissivity values for table                    |                |
18 | nea                  | Number of entries in the emissivity table               |                |
19 | m_L_actSCA             | Length of the active receiver surface                   |                |
20 | single_point         | Logical flag - is the calculation for a single point?   |                |
21 | Epsilon_32           | Constant value for emissivity if table isn't used       |                |
22 | Epsilon_4            | Envelope inner surface emissivity                       |                |
23 | m_EPSILON_5            | Envelope outer surface emissivity                       |                |
24 | Alpha_abs            | Absorber tube absorptance                               |                |
25 | m_alpha_env            | Envelope absorptance                                    |                |
26 | m_ColOptEff            | Collector optical efficiency                            |                |
27 | m_Tau_envelope         | Total envelope transmittance                            |                |
28 | m_P_a                  | Annulus gas pressure                                    | torr           |
29 | m_Flow_type            | Flag indicating the presence of an internal plug        |                |
30 | m_AnnulusGas           | Annulus gas type                                        |                |
31 | m_Fluid                | Heat transfer fluid type                                |                |
32 | m_AbsorberMaterial     | Absorber material type                                  |                |
33 | time                 | Simulation time                                         |                |

Subroutine outputs
----------------------------------------------------------------------------------------------------------------------
Nb | Variable             | Description                                             | Input  Units   | Internal Units
---|----------------------|---------------------------------------------------------|----------------|----------------
1  | q_heatloss           | Total heat loss from the receiver                       | W/m            |
2  | q_12conv             | Total heat absorption into the HTF                      | W/m            |
3  | q_34tot              | Convective and radiative heat loss                      |                |
4  | c_1ave               | Specific heat of the HTF across the receiver            | kJ/kg-K        |
5  | rho_1ave             | Density of the HTF across the receiver                  |                |

----------------------------------------------------------------------------------------------------------------------
Forristall Temperature distribution diagram
*****************************************************
m_Fluid (1) ----------->(2)<--Absorber-->(3)<-- Annulus -->(4)<--- Glass  --->(5)<-- Air (6)/Sky (7)


T_1 = Bulk heat transfer fluid (HTF) temperature
T_2 = Absorber Inside surface temperature
T_3 = Absorber outside surface temperature
T_4 = Glass envelope inside surface temperature
T_5 = Glass envelope outside surface temperature
T_6 = Ambient temperature
T_7 = Effective Sky Temperature

q_12conv = Convection heat transfer rate per unit length between the HTF and the inside of the receiver tube
q_23cond = Conduction heat transfer rate per unit length through the absorber
q_34conv = Convection heat transfer rate per unit length between the absorber outer surface and the glazing inner surface
q_34rad = Radiation heat transfer rate per unit length between the absorber outer surface and the glazing inner surface
q_45cond = Conduction heat transfer rate per unit length through the glazing
q_56conv = Convection heat transfer rate per unit length between the glazing outer surface and the ambient air
q_57rad = Radiation heat transfer rate per unit length between the glazing outer surface and the sky
----------------------------------------------------------------------------------------------------------------------
*/
/*
void EvacReceiver(double T_1_in, double m_dot, double T_amb, double m_T_sky, double v_6, double P_6, double m_q_i, double m_A_cs,
double m_D_2, double m_D_3, double m_D_4, double m_D_5, double m_D_p, double m_D_h, int eps_mode, double xx[], double yy[], int nea,
double m_L_actSCA,double single_point,  double Epsilon_32, double Epsilon_4,
double m_EPSILON_5, double Alpha_abs, double m_alpha_env, double m_ColOptEff, double m_Tau_envelope, double m_P_a, int m_Flow_type,
int m_AbsorberMaterial, int annulusGas, bool glazingIntact, int m_Fluid, int ncall, double time,
//outputs
double q_heatloss, double q_12conv, double q_34tot, double c_1ave, double rho_1ave)
*/
void C_csp_trough_collector_receiver::EvacReceiver(double T_1_in, double m_dot, double T_amb, double m_T_sky, double v_6, double P_6, double m_q_i,
	int hn /*HCE number [0..3] */, int hv /* HCE variant [0..3] */, int ct /*Collector type*/, int sca_num, bool single_point, int ncall, double time,
	//outputs
	double &q_heatloss, double &q_12conv, double &q_34tot, double &c_1ave, double &rho_1ave)
{

	//cc -- note that collector/hce geometry is part of the parent class. Only the indices specifying the
	//		number of the HCE and collector need to be passed here.

	//---Variable declarations------
	bool reguess;
	double T_2, T_3, T_4, T_5, T_6, T_7, m_v_1, k_23, q_34conv, q_34rad, h_34conv, h_34rad, q_23cond,
		k_45, q_45cond, q_56conv, h_56conv, q_57rad, q_3SolAbs, q_5solabs, q_cond_bracket, R_45cond,
		T_2g, cp_1, T3_tol, q5_tol, T1_tol, T2_tol, Diff_T3, diff_q5, T_lower, T_upper,
		q_5out, T_1_out, diff_T1, T_1_ave, T_1_out1, diff_T2, eps_3, q_in_W, T_upper_max, y_upper,
		y_lower, upmult, q5_tol_1, T3_upper, T3_lower, y_T3_upper, y_T3_lower, abs_diffT3;

	bool UPFLAG, LOWFLAG, T3upflag, T3lowflag, is_e_table;
	int m_qq, q5_iter, T1_iter, q_conv_iter;

	double T_save_tot, colopteff_tot;
	
	//cc--> note that xx and yy have size 'nea'

	//-------

	bool glazingIntact = m_GlazingIntact(hn, hv); //.at(hn, hv);

	//---Re-guess criteria:---
	if (m_reguess_args == NULL) goto lab_reguess;

	if (time <= 2) goto lab_reguess;
	
	if (((int)m_reguess_args[0] == 1) != m_GlazingIntact(hn, hv)) goto lab_reguess;	//glazingintact state has changed

	if (m_P_a(hn, hv) != m_reguess_args[1]) goto lab_reguess;                   //Reguess for different annulus pressure

	if (fabs(m_reguess_args[2] - T_1_in) > 50.) goto lab_reguess;

	for (int i = 0; i<5; i++){ if (m_T_save[i] < m_T_sky - 1.) goto lab_reguess; }

	T_save_tot = 0.;
	for (int i = 0; i<5; i++){ T_save_tot += m_T_save[i]; }
	if (T_save_tot != T_save_tot) goto lab_reguess;	//NaN check.. a value is only not equal to itself if it is NaN

	reguess = false;
	goto lab_keep_guess;
lab_reguess:
	reguess = true;
lab_keep_guess:


	//------------------------

	if (reguess) {
		if (m_GlazingIntact(hn, hv)) {
			m_T_save[0] = T_1_in;
			m_T_save[1] = T_1_in + 2.;
			m_T_save[2] = m_T_save[1] + 5.;
			if (m_P_a(hn, hv) > 1.0){          //Set guess values for different annulus pressures
				m_T_save[3] = m_T_save[2] - 0.5*(m_T_save[2] - T_amb);       //If higher pressure, guess higher T4   
				T_upper_max = m_T_save[2] - 0.2*(m_T_save[2] - T_amb);     //Also, high upper limit for T4
			}
			else{
				m_T_save[3] = m_T_save[2] - 0.9*(m_T_save[2] - T_amb);       //If lower pressure, guess lower T4
				T_upper_max = m_T_save[2] - 0.5*(m_T_save[2] - T_amb);     //Also, low upper limit for T4
			}
			m_T_save[4] = m_T_save[3] - 2.;
			if (m_reguess_args != NULL){
				m_reguess_args[1] = m_P_a(hn, hv);               //Reset previous pressure
				m_reguess_args[0] = m_GlazingIntact(hn, hv) ? 1. : 0.;   //Reset previous glazing logic
				m_reguess_args[2] = T_1_in;            //Reset previous T_1_in
			}
		}
		else{
			m_T_save[0] = T_1_in;
			m_T_save[1] = T_1_in + 2.;
			m_T_save[2] = m_T_save[1] + 5.;
			m_T_save[3] = T_amb;
			m_T_save[4] = T_amb;
			if (m_reguess_args != NULL){
				m_reguess_args[0] = m_GlazingIntact(hn, hv) ? 1. : 0.;   //Reset previous glazing logic
				m_reguess_args[1] = T_1_in;            //Reset previous T_1_in
			}
		}
	}

	//Set intial guess values
	T_2 = m_T_save[1];
	T_3 = m_T_save[2];
	T_4 = m_T_save[3];
	T_5 = m_T_save[4];
	//Set constant temps
	T_6 = T_amb;
	T_7 = m_T_sky;

	m_qq = 0;                  //Set iteration counter for T3 loop

	T_2g = T_2;              //Initial guess value for T_2 (only used in property lookup)        
	cp_1 = 1950.;            //Initial guess value for cp of WF

	//Tolerances for iteration
	T3_tol = 1.5e-3;
	q5_tol = 1.0e-3;         //Since iterations are nested inside T3, make tolerances a bit tighter        
	T1_tol = 1.0e-3;
	T2_tol = 1.0e-3;

	//Decreasing the tolerance helps get out of repeating m_defocus iterations
	if (ncall>8) {
		T3_tol = 1.5e-4;        //1.0   
		q5_tol = 1.0e-4;        //max(1.0, 0.001*m_q_i)
		T1_tol = 1.0e-4;        //1.0
		T2_tol = 1.0e-4;        //1.0
	}

	Diff_T3 = 10.0 + T3_tol;    //Set difference > tolerance

	//Constants
	k_45 = 1.04;                             //[W/m-K]  Conductivity of glass
	R_45cond = log(m_D_5(hn, hv) / m_D_4(hn, hv)) / (2.*CSP::pi*k_45);    //[K-m/W]Equation of thermal resistance for conduction through a cylinder

	colopteff_tot = m_ColOptEff(ct, sca_num)*m_Dirt_HCE(hn, hv)*m_Shadowing(hn, hv);	//The total optical efficiency

	if (m_GlazingIntact(hn, hv)){   //These calculations (q_3SolAbs,q_5solAbs) are not dependent on temperature, so only need to be computed once per call to subroutine

		q_3SolAbs = m_q_i * colopteff_tot * m_Tau_envelope.at(hn, hv) * m_alpha_abs.at(hn, hv);  //[W/m]  
		//We must account for the radiation absorbed as it passes through the envelope
		q_5solabs = m_q_i * colopteff_tot * m_alpha_env(hn, hv);   //[W/m]  
	}
	else{
		//Calculate the absorbed energy 
		q_3SolAbs = m_q_i * colopteff_tot * m_alpha_abs(hn, hv);  //[W/m]  
		//No envelope
		q_5solabs = 0.0;                            //[W/m]

	}

	is_e_table = false;
	if (m_epsilon_3.getTableSize(hn, hv) < 2){
		eps_3 = m_epsilon_3.getSingleValue(hn, hv);
	}
	else{
		eps_3 = m_epsilon_3.interpolate(hn, hv, T_3 - 273.15);          //Set epsilon value for case that eps_mode = 1.  Will reset inside temp loop if eps_mode > 1.
		is_e_table = true;	//The emissivity is in tabular form
	}

	T3upflag = false;
	T3lowflag = false;

	double T3_adjust = 0.0;
	double T3_prev_qq = 0.0;

	while (((fabs(Diff_T3)>T3_tol) && (m_qq<100)) || (m_qq<2)){    //Outer loop: Find T_3 such than energy balance is satisfied
		m_qq = m_qq + 1; //loop counter

		T3_prev_qq = T_3;

		if (m_qq>1){
			if ((T3upflag) && (T3lowflag)){
				if (Diff_T3 > 0.){
					T3_upper = T_3;
					y_T3_upper = Diff_T3;
				}
				else {
					T3_lower = T_3;
					y_T3_lower = Diff_T3;
				}
				T_3 = (y_T3_upper) / (y_T3_upper - y_T3_lower)*(T3_lower - T3_upper) + T3_upper;

			}
			else {
				if (Diff_T3 > 0.){
					T3_upper = T_3;
					y_T3_upper = Diff_T3;
					T3upflag = true;
				}
				else {
					T3_lower = T_3;
					y_T3_lower = Diff_T3;
					T3lowflag = true;
				}

				if ((T3upflag) && (T3lowflag)){
					T_3 = (y_T3_upper) / (y_T3_upper - y_T3_lower)*(T3_lower - T3_upper) + T3_upper;
				}
				else
				{
					if (Diff_T3 > 0.0)
						T_3 = T_3 - 50.0;
					else
						T_3 = T_3 + 50.0;
					//T_3 = T_3 - abs_diffT3;         //Note that recalculating T_3 using this exact equation, rather than T_3 = T_3 - frac*diff_T3 was found to solve in fewer iterations
				}
			}
		}

		T3_adjust = T_3 - T3_prev_qq;

		//Calculate temperature sensitive emissivity using T_3, if required
		if (is_e_table) eps_3 = m_epsilon_3.interpolate(hn, hv, (T_3 - 273.15)); //call interp((T_3-273.15),eps_mode,xx,yy,eps3old,eps_3)

		//Separate m_GlazingIntact = true and m_GlazingIntact = false  If true, T4 must be solved, if false then T4 is explicitly known (or doesn't exist, depending on how you want to look at it)
		//Solving for correct T4 as it relates to current T3 value
		if (m_GlazingIntact(hn, hv)) {

			//**********************************************
			//************* SET UP T_4 ITERATION **********************
			//**********************************************

			//if(m_qq==1){               //If first iteration, set T_4 bounds to phyiscal limits defined by T_3 and m_T_sky
			//	T_lower = m_T_sky;         //Lowest possible temperature of T_4 is sky temp        
			//	T_upper = max(T_upper_max,T_amb);    //Highest possible temperature is the highest temperature on either side of T_4: either T_3 or ambient
			//	q5_tol_1= 0.001;           //Just get T4 in the ball park.  '20' may not be the optimum value.....
			//} 
			//else {                                            //For additional iterations:
			//	T_lower = T_lower - max(abs_diffT3,0.0);       //If diff_T3 is + then new T3 < old T3 so adjust lower limit
			//	T_upper = T_upper + fabs(min(abs_diffT3,0.0));  //If diff_T3 is (-) then new T3 > old T3 so adjust upper limit
			//	q5_tol_1= q5_tol;        //For remaining T3 iterations, use specified tolerance (note that 2 iterations for T3 are gauranteed)                   
			//}

			if (m_qq == 1)
			{
				T_lower = m_T_sky;
				T_upper = max(T_3, T_amb);
			}
			else
			{
				if (T3_adjust > 0.0)	// new T3 > old T3 so adjust upper limit
				{
					T_upper = min(T_3, T_upper + 1.25*T3_adjust);
					T_lower = T_4;
					T_4 = T_4 + 0.5*T3_adjust;
				}
				else
				{
					T_lower = max(m_T_sky, T_lower + 1.25*T3_adjust);
					T_upper = T_4;
					T_4 = T_4 + 0.5*T3_adjust;
				}
			}

			q5_tol_1 = q5_tol;

			diff_q5 = q5_tol_1 + 1.0;       //Set diff > tolerance
			q5_iter = 0;                     //Set iteration counter

			UPFLAG = false;           //Set logic to switch from bisection to false position mode
			LOWFLAG = false;           //Set logic to switch from bisection to false position mode   
			//***********************************************************************************
			//************* Begin Bisection/False Position Iteration method *********************
			//***********************************************************************************
			while ((fabs(diff_q5)>q5_tol_1) && (q5_iter<100)){       //Determine T_4 such that energy balance from T_3 to surroundings is satisfied

				q5_iter = q5_iter + 1;                       //Increase iteration counter

				//The convective heat exchange between the absorber and the envelope
				//      UNITS   ( K , K, torr, Pa , m/s,  K , -, -, W/m, W/m2-K)
				FQ_34CONV(T_3, T_4, P_6, v_6, T_6, hn, hv, q_34conv, h_34conv);

				//The radiative heat exchange between the absorber and the envelope
				//    Units         ( K ,  K ,  m ,  m , K  ,    -     ,    -     ,   logical    ,  W/m   , W/m2-K)
				FQ_34RAD(T_3, T_4, T_7, eps_3, hn, hv, q_34rad, h_34rad);
				//The total heat exchange between absorber and envelope
				q_34tot = q_34conv + q_34rad;            //[W/m]

				//**********************************************
				//************* Calculate T_5 *************
				//**********************************************
				//The thermal energy flow across 45 is equal to the energy from the absorber plus
				//the thermal energy that is generated by direct heating of the glass envelope
				q_45cond = q_34tot + q_5solabs;          //[W/m]

				//Knowing heat flow and properties, T_5 can be calculated
				T_5 = T_4 - q_45cond*R_45cond;           //[K]

				//*************************************************************************
				//************* Calculate HT from exterior surface to ambient *************
				//*************************************************************************
				//With T_5 and T_6 (amb T) calculate convective and radiative loss from the glass envelope
				//           units   ( K ,  K ,  torr, m/s, -, -, W/m, W/m2-K)
				FQ_56CONV(T_5, T_6, P_6, v_6, hn, hv, q_56conv, h_56conv); //[W/m]
				q_57rad = m_EPSILON_5(hn, hv) * 5.67e-8 * (pow(T_5, 4) - pow(T_7, 4));
				q_5out = q_57rad + q_56conv;     //[W/m]

				//***************************************************************************
				//********** Compare q_5out with q_45 cond***********************************
				//***************************************************************************
				diff_q5 = (q_5out - q_45cond) / fabs(q_45cond);     //[W/m]

				//Determine next guess for T_4.  Want to use false position method, but it requires that the *results* at both ends of the bracket are known.  We have
				//defined a bracket but not the results.  Use the guess T_4 to get the results at one end of a new bracket.  Then calculate a new T_4 that is highly weighted 
				//towards the side of the original bracket that the 1st T_4 did not replace.  In most cases, this new T_4 will result in the opposite diff_q5, which 
				//defines both sides of the bracket.  If results for both sides are then defined, "LOWFLAG" and "UPFLAG" will be true, and false position method will be applied.

				if (LOWFLAG && UPFLAG){          //False position method     
					if (diff_q5>0.0){
						T_upper = T_4;       //If energy leaving T_5 is greater than energy entering T_5, then T_4 guess is too high
						y_upper = diff_q5;   //so set new upper limit to T_4
					}
					else {                    //If energy leaving T_5 is less than energy entering T_5, then T_4 guess is too low
						T_lower = T_4;       //so set new lower limit to T_4
						y_lower = diff_q5;   //also, set result to go along with lower limit
					}
					T_4 = (y_upper) / (y_upper - y_lower)*(T_lower - T_upper) + T_upper;

				}
				else {                        //For bisection method...

					if (diff_q5>0.0){    //If energy leaving T_5 is greater than energy entering T_5, then T_4 guess is too high
						T_upper = T_4;       //so set new upper limit to T_4
						y_upper = diff_q5;   //also, set result to go along with upper limit
						UPFLAG = true;    //Upper result is now known
						if (m_qq == 1){
							upmult = 0.1;       //Just want to get in ballpark for first iteration of receiver
						}
						else {
							upmult = 0.1;       //Weight such that next calculated T_4 (if using bisection method) results in negative diff_q5
						}

					}
					else {                    //If energy leaving T_5 is less than energy entering T_5, then T_4 guess is too low          
						T_lower = T_4;       //so set new lower limit to T_4
						y_lower = diff_q5;   //also, set result to go along with lower limit
						LOWFLAG = true;    //Lower result is now known
						if (m_qq == 1){
							upmult = 0.1;       //Just want to get in ballpark for first iteration of receiver
						}
						else {
							upmult = 0.9;       //Weight such that next calculated T_4 (if using bisection method) results in positive diff_q5
						}

					}

					if (LOWFLAG && UPFLAG){  //If results of bracket are defined, use false position
						T_4 = (y_upper) / (y_upper - y_lower)*(T_lower - T_upper) + T_upper;
					}
					else {                            //If not, keep bisection
						T_4 = (1. - upmult)*T_lower + upmult*T_upper;
					}

				}

				//*********************************************************************************************
				//********** END Bisection/False Position Iteration Loop on T_4 *******************************
				//*********************************************************************************************       
			}

		}
		else {      //Glazing is not intact

			//Know convection and radiation forcing temps
			//----Having guessed the system temperatures, calculate the thermal losses starting from
			//----the absorber surface (3)
			//The convective heat exchange between the absorber and the envelope
			FQ_34CONV(T_3, T_4, P_6, v_6, T_6, hn, hv, q_34conv, h_34conv);
			//The radiative heat exchange between the absorber and the envelope
			FQ_34RAD(T_3, T_4, T_7, eps_3, hn, hv, q_34rad, h_34rad);
			//The total heat exchange between absorber and envelope
			q_34tot = q_34conv + q_34rad;    //[W/m]

		}      //Know heat transfer from outer surface of receiver tube to ambient

		//Bracket Losses
		//Bracket conduction losses apply 
		q_cond_bracket = FQ_COND_BRACKET(T_3, T_6, P_6, v_6, hn, hv); //[W/m] 

		q_12conv = q_3SolAbs - (q_34tot + q_cond_bracket);         //[W/m] Energy transfer to/from fluid based on energy balance at T_3

		q_in_W = q_12conv * m_L_actSCA[ct];                           //Convert [W/m] to [W] for some calculations

		if (!single_point) {
			T_1_out = max(m_T_sky, q_in_W / (m_dot*cp_1) + T_1_in);    //Estimate outlet temperature with previous cp

			diff_T1 = T1_tol + 1.0;                                 //Set diff > tolerance
			T1_iter = 0;                                             //Set iteration counter    

			while ((fabs(diff_T1)>T1_tol) && (T1_iter<100)){       //Find correct cp& rho and solve for T_1_ave

				T1_iter++;                   //Increase iteration counter
				T_1_ave = (T_1_out + T_1_in) / 2.0;     //Average fluid temperature
				cp_1 = m_htfProps.Cp(T_1_ave)*1000.;
				T_1_out1 = max(m_T_sky, q_in_W / (m_dot*cp_1) + T_1_in);  //Estimate outlet temperature with previous cp 
				diff_T1 = (T_1_out - T_1_out1) / T_1_out;  //Difference between T_1_out used to calc T_ave, and T_1_out calculated with new cp
				T_1_out = T_1_out1;                      //Calculate new T_1_out

			}
		}
		else {
			//If we're only calculating performance for a single point, set the receiver ave/outlet temperature to the inlet.
			T_1_out = T_1_in;
			T_1_ave = T_1_in;
		}

		rho_1ave = m_htfProps.dens(T_1_ave, 0.0);       //[kg/m^3] Density
		m_v_1 = m_dot / (rho_1ave*m_A_cs(hn, hv));             //HTF bulk velocity

		q_conv_iter = 0;                 //Set iteration counter
		diff_T2 = 1.0 + T2_tol;         //Set diff > tolerance

		bool T2upflag = false;
		bool T2lowflag = false;

		double y_T2_low = std::numeric_limits<double>::quiet_NaN();
		double y_T2_up = std::numeric_limits<double>::quiet_NaN();

		double T2_low = min(T_1_ave, T_3);
		double T2_up = max(T_1_ave, T_3);

		//Ensure convective calculations are correct (converge on T_2)
		while ((fabs(diff_T2)>T2_tol) && (q_conv_iter<100)){

			q_conv_iter++;       //Increase iteration counter

			T_2 = max(10.0, fT_2(q_12conv, T_1_ave, T_2g, m_v_1, hn, hv));	//Calculate T_2 (with previous T_2 as input)
			diff_T2 = (T_2 - T_2g) / T_2;          //T_2 difference

			if (diff_T2 > 0.0)			// Calculated > Guessed, set lower limit and increase guessed
			{
				T2_low = T_2g;
				T2lowflag = true;
				y_T2_low = diff_T2;
				if (T2upflag)
					T_2g = y_T2_up / (y_T2_up - y_T2_low)*(T2_low - T2_up) + T2_up;
				else
					T_2g = T2_up;
			}
			else						// Calculated < Guessed, set upper limit and decrease guessed
			{
				T2_up = T_2g;
				T2upflag = true;
				y_T2_up = diff_T2;
				if (T2lowflag)
					T_2g = y_T2_up / (y_T2_up - y_T2_low)*(T2_low - T2_up) + T2_up;
				else
					T_2g = T2_low;
			}

			if ((T2_up - T2_low) / T2_low < T2_tol / 10.0)
				break;

		}

		//The conductive heat transfer equals the convective heat transfer (energy balance)
		q_23cond = q_12conv;         //[W/m]

		//Calculate tube conductivity 
		k_23 = FK_23(T_2, T_3, hn, hv);       //[W/m-K]  

		//Update the absorber surface temperature (T_3) according to new heat transfer rate
		abs_diffT3 = T_3 - (T_2 + q_23cond*log(m_D_3(hn, hv) / m_D_2(hn, hv)) / (2.*CSP::pi*k_23));
		Diff_T3 = abs_diffT3 / T_3;


	}

	//Warning of convergence failure
	//if(m_qq>99) {                           //End simulation if loop does not converge
	//    call messages(-1,"Trough Energy Balance Convergence m_Error 1",'WARNING',INFO(1),INFO(2))
	//    return
	//}
	//
	//if(T1_iter>99) {
	//    call messages(-1,"Trough Energy Balance Convergence m_Error 2",'WARNING',INFO(1),INFO(2))
	//    return
	//}
	//
	//if(q_conv_iter>99) {
	//    call messages(-1,"Trough Energy Balance Convergence m_Error 3",'WARNING',INFO(1),INFO(2))
	//    return
	//}
	//
	//if(q5_iter>99) {
	//    call messages(-1,"Trough Energy Balance Convergence m_Error 4",'WARNING',INFO(1),INFO(2))
	//    return
	//}

	//Calculate specific heat in kJ/kg
	c_1ave = cp_1 / 1000.;

	q_heatloss = q_34tot + q_cond_bracket + q_5solabs;   //[W/m]

	//Save temperatures
	m_T_save[1] = T_2;
	m_T_save[2] = T_3;
	m_T_save[3] = T_4;
	m_T_save[4] = T_5;

};


/*
#################################################################################################################
#################################################################################################################
#################################################################################################################


"******************************************************************************************************************************
FUNCTION Fq_12conv :  Convective heat transfer rate from the HTF to the inside of the receiver tube
******************************************************************************************************************************"
Author: R.E. Forristall (2003, EES)
Implemented and revised:  M.J. Wagner (10/2009)
Copyright:  National Renewable Energy Lab (Golden, CO) 2009
note:  This function was programmed and tested against the EES original.
Small variations in output are due to slightly different fluid
properties used in the two models.

Newton's Law of Cooling.

q' = h * D_i *  PI * (T_m - T_s)

h = Nu_Di * k / D_i

Where

q' = convection heat transfer rate per unit length [W/m]
h = convection heat transfer coefficient [W/m^2-k]
D_i = inside diameter of absorber pipe [m]
T_m = mean (bulk) temperature of HTF [C]
T_s = inside surface temperature of absorber pipe [C]
Nu_Di = Nusselt number based on inside diameter
k = conduction heat transfer coefficient of HTF [W/m-K]

The Nusselt number is estimated with the correlation developed by Gnielinski.

Nu# = (f / 8) * (Re_Di - 1000) * Pr / (1 + 12.7 * (f / 8)^(1/2) * (Pr^(2/3) -1))  * (Pr / Pr_w)^0.11
f = (1.82 * log10(Re_Di) - 1.64)^(-2)
Re_Di = Rho * v_m * Di / u
Pr  = Cp * u / k

Where

Nu# = Nusselt number
Re_Di = Reynolds number for internal pipe flow
Pr = Prandtl number
Pr_w = Prandtl number evaluated at the wall temperature
u = fluid absolute viscosity [kg/m-s]
Di = inside diameter [m]
Cp = fluid specific heat [J/kg-K]
k = fluid thermal conductivity [W/m-K]
Rho = fluid density [kg/m^3]
v_m = mean fluid velocity [m/s]

The above correlation is valid for 0.5 < Pr < 2000 and 2300< Re_Di < 5 * 10^6 and can be used for both uniform heat flux and uniform wall temperature cases. With the exception of Pr_w, all properties are evaluated at the mean fluid temperature.

If Re_D <= 2300 and the choice was made from the diagram window  to use the laminar flow model, one of  the following correlations is used.

for inner tube flow (uniform flux condition)
Nu# = 4.36

for inner annulus flow (uniform flux condition -- estimated from table for Nu# with heat fluxes at both surfaces)
m_D_p/m_D_2	Nu#
0		4.364
0.05		4.792
0.10		4.834
0.20		4.833
0.40		4.979
0.60		5.099
0.80		5.24
1.00		5.385


For the "SNL test platform" case the inside diameter in the above correlations is replaced with the following hydraulic diameter definition.

m_D_h = 4 * A_c / P = D_ao - D_ai

Where

m_D_h = hydraulic diameter [m]
A_c = flow cross sectional area [m^2]
P = wetted perimeter [m]
D_ai = inner annulus diameter [m]
D_ao = outer annulus diameter [m]

(Sources: Incropera, F., DeWitt, D., Fundamentals of Heat and Mass Transfer, Third Edition; John Wiley and Sons, New York, 1981, pp. 489-491, 502-503. Gnielinski, V., "New Equations for Heat and Mass Transfer in Turbulent Pipe and Channel Flow," International Chemical Engineering, Vol. 16, No. 2, April 1976.)
*/

double C_csp_trough_collector_receiver::fT_2(double q_12conv, double T_1, double T_2g, double m_v_1, int hn, int hv){
	//		convection 1->2,  HTF temp, guess T2,  fluid velocity, HCE #, HCE variant
	//     Input units (  K   ,  K ,  real,  m/s, - , -)

	double Cp_1, Cp_2, f, h_1, k_1, k_2, mu_1, mu_2, Nu_D2, Pr_1, Pr_2, Re_D2, rho_1, DRatio;
	bool includelaminar = true;	//cc -- this is always set to TRUE in TRNSYS

	T_2g = max(T_2g, m_T_htf_prop_min);		//[K]

	// Thermophysical properties for HTF 
	mu_1 = m_htfProps.visc(T_1);  //[kg/m-s]
	mu_2 = m_htfProps.visc(T_2g);  //[kg/m-s]
	Cp_1 = m_htfProps.Cp(T_1)*1000.;  //[J/kg-K]
	Cp_2 = m_htfProps.Cp(T_2g)*1000.;  //[J/kg-K]
	k_1 = max(m_htfProps.cond(T_1), 1.e-4);  //[W/m-K]
	k_2 = max(m_htfProps.cond(T_2g), 1.e-4);  //[W/m-K]
	rho_1 = m_htfProps.dens(T_1, 0.0);  //[kg/m^3]

	Pr_2 = (Cp_2 * mu_2) / k_2;
	Pr_1 = (Cp_1 * mu_1) / k_1;

	if (m_v_1 > 0.1) {

		Re_D2 = (rho_1 * m_D_h(hn, hv) * m_v_1) / (mu_1);

		// Nusselt Number for laminar flow case if option to include laminar flow model is chosen 
		if ((includelaminar == true) && (Re_D2 <= 2300.)) {
			if (m_Flow_type(hn, hv) == 2.0) {
				DRatio = m_D_p(hn, hv) / m_D_2(hn, hv);
				//Estimate for uniform heat flux case (poly. regression based on lookup table in Forristall EES model)
				//---Note that this regression is based on an 8-point table, and is highly non-practical outside of DRatio bounds
				//---0 and 1
				if (DRatio > 1.) {
					Nu_D2 = 5.385;
				}
				else if (DRatio < 0.) {
					Nu_D2 = 4.364;
				}
				else{
					Nu_D2 = 41.402*pow(DRatio, 5) - 109.702*pow(DRatio, 4) + 104.570*pow(DRatio, 3) - 42.979*pow(DRatio, 2) + 7.686*DRatio + 4.411;
				}
			}
			else{
				Nu_D2 = 4.36;				//uniform heat flux
			}
		}
		else{
			// Warning statements if turbulent/transitional flow Nusselt Number correlation is used out of recommended range 
			//		if (Pr_1 <= 0.5) or (2000 <= Pr_1) { CALL WARNING('The result may not be accurate, since 0.5 < Pr_1 < 2000 does not hold. See PROCEDURE Pq_12conv. Pr_1 = XXXA1', Pr_1)
			//		if (Pr_2 <= 0.5) or (2000 <= Pr_2) { CALL WARNING('The result may not be accurate, since 0.5 < Pr_2 < 2000 does not hold. See PROCEDURE Pq_12conv. Pr_2 = XXXA1', Pr_2)
			//		If ( Re_D2 <= (2300) ) or (5*10**6 <= Re_D2 ) Then CALL WARNING('The result may not be accurate, since 2300 < Re_D2 < (5 * 10**6) does not hold. See PROCEDURE Pq_12conv. Re_D2 = XXXA1', Re_D2)

			// Turbulent/transitional flow Nusselt Number correlation (modified Gnielinski correlation) 	
			f = pow(1.82 * log10(Re_D2) - 1.64, -2);
			Nu_D2 = (f / 8.) * (Re_D2 - 1000.) * Pr_1 / (1. + 12.7 * sqrt(f / 8.) * (pow(Pr_1, 0.6667) - 1.)) * pow(Pr_1 / Pr_2, 0.11);
		}

		h_1 = Nu_D2 * k_1 / m_D_h(hn, hv);  //[W/m**2-K]
		return T_1 + q_12conv / (h_1*m_D_2(hn, hv)*CSP::pi);
		//q_12conv = h_1 * m_D_2 * PI  * (T_2 - T_1ave)  //[W/m]
	}
	else{
		h_1 = 0.0001;
		return T_1;
	}

};



/******************************************************************************************************************************
FUNCTION fq_34conv :	Convective heat transfer rate between the absorber outer surface and the glazing inner surface
******************************************************************************************************************************"
NOTE: Temperatures input in terms of degrees K

Author: R.E. Forristall (2003, EES)
Implemented and revised:  M.J. Wagner (10/2009)
Copyright:  National Renewable Energy Lab (Golden, CO) 2009

{ Four cases:

1. Vacuum in annulus: free-molecular heat transfer model for an annulus.
2. Low or lost vacuum: natural convection heat transfer model for an annulus.
3. No glazing, no wind: natural convection heat transfer model for a horizontal cylinder.
4. No glazing, with wind: forced convection heat transfer model for a horizontal cylinder.


Case 1:

Free-molecular heat transfer for an annular space between horizontal cylinders.

q' = D_i * PI * h * (T_i - T_o)
h = k_gas / (D_i / 2 * ln(D_o / D_i) + b * Lambda * (D_i / D_o + 1))
b = (2 - a) / a * (9 * Gamma - 5) / (2 * (Gamma + 1))
Lambda = 2.331 * 10^(-20) * T_avg / (P * Delta^2)

Where

q' = convection heat transfer rate per unit length [W/m]
D_i = outer absorber diameter [m]
D_o = inner glazing diameter [m]
h = convection heat transfer coefficient for annulus gas [W/m^2-K]
T_i = outer absorber surface temperature [C]
T_o = inner glazing surface temperature [C]
k_gas = thermal conductivity of the annulus fluid at standard temperature and pressure [W/m^2-K]
b = interaction coefficient [dimensionless]
Lambda = mean-free-path between collisions of a molecule [cm]
a = accommodation coefficient [dimensionless]
Gamma = ratio of specific heats for the annulus fluid [dimensionless]
T_avg = average temperature of the annulus fluid [K]
P = pressure of the annulus gas [mm of Hg]
Delta = molecular diameter of the annulus gas [cm]

The above correlation is valid for Ra_Do < (D_o / (D_o -D_i))^4, but may over estimate q' slightly for large vacuums.

(Source: Ratzel, A., Hickox, C., Gartling, D., "Techniques for Reducing Thermal Conduction and Natural Convection Heat Losses
in Annular Receiver Geometries," Journal of Heat Transfer, Vol. 101, No. 1, February 1979; pp. 108-113)


Case 2:

Modified Raithby and Hollands correlation for natural convection in an annular space between horizontal cylinders.

q' = 2.425 * k * (T_i - T_o) / (1 + (D_i / D_o)^(3/5))^(5/4) * (Pr * Ra_Di / (0.861 + Pr))^(1/4)
Pr = NU / Alpha
Ra_Di = m_g * Beta * (T_i - T_o) * (D_i)^3 / (Alpha * NU)
Beta = 1 / T_avg		"Ideal Gas"

Where

k = conduction heat transfer coefficient for the annulus gas [W/m-K]
Pr = Prandtl number
NU = kinematic viscosity [m^2/s]
Alpha = thermal diffusivity [m^2/s]
Ra_Di = Rayleigh number based on the annulus inner diameter
m_g = local acceleration due to gravity [m/s^2]
Beta = volumetric thermal expansion coefficient [1/K]
Rho_o = annulus gas density at the outer surface [kg/m^3]
Rho_i = annulus gas density at the inner surface [kg/m^3]
T_avg = average temperature, (T_i + T_o) / 2 [K]

Above correlation is valid for Ra_Do > (D_o / (D_o -D_i))^4. All physical properties are evaluated at the average temperature, (T_i + T_o)/2.

(Source: Bejan, A., Convection Heat Transfer, Second Edition; John Wiley & Son's, New York, 1995, pp. 257-259.)


Case 3:

Churchill and Chu correlation for natural convection from a long isothermal horizontal cylinder.

Nu_bar = (0.60 + (0.387 * Ra_D^(1/6)) / (1 + (0.559 / Pr)^(9/16))^(8/27) )^2
Ra_D = m_g * Beta * (T_s - T_inf) * D^3 / (Alpha * NU)
Beta =  1 / T_f	"Ideal Gas"
Alpha = k / (Cp * Rho)
Pr = NU / Alpha

h = Nu_bar * k / D

q' = h * PI * D * (T_s - T_inf)

Where

Nu_bar = average Nusselt number
Ra_D = Rayleigh number based on diameter
Rho = fluid density  [kg/m^3]
Cp = specific heat at constant pressure [kJ / kg-K]
T_inf = fluid temperature in the free stream [C]
T_s = surface temperature [C]
T_f = film temperature, (T_s + T_inf) / 2 [K]
T_inf = ambient air temperature [C]

Above correlation is valid for  10^(-5) < Ra_D < 10^12. All physical properties are evaluated at the film temperature, (T_s + T_inf) / 2.

(Source: Incropera, F., DeWitt, D., Fundamentals of Heat and Mass Transfer, Third Edition; John Wiley and Sons, New York, 1981, pp. 550-552.)


Case 4:

Zhukauskas's correlation for external forced convection flow normal to an isothermal cylinder.

Nu_bar = C * Re_D^m * Pr^n * (Pr / Pr_s)^(1/4)

Re_D		C			m
1-40		0.75			0.4
40-1000		0.51			0.5
1e3- 2e5	0.26			0.6
2e5-1e6	0.076			0.7

n = 0.37, Pr <=10
n = 0.36, Pr >10

Re_D =  U_inf * D / NU
Pr  = NU / Alpha
Alpha = k / (Cp * Rho)

Q =  h * D * PI * (T_s - T_inf) * L

Where,

Re_D = Reynolds number evaluated at the diameter
Cp = specific heat at constant pressure of air [W/m-K]
Rho = density of air [kg/m^3]
C, m, n = constants

Above correlation is valid for  0.7 < Pr < 500, and 1 < Re_D < 10^6. All physical properties evaluated
at the free stream temperature, T_inf,  except Pr_s.

(Source: Incropera, F., DeWitt, D., Fundamentals of Heat and Mass Transfer, Third Edition; John Wiley and
Sons, New York, 1981, p. 413.)
}*/
//subroutine FQ_34CONV(T_3,T_4, m_D_3, m_D_4, m_P_a, P_6, v_6, T_6, annulusGas, glazingIntact, q_34conv, h_34)
void C_csp_trough_collector_receiver::FQ_34CONV(double T_3, double T_4, double P_6, double v_6, double T_6, int hn, int hv, double &q_34conv, double &h_34){
	//      UNITS   ( K , K ,  Pa , m/s,  K , -, -, W/m, W/m2-K)

	double a, Alpha_34, b, Beta_34, C, C1, Cp_34, Cv_34, Delta, Gamma, k_34, Lambda,
		m, mu_34, n, nu_34, P, Pr_34, P_A1, Ra_D3, Ra_D4, rho_34, T_34, T_36,
		grav, Nu_bar, rho_3, rho_6, mu_36, rho_36, cp_36,
		k_36, nu_36, alpha_36, beta_36, Pr_36, h_36, mu_3, mu_6, k_3, k_6, cp_3, Cp_6, nu_6, nu_3,
		Alpha_3, alpha_6, Re_D3, Pr_3, Pr_6, Natq_34conv, Kineticq_34conv;

	grav = 9.81; //m/s2  gravitation constant

	P_A1 = m_P_a(hn, hv) * 133.322368;  //convert("torr", "Pa")  //[Pa]

	T_34 = (T_3 + T_4) / 2.;  //[C]
	T_36 = (T_3 + T_6) / 2.;  //[C]

	if (!m_GlazingIntact(hn, hv)) {

		// Thermophysical Properties for air 
		rho_3 = m_airProps.dens(T_3, P_6);  //[kg/m**3], air is fluid 1.
		rho_6 = m_airProps.dens(T_6, P_6);  //[kg/m**3], air is fluid 1.

		if (v_6 <= 0.1) {
			mu_36 = m_airProps.visc(T_36);  //[N-s/m**2], AIR
			rho_36 = m_airProps.dens(T_36, P_6);  //[kg/m**3], AIR
			cp_36 = m_airProps.Cp(T_36)*1000.;  //[J/kg-K], AIR
			k_36 = m_airProps.cond(T_36);  //[W/m-K], AIR
			nu_36 = mu_36 / rho_36;  //[m**2/s] kinematic viscosity, AIR
			alpha_36 = k_36 / (cp_36 * rho_36);  //[m**2/s], thermal diffusivity, AIR
			beta_36 = 1.0 / T_36;  //[1/K]
			Ra_D3 = grav * beta_36 * fabs(T_3 - T_6) * pow(m_D_3(hn, hv), 3) / (alpha_36 * nu_36);

			// Warning Statement if following Nusselt Number correlation is used out of recommended range //
			//If ((Ra_D3 <= 1.e-5) || (Ra_D3 >= 1.e12)) continue
			//CALL WARNING('The result may not be accurate, since 10**(-5) < Ra_D3 < 10**12 does not hold. See Function fq_34conv. Ra_D3 = XXXA1', Ra_D3)

			// Churchill and Chu correlation for natural convection from a long isothermal horizontal cylinder //
			Pr_36 = nu_36 / alpha_36;
			Nu_bar = pow(0.60 + (0.387 * pow(Ra_D3, 0.1667)) / pow(1. + pow(0.559 / Pr_36, 0.5625), 0.2963), 2);
			h_36 = Nu_bar * k_36 / m_D_3(hn, hv);  //[W/m**2-K]//
			q_34conv = h_36 * CSP::pi * m_D_3(hn, hv) * (T_3 - T_6);  //[W/m]//
			h_34 = h_36;  //Set output coefficient
		}
		else {

			// Thermophysical Properties for air 
			mu_3 = m_airProps.visc(T_3);  //[N-s/m**2]
			mu_6 = m_airProps.visc(T_6);  //[N-s/m**2]
			k_3 = m_airProps.cond(T_3);  //[W/m-K]
			k_6 = m_airProps.cond(T_6);  //[W/m-K]
			cp_3 = m_airProps.Cp(T_3)*1000.;  //[J/kg-K]
			Cp_6 = m_airProps.Cp(T_6)*1000.;  //[J/kg-K]
			nu_6 = mu_6 / rho_6;  //[m**2/s]
			nu_3 = mu_3 / rho_3;  //[m**2/s]
			Alpha_3 = k_3 / (cp_3 * rho_3);  //[m**2/s]
			alpha_6 = k_6 / (Cp_6 * rho_6);  //[m**2/s]
			Re_D3 = v_6 * m_D_3(hn, hv) / nu_6;
			Pr_3 = nu_3 / Alpha_3;
			Pr_6 = nu_6 / alpha_6;

			// Warning Statements if following Nusselt Number correlation is used out of range //
			//if (Re_D3 <= 1) or (Re_D3 >= 10**6) { CALL WARNING('The result may not be accurate, since 1 < Re_D3 < 10**6 does not hold. See Function fq_34conv. Re_D3 = XXXA1', Re_D3)
			//If (Pr_6 <= 0.7) or (Pr_6 >= 500) Then CALL WARNING('The result may not be accurate, since 0.7 < Pr_6 < 500 does not hold. See Function fq_34conv. Pr_6 = XXXA1', Pr_6)

			// Coefficients for external forced convection Nusselt Number correlation (Zhukauskas's correlation) //
			if (Pr_6 <= 10) {
				n = 0.37;
			}
			else{
				n = 0.36;
			}

			if (Re_D3 < 40) {
				C = 0.75;
				m = 0.4;
			}
			else{

				if ((40 <= Re_D3) && (Re_D3 < 1000.)){
					C = 0.51;
					m = 0.5;
				}
				else{
					if ((1.e3 <= Re_D3) && (Re_D3 < 2.e5)) {
						C = 0.26;
						m = 0.6;
					}
					else{
						if ((2.e5 <= Re_D3) && (Re_D3 < 1.e6)) {
							C = 0.076;
							m = 0.7;
						}
					}
				}
			}

			// Zhukauskas's correlation for external forced convection flow normal to an isothermal cylinder 
			Nu_bar = C * pow(Re_D3, m)  * pow(Pr_6, n) * pow(Pr_6 / Pr_3, 0.25);
			h_36 = Nu_bar  *  k_6 / m_D_3(hn, hv);  //[W/m**2-K]
			q_34conv = h_36  *  m_D_3(hn, hv)  * CSP::pi *  (T_3 - T_6);  //[W/m]	
			h_34 = h_36;  //set output coefficient
		}
	}
	else {

		// Thermophysical Properties for gas in annulus space 
		mu_34 = m_AnnulusGasMat.at(hn, hv)->visc(T_34);  //[kg/m-s] 
		Cp_34 = m_AnnulusGasMat.at(hn, hv)->Cp(T_34)*1000.;  //[J/kg-K]
		Cv_34 = m_AnnulusGasMat.at(hn, hv)->Cv(T_34)*1000.;  //[J/kg-K]
		rho_34 = m_AnnulusGasMat.at(hn, hv)->dens(T_34, P_A1);  //[kg/m**3]
		k_34 = m_AnnulusGasMat.at(hn, hv)->cond(T_34);  //[W/m-K]

		// Modified Raithby and Hollands correlation for natural convection in an annular space between horizontal cylinders 
		Alpha_34 = k_34 / (Cp_34 * rho_34);  //[m**2/s]//
		nu_34 = mu_34 / rho_34;  //[m**2/s]//
		Beta_34 = 1. / max(T_34, 1.0);  //[1/K]//
		Ra_D3 = grav * Beta_34 * fabs(T_3 - T_4) * pow(m_D_3(hn, hv), 3) / (Alpha_34 * nu_34);
		Ra_D4 = grav * Beta_34 * fabs(T_3 - T_4) * pow(m_D_4(hn, hv), 3) / (Alpha_34 * nu_34);
		Pr_34 = nu_34 / Alpha_34;
		Natq_34conv = 2.425 * k_34 * (T_3 - T_4) / pow(1 + pow(m_D_3(hn, hv) / m_D_4(hn, hv), 0.6), 1.25) * pow(Pr_34 * Ra_D3 / (0.861 + Pr_34), 0.25);  //[W/m]//	
		P = m_P_a(hn, hv);  //[mmHg] (note that 1 torr = 1 mmHg by definition)
		C1 = 2.331e-20;  //[mmHg-cm**3/K]//

		// Free-molecular heat transfer for an annular space between horizontal cylinders 
		if (m_AnnulusGasMat.at(hn, hv)->GetFluid() == HTFProperties::Air) { //AIR
			Delta = 3.53e-8;  //[cm]
		}

		if (m_AnnulusGasMat.at(hn, hv)->GetFluid() == HTFProperties::Hydrogen_ideal){ //H2
			Delta = 2.4e-8;  //[cm]
		}

		if (m_AnnulusGasMat.at(hn, hv)->GetFluid() == HTFProperties::Argon_ideal){  //Argon
			Delta = 3.8e-8;  //[cm]
		}

		Lambda = C1 * T_34 / (P * Delta*Delta);  //[cm]
		Gamma = Cp_34 / Cv_34;
		a = 1.;
		b = (2. - a) / a * (9. * Gamma - 5.) / (2. * (Gamma + 1.));
		h_34 = k_34 / (m_D_3(hn, hv) / 2. * log(m_D_4(hn, hv) / m_D_3(hn, hv)) + b * Lambda / 100.* (m_D_3(hn, hv) / m_D_4(hn, hv) + 1.));  //[W/m**2-K]
		Kineticq_34conv = m_D_3(hn, hv) * CSP::pi * h_34 * (T_3 - T_4);  //[W/m]

		// Following compares free-molecular heat transfer with natural convection heat transfer and uses the largest value for heat transfer in annulus 
		if (Kineticq_34conv > Natq_34conv) {
			q_34conv = Kineticq_34conv;  //[W/m]
		}
		else{
			q_34conv = Natq_34conv;  //[W/m]
			h_34 = q_34conv / (m_D_3(hn, hv)*CSP::pi*(T_3 - T_4));  //Recalculate the convection coefficient for natural convection
		}
	}

};




/******************************************************************************************************************************
FUNCTION fq_34rad :	Radiation heat transfer rate between the absorber surface and glazing inner surface
******************************************************************************************************************************"
NOTE: Temperatures input in terms of degrees K

Author: R.E. Forristall (2003, EES)
Implemented and revised:  M.J. Wagner (10/2009)
Copyright:  National Renewable Energy Lab (Golden, CO) 2009
note  :  Tested against original EES version

{ 	Radiation heat transfer for a two-surface enclosure.

Two cases, one if the glazing envelope is intact and one if the glazing is missing or damaged.

Case 1: Long (infinite) concentric cylinders.

q' = sigma * PI * D_1 * (T_1^4 - T_2^4) / (1 / EPSILON_1 + (1 - EPSILON_2) / EPSILON_2 * (D_1 / m_D_2))

Where,

q' = radiation heat transfer per unit length [W/m]
sigma = Stephan-Boltzmann constant [W/m^2-K^4]
T_1 = absorber outer surface temperature [K]
T_2 = glazing inner surface temperature [K]
D_1 = outer absorber diameter [m]
m_D_2 = inner glazing diameter [m]
EPSILON_1 = emissivity of inner surface
EPSILON_2 = emissivity of outer surface

Case 2: Small convex object in a large cavity.

q' = sigma * PI * D_1 * EPSILON_1 * (T_1^4 - T_2^4)
}*/

void C_csp_trough_collector_receiver::FQ_34RAD(double T_3, double T_4, double T_7, double epsilon_3_v, int hn, int hv, double &q_34rad, double &h_34){
	//units		(K, K, K, -, -, -, W/m, W/m2-K)
	double sigma = 5.67e-8, T_ave;
	T_ave = (T_3 + T_4) / 2.;
	if (!m_GlazingIntact.at(hn, hv)) {
		q_34rad = epsilon_3_v * CSP::pi * m_D_3(hn, hv)  * sigma * (pow(T_3, 4) - pow(T_7, 4));  //[W/m]
		h_34 = q_34rad / (CSP::pi*m_D_3(hn, hv)*(T_3 - T_7));
	}
	else {
		h_34 = sigma*(T_3*T_3 + T_4*T_4)*(T_3 + T_4) / (1.0 / epsilon_3_v + m_D_3(hn, hv) / m_D_4(hn, hv) * (1.0 / m_EPSILON_4(hn, hv) - 1.0));
		q_34rad = CSP::pi* m_D_3(hn, hv) * h_34 * (T_3 - T_4);
	}

}


/******************************************************************************************************************************
FUNCTION fq_56conv :	Convective heat transfer rate between the glazing outer surface and the ambient air
******************************************************************************************************************************"
Author: R.E. Forristall (2003, EES)
Implemented and revised:  M.J. Wagner (10/2009)
Copyright:  National Renewable Energy Lab (Golden, CO) 2009
note  :  Tested against original EES version

{ 	h6	Heat Transfer Coefficient

If no wind, then the Churchill and Chu correlation is used. If wind, then the Zhukauskas's correlation is used. These correlations are described above for q_34conv.
}*/

void C_csp_trough_collector_receiver::FQ_56CONV(double T_5, double T_6, double P_6, double v_6, int hn, int hv, double &q_56conv, double &h_6)
//           units   ( K ,  K , torr, m/s,  W/m    , W/m2-K)
{
	double alpha_5, alpha_6, C, Cp_5, Cp_56, Cp_6, k_5, k_56, k_6, m, mu_5, mu_56, mu_6, n, Nus_6,
		nu_5, nu_6, Pr_5, Pr_6, Re_D5, rho_5, rho_56, rho_6, T_56, Nu_bar,
		nu_56, alpha_56, beta_56, Ra_D5, Pr_56;

	T_56 = (T_5 + T_6) / 2.0;  //[K]

	// Thermophysical Properties for air 
	mu_5 = m_airProps.visc(T_5);  //[kg/m-s]
	mu_6 = m_airProps.visc(T_6);  //[kg/m-s]
	mu_56 = m_airProps.visc(T_56);  //[kg/m-s]
	k_5 = m_airProps.cond(T_5);  //[W/m-K]
	k_6 = m_airProps.cond(T_6);  //[W/m-K]
	k_56 = m_airProps.cond(T_56);  //[W/m-K]
	Cp_5 = m_airProps.Cp(T_5)*1000.;  //[J/kg-K]
	Cp_6 = m_airProps.Cp(T_6)*1000.;  //[J/kg-K]
	Cp_56 = m_airProps.Cp(T_56)*1000.;  //[J/kg-K]
	rho_5 = m_airProps.dens(T_5, P_6);  //[kg/m^3]
	rho_6 = m_airProps.dens(T_6, P_6);  //[kg/m^3]
	rho_56 = m_airProps.dens(T_56, P_6);  //[kg/m^3]

	// if the glass envelope is missing then the convection heat transfer from the glass 
	//envelope is forced to zero by T_5 = T_6 
	if (!m_GlazingIntact(hn, hv)) {
		q_56conv = (T_5 - T_6);  //[W/m]
	}
	else{
		if (v_6 <= 0.1) {

			// Coefficients for Churchill and Chu natural convection correlation //
			nu_56 = mu_56 / rho_56;  //[m^2/s]
			alpha_56 = k_56 / (Cp_56 * rho_56);  //[m^2/s]
			beta_56 = 1.0 / T_56;  //[1/K]
			Ra_D5 = CSP::grav *beta_56 * fabs(T_5 - T_6) * pow(m_D_5(hn, hv), 3) / (alpha_56 * nu_56);

			// Warning Statement if following Nusselt Number correlation is used out of range //
			//If (Ra_D5 <= 10**(-5)) or (Ra_D5 >= 10**12) Then CALL WARNING('The result may not be accurate, 
			//since 10**(-5) < Ra_D5 < 10**12 does not hold. See Function fq_56conv. Ra_D5 = XXXA1', Ra_D5)

			// Churchill and Chu correlation for natural convection for a horizontal cylinder //
			Pr_56 = nu_56 / alpha_56;
			Nu_bar = pow(0.60 + (0.387 * pow(Ra_D5, 0.1667)) / pow(1.0 + pow(0.559 / Pr_56, 0.5625), 0.2963), 2);
			h_6 = Nu_bar * k_56 / m_D_5(hn, hv);  //[W/m**2-K]
			q_56conv = h_6 * CSP::pi * m_D_5(hn, hv) * (T_5 - T_6);  //[W/m]
		}
		else {

			// Coefficients for Zhukauskas's correlation //
			alpha_5 = k_5 / (Cp_5 * rho_5);  //[m**2/s]
			alpha_6 = k_6 / (Cp_6 * rho_6);  //[m**2/s]
			nu_5 = mu_5 / rho_5;  //[m**2/s]
			nu_6 = mu_6 / rho_6;  //[m**2/s]
			Pr_5 = nu_5 / alpha_5;
			Pr_6 = nu_6 / alpha_6;
			Re_D5 = v_6 * m_D_5(hn, hv) * rho_6 / mu_6;

			// Warning Statement if following Nusselt Number correlation is used out of range //
			//			if (Pr_6 <= 0.7) or (Pr_6 >= 500) { CALL WARNING('The result may not be accurate, since 0.7 < Pr_6 < 500 does not hold. See Function fq_56conv. Pr_6 = XXXA1', Pr_6)
			//			If (Re_D5 <= 1) or (Re_D5 >= 10**6) Then CALL WARNING('The result may not be accurate, since 1 < Re_D5 < 10**6 does not hold. See Function fq_56conv. Re_D5 = XXXA1 ', Re_D5)

			// Zhukauskas's correlation for forced convection over a long horizontal cylinder //
			if (Pr_6 <= 10) {
				n = 0.37;
			}
			else{
				n = 0.36;
			}

			if (Re_D5 < 40.0) {
				C = 0.75;
				m = 0.4;
			}
			else{
				if ((40.0 <= Re_D5) && (Re_D5 < 1.e3)) {
					C = 0.51;
					m = 0.5;
				}
				else{
					if ((1.e3 <= Re_D5) && (Re_D5 < 2.e5)) {
						C = 0.26;
						m = 0.6;
					}
					else{
						if ((2.e5 <= Re_D5) && (Re_D5 < 1.e6)) {
							C = 0.076;
							m = 0.7;
						}
					}
				}
			}

			Nus_6 = C * pow(Re_D5, m) *  pow(Pr_6, n)  * pow(Pr_6 / Pr_5, 0.25);
			h_6 = Nus_6 * k_6 / m_D_5(hn, hv);  //[W/m**2-K]
			q_56conv = h_6 * CSP::pi * m_D_5(hn, hv) * (T_5 - T_6);  //[W/m]
		}
	}
}




/******************************************************************************************************************************
FUNCTION fq_cond_bracket:	Heat loss estimate through HCE support bracket
******************************************************************************************************************************"
Author: R.E. Forristall (2003, EES)
Implemented and revised:  M.J. Wagner (10/2009)
Copyright:  National Renewable Energy Lab (Golden, CO) 2009
note  :  Tested against original EES version
*/
double C_csp_trough_collector_receiver::FQ_COND_BRACKET(double T_3, double T_6, double P_6, double v_6, int hn, int hv){
	//           units                    ( K ,  K , bar, m/s)

	double P_brac, D_brac, A_CS_brac, k_brac, T_base, T_brac, T_brac6, mu_brac6, rho_brac6,
		Cp_brac6, k_brac6, nu_brac6, Alpha_brac6, Beta_brac6, Ra_Dbrac, Pr_brac6, Nu_bar, h_brac6,
		mu_brac, mu_6, rho_6, rho_brac, k_6, Cp_brac, nu_6, Cp_6, Nu_brac, Alpha_brac,
		Re_Dbrac, Pr_brac, Pr_6, n, C, m, L_HCE, alpha_6;


	// effective bracket perimeter for convection heat transfer
	P_brac = 0.2032;  //[m]

	// effective bracket diameter (2 x 1in) 
	D_brac = 0.0508;  //[m]

	// minimum bracket cross-sectional area for conduction heat transfer
	A_CS_brac = 0.00016129;  //[m**2]

	// conduction coefficient for carbon steel at 600 K
	k_brac = 48.0;  //[W/m-K]

	// effective bracket base temperature
	T_base = T_3 - 10.0;  //[C]

	// estimate average bracket temperature 
	T_brac = (T_base + T_6) / 2.0;  //[C]  //NOTE: MJW modified from /3 to /2.. believed to be an error

	// estimate film temperature for support bracket 
	T_brac6 = (T_brac + T_6) / 2.0;  //[C]

	// convection coefficient with and without wind
	if (v_6 <= 0.1) {

		mu_brac6 = m_airProps.visc(T_brac6);  //[N-s/m**2]
		rho_brac6 = m_airProps.dens(T_brac6, P_6);  //[kg/m**3]
		Cp_brac6 = m_airProps.Cp(T_brac6)*1000.;  //[J/kg-K]
		k_brac6 = m_airProps.cond(T_brac6);  //[W/m-K]
		nu_brac6 = mu_brac6 / rho_brac6;  //[m**2/s]
		Alpha_brac6 = k_brac6 / (Cp_brac6 * rho_brac6);  //[m**2/s]
		Beta_brac6 = 1.0 / T_brac6;  //[1/K]
		Ra_Dbrac = CSP::grav * Beta_brac6 * fabs(T_brac - T_6) * D_brac*D_brac*D_brac / (Alpha_brac6 * nu_brac6);

		// Warning Statement if following Nusselt Number correlation is used out of recommended range 
		//If ((Ra_Dbrac <= 1.e-5)) || (Ra_Dbrac >= 1.e12) Then CALL WARNING('The result may not be accurate, 
		//since 10**(-5) < Ra_Dbrac < 10**12 does not hold. See Function fq_cond_bracket. Ra_Dbrac = XXXA1', Ra_Dbrac)

		// Churchill and Chu correlation for natural convection from a long isothermal horizontal cylinder 
		Pr_brac6 = nu_brac6 / Alpha_brac6;
		Nu_bar = pow(0.60 + (0.387 * pow(Ra_Dbrac, 0.1667)) / pow(1.0 + pow(0.559 / Pr_brac6, 0.5625), 0.2963), 2);
		h_brac6 = Nu_bar * k_brac6 / D_brac;  //[W/m**2-K]
	}
	else{

		// Thermophysical Properties for air 
		mu_brac = m_airProps.visc(T_brac);  //[N-s/m**2]
		mu_6 = m_airProps.visc(T_6);  //[N-s/m**2]
		rho_6 = m_airProps.dens(T_6, P_6);  //[kg/m**3]
		rho_brac = m_airProps.dens(T_brac, P_6);  //[kg/m**3]
		k_brac = m_airProps.cond(T_brac);  //[W/m-K]
		k_6 = m_airProps.cond(T_6);  //[W/m-K]
		k_brac6 = m_airProps.cond(T_brac6);  //[W/m-K]
		Cp_brac = m_airProps.Cp(T_brac)*1000.;  //[J/kg-K]
		Cp_6 = m_airProps.Cp(T_6)*1000.;  //[J/kg-K]
		nu_6 = mu_6 / rho_6;  //[m**2/s]
		Nu_brac = mu_brac / rho_brac;  //[m**2/s]

		Alpha_brac = k_brac / (Cp_brac * rho_brac);  //[m**2/s]	1.21.14 twn: Fixed unit conversion error
		alpha_6 = k_6 / (Cp_6 * rho_6);  //[m**2/s] 1.21.14 twn: Fixed unit conversion error
		Re_Dbrac = v_6 * D_brac / nu_6;
		Pr_brac = Nu_brac / Alpha_brac;
		Pr_6 = nu_6 / alpha_6;

		// Warning Statements if following Nusselt Correlation is used out of range 
		//		if (Re_Dbrac <= 1) or (Re_Dbrac >= 10**6) { CALL WARNING('The result may not be accurate, since 1 < Re_Dbrac < 10**6 does not hold. See Function fq_cond_bracket. Re_Dbrac = XXXA1', Re_Dbrac)
		//		If (Pr_6 <= 0.7) or (Pr_6 >= 500) Then CALL WARNING('The result may not be accurate, since 0.7 < Pr_6 < 500 does not hold. See Function fq_cond_bracket. Pr_6 = XXXA1', Pr_6)

		// Coefficients for external forced convection Nusselt Number correlation (Zhukauskas's correlation) 
		if (Pr_6 <= 10.) {
			n = 0.37;
		}
		else {
			n = 0.36;
		}

		if (Re_Dbrac < 40.) {
			C = 0.75;
			m = 0.4;
		}
		else {

			if ((40. <= Re_Dbrac) && (Re_Dbrac< 1.e3)) {
				C = 0.51;
				m = 0.5;
			}
			else {
				if ((1.e3 <= Re_Dbrac) && (Re_Dbrac < 2.e5)) {
					C = 0.26;
					m = 0.6;
				}
				else {
					if ((2.e5 <= Re_Dbrac) && (Re_Dbrac < 1.e6)) {
						C = 0.076;
						m = 0.7;
					}
				}
			}
		}

		// Zhukauskas's correlation for external forced convection flow normal to an isothermal cylinder 
		Nu_bar = C * pow(Re_Dbrac, m)  * pow(Pr_6, n) * pow(Pr_6 / Pr_brac, 0.25);
		h_brac6 = Nu_bar  *  k_brac6 / D_brac;  //[W/m**2-K]

	}

	// estimated conduction heat loss through HCE support brackets / HCE length 
	L_HCE = 4.06;  //[m]
	return sqrt(h_brac6 * P_brac * k_brac * A_CS_brac) * (T_base - T_6) / L_HCE;  //[W/m]

}



/******************************************************************************************************************************
Subroutine pOpticalEfficiency:  Optical Efficiencies based on HCE type
******************************************************************************************************************************"
Author: R.E. Forristall (2003, EES)
Implemented and revised:  M.J. Wagner (10/2009)
Copyright:  National Renewable Energy Lab (Golden, CO) 2009
note  :  Tested against original EES version

subroutine OpticalEfficiency(m_OptCharType,m_CollectorType, m_reflectivity, K, m_Shadowing, &
m_TrackingError, m_GeomEffects, m_Rho_mirror_clean, m_Dirt_mirror,m_Dirt_HCE, m_Error, m_ColOptEff)

implicit none

integer,intent(in):: m_OptCharType, m_CollectorType
real(8),intent(inout):: m_reflectivity, shadowing, trackingError, m_GeomEffects, m_Rho_mirror_clean, &
m_Dirt_mirror, m_Dirt_HCE, m_Error
real(8),intent(out):: m_ColOptEff
real(8):: K


//Various methods for characterizing the optical performance of collectors are anticipated.  Among these will be
//the traditional SAM method of specifying the various "derate" factors or selecting these from a library.  The
//other methods are likely going to involve calculation of an intercept factor based on input of surface character-
//istics.

select case(m_OptCharType)
case(1)  //The traditional method of entering derate factors that roll up into a single value:
If (m_CollectorType == 1) then   // 'User-Defined'
m_Shadowing = min(1.0,m_Shadowing)
m_TrackingError = min(1.0,m_TrackingError)
m_GeomEffects = min(1.0,m_GeomEffects)
m_Rho_mirror_clean = min(1.0,m_Rho_mirror_clean)
m_Dirt_mirror = min(1.0,m_Dirt_mirror)
m_Dirt_HCE = min(1.0,m_Dirt_HCE)
m_Error = min(1.0,m_Error)
}

If (m_CollectorType == 2) then    //'LS-2'
m_Shadowing = 0.974
m_TrackingError = 0.994
m_GeomEffects = 0.98
m_Rho_mirror_clean = 0.935
m_Dirt_mirror = min(1.0,m_reflectivity/m_Rho_mirror_clean)
m_Dirt_HCE = min(1.0,(1.0+ m_Dirt_mirror)/2.0)
m_Error = 0.96
}

If ((m_CollectorType == 3) || (m_CollectorType == 4)) then    //'LS-3' or 'IST'
m_Shadowing = 0.974
m_TrackingError = 0.994
m_GeomEffects = 0.98
m_Rho_mirror_clean = 0.935
m_Dirt_mirror = min(1.0,m_reflectivity/m_Rho_mirror_clean)
m_Dirt_HCE = min(1.0,(1.0+ m_Dirt_mirror)/2.0)
m_Error = 0.96
}

m_ColOptEff = m_Shadowing * m_TrackingError * m_GeomEffects * m_Rho_mirror_clean * m_Dirt_mirror * m_Dirt_HCE * m_Error * K

case(2:)
continue //reserve space for additional characterization methods here...
end select


end subroutine
*/



/******************************************************************************************************************************
FUNCTION fk_23:	Absorber conductance
******************************************************************************************************************************"
{ Based on linear fit of data from "Alloy Digest, Sourcebook, Stainless Steels"; ASM International, 2000.}
*/

double C_csp_trough_collector_receiver::FK_23(double T_2, double T_3, int hn, int hv)
{
	double T_23;

	//Absorber materials:
	// (1)   304L
	// (2)   216L
	// (3)   321H
	// (4)   B42 Copper Pipe

	T_23 = (T_2 + T_3) / 2. - 273.15;  //[C]
	return m_AbsorberPropMat(hn, hv)->cond(T_23);

}

/***************************************************************************************************
*********************************************************************
* PipeFlow_turbulent:                                               *
* This procedure calculates the average Nusselt number and friction *
* factor for turbulent flow in a pipe given Reynolds number (Re),   *
* Prandtl number (Pr), the pipe length diameter ratio (LoverD) and  *
* the relative roughness}                                           *
*********************************************************************
*/ /*
//subroutine PipeFlow(Re, Pr, LoverD,relRough, Nusselt, f)
void PipeFlow(double Re, double Pr, double LoverD, double relRough, double &Nusselt, double &f){


double f_fd,Nusselt_L, Gz, Gm, Nusselt_T, Nusselt_H,fR,X;

//Correlation for laminar flow.. Note that no transitional effects are considered
if (Re < 2300.) {
//This procedure calculates the average Nusselt number and friction factor for laminar flow in a pipe
//..given Reynolds number (Re), Prandtl number (Pr), the pipe length diameter ratio (LoverD)
//..and the relative roughness}
Gz=Re*Pr/LoverD;
X=LoverD/Re;
fR=3.44/sqrt(X)+(1.25/(4*X)+16-3.44/sqrt(X))/(1+0.00021*pow(X,-2));
f=4.*fR/Re;
//{f$='Shah' {Shah, R.K.  and London, A.L. "Laminar Flow Forced Convection in Ducts",
//..Academic Press, 1978 ,Eqn 192, p98}}
Gm=pow(Gz,1./3.);
Nusselt_T=3.66+((0.049+0.02/Pr)*pow(Gz,1.12))/(1+0.065*pow(Gz,0.7));
Nusselt_H=4.36+((0.1156 +0.08569 /pow(Pr,0.4))*Gz)/(1+0.1158*pow(Gz,0.6));
//{Nusselt$='Nellis and Klein fit to Hornbeck'  {Shah, R.K.  and London, A.L. "Laminar Flow Forced Convection in Ducts",
//..Academic Press, 1978 ,Tables  20 and 22}}
Nusselt = Nusselt_T;  //Constant temperature Nu is better approximation
}
else { //Correlation for turbulent flow
f_fd = pow(0.79*log(Re)-1.64, -2); //Petukhov, B.S., in Advances in Heat Transfer, Vol. 6, Irvine and Hartnett, Academic Press, 1970
Nusselt_L= ((f_fd/8.)*(Re-1000)*Pr)/(1.+12.7*sqrt(f_fd/8.)*(pow(Pr, 2/3.)-1.)); //Gnielinski, V.,, Int. Chem. Eng., 16, 359, 1976

if (relRough > 1e-5) {

//f=8.*((8./Re)**12+((2.457*log(1./((7./Re)**0.9+0.27*(RelRough))))**16+(37530./Re)**16)**(-1.5))**(1./12.)
//mjw 8.30.2010 :: not used

f_fd=pow(-2.*log10(2*relRough/7.4-5.02*log10(2*relRough/7.4+13/Re)/Re), -2);

Nusselt_L= ((f_fd/8.)*(Re-1000.)*Pr)/(1.+12.7*sqrt(f_fd/8.)*(pow(Pr, 2/3.)-1.)); //Gnielinski, V.,, Int. Chem. Eng., 16, 359, 1976}
}
f=f_fd*(1.+pow(1./LoverD, 0.7)); //account for developing flow
Nusselt= Nusselt_L*(1.+pow(1./LoverD, 0.7));  //account for developing flow
}

}  */

/*
***************************************************************************************************
Trough system piping loss model
***************************************************************************************************

This piping loss model is derived from the pressure drop calculations presented in the
following document:

Parabolic Trough Solar System Piping Model
Final Report May 13, 2002 � December 31, 2004

B. Kelly
Nexant, Inc. San Francisco, California

D. Kearney
Kearney & Associates
Vashon, Washington

Subcontract Report
NREL/SR-550-40165
July 2006

----------------------------
Note on use of this function
----------------------------
The function returns the pressure drop across a given length of pipe, and also accounts for
a variety of possible pressure-loss components. This function should be called multiple times -
once for each section under consideration.  For example, separate calls should be made for the
HCE pressure drop, the pressure drop in each section of the header in which flow/geometrical
conditions vary, the section of pipe leading to the header, and so on.

----------------------------
Inputs
----------------------------
No | Name         | Description                           | Units     |  Type
===================================================================================
1 | m_Fluid        | Number associated with fluid type     | none      | float
2 | m_dot        | Mass flow rate of the fluid           | kg/s      | float
3 | T            | m_Fluid temperature                     | K         | float
4 | P            | m_Fluid pressure                        | Pa        | float
5 | D            | Diameter of the contact surface       | m         | float
6 | m_Rough        | Pipe roughness                        | m         | float
7 | L_pipe       | Length of pipe for pressure drop      | m         | float
8 | Nexp         | Number of expansions                  | none      | float
9 | Ncon         | Number of contractions                | none      | float
10 | Nels         | Number of standard elbows             | none      | float
11 | Nelm         | Number of medium elbows               | none      | float
12 | Nell         | Number of long elbows                 | none      | float
13 | Ngav         | Number of gate valves                 | none      | float
14 | Nglv         | Number of globe valves                | none      | float
15 | Nchv         | Number of check valves                | none      | float
16 | Nlw          | Number of loop weldolets              | none      | float
17 | Nlcv         | Number of loop control valves         | none      | float
18 | Nbja         | Number of ball joint assemblies       | none      | float
===================================================================================
----------------------------
Outputs
----------------------------
1. PressureDrop  (Pa)
*/
double C_csp_trough_collector_receiver::PressureDrop(double m_dot, double T, double P, double D, double m_Rough, double L_pipe,
	double Nexp, double Ncon, double Nels, double Nelm, double Nell, double Ngav, double Nglv,
	double Nchv, double Nlw, double Nlcv, double Nbja){

	double rho, v_dot, mu, nu, u_fluid, Re, f, DP_pipe, DP_exp, DP_con, DP_els, DP_elm, DP_ell, DP_gav,
		DP_glv, DP_chv, DP_lw, DP_lcv, DP_bja, HL_pm;

	//Calculate fluid properties and characteristics
	rho = m_htfProps.dens(T, P);
	mu = m_htfProps.visc(T);
	nu = mu / rho;
	v_dot = m_dot / rho;   //fluid volumetric flow rate
	u_fluid = v_dot / (CSP::pi*(D / 2.)*(D / 2.));  //m_Fluid mean velocity

	//Dimensionless numbers
	Re = u_fluid*D / nu;
	//if(Re<2300.) then
	//    f = 64./max(Re,1.0)
	//else
	f = FricFactor(m_Rough / D, Re);
	if (f == 0) return std::numeric_limits<double>::quiet_NaN();
	//}

	//Calculation of pressure loss from pipe length
	HL_pm = f*u_fluid*u_fluid / (2.0*D*CSP::grav);
	DP_pipe = HL_pm*rho*CSP::grav*L_pipe;

	//Calculation of pressure loss from Fittings
	DP_exp = 0.25*rho*u_fluid*u_fluid*Nexp;
	DP_con = 0.25*rho*u_fluid*u_fluid*Ncon;
	DP_els = 0.9 * D / f * HL_pm * rho * CSP::grav * Nels;
	DP_elm = 0.75 * D / f * HL_pm * rho * CSP::grav * Nelm;
	DP_ell = 0.6 * D / f * HL_pm * rho * CSP::grav * Nell;
	DP_gav = 0.19 * D / f * HL_pm * rho * CSP::grav * Ngav;
	DP_glv = 10.0 * D / f * HL_pm * rho * CSP::grav * Nglv;
	DP_chv = 2.5 * D / f * HL_pm * rho * CSP::grav * Nchv;
	DP_lw = 1.8 * D / f * HL_pm * rho * CSP::grav * Nlw;
	DP_lcv = 10.0 * D / f * HL_pm * rho * CSP::grav * Nlcv;
	DP_bja = 8.69 * D / f * HL_pm * rho * CSP::grav * Nbja;

	return DP_pipe + DP_exp + DP_con + DP_els + DP_elm + DP_ell + DP_gav + DP_glv + DP_chv + DP_lw + DP_lcv + DP_bja;

}


/**************************************************************************************************
Friction factor (taken from Piping loss model)
***************************************************************************************************
Uses an iterative method to solve the implicit friction factor function.
For more on this method, refer to Fox, et al., 2006 Introduction to m_Fluid Mechanics.			 */

double C_csp_trough_collector_receiver::FricFactor(double m_Rough, double Reynold){

	double Test, TestOld, X, Xold, Slope;
	double Acc = .01; //0.0001
	int NumTries;

	if (Reynold < 2750.) {
		return 64. / max(Reynold, 1.0);
	}

	X = 33.33333;  //1. / 0.03
	TestOld = X + 2. * log10(m_Rough / 3.7 + 2.51 * X / Reynold);
	Xold = X;
	X = 28.5714;  //1. / (0.03 + 0.005)
	NumTries = 0;

	while (NumTries < 21){
		NumTries++;
		Test = X + 2 * log10(m_Rough / 3.7 + 2.51 * X / Reynold);
		if (fabs(Test - TestOld) <= Acc) {
			return 1. / (X * X);
		}

		Slope = (Test - TestOld) / (X - Xold);
		Xold = X;
		TestOld = Test;
		X = max((Slope * X - Test) / Slope, 1.e-5);
	}

	//call Messages(-1," Could not find friction factor solution",'Warning',0,250) 
	return 0;
}


/**************************************************************************************************
---------------------------------------------------------------------------------
--Inputs
* nhsec - [-] number of header sections
* m_nfsec - [-] number of field section
* m_nrunsec- [-] number of unique runner diameter sections
* rho   - [kg/m3] m_Fluid density
* V_max - [m/s] Maximum fluid velocity at design
* V_min - [m/s] Minimum fluid velocity at design
* m_dot - [kg/s] Mass flow rate at design
--Outputs
* m_D_hdr - [m] An ARRAY containing the header diameter for each loop section
* m_D_runner - [m] An ARRAY containing the diameter of the runner pipe sections
* summary - Address of string variable on which summary contents will be written.
---------------------------------------------------------------------------------			*/

void C_csp_trough_collector_receiver::header_design(int nhsec, int m_nfsec, int m_nrunsec, double rho, double V_max, double V_min, double m_dot,
	std::vector<double> &m_D_hdr, std::vector<double> &m_D_runner, std::string *summary){
	 
	summary = NULL;

	//resize the header matrices if they are incorrect
	//real(8),intent(out):: m_D_hdr(nhsec), m_D_runner(m_nrunsec)
	if (m_D_hdr.size() != nhsec) m_D_hdr.resize(nhsec);
	if (m_D_runner.size() != m_nrunsec) m_D_runner.resize(m_nrunsec);

	//----
	int nst, nend, nd;
	double m_dot_max, m_dot_min, m_dot_ts, m_dot_hdr, m_dot_2loops, m_dot_temp;

	for (int i = 0; i<nhsec; i++){ m_D_hdr[i] = 0.; }

	//mass flow to section is always half of total
	m_dot_ts = m_dot / 2.;
	//Mass flow into 1 header
	m_dot_hdr = 2.*m_dot_ts / (float(m_nfsec));
	//Mass flow into the 2 loops attached to a single header section
	m_dot_2loops = m_dot_hdr / float(nhsec);

	//Runner diameters
	//runner pipe needs some length to go from the power block to the headers
	m_D_runner.at(0) = pipe_sched(sqrt(4.*m_dot_ts / (rho*V_max*CSP::pi)));
	//other runner diameters
	m_dot_temp = m_dot_ts*(1. - float(m_nfsec % 4) / float(m_nfsec));  //mjw 5.4.11 Fix mass flow rate for m_nfsec/2==odd 
	if (m_nrunsec>1) {
		for (int i = 1; i<m_nrunsec; i++){
			m_D_runner[i] = pipe_sched(sqrt(4.*m_dot_temp / (rho*V_max*CSP::pi)));
			m_dot_temp = max(m_dot_temp - m_dot_hdr * 2, 0.0);
		}
	}

	//Calculate each section in the header
	nst = 0; nend = 0; nd = 0;
	m_dot_max = m_dot_hdr;
	for (int i = 0; i<nhsec; i++){
		if ((i == nst) && (nd <= 10)) {
			//If we've reached the point where a diameter adjustment must be made...
			//Also, limit the number of diameter reductions to 10

			nd++; //keep track of the total number of diameter sections
			//Calculate header diameter based on max velocity
			m_D_hdr[i] = pipe_sched(sqrt(4.*m_dot_max / (rho*V_max*CSP::pi)));
			//Determine the mass flow corresponding to the minimum velocity at design
			m_dot_min = rho*V_min*CSP::pi*m_D_hdr[i] * m_D_hdr[i] / 4.;
			//Determine the loop after which the current diameter calculation will no longer apply
			nend = (int)floor((m_dot_hdr - m_dot_min) / (m_dot_2loops));  //tn 4.12.11 ceiling->floor
			//The starting loop for the next diameter section starts after the calculated ending loop
			nst = nend;
			//Adjust the maximum required flow rate for the next diameter section based on the previous 
			//section's outlet conditions
			m_dot_max = max(m_dot_hdr - m_dot_2loops*float(nend), 0.0);
		}
		else{
			//If we haven't yet reached the point where the minimum flow condition is acheived, just
			//set the header diameter for this loop to be equal to the last diameter
			m_D_hdr[i] = m_D_hdr.at(i - 1);
		}
	}

	//Print the results to a string
	if (summary != NULL){
		summary->clear();
		char tstr[TSTRLEN];
		//Write runner diam
		MySnprintf(tstr, TSTRLEN,
			"Piping geometry file\n\nMaximum fluid velocity: %.2lf\nMinimum fluid velocity: %.2lf\n\n",
			V_max, V_min);
		summary->append(tstr);

		for (int i = 0; i<m_nrunsec; i++){
			MySnprintf(tstr, TSTRLEN, "To section %d header pipe diameter: %.4lf m (%.2lf in)\n", i + 1, m_D_runner[i], m_D_runner[i] * m_mtoinch);
			summary->append(tstr);
		}
		//Write header diams
		summary->append("Loop No. | Diameter [m] | Diameter [in] | Diam. ID\n--------------------------------------------------\n");

		nd = 1;
		for (int i = 0; i<nhsec; i++){
			if (i>1) {
				if (m_D_hdr[i] != m_D_hdr.at(i - 1)) nd = nd + 1;
			}
			MySnprintf(tstr, TSTRLEN, "  %4d   |    %6.4lf    |    %6.4lf     | %3d\n", i + 1, m_D_hdr[i], m_D_hdr[i] * m_mtoinch, nd);
			summary->append(tstr);
		}
		//110 format(2X,I4,3X,"|",4X,F6.4,4X,"|",4X,F6.3,5X,"|",1X,I3)
	}

}

/***************************************************************************************************
This function takes a piping diameter "De" [m] and locates the appropriate pipe schedule
from a list of common pipe sizes. The function always returns the pipe schedule equal to or
immediately larger than the ideal diameter De.
The pipe sizes are selected based on the assumption of a maximum hoop stress of 105 MPa and a total
solar field pressure drop of 20 Bar. The sizes correspond to the pipe schedule with a wall thickness
sufficient to match these conditions. For very large pipe diameters (above 42in), no suitable schedule
was found, so the largest available schedule is applied.
Data and stress calculations were obtained from Kelly & Kearney piping model, rev. 1/2011.
*/

double C_csp_trough_collector_receiver::pipe_sched(double De) {

	int np = 25;

	//D_inch = (/2.50, 3.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 22.0, 24.0, 26.0, &
	//           28.0, 30.0, 32.0, 34.0, 36.0, 42.0, 48.0, 54.0, 60.0, 66.0, 72.0/)

	double D_m[] = { 0.06880860, 0.08468360, 0.1082040, 0.16146780, 0.2063750, 0.260350, 0.311150, 0.33975040,
		0.39055040, 0.438150, 0.488950, 0.53340, 0.58420, 0.6350, 0.679450, 0.730250, 0.781050,
		0.82864960, 0.87630, 1.02870, 1.16840, 1.32080, 1.47320, 1.62560, 1.7780 };

	//Select the smallest pipe schedule above the diameter provided
	for (int i = 0; i<np; i++){
		if (D_m[i] >= De) return D_m[i];
	}
	//Nothing was found, so return an error

	m_error_msg = util::format("No suitable pipe schedule found for this plant design. Looking for a schedule above %.2f in ID. "
		"Maximum schedule is %.2f in ID. Using the exact pipe diameter instead."
		"Consider increasing the header design velocity range or the number of field subsections.",
		De*m_mtoinch, D_m[np - 1] * m_mtoinch);
	mc_csp_messages.add_message(C_csp_messages::WARNING, m_error_msg);

	//message(TCS_WARNING, "No suitable pipe schedule found for this plant design. Looking for a schedule above %.2f in ID. "
	//	"Maximum schedule is %.2f in ID. Using the exact pipe diameter instead."
	//	"Consider increasing the header design velocity range or the number of field subsections.",
	//	De*m_mtoinch, D_m[np - 1] * m_mtoinch);
	return De;  //mjw 10/10/2014 - NO! ---> std::numeric_limits<double>::quiet_NaN();
}

//***************************************************************************************************
double C_csp_trough_collector_receiver::Pump_SGS(double rho, double m_dotsf, double sm){

	int nl = 8;
	double v_dotpb, v_dotsf, m_dotpb, vel_max;
	double
		*V_dot = new double[nl],
		*D = new double[nl],
		*V = new double[nl];

	//Line no.	
	//1	Expansion vessel or thermal storage tank to pump suction header
	//2	Individual pump suction line, from suction header to pump inlet
	//3	Individual pump discharge line, from pump discharge to discharge header
	//4	Pump discharge header
	//5	Collector field outlet header to expansion vessel or thermal storage tank
	//6	Steam generator supply header
	//7	Inter steam generator piping
	//8	Steam generator exit header to expansion vessel or thermal storage
	//Assume standard lengths for each line [m] (Kelly & Kearney)
	//Assume 3 pumps at 50% each. #3) 3*30. 
	double L_line[] = { 0.0, 0.0, 90.0, 100.0, 120.0, 80.0, 120.0, 80.0 };

	//Assume a maximum HTF velocity of 1.85 m/s (based on average from Kelly & Kearney model
	vel_max = 1.85;

	//design-point vol. flow rate m3/s
	m_dotpb = m_dotsf / sm;
	v_dotpb = m_dotpb / rho;
	v_dotsf = m_dotsf / rho;

	//Set the volumetric flow rate for each line.
	V_dot[0] = v_dotsf;
	V_dot[1] = v_dotsf / 2.0;
	V_dot[2] = V_dot[1];
	V_dot[3] = v_dotsf;
	V_dot[4] = V_dot[3];
	V_dot[5] = v_dotpb;
	V_dot[6] = V_dot[5];
	V_dot[7] = V_dot[5];

	//for each line..
	double psum = 0.;
	for (int i = 0; i<nl; i++){
		//Calculate the pipe diameter
		D[i] = pipe_sched(sqrt(4.0*V_dot[i] / (vel_max*CSP::pi)));
		//Calculate the total volume
		V[i] = pow(D[i], 2) / 4.0*CSP::pi*L_line[i];
		psum += V[i];
	}

	delete[] V_dot;
	delete[] D;
	delete[] V;

	return psum;

}

//***************************************************************************************************





