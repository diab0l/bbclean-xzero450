/***********************************************************************
**
** Module:		getNTStats.cpp
**
** Description:	This module implements a system	status monitor plugin on NT.
**
** Author:		Paul Wells (pdw63)
**
** Created:		07/19/99
**
** Modified:	
**
**				Copyright (C) 1999 Paul	Wells
**				All Rights Reserved
**
** This	program	is free	software; you can redistribute it and/or modify
** it under the	terms of the GNU General Public	License	as published by
** the Free Software Foundation; either	version	2 of the License, or
** (at your option) any	later version.
** 
** This	program	is distributed in the hope that	it will	be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,	USA.
** 
***********************************************************************/

//-------------------------------------------------------------------------
// Removed some things to make the GetData procedure faster - Theo 08/04/04
//-------------------------------------------------------------------------

#include <windows.h>
#include "getStats.h"
//#include <psapi.h>		// for EnumProcesses()

#define __ENABLE_PROCESS_NUMBER__


#define	SystemBasicInformation			0 
#define	SystemPerformanceInformation	2 
#define	SystemTimeInformation			3 

#define	Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart)) 

typedef	struct 
{ 
	DWORD	dwUnknown1; 
	ULONG	uKeMaximumIncrement; 
	ULONG	uPageSize; 
	ULONG	uMmNumberOfPhysicalPages; 
	ULONG	uMmLowestPhysicalPage; 
	ULONG	uMmHighestPhysicalPage;	
	ULONG	uAllocationGranularity;	
	PVOID	pLowestUserAddress; 
	PVOID	pMmHighestUserAddress; 
	ULONG	uKeActiveProcessors; 
	BYTE	bKeNumberProcessors; 
	BYTE	bUnknown2; 
	WORD	wUnknown3; 
} SYSTEM_BASIC_INFORMATION; 

typedef	struct 
{ 
	LARGE_INTEGER	liIdleTime; 
	DWORD			dwSpare[76]; 
} SYSTEM_PERFORMANCE_INFORMATION; 

typedef	struct 
{ 
	LARGE_INTEGER	liKeBootTime; 
	LARGE_INTEGER	liKeSystemTime;	
	LARGE_INTEGER	liExpTimeZoneBias; 
	ULONG			uCurrentTimeZoneId; 
	DWORD			dwReserved; 
} SYSTEM_TIME_INFORMATION; 

// ntdll!NtQuerySystemInformation (NT specific!) 
// 
// The function	copies the system information of the 
// specified type into a buffer	
// 
// NTSYSAPI 
// NTSTATUS 
// NTAPI 
// NtQuerySystemInformation( IN		UINT	SystemInformationClass,	 // information	type 
//							 OUT	PVOID	SystemInformation,		 // pointer to buffer 
//							 IN		ULONG	SystemInformationLength, // buffer size	in bytes 
//							 OUT	PULONG	ReturnLength OPTIONAL ); // pointer to a 32-bit	
//																	 // variable that receives 
//																	 // the	number of bytes	
//																	 // written to the buffer 
//  

typedef	LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG); 

PROCNTQSI NtQuerySystemInformation; 

SYSTEM_BASIC_INFORMATION	m_SysBaseInfo; 


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// CGetStats
//
// Intialize our class.
//
CGetStats::CGetStats()
{
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ~CGetStats
//
// Terminate our class.
//
CGetStats::~CGetStats()
{
}


/*void CGetStats::GetMemData (UINT& iSwapMemUsed,	UINT& iPhysMemUsed)
{
	DWORD			dwSwapMemPerCent;
	DWORD			dwPhysMemPerCent;

	GlobalMemoryStatus (&m_statMEM);

	dwSwapMemPerCent = (DWORD)(100.0 - ((m_statMEM.dwAvailPageFile * 100.0) /	m_statMEM.dwTotalPageFile));

	iSwapMemUsed = (UINT) dwSwapMemPerCent;

	dwPhysMemPerCent = (DWORD)(100.0 - ((m_statMEM.dwAvailPhys * 100.0) / m_statMEM.dwTotalPhys));
	
	iPhysMemUsed = (UINT) dwPhysMemPerCent;
}*/

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GetBatData	Getting	Battery	Data		  add by dllee
//
// if no battery, return false
void CGetStats::GetBatData (UINT& iPerCentBAT, bool& bACOnline,	bool& bBATCharging)
{
	SYSTEM_POWER_STATUS s;
	
	iPerCentBAT=0;
	bACOnline=false;
	bBATCharging=false;

	if(GetSystemPowerStatus(&s))
	{
		bACOnline	=(s.ACLineStatus==1);
		bBATCharging=((s.BatteryFlag & BATTERY_FLAG_CHARGING)==BATTERY_FLAG_CHARGING);
		iPerCentBAT =(UINT) s.BatteryLifePercent;
	}
}
bool CGetStats::GetBatData (void)
{
	SYSTEM_POWER_STATUS s;

	return (GetSystemPowerStatus(&s)!=0);
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GetProcessNumber	Getting	Number of Process (psapi.dll)	  add by dllee
//
/*void CGetStats::GetProcessNumber (UINT& iProcessNumber)
{
	iProcessNumber=0;
//#ifdef __ENABLE_PROCESS_NUMBER__
	DWORD aProcesses[1024], cbNeeded;
	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return;

	// Calculate how many process IDs were returned
	iProcessNumber = cbNeeded / sizeof(DWORD);
//#endif
}
*/

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// CGet9XStats
//
CGet9XStats::CGet9XStats()
{
	HKEY	hkey; 
	DWORD	dwDataSize; 
	DWORD	dwType;	
	DWORD	dwCpuUsage; 

	// starting the	counter	
	if ( RegOpenKeyEx( HKEY_DYN_DATA, 
					   "PerfStats\\StartStat", 
					   0,KEY_ALL_ACCESS, 
					   &hkey ) != ERROR_SUCCESS) 
		return;	

	dwDataSize = sizeof(DWORD); 

	RegQueryValueEx( hkey, 
					 "KERNEL\\CPUUsage", 
					 NULL,&dwType, 
					 (LPBYTE)&dwCpuUsage, 
					 &dwDataSize );	

	RegCloseKey (hkey); 

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ~CGet9XStats
//
CGet9XStats::~CGet9XStats()
{
	HKEY	hkey; 
	DWORD	dwDataSize; 
	DWORD	dwType;	
	DWORD	dwCpuUsage; 

	// stoping the counter 
	if ( RegOpenKeyEx( HKEY_DYN_DATA, 
					   "PerfStats\\StopStat", 
					   0,KEY_ALL_ACCESS, 
					   &hkey ) != ERROR_SUCCESS) 
	   return; 

	dwDataSize = sizeof(DWORD); 

	RegQueryValueEx( hkey, 
					 "KERNEL\\CPUUsage", 
					 NULL,&dwType, 
					 (LPBYTE)&dwCpuUsage, 
					 &dwDataSize );	

	RegCloseKey(hkey); 
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GetData
//
void CGet9XStats::GetData(UINT&	iPerCentCPU)//, UINT& iSwapMemUsed, UINT& iPhysMemUsed)
{
	HKEY	hkey; 
	DWORD	dwDataSize; 
	DWORD	dwType;	
	DWORD	dwCpuUsage; 

	// geting current counter's value 
	if ( RegOpenKeyEx( HKEY_DYN_DATA, 
					  "PerfStats\\StatData", 
					  0,KEY_READ, 
					  &hkey	) != ERROR_SUCCESS) 
		return;	

	dwDataSize = sizeof(DWORD); 

	RegQueryValueEx( hkey, 
					 "KERNEL\\CPUUsage", 
					 NULL,&dwType, 
					 (LPBYTE)&dwCpuUsage, 
					 &dwDataSize );	

	RegCloseKey(hkey); 

	iPerCentCPU = (UINT) dwCpuUsage;

//	GetMemData (iSwapMemUsed, iPhysMemUsed);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// CGetNTStats
//
CGetNTStats::CGetNTStats()
{
	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress( GetModuleHandle("ntdll"),	"NtQuerySystemInformation" ); 

	if (!NtQuerySystemInformation) 
		return;	

	// get number of processors in the system 
	NtQuerySystemInformation(SystemBasicInformation,&m_SysBaseInfo,sizeof(m_SysBaseInfo),NULL); 

	// Initialise some variables
	m_liOldIdleTime.LowPart	= 0; 
	m_liOldIdleTime.HighPart = 0; 

	m_liOldSystemTime.LowPart = 0; 
	m_liOldSystemTime.HighPart = 0;	
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ~CGetNTStats
//
CGetNTStats::~CGetNTStats()
{
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GetData
//
void CGetNTStats::GetData(UINT&	iPerCentCPU)//, UINT& iSwapMemUsed, UINT& iPhysMemUsed)
{
	LONG							status;
	SYSTEM_TIME_INFORMATION			SysTimeInfo; 
	SYSTEM_PERFORMANCE_INFORMATION	SysPerfInfo; 
	double							dbIdleTime; 

	//
	// CPU first
	//
	// get new system time 
	status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0); 

	if (status!=NO_ERROR) 
		return;	

	// get new CPU's idle time 
	status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL); 

	if (status != NO_ERROR)	
		return;	

	// if it's a first call	- skip it 
	if (m_liOldIdleTime.QuadPart !=	0) 
	{ 
		double dbSystemTime; 

		// CurrentValue	= NewValue - OldValue 
		dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(m_liOldIdleTime); 
		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(m_liOldSystemTime); 

		// CurrentCpuIdle = IdleTime / SystemTime 
		dbIdleTime = dbIdleTime	/ dbSystemTime;	

		// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors	
		dbIdleTime = 100.0 - dbIdleTime	* 100.0	/ (double)m_SysBaseInfo.bKeNumberProcessors + 0.5; 
	} 
	else
		dbIdleTime = 0;

	// store new CPU's idle	and system time	
	m_liOldIdleTime		= SysPerfInfo.liIdleTime; 
	m_liOldSystemTime	= SysTimeInfo.liKeSystemTime; 

	iPerCentCPU = (UINT) dbIdleTime;

//	GetMemData (iSwapMemUsed, iPhysMemUsed);
}
