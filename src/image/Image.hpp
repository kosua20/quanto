#include "Common.hpp"

struct Image {
public:
	unsigned char* data = nullptr;
	size_t size = 0;
	unsigned int w = 0;
	unsigned int h = 0;

	bool load(const std::string& path);

	bool save(const std::string& path);
	
	void clean();

	void makeOpaque();

private:

	bool _stbiAllocated = false;
	
};

