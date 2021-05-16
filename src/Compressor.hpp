#include "Common.hpp"
#include "image/Image.hpp"

enum class CompressorMode : int {
	   IMAGEQUANT = 0,
	   PNGNQ = 1,
	   POSTERIZER = 2,
	   COUNT = 3
   };

struct Settings {

	CompressorMode mode = CompressorMode::IMAGEQUANT;
	unsigned int colorCount = 255;
	bool dither = false;

	unsigned int clampedColorCount(unsigned int mini, unsigned int maxi) const {
		return std::min(std::max(mini, colorCount), maxi);
	}
};

namespace Compressor {

bool ImageQuant(const Image& inImage, const Settings& settings, Image& outImage);

bool PngNeuQuant(const Image& inImage, const Settings& settings, Image& outImage);

bool Posterizer(const Image& inImage, const Settings& settings, Image& outImage);

bool compress(const Image& inImage, const Settings& settings, Image& outImage);

}
