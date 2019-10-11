#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define TRUE                1
#define FALSE               (!TRUE)

#define INCREMENT_AMOUNT    1 //Add one record at a time

#define STRBUFFERSIZE       255

/*  Define the structure of the STORAGE_DEVICE database */

/*  Create a _STORAGE_DEVICE structure name 
    This allows _STORAGE_DEVICE to be used in the declaration of the 
    structure as a set of pounters
*/
struct _STORAGE_DEVICE;

typedef struct _STORAGE_DEVICE {
    struct _STORAGE_DEVICE *NextStorageDevice; // Link to next, or NULL if none
    struct _STORAGE_DEVICE *PrevStorageDevice; //Link to previous, or NULL if none

    long int byteCapacity; // Capacity in bytes
    char canBeOverwritten; // FALSE if you can't overwrite a storage, TRUE if can
    char storageType[26]; //25 chars for a storage type; 1 for null at end
    int InventoryNumber;
} STORAGE_DEVICE;

//typedef STORAGE_DEVICE near *NPSTORAGE_DEVICE;
typedef STORAGE_DEVICE *PSTORAGE_DEVICE;

void GiveHelp(void);

FILE *openFileWT(char* fileName)
{
    FILE *file = fopen(fileName, "wt");
    if (file == NULL)
    {
        printf("ERROR: File '%s' couldn't be opened. \n", fileName);
        exit(4);
    }
}

void SaveRecords(FILE* OutputFile, char* outputFileName, 
            PSTORAGE_DEVICE TempStorageDevice, 
            PSTORAGE_DEVICE FirstStorageDevice, 
            int nDebug);

void main(int argc, char* argv[]) {
    // FILE *InputFile;
    FILE *OutputFile;
    PSTORAGE_DEVICE FirstStorageDevice = NULL;
    PSTORAGE_DEVICE LastStorageDevice = NULL;
    PSTORAGE_DEVICE StorageDevice = NULL;
    PSTORAGE_DEVICE TempStorageDevice = NULL;

    // Динамически, потому что передаем в функцию
    // char* inputFileName; 
    char* outputFileName;

    // Локальный буфер, никуда не передаем
    char szBuffer[257];

    int nNotDone = TRUE;
    int nDebug = FALSE;
    int nNeedSaving = FALSE;

    if (argc != 2)
    {
        printf("ERROR: Output file not specified. \n");
        exit(4);
    }
    else
    {
        // inputFileName = (char*) malloc(strlen ( argv[1] ) +1 );
        outputFileName = (char*) malloc(strlen ( argv[2] ) +1 );
        //strcpy(inputFileName, argv[1]);
        strcpy(outputFileName, argv[2]);
    }

    OutputFile = openFileWT(outputFileName);
    fclose(OutputFile);

    /* Предполагалось, что для тестов можно переключать стандартный поток ввода 
       на ввод из файла, однако если сделать так, как ниже
       и прописать везде
       fgets(szBuffer, sizeof(szBuffer), InputFile);
       то ввод из printf сразу пойдет в fgets и все бесконечно зациклится.
       Поэтому тесты только с ручным вводом из stdin
    */

    // if (strcpy(inputFileName, "stdin") == 0)
    // {
    //     InputFile = stdin;
    // }
    // else
    // {
    //     InputFile = openFileWT(inputFileName);
    // }
    
    
    printf("Demo of a linked list concepts\n"
        "\n"
        " Commands are: \n"
        "   A - Add a storage device record. \n"
        "   D - Display current list. \n"
        "   X - Exit from program. \n"
        "   Z - Toggle debug mode. \n"
        "   ? - Display the command list."
        "   H - Display the command list.\n"
        "   S - Save the list. \n"
        "   L - Enter capacity and display all devices with less capacity. \n"
        "\n"
    );

    while (nNotDone) {
        printf("Enter command (A, D, X, Z, S, L)?");
        fgets(szBuffer, sizeof(szBuffer), stdin);
        
        switch(szBuffer[0])
        {
            case 'H': /* Give some help */
            case 'h':
            case '?':
                GiveHelp();
                break;

            case 'A': //Add a record
            case 'a':
                StorageDevice = (PSTORAGE_DEVICE)calloc(sizeof(STORAGE_DEVICE), INCREMENT_AMOUNT);
                printf("Enter storage capacity in bytes ");
                fgets(szBuffer, sizeof(szBuffer), stdin);
                sscanf(szBuffer, "%ld", &StorageDevice->byteCapacity);

                if (StorageDevice->byteCapacity > 0)
                { /* Insert this record in the list, sorted by byteCapacity */
                    nNeedSaving = TRUE;
                    if (FirstStorageDevice == NULL) {
                        printf("It is a first record \n");
                        StorageDevice->NextStorageDevice = NULL;
                        StorageDevice->PrevStorageDevice = NULL;

                        FirstStorageDevice = StorageDevice;
                        LastStorageDevice = StorageDevice;
                        TempStorageDevice = NULL;
                    }
                    else 
                    {
                        TempStorageDevice = FirstStorageDevice;
                    }

                    /*  Searching for a place to insert the element
                        TempStorageDevice a current checked storage device, 
                        starting from first one 
                        in the list
                    */

                    while (TempStorageDevice)
                    {
                        if (nDebug)
                        {
                            printf("TESTING FOR ADD: %ld %ld\n",
                                StorageDevice->byteCapacity,
                                TempStorageDevice->byteCapacity);
                        }

                        // If StorageDevice byteCapacity < TempStorageDevice byteCapacity
                        // Or TempStorageDevice is last one
                        if (StorageDevice->byteCapacity <  
                            TempStorageDevice->byteCapacity || 
                            TempStorageDevice == LastStorageDevice)
                        {
                            /*  If TempStorageDevice is also a first one
                                There is only one record
                                If StorageDevice byteCapacity < TempStorageDevice byteCapacity

                                BEFORE: [First]
                                        [Temp]

                                AFTER : [NULL] --> [StorageDevice] --> [First] 
                                                                       [Temp]
                            */
                            if (StorageDevice->byteCapacity < 
                            TempStorageDevice->byteCapacity && 
                            TempStorageDevice == FirstStorageDevice) 
                            {
                                if (nDebug)
                                {
                                    printf("Assigning as first\n");
                                }
                                StorageDevice->NextStorageDevice = FirstStorageDevice;
                                FirstStorageDevice = StorageDevice;
                                StorageDevice->PrevStorageDevice = NULL;
                                TempStorageDevice = StorageDevice->NextStorageDevice;
                                TempStorageDevice->PrevStorageDevice = StorageDevice;
                            }
                            else
                            {
                                /*  If Cusromer byteCapacity > TempStorageDevice byteCapacity
                                    And TempStorageDevice id a last one

                                    BEFORE: [Prev or NULL] --> [Last] --> NULL
                                                               [Temp]

                                    LIST: [Last] --> [StorageDevice] --> NULL
                                          [Temp]
                                */
                                if (StorageDevice->byteCapacity >
                                    TempStorageDevice->byteCapacity && 
                                    TempStorageDevice == LastStorageDevice)
                                {
                                    if (nDebug)
                                    {
                                        printf("Assigning as last\n");
                                    }
                                    StorageDevice->PrevStorageDevice = LastStorageDevice;
                                    LastStorageDevice = StorageDevice;
                                    StorageDevice -> NextStorageDevice = NULL;
                                    TempStorageDevice = StorageDevice->PrevStorageDevice;
                                    TempStorageDevice->NextStorageDevice = StorageDevice;
                                }
                                else
                                { /* Insert in the middle of the list
                                    [Prev] --> [Next]
                                               [Temp]

                                    [Prev] --> [StorageDevice] --> [Next]
                                                                   [Temp]

                                    [Prev] --> [StorageDevice] --> [Next]
                                    [Temp]
                                */
                                    if (nDebug)
                                    {
                                        printf("Assigning inside list\n");
                                    }

                                    StorageDevice->PrevStorageDevice = TempStorageDevice->PrevStorageDevice;
                                    StorageDevice->NextStorageDevice = TempStorageDevice;
                                    TempStorageDevice->PrevStorageDevice = StorageDevice;
                                    TempStorageDevice = StorageDevice->PrevStorageDevice;
                                    TempStorageDevice->NextStorageDevice = StorageDevice;
                                }                             
                            }
                            TempStorageDevice = NULL;
                        }
                        else
                        {
                            TempStorageDevice = TempStorageDevice->NextStorageDevice;
                        }
                    }

                    if(!nDebug)
                    {   
                        printf("Enter storage type (ex. CD, HDD, DVD): ");
                        fgets(szBuffer, sizeof(szBuffer), stdin);
                        szBuffer[sizeof(StorageDevice->storageType) - 1] = '\0';
                        strcpy(StorageDevice->storageType, szBuffer);

                        printf("Enter Inventory number: ");
                        fgets(szBuffer, sizeof(szBuffer), stdin);
                        sscanf(szBuffer, "%d", &StorageDevice->InventoryNumber);

                        printf("Enter 1 if the device can be overwritten, 0 if can't: ");
                        fgets(szBuffer, sizeof(szBuffer), stdin);
                        sscanf(szBuffer, "%c", &StorageDevice->canBeOverwritten);
                    }
                }
                else
                {
                    printf("\aSorry, name must not be blank!\n");
                }
                break;
            case 'Z': // Debug mode toggle
            case 'z':
                nDebug = !nDebug;
                break;
            
            case 'D': //Display all records
            case 'd':
                TempStorageDevice = FirstStorageDevice;
                printf("Display Storage Devices\n");
                while (TempStorageDevice)
                {
                    if (nDebug)
                    {
                        printf(
                            "Name '%10s' Me %lp Next %lp Prev %lp\n", 
                            TempStorageDevice->byteCapacity, 
                            TempStorageDevice,
                            TempStorageDevice->NextStorageDevice,
                            TempStorageDevice->PrevStorageDevice);
                    }
                    else
                    {
                        char canOverwrite = TempStorageDevice->canBeOverwritten;
                        printf("Capacity (byte) %ld Type %s Overwritten %3s "
                            "Inventory Number %d \n", 
                        TempStorageDevice->byteCapacity, 
                        TempStorageDevice->storageType,
                        canOverwrite ? "Yes" : "No",
                        TempStorageDevice->InventoryNumber);
                    }
                    TempStorageDevice = TempStorageDevice->NextStorageDevice;
                }
                break;

            case 'L': //Display all records, that are less than given number
            case 'l':
                printf("Enter desired capacity: \n");    
                long int capacity = 0;
                fgets(szBuffer, sizeof(szBuffer), stdin);
                sscanf(szBuffer, "%ld", &capacity);

                TempStorageDevice = FirstStorageDevice;
                printf("Display Storage Devices that store less data than %ld\n", capacity);
                while (TempStorageDevice && TempStorageDevice->byteCapacity <= capacity)
                {
                    if (nDebug)
                    {
                        printf(
                            "Name '%10s' Me %lp Next %lp Prev %lp\n", 
                            TempStorageDevice->byteCapacity, 
                            TempStorageDevice,
                            TempStorageDevice->NextStorageDevice,
                            TempStorageDevice->PrevStorageDevice);
                    }
                    else
                    {
                        char canOverwrite = TempStorageDevice->canBeOverwritten;
                        printf("Capacity (byte) %ld Type %s Overwritten %3s "
                            "Inventory Number %d \n", 
                        TempStorageDevice->byteCapacity, 
                        TempStorageDevice->storageType,
                        canOverwrite ? "Yes" : "No",
                        TempStorageDevice->InventoryNumber);
                    }
                    TempStorageDevice = TempStorageDevice->NextStorageDevice;
                }
                break;
            
            case 'X': // Exit; prompt for save if needed
            case 'x':
                nNotDone = FALSE;

                szBuffer[0] = '\0';

                while (nNeedSaving && szBuffer[0] == '\0')
                {
                    printf("\nSave the data? (у|n)");
                    fgets(szBuffer, sizeof(szBuffer), stdin);
                    if (szBuffer[0] == 'n' || szBuffer[0] == 'N')
                    {
                        nNeedSaving = FALSE;
                    }
                    else
                    {
                        if (szBuffer[0] != 'y' && szBuffer[0] != 'Y')
                        {
                            printf("\nWrong answer, "
                                "please respond with 'y' or 'n'");

                            szBuffer[0] = '\0';
                        }
                    }
                }

                if(nNeedSaving)
                { 
                    SaveRecords(OutputFile, outputFileName, TempStorageDevice,
                            FirstStorageDevice, nDebug);
                }

                // Free all the memory
                TempStorageDevice = FirstStorageDevice;
                // Ends when TempStorageDevice is NULL
                while (TempStorageDevice)
                {
                    PSTORAGE_DEVICE p_next = TempStorageDevice->NextStorageDevice;
                    free(TempStorageDevice);
                    TempStorageDevice = p_next;
                }
                break;

            case 'S': //Save all records
            case 's':
                SaveRecords(OutputFile, outputFileName, TempStorageDevice,
                            FirstStorageDevice, nDebug);
                nNeedSaving = FALSE;
                break;   
        }
    }

    // if (strcpy("stdin", inputFileName) != 0)
    // {
    //     fclose(InputFile);
    // }
    // free(inputFileName);
    free(outputFileName);
}

void SaveRecords(FILE* OutputFile, char* outputFileName, 
            PSTORAGE_DEVICE TempStorageDevice, 
            PSTORAGE_DEVICE FirstStorageDevice, 
            int nDebug)
{
    printf("Saving records\n");

    OutputFile = openFileWT(outputFileName);

    TempStorageDevice = FirstStorageDevice;
    // Ends when TempStorageDevice is NULL
    while (TempStorageDevice)
    {
        if (nDebug)
        {
            fprintf(OutputFile, 
            "Name '%10s' Me %lf, Next %lp Prev %lp\n",
            TempStorageDevice->byteCapacity,
            TempStorageDevice,
            TempStorageDevice->NextStorageDevice,
            TempStorageDevice->PrevStorageDevice);
        }
        else
        {
            char canOverwrite = TempStorageDevice->canBeOverwritten;
            fprintf(OutputFile, 
                "Capacity (byte) %ld Type %s Overwritten %3s "
                "Inventory Number %d \n", 
                TempStorageDevice->byteCapacity, 
                TempStorageDevice->storageType,
                canOverwrite ? "Yes" : "No",
                TempStorageDevice->InventoryNumber);
        }

        TempStorageDevice = TempStorageDevice->NextStorageDevice;
    }
    fclose(OutputFile);
}

void GiveHelp() 
{
    printf("\nHelp goes here\n");
}
