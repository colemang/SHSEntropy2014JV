#include "WPILib.h"
#include "IODefinitions.h"
#include "EntropyDrive.h"
#include "EntropyJoystick.h"
#include "ExampleSHS.h"
#include "GenericHID.h"
#include "vision.h"
#include "configuration.h"

#define cCyclesToWait 20

class EntropyRobot2014 : public IterativeRobot
{
	// Declare variable for the robot system
	enum Calibrate
	{
		cNone,
		cHTop,
		cHBottom,
		cSTop,
		cSBottom,
		cVTop,
		cVBottom,
		cAutoDrive,
		cMax,
	};
		
	// Declare a variable to use to access the driver station object
	DriverStation *EntropyDriverStation;			// driver station object
	UINT32 m_priorPacketNumber;					// keep track of the most recent packet number from the DS
	UINT8 m_dsPacketsReceivedInCurrentSecond;	// keep track of the ds packets received in the current second
	
	// Declare variables for the two joysticks being used
	EntropyJoystick *DriveStick;			// EntropyJoystick used for robot driving
	EntropyJoystick *GameStick;			// EntropyJoystick for all other functions		
	
	//Output to Driver Station;
	DriverStationLCD *ds; 
	
	// Declare SHS Subsystems here
	EntropyDrive MyRobot;		// The Robot Drive instance
	Configuration *m_Configuration;
		
	
	// Local variables to count the number of periodic loops performed
	UINT32 m_autoPeriodicLoops;
	UINT32 m_disabledPeriodicLoops;
	UINT32 m_telePeriodicLoops;

	int m_Constant[ cMax];
	

public:
/**
 * Constructor for this "EntropyRobotDrive2014" Class.
 */
	EntropyRobot2014(void)	{
		printf(" Constructor Started\n");

		// Establish Hardware IO Controllers
		DriveStick = new EntropyJoystick(IODefinitions::USB_PORT_1);
		GameStick = new EntropyJoystick(IODefinitions::USB_PORT_2);
		m_Configuration = NULL;
		InitializeConfiguration();

		// Acquire the Driver Station object
		EntropyDriverStation = DriverStation::GetInstance();
		m_priorPacketNumber = 0;
		m_dsPacketsReceivedInCurrentSecond = 0;

		// Initialize counters to record the number of loops completed in autonomous and teleop modes
		m_autoPeriodicLoops = 0;
		m_disabledPeriodicLoops = 0;
		m_telePeriodicLoops = 0;

		ds = DriverStationLCD::GetInstance();
		
		printf("EntropyBot14 Constructor Completed\n");
	}
	
	
	/********************************** Init Routines *************************************/

	void RobotInit(void) {
		// Actions which would be performed once (and only once) upon initialization of the
		// robot would be put here.
		
		
		// Initialize SHS Subsystems here
		MyRobot.Initialize();
		
		
		printf("RobotInit() completed.\n");
	}
	
	void DisabledInit(void) {
		m_disabledPeriodicLoops = 0;			// Reset the loop counter for disabled mode
		
		// Move the cursor down a few, since we'll move it back up in periodic.
		printf("\x1b[2B");
	}

	void AutonomousInit(void) {
		m_autoPeriodicLoops = 0;				// Reset the loop counter for autonomous mode
		
	}

	void TeleopInit(void) {
		m_telePeriodicLoops = 0;				// Reset the loop counter for teleop mode
		m_dsPacketsReceivedInCurrentSecond = 0;	// Reset the number of dsPackets in current second
				
		printf("Telop Init completed.\n");
	}

	/********************************** Periodic Routines *************************************/
	
	void DisabledPeriodic(void)  {
		static INT32 printSec = (INT32)GetClock() + 1;
		static const INT32 startSec = (INT32)GetClock();


		// increment the number of disabled periodic loops completed
		m_disabledPeriodicLoops++;
		
		//Disable Drive
		MyRobot.Cleanup();
		//MyShooter.Cleanup();
		
		// while disabled, printout the duration of current disabled mode in seconds
		if (GetClock() > printSec) {
			// Move the cursor back to the previous line and clear it.
			printf("\x1b[1A\x1b[2K");
			printf("Disabled seconds: %d\r\n", printSec - startSec);			
			printSec++;
		}
	}

	void AutonomousPeriodic(void) {
		m_autoPeriodicLoops++;
#if 0
		static int Clock=0;
		bool correct = DriveStick->GetButton(Joystick::kTriggerButton);
		bool Reset = DriveStick->GetButton (Joystick::kTopButton);
		ds->PrintfLine(DriverStationLCD::kUser_Line1, "%s %s",
				correct ? "correct on" : "correct off",
						Reset ? "Reset": "No Reset");
		//ds->PrintfLine(DriverStationLCD::kUser_Line6, "%d %c %c", Clock, correct? "C" : "c", Reset? "R" : "r");
		if (Reset)
			Clock=0, MyRobot.ResetCounters();
		else
		{
		    ++Clock;
		    if(Clock<=100)       MyRobot.DriveRobot(1.0,0.0, ds, correct);		// drive forward
		    else if (Clock<=200) MyRobot.DriveRobot(-1.0,0.0, ds, correct);  // stop
		    else if (Clock<=250) MyRobot.DriveRobot(-1.0,0.0, ds, correct);  // drive back halfway
		    else if (Clock<=300) MyRobot.DriveRobot(1.0,0.0, ds, correct);   // stop
		    else
		    {
		    	// Real teleop mode: use the JoySticks to drive
		    	MyRobot.DriveRobot(DriveStick->GetY(),(DriveStick->GetX()), ds);
		    }
		}
#endif
	}
	
	class AutoDrive{
		unsigned int Distance;
		unsigned int t;
		bool Reverse;
	public:
		AutoDrive(unsigned int D =1000, bool r=false) : Distance(D), t(0), Reverse(r) {}
		void Periodic(EntropyDrive& MyRobot, DriverStationLCD* ds)
		{
			unsigned int ctrs = (unsigned)(Reverse? -1 : 1) * MyRobot.GetCounters();
			ds->PrintfLine(DriverStationLCD::kUser_Line2, "c%d d%d t%d ",ctrs, Distance, t);
			if (ctrs < (Distance - t))
				MyRobot.DriveRobot(Reverse?1.0:-1.0, 0.0, ds);
			else
				MyRobot.DriveRobot(0.0, 0.0, ds);
			if (t == 0)
			{
				if (MyRobot.AtTopSpeed(Reverse)) t = ctrs;
				if (ctrs > Distance/2) t = ctrs;
				// if t is just way bigger than we meant to travel, set it to the distance (which will stop us)
				if (t >= Distance) t = Distance;
			}
		}
	};
	void TeleopPeriodic(void) {
		// increment the number of teleop periodic loops completed
		m_telePeriodicLoops++;
		static AutoDrive *autoDrive = NULL;
		bool autoButton = DriveStick->GetButton(Joystick::kTriggerButton);
		if (autoButton)
		{
			if (autoDrive == NULL)
				autoDrive = new AutoDrive(100*m_Configuration->GetValue(m_Constant[cAutoDrive]));
			autoDrive->Periodic(MyRobot, ds);
			ds->PrintfLine (DriverStationLCD::kUser_Line6, "autoDrive on");
		}
		else
		{
			ds->PrintfLine (DriverStationLCD::kUser_Line6, "autoDrive off");
			if (autoDrive != NULL)
			{
				MyRobot.ResetCounters();
				delete autoDrive;
				autoDrive = NULL;
			}

			if( !m_Configuration)
			{
				printf( "Configuration Initialize");
				InitializeConfiguration();
			}
			
			m_Configuration->Execute( DriveStick->GetRawButton( 2), DriveStick->GetZ(), ds);
			
			if(DriveStick->GetTrigger())
			{
				ds->PrintfLine(DriverStationLCD::kUser_Line6, "Calculating distance...");
				Vision *vision = new Vision();
				double distance = vision->TakeDistancePicture( ds, m_Configuration->GetValue( m_Constant[ cHBottom]), m_Configuration->GetValue( m_Constant[ cHTop]), m_Configuration->GetValue( m_Constant[ cSBottom]), m_Configuration->GetValue( m_Constant[ cSTop]), m_Configuration->GetValue( m_Constant[ cVBottom]), m_Configuration->GetValue( m_Constant[ cVTop]));
				if( distance < 0.000001)
					ds->PrintfLine(DriverStationLCD::kUser_Line6, "No target found");
				else
					ds->PrintfLine(DriverStationLCD::kUser_Line6, "distance to target: %lf",distance);
				delete vision;
			}

			
			// Real teleop mode: use the JoySticks to drive
				MyRobot.DriveRobot(DriveStick->GetY(),(DriveStick->GetX()), ds);
		}

    	ds->UpdateLCD();
	} // TeleopPeriodic(void)
	
	void InitializeConfiguration()
	{
		m_Configuration = new Configuration( 1);

		m_Constant[ cHBottom] = m_Configuration->AddOption( "H Bottom");
		m_Constant[ cHTop] = m_Configuration->AddOption( "H Top", 255);
		m_Constant[ cSBottom] = m_Configuration->AddOption( "S Bottom");
		m_Constant[ cSTop] = m_Configuration->AddOption( "S Top", 255);
		m_Constant[ cVBottom] = m_Configuration->AddOption( "V Bottom");
		m_Constant[ cVTop] = m_Configuration->AddOption( "V Top", 255);
		m_Constant[ cAutoDrive] = m_Configuration->AddOption( "AutoDrive", 1, 100);
		
	}

	
	
	
/********************************** Continuous Routines *************************************/

	/* 
	void DisabledContinuous(void) {
	}

	void AutonomousContinuous(void)	{
	}

	void TeleopContinuous(void) {
	}
	*/

	

			
};

START_ROBOT_CLASS(EntropyRobot2014);
