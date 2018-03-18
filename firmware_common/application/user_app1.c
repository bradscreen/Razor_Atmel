/**********************************************************************************************************************
File: user_app1.c

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app1.c file template

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

/* Existing variables */
extern u32 G_u32AntApiCurrentDataTimeStamp;
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;
/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */

static LedRateType current_led_rates[ANT_APPLICATION_MESSAGE_BYTES];
static u8 led_delays[ANT_APPLICATION_MESSAGE_BYTES];

#define LED_MAX_BRIGHTNESS LED_PWM_50
#define LED_FADE_DELAY_CYCLES 2

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  -
*/
void UserApp1Initialize(void)
{
AntAssignChannelInfoType AntSetupData;
AntSetupData.AntChannel = ANT_CHANNEL_SCANNING;
AntSetupData.AntChannelPeriodHi = 0;
AntSetupData.AntChannelPeriodLo = 0;
AntSetupData.AntChannelType = CHANNEL_TYPE_SLAVE;
AntSetupData.AntDeviceIdHi = 0;
AntSetupData.AntDeviceIdLo = 0;
AntSetupData.AntDeviceType = 0;
AntSetupData.AntFrequency = 50;
AntSetupData.AntNetwork = ANT_NETWORK_DEFAULT;
for(u8 i =0; i< ANT_NETWORK_NUMBER_BYTES; i++)
{
    AntSetupData.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
}
AntSetupData.AntTransmissionType = 0;
AntSetupData.AntTxPower = RADIO_TX_POWER_4DBM;

for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
  {
  current_led_rates[i] = LED_PWM_5;
  led_delays[i] = LED_FADE_DELAY_CYCLES;
  }

  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&AntSetupData) )
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
    LedPWM(0,1);
    LedPWM(1,1);
    LedPWM(2,1);
    LedPWM(3,1);
    LedPWM(4,1);
    LedPWM(5,1);
    LedPWM(6,1);
    LedPWM(7,1);
    LedPWM(8,3);
    LedPWM(9,3);
    LedPWM(10,3);
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ??? */
static void UserApp1SM_Idle(void)
{
  static u8 count = 0;
  static bool channel_open = FALSE;

  if(count < 200)
  {
    count++;
    return;
  }
  if(!channel_open)
  {
    if(AntOpenScanningChannel())
    {
      channel_open = TRUE;
    }
  }
  if(AntRadioStatusChannel(ANT_CHANNEL_SCANNING) != ANT_OPEN)
  {
  channel_open = FALSE;
  }
  if( channel_open )
  {
    if( AntReadAppMessageBuffer() && G_eAntApiCurrentMessageClass == ANT_DATA )
      {
        for(u8 i = 0; i< ANT_APPLICATION_MESSAGE_BYTES; i++)
        {
          if( G_au8AntApiCurrentMessageBytes[i] > 0 )
          {
            if(current_led_rates[i] != LED_MAX_BRIGHTNESS)
            {
            current_led_rates[i] = LED_MAX_BRIGHTNESS;
            LedPWM(i, current_led_rates[i]);
            }

          }
          else
          {
            if(--led_delays[i] == 0 && current_led_rates[i] != LED_PWM_0)
            {
              current_led_rates[i]--;
              led_delays[i] = LED_FADE_DELAY_CYCLES;
              LedPWM(i, current_led_rates[i]);
            }
          }


        }
    }
  }
} /* end UserApp1SM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)
{

} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
