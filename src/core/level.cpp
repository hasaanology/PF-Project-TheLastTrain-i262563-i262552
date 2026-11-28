#include "level.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <climits>
using namespace std;

bool loadBuildingMap(const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    // TASK 1: Validate parameters

    if (filename == nullptr)
    {
        return false;
    }

    if (horizontalConn == nullptr ||
        roomEntities == nullptr ||
        entityCounts == nullptr)
    {
        return false;
    }

    // Initialize horizontal connections
    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        for (int connection = 0;
             connection < ROOMS_PER_FLOOR - 1;
             connection++)
        {
            horizontalConn[floor][connection] = CONN_NONE;
        }
    }

    // Initialize room entities and entity counts
    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        for (int room = 0; room < ROOMS_PER_FLOOR; room++)
        {
            entityCounts[floor][room] = 0;

            for (int entity = 0;
                 entity < MAX_ENTITIES_PER_ROOM;
                 entity++)
            {
                roomEntities[floor][room][entity] = ENT_NONE;
            }
        }
    }

    // TASK 2: Open and read the building file

    ifstream buildingFile(filename);

    if (!buildingFile.is_open())
    {
        return false;
    }

    string line;

    // Store positions of room labels for every floor
    int roomPositions[NUM_FLOORS][ROOMS_PER_FLOOR];

    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        for (int room = 0; room < ROOMS_PER_FLOOR; room++)
        {
            roomPositions[floor][room] = -1;
        }
    }

    // Store the floor whose entity row must be processed
    int pendingFloor = -1;

    // Expected entities for every room
char expectedEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM] =
{
    {
        {ENT_NONE},
        {ENT_MASTER_KEY},
        {ENT_NONE},
        {ENT_ENERGY_PACK}
    },

    {
        {ENT_NONE},
        {ENT_NONE},
        {ENT_EMERGENCY_TOOLKIT},
        {ENT_NONE}
    },

    {
        {ENT_NONE},
        {ENT_CLUE_4, ENT_FUSE},
        {ENT_CAT},
        {ENT_CONTROL_PANEL, ENT_LIFT_PASS}
    },

    {
        {ENT_PASSENGER},
        {ENT_MEDICAL_KIT, ENT_EMERGENCY_CARD},
        {ENT_CLUE_2},
        {ENT_NONE}
    },

    {
        {ENT_CLUE_7, ENT_CABINET_TRAIN_KEY},
        {ENT_NONE},
        {ENT_CAT_CARRIER},
        {ENT_OBSERVATION_TOKEN}
    }
};
    // Remember which floor-label rows were found
    bool floorLineFound[NUM_FLOORS] = { false };

    // Read the file one line at a time
    while (getline(buildingFile, line))
    {
        // Check maximum line length
        if (line.length() >= MAX_LINE_LEN)
        {
            buildingFile.close();
            return false;
        }

        // Skip empty lines
        if (line.empty())
        {
            continue;
        }

        // TASK 4: Read and store room entities

        if (pendingFloor != -1)
        {
            for (int room = 0; room < ROOMS_PER_FLOOR; room++)
            {
                int startPosition = roomPositions[pendingFloor][room];
                int endPosition;

                if (room < ROOMS_PER_FLOOR - 1)
                {
                    endPosition = roomPositions[pendingFloor][room + 1];
                }
                else
                {
                    // Stop at the right wall of the last room.
                    // Characters after this wall (such as Lift 2 guide dots)
                    // are map decoration, not room entities.
                    size_t rightWall = line.find('|',
                        static_cast<size_t>(startPosition));

                    if (rightWall == string::npos)
                    {
                        buildingFile.close();
                        return false;
                    }

                    endPosition = static_cast<int>(rightWall);
                }

                // Validate positions
                if (startPosition < 0 ||
                    startPosition >= static_cast<int>(line.length()) ||
                    endPosition > static_cast<int>(line.length()) ||
                    endPosition < startPosition)
                {
                    buildingFile.close();
                    return false;
                }

                // Read every character belonging to this room
                for (int i = startPosition; i < endPosition; i++)
                {
                    char symbol = line[i];

                    // Ignore spaces and room separators
                    if (isspace(static_cast<unsigned char>(symbol)) ||
                        symbol == '|')
                    {
                        continue;
                    }

                    // Check whether this is a valid entity
                    switch (symbol)
                    {
                        case ENT_MASTER_KEY:
                        case ENT_ENERGY_PACK:
                        case ENT_EMERGENCY_TOOLKIT:
                        case ENT_CLUE_4:
                        case ENT_FUSE:
                        case ENT_CAT:
                        case ENT_CONTROL_PANEL:
                        case ENT_LIFT_PASS:
                        case ENT_PASSENGER:
                        case ENT_MEDICAL_KIT:
                        case ENT_EMERGENCY_CARD:
                        case ENT_CLUE_2:
                        case ENT_CLUE_7:
                        case ENT_CABINET_TRAIN_KEY:
                        case ENT_CAT_CARRIER:
                        case ENT_OBSERVATION_TOKEN:
                            break;

                        default:
                            buildingFile.close();
                            return false;
                    }

                    // Check room capacity
                    if (entityCounts[pendingFloor][room] >=
                        MAX_ENTITIES_PER_ROOM)
                    {
                        buildingFile.close();
                        return false;
                    }

                    // Store the entity
                    int entityIndex = entityCounts[pendingFloor][room];

                    roomEntities[pendingFloor][room][entityIndex] = symbol;

                    entityCounts[pendingFloor][room]++;
                }
            }

            // Entity row has been processed
            pendingFloor = -1;
        }

        // TASK 3: Identify floor and room labels

        for (int floor = 1; floor <= NUM_FLOORS; floor++)
        {
            // Initialize room positions for this floor
            for (int room = 0; room < ROOMS_PER_FLOOR; room++)
            {
                roomPositions[floor - 1][room] = -1;
            }

            int foundRooms = 0;

            // Search for all four room labels
            for (int room = 1; room <= ROOMS_PER_FLOOR; room++)
            {
                string label =
                    "F" + to_string(floor) + "-R" + to_string(room);

                size_t position = line.find(label);

                if (position != string::npos)
                {
                    roomPositions[floor - 1][room - 1] =
                        static_cast<int>(position);

                    foundRooms++;
                }
            }

            // Check whether this line contains room labels
            if (foundRooms > 0)
            {
                // All four room labels must be present
                if (foundRooms != ROOMS_PER_FLOOR)
                {
                    buildingFile.close();
                    return false;
                }

                // Make sure room labels appear once and in the correct order
                for (int room = 0; room < ROOMS_PER_FLOOR; room++)
                {
                    string label =
                        "F" + to_string(floor) + "-R" + to_string(room + 1);

                    int position = roomPositions[floor - 1][room];

                    if (position < 0)
                    {
                        buildingFile.close();
                        return false;
                    }

                    // Reject a repeated room label
                    size_t secondPosition =
                        line.find(label, static_cast<size_t>(position + label.length()));

                    if (secondPosition != string::npos)
                    {
                        buildingFile.close();
                        return false;
                    }

                    // Each room label must appear after the previous one
                    if (room > 0 &&
                        roomPositions[floor - 1][room] <=
                        roomPositions[floor - 1][room - 1])
                    {
                        buildingFile.close();
                        return false;
                    }
                }

                // Prevent duplicate floor rows
                if (floorLineFound[floor - 1])
                {
                    buildingFile.close();
                    return false;
                }

                // Only one floor-label row should be pending
                if (pendingFloor != -1)
                {
                    buildingFile.close();
                    return false;
                }

                floorLineFound[floor - 1] = true;

                // Read horizontal connections
                for (int connection = 0;
                     connection < ROOMS_PER_FLOOR - 1;
                     connection++)
                {
                    int startPosition =
                        roomPositions[floor - 1][connection] + 5;

                    int endPosition =
                        roomPositions[floor - 1][connection + 1];

                    char connectionSymbol = CONN_NONE;

                    int connectionCount = 0;

                    // Examine characters between room labels
                    for (int i = startPosition; i < endPosition; i++)
                    {
                        char symbol = line[i];

                        if (isspace(static_cast<unsigned char>(symbol)))
                        {
                            continue;
                        }

                        if (symbol == 'D' ||
                            symbol == 'W' ||
                            symbol == '#')
                        {
                            connectionSymbol = symbol;
                            connectionCount++;
                        }
                        else if (symbol == 'L')
                        {
                            // L1 and L2 are lift markers
                            if (i + 1 < endPosition &&
                                (line[i + 1] == '1' ||
                                 line[i + 1] == '2'))
                            {
                                i++;
                            }
                            else
                            {
                                connectionSymbol = CONN_LOCKED_DOOR;
                                connectionCount++;
                            }
                        }
                        else
                        {
                            buildingFile.close();
                            return false;
                        }
                    }

                    // Reject multiple connection symbols
                    if (connectionCount > 1)
                    {
                        buildingFile.close();
                        return false;
                    }

                    // Store the connection
                    horizontalConn[floor - 1][connection] =
                        connectionSymbol;
                }

                // Remember which floor's entity row comes next
                pendingFloor = floor - 1;
            }
        }
    }

    // TASK 5: Validate the complete building map

    // Check for file-reading errors
    if (buildingFile.bad())
    {
        buildingFile.close();
        return false;
    }

    // Check that all five floors were found
    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        if (floorLineFound[floor] == false)
        {
            buildingFile.close();
            return false;
        }
    }

    // Check that the last entity row was processed
    if (pendingFloor != -1)
    {
        buildingFile.close();
        return false;
    }

    // Check expected horizontal connections
    char expectedConnections[NUM_FLOORS][ROOMS_PER_FLOOR - 1] =
    {
        {CONN_WINDOW, CONN_NONE, CONN_DOOR},
        {CONN_DOOR, CONN_NONE, CONN_DOOR},
        {CONN_LOCKED_DOOR, CONN_WINDOW, CONN_DOOR},
        {CONN_WINDOW, CONN_SOLID_WALL, CONN_DOOR},
        {CONN_DOOR, CONN_DOOR, CONN_WINDOW}
    };

    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        for (int connection = 0;
             connection < ROOMS_PER_FLOOR - 1;
             connection++)
        {
            if (horizontalConn[floor][connection] !=
                expectedConnections[floor][connection])
            {
                buildingFile.close();
                return false;
            }
        }
    }

    // Check expected entity counts from the map file
    int expectedCounts[NUM_FLOORS][ROOMS_PER_FLOOR] =
    {
        {0, 1, 0, 1},
        {0, 0, 1, 0},
        {0, 2, 1, 2},
        {1, 2, 1, 0},
        {2, 0, 1, 1}
    };
// Check every room
for (int floor = 0; floor < NUM_FLOORS; floor++)
{
    for (int room = 0; room < ROOMS_PER_FLOOR; room++)
    {
        // First check the number of entities
        if (entityCounts[floor][room] != expectedCounts[floor][room])
        {
            cout << "Error: Incorrect number of entities in a room." << endl;
            buildingFile.close();
            return false;
        }

        // Remember which expected entities have been matched
        bool matched[MAX_ENTITIES_PER_ROOM] = {false};

        // Check every entity found in the map
        for (int entity = 0; entity < entityCounts[floor][room]; entity++)
        {
            bool found = false;

            // Search for this entity in the expected entities
            for (int expected = 0;
                 expected < expectedCounts[floor][room];
                 expected++)
            {
                if (roomEntities[floor][room][entity] ==
                        expectedEntities[floor][room][expected]
                    && matched[expected] == false)
                {
                    matched[expected] = true;
                    found = true;
                    break;
                }
            }

            // Entity was not expected in this room
            if (found == false)
            {
                cout << "Error: Incorrect entity found in a room." << endl;
                buildingFile.close();
                return false;
            }
        }
    }
}
    // Special project requirement:
    // F1-R4 must contain two energy packs.
    // The map file displays only one, so add the second here.

    int energyPackCount = 0;

    for (int entity = 0; entity < entityCounts[0][3]; entity++)
    {
        if (roomEntities[0][3][entity] == ENT_ENERGY_PACK)
        {
            energyPackCount++;
        }
    }

    if (energyPackCount != 1)
    {
        buildingFile.close();
        return false;
    }

    if (entityCounts[0][3] >= MAX_ENTITIES_PER_ROOM)
    {
        buildingFile.close();
        return false;
    }

    int nextEntityIndex = entityCounts[0][3];

    roomEntities[0][3][nextEntityIndex] = ENT_ENERGY_PACK;

    entityCounts[0][3]++;

    // Close the file
    buildingFile.close();

    return true;
}




bool parseNumber(const string& value, int& result)
{
    if (value.empty())
    {
        return false;
    }

    result = 0;

    for (int i = 0; i < static_cast<int>(value.length()); i++)
    {
        if (value[i] < '0' || value[i] > '9')
        {
            return false;
        }

        int digit = value[i] - '0';

        // Prevent integer overflow
        if (result > (INT_MAX - digit) / 10)
        {
            return false;
        }

        result = result * 10 + digit;
    }

    return true;
}
bool parseYesNo(const string& value, bool& result)
{
    if (value == "YES")
    {
        result = true;
        return true;
    }

    if (value == "NO")
    {
        result = false;
        return true;
    }

    return false;
}


bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft)
{
    if (filename == nullptr)
    {
        return false;
    }
    if (inventory == nullptr || objectives == nullptr)
{
    return false;
}

    ifstream inputFile(filename);

    if (!inputFile.is_open())
    {
        return false;
    }

    // Track whether every required setting was found
    bool found[17] = { false };

    // Initialize the game state
    playerFloor = 0;
    playerRoom = 0;

    energy = INITIAL_ENERGY;
    time = INITIAL_TIME;
    score = INITIAL_SCORE;

    inventoryCount = 0;
    emergencyTripsLeft = 3;

    for (int i = 0; i < MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = ENT_NONE;
    }

    for (int i = 0; i < NUM_OBJECTIVES; i++)
    {
        objectives[i] = false;
    }

    string line;

   while (getline(inputFile, line))
{
    char lineBuffer[MAX_LINE_LEN];

    // Check whether the line is too long
    if (line.length() >= MAX_LINE_LEN)
    {
        inputFile.close();
        return false;
    }

    // Copy characters into the character array
    int i = 0;

    while (line[i] != '\0')
    {
        lineBuffer[i] = line[i];
        i++;
    }

    lineBuffer[i] = '\0';

    // Remove unnecessary spaces
    trimWhitespace(lineBuffer);

    line = lineBuffer;

    // Ignore empty lines
    if (line.empty())
    {
        continue;
    }

        size_t colonPosition = line.find(':');

        if (colonPosition == string::npos)
        {
            inputFile.close();
            return false;
        }


// Get the setting name before the colon
string key = line.substr(0, colonPosition);

// Get the setting value after the colon
string value = line.substr(colonPosition + 1);

// Remove extra spaces from the key     
i = 0;

while (key[i] != '\0')
{
    lineBuffer[i] = key[i];
    i++;
}

lineBuffer[i] = '\0';

trimWhitespace(lineBuffer);
key = lineBuffer;

// Remove extra spaces from the value
i = 0;

while (value[i] != '\0')
{
    lineBuffer[i] = value[i];
    i++;
}

lineBuffer[i] = '\0';

trimWhitespace(lineBuffer);
value = lineBuffer;

        int number = 0;
        bool yesNoValue = false;

        // GAME_STATUS
        if (key == "GAME_STATUS")
        {
             if (found[0])
    {
        inputFile.close();
        return false;
    }

            if (value != "NOT_STARTED")
            {
                inputFile.close();
                return false;
            }

            found[0] = true;
        }

        // START_ROOM
        else if (key == "START_ROOM")
        {
            if (found[1])
    {
        inputFile.close();
        return false;
    }  
            if(value != "PLAYER_SELECTS")
            {
                inputFile.close();
                return false;
            }

            // Zero means the player has not selected a room yet
            playerFloor = 0;
            playerRoom = 0;

            found[1] = true;
        }

        // INITIAL_ENERGY
        else if (key == "INITIAL_ENERGY")
        {
            if (found[2])
    {
        inputFile.close();
        return false;
    }
            if (!parseNumber(value, number) ||
                number < 0 || number > MAX_ENERGY)
            {
                inputFile.close();
                return false;
            }

            energy = number;
            found[2] = true;
        }

        // INITIAL_TIME
        else if (key == "INITIAL_TIME")
        {
            if (found[3])
{
    inputFile.close();
    return false;
}
            if (!parseNumber(value, number) ||
                number < 0 || number > MAX_TIME)
            {
                inputFile.close();
                return false;
            }

            time = number;
            found[3] = true;
        }

        // INITIAL_SCORE
        else if (key == "INITIAL_SCORE")
        {
            if (found[4])
{
    inputFile.close();
    return false;
}
            if (!parseNumber(value, number) || number > 520)
            {
                inputFile.close();
                return false;
            }

            score = number;
            found[4] = true;
        }

        // MAX_ENERGY
        else if (key == "MAX_ENERGY")
        {
            if (found[5])
{
    inputFile.close();
    return false;
}
            if (!parseNumber(value, number) || number != MAX_ENERGY)
            {
                inputFile.close();
                return false;
            }

            found[5] = true;
        }

        // MAX_TIME
        else if (key == "MAX_TIME")
        {
            if (found[6])
{
    inputFile.close();
    return false;
}
            if (!parseNumber(value, number) || number != MAX_TIME)
            {
                inputFile.close();
                return false;
            }

            found[6] = true;
        }

        // INITIAL_INVENTORY
        else if (key == "INITIAL_INVENTORY")
        {
            if (found[7])
{
    inputFile.close();
    return false;
}
            if (value != "NONE")
            {
                inputFile.close();
                return false;
            }

            inventoryCount = 0;
            found[7] = true;
        }

        // PASSENGER_RESCUED
        else if (key == "PASSENGER_RESCUED")
        {
            if (found[8])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            objectives[OBJ_PASSENGER_RESCUED] = yesNoValue;
            found[8] = true;
        }

        // CAT_RESCUED
        else if (key == "CAT_RESCUED")
        {
            if (found[9])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            objectives[OBJ_CAT_SECURED] = yesNoValue;
            found[9] = true;
        }

        // CLUES_COLLECTED
        else if (key == "CLUES_COLLECTED")
        {
            if (found[10])
            {
                inputFile.close();
                return false;
            }

            if (value == "NONE")
            {
                found[10] = true;
            }
            else if (value.empty())
            {
                inputFile.close();
                return false;
            }
            else
            {
                bool clue4Found = false;
                bool clue2Found = false;
                bool clue7Found = false;
                bool expectingDigit = true;

                for (int i = 0; i < static_cast<int>(value.length()); i++)
                {
                    char current = value[i];

                    // Ignore spaces around the comma-separated values
                    if (isspace(static_cast<unsigned char>(current)))
                    {
                        continue;
                    }

                    if (expectingDigit)
                    {
                        if (current == '4' && !clue4Found)
                        {
                            objectives[OBJ_CLUE_4] = true;
                            clue4Found = true;
                        }
                        else if (current == '2' && !clue2Found)
                        {
                            objectives[OBJ_CLUE_2] = true;
                            clue2Found = true;
                        }
                        else if (current == '7' && !clue7Found)
                        {
                            objectives[OBJ_CLUE_7] = true;
                            clue7Found = true;
                        }
                        else
                        {
                            inputFile.close();
                            return false;
                        }

                        expectingDigit = false;
                    }
                    else
                    {
                        if (current == ',')
                        {
                            expectingDigit = true;
                        }
                        else
                        {
                            inputFile.close();
                            return false;
                        }
                    }
                }

                // Reject a trailing comma or a value with no clue
                if (expectingDigit)
                {
                    inputFile.close();
                    return false;
                }

                found[10] = true;
            }
        }

       // CABINET_UNLOCKED
        else if (key == "CABINET_UNLOCKED")
        {
            if (found[11])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            objectives[OBJ_CABINET_UNLOCKED] = yesNoValue;
            found[11] = true;
        }

        // TRAIN_KEY_COLLECTED
        else if (key == "TRAIN_KEY_COLLECTED")
        {
            if (found[12])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            objectives[OBJ_TRAIN_KEY] = yesNoValue;
            found[12] = true;
        }

        // FUSE_INSTALLED
        else if (key == "FUSE_INSTALLED")
        {
            if (found[13])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            objectives[OBJ_FUSE_INSTALLED] = yesNoValue;
            found[13] = true;
        }

        // LIFT_PASS_COLLECTED
        else if (key == "LIFT_PASS_COLLECTED")
        {
            if (found[14])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            objectives[OBJ_LIFT_PASS] = yesNoValue;
            found[14] = true;
        }

        // TRAIN_PREPARED
        else if (key == "TRAIN_PREPARED")
        {
            if (found[15])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            objectives[OBJ_TRAIN_PREPARED] = yesNoValue;
            found[15] = true;
        }

        // PLAYER_AT_TRAIN
        else if (key == "PLAYER_AT_TRAIN")
        {
            if (found[16])
{
    inputFile.close();
    return false;
}
            if (!parseYesNo(value, yesNoValue))
            {
                inputFile.close();
                return false;
            }

            // The player cannot already be at the train at game start
            if (yesNoValue)
            {
                inputFile.close();
                return false;
            }

            found[16] = true;
        }

        else
        {
            // Reject unknown configuration settings
            inputFile.close();
            return false;
        }
    }

    // Check whether the file had a reading error
    if (inputFile.bad())
    {
        inputFile.close();
        return false;
    }

    inputFile.close();

    // Check that all 17 required settings were present
    for (int i = 0; i < 17; i++)
    {
        if (!found[i])
        {
            return false;
        }
    }

    return true;
}


bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
{
    // Check whether the connection array exists
    if (horizontalConn == nullptr)
    {
        return false;
    }

    // Check whether the starting floor is valid
    if (startFloor < 1 || startFloor > NUM_FLOORS)
    {
        return false;
    }

    // Check whether the starting room is valid
    if (startRoom < 1 || startRoom > ROOMS_PER_FLOOR)
    {
        return false;
    }

    // Check that every horizontal connection contains a valid symbol
    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        for (int connection = 0;
             connection < ROOMS_PER_FLOOR - 1;
             connection++)
        {
            char symbol = horizontalConn[floor][connection];

            if (symbol != CONN_NONE &&
                symbol != CONN_DOOR &&
                symbol != CONN_WINDOW &&
                symbol != CONN_LOCKED_DOOR &&
                symbol != CONN_SOLID_WALL)
            {
                return false;
            }
        }
    }

    // This BFS checks map connectivity only:
    // locked doors and Lift 2 are treated as available connections.
    // It does not check inventory or game objectives.

    // Remember which rooms have been visited
    bool visited[NUM_FLOORS][ROOMS_PER_FLOOR] = {false};

    // Queue to store rooms waiting to be checked
    int queueFloor[TOTAL_ROOMS];
    int queueRoom[TOTAL_ROOMS];

    int front = 0;
    int rear = 0;

    // Add the starting room to the queue
    queueFloor[rear] = startFloor;
    queueRoom[rear] = startRoom;
    rear++;

    // Mark starting room as visited
    visited[startFloor - 1][startRoom - 1] = true;

    // Each room is added only once because it is marked visited.
    // Therefore the queue can hold at most TOTAL_ROOMS rooms.

    // Continue while the queue is not empty
    while (front < rear)
    {
        // Take the next room from the queue
        int currentFloor = queueFloor[front];
        int currentRoom = queueRoom[front];
        front++;

        // Check whether we reached the train
        if (currentFloor == NUM_FLOORS && currentRoom == 2)
        {
            return true;
        }

        int floorIndex = currentFloor - 1;
        int roomIndex = currentRoom - 1;

        // Check the left room
        if (currentRoom > 1)
        {
            char connection = horizontalConn[floorIndex][roomIndex - 1];

            if (connection == CONN_DOOR ||
                connection == CONN_WINDOW ||
                connection == CONN_LOCKED_DOOR)
            {
                if (visited[floorIndex][roomIndex - 1] == false)
                {
                    visited[floorIndex][roomIndex - 1] = true;

                    queueFloor[rear] = currentFloor;
                    queueRoom[rear] = currentRoom - 1;
                    rear++;
                }
            }
        }

        // Check the right room
        if (currentRoom < ROOMS_PER_FLOOR)
        {
            char connection = horizontalConn[floorIndex][roomIndex];

            if (connection == CONN_DOOR ||
                connection == CONN_WINDOW ||
                connection == CONN_LOCKED_DOOR)
            {
                if (visited[floorIndex][roomIndex + 1] == false)
                {
                    visited[floorIndex][roomIndex + 1] = true;

                    queueFloor[rear] = currentFloor;
                    queueRoom[rear] = currentRoom + 1;
                    rear++;
                }
            }
        }

        // Stair S1: F1-R1 <-> F2-R1
        if (currentFloor == 1 && currentRoom == 1)
        {
            if (visited[1][0] == false)
            {
                visited[1][0] = true;
                queueFloor[rear] = 2;
                queueRoom[rear] = 1;
                rear++;
            }
        }
        else if (currentFloor == 2 && currentRoom == 1)
        {
            if (visited[0][0] == false)
            {
                visited[0][0] = true;
                queueFloor[rear] = 1;
                queueRoom[rear] = 1;
                rear++;
            }
        }

        // Stair S2: F2-R1 <-> F3-R1
        if (currentFloor == 2 && currentRoom == 1)
        {
            if (visited[2][0] == false)
            {
                visited[2][0] = true;
                queueFloor[rear] = 3;
                queueRoom[rear] = 1;
                rear++;
            }
        }
        else if (currentFloor == 3 && currentRoom == 1)
        {
            if (visited[1][0] == false)
            {
                visited[1][0] = true;
                queueFloor[rear] = 2;
                queueRoom[rear] = 1;
                rear++;
            }
        }

        // Stair S3: F4-R1 <-> F5-R1
        if (currentFloor == 4 && currentRoom == 1)
        {
            if (visited[4][0] == false)
            {
                visited[4][0] = true;
                queueFloor[rear] = 5;
                queueRoom[rear] = 1;
                rear++;
            }
        }
        else if (currentFloor == 5 && currentRoom == 1)
        {
            if (visited[3][0] == false)
            {
                visited[3][0] = true;
                queueFloor[rear] = 4;
                queueRoom[rear] = 1;
                rear++;
            }
        }

        // Lift 1: F1-R3 <-> F2-R3
        if (currentFloor == 1 && currentRoom == 3)
        {
            if (visited[1][2] == false)
            {
                visited[1][2] = true;
                queueFloor[rear] = 2;
                queueRoom[rear] = 3;
                rear++;
            }
        }
        else if (currentFloor == 2 && currentRoom == 3)
        {
            if (visited[0][2] == false)
            {
                visited[0][2] = true;
                queueFloor[rear] = 1;
                queueRoom[rear] = 3;
                rear++;
            }
        }

        // Lift 2: F2-R4, F4-R4 and F5-R4
        if (currentFloor == 2 && currentRoom == 4)
        {
            if (visited[3][3] == false)
            {
                visited[3][3] = true;
                queueFloor[rear] = 4;
                queueRoom[rear] = 4;
                rear++;
            }

            if (visited[4][3] == false)
            {
                visited[4][3] = true;
                queueFloor[rear] = 5;
                queueRoom[rear] = 4;
                rear++;
            }
        }
        else if (currentFloor == 4 && currentRoom == 4)
        {
            if (visited[1][3] == false)
            {
                visited[1][3] = true;
                queueFloor[rear] = 2;
                queueRoom[rear] = 4;
                rear++;
            }

            if (visited[4][3] == false)
            {
                visited[4][3] = true;
                queueFloor[rear] = 5;
                queueRoom[rear] = 4;
                rear++;
            }
        }
        else if (currentFloor == 5 && currentRoom == 4)
        {
            if (visited[1][3] == false)
            {
                visited[1][3] = true;
                queueFloor[rear] = 2;
                queueRoom[rear] = 4;
                rear++;
            }

            if (visited[3][3] == false)
            {
                visited[3][3] = true;
                queueFloor[rear] = 4;
                queueRoom[rear] = 4;
                rear++;
            }
        }
    }

    // Train could not be reached
    return false;
}


const char* getEntityName(char entitySymbol)
{
    switch (entitySymbol)
    {
        case ENT_NONE:
            return "Empty";

        case ENT_MASTER_KEY:
            return "Master Key";

        case ENT_ENERGY_PACK:
            return "Energy Pack";

        case ENT_EMERGENCY_TOOLKIT:
            return "Emergency Toolkit";

        case ENT_CLUE_4:
            return "Clue Digit 4";

        case ENT_FUSE:
            return "Fuse";

        case ENT_CAT:
            return "Cat";

        case ENT_CONTROL_PANEL:
            return "Control Panel";

        case ENT_LIFT_PASS:
            return "Lift Pass";

        case ENT_PASSENGER:
            return "Injured Passenger";

        case ENT_MEDICAL_KIT:
            return "Medical Kit";

        case ENT_EMERGENCY_CARD:
            return "Emergency Access Card";

        case ENT_CLUE_2:
            return "Clue Digit 2";

        case ENT_CLUE_7:
            return "Clue Digit 7";

        case ENT_CABINET_TRAIN_KEY:
            return "Cabinet / Train Key";

        case ENT_CAT_CARRIER:
            return "Cat Carrier";

        case ENT_OBSERVATION_TOKEN:
            return "Observation Token";

        default:
            return "Unknown Entity";
    }
}

