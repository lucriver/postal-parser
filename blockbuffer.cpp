#include "blockbuffer.h"

BlockBuffer::BlockBuffer(){
    State *state_x = new State[50];
    ptr = state_x;
    for(int i=0;i<=49;i++){
        state_x[i].greatestLat = 0;
        state_x[i].leastLat = 0;
        state_x[i].leastLong = 0;
        state_x[i].greatestLong = 0;
        stateList[i].copy(state_x[i].stateName,2,0);
        for(int j=0;j<=3;j++){
            state_x[i].zipCode[j] = 0;
        }
    }
}

BlockBuffer::~BlockBuffer(){
    for(int i=0;i<=49;i++)
        delete[] ptr;
}

void BlockBuffer::createTable(std::string fileName){
    std::ifstream myFile;
    myFile.open(fileName);

    while(!(myFile.eof())){
        std::string inputBuffer;
        getline(myFile,inputBuffer,'\n');
        getline(myFile,inputBuffer,'\n');
        std::istringstream iss(inputBuffer);
        std::string field;
        getline(iss,field,':');
        getline(iss,field,'|');
        try{
            stoi(field);
        }
        catch(std::invalid_argument field){
            break;
        }
        int recordQuantity = stoi(field);
        for(int i=0;i<recordQuantity;i++){
            getline(iss,field,',');
            getline(iss,field,',');
            try{
                stoi(field);
            }
            catch(std::invalid_argument field){
                break;
            }
            int tempZipCode = stoi(field);
            getline(iss,field,',');
            getline(iss,field,',');
            for(int i=0;i<=49;i++){
                if(field == stateList[i]){
                    stateList[i].copy(ptr[i].stateName,2,0);
                    getline(iss,field,',');
                    getline(iss,field,',');
                    float tempLatit = stof(field);
                    getline(iss,field,'|');
                    float tempLongit = stof(field);
                    ptr[i].updateZipCode(tempZipCode,tempLatit,tempLongit);
                    break;
                }else if(field == "DC"){
                    stateList[46].copy(ptr[46].stateName,2,0);
                    getline(iss,field,',');
                    getline(iss,field,',');
                    float tempLatit = stof(field);
                    getline(iss,field,'|');
                    float tempLongit = stof(field);
                    ptr[46].updateZipCode(tempZipCode,tempLatit,tempLongit);
                    break;
                }else if(i == 49 && field != stateList[i]){
                    continue;
                }
            }
        }
    }
    myFile.close();
}

void BlockBuffer::outputHeader(){
    std::cout << std::setw(5) << std::left << "State"
              << std::setw(15) << std::right << "Northern-Most"
              << std::setw(15) << std::right << "Southern-Most"
              << std::setw(15) << std::right << "Eastern-Most"
              << std::setw(15) << std::right << "Western-Most" << std::endl;
}

void BlockBuffer::outputTable(){
    outputHeader();
    for(int i=0;i<=49;i++)
        ptr[i].outputState();   
}

void BlockBuffer::outputState(std::string state){
    for(int i=0;i<=49;i++){
        if(state == stateList[i]){
            outputHeader();
            ptr[i].outputState();
            return;
        }
    }
    std::cout << "ERROR: State could not be found." << std::endl;
}

void BlockBuffer::State::updateZipCode(int zip_code,float latit,float longit){
    if(greatestLat == 0)
        greatestLat = latit;
    if(leastLat == 0)
        leastLat = latit;
    if(leastLong == 0)
        leastLong = longit;
    if(greatestLong == 0)
        greatestLong = longit;

    if(greatestLat < latit){
        greatestLat = latit;
        zipCode[maxLat] = zip_code;
    }
    else if(latit < leastLat){
        leastLat = latit;
        zipCode[minLat] = zip_code;
    }
    if(longit < leastLong){
        leastLong = longit;
        zipCode[minLong] = zip_code;
    }
    else if(greatestLong < longit){
        greatestLong = longit;
        zipCode[maxLong] = zip_code; 
    }
}

void BlockBuffer::State::outputState(){
    std::cout << std::setw(5) << std::left << std::setw(0) << stateName[0] << stateName[1]
              << std::setw(17) << std::right << zipCode[maxLat] 
              << std::setw(16) << std::right << zipCode[minLat]
              << std::setw(15) << std::right << zipCode[minLong]
              << std::setw(15) << std::right << zipCode[maxLong] << std::endl;
}


