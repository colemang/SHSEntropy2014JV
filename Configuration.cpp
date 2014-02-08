#include "WPILib.h"
#include "IODefinitions.h"
#include "GenericHID.h"
#include "Configuration.h"

#define cMaxOptions 20
#define cCyclesToWait 20


Configuration::Configuration( int line = 1)
{
	m_DriveStationLine = line;
	m_Options = (Option **) new Option *[ cMaxOptions];
	m_OptionCount = 0;
	m_CurrentOption = 0;
	m_ButtonDown = 0;
	m_WaitCycles = 0;

	
	AddOption( "Configure", 0, 0, 0);
}

Configuration::~Configuration()
{
	for( int i = 0; i < m_OptionCount; i++)
		delete m_Options[ i];
	delete[] m_Options;
}

int Configuration::AddOption( Option gift *option)
{
	m_Options[ m_OptionCount] = option;
	return m_OptionCount++;
}

int Configuration::AddOption( char *name, int value, int max, int increment)
{
	Option *option = new Option();
	option->name = name;
	option->max = max;
	option->increment = increment;
	option->value = value;
	return AddOption( option);
}

int Configuration::GetValue( int index)
{
	return m_Options[ index]->value;
}

void Configuration::Execute( bool button, float stick, DriverStationLCD *ds)
{
	if( button && m_ButtonDown == 0)
	{
		m_CurrentOption++;;
		if( m_CurrentOption == m_OptionCount)
			m_CurrentOption = 0;
		m_ButtonDown = 2;
	}
	
	if( !button && m_ButtonDown > 0)
		m_ButtonDown = 0;
	
	m_WaitCycles++;
	if(!(m_WaitCycles % cCyclesToWait) && m_CurrentOption != 0)
	{
		int adjustment = 0;
		if( stick > .1)
			adjustment = -1;
		if( stick > .8)
			adjustment = -m_Options[ m_CurrentOption]->increment;
		if( stick < -.1)
			adjustment = 1;
		if( stick < -.8)
			adjustment = m_Options[ m_CurrentOption]->increment;
		AdjustCalibration( adjustment);
	}

	if( m_CurrentOption == 0)
		ds->PrintfLine(DriverStationLCD::kUser_Line1, "%s", m_Options[ m_CurrentOption]->name);
	else
		ds->PrintfLine(DriverStationLCD::kUser_Line1, "%s: %d", m_Options[ m_CurrentOption]->name, m_Options[ m_CurrentOption]->value);
	
}

void Configuration::AdjustCalibration( int adjustment)
{
	Option *option = m_Options[ m_CurrentOption];
	option->value += adjustment;
	if( option->value > option->max)
		option->value = option->max;
	if( option->value < 0)
		option->value = 0;
}

