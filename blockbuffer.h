//-------------------------------------------------------------------------------
// blockbuffer.h
// This class acts primarily as a buffer for sorting and saving the zip code
// maximums and minimums for each state. 
//-------------------------------------------------------------------------------
// Author: Lucas Hirt
//-------------------------------------------------------------------------------

#ifndef BLOCKBUFFER_H
#define BLOCKBUFFER_H
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>

class BlockBuffer{
    public:
        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Default constructor.
         * @pre   None
         * @post  An array of 50 State structs are dynamically allocated. *ptr is initalized to point to this
         *        array of structs. All numerical variables of the struct are initialized to 0 and every struct's 
         *        stateName corresponds with a single state from stateList[].
         */
        BlockBuffer();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Destructor.
         * @pre   *ptr points to the State struct array
         * @post  Dynamically allocated struct array is returned to memory
         */
        ~BlockBuffer();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Parses through the entire file and properly assigns the values to each member of
         *        the State struct.
         * @pre   File name has been acquired from user and the file is in a format such that the order goes:
         *        zipcode,placename,state,county,latitude,longitude
         * @post  All 50 members of the State struct array have values assigned for all members in the State 
         *        struct.
         */           
        void createTable(std::string);

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Readability function that outputs labels for each column of data output by the table.
         * @pre   None
         * @post  None
         */
        void outputHeader();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Outputs the state abbreviation and all 4 zipcodes for each member in the State struct array.
         * @pre   createTable() has been run.
         * @post  5 columns output to terminal. State abbreviation, northern most zip, southern most, eastern
         *        most and western most.
         */
        void outputTable();

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief Outputs a partial table such that it only shows information for the state of the users choosing
         * @pre   String containing abbrevation for the state in which the user wants information for
         * @post  If string is correct, information for state is output, otherwise error message is thrown.
         */ 
        void outputState(std::string xy);

        //---------------------------------------------------------------------------------------------------
        /**
         * @brief enumerated variables are used in the updateZipCode() function for readability, vaules correspond
         *        to the indexes of zipCode[] array in the State struct.
         */        
        enum coordinateList {maxLat,minLat,minLong,maxLong};
    
        //---------------------------------------------------------------------------------------------------
        /**
         * @brief enumarated strings are implemented throughout multiple functions of the class for purposes
         *        such as assignments for members of the State struct array and conditional comparisons 
         *        from reading of the input file.
         */   
        std::string stateList[50] = { "AK","AL","AR","AZ","CA","CO","CT","DE","FL","GA",
                                    "HI","IA","ID","IL","IN","KS","KY","LA","MA","MD",
                                    "ME","MI","MN","MO","MS","MT","NC","ND","NE","NH",
                                    "NJ","NM","NV","NY","OH","OK","OR","PA","RI","SC",
                                    "SD","TN","TX","UT","VA","VT","WA","WI","WV","WY"};
    private:
        //---------------------------------------------------------------------------------------------------
        /**
         * @brief This struct is created on the basis that each unique struct object may represent a single
         *        state/region with its own 3 character abbrevation, points of maximum/minimum longitude and 
         *        latitude and 4 zip codes that correspond with these maxima and minima.
         */
        struct State{

            /** @brief Stores the zip codes of the greatest latitude, smallest latitude, least longitude and
             *         greatest longitude, respectively. */
            int zipCode[4];

            /** @brief Stores the value of the greatest latitude of the State object. */
            float greatestLat;

            /** @brief Stores the value of the smallest latitude of the State object. */
            float leastLat;

            /** @brief Stores the value of the smallest longitude of the State object. */
            float leastLong;

            /** @brief Stores the value of the greatest longitude of the State object. */
            float greatestLong;

            /** @brief Stores the 2 letter abbreviation of the name of the State object. */
            char stateName[2];
    

            //---------------------------------------------------------------------------------------------------
            /**
             * @brief Compares the values of the members of the State object and updates them based on the
             *        input from the file.
             * @pre   createTable() is invoked and file name has been acquired.
             * @post  The State object will have greatestLat, leastLat, leastLong, greatestLong and all zipCode[]
             *        members properly updated.
             */
            void updateZipCode(int,float,float);

            //---------------------------------------------------------------------------------------------------
            /**
             * @brief Outputs the state abbreviation and all 4 zipcodes for a State struct object.
             * @pre   outputTable() or outputState(string) is invoked.
             * @post  None
             */
            void outputState();
        };

        /** @brief Points to the State struct array allocated by the Buffer constructor. Allows array to be accessed from
         *        other functions.*/
        State *ptr;
};

#endif