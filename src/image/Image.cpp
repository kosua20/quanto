#include "Image.hpp"
#include "system/TextUtilities.hpp"
#include "system/System.hpp"

#include "lodepng/lodepng.h"
#include "lodepng/lodepng_util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"


bool Image::load(const std::string& path){
	clean();
	
	// Check extension.
	const std::string::size_type pos = path.find_last_of('.');
	std::string extension;
	if(pos != std::string::npos) {
		extension = path.substr(pos);
	}

	// Load image.

	// For PNG, use lodepng so that we can apply the profile properly.
	if(extension == ".png"){
		_stbiAllocated = false;

		LodePNGState state;
		lodepng_state_init(&state);
		state.info_raw.colortype = LCT_RGBA;
		state.info_raw.bitdepth = 8;

		unsigned char* fileData = 0;
		size_t fileSize = 0;

		unsigned int error = lodepng_load_file(&fileData, &fileSize, path.c_str());
		if(error != 0){
			lodepng_state_cleanup(&state);
			return false;
		}

		unsigned char* tmpData = 0;
		error = lodepng_decode(&tmpData, &w, &h, &state, fileData, fileSize);
		if(error != 0){
			free(fileData);
			lodepng_state_cleanup(&state);
			return false;
		}

		const size_t bpp = lodepng_get_bpp(&state.info_raw);
		const size_t finalSize = (w * h * bpp + 7) / 8;
		data = (unsigned char*)malloc(finalSize);

		error = lodepng::convertToSrgb(data, tmpData, w, h, &state);

		// Cleanup.
		free(tmpData);
		free(fileData);
		lodepng_state_cleanup(&state);

		if(error != 0){
			free(data);
			return false;
		}

		size = fileSize;
		return true;
	}

	// For other formats use stb_image.
	_stbiAllocated = true;
	int wi = 0;
	int hi = 0;
	int n;
	data = nullptr;
	data = stbi_load(path.c_str(), &wi, &hi, &n, 4);
	if(data == nullptr || wi == 0 || hi == 0){
		return false;
	}

	size = System::getFileSize(path);

	w = (unsigned int)wi;
	h = (unsigned int)hi;
	return true;
}

bool Image::save(const std::string& path){
	return System::writeDataToFile(data, size, path);
}


void Image::clean(){
	if(data != nullptr){
		if(_stbiAllocated){
			stbi_image_free(data);
		} else {
			free(data);
		}
		data = nullptr;
	}
	w = 0;
	h = 0;
	size = 0;
}

void Image::makeOpaque(){
	if(data == nullptr ){
		return;
	}

	for(unsigned int y = 0; y < h; ++y){
		for(unsigned int x = 0; x < w; ++x){
			const size_t px = y * w + x;
			data[4 * px + 3] = 255;
		}
	}
}
