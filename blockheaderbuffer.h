//-------------------------------------------------------------------------------
// blockheaderbuffer.h
// This class acts as a buffer for the block file. It acts as a buffer for all types
// of processes except sorting zip code minimum and maximums.
// Features:
//      Insertion of record.
//      Deletion of record.
//      Search for specific zip code information.
//      etc...
//-------------------------------------------------------------------------------
// Author: Lucas Hirt
//-------------------------------------------------------------------------------


#ifndef BLOCKHEADERBUFFFER_H
#define BLOCKHEADERBUFFER_H
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cstdio>
#include <array>
#include <ctime>
#include <time.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>

class BlockHeaderBuffer{
    public:
        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Default constructor, initializes all private members of the class.
         */
        BlockHeaderBuffer();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Error checked string ets passed in and is assigned to the private member sourceFileName;
         */
        void getSourceFilename(std::string);

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Parses the raw source file header information, being csv, rather than parsing block file.
         */
        void parseSourceHeader(std::ifstream&,std::string[]);

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Converts the raw csv header parsed and converts it to a proper block file formatted header with additional header variables.
         */
        void generateHeader();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Takes the converted header from the csv and the raw CSV and converts it to a proper block file. 
         */
        void generateBlockFile();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Returns the raw header information as a string. 
         */
        std::string headerString();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Returns the string of the block file name private member. 
         */
        std::string blockFilenameString();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Creates an index file based on the record file.
         * @pre   Record file with properly initialized header.
         * @post  Index file created with two columns, one for primary key, one for offset of original record.
         */
        void createIndexFile();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Acquires the zip code information to display from the user.
         * @pre   Index file created, record file initialized.
         * @post  Memory is written with number of objects being number of zipcodes entered, invalid or not.
         * Details for each zip code is also written to memory. This is done by allocating a dynamic
         * vecotr of ZipCode structs. This is called ptr. Vector is used for stability.
         */
        void recordCommandLine();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Outputs contents written to memory by recordCommandLine();
         * @pre   recordCommandLine() has been run.
         */
        void outputMemoryContents();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Writes contents from memory to an external file with random name.
         * @pre   recordCommandLine() has been run.
         * @post  File created with random name, contained in file is contents of dynamic pointer vector.
         */
        void writeMemoryFile();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Takes the current header values and outputs them to the terminal.
         * @pre   Constructor ran.
         */
        void outputFormattedHeader();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Goes into the block file and properly assigns the RBN points to ensure that filled RBNs do not point to unfilled RBNs and vice versa.
         */
        void updateRbnLinks();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Utilizes a sample blockfile of a smaller sample size than the original one. Demonstrates insertion of a zip code and its relevant effects on the file and index file.
         */
        void recordInsertion();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Goes into the block file and properly assigns the RBN points to ensure that filled RBNs do not point to unfilled RBNs and vice versa.
         * This time, in the context of the smaller samply file.
         */ 
        void updateRbnLinksInsert();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Dumps the RBNS in a block file formatted for readability. Will output two types of ordering, logically contiguous RBNS and physically contiguous RBNs.
         */
        void dumpMemory();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Allows user to retrieve a zip code they might have written to the sample file. Writes this information into memory.
         */
        void parseSampleFile();


        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Allows user to delete a zip code from the blockfile if it is contained in the block file.
         */
        void recordDeletion();
    private:
        //---------------------------------------------------------------------------------------------------
        /**
         * @brief This struct is created on the basis that each zipcode entered by the user will be written
         * to memory using a dynamic vector. Each object will contain the zipcode, a byte offset
         * for location in the record file, and a 128 character char array that represents the details of
         * the zip code. 
         */
        struct ZipCode{
            //---------------------------------------------------------------------------------------------------
            /**
             * @brief Zip code value. Used in index file and for location in record file.
             */
            int primaryKey;

            //---------------------------------------------------------------------------------------------------
            /**
             * @brief Used in index file. Is used as a value for location in the record file.
             */
            int rbn;

            //---------------------------------------------------------------------------------------------------
            /**
             * @brief Contains entire line from zip code line in record. Is used for storing details of zip code.
             */
            char line[128];
        };
        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Dynamically allocated vector pointer for each zip code we are saving information for.
         */
        std::vector<ZipCode> *ptr;

        std::vector<std::string> recordLineFieldId;

        /**
         * @brief Used to indicate what the original record file name is that information is being gathered from. 
         */
        std::string sourceFilename;

        /**
         * @brief Used to indicate the filename of the actual block file itself.
         */
        std::string blockFilename;

        /**
         * @brief Indicates the simple index file associated with the block file itself.
         */
        std::string indexFilename;

        /**
         * @brief Used to indicate what type of structure is being read. May hold pdf, CSV etc...
         */
        std::string fileStructureType;

        /**
         * @brief Inidcates which size format integers are to be read in. Ex. ASCII, binary, etc...
         */
        std::string sizeFormatType;

        /**
         * @brief Indicates how the index file is to be read, contained in the header of blockfile and index file itself.
         */
        std::string indexSchema;

        /**
         * @brief A single concatenated string representing which each field in a record represents, data-wise.
         */
        std::string fieldIndicator;

        /**
         * @brief Inidcates which field in a record is the primary key.
         */
        std::string primaryKey;

        /**
         * @brief A single concatenated string that is a raw string of the header in the blockfile.
         */
        std::string blockFileHeader;

        /**
         * @brief Used to indicate a header version. Version differences may indicate different protocol.
         */
        float version;

        /**
         * @brief Contains the maximum length of each line in a record file. Found in record header. 
         */
        int headerRecordSizeBytes;

       /**
         * @brief Indicates the maximum 
         */
        int blockSize;

       /**
         * @brief Indicates the minimum percentage block fill for a block to be considered filled.
         */
        int minimumBlockQuantity;

        /**
         * @brief Inidcates the number of unique records in the block file.
         */ 
        int recordQuantity;

        /**
         * @brief Inidcates the number of fields in each individual record. 
         */ 
        int fieldsPerRecord;

        /**
         * @brief Contains the RBN of the first available block file for filling.
         */ 
        int avail;

        /**
         * @brief Contains the RBN of the first block file that is filled.
         */ 
        int filled;
        int staleflag;
};


#endif