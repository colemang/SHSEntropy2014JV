#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define gift


class Configuration
{
public:
	struct Option
	{
		char *name;
		int max;
		int increment;
		int value;
	};
	
public:
	int m_DriveStationLine;
	
public:
	Configuration( int line);	
	~Configuration();
	
	int AddOption( char *name, int value = 0, int max = 255, int increment = 10);
	void Execute( bool button, float stick, DriverStationLCD *ds);
	int GetValue( int index);
	int GetOptionCount(){ return m_OptionCount;}

protected:
	Option **m_Options;
	int m_OptionCount;
	int m_CurrentOption;
	int m_ButtonDown;
	int m_WaitCycles;	

	int AddOption( Option gift *option);
	void AdjustCalibration( int adjustment);
	
};
	
	


#endif
