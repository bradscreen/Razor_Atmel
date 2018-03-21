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
#include "stdio.h"

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
static u8 ant_frequency = 50;

#define LED_MAX_BRIGHTNESS LED_PWM_100
#define LED_FADE_DELAY_CYCLES 50

static AntAssignChannelInfoType AntSetupData;
  static u8 count = 0;
  static bool slow = TRUE;
  static bool channel_open = FALSE;
  static u16 no_data_ticks;
  static u16 switch_counter = 0;
  static u8 msg[19];

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
  LCDCommand(LCD_CLEAR_CMD);

AntSetupData.AntChannel = ANT_CHANNEL_SCANNING;
AntSetupData.AntChannelPeriodHi = 0;
AntSetupData.AntChannelPeriodLo = 0;
AntSetupData.AntChannelType = CHANNEL_TYPE_SLAVE;
AntSetupData.AntDeviceIdHi = 0;
AntSetupData.AntDeviceIdLo = 0;
AntSetupData.AntDeviceType = 0;
//AntSetupData.AntFrequency = ant_frequency;
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

UserApp1_StateMachine = UserApp1SM_Setup;

snprintf( msg, 19, "Scan frequency:   " );
LCDMessage( LINE1_START_ADDR, msg );

//  /* If good initialization, set state to Idle */
//  if( AntAssignChannel(&AntSetupData) )
//  {
//    u8 msg[19];
//    sprintf(msg, "Scan frequency: %d", ant_frequency);
//      LCDMessage(LINE1_START_ADDR, msg);
//    UserApp1_StateMachine = UserApp1SM_Scan;
//  }
//  else
//  {
//    /* The task isn't properly initialized, so shut it down and don't run */
//    UserApp1_StateMachine = UserApp1SM_Error;
//  }

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
static void UserApp1SM_Setup(void)
{
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    AntSetupData.AntFrequency = ant_frequency;
    snprintf( msg, 19, "Scan frequency: %d", ant_frequency);
    LCDMessage(LINE1_START_ADDR, msg);
    if( AntAssignChannel(&AntSetupData) )
    {
      UserApp1_StateMachine = UserApp1SM_Scan;
    }
    else
    {
      UserApp1_StateMachine = UserApp1SM_Error;
    }
  }

  // button3 should increase freq
  if(WasButtonPressed(BUTTON3))
  {
    ButtonAcknowledge(BUTTON3);
    if(ant_frequency < 99)
    {
      // increment and print new freq, start counter to switch the channel
    ant_frequency++;
    //switch_counter=1;
    //LCDCommand(LCD_CLEAR_CMD);
    snprintf( msg, 19,"Scan frequency: %d", ant_frequency);
    LCDMessage(LINE1_START_ADDR, msg);
    }
  }
     // button2 should decrease freq
     if(WasButtonPressed(BUTTON2))
      {
        ButtonAcknowledge(BUTTON2);
        if(ant_frequency > 0)
        {
          // decrement and print new freq, start counter to switch the channel
        ant_frequency--;
        //switch_counter=1;
        //LCDCommand(LCD_CLEAR_CMD);
        snprintf( msg, 19, "Scan frequency: %d", ant_frequency);
        LCDMessage(LINE1_START_ADDR, msg);
        }
      }

//    // if we are going to switch channels soon:
//    if(switch_counter > 0)
//    {
//        switch_counter++;
//        // wait for a second and then reset variables, move states
//        if(switch_counter >=1000)
//        {
//          switch_counter=0;
//        UserApp1_StateMachine = UserApp1SM_Setup;
//        count = 0;
//        channel_open = FALSE;
//        }
//      }

//  static u8 count = 100;
//
//  if(AntRadioStatusChannel(ANT_CHANNEL_SCANNING) == ANT_OPEN)
//    {
////      // wait before retrying command
////  if(count < 100)
////  {
////    count++;
////    return;
////  }
////      AntCloseChannelNumber(0);
////   count = 0;
////      return;
////    }
////    else if(AntRadioStatusChannel(ANT_CHANNEL_SCANNING) == ANT_CLOSED)
////    {
////
////       if(count < 100)
////  {
////    count++;
////    return;
////  }
//      AntUnassignChannelNumber(0);
//      count = 0;
//      return;
//    }
//    else if(AntRadioStatusChannel(ANT_CHANNEL_SCANNING) == ANT_UNCONFIGURED)
//    {
//       UserApp1_StateMachine = UserApp1Initialize;
//    }
}
/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ??? */
static void UserApp1SM_Scan(void)
{
    u8 light_intensity;
  LedRateType new_rate;

#if SLOW
  // try running the state half as often
  if(slow)
  {
    slow = FALSE;
    return;
  }
  slow = TRUE;
#endif

  // wait before opening channel
  if(count < 100)
  {
    count++;
    return;
  }

  // open and check channel status
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
  return;
  }

  // handle incoming data
  if( AntReadAppMessageBuffer() && G_eAntApiCurrentMessageClass == ANT_DATA )
    {
      // for every message byte / LED:
      for(u8 i = 0; i< ANT_APPLICATION_MESSAGE_BYTES; i++)
      {
        // convert the byte into an LED brightness bin
        light_intensity = (u8)( G_au8AntApiCurrentMessageBytes[i] / LED_PWM_PERIOD );
        for(u8 j = 0; j < LED_PWM_PERIOD; j++)
        {
          if( light_intensity <= j )
          {
            new_rate = light_intensity;
            break;
          }
        }
        // if there was data in the byte, set the brightness
        if( G_au8AntApiCurrentMessageBytes[i] > 0 )
        {
          if( current_led_rates[i] != new_rate )
          {
          current_led_rates[i] = new_rate;
          LedPWM(i, current_led_rates[i]);
          }
        }

        else    // fade out the LEDs with no corresponding data
        {
          if(/*--led_delays[i] == 0 && */
             current_led_rates[i] != LED_PWM_0)
          {
            current_led_rates[i]--;
            led_delays[i] = LED_FADE_DELAY_CYCLES;
            LedPWM(i, current_led_rates[i]);
          }
        }

      }
    }
  else  // no data
  {
    if(++no_data_ticks >= 3000)
    {
      no_data_ticks = 0;
    // slowly fade
    for(u8 i = 0; i< ANT_APPLICATION_MESSAGE_BYTES; i++)
    {
    if(/*--led_delays[i] == 0 &&*/
         current_led_rates[i] != LED_PWM_0)
      {
        current_led_rates[i]--;
        led_delays[i] = LED_FADE_DELAY_CYCLES;
        LedPWM(i, current_led_rates[i]);
      }
    }
    }
  }
} /* end UserApp1SM_Scan() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)
{
  LCDMessage(LINE1_START_ADDR, "Channel assign failed");
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
