#include "logger.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cctype>
using namespace std;

// Maximum number of emergency trips
static const int MAX_EMERGENCY_TRIPS = 3;

// Maximum possible score, calculated from the scoring constants
static const int MAX_POSSIBLE_SCORE =
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

// All valid item symbols
static const char VALID_SYMBOLS[] = "<)\\4*~;(!%$27^@&";

// Names used to save and load objective information
static const char* OBJECTIVE_KEYS[NUM_OBJECTIVES] = {
    "OBJ_MASTER_KEY", "OBJ_CLUE_4", "OBJ_CLUE_2", "OBJ_CLUE_7",
    "OBJ_CABINET_UNLOCKED", "OBJ_TRAIN_KEY", "OBJ_FUSE_COLLECTED",
    "OBJ_FUSE_INSTALLED", "OBJ_LIFT_PASS", "OBJ_LIFT2_ACTIVE",
    "OBJ_PASSENGER_RESCUED", "OBJ_CAT_CARRIER", "OBJ_CAT_SECURED",
    "OBJ_TRAIN_PREPARED", "OBJ_MISSION_WON", "OBJ_MISSION_FAILED"
};

// User-friendly objective names for the summary report
static const char* OBJECTIVE_NAMES[NUM_OBJECTIVES] = {
    "Master Key collected",
    "Clue digit 4 found",
    "Clue digit 2 found",
    "Clue digit 7 found",
    "Cabinet unlocked (code 427)",
    "Train Key acquired",
    "Fuse collected",
    "Fuse installed at Control Panel",
    "Lift Pass collected",
    "Lift 2 active",
    "Injured passenger rescued",
    "Cat Carrier collected",
    "Cat secured",
    "Train prepared and boarded",
    "Mission won",
    "Mission failed"
};

// Score awarded for completing each objective
static const int OBJECTIVE_POINTS[NUM_OBJECTIVES] = {
    SCORE_KEY_COLLECT,          // Master Key
    SCORE_CLUE_FOUND,           // Clue 4
    SCORE_CLUE_FOUND,           // Clue 2
    SCORE_CLUE_FOUND,           // Clue 7
    SCORE_CABINET_UNLOCK,       // Cabinet
    SCORE_TRAIN_KEY,            // Train Key
    0,                          // Fuse collected
    SCORE_INSTALL_FUSE,         // Fuse installed
    SCORE_LIFT_PASS,            // Lift Pass
    0,                          // Lift 2 active
    SCORE_RESCUE_PASSENGER,     // Passenger rescued
    0,                          // Cat Carrier
    SCORE_SECURE_CAT,           // Cat secured
    SCORE_PREPARE_TRAIN,        // Train prepared
    0,                          // Mission won
    0                           // Mission failed
};


bool saveGameStateToFile(const char* filename,
                         int playerFloor, int playerRoom,
                         int energy, int time, int score,
                         const char inventory[MAX_INVENTORY_CAPACITY],
                         int inventoryCount,
                         const bool objectives[NUM_OBJECTIVES],
                         int emergencyTripsLeft,
                         const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                         const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    // Check whether the filename is valid
    if (filename == nullptr)
    {
        return false;
    }

    // Check whether the arrays exist
    if (inventory == nullptr || objectives == nullptr ||
        roomEntities == nullptr || entityCounts == nullptr)
    {
        return false;
    }

    // Check whether the game values are within valid limits
    if (playerFloor < 1 || playerFloor > NUM_FLOORS ||
        playerRoom < 1 || playerRoom > ROOMS_PER_FLOOR ||
        inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY ||
        energy < 0 || energy > MAX_ENERGY ||
        time < 0 || time > MAX_TIME ||
        score < 0 || score > MAX_POSSIBLE_SCORE ||
        emergencyTripsLeft < 0 ||
        emergencyTripsLeft > MAX_EMERGENCY_TRIPS)
    {
        return false;
    }

    string validSymbols = VALID_SYMBOLS;

    // Validate every inventory symbol
    for (int i = 0; i < inventoryCount; i++)
    {
        if (validSymbols.find(inventory[i]) == string::npos)
        {
            return false;
        }
    }

    // Validate all room entity counts and symbols
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            if (entityCounts[f][r] < 0 ||
                entityCounts[f][r] > MAX_ENTITIES_PER_ROOM)
            {
                return false;
            }

            for (int i = 0; i < entityCounts[f][r]; i++)
            {
                if (validSymbols.find(roomEntities[f][r][i]) == string::npos)
                {
                    return false;
                }
            }
        }
    }

    // Open the save file
    ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    // Write the file header
    file << "THE LAST TRAIN SAVE FILE" << endl;

    // Save player information
    file << "PLAYER_FLOOR: " << playerFloor << endl;
    file << "PLAYER_ROOM: " << playerRoom << endl;
    file << "ENERGY: " << energy << endl;
    file << "TIME: " << time << endl;
    file << "SCORE: " << score << endl;
    file << "EMERGENCY_TRIPS_LEFT: " << emergencyTripsLeft << endl;

    // Save inventory count
    file << "INVENTORY_COUNT: " << inventoryCount << endl;

    // Save inventory items
    file << "INVENTORY:";

    if (inventoryCount == 0)
    {
        file << " NONE";
    }

    for (int i = 0; i < inventoryCount; i++)
    {
        file << " " << inventory[i];
    }

    file << endl;

    // Save all objective states
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        file << OBJECTIVE_KEYS[i] << ": ";

        if (objectives[i])
        {
            file << "YES" << endl;
        }
        else
        {
            file << "NO" << endl;
        }
    }

    // Save the entities remaining in every room
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            int count = entityCounts[f][r];

            file << "ROOM_F" << (f + 1) << "_R" << (r + 1) << ":";

            if (count == 0)
            {
                file << " NONE";
            }

            for (int i = 0; i < count; i++)
            {
                file << " " << roomEntities[f][r][i];
            }

            file << endl;
        }
    }

    // Close the file and check for writing errors
    file.close();

    return !file.fail();
}


bool loadGameStateFromFile(const char* filename,
                           int& playerFloor, int& playerRoom,
                           int& energy, int& time, int& score,
                           char inventory[MAX_INVENTORY_CAPACITY],
                           int& inventoryCount,
                           bool objectives[NUM_OBJECTIVES],
                           int& emergencyTripsLeft,
                           char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                           int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    // Check whether the filename is valid
    if (filename == nullptr)
    {
        return false;
    }

    // Check whether the arrays exist
    if (inventory == nullptr || objectives == nullptr ||
        roomEntities == nullptr || entityCounts == nullptr)
    {
        return false;
    }

    // Open the save file
    ifstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    // Names of the seven numerical values in the save file
    const int NUMBER_KEYS = 7;

    const char* numberKeys[NUMBER_KEYS] = {
        "PLAYER_FLOOR", "PLAYER_ROOM", "ENERGY", "TIME",
        "SCORE", "EMERGENCY_TRIPS_LEFT", "INVENTORY_COUNT"
    };

    int numberValues[NUMBER_KEYS] = { 0 };
    bool numberFound[NUMBER_KEYS] = { false };

    // Temporary inventory array
    char loadedInventory[MAX_INVENTORY_CAPACITY];

    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        loadedInventory[i] = ENT_NONE;
    }

    int loadedInventorySymbols = 0;
    bool inventoryFound = false;

    // Temporary objective array
    bool loadedObjectives[NUM_OBJECTIVES] = { false };
    bool objectiveFound[NUM_OBJECTIVES] = { false };

    // Temporary room entity arrays
    char loadedEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];

    int loadedCounts[NUM_FLOORS][ROOMS_PER_FLOOR] = { 0 };

    bool roomFound[NUM_FLOORS][ROOMS_PER_FLOOR] = { false };

    // Initialize all temporary room entities
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            for (int i = 0; i < MAX_ENTITIES_PER_ROOM; i++)
            {
                loadedEntities[f][r][i] = ENT_NONE;
            }
        }
    }

    bool headerFound = false;

    string validSymbols = VALID_SYMBOLS;

    // Find the larger array capacity for temporary list storage
    const int LIST_SIZE =
        (MAX_INVENTORY_CAPACITY > MAX_ENTITIES_PER_ROOM)
        ? MAX_INVENTORY_CAPACITY : MAX_ENTITIES_PER_ROOM;

    string line;

    // Read the save file one line at a time
    while (getline(file, line))
    {
        // Remove the carriage return from Windows-style lines
        if (!line.empty() && line[line.length() - 1] == '\r')
        {
            line.erase(line.length() - 1);
        }

        // Find the first non-whitespace character
        size_t start = line.find_first_not_of(" \t");

        // Ignore empty lines
        if (start == string::npos)
        {
            continue;
        }

        // Find the last non-whitespace character
        size_t end = line.find_last_not_of(" \t");

        // Remove unnecessary spaces around the complete line
        line = line.substr(start, end - start + 1);

        // Check the save file header
        if (line == "THE LAST TRAIN SAVE FILE")
        {
            // Reject duplicate headers
            if (headerFound)
            {
                return false;
            }

            headerFound = true;
            continue;
        }

        // Reject any data appearing before the header
        if (!headerFound)
        {
            return false;
        }

        // Split the line at the colon
        size_t colon = line.find(':');

        if (colon == string::npos)
        {
            return false;
        }

        string key = line.substr(0, colon);
        string value = line.substr(colon + 1);

        // Remove spaces around the key
        size_t keyStart = key.find_first_not_of(" \t");

        if (keyStart == string::npos)
        {
            return false;
        }

        key = key.substr(
            keyStart,
            key.find_last_not_of(" \t") - keyStart + 1
        );

        // Remove spaces around the value
        size_t valueStart = value.find_first_not_of(" \t");

        if (valueStart == string::npos)
        {
            value = "";
        }
        else
        {
            value = value.substr(
                valueStart,
                value.find_last_not_of(" \t") - valueStart + 1
            );
        }

        // Convert the key and value to uppercase
        for (int i = 0; i < (int)key.length(); i++)
        {
            key[i] = (char)toupper((unsigned char)key[i]);
        }

        for (int i = 0; i < (int)value.length(); i++)
        {
            value[i] = (char)toupper((unsigned char)value[i]);
        }

        // Identify whether this is a numerical key
        int numberIndex = -1;

        for (int i = 0; i < NUMBER_KEYS; i++)
        {
            if (key == numberKeys[i])
            {
                numberIndex = i;
                break;
            }
        }

        // Identify whether this is an objective key
        int objectiveIndex = -1;

        for (int i = 0; i < NUM_OBJECTIVES; i++)
        {
            if (key == OBJECTIVE_KEYS[i])
            {
                objectiveIndex = i;
                break;
            }
        }

        bool isInventoryKey = (key == "INVENTORY");

        // Identify room keys such as ROOM_F3_R2
        int roomFloorIndex = -1;
        int roomIndex = -1;

        if (key.length() == 10 &&
            key.substr(0, 6) == "ROOM_F" &&
            isdigit((unsigned char)key[6]) &&
            key.substr(7, 2) == "_R" &&
            isdigit((unsigned char)key[9]))
        {
            int f = key[6] - '0';
            int r = key[9] - '0';

            if (f >= 1 && f <= NUM_FLOORS &&
                r >= 1 && r <= ROOMS_PER_FLOOR)
            {
                roomFloorIndex = f - 1;
                roomIndex = r - 1;
            }
        }
        if (numberIndex >= 0)
        {
            // Reject duplicate numerical keys
            if (numberFound[numberIndex])
            {
                return false;
            }

            // The number must contain between 1 and 9 digits
            bool validNumber =
                (value.length() > 0 && value.length() <= 9);

            for (int i = 0; i < (int)value.length(); i++)
            {
                if (!isdigit((unsigned char)value[i]))
                {
                    validNumber = false;
                }
            }

            if (!validNumber)
            {
                return false;
            }

            numberValues[numberIndex] = stoi(value);
            numberFound[numberIndex] = true;
        }

        else if (objectiveIndex >= 0)
        {
            // Reject duplicate objective keys
            if (objectiveFound[objectiveIndex])
            {
                return false;
            }

            if (value == "YES")
            {
                loadedObjectives[objectiveIndex] = true;
            }
            else if (value == "NO")
            {
                loadedObjectives[objectiveIndex] = false;
            }
            else
            {
                return false;
            }

            objectiveFound[objectiveIndex] = true;
        }

        else if (isInventoryKey || roomIndex >= 0)
        {
            // Reject duplicate inventory entries
            if (isInventoryKey && inventoryFound)
            {
                return false;
            }

            // Reject duplicate room entries
            if (!isInventoryKey &&
                roomFound[roomFloorIndex][roomIndex])
            {
                return false;
            }

            // Select the correct maximum list size
            int listLimit = MAX_ENTITIES_PER_ROOM;

            if (isInventoryKey)
            {
                listLimit = MAX_INVENTORY_CAPACITY;
            }

            char list[LIST_SIZE];
            int listCount = 0;

            // NONE means the list is empty
            if (value != "NONE")
            {
                for (int i = 0; i < (int)value.length(); i++)
                {
                    char ch = value[i];

                    // Ignore spaces and tabs between symbols
                    if (ch == ' ' || ch == '\t')
                    {
                        continue;
                    }

                    // Check the symbol and available capacity
                    if (validSymbols.find(ch) == string::npos ||
                        listCount >= listLimit)
                    {
                        return false;
                    }

                    list[listCount] = ch;
                    listCount++;
                }
            }

            // Copy the list into the correct temporary array
            if (isInventoryKey)
            {
                for (int i = 0; i < listCount; i++)
                {
                    loadedInventory[i] = list[i];
                }

                loadedInventorySymbols = listCount;
                inventoryFound = true;
            }
            else
            {
                for (int i = 0; i < listCount; i++)
                {
                    loadedEntities[roomFloorIndex][roomIndex][i] = list[i];
                }

                loadedCounts[roomFloorIndex][roomIndex] = listCount;
                roomFound[roomFloorIndex][roomIndex] = true;
            }
        }

        // Reject unknown keys
        else
        {
            return false;
        }
    }

    // Check whether the file had a reading error
    if (file.bad())
    {
        return false;
    }

    if (!headerFound || !inventoryFound)
    {
        return false;
    }

    // Check that all numerical values were found
    for (int i = 0; i < NUMBER_KEYS; i++)
    {
        if (!numberFound[i])
        {
            return false;
        }
    }

    // Check that all objectives were found
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        if (!objectiveFound[i])
        {
            return false;
        }
    }

    // Check that all rooms were found
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            if (!roomFound[f][r])
            {
                return false;
            }
        }
    }
    int floorValue = numberValues[0];
    int roomValue = numberValues[1];
    int energyValue = numberValues[2];
    int timeValue = numberValues[3];
    int scoreValue = numberValues[4];
    int tripsValue = numberValues[5];
    int inventoryCountValue = numberValues[6];

    if (floorValue < 1 || floorValue > NUM_FLOORS ||
        roomValue < 1 || roomValue > ROOMS_PER_FLOOR)
    {
        return false;
    }

    if (energyValue < 0 || energyValue > MAX_ENERGY)
    {
        return false;
    }

    if (timeValue < 0 || timeValue > MAX_TIME)
    {
        return false;
    }

    if (scoreValue < 0 || scoreValue > MAX_POSSIBLE_SCORE)
    {
        return false;
    }

    if (tripsValue < 0 || tripsValue > MAX_EMERGENCY_TRIPS)
    {
        return false;
    }

    if (inventoryCountValue < 0 ||
        inventoryCountValue > MAX_INVENTORY_CAPACITY ||
        inventoryCountValue != loadedInventorySymbols)
    {
        return false;
    }
    playerFloor = floorValue;
    playerRoom = roomValue;
    energy = energyValue;
    time = timeValue;
    score = scoreValue;
    emergencyTripsLeft = tripsValue;
    inventoryCount = inventoryCountValue;

    // Restore inventory
    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = loadedInventory[i];
    }

    // Restore objectives
    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        objectives[i] = loadedObjectives[i];
    }

    // Restore room entities and their counts
    for (int f = 0; f < NUM_FLOORS; f++)
    {
        for (int r = 0; r < ROOMS_PER_FLOOR; r++)
        {
            entityCounts[f][r] = loadedCounts[f][r];

            for (int i = 0; i < MAX_ENTITIES_PER_ROOM; i++)
            {
                roomEntities[f][r][i] = loadedEntities[f][r][i];
            }
        }
    }

    return true;
}

void writeGameSummaryReport(const char* filename,
                            int score, int remainingEnergy, int remainingTime,
                            const bool objectives[NUM_OBJECTIVES],
                            bool victory)
{
    // Check the filename
    if (filename == nullptr)
    {
        cout << "Warning: no file name was given for the summary report."
             << endl;
        return;
    }

    // Check the objective array
    if (objectives == nullptr)
    {
        cout << "Warning: objective data is missing." << endl;
        return;
    }

    // Open the report file
    ofstream file(filename);

    if (!file.is_open())
    {
        cout << "Warning: could not write the summary report to "
             << filename << endl;
        return;
    }

    file << "===========================================" << endl;
    file << "        THE LAST TRAIN - GAME SUMMARY" << endl;
    file << "===========================================" << endl;
    file << endl;

    file << "FINAL RESULT" << endl;

    if (victory)
    {
        file << "  Result           : VICTORY" << endl;
    }
    else
    {
        file << "  Result           : DEFEAT" << endl;

        if (remainingEnergy <= 0 && remainingTime <= 0)
        {
            file << "  Reason           : Energy and Time both reached 0"
                 << endl;
        }
        else if (remainingEnergy <= 0)
        {
            file << "  Reason           : Energy reached 0" << endl;
        }
        else if (remainingTime <= 0)
        {
            file << "  Reason           : Time reached 0" << endl;
        }
        else
        {
            file << "  Reason           : Mission ended before it was completed"
                 << endl;
        }
    }

    file << "  Final score      : " << score
         << " / " << MAX_POSSIBLE_SCORE << endl;

    file << "  Remaining energy : " << remainingEnergy << endl;
    file << "  Remaining time   : " << remainingTime << endl;
    file << endl;


    file << "OBJECTIVE CHECKLIST" << endl;

    int doneCount = 0;
    int expectedScore = 0;

    // The first 14 entries represent the actual game objectives
    for (int i = 0; i < OBJ_MISSION_WON; i++)
    {
        if (objectives[i])
        {
            file << "  [X] ";
            doneCount++;

            expectedScore = expectedScore + OBJECTIVE_POINTS[i];
        }
        else
        {
            file << "  [ ] ";
        }

        if (OBJECTIVE_POINTS[i] > 0)
        {
            file << left << setw(34) << OBJECTIVE_NAMES[i]
                 << "(+" << OBJECTIVE_POINTS[i] << ")" << endl;
        }
        else
        {
            file << OBJECTIVE_NAMES[i] << endl;
        }
    }

    // Display the victory bonus separately
    if (victory)
    {
        file << "  [X] " << left << setw(34)
             << "Full mission victory bonus"
             << "(+" << SCORE_MISSION_VICTORY << ")" << endl;

        expectedScore = expectedScore + SCORE_MISSION_VICTORY;
    }
    else
    {
        file << "  [ ] " << left << setw(34)
             << "Full mission victory bonus"
             << "(+" << SCORE_MISSION_VICTORY << ")" << endl;
    }

    file << endl;

    file << "  Objectives completed : "
         << doneCount << " / " << OBJ_MISSION_WON << endl;

    file << endl;

    file << "SESSION AUDIT" << endl;

    file << "  Score expected from the objectives : "
         << expectedScore << endl;

    file << "  Score reported by the game         : "
         << score << endl;

    // Check whether the score matches the completed objectives
    file << "  Score check     : ";

    if (expectedScore == score)
    {
        file << "PASSED" << endl;
    }
    else
    {
        file << "MISMATCH" << endl;
    }

    // Check energy and time values
    file << "  Resource check  : ";

    if (remainingEnergy >= 0 && remainingEnergy <= MAX_ENERGY &&
        remainingTime >= 0 && remainingTime <= MAX_TIME)
    {
        file << "PASSED (Energy and Time are within their valid limits)"
             << endl;
    }
    else
    {
        file << "OUT OF RANGE" << endl;
    }

    // Check whether the victory result is consistent
    file << "  Result check    : ";

    if (victory && !objectives[OBJ_TRAIN_PREPARED])
    {
        file << "INCONSISTENT (victory without the train prepared)"
             << endl;
    }
    else if (victory &&
             (remainingEnergy <= 0 || remainingTime <= 0))
    {
        file << "INCONSISTENT (victory with no Energy or Time left)"
             << endl;
    }
    else
    {
        file << "PASSED" << endl;
    }

    file << endl;

    file << "===========================================" << endl;
    file << "                END OF REPORT" << endl;
    file << "===========================================" << endl;

    // Close the file and report any writing error
    file.close();

    if (file.fail())
    {
        cout << "Warning: an error occurred while writing the summary report."
             << endl;
    }
}
