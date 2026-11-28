
#include "switches.h"
#include <cstring>

static const int MAX_EMERGENCY_TRIPS = 3;

static const int MAX_SCORE =
    SCORE_KEY_COLLECT +
    3 * SCORE_CLUE_FOUND +
    SCORE_CABINET_UNLOCK +
    SCORE_TRAIN_KEY +
    SCORE_RESCUE_PASSENGER +
    SCORE_SECURE_CAT +
    SCORE_INSTALL_FUSE +
    SCORE_LIFT_PASS +
    SCORE_PREPARE_TRAIN +
    SCORE_MISSION_VICTORY;


// Function 1: Check whether Lift 1 can be used
bool canUseLift1(int fromFloor, int toFloor)
{
    if (fromFloor < 1 || fromFloor > NUM_FLOORS)
    {
        return false;
    }

    if (toFloor < 1 || toFloor > NUM_FLOORS)
    {
        return false;
    }

    // Lift 1 connects only floors 1 and 2
    if ((fromFloor == 1 && toFloor == 2) ||
        (fromFloor == 2 && toFloor == 1))
    {
        return true;
    }

    return false;
}


// Function 2: Check whether Lift 2 can be used
bool canUseLift2(int fromFloor, int toFloor,
                 bool lift2Operational, int emergencyTripsLeft)
{
    if (fromFloor < 1 || fromFloor > NUM_FLOORS)
    {
        return false;
    }

    if (toFloor < 1 || toFloor > NUM_FLOORS)
    {
        return false;
    }

    // Check whether the emergency trip count is valid
    if (emergencyTripsLeft < 0 ||
        emergencyTripsLeft > MAX_EMERGENCY_TRIPS)
    {
        return false;
    }

    // Lift 2 connects floors 2, 4 and 5
    bool connectedFloors = false;

    if ((fromFloor == 2 && toFloor == 4) ||
        (fromFloor == 4 && toFloor == 2) ||
        (fromFloor == 2 && toFloor == 5) ||
        (fromFloor == 5 && toFloor == 2) ||
        (fromFloor == 4 && toFloor == 5) ||
        (fromFloor == 5 && toFloor == 4))
    {
        connectedFloors = true;
    }

    if (connectedFloors == false)
    {
        return false;
    }

    // If Lift 2 is operational, allow its use
    if (lift2Operational == true)
    {
        return true;
    }

    // Otherwise, an emergency trip is required
    if (emergencyTripsLeft > 0)
    {
        return true;
    }

    return false;
}


// Function 3: Check whether the Master Key is in inventory
bool unlockFloor3Door(
    const char inventory[MAX_INVENTORY_CAPACITY],
    int inventoryCount)
{
    if (inventory == nullptr)
    {
        return false;
    }

    if (inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }

    if (inventoryCount == 0)
    {
        return false;
    }

    // Search for the Master Key
    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == ENT_MASTER_KEY)
        {
            return true;
        }
    }

    return false;
}


// Function 4: Install the Fuse at the Control Panel
bool installFuseControlPanel(
    char inventory[MAX_INVENTORY_CAPACITY],
    int& inventoryCount,
    bool objectives[NUM_OBJECTIVES],
    int& score)
{
    // Check whether the arrays are valid
    if (inventory == nullptr || objectives == nullptr)
    {
        return false;
    }

    // Check inventory count
    if (inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY)
    {
        return false;
    }

    // Check whether the score is valid
    if (score < 0 ||
        score > MAX_SCORE - SCORE_INSTALL_FUSE)
    {
        return false;
    }

    // Check whether the Fuse is already installed
    if (objectives[OBJ_FUSE_INSTALLED] == true)
    {
        return false;
    }

    // Check whether the Fuse was collected
    if (objectives[OBJ_FUSE_COLLECTED] == false)
    {
        return false;
    }

    // Search for the Fuse in inventory
    int fusePosition = -1;

    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == ENT_FUSE)
        {
            fusePosition = i;
            break;
        }
    }

    if (fusePosition == -1)
    {
        return false;
    }

    // Remove the Fuse from inventory
    for (int i = fusePosition; i < inventoryCount - 1; i++)
    {
        inventory[i] = inventory[i + 1];
    }

    inventoryCount--;

    // Clear the unused inventory position
    inventory[inventoryCount] = ENT_NONE;

    // Mark the Fuse installation objective as complete
    objectives[OBJ_FUSE_INSTALLED] = true;

    // Increase the player's score
    score += SCORE_INSTALL_FUSE;

    return true;
}


// Function 5: Verify the Cabinet Code
bool verifyCabinetCode(
    const char* enteredCode,
    const bool objectives[NUM_OBJECTIVES])
{
    if (enteredCode == nullptr || objectives == nullptr)
    {
        return false;
    }

    // Check whether all three clues have been collected
    if (objectives[OBJ_CLUE_4] == false ||
        objectives[OBJ_CLUE_2] == false ||
        objectives[OBJ_CLUE_7] == false)
    {
        return false;
    }

    // Compare the entered code with the correct code
    if (strcmp(enteredCode, "427") == 0)
    {
        return true;
    }

    return false;
}
