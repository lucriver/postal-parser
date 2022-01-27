#include "blockheaderbuffer.h"

BlockHeaderBuffer::BlockHeaderBuffer(){
    std::string defaultBlockFilename = "blockfile.txt";
    std::string defaultIndexFilename = "indexfile.txt";
    blockFilename = defaultBlockFilename;
    indexFilename = defaultIndexFilename;
    fileStructureType       = "Blocked-sequence-set-with-comma-separated-fields";
    version                 = 1.0;
    headerRecordSizeBytes   = 64;
    sizeFormatType          = "ASCII";
    blockSize               = 512;
    minimumBlockQuantity    = 50;
    indexSchema             = "{(greatest-block-key)(RBN)}";
    recordQuantity          = 0;
    fieldsPerRecord         = 0;
    fieldIndicator          = "";
    primaryKey              = "";
    avail                   = 00000;
    filled                  = 00000;
    staleflag               = 0;
}

void BlockHeaderBuffer::getSourceFilename(std::string filename){
    std::fstream iFile(filename);
    if(iFile.fail()){
        std::cout << "Bad file name." << std::endl;
        exit(0);
    }
    sourceFilename = filename;
}

void BlockHeaderBuffer::parseSourceHeader(std::ifstream& inputFile,std::string array[]){
    for( int i = 0; i <= 4; i++ )
        getline(inputFile,array[i],',');
    getline(inputFile,array[5],'\n');

    std::string buffer;
    for( int i = 0; i <= 5; i++ )
        buffer = buffer + array[i] + ',';
        
    int bufferLength = buffer.length();
    for( int i = 0; i < bufferLength; i++ ){
        if(buffer[i] == '\n')
            buffer[i] = '-';
        if(buffer[i] == '"')
            buffer[i] = '-';
    }

    std::stringstream ss(buffer);
    for( int i = 0; i <= 5; i++ ){
        getline(ss,array[i],',');
        recordLineFieldId.push_back(array[i]);
    }
}

void BlockHeaderBuffer::generateHeader(){
    std::ifstream sourceFileInput;
    sourceFileInput.open(sourceFilename);

    const int FIELDCOUNT = 6;
    std::string field[FIELDCOUNT];
    parseSourceHeader(sourceFileInput,field);

    int recordCount = 0;
    std::string buffer;
    sourceFileInput.seekg(0);
    while(!(sourceFileInput.eof())){
        getline(sourceFileInput,buffer,'\n');
        recordCount++;
    }

    recordQuantity = recordCount;
    fieldsPerRecord = FIELDCOUNT;
    primaryKey = field[0];

    std::string file_structure_type    = "File-struct-type:" + fileStructureType;
    std::string v_number               = "Version:"          + std::to_string(version);
    std::string header_record_size_bytes = "Record-size(fixed):" + std::to_string(headerRecordSizeBytes);
    std::string size_format_type       = "Size-format:"      + sizeFormatType;
    std::string block_size             = "Block-size(fixed):"  + std::to_string(blockSize);
    std::string minimum_block_quantity = "Block-fill-minimum:" + std::to_string(minimumBlockQuantity) + '%';
    std::string index_filename         = "Index-filename:"   + indexFilename;
    std::string index_schema           = "Index-schema:"     + indexSchema;
    std::string record_quantity        = "Records:"          + std::to_string(recordQuantity);
    std::string fields_per_record      = "Fields/record:"    + std::to_string(fieldsPerRecord);
    for( int i = 0; i <= 5; i++ )
        fieldIndicator += field[i] + ":2|";
    std::string primary_key            = "Primary-key:"  + primaryKey;
    std::string avail_rbn              = "First-avail:"  + std::to_string(avail);
    std::string filled_rbn             = "First-filled:" + std::to_string(filled);
    std::string stale_flag             = "Stale-flag:"   + std::to_string(staleflag);

    blockFileHeader = file_structure_type + ',' + v_number + ',' + header_record_size_bytes + ',' + 
                      size_format_type + ',' + block_size + ',' + minimum_block_quantity + ',' + 
                      index_filename + ',' + index_schema + ',' + record_quantity + ',' + 
                      fields_per_record + ',' + fieldIndicator + ',' + primary_key + ',' + 
                      avail_rbn + ',' + filled_rbn + ',' + stale_flag + '\n';

}

void BlockHeaderBuffer::generateBlockFile() {
    std::ifstream sourceFileInput;
    std::fstream tempFile;
    sourceFileInput.open(sourceFilename);
    tempFile.open("temp.txt",std::fstream::in|std::fstream::out|std::fstream::trunc);

    std::string skipHeader;
    for(int i=0;i<=2;i++)
        getline(sourceFileInput,skipHeader,'\n');

    int blockRecordMaxQuantity = 6;
    int blockNumber = 0;
    std::string blockPrefixLetter = "B";
    while(!(sourceFileInput.eof())) { 
        int blockRecordQuantity = 0;
        std::string blockContents;
        std::string blockLabel = blockPrefixLetter + std::to_string(blockNumber) + ':';         
        for( int i = 0; i < blockRecordMaxQuantity; i++ ) {
            std::string buffer;
            getline(sourceFileInput,buffer,'\n');
            if(buffer == "")
                break;
            buffer = ',' + buffer;                                                              
            int bufferLength = buffer.length();        
            std::string record = std::to_string(bufferLength) + buffer + '|';                   
            blockContents = blockContents + record;
            blockRecordQuantity++;
        }
        blockLabel = blockLabel + std::to_string(blockRecordQuantity) + '|';
        std::string nextBlock = '>' + blockPrefixLetter + std::to_string(blockNumber + 1) + '\n';
        std::string block = blockLabel + blockContents + nextBlock;
        tempFile << block;
        blockNumber++;
    }
    sourceFileInput.close();

    std::ofstream blockFileWrite;
    blockFileWrite.open(blockFilename);
    blockFileWrite << blockFileHeader;
    tempFile.seekg(0);
    while(!(tempFile.eof())){
        std::string line;
        getline(tempFile,line,'\n');
        blockFileWrite << line << '\n';
    }
    blockFileWrite.close();
    tempFile.close();

    std::remove("temp.txt");
}

std::string BlockHeaderBuffer::blockFilenameString(){
    return blockFilename;
}

void BlockHeaderBuffer::createIndexFile(){
    std::ifstream blockFile;
    std::ofstream indexFile;
    blockFile.open(blockFilename);
    indexFile.open(indexFilename);
    indexFile << "{(highest key in block)(RBN)}" << std::endl;
    
    std::string buffer;
    while(blockFile.peek() != EOF){
        getline(blockFile,buffer,'\n');
        getline(blockFile,buffer,'B');
        getline(blockFile,buffer,':');
        try{
            stoi(buffer);
        }
        catch(std::invalid_argument buffer){
            continue;
        }
        int rbn = stoi(buffer);
        getline(blockFile,buffer,'|');
        int recordCount = stoi(buffer);
        for(int i=0;i<recordCount-1;i++){
            getline(blockFile,buffer,'|');
        }
        for(int i=0;i<=1;i++)
            getline(blockFile,buffer,',');
        for(int i=0;i<recordCount-1;i++)
            indexFile << '{' << '(' << buffer << ')' << '(' << rbn << ')' << '}' << ',';
        indexFile << '{' << '(' << buffer << ')' << '(' << rbn << ')' << '}' << '\n'; 
    }

    blockFile.close();
    indexFile.close();
}

void BlockHeaderBuffer::recordCommandLine(){
    std::vector<ZipCode> *indexBuffer = new std::vector<ZipCode>();
    std::string buffer;
    std::cout << "> ";  
    std::string inputString;
    std::getline(std::cin,inputString);

    // ---- Initializes dynamic ZipCode vector members ----
    std::stringstream ss(inputString);
    std::string tempZipCode;
    int zipCount = 0;    
    while(!ss.eof()){
        ss >> tempZipCode;
        zipCount++;
        int iterator = 2;  
        std::string realZipCode;
        while(tempZipCode[iterator] != ' '){
            realZipCode = realZipCode + tempZipCode[iterator];
            iterator++;
        }
        ZipCode tempIndexBuffer;
        tempIndexBuffer.primaryKey = stoi(realZipCode);
        indexBuffer->push_back(tempIndexBuffer);
    }
    for(int i=0;i<zipCount;i++){
        indexBuffer->at(i).rbn = -1;
        for(int p=0;p<128;p++)
            indexBuffer->at(i).line[p] = '\0';
    }
    // ---- Initializes dynamic ZipCode vector members ----

    // ---- Grabs plausible RBN's from index file ----
    std::ifstream iFile;
    iFile.open(indexFilename);
    for(int i=0;i<zipCount;i++){
        while(!(iFile.eof())){
            getline(iFile,buffer,'\n');
            getline(iFile,buffer,'(');
            getline(iFile,buffer,')'); 
            try{
                stoi(buffer);
            }catch(std::invalid_argument buffer){
                break;
            }
            int checker_zip = stoi(buffer);
            getline(iFile,buffer,'(');
            getline(iFile,buffer,')');
            try{
                stoi(buffer);
            }catch(std::invalid_argument buffer){
                break;
            }
            int checker_rbn = stoi(buffer);
            if(checker_zip > indexBuffer->at(i).primaryKey){
                indexBuffer->at(i).rbn = checker_rbn;
                break;
            }else if(checker_zip == indexBuffer->at(i).primaryKey){
                indexBuffer->at(i).rbn = checker_rbn;
                break;
            }
        }
    }
    iFile.close();
    // ---- Grabs plausible RBN's from index file ----

    // ---- Locates plausible RBN for primary key, assigns accordingly -----
    for(int i=0;i<zipCount;i++){
        iFile.open(blockFilename);
        while(!(iFile.eof())){
            getline(iFile,buffer,'\n');
            getline(iFile,buffer,'B');
            getline(iFile,buffer,':');
            try{
                stoi(buffer);
            }catch(std::invalid_argument buffer){
                iFile.close();
                break;
            }
            int r_b_n = stoi(buffer);
            if(r_b_n == indexBuffer->at(i).rbn){
                getline(iFile,buffer,'|');
                try{
                    stoi(buffer);
                }catch(std::invalid_argument buffer){
                    iFile.close();
                    break;
                }
                int record_count = stoi(buffer);
                getline(iFile,buffer,'>');
                std::stringstream ss(buffer);
                for(int p=0;p<record_count;p++){
                    std::string field;
                    getline(ss,field,',');
                    std::string length_string = field;
                    getline(ss,field,',');
                    std::string zip_code_string = field;
                    if(field == std::to_string(indexBuffer->at(i).primaryKey)){
                        std::string line;
                        getline(ss,line,'|');
                        int length = stoi(length_string);
                        strncpy(indexBuffer->at(i).line,line.c_str(),length);
                        break;
                    }else{
                        getline(ss,field,'|');
                    }
                }
            }
        }
    }
    iFile.close();
    // ---- Locates plausible RBN for primary key, assigns accordingly -----

    // ---- Check to see what members have been filled, set rbn accordingly -----
    for(int i=0;i<zipCount;i++){
        if(indexBuffer->at(i).line[0] == '\0')
            indexBuffer->at(i).rbn = -1;
    }
    // ---- Check to see what members have been filled, set rbn accordingly -----

    ptr = indexBuffer; // Assign class pointer for out-of-scope referencing of the vector.
}

void BlockHeaderBuffer::outputMemoryContents(){
    for(int i=0;i<ptr->size();i++){
        std::cout << "-----------------------------------------------------" << std::endl;
        if(ptr->at(i).rbn == -1){
            std::cout << "Zip code: " << ptr->at(i).primaryKey << " could not be found." << std::endl;
            continue;
        }
        int p = 0;
        int z = 1;
        std::cout << recordLineFieldId[0] << ": " << ptr->at(i).primaryKey << std::endl;
        std::cout << recordLineFieldId[z] << ": ";
            while(ptr->at(i).line[p] != '\0'){
                if(ptr->at(i).line[p] == ','){
                    std::cout << std::endl;
                    z++;
                    std::cout << recordLineFieldId[z] << ": ";
                    p++;
                }
                std::cout << ptr->at(i).line[p];
                p++;
            }
            std::cout << std::endl;
    }
    std::cout << "-----------------------------------------------------" << std::endl;
}

void BlockHeaderBuffer::writeMemoryFile(){
    std::string filename = "memorycontents";
    srand(time(NULL));
    int randomizer = rand() % 100 + 1;
    filename = filename + std::to_string(randomizer) + ".txt";
    std::cout << "Writing memory contents to file... " << std::endl;
    std::ofstream wFile;
    wFile.open(filename);
    for(int i=0;i<ptr->size();i++){
        wFile << "-----------------------------------------------------" << std::endl;
        if(ptr->at(i).rbn == -1){
            wFile << "Zip code: " << ptr->at(i).primaryKey << " could not be found." << std::endl;
            continue;
        }
        int p = 0;
        int z = 1;
        wFile << recordLineFieldId[0] << ": " << ptr->at(i).primaryKey << std::endl;
        wFile << recordLineFieldId[z] << ": ";
            while(ptr->at(i).line[p] != '\0'){
                if(ptr->at(i).line[p] == ','){
                    wFile << std::endl;
                    z++;
                    wFile << recordLineFieldId[z] << ": ";
                    p++;
                }
                wFile << ptr->at(i).line[p];
                p++;
            }
            wFile << std::endl;
    }
    wFile << "-----------------------------------------------------" << std::endl;
    wFile.close();
    std::cout << "Done. Filename: " << filename << std::endl;   
}

void BlockHeaderBuffer::outputFormattedHeader(){
    std::stringstream ss(fieldIndicator);
    std::string recordFields,field;
    for(int i=0;i<fieldsPerRecord;i++){
        getline(ss,field,':');
        recordFields = recordFields + std::to_string(i) + ':' + field + "   ";
        getline(ss,field,'|');
    }
    std::cout << "~~~~~~~~~~~~~~~Header~info~~~~~~~~~~~~~~~~~~\n";
    std::cout << "---------------Filenames---------------------\n"
              << "source file name: " << sourceFilename << "   "
              << "block file name: " << blockFilename << "   "
              << "index filename: " << indexFilename << std::endl
              << "---------------Type-info---------------------\n";
    std::cout << "File structure type: " << fileStructureType << "   "
              << "Version: " << version << "   "
              << "Block size: " << blockSize << std::endl
              << "---------------Record-Info-------------------\n";
    std::cout << "Fields per record: " << fieldsPerRecord << std::endl;
    std::cout << "Field index contents: " << recordFields << std::endl
              << "Record count: " << recordQuantity << std::endl
              << "---------------Block-Status------------------\n";
    std::cout << "First unfilled block: " << avail << "   "
              << "First filled block: " << filled <<std::endl;
    std::cout << "~~~~~~~~~~~~~~~Header~info~~~~~~~~~~~~~~~~~~\n";
}

void BlockHeaderBuffer::updateRbnLinks(){
    std::string buffer, field;
    std::vector<int> filledRbns(0);
    std::vector<int> unfilledRbns(0);

    std::ifstream iFile;
    iFile.open("blockfile.txt");
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        std::string rbn_number = field;
        getline(ss,field,'|');
        std::string record_count = field;
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        if(stoi(record_count) <= 3){
            unfilledRbns.push_back(stoi(rbn_number));
        }else{
            filledRbns.push_back(stoi(rbn_number));
        }
    }
    iFile.close();

    //------

    std::sort(filledRbns.begin(),filledRbns.end());
    std::sort(unfilledRbns.begin(),unfilledRbns.end());
    int size = unfilledRbns.size() + filledRbns.size();
    std::vector<int> linkedRbns(size);

    int max_filled = *std::max_element(filledRbns.begin(),filledRbns.end());
    int max_unfilled = *std::max_element(unfilledRbns.begin(),unfilledRbns.end());

    for(int i=0;i<linkedRbns.size();i++){
        if(i == max_filled){
            linkedRbns.at(i) = -9;
            continue;
        }
        if(i == max_unfilled){
            linkedRbns.at(i) = -1;
            continue;
        }
        if(std::binary_search(filledRbns.begin(),filledRbns.end(),i)){
            std::vector<int>::iterator it;
            it = std::find(filledRbns.begin(),filledRbns.end(),i);
            int index = it - filledRbns.begin();
            index = index + 1;
            linkedRbns.at(i) = filledRbns.at(index);
        }
        else if(!(std::binary_search(filledRbns.end(),filledRbns.end(),i))){
            std::vector<int>::iterator it;
            it = std::find(unfilledRbns.begin(),unfilledRbns.end(),i);
            int index = it - unfilledRbns.begin();
            index = index + 1;
            linkedRbns.at(i) = unfilledRbns.at(index);
        }

    }
    avail = unfilledRbns.at(0);
    filled = filledRbns.at(0);
    generateHeader();
    std::ofstream wFile;
    iFile.open("blockfile.txt");
    wFile.open("temp2.txt");
    wFile << blockFileHeader;
    int i = 0;
    while( iFile.peek() != EOF){
        if(i == linkedRbns.size())
            break;
        getline(iFile,buffer,'\n');
        getline(iFile,buffer,'>');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        int index = stoi(field);
        if(linkedRbns.at(index) == -9){
            wFile << buffer << ">B!\n";
            i++;
        }else if(linkedRbns.at(index) == -1){
            wFile << buffer << ">B+\n";
            i++;
        }else{
            std::string destination = std::to_string(linkedRbns.at(index));
            wFile << buffer << ">B" << destination << '\n';
            i++;
        }
    }
    iFile.close();
    wFile.close();
    std::remove("blockfile.txt");
    std::rename("temp2.txt","blockfile.txt");
}

void BlockHeaderBuffer::updateRbnLinksInsert(){
    std::string new_block_file = "sampleblockfileinsertionafter.txt";
    std::string new_index_file = "sampleindexfileinsertionafter.txt";
    std::string buffer, field;
    std::vector<int> filledRbns(0);
    std::vector<int> unfilledRbns(0);

    std::ifstream iFile;
    iFile.open(blockFilename);
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        std::string rbn_number = field;
        getline(ss,field,'|');
        std::string record_count = field;
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        if(stoi(record_count) <= 3){
            unfilledRbns.push_back(stoi(rbn_number));
        }else{
            filledRbns.push_back(stoi(rbn_number));
        }
    }
    iFile.close();

    //------

    std::sort(filledRbns.begin(),filledRbns.end());
    std::sort(unfilledRbns.begin(),unfilledRbns.end());
    int size = unfilledRbns.size() + filledRbns.size();
    std::vector<int> linkedRbns(size);

    int max_filled = *std::max_element(filledRbns.begin(),filledRbns.end());
    int max_unfilled = *std::max_element(unfilledRbns.begin(),unfilledRbns.end());

    for(int i=0;i<linkedRbns.size();i++){
        if(i == max_filled){
            linkedRbns.at(i) = -9;
            continue;
        }
        if(i == max_unfilled){
            linkedRbns.at(i) = -1;
            continue;
        }
        if(std::binary_search(filledRbns.begin(),filledRbns.end(),i)){
            std::vector<int>::iterator it;
            it = std::find(filledRbns.begin(),filledRbns.end(),i);
            int index = it - filledRbns.begin();
            index = index + 1;
            linkedRbns.at(i) = filledRbns.at(index);
        }
        else if(!(std::binary_search(filledRbns.end(),filledRbns.end(),i))){
            std::vector<int>::iterator it;
            it = std::find(unfilledRbns.begin(),unfilledRbns.end(),i);
            int index = it - unfilledRbns.begin();
            index = index + 1;
            linkedRbns.at(i) = unfilledRbns.at(index);
        }

    }
    avail = unfilledRbns.at(0);
    filled = filledRbns.at(0);

    iFile.open(blockFilename);
    getline(iFile,buffer,'\n');
    std::stringstream ss(buffer);
    std::string new_header_string;
    getline(ss,field,',');
    new_header_string = field;
    for(int i=0;i<11;i++){
        getline(ss,field,',');
        new_header_string = new_header_string + ',' + field;
    }
    new_header_string = new_header_string + ',' + "First-avail:" + std::to_string(avail) + ',' + "First-filled:" + std::to_string(filled) + ',' + "Stale-flag:0\n";
    iFile.close();
    blockFileHeader = new_header_string;
    std::ofstream wFile;
    iFile.open(blockFilename);
    wFile.open(new_block_file);
    wFile << blockFileHeader;
    int i = 0;
    while( iFile.peek() != EOF){
        if(i == linkedRbns.size())
            break;
        getline(iFile,buffer,'\n');
        getline(iFile,buffer,'>');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        int index = stoi(field);
        if(linkedRbns.at(index) == -9){
            wFile << buffer << ">B!\n";
            i++;
        }else if(linkedRbns.at(index) == -1){
            wFile << buffer << ">B+\n";
            i++;
        }else{
            std::string destination = std::to_string(linkedRbns.at(index));
            wFile << buffer << ">B" << destination << '\n';
            i++;
        }
    }
    iFile.close();
    wFile.close();
    blockFilename = new_block_file;
    indexFilename = new_index_file;
    createIndexFile();
}

void BlockHeaderBuffer::dumpMemory(){
    std::string buffer,field;
    std::ifstream iFile;
    iFile.open(blockFilename);
    getline(iFile,buffer,'\n');
    int blocks = 0;
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        blocks++;
    }
    blocks = blocks - 1;
    std::vector<std::string> block(blocks);
    iFile.close();
    iFile.open(blockFilename);
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        int index = stoi(field);
        block.at(index) = buffer;
    }
    iFile.close();

    std::cout << "First RBN filled: " << filled << std::endl;
    std::cout << "First RBN avail: " << avail << std::endl;
    std::cout << "! indicates no further filled blocks" << std::endl;
    std::cout << "+ indicates no further avail blocks" << std::endl;
    std::cout << "~~~~~~~Logically-Contiguous~~~~~~~~~~~~~" << std::endl;
    for(int i=0;i<block.size();i++){
        std::cout << 'B' + std::to_string(i) + ' ';
        std::stringstream ss(block.at(i));
        getline(ss,field,':');
        getline(ss,field,'|');
        int record_count = stoi(field);
        std::cout << record_count << '|';
        for(int p=0;p<record_count;p++){
            getline(ss,field,',');
            getline(ss,field,',');
            int zip = stoi(field);
            std::cout << zip << '|';
            getline(ss,field,'|');
        }
        std::cout << " >B";
        getline(ss,field,'>');
        getline(ss,field,'B');
        getline(ss,field,'\n');
        std::cout << field << std::endl;
    }

    int index = 0;
    iFile.open(blockFilename);
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        block.at(index) = buffer;
        index++;
    }
    iFile.close();

    std::cout << "~~~~~~~Physically-Contiguous~~~~~~~~~~~~~~" << std::endl;
    for(int i=0;i<block.size();i++){
        std::cout << 'B';
        std::stringstream ss(block.at(i));
        getline(ss,field,'B');
        getline(ss,field,':');
        std::cout << field << ' ';
        getline(ss,field,'|');
        int record_count = stoi(field);
        std::cout << record_count << '|';
        for(int p=0;p<record_count;p++){
            getline(ss,field,',');
            getline(ss,field,',');
            int zip = stoi(field);
            std::cout << zip << '|';
            getline(ss,field,'|');
        }
        std::cout << " >B";
        getline(ss,field,'>');
        getline(ss,field,'B');
        getline(ss,field,'\n');
        std::cout << field << std::endl;
    }
}

void BlockHeaderBuffer::recordInsertion(){
    std::cout << "---------INSERTION----------\n";
    BlockHeaderBuffer blockbuffer2;
    blockbuffer2.primaryKey = primaryKey;
    blockbuffer2.recordLineFieldId = recordLineFieldId;
    blockbuffer2.fieldIndicator = fieldIndicator;
    blockbuffer2.blockFilename = "sampleblockfileinsertionoriginal.txt";
    blockbuffer2.indexFilename = "sampleindexfileinsertionoriginal.txt";
    std::cout << "Sample blockfile name: " << blockbuffer2.blockFilename << std::endl;
    std::cout << "Sample block index filename: " << blockbuffer2.indexFilename << std::endl;
    
    std::ifstream iFile;
    iFile.open(blockbuffer2.blockFilename);
    std::string header_string;
    getline(iFile,header_string,'\n');
    blockbuffer2.blockFileHeader = header_string + '\n';
    iFile.close();
    blockbuffer2.createIndexFile();
    blockbuffer2.updateRbnLinksInsert();

    std::string zip_code_string;
    std::getline(std::cin,zip_code_string);
    while((zip_code_string.length() > 5) || (zip_code_string.length() == 0)){
        std::cout << "Please enter a zipcode: ";
        std::getline(std::cin,zip_code_string);
    }
    std::cout << "Please enter a city: ";
    std::string city;
    std::getline(std::cin,city);
    std::cout << "Please enter a state abbreviation. Ex.(MN): ";
    std::string state;
    std::getline(std::cin,state);
    std::cout << "Please enter a county: ";
    std::string county;
    std::getline(std::cin,county);
    std::cout << "Please enter a latitude cordinate: ";
    std::string latitude_string;
    std::getline(std::cin,latitude_string);
    std::cout << "Please enter a longitude cordinate: ";
    std::string longitude_string;
    std::getline(std::cin,longitude_string);

    std::string record = ',' + zip_code_string + ',' +  city + ',' + state + ',' + county + ',' + longitude_string + ',' + latitude_string;
    int length = record.length();
    std::string length_indicated_record = std::to_string(length) + record + '|';

    blockbuffer2.dumpMemory();

    iFile.open(blockbuffer2.indexFilename);
    std::string buffer;
    int zip_code = stoi(zip_code_string);
    int closest_zip = 99999;
    int closest_rbn = 99999;
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'(');
        getline(iFile,buffer,')');
        try{
            stoi(buffer);
        }catch(std::invalid_argument buffer){
            break;
        }
        int check_zip = stoi(buffer);
        if(zip_code < check_zip){
            if((std::abs(check_zip - zip_code)) < std::abs(check_zip - closest_zip)){
                closest_zip = check_zip;
                getline(iFile,buffer,'(');
                getline(iFile,buffer,')');
                closest_rbn = stoi(buffer);
            }
        }
        getline(iFile,buffer,'\n');
    }
    iFile.close();

    std::string field;
    std::ofstream wFile;
    iFile.open(blockbuffer2.blockFilename);
    wFile.open("temp.txt");
    getline(iFile,buffer,'\n');
    std::stringstream ss(buffer);
    std::string new_header_string;
    getline(ss,field,',');
    new_header_string = field + ',';
    for(int i=0;i<7;i++){
        getline(ss,field,',');
        new_header_string = new_header_string + ',' + field;
    }
    int pos = ss.tellp();
    getline(ss,field,':');
    getline(ss,field,',');
    int record_counter = stoi(field);
    ss.seekp(pos);
    new_header_string = new_header_string + "Records:" + std::to_string(record_counter + 1);
    getline(ss,field,',');
    for(int i=0;i<6;i++){
        getline(ss,field,',');
        new_header_string = new_header_string + ',' + field;
    }
    blockbuffer2.blockFileHeader = new_header_string;
    wFile << blockbuffer2.blockFileHeader << '\n';
    iFile.close();
    iFile.open(blockbuffer2.blockFilename);
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        int rbn_check = stoi(field);
        if(rbn_check == closest_rbn){
            ss.seekg(0);
            getline(ss,field,':');
            wFile << field << ':';
            getline(ss,field,'|');
            int count = stoi(field) + 1;
            wFile << count << '|';
            wFile << length_indicated_record;
            getline(ss,field,'\n');
            wFile << field << '\n';
        }
        else{
            wFile << buffer << '\n';
        }
    }
    iFile.close();
    wFile.close();

    std::remove("sampleblockfileinsertionafter.txt");
    std::rename("temp.txt","sampleblockfileinsertionafter.txt");
    blockbuffer2.createIndexFile();
    std::cout << std::endl;
    std::cout << "       |                   |" << std::endl;
    std::cout << "       V                   V" << std::endl;
    std::cout << std::endl;
    blockbuffer2.dumpMemory();
    std::cout << "Enter zip code to search for ";
    blockbuffer2.parseSampleFile();
    blockbuffer2.outputMemoryContents();
}
 
void BlockHeaderBuffer::parseSampleFile(){
    std::vector<ZipCode> *indexBuffer = new std::vector<ZipCode>();
    std::string buffer;
    std::cout << "> ";  
    std::string inputString;
    std::getline(std::cin,inputString);

    // ---- Initializes dynamic ZipCode vector members ----
    std::stringstream ss(inputString);
    std::string tempZipCode;
    int zipCount = 0;    
    while(!ss.eof()){
        ss >> tempZipCode;
        zipCount++;
        int iterator = 2;  
        std::string realZipCode;
        while(tempZipCode[iterator] != ' '){
            realZipCode = realZipCode + tempZipCode[iterator];
            iterator++;
        }
        ZipCode tempIndexBuffer;
        tempIndexBuffer.primaryKey = stoi(realZipCode);
        indexBuffer->push_back(tempIndexBuffer);
    }
    for(int i=0;i<zipCount;i++){
        indexBuffer->at(i).rbn = -1;
        for(int p=0;p<128;p++)
            indexBuffer->at(i).line[p] = '\0';
    }

    std::fstream iFile;
    std::string field;
    iFile.open(blockFilename);
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        getline(ss,field,':');
        getline(ss,field,'|');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        int record_count = stoi(field);
        for(int i=0;i<record_count-1;i++){
            getline(ss,field,',');
            getline(ss,field,',');
            for(int p=0;p<indexBuffer->size();p++){
                if(stoi(field) == indexBuffer->at(p).primaryKey){
                    indexBuffer->at(p).rbn = 1;
                    getline(ss,field,'|');
                    std::string record_info = field;
                    int length = record_info.length();
                    strncpy(indexBuffer->at(p).line,record_info.c_str(),length);
                    break;
                }else{
                    getline(ss,field,'|');
                }
            }
        }
    }
    iFile.close();
    ptr = indexBuffer;
}

void BlockHeaderBuffer::recordDeletion(){
    std::cout << "---------Deletion----------\n";
    BlockHeaderBuffer blockbuffer2;
    blockbuffer2.primaryKey = primaryKey;
    blockbuffer2.recordLineFieldId = recordLineFieldId;
    blockbuffer2.fieldIndicator = fieldIndicator;
    blockbuffer2.blockFilename = "sampleblockfiledeleteoriginal.txt";
    blockbuffer2.indexFilename = "sampleindexfiledeleteoriginal.txt";
    std::cout << "Sample blockfile name: " << blockbuffer2.blockFilename << std::endl;
    std::cout << "Sample block index filename: " << blockbuffer2.indexFilename << std::endl;

    std::string new_blockfile_name = "sampleblockfileafterdelete.txt";
    std::string new_index_file = "sampleindexfiledeleteafter.txt";
    std::ifstream iFile;
    iFile.open(blockbuffer2.blockFilename);
    std::string header_string;
    getline(iFile,header_string,'\n');
    blockbuffer2.blockFileHeader = header_string + '\n';
    iFile.close();
    blockbuffer2.createIndexFile();
    blockbuffer2.updateRbnLinksInsert();

    std::string zip_code_string;
    std::getline(std::cin,zip_code_string);
    while((zip_code_string.length() > 5) || (zip_code_string.length() == 0)){
        std::cout << "Please enter a zip code to remove: ";
        std::getline(std::cin,zip_code_string);
    }

    blockbuffer2.dumpMemory();

    int found = 0;
    int rbn;
    std::string buffer, field;
    iFile.open(blockbuffer2.blockFilename);
    getline(iFile,buffer,'\n');
    while(!(iFile.eof())){
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        getline(ss,field,'B');
        getline(ss,field,':');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        int check_rbn = stoi(field);
        getline(ss,field,'|');
        try{
            stoi(field);
        }catch(std::invalid_argument field){
            break;
        }
        int record_count = stoi(field);
        for(int i=0;i<record_count;i++){
            getline(ss,field,',');
            getline(ss,field,',');
            int zip_check = stoi(field);
            if(zip_check == stoi(zip_code_string)){
                rbn = check_rbn;
                found = 1;
                break;
            }else{
                getline(ss,field,'|');
            }
        }
    }
    iFile.close();

    std::cout << "RBN: " << rbn << std::endl;

    if(found){
        iFile.open(blockbuffer2.blockFilename);
        std::ofstream wFile;
        wFile.open(new_blockfile_name);
        getline(iFile,buffer,'\n');
        std::stringstream ss(buffer);
        std::string new_header_string;
        getline(ss,field,',');
        new_header_string = field + ',';
        for(int i=0;i<7;i++){
            getline(ss,field,',');
            new_header_string = new_header_string + ',' + field;
        }
        int pos = ss.tellp();
        getline(ss,field,':');
        getline(ss,field,',');
        int record_counter = stoi(field);
        ss.seekp(pos);
        new_header_string = new_header_string + "Records:" + std::to_string(record_counter - 1);
        getline(ss,field,',');
        for(int i=0;i<6;i++){
            getline(ss,field,',');
            new_header_string = new_header_string + ',' + field;
        }
        blockbuffer2.blockFileHeader = new_header_string;
        wFile << blockbuffer2.blockFileHeader << '\n';
        iFile.close();
        iFile.open(blockbuffer2.blockFilename);
        getline(iFile,buffer,'\n');
        while(!(iFile.eof())){
            getline(iFile,buffer,'\n');
            std::stringstream ss(buffer);
            getline(ss,field,'B');
            getline(ss,field,':');
            try{
                stoi(field);
            }catch(std::invalid_argument field){
                break;
            }
            if(rbn == stoi(field)){
                ss.seekg(0);
                getline(ss,field,':');
                wFile << field << ':';
                getline(ss,field,'|');
                int record_count = stoi(field);
                wFile << record_count - 1 << '|';
                for(int i=0;i<record_count;i++){
                    getline(ss,field,'|');
                    std::string entire_string = field;
                    std::stringstream ss2(field);
                    getline(ss2,field,',');
                    getline(ss2,field,',');
                    std::string zip = field;
                    try{
                        stoi(zip);
                    }catch(std::invalid_argument field){
                        break;
                    }
                    if(stoi(zip) == stoi(zip_code_string)){
                        continue;
                    }else{
                        wFile << entire_string << '|';
                        getline(ss,field,'|');
                        wFile << field << '|';
                    }
                }
                int pos = wFile.tellp();
                wFile.seekp(pos);
                wFile << '\n';
            }else{
                wFile << buffer << '\n';
            }
        }
        iFile.close();
        wFile.close();
    }else{
        std::cout << "Zip code not found.\n";
    }

    blockbuffer2.blockFilename = new_blockfile_name;
    blockbuffer2.indexFilename = new_index_file;
    blockbuffer2.createIndexFile();
 
    std::cout << std::endl;
    std::cout << "       |                   |" << std::endl;
    std::cout << "       V                   V" << std::endl;
    std::cout << std::endl;
    blockbuffer2.dumpMemory();
}