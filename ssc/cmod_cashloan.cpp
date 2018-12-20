/*******************************************************************************************************
*  Copyright 2017 Alliance for Sustainable Energy, LLC
*
*  NOTICE: This software was developed at least in part by Alliance for Sustainable Energy, LLC
*  (�Alliance�) under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
*  The Government retains for itself and others acting on its behalf a nonexclusive, paid-up,
*  irrevocable worldwide license in the software to reproduce, prepare derivative works, distribute
*  copies to the public, perform publicly and display publicly, and to permit others to do so.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted
*  provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer in the documentation and/or
*  other materials provided with the distribution.
*
*  3. The entire corresponding source code of any redistribution, with or without modification, by a
*  research entity, including but not limited to any contracting manager/operator of a United States
*  National Laboratory, any institution of higher learning, and any non-profit organization, must be
*  made publicly available under this license for as long as the redistribution is made available by
*  the research entity.
*
*  4. Redistribution of this software, without modification, must refer to the software by the same
*  designation. Redistribution of a modified version of this software (i) may not refer to the modified
*  version by the same designation, or by any confusingly similar designation, and (ii) must refer to
*  the underlying software originally provided by Alliance as �System Advisor Model� or �SAM�. Except
*  to comply with the foregoing, the terms �System Advisor Model�, �SAM�, or any confusingly similar
*  designation may not be used to refer to any modified version of this software or any modified
*  version of the underlying software originally provided by Alliance without the prior written consent
*  of Alliance.
*
*  5. The name of the copyright holder, contributors, the United States Government, the United States
*  Department of Energy, or any of their employees may not be used to endorse or promote products
*  derived from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER,
*  CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR
*  EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
*  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************************************/
#include <sstream>
#include <vector>

#include "core.h"
#include "lib_financial.h"
#include "lib_util.h"
#include "common_financial.h"
using namespace libfin;

static var_info vtab_cashloan[] = {
/*   VARTYPE           DATATYPE          NAME                        LABEL                                  UNITS         META                      GROUP            REQUIRED_IF                 CONSTRAINTS                      UI_HINTS*/

	{ SSC_INPUT,        SSC_NUMBER,		 "market",                   "Residential or Commercial Market",   "0/1",          "0=residential,1=comm.", "Cashloan",      "?=1",                     "INTEGER,MIN=0,MAX=1",            "" },
	{ SSC_INPUT, SSC_NUMBER, "mortgage", "Use mortgage style loan (res. only)", "0/1", "0=standard loan,1=mortgage", "Cashloan", "?=0", "INTEGER,MIN=0,MAX=1", "" },

	{ SSC_INPUT,        SSC_NUMBER,      "total_installed_cost",                          "Total installed cost",                               "$",      "",                      "Cashloan",            "*",                      "MIN=0",                                         "" },
	{ SSC_INPUT,        SSC_NUMBER,      "salvage_percentage",                       "Salvage value percentage",                        "%",      "",                      "Cashloan",      "?=0.0",                  "MIN=0,MAX=100",                 "" },
	
	{ SSC_INPUT,        SSC_ARRAY,       "annual_energy_value",             "Energy value",                       "$",            "",                      "Cashloan",      "*",                       "",                                         "" },
	{ SSC_INPUT,        SSC_ARRAY,       "annual_themal_value",             "Energy value",                       "$",            "",                      "Cashloan",      "",                       "",                                         "" },
	{ SSC_INPUT, SSC_ARRAY, "gen", "Power generated by renewable resource", "kW", "", "", "*", "", "" },
	{ SSC_INPUT, SSC_ARRAY, "degradation", "Annual degradation", "%", "", "AnnualOutput", "*", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "system_use_lifetime_output", "Lifetime hourly system outputs", "0/1", "0=hourly first year,1=hourly lifetime", "AnnualOutput", "*", "INTEGER,MIN=0", "" },

	/* financial outputs */
	{ SSC_OUTPUT,        SSC_NUMBER,     "cf_length",                "Number of periods in cash flow",      "",             "",                      "Cash Flow",      "*",                       "INTEGER",                                  "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "lcoe_real",                "Real LCOE",                          "cents/kWh",    "",                      "Cash Flow",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "lcoe_nom",                 "Nominal LCOE",                       "cents/kWh",    "",                      "Cash Flow",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "payback",                  "Payback period",                            "years",        "",                      "Cash Flow",      "*",                       "",                                         "" },
	// added 9/26/16 for Owen Zinaman Mexico
	{ SSC_OUTPUT, SSC_NUMBER, "discounted_payback", "Discounted payback period", "years", "", "Cash Flow", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "npv", "Net present value", "$", "", "Cash Flow", "*", "", "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "present_value_oandm",                      "Present value of O&M expenses",				   "$",            "",                      "Financial Metrics",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "present_value_oandm_nonfuel",              "Present value of non-fuel O&M expenses",				   "$",            "",                      "Financial Metrics",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "present_value_fuel",                      "Present value of fuel expenses",				   "$",            "",                      "Financial Metrics",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "present_value_insandproptax",             "Present value of insurance and property tax",				   "$",            "",                      "Financial Metrics",      "*",                       "",                                         "" },

	{ SSC_OUTPUT, SSC_NUMBER, "adjusted_installed_cost", "Net capital cost", "$", "", "Financial Metrics", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "loan_amount", "Debt", "$", "", "Financial Metrics", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "first_cost", "Equity", "$", "", "Financial Metrics", "*", "", "" },
    { SSC_OUTPUT, SSC_NUMBER, "total_cost", "Total installed cost", "$", "", "Financial Metrics", "*", "", "" },
		

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_energy_net",      "Energy",                  "kWh",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_energy_value",      "Value of electricity savings",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_thermal_value",      "Value of thermal savings",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	// real estate value added 6/24/13
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_value_added",      "Real estate value added",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_fixed_expense",      "O&M fixed expense",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_production_expense", "O&M production-based expense",       "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_capacity_expense",   "O&M capacity-based expense",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_fixed1_expense",      "Battery fixed expense",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_production1_expense", "Battery production-based expense",       "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_capacity1_expense",   "Battery capacity-based expense",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_fixed2_expense",      "Fuel cell fixed expense",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_production2_expense", "Fuel cell production-based expense",       "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_capacity2_expense",   "Fuel cell capacity-based expense",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },


	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_fuel_expense",       "O&M fuel expense",                   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_opt_fuel_1_expense",       "O&M biomass feedstock expense",                   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_om_opt_fuel_2_expense",       "O&M coal feedstock expense",                   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },


	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_property_tax_assessed_value","Property tax net assessed value", "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_property_tax_expense",  "Property tax expense",               "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_insurance_expense",     "Insurance expense",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_net_salvage_value",     "Net salvage value",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_operating_expenses",    "Total operating expense",            "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_deductible_expenses",   "Deductible expenses",                "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
		
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_debt_balance",          "Debt balance",                       "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_debt_payment_interest", "Interest payment",                   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_debt_payment_principal","Principal payment",                  "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_debt_payment_total",    "Total P&I debt payment",             "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	
	{ SSC_OUTPUT,        SSC_NUMBER,     "ibi_total_fed",             "Federal IBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "ibi_total_sta",             "State IBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "ibi_total_uti",             "Utility IBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "ibi_total_oth",             "Other IBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "ibi_total",             "Total IBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	
	{ SSC_OUTPUT,        SSC_NUMBER,     "cbi_total_fed",             "Federal CBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "cbi_total_sta",             "State CBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "cbi_total_uti",             "Utility CBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "cbi_total_oth",             "Other CBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "cbi_total",             "Total CBI income",         "$",            "",                      "Cash Flow",      "*",                     "",                "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_pbi_total_fed",             "Federal PBI income",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_pbi_total_sta",             "State PBI income",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_pbi_total_uti",             "Utility PBI income",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_pbi_total_oth",             "Other PBI income",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_pbi_total",             "Total PBI income",         "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_ptc_fed",               "Federal PTC",                 "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_ptc_sta",               "State PTC",                   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_ptc_total",               "Total PTC",                   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT,        SSC_NUMBER,      "itc_total_fed",         "Federal ITC",                 "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,      "itc_total_sta",         "State ITC",                   "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,      "itc_total",         "Total ITC",                   "$",            "",                      "Cash Flow",      "*",                     "",                "" },
	
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_sta_depr_sched",                        "State depreciation schedule",              "%",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_sta_depreciation",                      "State depreciation",                       "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_sta_incentive_income_less_deductions",  "State incentive income less deductions",   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_sta_taxable_income_less_deductions",    "State taxable income less deductions",     "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_sta_tax_savings",                       "State tax savings",                        "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_fed_depr_sched",                        "Federal depreciation schedule",            "%",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_fed_depreciation",                      "Federal depreciation",                     "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_fed_incentive_income_less_deductions",  "Federal incentive income less deductions", "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_fed_taxable_income_less_deductions",    "Federal taxable income less deductions",   "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_fed_tax_savings",                       "Federal tax savings",                      "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_sta_and_fed_tax_savings",               "Total tax savings (federal and state)",      "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_after_tax_net_equity_cost_flow",        "After-tax annual costs",           "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_after_tax_cash_flow",                   "After-tax cash flow",                      "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },

	{ SSC_OUTPUT, SSC_ARRAY, "cf_discounted_costs", "Discounted costs", "$", "", "Cash Flow", "*", "LENGTH_EQUAL=cf_length", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "cf_discounted_savings", "Discounted savings", "$", "", "Cash Flow", "*", "LENGTH_EQUAL=cf_length", "" },

	{ SSC_OUTPUT, SSC_ARRAY, "cf_discounted_payback", "Discounted payback", "$", "", "Cash Flow", "*", "LENGTH_EQUAL=cf_length", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "cf_discounted_cumulative_payback", "Cumulative discounted payback", "$", "", "Cash Flow", "*", "LENGTH_EQUAL=cf_length", "" },



	{ SSC_OUTPUT, SSC_ARRAY, "cf_payback_with_expenses", "Simple payback with expenses", "$", "", "Cash Flow", "*", "LENGTH_EQUAL=cf_length", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "cf_cumulative_payback_with_expenses", "Cumulative simple payback with expenses", "$", "", "Cash Flow", "*", "LENGTH_EQUAL=cf_length", "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_payback_without_expenses",              "Simple payback without expenses",                 "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_cumulative_payback_without_expenses",   "Cumulative simple payback without expenses",      "$",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	


	{ SSC_OUTPUT,        SSC_NUMBER,     "lcoptc_fed_real",                "Levelized federal PTC (real)",                          "cents/kWh",    "",                      "Financial Metrics",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "lcoptc_fed_nom",                 "Levelized federal PTC (nominal)",                       "cents/kWh",    "",                      "Financial Metrics",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "lcoptc_sta_real",                "Levelized state PTC (real)",                          "cents/kWh",    "",                      "Financial Metrics",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "lcoptc_sta_nom",                 "Levelized state PTC (nominal)",                       "cents/kWh",    "",                      "Financial Metrics",      "*",                       "",                                         "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "wacc",                "Weighted average cost of capital (WACC)",                          "",    "",                      "Financial Metrics",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "effective_tax_rate",                 "Effective tax rate",                       "%",    "",                      "Financial Metrics",      "*",                       "",                                         "" },

// NTE additions 8/10/17
	{ SSC_INPUT,        SSC_ARRAY,       "elec_cost_with_system",             "Energy value",                       "$",            "",                      "thirdpartyownership",      "*",                       "",                                         "" },
	{ SSC_INPUT,        SSC_ARRAY,       "elec_cost_without_system",             "Energy value",                       "$",            "",                      "thirdpartyownership",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "cf_nte",      "Not to exceed (NTE)",         "cents/kWh",            "",                      "Cash Flow",      "*",                     "LENGTH_EQUAL=cf_length",                "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "year1_nte",                "Year 1 NTE",                          "cents/kWh",    "",                      "Cash Flow",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "lnte_real",                "Real LNTE",                          "cents/kWh",    "",                      "Cash Flow",      "*",                       "",                                         "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "lnte_nom",                 "Nominal LNTE",                       "cents/kWh",    "",                      "Cash Flow",      "*",                       "",                                         "" },


var_info_invalid };

extern var_info
	vtab_standard_financial[],
	vtab_standard_loan[],
	vtab_oandm[],
	vtab_depreciation[],
	vtab_battery_replacement_cost[],
	vtab_fuelcell_replacement_cost[],
	vtab_tax_credits[],
	vtab_payment_incentives[];

enum {
	CF_degradation,
	CF_energy_net,
	CF_energy_value,
	CF_thermal_value,
	CF_value_added,

	CF_om_fixed_expense,
	CF_om_production_expense,
	CF_om_capacity_expense,
	CF_om_fixed1_expense,
	CF_om_production1_expense,
	CF_om_capacity1_expense,
	CF_om_fixed2_expense,
	CF_om_production2_expense,
	CF_om_capacity2_expense,
	CF_om_fuel_expense,

	CF_om_opt_fuel_2_expense,
	CF_om_opt_fuel_1_expense,

	CF_federal_tax_frac,
	CF_state_tax_frac,
	CF_effective_tax_frac,

	CF_property_tax_assessed_value,
	CF_property_tax_expense,
	CF_insurance_expense,
	CF_net_salvage_value,
	CF_operating_expenses,

	CF_deductible_expenses,

	CF_debt_balance,
	CF_debt_payment_interest,
	CF_debt_payment_principal,
	CF_debt_payment_total,
	
	CF_pbi_fed,
	CF_pbi_sta,
	CF_pbi_uti,
	CF_pbi_oth,
	CF_pbi_total,
	
	CF_ptc_fed,
	CF_ptc_sta,
	CF_ptc_total,
	
	CF_sta_depr_sched,
	CF_sta_depreciation,
	CF_sta_incentive_income_less_deductions,
	CF_sta_taxable_income_less_deductions,
	CF_sta_tax_savings,
	
	CF_fed_depr_sched,
	CF_fed_depreciation,
	CF_fed_incentive_income_less_deductions,
	CF_fed_taxable_income_less_deductions,
	CF_fed_tax_savings,

	CF_sta_and_fed_tax_savings,
	CF_after_tax_net_equity_cost_flow,
	CF_after_tax_cash_flow,

	CF_payback_with_expenses,
	CF_cumulative_payback_with_expenses,

	// Added for Owen Zinaman for Mexico Rates and analyses 9/26/16
	//- see C:\Projects\SAM\Documentation\Payback\DiscountedPayback_2016.9.26
	CF_discounted_costs,
	CF_discounted_savings,
	CF_discounted_payback,
	CF_discounted_cumulative_payback,

	
	CF_payback_without_expenses,
	CF_cumulative_payback_without_expenses,

	CF_battery_replacement_cost_schedule,
	CF_battery_replacement_cost,

	CF_fuelcell_replacement_cost_schedule,
	CF_fuelcell_replacement_cost,

	CF_nte,

	CF_max };




class cm_cashloan : public compute_module
{
private:
	util::matrix_t<double> cf;
	double ibi_fed_amount;
	double ibi_sta_amount;
	double ibi_uti_amount;
	double ibi_oth_amount;
	double ibi_fed_per;
	double ibi_sta_per;
	double ibi_uti_per;
	double ibi_oth_per;
	double cbi_fed_amount;
	double cbi_sta_amount;
	double cbi_uti_amount;
	double cbi_oth_amount;

	hourly_energy_calculation hourly_energy_calcs;


public:
	cm_cashloan()
	{
		add_var_info( vtab_standard_financial );
		add_var_info( vtab_standard_loan );
		add_var_info( vtab_oandm );
		add_var_info( vtab_depreciation );
		add_var_info( vtab_tax_credits );
		add_var_info( vtab_payment_incentives );
		add_var_info(vtab_battery_replacement_cost);
		add_var_info(vtab_fuelcell_replacement_cost);
		add_var_info(vtab_cashloan);
	}

	void exec( ) throw( general_error )
	{
		int i;

		bool is_commercial = (as_integer("market")==1);
		bool is_mortgage = (as_integer("mortgage")==1);


//		throw exec_error("cmod_cashloan", "mortgage = " + util::to_string(as_integer("mortgage")));
//		if (is_commercial) log("commercial market"); else log("residential market");
//		if (is_mortgage) log("mortgage loan"); else log("standard loan");
		

		int nyears = as_integer("analysis_period");

		// initialize cashflow matrix
		cf.resize_fill( CF_max, nyears+1, 0.0 );

		// initialize energy and revenue
		size_t count = 0;
		ssc_number_t *arrp = 0;
		

		// degradation
		// degradation starts in year 2 for single value degradation - no degradation in year 1 - degradation =1.0
		// lifetime degradation applied in technology compute modules
		if (as_integer("system_use_lifetime_output") == 1)
		{
			for (i = 1; i <= nyears; i++) cf.at(CF_degradation, i) = 1.0;
		}
		else
		{
			size_t count_degrad = 0;
			ssc_number_t *degrad = 0;
			degrad = as_array("degradation", &count_degrad);

			if (count_degrad == 1)
			{
				for (i = 1; i <= nyears; i++) cf.at(CF_degradation, i) = pow((1.0 - degrad[0] / 100.0), i - 1);
			}
			else if (count_degrad > 0)
			{
				for (i = 0; i < nyears && i < (int)count_degrad; i++) cf.at(CF_degradation, i + 1) = (1.0 - degrad[i] / 100.0);
			}
		}

		// energy

		hourly_energy_calcs.calculate(this);

		if (as_integer("system_use_lifetime_output")==0)
		{
			double first_year_energy = 0.0;
			for (int h = 0; h < 8760; h++) 
				first_year_energy += hourly_energy_calcs.hourly_energy()[h];
			for (int y = 1; y <= nyears; y++)
				cf.at(CF_energy_net, y) = first_year_energy * cf.at(CF_degradation, y);
		}
		else
		{
			for (int y = 1; y <= nyears; y++)
			{
				cf.at(CF_energy_net, y) = 0;
				int ind = 0;
				for (int m = 0; m<12; m++)
					for (int d = 0; d<util::nday[m]; d++)
						for (int h = 0; h<24; h++)
							if (ind<8760)
							{
					cf.at(CF_energy_net, y) += hourly_energy_calcs.hourly_energy()[(y - 1) * 8760 + ind] * cf.at(CF_degradation, y);
								ind++;
							}
			}

		}

		if (is_assigned("annual_thermal_value"))
		{
			arrp = as_array("annual_thermal_value", &count);
			i = 0;
			while (i < nyears && i < (int)count)
			{
				cf.at(CF_thermal_value, i + 1) = (double)arrp[i +1];
				i++;
			}
		}


		arrp = as_array("annual_energy_value", &count);
		i=0;
		while ( i < nyears && i < (int)count )
		{
			cf.at(CF_energy_value, i+1) = (double) arrp[i+1];
			i++;
		}
		
		double year1_fuel_use = as_double("annual_fuel_usage"); // kWht
    	double nameplate = as_double("system_capacity"); // kW
		
		double inflation_rate = as_double("inflation_rate")*0.01;
		double property_tax = as_double("property_tax_rate")*0.01;
		double property_tax_decline_percentage = as_double("prop_tax_assessed_decline");
		double insurance_rate = as_double("insurance_rate")*0.01;
		double salvage_frac = as_double("salvage_percentage")*0.01;

		//double federal_tax_rate = as_double("federal_tax_rate")*0.01;
		//double state_tax_rate = as_double("state_tax_rate")*0.01;
		//double effective_tax_rate = state_tax_rate + (1.0-state_tax_rate)*federal_tax_rate;
		arrp = as_array("federal_tax_rate", &count);
		if (count > 0)
		{
			if (count == 1) // single value input
			{
				for (i = 0; i < nyears; i++)
					cf.at(CF_federal_tax_frac, i + 1) = arrp[0]*0.01;
			}
			else // schedule
			{
				for (i = 0; i < nyears && i < (int)count; i++)
					cf.at(CF_federal_tax_frac, i + 1) = arrp[i] * 0.01;
			}
		}
		arrp = as_array("state_tax_rate", &count);
		if (count > 0)
		{
			if (count == 1) // single value input
			{
				for (i = 0; i < nyears; i++)
					cf.at(CF_state_tax_frac, i + 1) = arrp[0] * 0.01;
			}
			else // schedule
			{
				for (i = 0; i < nyears && i < (int)count; i++)
					cf.at(CF_state_tax_frac, i + 1) = arrp[i] * 0.01;
			}
		}
		for (i = 0; i <= nyears;i++)
			cf.at(CF_effective_tax_frac, i) = cf.at(CF_state_tax_frac, i) +
				(1.0 - cf.at(CF_state_tax_frac, i))*cf.at(CF_federal_tax_frac, i);



		
		double real_discount_rate = as_double("real_discount_rate")*0.01;
		double nom_discount_rate = (1.0 + real_discount_rate) * (1.0 + inflation_rate) - 1.0;


//		double hard_cost = as_double("total_hard_cost");
//		double total_sales_tax = as_double("percent_of_cost_subject_sales_tax")*0.01*hard_cost*as_double("sales_tax_rate")*0.01;
//		double soft_cost = as_double("total_soft_cost") + total_sales_tax;
//		double total_cost = hard_cost + soft_cost;
		double total_cost = as_double("total_installed_cost");
		double property_tax_assessed_value = total_cost * as_double("prop_tax_cost_assessed_percent") * 0.01;

		int loan_term = as_integer("loan_term");
		double loan_rate = as_double("loan_rate")*0.01;
		double debt_frac = as_double("debt_fraction")*0.01;
				
		// precompute expenses from annual schedules or value+escalation
		escal_or_annual( CF_om_fixed_expense, nyears, "om_fixed", inflation_rate, 1.0, false, as_double("om_fixed_escal")*0.01 );
		escal_or_annual( CF_om_production_expense, nyears, "om_production", inflation_rate, 0.001, false, as_double("om_production_escal")*0.01 );  
		escal_or_annual( CF_om_capacity_expense, nyears, "om_capacity", inflation_rate, 1.0, false, as_double("om_capacity_escal")*0.01 );  
		escal_or_annual( CF_om_fuel_expense, nyears, "om_fuel_cost", inflation_rate, as_double("system_heat_rate")*0.001, false, as_double("om_fuel_cost_escal")*0.01 );

		// additional o and m sub types (e.g. batteries and fuel cells)
		int add_om_num_types = as_integer("add_om_num_types");
		ssc_number_t nameplate1 = 0;
		ssc_number_t nameplate2 = 0;

		if (add_om_num_types > 0)
		{
			escal_or_annual(CF_om_fixed1_expense, nyears, "om_fixed1", inflation_rate, 1.0, false, as_double("om_fixed_escal")*0.01);
			escal_or_annual(CF_om_production1_expense, nyears, "om_production1", inflation_rate, 0.001, false, as_double("om_production_escal")*0.01);
			escal_or_annual(CF_om_capacity1_expense, nyears, "om_capacity1", inflation_rate, 1.0, false, as_double("om_capacity_escal")*0.01);
			nameplate1 = as_number("om_capacity1_nameplate");
		}
		if (add_om_num_types > 1)
		{
			escal_or_annual(CF_om_fixed2_expense, nyears, "om_fixed2", inflation_rate, 1.0, false, as_double("om_fixed_escal")*0.01);
			escal_or_annual(CF_om_production2_expense, nyears, "om_production2", inflation_rate, 0.001, false, as_double("om_production_escal")*0.01);
			escal_or_annual(CF_om_capacity2_expense, nyears, "om_capacity2", inflation_rate, 1.0, false, as_double("om_capacity_escal")*0.01);
			nameplate2 = as_number("om_capacity2_nameplate");
		}

		// battery cost - replacement from lifetime analysis
		if ((as_integer("en_batt") == 1) && (as_integer("batt_replacement_option") > 0))
		{
			ssc_number_t *batt_rep = 0;
			if (as_integer("batt_replacement_option")==1)
				batt_rep = as_array("batt_bank_replacement", &count); // replacements per year calculated
			else // user specified
				batt_rep = as_array("batt_replacement_schedule", &count); // replacements per year user-defined
			double batt_cap = as_double("batt_computed_bank_capacity");
			escal_or_annual(CF_battery_replacement_cost_schedule, nyears, "om_replacement_cost1", inflation_rate, batt_cap, false, as_double("om_replacement_cost_escal")*0.01);

			for (int y = 0; y<nyears; y++)
				cf.at(CF_battery_replacement_cost_schedule, y + 1) = batt_repl_cost * batt_cap * pow(1 + batt_repl_cost_escal + inflation_rate, y);

			for (int y = 0; i < nyears && y<(int)count; y++)
				cf.at(CF_battery_replacement_cost, y + 1) = batt_rep[y] * 
					cf.at(CF_battery_replacement_cost_schedule, y + 1);
		}

		// fuelcell cost - replacement from lifetime analysis
		if (is_assigned("fuelcell_replacement_option") && (as_integer("fuelcell_replacement_option") > 0))
		{
			ssc_number_t *fuelcell_rep = 0;
			if (as_integer("fuelcell_replacement_option") == 1)
				fuelcell_rep = as_array("fuelcell_replacement", &count); // replacements per year calculated
			else // user specified
				fuelcell_rep = as_array("fuelcell_replacement_schedule", &count); // replacements per year user-defined
			escal_or_annual(CF_fuelcell_replacement_cost_schedule, nyears, "om_replacement_cost2", inflation_rate, nameplate2, false, as_double("om_replacement_cost_escal")*0.01);
			
			for (int i = 0; i < nyears && i < (int)count; i++) {
				cf.at(CF_fuelcell_replacement_cost, i + 1) = fuelcell_rep[i] *
					cf.at(CF_fuelcell_replacement_cost_schedule, i + 1);
			}
		}


		escal_or_annual( CF_om_opt_fuel_1_expense, nyears, "om_opt_fuel_1_cost", inflation_rate, 1.0, false, as_double("om_opt_fuel_1_cost_escal")*0.01 );  
		escal_or_annual( CF_om_opt_fuel_2_expense, nyears, "om_opt_fuel_2_cost", inflation_rate, 1.0, false, as_double("om_opt_fuel_2_cost_escal")*0.01 );  

		double om_opt_fuel_1_usage = as_double("om_opt_fuel_1_usage");
		double om_opt_fuel_2_usage = as_double("om_opt_fuel_2_usage");
		
		// ibi fixed
		ibi_fed_amount = as_double("ibi_fed_amount");
		ibi_sta_amount = as_double("ibi_sta_amount");
		ibi_uti_amount = as_double("ibi_uti_amount");
		ibi_oth_amount = as_double("ibi_oth_amount");

		// ibi percent
		ibi_fed_per = as_double("ibi_fed_percent")*0.01*total_cost;
		if (ibi_fed_per > as_double("ibi_fed_percent_maxvalue")) ibi_fed_per = as_double("ibi_fed_percent_maxvalue"); 
		ibi_sta_per = as_double("ibi_sta_percent")*0.01*total_cost;
		if (ibi_sta_per > as_double("ibi_sta_percent_maxvalue")) ibi_sta_per = as_double("ibi_sta_percent_maxvalue"); 
		ibi_uti_per = as_double("ibi_uti_percent")*0.01*total_cost;
		if (ibi_uti_per > as_double("ibi_uti_percent_maxvalue")) ibi_uti_per = as_double("ibi_uti_percent_maxvalue"); 
		ibi_oth_per = as_double("ibi_oth_percent")*0.01*total_cost;
		if (ibi_oth_per > as_double("ibi_oth_percent_maxvalue")) ibi_oth_per = as_double("ibi_oth_percent_maxvalue"); 

		// cbi
		cbi_fed_amount = 1000.0*nameplate*as_double("cbi_fed_amount");
		if (cbi_fed_amount > as_double("cbi_fed_maxvalue")) cbi_fed_amount = as_double("cbi_fed_maxvalue"); 
		cbi_sta_amount = 1000.0*nameplate*as_double("cbi_sta_amount");
		if (cbi_sta_amount > as_double("cbi_sta_maxvalue")) cbi_sta_amount = as_double("cbi_sta_maxvalue"); 
		cbi_uti_amount = 1000.0*nameplate*as_double("cbi_uti_amount");
		if (cbi_uti_amount > as_double("cbi_uti_maxvalue")) cbi_uti_amount = as_double("cbi_uti_maxvalue"); 
		cbi_oth_amount = 1000.0*nameplate*as_double("cbi_oth_amount");
		if (cbi_oth_amount > as_double("cbi_oth_maxvalue")) cbi_oth_amount = as_double("cbi_oth_maxvalue"); 
		
		// precompute pbi
		compute_production_incentive( CF_pbi_fed, nyears, "pbi_fed_amount", "pbi_fed_term", "pbi_fed_escal" );
		compute_production_incentive( CF_pbi_sta, nyears, "pbi_sta_amount", "pbi_sta_term", "pbi_sta_escal" );
		compute_production_incentive( CF_pbi_uti, nyears, "pbi_uti_amount", "pbi_uti_term", "pbi_uti_escal" );
		compute_production_incentive( CF_pbi_oth, nyears, "pbi_oth_amount", "pbi_oth_term", "pbi_oth_escal" );

		// precompute ptc
		compute_production_incentive_IRS_2010_37( CF_ptc_sta, nyears, "ptc_sta_amount", "ptc_sta_term", "ptc_sta_escal" );
		compute_production_incentive_IRS_2010_37( CF_ptc_fed, nyears, "ptc_fed_amount", "ptc_fed_term", "ptc_fed_escal" );
	
		// reduce itc bases
		double federal_itc_basis = total_cost
			- ( as_boolean("ibi_fed_amount_deprbas_fed")  ? ibi_fed_amount : 0 )
			- ( as_boolean("ibi_sta_amount_deprbas_fed")  ? ibi_sta_amount : 0 )
			- ( as_boolean("ibi_uti_amount_deprbas_fed")  ? ibi_uti_amount : 0 )
			- ( as_boolean("ibi_oth_amount_deprbas_fed")  ? ibi_oth_amount : 0 )
			- ( as_boolean("ibi_fed_percent_deprbas_fed") ? ibi_fed_per : 0 )
			- ( as_boolean("ibi_sta_percent_deprbas_fed") ? ibi_sta_per : 0 )
			- ( as_boolean("ibi_uti_percent_deprbas_fed") ? ibi_uti_per : 0 )
			- ( as_boolean("ibi_oth_percent_deprbas_fed") ? ibi_oth_per : 0 )
			- ( as_boolean("cbi_fed_deprbas_fed")  ? cbi_fed_amount : 0 )
			- ( as_boolean("cbi_sta_deprbas_fed")  ? cbi_sta_amount : 0 )
			- ( as_boolean("cbi_uti_deprbas_fed")  ? cbi_uti_amount : 0 )
			- ( as_boolean("cbi_oth_deprbas_fed")  ? cbi_oth_amount : 0 );


		// itc fixed
		double itc_fed_amount = as_double("itc_fed_amount");

		// itc percent - max value used for comparison to qualifying costs
		double itc_fed_frac = as_double("itc_fed_percent")*0.01;
		double itc_fed_per = itc_fed_frac * federal_itc_basis;
		if (itc_fed_per > as_double("itc_fed_percent_maxvalue")) itc_fed_per = as_double("itc_fed_percent_maxvalue");



		double state_itc_basis = total_cost
			- ( as_boolean("ibi_fed_amount_deprbas_sta")  ? ibi_fed_amount : 0 )
			- ( as_boolean("ibi_sta_amount_deprbas_sta")  ? ibi_sta_amount : 0 )
			- ( as_boolean("ibi_uti_amount_deprbas_sta")  ? ibi_uti_amount : 0 )
			- ( as_boolean("ibi_oth_amount_deprbas_sta")  ? ibi_oth_amount : 0 )
			- ( as_boolean("ibi_fed_percent_deprbas_sta") ? ibi_fed_per : 0 )
			- ( as_boolean("ibi_sta_percent_deprbas_sta") ? ibi_sta_per : 0 )
			- ( as_boolean("ibi_uti_percent_deprbas_sta") ? ibi_uti_per : 0 )
			- ( as_boolean("ibi_oth_percent_deprbas_sta") ? ibi_oth_per : 0 )
			- ( as_boolean("cbi_fed_deprbas_sta")  ? cbi_fed_amount : 0 )
			- ( as_boolean("cbi_sta_deprbas_sta")  ? cbi_sta_amount : 0 )
			- ( as_boolean("cbi_uti_deprbas_sta")  ? cbi_uti_amount : 0 )
			- ( as_boolean("cbi_oth_deprbas_sta")  ? cbi_oth_amount : 0 );


		// itc fixed
		double itc_sta_amount = as_double("itc_sta_amount");

		// itc percent - max value used for comparison to qualifying costs
		double itc_sta_frac = as_double("itc_sta_percent")*0.01;
		double itc_sta_per = itc_sta_frac * state_itc_basis;
		if (itc_sta_per > as_double("itc_sta_percent_maxvalue")) itc_sta_per = as_double("itc_sta_percent_maxvalue");


		double federal_depr_basis = federal_itc_basis
			- ( as_boolean("itc_fed_amount_deprbas_fed")   ? 0.5*itc_fed_amount : 0 )
			- ( as_boolean("itc_fed_percent_deprbas_fed")  ? 0.5*itc_fed_per : 0 )
			- ( as_boolean("itc_sta_amount_deprbas_fed")   ? 0.5*itc_sta_amount : 0 )
			- ( as_boolean("itc_sta_percent_deprbas_fed")  ? 0.5*itc_sta_per : 0 );

		double state_depr_basis = state_itc_basis 
			- ( as_boolean("itc_fed_amount_deprbas_sta")   ? 0.5*itc_fed_amount : 0 )
			- ( as_boolean("itc_fed_percent_deprbas_sta")  ? 0.5*itc_fed_per : 0 )
			- ( as_boolean("itc_sta_amount_deprbas_sta")   ? 0.5*itc_sta_amount : 0 )
			- ( as_boolean("itc_sta_percent_deprbas_sta")  ? 0.5*itc_sta_per : 0 );

		if (is_commercial)
		{
			// only compute depreciation for commercial market

			switch( as_integer("depr_sta_type") )
			{
			case 1: depreciation_sched_macrs_half_year( CF_sta_depr_sched, nyears ); break;
			case 2: depreciation_sched_straight_line( CF_sta_depr_sched, nyears, as_integer("depr_sta_sl_years") ); break;
			case 3: 
				{
					size_t arr_len;
					ssc_number_t *arr_cust = as_array( "depr_sta_custom", &arr_len );
					depreciation_sched_custom( CF_sta_depr_sched, nyears, arr_cust, (int)arr_len );
					break;
				}
			}

			switch( as_integer("depr_fed_type") )
			{
			case 1: depreciation_sched_macrs_half_year( CF_fed_depr_sched, nyears ); break;
			case 2: depreciation_sched_straight_line( CF_fed_depr_sched, nyears, as_integer("depr_fed_sl_years") ); break;
			case 3: 
				{
					size_t arr_len;
					ssc_number_t *arr_cust = as_array( "depr_fed_custom", &arr_len );
					depreciation_sched_custom( CF_fed_depr_sched, nyears, arr_cust, (int)arr_len );
					break;
				}
			}
		}
		
		double state_tax_savings = 0.0;
		double federal_tax_savings = 0.0;

		double adjusted_installed_cost = total_cost
			- ibi_fed_amount
			- ibi_sta_amount
			- ibi_uti_amount
			- ibi_oth_amount
			- ibi_fed_per
			- ibi_sta_per
			- ibi_uti_per
			- ibi_oth_per
			- cbi_fed_amount
			- cbi_sta_amount
			- cbi_uti_amount
			- cbi_oth_amount;

		double loan_amount = debt_frac * adjusted_installed_cost;
		double first_cost = adjusted_installed_cost - loan_amount;  //cpg: What is the difference between adjusted_installed_cost and capital_investment?
		double capital_investment = loan_amount + first_cost;
		
		cf.at(CF_after_tax_net_equity_cost_flow,0) = -first_cost + state_tax_savings + federal_tax_savings;
		cf.at(CF_after_tax_cash_flow,0) = cf.at(CF_after_tax_net_equity_cost_flow,0);

		cf.at(CF_payback_with_expenses, 0) = -capital_investment;
		cf.at(CF_cumulative_payback_with_expenses, 0) = -capital_investment;

		cf.at(CF_discounted_costs, 0) = capital_investment;
		cf.at(CF_discounted_payback, 0) = cf.at(CF_discounted_savings, 0) - cf.at(CF_discounted_costs, 0);
		cf.at(CF_discounted_cumulative_payback, 0) = cf.at(CF_discounted_payback, 0);

		cf.at(CF_payback_without_expenses,0) = -capital_investment;
		cf.at(CF_cumulative_payback_without_expenses,0) = -capital_investment;

		double ibi_total = ibi_fed_amount + ibi_fed_per + ibi_sta_amount + ibi_sta_per + ibi_uti_amount + ibi_uti_per + ibi_oth_amount + ibi_oth_per;
		double cbi_total = cbi_fed_amount + cbi_sta_amount + cbi_uti_amount + cbi_oth_amount;
		double itc_total_fed = itc_fed_amount + itc_fed_per;
		double itc_total_sta = itc_sta_amount + itc_sta_per;

		for (i=1; i<=nyears; i++)
		{			
			// compute expenses
			cf.at(CF_om_production_expense,i) *= cf.at(CF_energy_net,i);
			cf.at(CF_om_capacity_expense,i) *= nameplate;

			cf.at(CF_om_capacity1_expense, i) *= nameplate1;
			cf.at(CF_om_capacity2_expense, i) *= nameplate2;
			// TODO additional production o and m here


			cf.at(CF_om_fuel_expense,i) *= year1_fuel_use;
			cf.at(CF_om_opt_fuel_1_expense,i) *= om_opt_fuel_1_usage;
			cf.at(CF_om_opt_fuel_2_expense,i) *= om_opt_fuel_2_usage;
			double decline_percent = 100 - (i-1)*property_tax_decline_percentage;
			cf.at(CF_property_tax_assessed_value,i) = (decline_percent > 0) ? property_tax_assessed_value * decline_percent * 0.01:0.0;
			cf.at(CF_property_tax_expense,i) = cf.at(CF_property_tax_assessed_value,i) * property_tax;
			
			cf.at(CF_insurance_expense,i) = total_cost * insurance_rate * pow( 1 + inflation_rate, i-1 );

			cf.at(CF_net_salvage_value, i) = 0.0;
			if (i == nyears) /* salvage value handled as negative operating expense in last year */
				cf.at(CF_net_salvage_value,i) = total_cost * salvage_frac;

			cf.at(CF_operating_expenses,i) = 
				+cf.at(CF_om_fixed_expense, i)
				+ cf.at(CF_om_production_expense, i)
				+ cf.at(CF_om_capacity_expense, i)
				+ cf.at(CF_om_fixed1_expense, i)
				+ cf.at(CF_om_production1_expense, i)
				+ cf.at(CF_om_capacity1_expense, i)
				+ cf.at(CF_om_fixed2_expense, i)
				+ cf.at(CF_om_production2_expense, i)
				+ cf.at(CF_om_capacity2_expense, i)
				+ cf.at(CF_om_fuel_expense,i)
				+ cf.at(CF_om_opt_fuel_1_expense,i)
				+ cf.at(CF_om_opt_fuel_2_expense,i)
				+ cf.at(CF_property_tax_expense,i)
				+ cf.at(CF_insurance_expense,i)
				+ cf.at(CF_battery_replacement_cost, i)
				+ cf.at(CF_fuelcell_replacement_cost, i)
				- cf.at(CF_net_salvage_value,i);

			
			if (is_commercial)
				cf.at(CF_deductible_expenses,i) = -cf.at(CF_operating_expenses,i);  // commercial
			else
				cf.at(CF_deductible_expenses,i) = -cf.at(CF_property_tax_expense,i); // residential

			if (i == 1)
			{
				cf.at(CF_debt_balance, i-1) = loan_amount;
				cf.at(CF_debt_payment_interest, i) = loan_amount * loan_rate;
				cf.at(CF_debt_payment_principal,i) = -ppmt( loan_rate,       // Rate
																i,           // Period
																loan_term,   // Number periods
																loan_amount, // Present Value
																0,           // future Value
																0 );         // cash flow at end of period
				cf.at(CF_debt_balance, i) = cf.at(CF_debt_balance, i - 1) - cf.at(CF_debt_payment_principal, i);
			}
			else
			{
				if (i <= loan_term) 
				{
					cf.at(CF_debt_payment_interest, i) = loan_rate * cf.at(CF_debt_balance, i-1);

					if (loan_rate != 0.0)
					{
						cf.at(CF_debt_payment_principal,i) = loan_rate * loan_amount/(1 - pow((1 + loan_rate),-loan_term))
							- cf.at(CF_debt_payment_interest,i);
					}
					else
					{
						cf.at(CF_debt_payment_principal,i) = loan_amount / loan_term - cf.at(CF_debt_payment_interest,i);
					}
					cf.at(CF_debt_balance, i) = cf.at(CF_debt_balance, i - 1) - cf.at(CF_debt_payment_principal, i);
				}
			}

			cf.at(CF_debt_payment_total,i) = cf.at(CF_debt_payment_principal,i) + cf.at(CF_debt_payment_interest,i);
			
			// compute pbi total		
			cf.at(CF_pbi_total, i) = cf.at(CF_pbi_fed, i) + cf.at(CF_pbi_sta, i) + cf.at(CF_pbi_uti, i) + cf.at(CF_pbi_oth, i);
			// compute ptc total		
			cf.at(CF_ptc_total, i) = cf.at(CF_ptc_fed, i) + cf.at(CF_ptc_sta, i);
			
			// compute depreciation from basis and precalculated schedule
			cf.at(CF_sta_depreciation,i) = cf.at(CF_sta_depr_sched,i)*state_depr_basis;
			cf.at(CF_fed_depreciation,i) = cf.at(CF_fed_depr_sched,i)*federal_depr_basis;

			
			// ************************************************
			// tax effect on equity (state)

			cf.at(CF_sta_incentive_income_less_deductions, i) =
				+ cf.at(CF_deductible_expenses, i) 
				+ cf.at(CF_pbi_total,i)
				- cf.at(CF_sta_depreciation,i);

			if (i==1) cf.at(CF_sta_incentive_income_less_deductions, i) += ibi_total + cbi_total;

// sales tax is in depreciable bases and is already written off according to depreciation schedule.
//			if (is_commercial && i == 1) cf.at(CF_sta_incentive_income_less_deductions, i) -= total_sales_tax;


			if (is_commercial || is_mortgage) // interest only deductible if residential mortgage or commercial
				cf.at(CF_sta_incentive_income_less_deductions, i) -= cf.at(CF_debt_payment_interest,i);

			cf.at(CF_sta_taxable_income_less_deductions, i) = taxable_incentive_income( i, "sta" )
				+ cf.at(CF_deductible_expenses,i)
				- cf.at(CF_sta_depreciation,i);
			
// sales tax is in depreciable bases and is already written off according to depreciation schedule.
//			if (is_commercial && i == 1) cf.at(CF_sta_taxable_income_less_deductions,i) -= total_sales_tax;

			if (is_commercial || is_mortgage) // interest only deductible if residential mortgage or commercial
				cf.at(CF_sta_taxable_income_less_deductions, i) -= cf.at(CF_debt_payment_interest,i);

			cf.at(CF_sta_tax_savings, i) = cf.at(CF_ptc_sta,i) - cf.at(CF_state_tax_frac,i)*cf.at(CF_sta_taxable_income_less_deductions,i);
			if (i==1) cf.at(CF_sta_tax_savings, i) += itc_sta_amount + itc_sta_per;
			
			// ************************************************
			//	tax effect on equity (federal)

			cf.at(CF_fed_incentive_income_less_deductions, i) =
				+ cf.at(CF_deductible_expenses, i)
				+ cf.at(CF_pbi_total,i)
				- cf.at(CF_fed_depreciation,i)
				+ cf.at(CF_sta_tax_savings, i);
			
			if (i==1) cf.at(CF_fed_incentive_income_less_deductions, i) += ibi_total + cbi_total;

// sales tax is in depreciable bases and is already written off according to depreciation schedule.
//			if (is_commercial && i == 1) cf.at(CF_fed_incentive_income_less_deductions, i) -= total_sales_tax;
			
			if (is_commercial || is_mortgage) // interest only deductible if residential mortgage or commercial
				cf.at(CF_fed_incentive_income_less_deductions, i) -= cf.at(CF_debt_payment_interest,i);

			cf.at(CF_fed_taxable_income_less_deductions, i) = taxable_incentive_income( i, "fed" )
				+ cf.at(CF_deductible_expenses,i)
				- cf.at(CF_fed_depreciation,i)
				+ cf.at(CF_sta_tax_savings, i);

// sales tax is in depreciable bases and is already written off according to depreciation schedule.
//			if (is_commercial && i == 1) cf.at(CF_fed_taxable_income_less_deductions, i) -= total_sales_tax;

			if (is_commercial || is_mortgage) // interest only deductible if residential mortgage or commercial
				cf.at(CF_fed_taxable_income_less_deductions, i) -= cf.at(CF_debt_payment_interest,i);
			
			cf.at(CF_fed_tax_savings, i) = cf.at(CF_ptc_fed,i) - cf.at(CF_federal_tax_frac,i)*cf.at(CF_fed_taxable_income_less_deductions,i);
			if (i==1) cf.at(CF_fed_tax_savings, i) += itc_fed_amount + itc_fed_per;
			
			// ************************************************
			// combined tax savings and cost/cash flows
				
			cf.at(CF_sta_and_fed_tax_savings,i) = cf.at(CF_sta_tax_savings, i)+cf.at(CF_fed_tax_savings, i);

			cf.at(CF_after_tax_net_equity_cost_flow, i) =
				+ (is_commercial ? cf.at(CF_deductible_expenses, i) : -cf.at(CF_operating_expenses,i) )
				- cf.at(CF_debt_payment_total, i)
				+ cf.at(CF_pbi_total, i)
				+ cf.at(CF_sta_and_fed_tax_savings,i);

			/*
			Calculate discounted payback period from March 1995 NREL/TP-462-5173 p.58
			CF_discounted_costs,
			CF_discounted_savings,
			CF_discounted_payback,
			CF_discounted_cumulative_payback,
			*/
			// take costs to be positive in this context
			cf.at(CF_discounted_costs, i) = -cf.at(CF_after_tax_net_equity_cost_flow, i) - cf.at(CF_debt_payment_total, i);
			// interest already deducted and accounted for in tax savings (so add back here)
			if (is_commercial || is_mortgage)
				cf.at(CF_discounted_costs, i) += cf.at(CF_debt_payment_interest, i) * cf.at(CF_effective_tax_frac,i);
			// discount at nominal discount rate
			cf.at(CF_discounted_costs, i) /= pow((1.0 + nom_discount_rate), (i - 1));
			// savings reduced by effective tax rate for commercial since already included in tax savings
			cf.at(CF_discounted_savings, i) = ((is_commercial ? (1.0 - cf.at(CF_effective_tax_frac, i)) : 1.0)*cf.at(CF_energy_value, i)) / pow((1.0 + nom_discount_rate), (i - 1))
				+ ((is_commercial ? (1.0 - cf.at(CF_effective_tax_frac, i)) : 1.0)*cf.at(CF_thermal_value, i)) / pow((1.0 + nom_discount_rate), (i - 1));
			cf.at(CF_discounted_payback, i) = cf.at(CF_discounted_savings, i) - cf.at(CF_discounted_costs, i);
			cf.at(CF_discounted_cumulative_payback, i) =
				cf.at(CF_discounted_cumulative_payback, i - 1)
				+ cf.at(CF_discounted_payback, i);



			cf.at(CF_after_tax_cash_flow,i) = 
				cf.at(CF_after_tax_net_equity_cost_flow, i)
				+ ((is_commercial ? (1.0 - cf.at(CF_effective_tax_frac, i)) : 1.0)*cf.at(CF_energy_value, i))
				+((is_commercial ? (1.0 - cf.at(CF_effective_tax_frac, i)) : 1.0)*cf.at(CF_thermal_value, i));

			if ( is_commercial || is_mortgage )
				cf.at(CF_payback_with_expenses,i) =
					cf.at(CF_after_tax_cash_flow,i)
					+ cf.at(CF_debt_payment_interest, i) * (1 - cf.at(CF_effective_tax_frac, i))
					+ cf.at(CF_debt_payment_principal,i);
			else
				cf.at(CF_payback_with_expenses,i) =
					cf.at(CF_after_tax_cash_flow,i)
					+ cf.at(CF_debt_payment_interest,i)
					+ cf.at(CF_debt_payment_principal,i);

			cf.at(CF_cumulative_payback_with_expenses,i) = 
				cf.at(CF_cumulative_payback_with_expenses,i-1)
				+cf.at(CF_payback_with_expenses,i);
	
			if ( is_commercial || is_mortgage )
				cf.at(CF_payback_without_expenses,i) =
					+ cf.at(CF_after_tax_cash_flow,i)
					+ cf.at(CF_debt_payment_interest, i) * (1.0 - cf.at(CF_effective_tax_frac, i))
					+ cf.at(CF_debt_payment_principal,i)
					- cf.at(CF_deductible_expenses,i)
					+ cf.at(CF_deductible_expenses, i) * cf.at(CF_effective_tax_frac, i);
			else
				cf.at(CF_payback_without_expenses,i) =
					+ cf.at(CF_after_tax_cash_flow,i)
					+ cf.at(CF_debt_payment_interest,i)
					+ cf.at(CF_debt_payment_principal,i)
					- cf.at(CF_deductible_expenses,i)
					+ cf.at(CF_deductible_expenses, i) * cf.at(CF_effective_tax_frac, i);


			cf.at(CF_cumulative_payback_without_expenses,i) =
				+ cf.at(CF_cumulative_payback_without_expenses,i-1)
				+ cf.at(CF_payback_without_expenses,i);	
		}
		
		double npv_energy_real = npv( CF_energy_net, nyears, real_discount_rate );
//		if (npv_energy_real == 0.0) throw general_error("lcoe real failed because energy npv is zero");
//		double lcoe_real = -( cf.at(CF_after_tax_net_equity_cost_flow,0) + npv(CF_after_tax_net_equity_cost_flow, nyears, nom_discount_rate) ) * 100 / npv_energy_real;
		double lcoe_real = -( cf.at(CF_after_tax_net_equity_cost_flow,0) + npv(CF_after_tax_net_equity_cost_flow, nyears, nom_discount_rate) ) * 100;
		if (npv_energy_real == 0.0) 
		{
			lcoe_real = std::numeric_limits<double>::quiet_NaN();
		}
		else
		{
			lcoe_real /= npv_energy_real;
		}

		double npv_energy_nom = npv( CF_energy_net, nyears, nom_discount_rate );
//		if (npv_energy_nom == 0.0) throw general_error("lcoe nom failed because energy npv is zero");
//		double lcoe_nom = -( cf.at(CF_after_tax_net_equity_cost_flow,0) + npv(CF_after_tax_net_equity_cost_flow, nyears, nom_discount_rate) ) * 100 / npv_energy_nom;
		double lcoe_nom = -( cf.at(CF_after_tax_net_equity_cost_flow,0) + npv(CF_after_tax_net_equity_cost_flow, nyears, nom_discount_rate) ) * 100;
		if (npv_energy_nom == 0.0) 
		{
			lcoe_nom = std::numeric_limits<double>::quiet_NaN();
		}
		else
		{
			lcoe_nom /= npv_energy_nom;
		}

		double net_present_value = cf.at(CF_after_tax_cash_flow, 0) + npv(CF_after_tax_cash_flow, nyears, nom_discount_rate );

		double payback = compute_payback(CF_cumulative_payback_with_expenses, CF_payback_with_expenses, nyears);
		// Added for Owen Zinaman for Mexico Rates and analyses 9/26/16
		//- see C:\Projects\SAM\Documentation\Payback\DiscountedPayback_2016.9.26
		double discounted_payback = compute_payback(CF_discounted_cumulative_payback, CF_discounted_payback, nyears);

		// save outputs


	double npv_fed_ptc = npv(CF_ptc_fed,nyears,nom_discount_rate);
	double npv_sta_ptc = npv(CF_ptc_sta,nyears,nom_discount_rate);

	// TODO check this
//	npv_fed_ptc /= (1.0 - effective_tax_rate);
//	npv_sta_ptc /= (1.0 - effective_tax_rate);
	npv_fed_ptc /= (1.0 - cf.at(CF_effective_tax_frac,1));
	npv_sta_ptc /= (1.0 - cf.at(CF_effective_tax_frac, 1));

	double lcoptc_fed_nom=0.0;
	if (npv_energy_nom != 0) lcoptc_fed_nom = npv_fed_ptc / npv_energy_nom * 100.0;
	double lcoptc_fed_real=0.0;
	if (npv_energy_real != 0) lcoptc_fed_real = npv_fed_ptc / npv_energy_real * 100.0;

	double lcoptc_sta_nom=0.0;
	if (npv_energy_nom != 0) lcoptc_sta_nom = npv_sta_ptc / npv_energy_nom * 100.0;
	double lcoptc_sta_real=0.0;
	if (npv_energy_real != 0) lcoptc_sta_real = npv_sta_ptc / npv_energy_real * 100.0;

	assign("lcoptc_fed_nom", var_data((ssc_number_t) lcoptc_fed_nom));
	assign("lcoptc_fed_real", var_data((ssc_number_t) lcoptc_fed_real));
	assign("lcoptc_sta_nom", var_data((ssc_number_t) lcoptc_sta_nom));
	assign("lcoptc_sta_real", var_data((ssc_number_t) lcoptc_sta_real));



	double wacc = 0.0;
	wacc = (1.0 - debt_frac)*nom_discount_rate + debt_frac*loan_rate*(1.0 - cf.at(CF_effective_tax_frac, 1));

	wacc *= 100.0;
//	effective_tax_rate *= 100.0;


	assign("wacc", var_data( (ssc_number_t) wacc));
	assign("effective_tax_rate", var_data((ssc_number_t)(cf.at(CF_effective_tax_frac, 1)*100.0)));



		// NTE
		ssc_number_t *ub_w_sys = 0;
		ub_w_sys = as_array("elec_cost_with_system", &count);
		if (count != nyears+1)
			throw exec_error("third party ownership", util::format("utility bill with system input wrong length (%d) should be (%d)",count, nyears+1));
		ssc_number_t *ub_wo_sys = 0;
		ub_wo_sys = as_array("elec_cost_without_system", &count);
		if (count != nyears+1)
			throw exec_error("third party ownership", util::format("utility bill without system input wrong length (%d) should be (%d)",count, nyears+1));

		for (i = 0; i < (int)count; i++)
			cf.at(CF_nte, i) = (double) (ub_wo_sys[i] - ub_w_sys[i]) *100.0;// $ to cents
		double lnte_real = npv(  CF_nte, nyears, nom_discount_rate ); 

		for (i = 0; i < (int)count; i++)
			if (cf.at(CF_energy_net,i) > 0) cf.at(CF_nte,i) /= cf.at(CF_energy_net,i);

		double lnte_nom = lnte_real;
		if (npv_energy_real == 0.0) 
			lnte_real = std::numeric_limits<double>::quiet_NaN();
		else
			lnte_real /= npv_energy_real;
		if (npv_energy_nom == 0.0) 
			lnte_nom = std::numeric_limits<double>::quiet_NaN();
		else
			lnte_nom /= npv_energy_nom;

		assign( "lnte_real", var_data((ssc_number_t)lnte_real) );
		assign( "lnte_nom", var_data((ssc_number_t)lnte_nom) );
		save_cf(CF_nte, nyears, "cf_nte");
		assign( "year1_nte", var_data((ssc_number_t)cf.at(CF_nte,1)) );





		assign( "cf_length", var_data( (ssc_number_t) nyears+1 ));

		assign("payback", var_data((ssc_number_t)payback));
		assign("discounted_payback", var_data((ssc_number_t)discounted_payback));
		assign("lcoe_real", var_data((ssc_number_t)lcoe_real));
		assign( "lcoe_nom", var_data((ssc_number_t)lcoe_nom) );
		assign( "npv",  var_data((ssc_number_t)net_present_value) );

//		assign("first_year_energy_net", var_data((ssc_number_t) cf.at(CF_energy_net,1)));

		assign( "depr_basis_fed", var_data((ssc_number_t)federal_depr_basis ));
		assign( "depr_basis_sta", var_data((ssc_number_t)state_depr_basis ));
		assign( "discount_nominal", var_data((ssc_number_t)(nom_discount_rate*100.0) ));		
//		assign( "sales_tax_deduction", var_data((ssc_number_t)total_sales_tax ));		
		assign( "adjusted_installed_cost", var_data((ssc_number_t)adjusted_installed_cost ));		
		assign( "first_cost", var_data((ssc_number_t)first_cost ));		
		assign( "total_cost", var_data((ssc_number_t)total_cost ));		
		assign( "loan_amount", var_data((ssc_number_t)loan_amount ));		
		
		save_cf( CF_energy_net, nyears, "cf_energy_net" );
		save_cf(CF_energy_value, nyears, "cf_energy_value");
		save_cf(CF_thermal_value, nyears, "cf_thermal_value");


// real estate value added 6/24/13
		for (int y=1;y<nyears+1;y++)
		{
			double rr = 1.0;
			if (nom_discount_rate != -1.0) rr = 1.0/(1.0+nom_discount_rate);
			double result = 0;
			for (int j=nyears;j>=i;j--) 
			result = rr * result + cf.at(CF_energy_value, j) + cf.at(CF_thermal_value, j);
			cf.at(CF_value_added,i) = result*rr + cf.at(CF_net_salvage_value,i);
		}
		save_cf( CF_value_added, nyears, "cf_value_added" );

		save_cf(CF_federal_tax_frac, nyears, "cf_federal_tax_frac");
		save_cf(CF_state_tax_frac, nyears, "cf_state_tax_frac");
		save_cf(CF_effective_tax_frac, nyears, "cf_effective_tax_frac");


		save_cf(CF_om_fixed_expense, nyears, "cf_om_fixed_expense");
		save_cf(CF_om_production_expense, nyears, "cf_om_production_expense");
		save_cf(CF_om_capacity_expense, nyears, "cf_om_capacity_expense");
		save_cf(CF_om_fixed1_expense, nyears, "cf_om_fixed1_expense");
		save_cf(CF_om_production1_expense, nyears, "cf_om_production1_expense");
		save_cf(CF_om_capacity1_expense, nyears, "cf_om_capacity1_expense");
		save_cf(CF_om_fixed2_expense, nyears, "cf_om_fixed2_expense");
		save_cf(CF_om_production2_expense, nyears, "cf_om_production2_expense");
		save_cf(CF_om_capacity2_expense, nyears, "cf_om_capacity2_expense");
		save_cf( CF_om_fuel_expense, nyears, "cf_om_fuel_expense" );
		save_cf( CF_om_opt_fuel_1_expense, nyears, "cf_om_opt_fuel_1_expense" );
		save_cf( CF_om_opt_fuel_2_expense, nyears, "cf_om_opt_fuel_2_expense" );
		save_cf( CF_property_tax_assessed_value, nyears, "cf_property_tax_assessed_value" );
		save_cf( CF_property_tax_expense, nyears, "cf_property_tax_expense" );
		save_cf( CF_insurance_expense, nyears, "cf_insurance_expense" );
		save_cf( CF_net_salvage_value, nyears, "cf_net_salvage_value" );
		save_cf(CF_battery_replacement_cost, nyears, "cf_battery_replacement_cost");
		save_cf(CF_battery_replacement_cost_schedule, nyears, "cf_battery_replacement_cost_schedule");
		save_cf(CF_fuelcell_replacement_cost, nyears, "cf_fuelcell_replacement_cost");
		save_cf(CF_fuelcell_replacement_cost_schedule, nyears, "cf_fuelcell_replacement_cost_schedule");
		save_cf(CF_operating_expenses, nyears, "cf_operating_expenses");

		save_cf( CF_deductible_expenses, nyears, "cf_deductible_expenses");
		
		save_cf( CF_debt_balance, nyears, "cf_debt_balance" );
		save_cf( CF_debt_payment_interest, nyears, "cf_debt_payment_interest" );
		save_cf( CF_debt_payment_principal, nyears, "cf_debt_payment_principal" );
		save_cf( CF_debt_payment_total, nyears, "cf_debt_payment_total" );

		assign( "ibi_total_fed", var_data((ssc_number_t) (ibi_fed_amount + ibi_fed_per)));
		assign( "ibi_total_sta", var_data((ssc_number_t) (ibi_sta_amount + ibi_sta_per)));
		assign( "ibi_total_uti", var_data((ssc_number_t) (ibi_uti_amount + ibi_uti_per)));
		assign( "ibi_total_oth", var_data((ssc_number_t) (ibi_oth_amount + ibi_oth_per)));
		assign( "ibi_total", var_data((ssc_number_t) ibi_total));

		assign( "cbi_total_fed", var_data((ssc_number_t) (cbi_fed_amount)));
		assign( "cbi_total_sta", var_data((ssc_number_t) (cbi_sta_amount)));
		assign( "cbi_total_uti", var_data((ssc_number_t) (cbi_uti_amount)));
		assign( "cbi_total_oth", var_data((ssc_number_t) (cbi_oth_amount)));
		assign( "cbi_total", var_data((ssc_number_t) cbi_total));
		
		
		save_cf( CF_pbi_fed, nyears, "cf_pbi_total_fed" );
		save_cf( CF_pbi_sta, nyears, "cf_pbi_total_sta" );
		save_cf( CF_pbi_uti, nyears, "cf_pbi_total_uti" );
		save_cf( CF_pbi_oth, nyears, "cf_pbi_total_oth" );
		save_cf( CF_pbi_total, nyears, "cf_pbi_total" );
	
		save_cf( CF_ptc_fed, nyears, "cf_ptc_fed" );
		save_cf( CF_ptc_sta, nyears, "cf_ptc_sta" );
		save_cf( CF_ptc_total, nyears, "cf_ptc_total" );

		assign( "itc_total_fed", var_data((ssc_number_t) itc_total_fed));
		assign( "itc_total_sta", var_data((ssc_number_t) itc_total_sta));
		assign( "itc_total", var_data((ssc_number_t) (itc_total_fed+itc_total_sta)));
	
		save_cf( CF_sta_depr_sched, nyears, "cf_sta_depr_sched" );
		save_cf( CF_sta_depreciation, nyears, "cf_sta_depreciation" );
		save_cf( CF_sta_incentive_income_less_deductions, nyears, "cf_sta_incentive_income_less_deductions" );
		save_cf( CF_sta_taxable_income_less_deductions, nyears, "cf_sta_taxable_income_less_deductions" );
		save_cf( CF_sta_tax_savings, nyears, "cf_sta_tax_savings" );
	
		save_cf( CF_fed_depr_sched, nyears, "cf_fed_depr_sched" );
		save_cf( CF_fed_depreciation, nyears, "cf_fed_depreciation" );
		save_cf( CF_fed_incentive_income_less_deductions, nyears, "cf_fed_incentive_income_less_deductions" );
		save_cf( CF_fed_taxable_income_less_deductions, nyears, "cf_fed_taxable_income_less_deductions" );
		save_cf( CF_fed_tax_savings, nyears, "cf_fed_tax_savings" );

		save_cf( CF_sta_and_fed_tax_savings, nyears, "cf_sta_and_fed_tax_savings" );
		save_cf( CF_after_tax_net_equity_cost_flow, nyears, "cf_after_tax_net_equity_cost_flow" );
		save_cf( CF_after_tax_cash_flow, nyears, "cf_after_tax_cash_flow" );

		save_cf( CF_payback_with_expenses, nyears, "cf_payback_with_expenses" );
		save_cf( CF_cumulative_payback_with_expenses, nyears, "cf_cumulative_payback_with_expenses" );

		// For Owen and discounted payback period
		save_cf(CF_discounted_costs, nyears, "cf_discounted_costs");
		save_cf(CF_discounted_savings, nyears, "cf_discounted_savings");
		save_cf(CF_discounted_payback, nyears, "cf_discounted_payback");
		save_cf(CF_discounted_cumulative_payback, nyears, "cf_discounted_cumulative_payback");

		save_cf( CF_payback_without_expenses, nyears, "cf_payback_without_expenses" );
		save_cf( CF_cumulative_payback_without_expenses, nyears, "cf_cumulative_payback_without_expenses" );

	// for cost stacked bars
		//npv(CF_energy_value, nyears, nom_discount_rate)
		// present value of o and m value - note - present value is distributive - sum of pv = pv of sum
		double pvAnnualOandM = npv(CF_om_fixed_expense, nyears, nom_discount_rate);
		double pvFixedOandM = npv(CF_om_capacity_expense, nyears, nom_discount_rate);
		double pvVariableOandM = npv(CF_om_production_expense, nyears, nom_discount_rate);
		double pvFuelOandM = npv(CF_om_fuel_expense, nyears, nom_discount_rate);
		double pvOptFuel1OandM = npv(CF_om_opt_fuel_1_expense, nyears, nom_discount_rate);
		double pvOptFuel2OandM = npv(CF_om_opt_fuel_2_expense, nyears, nom_discount_rate);
	//	double pvWaterOandM = NetPresentValue(sv[svNominalDiscountRate], cf[cfAnnualWaterCost], analysis_period);

		assign( "present_value_oandm",  var_data((ssc_number_t)(pvAnnualOandM + pvFixedOandM + pvVariableOandM + pvFuelOandM))); // + pvWaterOandM);

		assign( "present_value_oandm_nonfuel", var_data((ssc_number_t)(pvAnnualOandM + pvFixedOandM + pvVariableOandM)));
		assign( "present_value_fuel", var_data((ssc_number_t)(pvFuelOandM + pvOptFuel1OandM + pvOptFuel2OandM)));

		// present value of insurance and property tax
		double pvInsurance = npv(CF_insurance_expense, nyears, nom_discount_rate);
		double pvPropertyTax = npv(CF_property_tax_expense, nyears, nom_discount_rate);

		assign( "present_value_insandproptax", var_data((ssc_number_t)(pvInsurance + pvPropertyTax)));
	}

/* These functions can be placed in common financial library with matrix and constants passed? */

	void save_cf(int cf_line, int nyears, const std::string &name)
	{
		ssc_number_t *arrp = allocate( name, nyears+1 );
		for (int i=0;i<=nyears;i++)
			arrp[i] = (ssc_number_t)cf.at(cf_line, i);
	}

	double compute_payback( int cf_cpb, int cf_pb, int nyears )
	{	
		// may need to determine last negative to positive transition for high replacement costs - see C:\Projects\SAM\Documentation\FinancialIssues\Payback_2015.9.8
		double dPayback = std::numeric_limits<double>::quiet_NaN(); // report as > analysis period
		bool bolPayback = false;
		int iPayback = 0;
		int i = 1; 
		while ((i<=nyears) && (!bolPayback))
		{
			if (cf.at(cf_cpb,i) > 0)
			{
				bolPayback = true;
				iPayback = i;
			}
			i++;
		}

		if (bolPayback)
		{
			dPayback = iPayback;
			if (cf.at(cf_pb, iPayback) != 0.0)
				dPayback -= cf.at(cf_cpb,iPayback) / cf.at(cf_pb,iPayback);
		}

		return dPayback;
	}


	double npv(int cf_line, int nyears, double rate) throw (general_error)
	{		
		if (rate <= -1.0) throw general_error("cannot calculate NPV with discount rate less or equal to -1.0");

		double rr = 1/(1+rate);
		double result = 0;
		for (int i=nyears;i>0;i--)
			result = rr * result + cf.at(cf_line,i);

		return result*rr;
	}

	void compute_production_incentive( int cf_line, int nyears, const std::string &s_val, const std::string &s_term, const std::string &s_escal )
	{
		size_t len = 0;
		ssc_number_t *parr = as_array(s_val, &len);
		int term = as_integer(s_term);
		double escal = as_double(s_escal)/100.0;

		if (len == 1)
		{
			for (int i=1;i<=nyears;i++)
				cf.at(cf_line, i) = (i <= term) ? parr[0] * cf.at(CF_energy_net,i) * pow(1 + escal, i-1) : 0.0;
		}
		else
		{
			for (int i=1;i<=nyears && i <= (int)len;i++)
				cf.at(cf_line, i) = parr[i-1]*cf.at(CF_energy_net,i);
		}
	}

		void compute_production_incentive_IRS_2010_37( int cf_line, int nyears, const std::string &s_val, const std::string &s_term, const std::string &s_escal )
	{
		// rounding based on IRS document and emails from John and Matt from DHF Financials 2/24/2011 and DHF model v4.4
		size_t len = 0;
		ssc_number_t *parr = as_array(s_val, &len);
		int term = as_integer(s_term);
		double escal = as_double(s_escal)/100.0;

		if (len == 1)
		{
			for (int i=1;i<=nyears;i++)
				cf.at(cf_line, i) = (i <= term) ? cf.at(CF_energy_net,i) / 1000.0 * round_dhf(1000.0 * parr[0] * pow(1 + escal, i-1)) : 0.0;
		}
		else
		{
			for (int i=1;i<=nyears && i <= (int)len;i++)
				cf.at(cf_line, i) = parr[i-1]*cf.at(CF_energy_net,i);
		}
	}


	void single_or_schedule( int cf_line, int nyears, double scale, const std::string &name )
	{
		size_t len = 0;
		ssc_number_t *p = as_array(name, &len);
		for (int i=1;i<=(int)len && i <= nyears;i++)
			cf.at(cf_line, i) = scale*p[i-1];
	}
	
	void single_or_schedule_check_max( int cf_line, int nyears, double scale, const std::string &name, const std::string &maxvar )
	{
		double max = as_double(maxvar);
		size_t len = 0;
		ssc_number_t *p = as_array(name, &len);
		for (int i=1;i<=(int)len && i <= nyears;i++)
			cf.at(cf_line, i) = min( scale*p[i-1], max );
	}

	double taxable_incentive_income(int year, const std::string &fed_or_sta)
	{
		double ti = 0.0;
		if (year==1) 
		{
			if ( as_boolean("ibi_fed_amount_tax_"+fed_or_sta) ) ti += ibi_fed_amount;
			if ( as_boolean("ibi_sta_amount_tax_"+fed_or_sta) ) ti += ibi_sta_amount;
			if ( as_boolean("ibi_uti_amount_tax_"+fed_or_sta) ) ti += ibi_uti_amount;
			if ( as_boolean("ibi_oth_amount_tax_"+fed_or_sta) ) ti += ibi_oth_amount;
		
			if ( as_boolean("ibi_fed_percent_tax_"+fed_or_sta) ) ti += ibi_fed_per;
			if ( as_boolean("ibi_sta_percent_tax_"+fed_or_sta) ) ti += ibi_sta_per;
			if ( as_boolean("ibi_uti_percent_tax_"+fed_or_sta) ) ti += ibi_uti_per;
			if ( as_boolean("ibi_oth_percent_tax_"+fed_or_sta) ) ti += ibi_oth_per;

			if ( as_boolean("cbi_fed_tax_"+fed_or_sta) ) ti += cbi_fed_amount;
			if ( as_boolean("cbi_sta_tax_"+fed_or_sta) ) ti += cbi_sta_amount;
			if ( as_boolean("cbi_uti_tax_"+fed_or_sta) ) ti += cbi_uti_amount;
			if ( as_boolean("cbi_oth_tax_"+fed_or_sta) ) ti += cbi_oth_amount;
		}

		if ( as_boolean("pbi_fed_tax_"+fed_or_sta) ) ti += cf.at( CF_pbi_fed, year );
		if ( as_boolean("pbi_sta_tax_"+fed_or_sta) ) ti += cf.at( CF_pbi_sta, year );
		if ( as_boolean("pbi_uti_tax_"+fed_or_sta) ) ti += cf.at( CF_pbi_uti, year );
		if ( as_boolean("pbi_oth_tax_"+fed_or_sta) ) ti += cf.at( CF_pbi_oth, year );

		return ti;
	}
	
	void depreciation_sched_macrs_half_year( int cf_line, int nyears )
	{
		for (int i=1; i<=nyears; i++)
		{
			double factor = 0.0;
			switch(i)
			{
			case 1: factor = 0.2000; break;
			case 2: factor = 0.3200; break;
			case 3: factor = 0.1920; break;
			case 4: factor = 0.1152; break;
			case 5: factor = 0.1152; break;
			case 6: factor = 0.0576; break;
			default: factor = 0.0; break;
			}
			cf.at(cf_line, i) = factor;
		}
	}

	void depreciation_sched_straight_line( int cf_line, int nyears, int slyears )
	{
		double depr_per_year = (slyears!=0) ? 1.0 / ((double)slyears) : 0;
		for (int i=1; i<=nyears; i++)
			cf.at(cf_line, i) = (i<=slyears) ? depr_per_year : 0.0;
	}
	
	void depreciation_sched_custom( int cf_line, int nyears, ssc_number_t *custp, int custp_len )
	{
		// note - allows for greater than or less than 100% depreciation - warning to user in samsim
		if (custp_len < 2)
		{
			cf.at(cf_line, 1) = custp[0]/100.0;
		}
		else
		{
			for (int i=1;i<=nyears && i-1 < custp_len;i++)
				cf.at(cf_line, i) = custp[i-1]/100.0;
		}
	}

	void escal_or_annual( int cf_line, int nyears, const std::string &variable, 
			double inflation_rate, double scale, bool as_rate=true, double escal = 0.0)
	{
		size_t count;
		ssc_number_t *arrp = as_array(variable, &count);

		if (as_rate)
		{
			if (count == 1)
			{
				escal = inflation_rate + scale*arrp[0];
				for (int i=0; i < nyears; i++)
					cf.at(cf_line, i+1) = pow( 1+escal, i );
			}
			else
			{
				for (int i=0; i < nyears && i < (int)count; i++)
					cf.at(cf_line, i+1) = 1 + arrp[i]*scale;
			}
		}
		else
		{
			if (count == 1)
			{
				for (int i=0;i<nyears;i++)
					cf.at(cf_line, i+1) = arrp[0]*scale*pow( 1+escal+inflation_rate, i );
			}
			else
			{
				for (int i=0;i<nyears && i<(int)count;i++)
					cf.at(cf_line, i+1) = arrp[i]*scale;
			}
		}
	}

	double min(double a, double b)
	{ // handle NaN
		if ((a != a) || (b != b))
			return 0;
		else
			return (a < b) ? a : b;
	}

	double max(double a, double b)
	{ // handle NaN
		if ((a != a) || (b != b))
			return 0;
		else
			return (a > b) ? a : b;
	}

};

DEFINE_MODULE_ENTRY( cashloan, "Residential/Commerical Finance model.", 1 );
