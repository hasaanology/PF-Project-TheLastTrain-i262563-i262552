#include "physics.h"
#include "util.h"

#include "physics.h"
#include "util.h"

static bool isLift2Stop(int floor, int room)
{
    if (room == 4 && (floor == 2 || floor == 4 || floor == 5))
    
        return true;
    
    else
        return false;
}

static bool isStairRoom(int floor, int room)
{
    if (room == 1 && floor >= 1 && floor <= NUM_FLOORS)
        return true;

    return false;
}

bool isDirectlyConnected(int fromF, int fromR, int toF, int toR,
                         const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                         char& outConnType)
{
    outConnType = CONN_NONE;

    // Check if the room numbers are valid
    if (fromF < 1 || fromF > NUM_FLOORS ||
        toF < 1 || toF > NUM_FLOORS ||
        fromR < 1 || fromR > ROOMS_PER_FLOOR ||
        toR < 1 || toR > ROOMS_PER_FLOOR)
    {
        return false;
    }

    if (fromF == toF && fromR == toR)
        return false;

    if (fromF == toF)
    {
        if (fromR - toR == 1 || toR - fromR == 1)
        {
            int connectionIndex;

            if (fromR < toR)
                connectionIndex = fromR - 1;
            else
                connectionIndex = toR - 1;

            char connection = horizontalConn[fromF - 1][connectionIndex];

            if (connection == CONN_DOOR ||
                connection == CONN_WINDOW ||
                connection == CONN_LOCKED_DOOR)
            {
                outConnType = connection;
                return true;
            }

            if (connection == CONN_SOLID_WALL)
            {
                outConnType = CONN_SOLID_WALL;
                return false;
            }
        }

        return false;
    }

    if (isStairRoom(fromF, fromR) && isStairRoom(toF, toR))
    {
        int lowerFloor;
        int floorGap;

        if (fromF < toF)
            lowerFloor = fromF;
        else
            lowerFloor = toF;

        if (fromF > toF)
            floorGap = fromF - toF;
        else
            floorGap = toF - fromF;

      
        if (floorGap == 1 && lowerFloor != 3)
        {
            outConnType = CONN_STAIRS;
            return true;
        }
    }

    if ((fromF == 1 && toF == 2) || (fromF == 2 && toF == 1))
    {
        if ((fromR == 2 || fromR == 3) &&
            (toR == 2 || toR == 3))
        {
            outConnType = CONN_LIFT1;
            return true;
        }
    }

    if (isLift2Stop(fromF, fromR) && isLift2Stop(toF, toR))
    {
        outConnType = CONN_LIFT2;
        return true;
    }

    return false;
}

bool canTraverse(int targetFloor, int targetRoom, char connType,
                 const char inventory[MAX_INVENTORY_CAPACITY],
                 int inventoryCount,
                 int emergencyTripsLeft,
                 bool lift2Operational)
{
    
    if (targetFloor < 1 || targetFloor > NUM_FLOORS ||
        targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR)
    {
        return false;
    }

    
    if (inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY)
        return false;
    if (connType == CONN_DOOR ||
        connType == CONN_WINDOW ||
        connType == CONN_STAIRS)
    {
        return true;
    }

    if (connType == CONN_LOCKED_DOOR)
    {
        if (hasItem(inventory, inventoryCount, ENT_MASTER_KEY))
            return true;

        return false;
    }

    if (connType == CONN_LIFT1)
        return true;

    
    if (connType == CONN_LIFT2)
    {
        if (!isLift2Stop(targetFloor, targetRoom))
            return false;

    
        if (lift2Operational)
        {
            if (hasItem(inventory, inventoryCount, ENT_LIFT_PASS))
                return true;

            return false;
        }

        if (emergencyTripsLeft > 0 &&
            hasItem(inventory, inventoryCount, ENT_EMERGENCY_CARD))
        {
            return true;
        }

        return false;
    }

    return false;
}


bool isEmergencyLift2Trip(char connType, bool lift2Operational)
{
    if (connType == CONN_LIFT2 && !lift2Operational)
        return true;

    return false;
}


void applyResourceDelta(int& energy, int& time,
                        int deltaEnergy, int deltaTime)
{
    energy = clampInt(energy + deltaEnergy, 0, MAX_ENERGY);
    time = clampInt(time + deltaTime, 0, MAX_TIME);
}


void applyScoreDelta(int& score, int deltaScore)
{
    score = score + deltaScore;

    if (score < 0)
        score = 0;
}


bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY],
             int inventoryCount, char itemSymbol)
{
    if (inventory == nullptr ||
        inventoryCount < 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        itemSymbol == ENT_NONE)
    {
        return false;
    }

    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
            return true;
    }

    return false;
}


bool addItem(char inventory[MAX_INVENTORY_CAPACITY],
             int& inventoryCount, char itemSymbol)
{
    if (inventory == nullptr ||
        inventoryCount < 0 ||
        inventoryCount >= MAX_INVENTORY_CAPACITY ||
        itemSymbol == ENT_NONE)
    {
        return false;
    }

    inventory[inventoryCount] = itemSymbol;
    inventoryCount++;

    return true;
}


bool removeItem(char inventory[MAX_INVENTORY_CAPACITY],
                int& inventoryCount, char itemSymbol)
{
    if (inventory == nullptr ||
        inventoryCount <= 0 ||
        inventoryCount > MAX_INVENTORY_CAPACITY ||
        itemSymbol == ENT_NONE)
    {
        return false;
    }

    int foundIndex = -1;

    for (int i = 0; i < inventoryCount; i++)
    {
        if (inventory[i] == itemSymbol)
        {
            foundIndex = i;
            break;
        }
    }


    if (foundIndex == -1)
        return false;

    // Move the remaining items one position to the left
    for (int i = foundIndex; i < inventoryCount - 1; i++)
    {
        inventory[i] = inventory[i + 1];
    }

    inventoryCount--;
    inventory[inventoryCount] = ENT_NONE;

    return true;
}



// Movement validation and resource adjustments without structures
