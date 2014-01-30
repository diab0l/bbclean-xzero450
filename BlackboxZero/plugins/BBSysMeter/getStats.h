/***********************************************************************
**
** Module:		getStats.h
**
** Description:	This module implements a system status monitor plugin.
**
** Author:		Paul Wells (pdw63)
**
** Created:		07/19/99
**
** Modified:	
**
**				Copyright (C) 1999 Paul Wells
**				All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
** 
***********************************************************************/

#ifndef __get_stats_h
#define __get_stats_h

class CGetStats
{
public:
					CGetStats();
	virtual			~CGetStats();

	virtual void	GetData(UINT& iPerCentCPU) = 0;//, UINT& iSwapMemUsed, UINT& iPhysMemUsed) = 0;

//	void			GetMemData (UINT& iSwapMemUsed, UINT& iPhysMemUsed);
//	LPMEMORYSTATUS	GetMemStat(void) { return &m_statMEM; }
	
	void			GetBatData (UINT& iPerCentBAT, bool& bACOnline, bool& bBATCharging);  // add by dllee 
	bool			GetBatData (void);
	void			GetProcessNumber (UINT& iProcessNumber);

private:
	MEMORYSTATUS	m_statMEM;

};

class CGet9XStats : public CGetStats
{
public:
	CGet9XStats();
	~CGet9XStats();
	void GetData(UINT& iPerCentCPU);//, UINT& iSwapMemUsed, UINT& iPhysMemUsed);

private:
};

class CGetNTStats : public CGetStats
{
public:
	CGetNTStats();
	~CGetNTStats();
	void GetData(UINT& iPerCentCPU);//, UINT& iSwapMemUsed, UINT& iPhysMemUsed);

private:
	LARGE_INTEGER	m_liOldIdleTime; 
	LARGE_INTEGER	m_liOldSystemTime; 
};

#endif // __get_stats_h