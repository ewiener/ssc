#ifndef _LIB_FUEL_CELL_DISPATCH_
#define _LIB_FUEL_CELL_DISPATCH_

#include "lib_fuel_cell.h"
#include "lib_util.h"

class FuelCellDispatch
{
public:
	/// Default constructor
	FuelCellDispatch() { /* Nothing to do */ };

	/// Construct with arguments
	FuelCellDispatch(FuelCell * fuelCell, size_t numberOfUnits, int dispatchOption, int shutdownOption, double dt_hour, 
		double fixed_percent, 
		std::vector<bool> canCharge,
		std::vector<bool> canDischarge,
		std::map<size_t, double> discharge_percent, 
		util::matrix_t<size_t> scheduleWeekday,
		util::matrix_t<size_t> scheduleWeekend);

	/// Destructor
	~FuelCellDispatch() { /* Nothing to do */ };

	/// Run dispatch for single step
	void runSingleTimeStep(size_t hour_of_year, double powerSystem_kWac=0, double powerLoad_kWac=0);

	/// Update dispatch option (for testing)
	void setDispatchOption(int dispatchOption);

	/// Dispatch option enumerations
	enum FC_DISPATCH_OPTION { FIXED, LOAD_FOLLOW, MANUAL };

private:

	size_t m_numberOfUnits;
	int m_dispatchOption;
	int m_shutdownOption;
	double dt_hour;
	double m_fixed_percent;
	std::vector< FuelCell *> m_fuelCellVector;

	std::vector<bool> m_canCharge;
	std::vector<bool> m_canDischarge;
	std::map<size_t, double> m_discharge_percent;
	util::matrix_t<size_t> m_scheduleWeekday;
	util::matrix_t<size_t> m_scheduleWeekend;
};

#endif
