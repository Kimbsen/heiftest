#include "hevcimagefilereader.hpp"
#include "log.hpp"
#include <iostream>
#include <fstream>
#include <cassert>


#define CHECK_FILE_FEATURE(X) do { \
        if(properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::X)){ fprintf(stdout, "  %-15s\tYES\n",#X); } else {  fprintf(stdout, "  %-15s\tNO\n",#X); } \
    } while(0)

struct Blob {
	uint32_t idx;
	std::vector<uint8_t> data;
};

struct FileData {
    uint32_t width;
    uint32_t height;
    uint8_t rows;
    uint8_t cols;    
    uint16_t rotation;
    std::vector<Blob> tiles;
};

void print_filedata(const FileData &filedata){
	fprintf(stdout, "\nFile information:\n");
	fprintf(stdout, " width      :%d\n",filedata.width);
	fprintf(stdout, " height     :%d\n",filedata.height);
	fprintf(stdout, " rows       :%d\n",filedata.rows);
	fprintf(stdout, " cols       :%d\n",filedata.cols);
	fprintf(stdout, " rotation   :%d\n",filedata.rotation);
	fprintf(stdout, " tiles      :%lu\n",filedata.tiles.size());
}

FileData LoadData(HevcImageFileReader &reader, uint32_t contextId) {
    ImageFileReaderInterface::GridItem gridItem;
    ImageFileReaderInterface::IdVector gridItemIds;    
    FileData filedata;

    //get all grid items
    reader.getItemListByType(contextId, "grid", gridItemIds);
    gridItem = reader.getItemGrid(contextId, gridItemIds.at(0));
    filedata.width = gridItem.outputWidth;
    filedata.height = gridItem.outputHeight;
    filedata.rows = gridItem.rowsMinusOne+1;
    filedata.cols = gridItem.columnsMinusOne+1;

    const uint32_t itemId = gridItemIds.at(0);
    const auto itemProperties =  reader.getItemProperties(contextId, itemId);
    for (const auto& property : itemProperties) {
        if (property.type == ImageFileReaderInterface::ItemPropertyType::IROT) {
            filedata.rotation = reader.getPropertyIrot(contextId, property.index).rotation;
        }
    }
    ImageFileReaderInterface::IdVector masterItemIds;    
    reader.getItemListByType(contextId, "master", masterItemIds);
    for(auto id : masterItemIds){
    	filedata.tiles.push_back(Blob{id});
    }

    for (auto & tile : filedata.tiles) {
        reader.getItemDataWithDecoderParameters(contextId, tile.idx, tile.data);
        //std::cout << "getTileBlob idx: " << tile.idx << " Size: " << tile.data.size() << "bytes" << std::endl;
    } 
    return filedata;
}

void WriteData(FileData data){
	for(auto tile : data.tiles){
		char buff[100];
        snprintf(buff, sizeof(buff), "%d.tile", tile.idx);
        std::string filename = buff;   
    	std::ofstream ofile(filename);
    	ofile.write((char*)&tile.data[0],tile.data.size());
    	ofile.close();
	}
}

int main(int argc, char *argv[]) {
    Log::setLevel(Log::LogLevel::INFO);

    HevcImageFileReader reader;
    fprintf(stdout, "Reading %s\n", argv[argc-1]);
    reader.initialize(argv[argc-1]);

	const auto& properties = reader.getFileProperties();
	assert(properties.fileFeature.hasFeature(ImageFileReaderInterface::FileFeature::HasRootLevelMetaBox));
	const uint32_t contextId = properties.rootLevelMetaBoxProperties.contextId;
	fprintf(stdout, "\nGot contextId:%d\n", contextId);

    fprintf(stdout,"properties of file:\n");
    CHECK_FILE_FEATURE(HasSingleImage);
    CHECK_FILE_FEATURE(HasImageCollection);
    CHECK_FILE_FEATURE(HasImageSequence);
    CHECK_FILE_FEATURE(HasCoverImage);
    CHECK_FILE_FEATURE(HasOtherTimedMedia);
    CHECK_FILE_FEATURE(HasRootLevelMetaBox);
    CHECK_FILE_FEATURE(HasMoovLevelMetaBox);
    CHECK_FILE_FEATURE(HasAlternateTracks);


    auto data = LoadData(reader,contextId);
    print_filedata(data);
    WriteData(data);

}