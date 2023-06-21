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

FILE *openFileR(char* fileName)
{
    FILE *file = fopen(fileName, "r");
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
void SaveRecordsLessCapacity(FILE* OutputFile, char* outputFileName, 
            PSTORAGE_DEVICE TempStorageDevice, 
            PSTORAGE_DEVICE FirstStorageDevice, 
            int nDebug, long int capacity);

void PrintRecord(char nDebug, PSTORAGE_DEVICE TempStorageDevice);
void SaveRecord(char nDebug, FILE* OutputFile, PSTORAGE_DEVICE TempStorageDevice);

int main(int argc, char* argv[]) {
    FILE *InputFile;
    FILE *OutputFile;
    PSTORAGE_DEVICE FirstStorageDevice = NULL;
    PSTORAGE_DEVICE LastStorageDevice = NULL;
    PSTORAGE_DEVICE StorageDevice = NULL;
    PSTORAGE_DEVICE TempStorageDevice = NULL;

    // Динамически, потому что передаем в функцию
    char* inputFileName; 
    char* outputFileName;

    // Локальный буфер, никуда не передаем
    char szBuffer[257];

    int nNotDone = TRUE;
    char nDebug = FALSE;
    char nNeedSaving = FALSE;
    char isStdin = FALSE;

    // Считываение входного и выходного потоков (названия файлов)
    // Из аргументов командной строки 
    if (argc != 3)
    {
        printf("ERROR: Output file not specified. \n");
        exit(4);
    }
    else
    {
        inputFileName = (char*) malloc(strlen ( argv[1] ) +1 );
        outputFileName = (char*) malloc(strlen ( argv[2] ) +1 );
        strcpy(inputFileName, argv[1]);
        strcpy(outputFileName, argv[2]);
    }

    // Проверка существования файла вывода
    OutputFile = openFileWT(outputFileName);
    fclose(OutputFile);

    // Выбор стандартного потока ввода или файла ввода
    if (strcmp(inputFileName, "stdin") == 0)
    {
        /*  Если прописать так, а вызывать 
            "fgets(szBuffer, sizeof(szBuffer), InputFile);"
            То все, что выводится в printf будет сразу подаваться в stdin
            И возникнет бесконечный цикл
        */

        // InputFile = stdin;
        isStdin = TRUE;
        printf("STDIN");
    }
    else
    {
        InputFile = openFileR(inputFileName);
    }
    
    if (isStdin) 
    {
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
    }

    // Основной цикл программы
    while (nNotDone) {
        if (isStdin)
        {
            printf("Enter command (A, D, X, Z, S, L)?");
            fgets(szBuffer, sizeof(szBuffer), stdin);
        }
        else
        {
            fgets(szBuffer, sizeof(szBuffer), InputFile);
        }
        
        switch(szBuffer[0])
        {
            case 'H': /* Помощь */
            case 'h':
            case '?':
                GiveHelp();
                break;

            case 'A': // Добавить запись
            case 'a':
                StorageDevice = (PSTORAGE_DEVICE)calloc(sizeof(STORAGE_DEVICE), INCREMENT_AMOUNT);
                if (isStdin)
                {
                    printf("Enter storage capacity in bytes ");
                    fgets(szBuffer, sizeof(szBuffer), stdin);
                }
                else
                {
                    fgets(szBuffer, sizeof(szBuffer), InputFile);
                }
                
                sscanf(szBuffer, "%ld", &StorageDevice->byteCapacity);

                if (StorageDevice->byteCapacity > 0)
                { /* Insert this record in the list, sorted by byteCapacity */
                    nNeedSaving = TRUE;
                    if (FirstStorageDevice == NULL) {
                        if (isStdin && nDebug)
                        {
                            printf("It is a first record \n");
                        }
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
                        if (nDebug && isStdin)
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
                                if (nDebug && isStdin)
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
                                    if (nDebug && isStdin)
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
                                    if (nDebug && isStdin)
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
                        if (isStdin)
                        {
                            printf("Enter storage type (ex. CD, HDD, DVD): ");
                            fgets(szBuffer, sizeof(szBuffer), stdin);
                        }
                        else
                        {
                            fgets(szBuffer, sizeof(szBuffer), InputFile);
                        }
                        szBuffer[sizeof(StorageDevice->storageType) - 1] = '\0';
                        strcpy(StorageDevice->storageType, szBuffer);

                        if (isStdin)
                        {
                            printf("Enter Inventory number: ");
                            fgets(szBuffer, sizeof(szBuffer), stdin);
                        }
                        else
                        {
                            fgets(szBuffer, sizeof(szBuffer), InputFile);
                        }
                        sscanf(szBuffer, "%d", &StorageDevice->InventoryNumber);

                        if (isStdin)
                        {
                            printf("Enter 1 if the device can be overwritten, 0 if can't: ");
                            fgets(szBuffer, sizeof(szBuffer), stdin);
                        }
                        else
                        {
                            fgets(szBuffer, sizeof(szBuffer), InputFile);
                        }
                        sscanf(szBuffer, "%c", &StorageDevice->canBeOverwritten);
                    }
                }
                else
                {
                    if (isStdin)
                    {
                        printf("\aSorry, name must not be blank!\n");
                    }
                }
                break;

            case 'Z': // Изменить Debug moode
            case 'z':
                nDebug = !nDebug;
                break;
            
            case 'D': // Напечатать все записи
            case 'd':
                TempStorageDevice = FirstStorageDevice;
                if (isStdin)
                {
                    printf("Display Storage Devices\n");
                    while (TempStorageDevice)
                    {
                        PrintRecord(nDebug, TempStorageDevice);
                        TempStorageDevice = TempStorageDevice->NextStorageDevice;
                    }
                }
                else
                {
                    SaveRecords(OutputFile, outputFileName, 
                        TempStorageDevice, FirstStorageDevice, nDebug);
                }
                break;

            case 'L': // Напечатать все записи с меньшим объемом данных, чем заданное число
            case 'l':
            {
                long int _capacity = 0;
                if (isStdin)
                {
                    printf("Enter desired capacity: \n");    
                    fgets(szBuffer, sizeof(szBuffer), stdin);
                }
                else
                {
                    fgets(szBuffer, sizeof(szBuffer), InputFile);
                }
                sscanf(szBuffer, "%ld", &_capacity);

                TempStorageDevice = FirstStorageDevice;

                if (isStdin)
                {
                    printf("Display Storage Devices that store less data than %ld\n", _capacity);
                    while (TempStorageDevice && TempStorageDevice->byteCapacity <= _capacity)
                    {
                        PrintRecord(nDebug, TempStorageDevice);
                        TempStorageDevice = TempStorageDevice->NextStorageDevice;
                    }
                }
                else
                {
                    SaveRecordsLessCapacity(OutputFile, outputFileName, 
                        TempStorageDevice, FirstStorageDevice, nDebug, _capacity);
                }
            }
                break;
            
            case 'X': // Выход без сохранения и записи
            case 'x':
                nNotDone = FALSE;
                szBuffer[0] = '\0';

                // SaveRecords(OutputFile, outputFileName, TempStorageDevice,
                //         FirstStorageDevice, nDebug);

                // Освобождение памяти
                TempStorageDevice = FirstStorageDevice;
                while (TempStorageDevice)
                {
                    PSTORAGE_DEVICE p_next = TempStorageDevice->NextStorageDevice;
                    free(TempStorageDevice);
                    TempStorageDevice = p_next;
                }
                break;

            case 'S': // Сохранить все записи
            case 's':
                if (nNeedSaving)
                {
                    SaveRecords(OutputFile, outputFileName, TempStorageDevice,
                            FirstStorageDevice, nDebug);
                    nNeedSaving = FALSE;
                }
                break;   
        }
    }

    if (strcpy(inputFileName, "stdin") != 0)
    {
        fclose(InputFile);
    }
    free(inputFileName);
    free(outputFileName);

    return 0;
}

void SaveRecords(FILE* OutputFile, char* outputFileName, 
            PSTORAGE_DEVICE TempStorageDevice, 
            PSTORAGE_DEVICE FirstStorageDevice, 
            int nDebug)
{
    OutputFile = openFileWT(outputFileName);

    TempStorageDevice = FirstStorageDevice;
    // Ends when TempStorageDevice is NULL
    while (TempStorageDevice)
    {
        SaveRecord(nDebug, OutputFile, TempStorageDevice);
        TempStorageDevice = TempStorageDevice->NextStorageDevice;
    }
    fclose(OutputFile);
}

void SaveRecordsLessCapacity(FILE* OutputFile, char* outputFileName, 
            PSTORAGE_DEVICE TempStorageDevice, 
            PSTORAGE_DEVICE FirstStorageDevice, 
            int nDebug, long int capacity)
{
    OutputFile = openFileWT(outputFileName);

    TempStorageDevice = FirstStorageDevice;
    // Ends when TempStorageDevice is NULL
    while (TempStorageDevice && TempStorageDevice->byteCapacity <= capacity)
    {
        SaveRecord(nDebug, OutputFile, TempStorageDevice);
        TempStorageDevice = TempStorageDevice->NextStorageDevice;
    }
    fclose(OutputFile);
}

void PrintRecord(char nDebug, PSTORAGE_DEVICE TempStorageDevice)
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
}

void SaveRecord(char nDebug, FILE* OutputFile, PSTORAGE_DEVICE TempStorageDevice)
{
    if (OutputFile == NULL)
    {
        return;
    }
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
}

void GiveHelp() 
{
    printf("\nHelp goes here\n");
}
