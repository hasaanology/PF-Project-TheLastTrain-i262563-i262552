
#include "viewer_iface.h"
#include "level.h"
#include "util.h"
#include <iostream>
#include <iomanip>
using namespace std;


// Function 1: Display the player's dashboard
void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft)
{
    // Names of all 20 rooms
    const char* roomNames[NUM_FLOORS][ROOMS_PER_FLOOR] = {
        { "Entrance Lobby", "Reception", "Waiting Area", "Utility Room" },
        { "Lounge", "Archive", "Lab", "Meeting Room" },
        { "Stairwell Annex", "Security Vestibule", "Animal Room", "Control Room" },
        { "First Aid Station", "Medical Room", "Maintenance Corridor", "Office" },
        { "Security Room", "Train Boarding Platform", "Storage Room", "Observation Deck" }
    };

    const int BAR_WIDTH = 20;
    const int LOW_LEVEL = 10;

    const int MAX_SCORE =
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

    printHeader("PLAYER STATUS");

    // Location
    cout << " Location  : ";

    if (playerFloor >= 1 && playerFloor <= NUM_FLOORS &&
        playerRoom >= 1 && playerRoom <= ROOMS_PER_FLOOR)
    {
        cout << "F" << playerFloor << "-R" << playerRoom
             << " (" << roomNames[playerFloor - 1][playerRoom - 1] << ")" << endl;
    }
    else
    {
        cout << "not chosen yet" << endl;
    }

    // Energy bar
    int safeEnergy = energy;

    if (safeEnergy < 0)
    {
        safeEnergy = 0;
    }

    if (safeEnergy > MAX_ENERGY)
    {
        safeEnergy = MAX_ENERGY;
    }

    int energyFilled = safeEnergy * BAR_WIDTH / MAX_ENERGY;

    cout << " Energy    : " << setw(3) << energy << " / "
         << setw(3) << MAX_ENERGY << "  [";

    for (int i = 0; i < BAR_WIDTH; ++i)
    {
        if (i < energyFilled)
        {
            cout << "#";
        }
        else
        {
            cout << "-";
        }
    }

    cout << "]";

    if (energy <= LOW_LEVEL)
    {
        cout << "  LOW ENERGY!";
    }

    cout << endl;

    // Time bar
    int safeTime = time;

    if (safeTime < 0)
    {
        safeTime = 0;
    }

    if (safeTime > MAX_TIME)
    {
        safeTime = MAX_TIME;
    }

    int timeFilled = safeTime * BAR_WIDTH / MAX_TIME;

    cout << " Time      : " << setw(3) << time << " / "
         << setw(3) << MAX_TIME << "  [";

    for (int i = 0; i < BAR_WIDTH; ++i)
    {
        if (i < timeFilled)
        {
            cout << "#";
        }
        else
        {
            cout << "-";
        }
    }

    cout << "]";

    if (time <= LOW_LEVEL)
    {
        cout << "  LOW TIME!";
    }

    cout << endl;

    // Score
    cout << " Score     : " << score << " / " << MAX_SCORE << endl;

    // Inventory
    int safeInventoryCount = inventoryCount;

    if (inventory == nullptr)
    {
        safeInventoryCount = 0;
    }

    if (safeInventoryCount < 0)
    {
        safeInventoryCount = 0;
    }

    if (safeInventoryCount > MAX_INVENTORY_CAPACITY)
    {
        safeInventoryCount = MAX_INVENTORY_CAPACITY;
    }

    cout << " Inventory : (" << safeInventoryCount
         << "/" << MAX_INVENTORY_CAPACITY << ")";

    if (safeInventoryCount == 0)
    {
        cout << " empty" << endl;
    }
    else
    {
        cout << endl;

        for (int i = 0; i < safeInventoryCount; ++i)
        {
            cout << "   " << setw(2) << (i + 1) << ". "
                 << getEntityName(inventory[i])
                 << " [" << inventory[i] << "]" << endl;
        }
    }

    // Emergency trips
    cout << " Emergency lift trips left : " << emergencyTripsLeft << endl;
    cout << "------------------------" << endl;
}


// Function 2: Display the details of a room
void renderRoomDetails(
    int floor, int room,
    const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
    int entityCount)
{
    const char* roomNames[NUM_FLOORS][ROOMS_PER_FLOOR] = {
        { "Entrance Lobby", "Reception", "Waiting Area", "Utility Room" },
        { "Lounge", "Archive", "Lab", "Meeting Room" },
        { "Stairwell Annex", "Security Vestibule", "Animal Room", "Control Room" },
        { "First Aid Station", "Medical Room", "Maintenance Corridor", "Office" },
        { "Security Room", "Train Boarding Platform", "Storage Room", "Observation Deck" }
    };

    printHeader("ROOM DETAILS");

    // Check whether the requested room exists
    if (floor < 1 || floor > NUM_FLOORS ||
        room < 1 || room > ROOMS_PER_FLOOR)
    {
        cout << " Invalid room." << endl;
        cout << "------------------------" << endl;
        return;
    }

    // Check whether room information is available
    if (roomEntities == nullptr)
    {
        cout << " Room information is unavailable." << endl;
        cout << "------------------------" << endl;
        return;
    }

    cout << " Room : F" << floor << "-R" << room
         << " (" << roomNames[floor - 1][room - 1] << ")" << endl;

    // Keep the entity count within valid limits
    if (entityCount < 0)
    {
        entityCount = 0;
    }

    if (entityCount > MAX_ENTITIES_PER_ROOM)
    {
        entityCount = MAX_ENTITIES_PER_ROOM;
    }

    // Display the entities inside the room
    int shown = 0;

    for (int i = 0; i < entityCount; ++i)
    {
        char symbol = roomEntities[floor - 1][room - 1][i];

        if (symbol == ENT_NONE)
        {
            continue;
        }

        if (shown == 0)
        {
            cout << " You can see:" << endl;
        }

        shown++;

        cout << "   " << setw(2) << shown << ". "
             << getEntityName(symbol)
             << " [" << symbol << "]" << endl;
    }

    if (shown == 0)
    {
        cout << " Nothing of interest here." << endl;
    }

    cout << "------------------------" << endl;
}


// Function 3: Display the ASCII building map
void renderAsciiBuilding(int playerFloor, int playerRoom)
{
    // Connections between rooms on each floor
    const char* links[NUM_FLOORS + 1][ROOMS_PER_FLOOR - 1] = {
        { "",  "",  "" },
        { "W", "",  "D" },
        { "D", "",  "D" },
        { "L", "W", "D" },
        { "W", "#", "D" },
        { "D", "D", "W" }
    };

    // Staircases on each floor
    const char* stairs[NUM_FLOORS + 1] = {
        "", "S1", "S1 S2", "S2", "S3", "S3"
    };

    // Lift 2 locations
    const char* lift2[NUM_FLOORS + 1] = {
        "", "", " L2", "", " L2", " L2"
    };

    printHeader("BUILDING MAP");

    cout << "      +------------------------------------------+" << endl;

    for (int f = NUM_FLOORS; f >= 1; --f)
    {
        // First line: room names and connections
        cout << left << setw(6) << stairs[f] << "|";

        for (int r = 1; r <= ROOMS_PER_FLOOR; ++r)
        {
            cout << " F" << f << "-R" << r << "   ";

            if (r < ROOMS_PER_FLOOR)
            {
                cout << left << setw(2) << links[f][r - 1];
            }
        }

        cout << "|" << lift2[f] << endl;

        // Second line: player position
        cout << "      |";

        for (int r = 1; r <= ROOMS_PER_FLOOR; ++r)
        {
            if (f == playerFloor && r == playerRoom)
            {
                cout << "  [YOU]  ";
            }
            else
            {
                cout << "         ";
            }

            if (r < ROOMS_PER_FLOOR)
            {
                cout << "| ";
            }
        }

        cout << "|" << endl;

        cout << "      +------------------------------------------+" << endl;
    }

    // Building map legend
    cout << endl;
    cout << " D = Door    W = Window    L = Locked door (needs Master Key)" << endl;
    cout << " # = Solid wall (no passage)" << endl;
    cout << " L1 = Lift 1 (connects room 3 on floors 1 and 2)" << endl;
    cout << " L2 = Lift 2 (room 4 of floors 2, 4 and 5)" << endl;
    cout << " S1, S2, S3 = Staircases (room 1): S1 F1-F2, S2 F2-F3, S3 F4-F5" << endl;

    if (playerFloor < 1 || playerFloor > NUM_FLOORS ||
        playerRoom < 1 || playerRoom > ROOMS_PER_FLOOR)
    {
        cout << " (The player has not chosen a start room yet.)" << endl;
    }

    cout << "------------------------" << endl;
}


// Function 4: Display the mission objectives checklist
void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES])
{
    if (objectives == nullptr)
    {
        printHeader("MISSION OBJECTIVES");
        cout << " Objective information is unavailable." << endl;
        cout << "------------------------" << endl;
        return;
    }

    const char* objectiveNames[NUM_OBJECTIVES] = {
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

    printHeader("MISSION OBJECTIVES");

    int doneCount = 0;

    // Display the first 14 actual objectives
    for (int i = 0; i < OBJ_MISSION_WON; ++i)
    {
        if (objectives[i])
        {
            cout << " [X] ";
            doneCount++;
        }
        else
        {
            cout << " [ ] ";
        }

        cout << objectiveNames[i] << endl;
    }

    cout << endl;
    cout << " Completed : " << doneCount
         << " / " << OBJ_MISSION_WON << endl;

    // Display the mission result
    cout << " Mission   : ";

    if (objectives[OBJ_MISSION_WON])
    {
        cout << "VICTORY" << endl;
    }
    else if (objectives[OBJ_MISSION_FAILED])
    {
        cout << "DEFEAT" << endl;
    }
    else
    {
        cout << "in progress" << endl;
    }

    cout << "------------------------" << endl;
}
