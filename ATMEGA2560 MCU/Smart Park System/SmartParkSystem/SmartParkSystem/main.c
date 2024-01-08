/*
 * SmartParkSystem.c
 *
 * Created: 10/28/2023 2:51:08 PM
 * Author : Emma
 */ 

#define F_CPU 16000000UL //crystal oscilator
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "lcd1/LCD1.h"
#include "lcd2/LCD2.h"
#include "UART/UART.h"

// ============== EEPROM ===============================================================================================
int isInitialized;
int initializedFlagAddress = 16; 
#define EEPROM_SIZE 4096
void eeprom_init();
void clearEEPROM();
void eeprom_init_print();

//// ============ ENTRY AND EXIT GATE =========================== //////////////////////////////////////////////////////////////////////////
// Define global variables to store data
int menuChoice = 0;
char newChoice = '\0';
bool receivingChoice = false; // Declare and initialize receivingChoice
char buffer[50];
//char buffer[3] = {0}; // Declare and initialize buffer
uint8_t bufferIndex = 0; // Declare and initialize bufferIndex
int currentParkCapacity = 0;
int waitingTourists = 0;
int touristsCollection = 0;
int childFee = 3000;      // Fee for children (10 years and below)
int adultFee = 5000;     // Fee for adults (above 10 years)
int childrenCount = 0;
int adultsCount = 0;
int vehiclesCount = 0;
int parkMaxCapacity = 100;  // Define the maximum park capacity
int moneyCollected;
int totalTouristsVisited = 0;

// FUNCTION PROTOTYPES
void displayLogin();
void receiveChoice();
void display_default_message();
void displayMenu();
void open_gate();
void close_gate();
char* readStringFromInput(int maxLength);
int readIntFromInput();
void handleMenuChoice(int choice);
void registerIncomingTourists();
void replenishFridge();
void sound_buzzer();
void display_incoming_message();
void registerExitingVehicle();
void displayVehiclesInPark();

// ======================= FRIDGE =============================================================
int fridgeMaxCapacity = 50;
int currentFridgeCapacity = 50;
int purchasedBottles = 0;
int bottleFee = 1500;
bool parkFull = false;
bool loggedIn = false;
char password[] = "1234";

// FRIDGE FUNCTION PROTOTYPES
void display_default_fridge_message();
char getKey();
void keypad_listener(void);
void move_money_slot_motor(int rotations);
void move_fridge_motor(int rotations);
void dispenseBottles(int numberOfBottles);
void execute_purchase();

typedef enum {
	KEYPAD_IDLE,
	PROCESS_INPUT
} KeypadState;

KeypadState keypadState = KEYPAD_IDLE;
char input[16];
int numBottles = 0;

// Vehicle structure to represent each vehicle
typedef struct {
	char numberPlate[20];
	int children;
	int adults;
	bool insidePark;  // true if inside the park, false if outside
} Vehicle;

// Array to store vehicle objects
Vehicle parkVehicles[100];  // Assuming a maximum of 100 vehicles

bool isParkFull();
// VARIABLE / CONSTANT INITIALIZATIONS 
bool buzzerOn = false;

int main(void)
{
	
	UART_init();
	
	// enabling pullup resistance on pin O of PORT D
	DDRD &= ~(1<<0);
	PORTD |= (1<<0);
	
	// enabling pullup resistance on pin O of PORT D
	DDRD &= ~(1<<1);
	PORTD |= (1<<1);
	
	// ENABLING INTERRUPTS
	// enable interrupts globally
	sei();
	
	// enable interrupts on INT0
	EIMSK |= (1<<INT0);		
	// on any logical change edge
	EICRA |= (1<<ISC00);
	
	// enable interrupts on INT1
	EIMSK |= (1<<INT1);
	// on any logical change edge
	EICRA |= (1<<ISC10);
	
	// BUZZER DDR //	
	PORTA &= ~(1<<0); // TURN BUZZER OFF
	DDRA=0xFF;
	
	// GATE MOTOR
	DDRC=0xFF;
	
	//KEYPAD
	// rows-input, columns-output
	DDRK=0x07;
	DDRJ=0xFF;
	
	// FRIDGE MOTOR
	DDRL=0xFF;	
	DDRF=0xFF;
	
	eeprom_init();
	
	//clearEEPROM();
	
	// INITIALIZING LCD  1
	DDRH=0xFF;
	DDRJ=0xFF;
	
	// Initialize the LCD1 with the desired display attributes
	lcd1_init(LCD1_FUNCTION_SET);
	lcd1_init(LCD1_DISPLAY_ON);
			
		
	// INITIALIZING LCD  2	
	DDRA=0xFF;
	DDRB=0xFF; 
	
	// Initialize the LCD with the desired display attributes
	lcd2_init(LCD2_FUNCTION_SET);
	lcd2_init(LCD2_DISPLAY_ON);

		
	// display default message
	//displayMenu();
	displayLogin();
	
	lcd2_clear();
	display_default_message();	
	
	lcd1_clear();
	display_default_fridge_message();
	

    /* Replace with your application code */
    while (1) 
    {
		
		receiveChoice(); // Check for new menu choice		
		keypad_listener();
		if (buzzerOn)
		{			
			sound_buzzer();
		}
    }
}

// BUZZER ISR
ISR(INT0_vect){
	buzzerOn = true;
}

ISR(INT1_vect){
	buzzerOn = true;
}



///////////////////// ENTRY AND EXIT GATE ////////////////////////////////////////
void display_default_message(){
	char word1[] = "Queen Elizabeth Park";
	char word2[] = "Welcome";
	lcd2_clear();
	lcd2_print(word1);
	
	// change to row 2
	lcd2_gotoxy(0, 1);
	
	lcd2_print(word2);
	
}

void receiveChoice() {
	char receivedChar;

	if (UCSR0A & (1 << RXC0)) {
		// Data is available to read
		receivedChar = UART_RxChar();
		UART_TxChar(receivedChar); // Echo back the character

		// Handle the received character
		if (receivingChoice) {
			if (receivedChar == '\r' || receivedChar == '\n') {
				if (bufferIndex > 0) {
					buffer[bufferIndex] = '\0'; // Null-terminate the string

					if (loggedIn) {
						newChoice = atoi(buffer); // Convert to an integer

						if (newChoice >= 1 && newChoice <= 12) {
							handleMenuChoice(newChoice);
							} else {
							UART_SendString("Invalid option. Please try again.\r\n");
						}
						} else {
						if (strcmp(buffer, password) == 0) {
							loggedIn = true;
							UART_SendString("Welcome....\r\n");
							_delay_ms(2000);
							displayMenu();
							} else {
							UART_SendString("Invalid Password. Please try again.\r\n");
							UART_SendString(buffer);
							displayLogin();
						}
					}

					// Stop receiving and reset the buffer
					receivingChoice = false;
					bufferIndex = 0;
				}
				} else {
				buffer[bufferIndex] = receivedChar;
				bufferIndex = (bufferIndex + 1) % 50;// Prevent buffer overflow
			}
			} else {
			// Start receiving immediately
			receivingChoice = true;
			buffer[0] = receivedChar;
			bufferIndex = 1;
		}
	}
}

void sound_buzzer() {	
	PORTA |= (1<<PA0);	
	display_incoming_message();
	_delay_ms(3000);
	PORTA &= ~(1<<0);	
	buzzerOn = false;
	_delay_ms(3000);
	display_default_message();
}

void display_registration_message(){
	char word1[] = "Registration...";		
	lcd2_clear();
	lcd2_gotoxy(0, 1);
	lcd2_print(word1);	
}
void display_incoming_message() {
	lcd2_clear();
	char word1[] = "Incoming Tourist";
	char word2[] = "Vehicle";
	lcd2_home();
	lcd2_print(word1);	
	lcd2_gotoxy(0, 1);
	lcd2_print(word2);
}

void display_post_registration_messages(){
	char word1[] = "Incoming Tourist";
	char word2[] = "Vehicle";
	char word3[] = "Car at Gate...";
	char word4[] = "Gate closing...";
	
	//open gate
	open_gate();
		
	//car at gate
	lcd2_clear();
	lcd2_home();
	lcd2_print(word1);
	lcd2_gotoxy(0, 1);
	lcd2_print(word2);
	lcd2_gotoxy(0, 2);
	lcd2_print(word3);	
	_delay_ms(3000);
	
	// gate closing
	lcd2_clear();
	lcd2_home();
	lcd2_print(word1);
	lcd2_gotoxy(0, 1);
	lcd2_print(word2);
	lcd2_gotoxy(0, 2);
	lcd2_print(word4);
	lcd2_home();
	
	close_gate();
	_delay_ms(2000);	
	
	PORTC = 0; // STOP MOTOR
	display_default_message();
	
}

void displayLogin(){
	UART_SendString("\r\n");
	UART_SendString("Welcome to the Queen Elizabeth Park Management Console\r\n");
	UART_SendString("Enter the password to login \r\n");
}

void displayMenu() {
	UART_SendString("\r\n");
	UART_SendString("Welcome to the Queen Elizabeth Park Management Console\r\n");
	UART_SendString("1. Register Incoming Tourists\r\n");
	UART_SendString("2. Register Outgoing Tourists\r\n");
	UART_SendString("3. Tourists in the park\r\n");
	UART_SendString("4. Vehicles in the park\r\n");
	UART_SendString("5. Money collected\r\n");
	UART_SendString("6. Drivers in the park\r\n");
	UART_SendString("7. Bottles in the fridge\r\n");
	UART_SendString("8. Replenish Fridge\r\n");
	UART_SendString("9. Cars outside park\r\n");
	UART_SendString("10. Park Capacity State\r\n");
	UART_SendString("11. Collect money\r\n");
	UART_SendString("12. Logout\r\n");
}


void handleMenuChoice(int choice) {
	switch (choice) {
	case 1:
	// Register Incoming Tourists
	display_registration_message();
	registerIncomingTourists();
	_delay_ms(2000);
	displayMenu();
	break;
	case 2:
	registerExitingVehicle();
	_delay_ms(2000);
	displayMenu();	
	display_post_registration_messages();
	break;
	case 3:
	// Tourists in the park
	// Send the currentParkCapacity, childrenCount, and adultsCount to the console
	UART_SendString("Tourists in the park:\n");
	UART_SendString("Children (<= 10 years): ");
	uart_send_int(childrenCount);
	UART_SendString("\r\n");
	UART_SendString("Adults (> 10 years): ");
	uart_send_int(adultsCount);
	_delay_ms(2000);
	displayMenu();
	break;
	case 4:
	// Vehicles in the park
	UART_SendString("Vehicles in the park: ");
	UART_SendString("\r\n");
	 displayVehiclesInPark();	
	 _delay_ms(2000);
	 displayMenu();
	break;
	case 5:
	// Money collected
	UART_SendString("Money collected: UGX");
	moneyCollected = (bottleFee * purchasedBottles) + touristsCollection;
	uart_send_int(moneyCollected);
	_delay_ms(2000);
	displayMenu();
	break;
	case 6:
	// Drivers in the park
	// Send the driversCount / vehiclesCount to the console
	UART_SendString("Drivers in the park: ");
	UART_TxChar(vehiclesCount + '0');
	_delay_ms(2000);
	displayMenu();
	break;
	case 7:
	// Bottles in the fridge
	UART_SendString("Bottles in the fridge: ");
	uart_send_int(currentFridgeCapacity);
	_delay_ms(2000);
	displayMenu();
	break;
	case 8:
	// Replenish Fridge
	replenishFridge();
	_delay_ms(2000);
	displayMenu();
	break;
	case 9:
	UART_SendString("Waiting Tourists: ");
	uart_send_int(waitingTourists);	
	_delay_ms(2000);
	displayMenu();
	break;
	case 10:
	// Park Capacity State
	// Send park capacity status to the console
	if (isParkFull()) {
		UART_SendString("Park is full.\n");
		} else {
		UART_SendString("Park is not full.\n");
	}
	_delay_ms(2000);
	displayMenu();
	break;
	case 11:
	touristsCollection = 0;
	purchasedBottles = 0;
	eeprom_write_block(&touristsCollection, (int*)26, sizeof(int));
	eeprom_write_block(&purchasedBottles, (int*)22, sizeof(int));
	UART_SendString("Money collected");
	_delay_ms(2000);
	displayMenu();
	break;
	case 12:
	loggedIn = false;
	UART_SendString("Thank you....\r\n");
	_delay_ms(2000);
	displayLogin();
	break;
	}
}

// Function to check if the park is full
bool isParkFull() {
	return currentParkCapacity >= parkMaxCapacity;
}


// Function to register incoming tourists and update counts
void registerIncomingTourists() {
	if (!isParkFull())
	{
		UART_SendString("Enter the number plate of the vehicle: ");
		char* numberPlate = readStringFromInput(20);
		
		UART_SendString("Enter the number of children (10 years and below): ");
		int children = readIntFromInput();
		
		UART_SendString("Enter the number of adults (above 10 years): ");
		int adults = readIntFromInput();
		
		int passengerCount = adults + children;
		
		if ((passengerCount + currentParkCapacity) > parkMaxCapacity)
		{
			UART_SendString("Over the max, sorry\r\n");
			_delay_ms(2000);
		} 
		else
		{		
		
			// Create a new Vehicle object and store the details
			Vehicle newVehicle;
			strcpy(newVehicle.numberPlate, numberPlate);
			newVehicle.children = children;
			newVehicle.adults = adults;
			newVehicle.insidePark = true;  // Vehicle is inside the park
		
			// Add the new vehicle to the vehicle array
			int index = vehiclesCount;  // Current count of registered vehicles
			parkVehicles[index] = newVehicle;
		
			// Update counts and other information
			childrenCount += children;
			adultsCount += adults;
		
			currentParkCapacity += (childrenCount + adultsCount);		
		
			vehiclesCount++;
		
			int driverCount = adults - 1;
			int childCollection = children * childFee;
			int adultCollection = driverCount * adultFee;
		  
			// Process Vehicle collection
			touristsCollection = touristsCollection + childCollection + adultCollection;
			eeprom_write_block(&touristsCollection, (int*)26, sizeof(int));
		
			//uart_send_int(touristsCollection);
		
			display_post_registration_messages();
			if(waitingTourists > 0){
				waitingTourists--;
			}
		}
	} 
	else
	{
		waitingTourists++;
		char word1[] = "PARK FULL";
		// registration
		lcd2_clear();
		lcd2_home();
		lcd2_print(word1);
		_delay_ms(5000);
		display_default_message();
		

	}
	
}

// Function to display vehicles in the park
void displayVehiclesInPark() {
	for (int i = 0; i < vehiclesCount; i++) {
		if (parkVehicles[i].insidePark) {
			// Display vehicle details
			UART_SendString("Number Plate: ");
			UART_SendString(parkVehicles[i].numberPlate);
			UART_SendString("\r\n");
			UART_SendString("Children: ");
			uart_send_int(parkVehicles[i].children);
			UART_SendString("\r\n");
			UART_SendString("Adults: ");
			uart_send_int(parkVehicles[i].adults);
			UART_SendString("\r\n");
			UART_SendString("Inside Park: ");
			UART_SendString(parkVehicles[i].insidePark ? "Yes" : "No");
			UART_SendString("\r\n\r\n");
		}
	}
}

void replenishFridge() {
	UART_SendString("Enter the password: ");
	char* entered_password = readStringFromInput(20);
	if (strcmp(entered_password, password) == 0) {
		UART_SendString("\n \r");
		UART_SendString("Enter the number of bottles to add to the fridge: ");
		int bottlesToAdd = readIntFromInput();
		UART_SendString(bottlesToAdd);
		
		if (bottlesToAdd > 0) {
			int bottlesUpdate = currentFridgeCapacity + bottlesToAdd;
			if (bottlesUpdate <= fridgeMaxCapacity){
				currentFridgeCapacity = bottlesUpdate;
				eeprom_write_block(&currentFridgeCapacity, (int*)10, sizeof(int));
				UART_SendString("Fridge replenished.");
				UART_SendString("\r\n");
				}else{
				UART_SendString("Fridge capacity surpassed");
				UART_SendString("\r\n");
			}
			_delay_ms(2000);
			} else {
			UART_SendString("Invalid number of bottles.");
			UART_SendString("\r\n");
			_delay_ms(2000);
		}
		} else {
		UART_SendString("Invalid password. Replenishment denied.");
		UART_SendString("\r\n");
		_delay_ms(2000);
	}
}

void registerExitingVehicle() {
	UART_SendString("Enter the number plate of the exiting vehicle: ");
	char* exitingNumberPlate = readStringFromInput(20);;
	for (int i = 0; i < 100; i++) {
		if (parkVehicles[i].insidePark && strcmp(parkVehicles[i].numberPlate, exitingNumberPlate) == 0) {
			childrenCount -= parkVehicles[i].children;
			adultsCount -= parkVehicles[i].adults;
			currentParkCapacity -= parkVehicles[i].children + parkVehicles[i].adults;
			parkVehicles[i].insidePark = false;
			UART_SendString("Exiting vehicle registered.");
			UART_SendString("\r\n");
			// Update number of drivers
			vehiclesCount--;
			return;
		}
	}
	UART_SendString("Vehicle not found in the park.");
	displayMenu();
}

// =================== EEPROM FUNCTIONS =================================

void eeprom_init(){
	/// EEPROM INITIALIZATIONS
	//Read the initialization flag from EEPROM
	eeprom_read_block(&isInitialized, (int*)initializedFlagAddress, sizeof(int));
	if (isInitialized != 1) {
		// The configuration values haven't been initialized, so initialize them and set the flag
		eeprom_write_block(&childFee, (int*)0, sizeof(int));
		eeprom_write_block(&bottleFee, (int*)2, sizeof(int));
		eeprom_write_block(&adultFee, (int*)4, sizeof(int));
		eeprom_write_block(&parkMaxCapacity, (int*)6, sizeof(int));
		eeprom_write_block(&fridgeMaxCapacity, (int*)8, sizeof(int));
		eeprom_write_block(&currentFridgeCapacity, (int*)10, sizeof(int));
		eeprom_write_block(&password, (void*)18, strlen(password));
		eeprom_write_block(&purchasedBottles, (int*)22, sizeof(int));
		eeprom_write_block(&touristsCollection, (int*)26, sizeof(int));

		// Set the initialization flag to true
		isInitialized = 1;
		eeprom_write_block(&isInitialized, (int*)initializedFlagAddress, sizeof(int));

		UART_SendString("Configuration values written to Memory.\r\n");
		} else {
		// The configuration values have already been initialized, so just read them
		eeprom_read_block(&childFee, (int*)0, sizeof(int));
		eeprom_read_block(&bottleFee, (int*)2, sizeof(int));
		eeprom_read_block(&adultFee, (int*)4, sizeof(int));
		eeprom_read_block(&parkMaxCapacity, (int*)6, sizeof(int));
		eeprom_read_block(&fridgeMaxCapacity, (int*)8, sizeof(int));
		eeprom_read_block(&currentFridgeCapacity, (int*)10, sizeof(int));
		eeprom_read_block(&password, (void*)18, strlen(password));
		eeprom_read_block(&purchasedBottles, (int*)22, sizeof(int));
		eeprom_read_block(&touristsCollection, (int*)26, sizeof(int));
		
		//eeprom_init_print();

		UART_SendString("Configuration values loaded from Memory.");
		UART_SendString("\r\n");
	}	
}

void eeprom_init_print(){
	uart_send_int(parkMaxCapacity);
	UART_SendString("\r\n");
	uart_send_int(fridgeMaxCapacity);
	UART_SendString("\r\n");
	uart_send_int(adultFee);
	UART_SendString("\r\n");
	uart_send_int(childFee);
	UART_SendString("\r\n");
	UART_SendString(password);
	UART_SendString("\r\n");
	uart_send_int(bottleFee);
	UART_SendString("\r\n");
	uart_send_int(purchasedBottles);
	UART_SendString("\r\n");
	uart_send_int(touristsCollection);
	UART_SendString("\r\n");
	uart_send_int(currentFridgeCapacity);
	UART_SendString("\r\n");
}
void clearEEPROM(){
// Clear the EEPROM by writing 0xFF to every byte.
for (int i = 0; i < EEPROM_SIZE; i++) {
	eeprom_write_byte(i, 0xFF);
}
}

void open_gate(){
	PORTC = 0b00000010;
}
void close_gate(){
	PORTC = 0b00000001;
}
/////////////////// ==================================== FRIDGE FUNCTIONS =============================================================
void display_default_fridge_message(){
	char word1[] = "Get a bottle of";
	char word2[] = "water at sh. 1500";
	char word3[] = "Select the number";
	char word4[] = " of bottles:";
	lcd1_clear();
	lcd1_print(word1);
	lcd1_gotoxy(0, 1);
	lcd1_print(word2);
	_delay_ms(3000);
	lcd1_clear();
	lcd1_home();
	lcd1_print(word3);
	lcd1_gotoxy(0, 1);
	lcd1_print(word4);

}

void move_fridge_motor(int rotations){
	int i = 0;
	while (i < rotations)
	{
		PORTL = 0x66;
		_delay_ms(50);
		PORTL = 0x33;
		_delay_ms(50);
		PORTL = 0x99;
		_delay_ms(50);
		PORTL = 0xCC;
		_delay_ms(1000);
		i++;
	}
}

void move_money_slot_motor(int rotations){
	int i = 0;
	while (i < rotations)
	{
		PORTF = 0x66;
		_delay_ms(50);
		PORTF = 0x33;
		_delay_ms(50);
		PORTF = 0x99;
		_delay_ms(50);
		PORTF = 0xCC;
		_delay_ms(1000);
		i++;
	}
}

void keypad_listener() {
	char key = getKey(); // Implement a function to get the keypress from your keypad (not shown in your code).

	switch (keypadState) {
		case KEYPAD_IDLE:
		if (key == '#') {
			// User pressed '#', transition to PROCESS_INPUT state
			keypadState = PROCESS_INPUT;
			} else if (key != '\0') {
			// User entered a key, collect it
			input[strlen(input)] = key;
			lcd1_data(key); // Assuming sendLCDData sends a single character to the LCD
		}
		break;

		case PROCESS_INPUT:
		numBottles = atoi(input);
		if (numBottles != 0) {
			execute_purchase(); // Implement this function to handle the purchase
			} else {
			display_default_fridge_message();
		}
		// Reset for the next input
		memset(input, 0, sizeof(input));
		keypadState = KEYPAD_IDLE;
		break;
	}
}


char getKey() {
	static char lastKey = '\0';
	char key = '\0';

	// Read the current state of the keypad
	PORTK = 0xFB;
	if ((PINK & 0x08) == 0) {
		key = '1';
		} else if ((PINK & 0x10) == 0) {
		key = '4';
		} else if ((PINK & 0x20) == 0) {
		key = '7';
	}

	PORTK = 0xFD;
	if ((PINK & 0x08) == 0) {
		key = '2';
		} else if ((PINK & 0x10) == 0) {
		key = '5';
		} else if ((PINK & 0x20) == 0) {
		key = '8';
		} else if ((PINK & 0x40) == 0) {
		key = '0';
	}

	PORTK = 0xFE;
	if ((PINK & 0x08) == 0) {
		key = '3';
		} else if ((PINK & 0x10) == 0) {
		key = '6';
		} else if ((PINK & 0x20) == 0) {
		key = '9';
		} else if ((PINK & 0x40) == 0) {
		key = '#';
	}

	// Debounce the keypress
	if (key == lastKey) {
		// Key is still the same, wait for stability
		_delay_ms(10);
		return '\0';
		} else {
		// Key has changed, update the last key
		lastKey = key;
		return key;
	}
}



void execute_purchase() {
	char key;
	
	int totalCost = numBottles * bottleFee;
	

	if (numBottles > currentFridgeCapacity)
	{		
		lcd1_clear();
		lcd1_print("Over the Max");
		_delay_ms(1000);
		display_default_fridge_message();
	}else{
		lcd1_clear();
		lcd1_home();
		lcd1_print("Cost: UGX ");
		lcd1_printNumber(totalCost);
		lcd1_gotoxy(0, 1);
		lcd1_print("Press # to confirm");
		lcd1_home();
		
		while (1) {
			key = getKey();
			if (key == '#') {
				break;
			}
		}
		
		// rotate 3 times
		_delay_ms(1000);
		move_money_slot_motor(3);
		_delay_ms(2000);
				
		// Dispense the required number of bottles
		dispenseBottles(numBottles);
				
		// UPDATING NO OF PURCHASED BOTTLES
		purchasedBottles += numBottles;
		eeprom_write_block(&purchasedBottles, (int*)22, sizeof(int));
		
		currentFridgeCapacity-=numBottles;
		eeprom_write_block(&currentFridgeCapacity, (int*)10, sizeof(int));

		// Display a message indicating a successful purchase
		lcd1_clear();
		lcd1_print("Thank You.....");
		_delay_ms(3000);
		display_default_fridge_message();
	}
}

// Function to dispense the required number of bottles
void dispenseBottles(int numberOfBottles) {	
	for (int i = 0; i < numberOfBottles; i++) {
		move_fridge_motor(2);
		_delay_ms(2000);
	}
}