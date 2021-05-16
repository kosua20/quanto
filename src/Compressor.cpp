#include "Compressor.hpp"

#include <imagequant/libimagequant.h>

extern "C" {
#include <posterizer/posterizer.h>
#include <pngnq/neuquant32.h>
#include <pngnq/pngnq.h>
}

#include "image/lodepng/lodepng.h"

#include <cmath>
#include <vector>
#include <array>

// Quantizer using libimagequant.
bool Compressor::ImageQuant(const Image& inImage, const Settings& settings, Image& outImage) {
	outImage.w = inImage.w;
	outImage.h = inImage.h;
	outImage.data = nullptr;
	outImage.size = 0;

	const int colorCountBounded = (int)settings.clampedColorCount(2, 256);

	// Settings.
	liq_attr* attributes = liq_attr_create();
	liq_set_max_colors(attributes, colorCountBounded);
	// Initial quantization.
	liq_image* image = liq_image_create_rgba(attributes, inImage.data, (int)inImage.w, (int)inImage.h, 0);
	liq_result* output = nullptr;
	liq_error res = liq_image_quantize(image, attributes, &output);

	if(res != LIQ_OK || output == nullptr){
		if(output){
			free(output);
		}
		liq_image_destroy(image);
		liq_attr_destroy(attributes);
		return false;
	}

	// Apply quantization to image (with dithering).
	const size_t pixelCount = size_t(inImage.w) * size_t(inImage.h);
	std::vector<unsigned char> buffer(pixelCount);
	liq_set_dithering_level(output, settings.dither ? 1.0f : 0.0f);
	liq_write_remapped_image(output, image, buffer.data(), pixelCount);
	// Get the palette.
	const liq_palette* palette = liq_get_palette(output);

	// Write PNG header.
	LodePNGState state;
	lodepng_state_init(&state);
	state.info_raw.colortype = LCT_PALETTE;
	state.info_raw.bitdepth = 8;
	state.info_png.color.colortype = LCT_PALETTE;
	state.info_png.color.bitdepth = 8;

	// Write PNG palette.
	for(unsigned int i = 0; i < palette->count; ++i){
		const liq_color& color = palette->entries[i];
		lodepng_palette_add(&(state.info_png.color), color.r, color.g, color.b, color.a);
		lodepng_palette_add(&(state.info_raw), color.r, color.g, color.b, color.a);
	}

	// Finish writing PNG data.
	unsigned int res2 = lodepng_encode(&outImage.data, &outImage.size, buffer.data(), inImage.w, inImage.h, &state);

	// Bit of cleaning.
	liq_result_destroy(output);
	liq_image_destroy(image);
	liq_attr_destroy(attributes);
	lodepng_state_cleanup(&state);

	return res2 == 0;
}

// Quantizer using PngNeuQuant.
bool Compressor::PngNeuQuant(const Image& inImage, const Settings& settings, Image& outImage) {
	outImage.w = inImage.w;
	outImage.h = inImage.h;
	outImage.data = nullptr;
	outImage.size = 0;

	// Settings.
	const unsigned int colorCountBounded = settings.clampedColorCount(2, 256);

	unsigned int sampleFactor =(unsigned int)(std::floor(1.0 + double(inImage.w * inImage.h) / (512.0 * 512.0)));
	sampleFactor = std::min(sampleFactor, (unsigned int)10);

	// Init network, learn color palette.
	network_data* network = initnet(inImage.data, inImage.w * inImage.h * 4, colorCountBounded, 1.0);
	std::array<unsigned char, MAXNETSIZE*4> map;
	std::array<unsigned int, MAXNETSIZE> remap;

	learn(network, sampleFactor, 0);
	inxbuild(network);
	getcolormap(network, map.data());

	// Remap indexes so all tRNS chunks are together (see pngNQ code).
	unsigned int botIdx = 0;
	unsigned int topIdx = colorCountBounded - 1;
	for(unsigned int i = 0; i < colorCountBounded; ++i){
		if(map[4 * i + 3] == 255){
			remap[i] = topIdx;
			--topIdx;
		} else {
			remap[i] = botIdx;
			++botIdx;
		}
		remap[i] = i;
	}

	if(botIdx != topIdx + 1){
		return false;
	}

	// Apply palette to image data (and dither).
	std::vector<unsigned char> buffer(size_t(inImage.w) * size_t(inImage.h));
	if(settings.dither){
		remap_floyd(network, inImage.data, inImage.w, inImage.h, map.data(), remap.data(), buffer.data(), 1);
	} else {
		remap_simple(network, inImage.data, inImage.w, inImage.h, remap.data(), buffer.data());
	}


	// Write PNG header.
	LodePNGState state;
	lodepng_state_init(&state);
	state.info_raw.colortype = LCT_PALETTE;
	state.info_raw.bitdepth = 8;
	state.info_png.color.colortype = LCT_PALETTE;
	state.info_png.color.bitdepth = 8;

	// Write PNG palette.
	std::array<unsigned int, MAXNETSIZE> remapInv;
	for(unsigned int i = 0; i < colorCountBounded; ++i){
		remapInv[remap[i]] = i;
	}

	for(unsigned int i = 0; i < colorCountBounded; ++i){
		const unsigned int lid = remapInv[i];
		const unsigned char r = map[4 * lid + 0];
		const unsigned char g = map[4 * lid + 1];
		const unsigned char b = map[4 * lid + 2];
		const unsigned char a = map[4 * lid + 3];
		lodepng_palette_add(&(state.info_png.color), r, g, b, a);
		lodepng_palette_add(&(state.info_raw), r, g, b, a);
	}

	// Write PNG data.
	// Finish writing PNG data.
	unsigned int res2 = lodepng_encode(&outImage.data, &outImage.size, buffer.data(), inImage.w, inImage.h, &state);

	// Cleanup.
	lodepng_state_cleanup(&state);

	return res2 == 0;
}


// Quantizer using mediantcut-posterizer.
bool Compressor::Posterizer(const Image& inImage, const Settings& settings, Image& outImage){
	outImage.w = inImage.w;
	outImage.h = inImage.h;
	outImage.data = nullptr;
	outImage.size = 0;


	// Posterization.
	set_gamma(1.0);
	const unsigned int colorCountBounded = settings.clampedColorCount(2, 255);

	posterizer(inImage.data, inImage.w, inImage.h, colorCountBounded, settings.dither);

	// Write PNG data.
	unsigned int res2 = lodepng_encode32(&outImage.data, &outImage.size, inImage.data, inImage.w, inImage.h);
	return res2 == 0;
}

bool Compressor::compress(const Image& inImage, const Settings& settings, Image& outImage){
	bool success = false;
	switch (settings.mode) {
		case CompressorMode::IMAGEQUANT:
			success = ImageQuant(inImage, settings, outImage);
			break;
		case CompressorMode::PNGNQ:
			success = PngNeuQuant(inImage, settings, outImage);
			break;
		case CompressorMode::POSTERIZER:
			success = Posterizer(inImage, settings, outImage);
			break;
		default:
			break;
	}
	return success;
}

