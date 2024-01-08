 #include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Stepper.h>
#include <Keypad.h>

#define buzz 13
const byte intr_pin = 2;

const int rs = 35, en = 36, d4 = 37, d5 = 38, d6 = 39, d7 = 40; // Pins to which LCD is connected
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
volatile bool buzzerOn = false;

bool registrationComplete = false;
// Constants
// Address in EEPROM to store the initialization flag
int initializedFlagAddress = 16; // Use an address beyond your configuration values

int isInitialized;
const int parkMaxCapacity = 100;  // Maximum number of tourists in the park
int currentParkCapacity = 0;
int waitingTourists = 0;
int touristsCollection = 0;
int childFee = 5000;      // Fee for children (10 years and below)
int adultFee = 10000;     // Fee for adults (above 10 years)

int driverCount;
int childCount = 0;
int adultCount = 0;
int moneyCollected = 0;
int totalDriversInsidePark = 0;

bool parkFull = false;
bool loggedIn = false;
String password = "1234";  // Change to your desired password

int menuChoice = 0;
unsigned long lastInputCheckTime = 0;
const unsigned long inputCheckInterval = 1000; // Check for input every 1 second

struct Vehicle {
  char numberPlate[11];  // 10 characters for the number plate
  int occupants;
  bool insidePark;
};
Vehicle parkVehicles[100];

/////////////////////// FRIDGE INITIALIZATIONS ////////////////////
int fridgeMaxCapacity = 50;
int currentFridgeCapacity = 50;
int purchasedBottles = 0;
int bottleFee = 1500;


// stepper motor initializations
int stepsPerRevolution = 4;
int motSpeed=360; //rpm
Stepper myStepper(stepsPerRevolution, 2,3,4,5);

// Fridge initializations
//const float bottleCost = 1500.0; // UGX
int numBottles = 0;
boolean bottles_entered = false;

const int rs2 = 48, en2 = 49, d4_2 = 50, d5_2 = 51, d6_2= 52, d7_2 = 53; // Pins to which LCD is connected
LiquidCrystal lcd2(rs2, en2, d4_2, d5_2, d6_2, d7_2);

enum KeypadState {
  KEYPAD_IDLE,
  COLLECT_INPUT,
  PROCESS_INPUT
};

KeypadState keypadState = KEYPAD_IDLE;
String input;

// KEYPAD initializations
// Define keypad layout for a 12-button keypad phone
const byte ROW_NUM = 4;
const byte COLUMN_NUM = 3;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; // Connect to keypad rows
byte pin_columns[COLUMN_NUM] = {10, 11, 12}; // Connect to keypad columns

Keypad keypad(makeKeymap(keys), pin_rows, pin_columns, ROW_NUM, COLUMN_NUM);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myStepper.setSpeed(motSpeed);
  
  // initializing LCD
  lcd2.begin(20, 2);
  display_default_fridge_message();

/// GATE SETUP
//Read the initialization flag from EEPROM
  EEPROM.get(initializedFlagAddress, isInitialized);

 

  if (isInitialized != 1) {
    // The configuration values haven't been initialized, so initialize them and set the flag
    EEPROM.put(0, childFee);
    EEPROM.put(2, bottleFee);
    EEPROM.put(4, adultFee);
    EEPROM.put(8, parkMaxCapacity);
    EEPROM.put(12, fridgeMaxCapacity);
    EEPROM.put(14, currentFridgeCapacity);
    EEPROM.put(20, purchasedBottles);
   // EEPROM.put(24, touristsCollection);
    EEPROM.put(28, password);

    // Set the initialization flag to true
    isInitialized = 1;
    EEPROM.put(initializedFlagAddress, isInitialized);

    Serial.println("Configuration values written to Memory.");
  } else {
    // The configuration values have already been initialized, so just read them
    EEPROM.get(0, childFee);  
    EEPROM.get(2, bottleFee);
    EEPROM.get(4, adultFee);
    EEPROM.get(8, parkMaxCapacity);
    EEPROM.get(12, fridgeMaxCapacity);
    EEPROM.get(14, currentFridgeCapacity);
    EEPROM.get(20, purchasedBottles);
    //EEPROM.get(24, touristsCollection);
    EEPROM.get(28, password);

    //Serial.println(currentParkCapacity);
    Serial.println(parkMaxCapacity);
    //Serial.println(waitingTourists);
    //Serial.println(touristsCollection);
    Serial.println(fridgeMaxCapacity);
    Serial.println(purchasedBottles);
    Serial.println(adultFee);
    Serial.println(childFee);
    Serial.println(password);
    Serial.println(bottleFee);
    Serial.println("Configuration values loaded from Memory.");
  }  



//  clearEEPROM(); 
  
  lcd.begin(20, 4);
  pinMode(buzz, OUTPUT);
  pinMode(intr_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intr_pin), CarEntranceISR, CHANGE);
  display_default_message();
  displayMenu();
  
  lastInputCheckTime = millis();

}

void loop() {
  if (buzzerOn) {
    sound_buzzer();
  }
  // put your main code here, to run repeatedly:
   unsigned long currentTime = millis();
  if (currentTime - lastInputCheckTime >= inputCheckInterval) {
    lastInputCheckTime = currentTime;
    int newChoice = readMenuChoice();
    if (newChoice != 0) {
      menuChoice = newChoice;
      handleMenuChoice(menuChoice);
      menuChoice = 0; // Reset the menu choice
    }
  }
  
  keypad_listener();
  delay(100);
}

void display_default_fridge_message() {
  lcd2.clear();
  lcd2.print("Get a bottle of");
  lcd2.setCursor(0,1);
  lcd2.print("water at sh.");
  lcd2.print(bottleFee);
  delay(1000);
  lcd2.clear();
  lcd2.print("Select the number");
  lcd2.setCursor(0,1);
  lcd2.print(" of bottles:");  
}

void execute_purchase() {
  char key;
  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;
  
  float totalCost = numBottles * bottleFee;
  
  lcd2.clear();
  lcd2.print("Cost: UGX ");
  lcd2.print(totalCost);
  lcd2.setCursor(0, 1);
  lcd2.print("Press #");
  
  while (elapsedTime < 3000) {  // Wait for 3 seconds
    key = keypad.getKey();
    if (key == '#') {
      break;  // Exit the loop if '#' is pressed
    }
    elapsedTime = millis() - startTime;
  }
  
  if (key == '#' && (numBottles <= currentFridgeCapacity)) {
    myStepper.step(stepsPerRevolution * 3);
    delay(2000);
    for (int i = 0; i < numBottles; i++) {
      myStepper.step(stepsPerRevolution);
      delay(2000);
    }
    
    // UPDATING NO OF PURCHASED BOTTLES
    purchasedBottles += numBottles;
    //EEPROM.put(20, purchasedBottles);

    // UPDATING CURRENT FRIDGE CAPACITY
    currentFridgeCapacity -= numBottles;
    //EEPROM.put(14,currentFridgeCapacity);
    
    lcd2.clear();
    lcd2.print("Thank You..");
    delay(1000);
    numBottles = 0;    
    display_default_fridge_message();
  }else if (numBottles > currentFridgeCapacity){
    lcd2.clear();
    lcd2.print("Over the Max");
    display_default_fridge_message();
    }
  else{
    numBottles = 0;
    }
}


void keypad_listener() {
  char key = keypad.getKey();

  switch (keypadState) {
    case IDLE:
      if (key == '#') {
        // User pressed '#', transition to PROCESS_INPUT state
        keypadState = PROCESS_INPUT;
      } else if (key != NO_KEY) {
        // User entered a key, collect it
        input += key;
        lcd2.clear();
        lcd2.print(key);
        // Optionally, you can set a timeout here to reset the state to IDLE if no key is pressed for a certain time.
      }
      break;

    case COLLECT_INPUT:
      // This state is not needed anymore

    case PROCESS_INPUT:
      numBottles = input.toInt();
      if (numBottles != 0) {
        execute_purchase();
      } else {
        display_default_fridge_message();
      }
      // Reset for the next input
      input = "";
      keypadState = KEYPAD_IDLE;
      break;
  }
}
void keypad_listener2() {
  char bottle_amount = '0';
  String input;
  char key;
  do {
    key = keypad.getKey();
    if (key) {
      input += key;
      lcd2.clear();
      lcd2.print(key);
    }
  } while (key != '#'); // Assume # is used to submit input
  numBottles = input.toInt();
    if(numBottles != 0){      
      execute_purchase();
    }else{
      display_default_fridge_message();
      }
}
//////////////////////////////// ENTRANCE AND EXIT GATE CODE ///////////////////////////////////////////////////
void CarEntranceISR() {
  buzzerOn = true;
}

void display_incoming_message() {
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Incoming Tourist");
  lcd.setCursor(4, 2);
  lcd.print("Vehicle");
}

void display_registration_message() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Registration...");
}

void display_car_at_gate_message() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Car at Gate...");
}

void display_gate_closing_message() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Gate closing...");
}

void display_default_message() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Queen Elizabeth Park");
  lcd.setCursor(4, 2);
  lcd.print("Welcome");
}

void sound_buzzer() {
  digitalWrite(buzz, HIGH);
  display_incoming_message();
  delay(1000);
  digitalWrite(buzz, LOW);
  buzzerOn = false;  
  display_incoming_message(); 
}

void registerVisitors() {
  String carNumberPlate;
  int childrenCount;
  int adultsCount;
  if(currentParkCapacity < parkMaxCapacity){    
  
  Serial.println("Enter registration data:");
  
  // Enter number of children
  Serial.println("Enter number of children (10 years and below): ");
  while (!Serial.available()) {
    // Wait for input
  }
  childrenCount = Serial.parseInt();

  // Enter car number plate
  Serial.println("Enter car number plate:");
  while (!Serial.available()) {
    // Wait for input
  }
  carNumberPlate = Serial.readStringUntil('\n');

  // Enter number of adults
  Serial.println("Enter number of adults (above 10 years): ");
  while (!Serial.available()) {
    // Wait for input
  }
  adultsCount = Serial.parseInt();

  // After collecting the data, update the park capacity
  // Update the vehicle data and save it to EEPROM
  for (int i = 0; i < 100; i++) {
    if (!parkVehicles[i].insidePark) {
      // Find an empty slot in the vehicle data array
      strncpy(parkVehicles[i].numberPlate, carNumberPlate.c_str(), sizeof(parkVehicles[i].numberPlate) - 1);
      parkVehicles[i].occupants = childrenCount + adultsCount;
      parkVehicles[i].insidePark = true;
      break;  // Exit the loop after finding an empty slot
    }
  }
  // Update the counts of tourists by age group
  childCount += childrenCount;
  adultCount += adultsCount;

  currentParkCapacity = childCount + adultCount;

  // Update number of drivers
  totalDriversInsidePark++;

  int driverCount = adultsCount - 1;
  Serial.println(driverCount);

  int childCollection = childCount * childFee;
  int adultCollection = driverCount * adultFee;
  
  // Process Vehicle collection
  touristsCollection = touristsCollection +  childCollection + adultCollection;
  Serial.println(touristsCollection);
  //EEPROM.put(24,touristsCollection);

  if(waitingTourists > 0){
    waitingTourists--;
    }

  // Display collected data with labels
  Serial.println("Registration Data:");
  Serial.print("Car Plate: ");
  Serial.println(carNumberPlate);
  Serial.print("Children Count: ");
  Serial.println(childrenCount);
  Serial.print("Adults Count: ");
  Serial.println(adultsCount);  
  delay(2000);
  displayMenu();  
  }else{
    waitingTourists++;
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("PARK FULL");
    delay(2000);
    }  
}

void displayMenu() {
  Serial.println("Welcome to the Queen Elizabeth Park Management Console");
  Serial.println("1. Register Incoming Tourists");
  Serial.println("2. Register Outgoing Tourists");
  Serial.println("3. Tourists in the park");
  Serial.println("4. Vehicles in the park");
  Serial.println("5. Money collected");
  Serial.println("6. Drivers in the park");
  Serial.println("7. Bottles in the fridge");
  Serial.println("8. Replenish Fridge");
  Serial.println("9. Cars outside park");
  Serial.println("10. Park Capacity State");
  Serial.println("11. Logout");
}

int readMenuChoice() {
  if (Serial.available() > 0) {
    int choice = Serial.parseInt();
    Serial.println(choice); // Echo the choice
    return choice;
  }
  return 0; // No input available
}

void handleMenuChoice(int choice) {
  switch (choice) {
    case 1:
      display_registration_message();
      registerVisitors();
      registrationComplete = true;
      display_car_at_gate_message();
      delay(3000);
      display_gate_closing_message();
      delay(2000);
      display_default_message();
      break;
   case 2:
      display_registration_message();
      registerExitingVehicle();
      display_car_at_gate_message();
      delay(3000);
      display_gate_closing_message();
      delay(2000);
      display_default_message();
      break;
    case 3:
      Serial.print("Children: ");
      Serial.println(childCount);
      Serial.print("Adults: ");
      Serial.println(adultCount);
      delay(2000);
      displayMenu();
      break;
    case 4:
      Serial.println("Vehicles in the park: ");
      displayVehiclesInsidePark();
      displayMenu();
      delay(2000);
      break;
    case 5:
      Serial.print("Money collected: UGX ");
      moneyCollected = (bottleFee * purchasedBottles) + touristsCollection;
      Serial.println(moneyCollected);
      delay(2000);
      displayMenu();
      break;
    case 6:
      Serial.print("Drivers in the park: ");
      Serial.println(totalDriversInsidePark);
      delay(2000);      
      displayMenu();
      break;
    case 7:
      Serial.print("Bottles in the fridge: ");
      Serial.println(currentFridgeCapacity);
      delay(2000);
      displayMenu();
      break;
    case 8:
      replenishFridge();
      delay(2000);
      displayMenu();
      break;
    case 9:
      Serial.print("Waiting Tourists ");
      Serial.println(waitingTourists);
      delay(2000);
      displayMenu();
      break;
    case 10:
      if(currentParkCapacity == parkMaxCapacity){
        Serial.println("Park Full");
        }else{
          Serial.println("Park Not Full");
      }  
      delay(2000);
      displayMenu();
      break;
    case 11:      
      loggedIn = false;
      Serial.println("Logged out.");    
      delay(2000);
      displayMenu();
      break;      
    default:
      Serial.println("Invalid choice. Please try again.");
      delay(2000);
      displayMenu();
  }
}

void replenishFridge() {
  char inputPassword[20];
  Serial.print("Enter the password: ");
  while (!Serial.available()) {
    // Wait for user input
  }
  String entered_password = Serial.readStringUntil('\n');
  if (entered_password == password) {
    Serial.println("\n");
    Serial.print("Enter the number of bottles to add to the fridge: ");
    while (!Serial.available()) {
      // Wait for user input
    }
    int bottlesToAdd = Serial.parseInt();
    Serial.println(bottlesToAdd);
    
    if (bottlesToAdd > 0) {
      int bottlesUpdate = currentFridgeCapacity + bottlesToAdd;
      if (bottlesUpdate <= fridgeMaxCapacity){
        currentFridgeCapacity = bottlesUpdate;
        EEPROM.put(14, currentFridgeCapacity);
      Serial.println("Fridge replenished.");
        }else{
          Serial.println("Fridge capacity surpassed");
          }
      delay(2000);
    } else {
      Serial.println("Invalid number of bottles.");
      delay(2000);
    }
  } else {
    Serial.println("Invalid password. Replenishment denied.");
    delay(2000);
  }
}

// Function to display vehicles inside the park
void displayVehiclesInsidePark() {
  Serial.println("Vehicles in the park:");
  //loadVehicleData();
  for (int i = 0; i < 100; i++) {
    if (parkVehicles[i].insidePark) {
      Serial.print("Number Plate: ");
      Serial.println(parkVehicles[i].numberPlate);
      Serial.print("Occupants: ");
      Serial.println(parkVehicles[i].occupants);
      Serial.println();
    }
  }
}

void registerExitingVehicle() {
  Serial.println("Enter the number plate of the exiting vehicle: ");
    while (!Serial.available()) {
    // Wait for user input
  }
  String exitingNumberPlate = Serial.readStringUntil('\n');
  for (int i = 0; i < 100; i++) {
    if (parkVehicles[i].insidePark && strcmp(parkVehicles[i].numberPlate, exitingNumberPlate.c_str()) == 0) {
        currentParkCapacity -= parkVehicles[i].occupants;
      parkVehicles[i].insidePark = false;
      Serial.println("Exiting vehicle registered.");
        // Update number of drivers
        totalDriversInsidePark--;
      return;
    }
  }
  Serial.println("Vehicle not found in the park.");  
  displayMenu();
}

void clearSerialMonitor() {
  Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
  delay(1000);            // wait for a second
}

void clearEEPROM(){
     for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  }
 

//  void checkEEPROM(){
//    int a;
//    for (int i = 0 ; i < EEPROM.length() ; i++) {
//      EEPROM.get(i, a);
//      Serial.print(a);
//      Serial.println();
//      delay(500);
//    }
//  }
