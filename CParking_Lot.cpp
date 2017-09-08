/*
 * Parkinglot.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: bindus
 */

/*
 * # Design a parking lot that displays the time when next empty slot will be available.
 * # Assumptions :
 * # Cars enter and  purchase time in range of SLOT_MINLIMIT to SLOT_MAXLIMIT,( 15 minutes to 8 hrs )
 * # Assume cars do leave before OR at expiry of purchased time and thus a slot becomes available
 * # Finite number of slots available : MAX_SLOTS
 * # In most real life parking lots, cars need to exit and enter again to buy/add more time
 * # So cant have method/interface to add time to an existing car in a slot
 * # Perhaps can enhance later to also handle the exception of towing/removing expired cars
 * # A timer function can peek at the first few entries in the Ticket Mapping to get ridd of sqautters
 * # CTicket can benefit with more attributes to keep information to record revenue generated, predict pattern of usage and persist all this to a Database ?
 * # Main classes : CParking_Lot(can be a Singleton?)  HAS  fixed #CSlots, issues CTicket and assign CSlot to CVehicles as they enter
 * # CParking_Lot maintains a mapping of CTickets issueed to CVehicles by ascending expiry time and a mapping of CSlots id to  CVehicles parked there
 * # CVehicles enter ( CParking_Lot) the parking lot , obtain a ticket
 * # CVehicles then park in a CSlot in the next available slot  and claim CSlot
 * # CVehicles can leave(before or at  expiry time) , presenting CTickets at exit, which invalidates CTicket, CVehicles to CSlot mapping.
 * # Anytime a vehicle enters or leave(at/before expiry), the availability is displayed
 * *
*/

#include<iostream>
#include<stdexcept>
#include <new>
#include<memory>
#include<queue>
#include <bits/stdc++.h>
#include <ctime>
#include<time.h>

using namespace std;

#define MAX_SLOTS 50
#define SEC_INA_MIN 60
#define SLOT_MAXLIMIT 60 *  8
#define SLOT_MINLIMIT 15


class CSlot {

private:
	unsigned long m_Slot_Id;
	bool m_handicapped;
	bool m_occupied;
public:

	CSlot(unsigned int id, bool type);
	void setClaimed(bool status);
	bool IsOccupied();
	unsigned long GetID();
};


CSlot::CSlot(unsigned int id, bool type){
		m_Slot_Id = id;
		m_handicapped =  type;
		m_occupied = false;
}


inline
void
CSlot::setClaimed(bool status ){
	m_occupied = status;
}


inline
unsigned long
CSlot::GetID(){
	return m_Slot_Id;
}

inline
bool
CSlot::IsOccupied(){

	return m_occupied;
}


class CTicket {

private:
	// car entrance time
	time_t m_arrivaltime;

	// arrival time + purchased time units
	time_t  m_endtime;

	// time units in secs purchased
	double  m_time_purhcased;

	//treat this like key in ticket to car mapping
	//will be unique since time managed in seconds.
	double m_expiry_key;

	// to catch expired slots/violations
	bool m_valid;

public:

	CTicket(const double minutes_purhcased);
	~CTicket();
	double getExpiry();
	void getExpiry(time_t & endtime);
};


CTicket::CTicket(const double minutes_purhcased) {

	time(&m_arrivaltime);

	m_time_purhcased  = minutes_purhcased * SEC_INA_MIN;

	m_endtime = m_arrivaltime + m_time_purhcased;

	// ignores leap seconds, unless enabled on platform
	m_expiry_key = difftime(m_endtime,m_arrivaltime);
	m_valid = 1;
}

CTicket::~CTicket(){

}

void CTicket::getExpiry(time_t & endtime){

	endtime = m_endtime;

}
double CTicket::getExpiry(){
	return m_expiry_key;
}


class CVehicle {

private:

	//string m_win_no;//unused for now unless towing implemented!
	CTicket * m_pCTicket;
	CSlot * m_pCSlot;

public:

	CVehicle();
	~CVehicle();
	void  setTicket(CTicket * pCTicket);
	void invalidateTicket();
	double PresentTicket();
	void  AssignSlot(CSlot * pCSlot);
	void ReleaseSlot(CSlot ** pCSlot);
	unsigned long CurrSlot();
};


CVehicle::CVehicle(){

	m_pCTicket = NULL;
	m_pCSlot = NULL;
}

CVehicle::~CVehicle(){

 //tickets & slots managed by ~CParking_lot
 m_pCTicket = NULL;
 m_pCSlot = NULL;

}



inline unsigned long
CVehicle::CurrSlot(){

	if(m_pCSlot)
		return(m_pCSlot->GetID());
	else
		return 0;
}


inline void
CVehicle::setTicket(CTicket * pCTicket){

	this->m_pCTicket = pCTicket;
}


inline void
CVehicle::invalidateTicket(){
	this->m_pCTicket = NULL;
}


inline void
CVehicle::AssignSlot(CSlot* pCSlot){

	pCSlot->setClaimed(true);
	this->m_pCSlot = pCSlot;
}

void
CVehicle::ReleaseSlot(CSlot ** pCSlot){


	this->m_pCSlot->setClaimed(false);
	*pCSlot = this->m_pCSlot;
	this->m_pCSlot = NULL;
}

inline double
CVehicle::PresentTicket(){

	return this->m_pCTicket->getExpiry();
}

typedef map<double, CTicket*>::iterator itrTicket;
typedef std::pair<double,CTicket*> ticket_pair;

typedef map<int, CVehicle *>::iterator itrVehicle;
typedef std::pair<unsigned long, CVehicle *> vehicle_pair;

//main class manages slots  and  tickets
class CParking_Lot {

	// slots not occupied
	unsigned int m_free_slots;

	//associative container key-value pair with expirytime (double) as key and Ticket as associated data, mapping in ascending order of expirytime
	// since stl map container is implemented using a red-black tree, they have the following asymptotic run times:
	// Insertion: O(log n)
	// Lookup: O(log n)
	// Deletion: O(log n)

	multimap<double, CTicket*> m_TicketsIssued;

	//slotid and vehicle in that slot when a car is parked.
	map<int, CVehicle *> m_VehiclesParked;

	//Finite number of Slots with fixed attributes when CParking_Lot is created.

	map<int, CSlot*> m_FreeSlots;

	//Time to display when next unoccupied/free slot available
	time_t m_slot_avail_time;


	bool IssueTicket(CVehicle * pCVehicle, int minutes);
	void UpdateAvailabilty();
	void SanityCheck();

public:

	CParking_Lot();
	~CParking_Lot();
	bool VehicleEnters(CVehicle * pCVehicle, int  minutes);
	bool ParkVehicle(CVehicle * CVehicle);
	bool VehicleLeaves(CVehicle * pCVehicle);
	void DbgDisplay();

};

typedef map<int, CSlot *>::iterator itrSlot;
typedef std::pair<unsigned long, CSlot *> slot_pair;

CParking_Lot::CParking_Lot(){

	m_TicketsIssued.clear();
	m_VehiclesParked.clear();
	m_free_slots = MAX_SLOTS;

	int i;
	//create the slots
	try {
		for (i=1; i <= MAX_SLOTS; i++){
			CSlot * pCSlot = new CSlot( i, false);
			m_FreeSlots.insert(slot_pair(pCSlot->GetID(), pCSlot));
		 }
	}
	catch(const std::bad_alloc& e){
			std::cout << "Allocation failed: " << e.what() << endl;
	}
	UpdateAvailabilty();
}

CParking_Lot::~CParking_Lot(){

	//free the slots here
	itrSlot ItrS;
	for (ItrS = m_FreeSlots.begin(); ItrS != m_FreeSlots.end(); ItrS++){
			delete ItrS->second;
	}
	m_FreeSlots.clear();

	//free all other transient objects that may have remained.
	itrTicket itrTicket;
	for (itrTicket = m_TicketsIssued.begin(); itrTicket != m_TicketsIssued.end(); itrTicket++){
		delete itrTicket->second;
	}
	m_TicketsIssued.clear();

	// we dont delete the CVehicle as these obj are not created/managed by CParking_Lot
	m_VehiclesParked.clear();
}

inline void
CParking_Lot::SanityCheck(){
#ifdef DBG
			//m_free_slots is  updated soon as getting vehicle enters && parks
			assert(MAX_SLOTS - m_free_slots)  = m_VehiclesParked.size());
			assert(m_TicketsIssued.size() == m_VehiclesParked.size());
#endif

}

//Find a free slot to assign to a car and updates mapping
//vehciles must park after getting tickets to preserve the symantics !
bool
CParking_Lot::ParkVehicle(CVehicle * pCVehicle){

	bool bret = false;
	CSlot * pCSlot = NULL;
	unsigned long slotid = 0;
	//get a free slot to assign
	itrSlot itrS = m_FreeSlots.begin();
	if(itrS != m_FreeSlots.end() ) {
		pCSlot = itrS->second;
	}

	slotid =  pCSlot->GetID();
	itrVehicle end  = m_VehiclesParked.end();
	if (end == m_VehiclesParked.find(slotid)){

		pCVehicle->AssignSlot(pCSlot);
		m_FreeSlots.erase(itrS);
		m_VehiclesParked.insert(vehicle_pair(slotid, pCVehicle));
		cout << "Vehicles parked in slot: #" << slotid << endl;
		m_free_slots--;
		//SanityCheck();
		bret =  true;
	}
	return bret;
}

//Display current availabilty status
void
CParking_Lot::UpdateAvailabilty(){

	struct tm * timeinfo;
	//assert(m_free_slots ==  (MAX_SLOTS - m_TicketsIssued.size()));
	if (m_free_slots){
		time(&m_slot_avail_time);
		timeinfo = localtime(&m_slot_avail_time);
		std::cout << " Next Slot(s): #" << m_free_slots <<" available at: " << asctime(timeinfo) << endl;

	} else {
		itrTicket itr = m_TicketsIssued.begin();
		if(itr != m_TicketsIssued.end() ){
			itr->second->getExpiry(m_slot_avail_time);
			timeinfo = localtime(&m_slot_avail_time);
			std::cout << " Next Slot available at: " << asctime(timeinfo) << endl;
		}
	}
}

//creates a ticket based on min requested
bool CParking_Lot::IssueTicket(CVehicle * pCVehicle, int  minutes){

	bool bret = false;
	if((minutes < SLOT_MINLIMIT ) && (minutes > SLOT_MAXLIMIT) )
		return bret;

	try {
			if (m_free_slots){
				CTicket * pCTicket = new CTicket(minutes);
				pCVehicle->setTicket(pCTicket);
				 m_TicketsIssued.insert(ticket_pair(pCTicket->getExpiry(), pCTicket));
				bret = true;
			}
	}
	catch(const std::bad_alloc& e){
		std::cout << "Allocation failed: " << e.what() << endl;
	}

	catch(const std::exception &e){

		std::cout << "std::exception: " << e.what() << endl;
	}

	return bret;
}

// Create ticket and assigns a parking slot
//Display change ins status
bool
CParking_Lot::VehicleEnters(CVehicle * pCVehicle, int  minutes){

	if(!pCVehicle)
		return false;

	if(IssueTicket(pCVehicle,minutes)){
		if( ParkVehicle(pCVehicle)) {
			UpdateAvailabilty();
			return true;
		}
	}
	return false;
}

// Invalidate ticket and release parking slot
//Display change ins status
bool
CParking_Lot::VehicleLeaves(CVehicle * pCVehicle){

	bool bret = false;
	assert(pCVehicle);
	itrTicket itr;
	CSlot * pCSlot = NULL;
	itr = m_TicketsIssued.find(pCVehicle->PresentTicket());
	if (itr != m_TicketsIssued.end()){
		delete itr->second; //ticket no longer valid so destroy
		m_TicketsIssued.erase(itr);
		pCVehicle->invalidateTicket();
	}
	//update slot mapping

	itrVehicle Itr = m_VehiclesParked.find(pCVehicle->CurrSlot());
	if(Itr !=  m_VehiclesParked.end()){

		pCVehicle->ReleaseSlot(&pCSlot); // retuun slot free to free pool
		m_FreeSlots.insert(slot_pair(pCSlot->GetID(), pCSlot));
		//we dont destroy the vehicle because CParking_Lot did  not create/ have ownership of it.
		m_VehiclesParked.erase(Itr);
		cout << "Vehicles leaving slot: #" << pCSlot->GetID() << endl;
		m_free_slots++;
		UpdateAvailabilty();
		bret = true;
	}
	return bret;
}

//utility fucntion
void
CParking_Lot::DbgDisplay(){

	cout << endl;
	cout << "curr free slots: #" << m_free_slots << endl;
	cout << "cars parked: #" << m_VehiclesParked.size() << endl;
	cout << "tickets issued: #" << m_TicketsIssued.size() << endl;
	cout << endl;
}

//purpose : test driver
int main(int argc, char * argcv[]){

	CParking_Lot oCParking_Lot;

	oCParking_Lot.DbgDisplay();

	CVehicle * pCVehicle1 = new CVehicle;
	CVehicle * pCVehicle2 = new CVehicle;

	oCParking_Lot.VehicleEnters(pCVehicle1, 120);
	oCParking_Lot.VehicleEnters(pCVehicle2, 360);
	oCParking_Lot.VehicleLeaves( pCVehicle2);

	oCParking_Lot.DbgDisplay();

	delete pCVehicle1;
	delete pCVehicle2;
}
