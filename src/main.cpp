#include "Common.hpp"
#include "Strings.hpp"
#include "Compressor.hpp"


#include "system/Config.hpp"
#include "system/System.hpp"
#include "system/TextUtilities.hpp"
#include "system/Terminal.hpp"

class QuantizerConfig : public Config {
public:
	
	explicit QuantizerConfig(const std::vector<std::string> & argv) : Config(argv) {
		
		// Process arguments.
		for(const auto & arg : arguments()) {
			// Config path.
			if(arg.key == "in" || arg.key == "i"){
				if(!arg.values.empty()){
					inPath = arg.values[0];
				}
			}
			if(arg.key == "out" || arg.key == "o"){
				if(!arg.values.empty()){
					outPath = arg.values[0];
				}
			}
			if(arg.key == "mode" || arg.key == "m") {
				if(!arg.values.empty()){
					const int rawMode = std::stoi(arg.values[0]);
					if(rawMode >= 0 && rawMode < int(CompressorMode::COUNT)){
						mode = CompressorMode(rawMode);
					}
				}
			}
			if(arg.key == "colors" || arg.key == "c") {
				if(!arg.values.empty()){
					const int rawCount = std::stoi(arg.values[0]);
					colorCount = (unsigned int)std::min(256, std::max(2, rawCount));
				}
			}
			if(arg.key == "dither" || arg.key == "d") {
				dither = true;
			}
			if(arg.key == "no-alpha" || arg.key == "na") {
				noAlpha = true;
			}
			if(arg.key == "force" || arg.key == "f") {
				inPlace = true;
			}
			if(arg.key == "version" || arg.key == "v") {
				version = true;
			}
			if(arg.key == "license") {
				license = true;
			}
			if(arg.key == "license-ext") {
				licenseExt = true;
			}
			if(arg.key == "quant") {
				bonus = true;
			}
			
		}

		registerArgument("in", "i", "Image to quantize", "path to file");
		registerArgument("out", "o", "Destination path. If empty and -f present, input will be overwritten", "path to file");
		registerArgument("force", "f", "Output in place");

		registerSection("Settings");
		registerArgument("mode", "m", "Compressor to use", "(0: ImageQuant, 1: PngNeuQuant, 2: Posterizer)");
		registerArgument("colors", "c", "number of colors in the palette", "in 2,256");
		registerArgument("dither", "d", "should dithering be applied");
		registerArgument("no-alpha", "na", "ignore alpha channel");

		registerSection("Infos");
		registerArgument("version", "v", "Displays the current Quantizer version.");
		registerArgument("license", "", "Display the license message.");
		registerArgument("license-ext", "", "Display the dependencies license messages.");
		
	}
	
	std::string inPath = "";
	std::string outPath = "";
	CompressorMode mode = CompressorMode::IMAGEQUANT;
	unsigned int colorCount = 256;
	bool dither = false;
	bool noAlpha = false;
	bool inPlace = false;

	// Messages.
	bool version = false;
	bool license = false;
	bool licenseExt = false;
	bool bonus = false;
};


int main(int argc, char** argv){
	
	QuantizerConfig config(std::vector<std::string>(argv, argv+argc));
	if(config.version){
		Log::Info() << versionMessage << std::endl;
		return 0;
	} else if(config.license){
		Log::Info() << licenseMessage << std::endl;
		return 0;
	} else if(config.licenseExt){
		Log::Info() << licenseExtMessage << std::endl;
		return 0;
	} else if(config.bonus){
		Log::Info() << bonusMessage << std::endl;
		return 0;
	} else if(config.showHelp(config.inPath.empty())){
		return 0;
	}


	Image image;
	if(!image.load(config.inPath)){
		Log::Error() << "Unable to load file at path \"" << config.inPath << "\"." << std::endl;
		return 1;
	}

	// Remove alpha channel if needed.
	if(config.noAlpha){
		image.makeOpaque();
	}

	// Quantize.
	const Settings settings = {config.mode, config.colorCount, config.dither};

	Image outImg;
	// Returns a compressed image containing the raw output PNG data and the file size.
	bool success = Compressor::compress(image, settings, outImg);
	
	// Cleanup the image now.
	image.clean();

	if(!success){
		return 2;
	}

	std::string outPath = config.outPath;
	if(outPath.empty()){
		if(config.inPlace){
			outPath = config.inPath;
		} else {
			outPath = config.inPath + "_" + System::timestamp() + ".png";
		}
	}
	// Save output.
	outImg.save(outPath);
	outImg.clean();
	return 0;
}