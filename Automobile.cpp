#include "Automobile.h"
#include "Logger.h"

/**
 * Constructor for the Automobile class.
 * Initializes the make, model, colour, year, and sets the fuel in the tank to 0.
 */
Automobile::Automobile(string _make, string _model, string _colour, int _year) {
    make = _make;
    model = _model;
    colour = _colour;
    year = _year;
    fuelInTank = 0;
}

/**
 * Sets the fuel efficiency of the automobile.
 * @param _efficiency The fuel efficiency to set.
 */
void Automobile::setFuelEfficiency(double _efficiency) {
    fuelEfficiency = _efficiency;
}

/**
 * Empties the fuel tank of the automobile.
 */
void Automobile::emptyFuel(void) {
    fuelInTank = 0;
}

/**
 * Adds fuel to the automobile's tank.
 * @param _liters The amount of fuel to add in liters.
 */
void Automobile::addFuel(double _liters) {
    fuelInTank += _liters;
    if(fuelInTank > 50) {
        fuelInTank = 50;
        char message[64];
        sprintf(message, "The %s %d %s %s is full of gas. Discarding the rest...\n", colour.c_str(), year, make.c_str(), model.c_str());
        Log(WARNING, __FILE__, __func__, __LINE__, message);
    }
}

/**
 * Drives the automobile for a specified distance.
 * Consumes fuel based on the distance and fuel efficiency.
 * @param _distance The distance to drive in kilometers.
 */
void Automobile::drive(double _distance) {
    double fuelConsumed = fuelEfficiency / 100 * _distance;
    fuelInTank -= fuelConsumed;
    if(fuelInTank < 0) {
        fuelInTank = 0;
        char message[64];
        sprintf(message, "The %s %d %s %s has no gas left in the tank\n", colour.c_str(), year, make.c_str(), model.c_str());
        Log(ERROR, __FILE__, __func__, __LINE__, message);
    }
}

/**
 * Displays a report of the automobile's current state.
 */
void Automobile::displayReport() {
    cout << "The " << colour << " " << year << " " << make << " " << model << " has " << fuelInTank << " left in the tank" << endl;
}
