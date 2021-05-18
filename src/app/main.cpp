#include "core/Common.hpp"
#include "core/Strings.hpp"
#include "core/Compressor.hpp"


#include "core/system/Config.hpp"
#include "core/system/System.hpp"
#include "core/system/TextUtilities.hpp"

#include "libs/gl3w/gl3w.h"
#include <GLFW/glfw3.h>
#include <sr_gui/sr_gui.h>
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_glfw.h"
#include "libs/imgui/imgui_impl_opengl3.h"

GLFWwindow* createWindow(int w, int h) {

	// Initialize glfw, which will create and setup an OpenGL context.
	if(!glfwInit()) {
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(w, h, "Quantizer", NULL, NULL);

	if(!window) {
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);

	if(gl3wInit()) {
		return NULL;
	}
	if(!gl3wIsSupported(3, 2)) {
		return NULL;
	}

	glfwSwapInterval(1);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8,8);
	style.FramePadding = ImVec2(10,4);
	style.CellPadding = ImVec2(4,2);
	style.ItemSpacing = ImVec2(10,10);
	style.ItemInnerSpacing = ImVec2(4,4);
	style.GrabMinSize = 18;
	style.FrameBorderSize = 0;
	style.WindowBorderSize = 0;
	style.FrameRounding = 12;
	style.GrabRounding = 12;
	style.PopupBorderSize = 0;
	style.PopupRounding = 6;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_FrameBg]                = ImVec4(0.58f, 0.58f, 0.58f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.84f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.84f, 0.98f, 0.40f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.08f, 0.71f, 0.77f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.05f, 0.61f, 0.73f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.03f, 0.69f, 0.82f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.39f, 0.45f, 1.00f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.05f, 0.61f, 0.73f, 1.00f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.03f, 0.69f, 0.82f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.05f, 0.39f, 0.45f, 1.00f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.05f, 0.61f, 0.73f, 1.00f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.03f, 0.69f, 0.82f, 1.00f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.05f, 0.39f, 0.45f, 1.00f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.05f, 0.61f, 0.73f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.05f, 0.39f, 0.45f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.05f, 0.61f, 0.73f, 1.00f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.03f, 0.69f, 0.82f, 1.00f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.05f, 0.39f, 0.45f, 1.00f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.03f, 0.69f, 0.82f, 1.00f);
	colors[ImGuiCol_TabActive]              = ImVec4(0.05f, 0.61f, 0.73f, 1.00f);
	colors[ImGuiCol_TabUnfocused]           = ImVec4(0.02f, 0.16f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.04f, 0.26f, 0.31f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.04f, 0.26f, 0.31f, 1.00f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.05f, 0.61f, 0.73f, 1.00f);

	return window;
}

GLuint setupTexture(int w, int h) {
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureId;
}

void upload(Image& img, GLuint tex){
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.w, img.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &img.data[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int main(int, char** ){
	
	GLFWwindow* window = createWindow(800,600);

	if(!window){
		Log::Error() << "Unable to create window." << std::endl;
		return 1;
	}

	sr_gui_init();

	std::string inPath;
	std::string inName;
	Image refImg;
	Image outImg;
	GLuint refTexId = 0;
	GLuint outTexId = 0;

	Settings settings;
	bool noAlpha = false;
	bool showRef = true;

	int winW, winH;

	const int toolbarBaseWidth = 780;
	const int toolbarBaseHeight = 20+40;


	float pixelScale = 1.f;
	ImVec2 mouseShift(0.f, 0.f);
	ImVec2 mousePrev(0.f, 0.f);


	const unsigned int winFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar;

	while(!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		// Screen resolution.
		glfwGetWindowSize(window, &winW, &winH);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		bool dirty = false;
		bool loaded = false;

		const bool splitToolbar = winW < toolbarBaseWidth;
		const int toolbarFinalHeight = toolbarBaseHeight + (splitToolbar ? 40 : 0);

		ImGui::SetNextWindowPos( ImVec2(0.f, float(winH - toolbarFinalHeight)) );
		ImGui::SetNextWindowSize( ImVec2(float(winW), float(toolbarFinalHeight)) );

		if(ImGui::Begin("Options", nullptr, winFlags)){

			if(ImGui::Button("Load...")){
				char** outPaths = nullptr;
				int count = 0;

				if(sr_gui_ask_load_files("Select an image", "", "", &outPaths, &count) == SR_GUI_VALIDATED && count > 0){
					inPath = std::string(outPaths[0]);
					Image tmpImg;
					if(tmpImg.load(inPath)){
						refImg.clean();
						refImg = tmpImg;
						dirty = true;
						loaded = true;

						if(refTexId > 0){
							glDeleteTextures(1, &refTexId);
						}
						refTexId = setupTexture(refImg.w, refImg.h);
						upload(refImg, refTexId);

						// Prepare output.
						if(outTexId > 0){
							glDeleteTextures(1, &outTexId);
						}
						outTexId = setupTexture(refImg.w, refImg.h);
						inName = TextUtilities::fileName(inPath);
					} else {
						sr_gui_show_message("Quantizer", "Unable to load image", SR_GUI_MESSAGE_LEVEL_ERROR);
					}

					for(int i = 0; i < count; ++i){
						free(outPaths[i]);
					}
					free(outPaths);
				}
			}

			ImGui::SameLine();
			if(ImGui::Button("Save...")){
				char* outPathTmp = nullptr;
				if(sr_gui_ask_save_file("Save to file", "", "png", &outPathTmp) == SR_GUI_VALIDATED){
					std::string outPath( outPathTmp );
					free( outPathTmp );
					if( !TextUtilities::hasSuffix( outPath, ".png" ) ) {
						outPath.append( ".png" );
					}
					if(!outImg.save(std::string(outPathTmp))){
						sr_gui_show_message("Quantizer", "Unable to save image", SR_GUI_MESSAGE_LEVEL_ERROR);
					}
				}
			}
			ImGui::SameLine();

			ImGui::Button("Show original");
			showRef = ImGui::IsItemActive();

			ImGui::SameLine();

			ImGui::PushItemWidth(110);
			dirty = ImGui::Combo("Mode", reinterpret_cast<int*>(&settings.mode), "ImageQuant\0PngNeuQuant\0Posterizer\0") || dirty;

			if(!splitToolbar){
				ImGui::SameLine();
			}

			int colorCount = (int)settings.colorCount;
			if(ImGui::SliderInt("Colors", &colorCount, 2, 256, "%d", ImGuiSliderFlags_AlwaysClamp)){
				dirty = true;
				settings.colorCount = colorCount;
			}

			ImGui::SameLine();

			dirty = ImGui::Checkbox("Dither", &settings.dither) || dirty;
			ImGui::SameLine();

			dirty = ImGui::Checkbox("No alpha", &noAlpha) || dirty;

			ImGui::PopItemWidth();

			const int percentage = (int)std::round((1.f - float(outImg.size) / float(refImg.size)) * 100.0f);

			if(refImg.data == nullptr){
				ImGui::Text("No image loaded.");
			} else {
				ImGui::Text("%s (%ux%u) - %zu bytes (saved %d%% of %zu bytes)", inName.c_str(), refImg.w, refImg.h, outImg.size, percentage, refImg.size);
			}
			ImGui::SameLine();
			ImGui::TextDisabled( "(?)" );
			if( ImGui::IsItemHovered() ) {
				ImGui::SetTooltip( "Blablabla" );
			}

		}
		ImGui::End();

		// Perform compression and update preview texture.
		if(dirty && refImg.data){
			showRef = false;

			// Clone the ref image.
			Image tmpImg;
			tmpImg.w = refImg.w;
			tmpImg.h = refImg.h;
			tmpImg.size = refImg.size;
			tmpImg.data = (unsigned char*)malloc(refImg.w * refImg.h * 4);
			std::memcpy(tmpImg.data, refImg.data, refImg.w * refImg.h * 4);

			if(noAlpha){
				tmpImg.makeOpaque();
			}

			Image tmpOutImg;
			if(Compressor::compress(tmpImg, settings, tmpOutImg)){
				outImg.clean();
				outImg = tmpOutImg;
				// We need to decode the paletized image.
				Image dispImg;
				outImg.convert(dispImg);
				upload(dispImg, outTexId);
				dispImg.clean();

			} else {
				sr_gui_show_message("Quantizer", "Unable to quantize image", SR_GUI_MESSAGE_LEVEL_ERROR);
			}

			tmpImg.clean();
		}

		if(loaded){
			mouseShift = ImVec2(0.0f, 0.0f);
			mousePrev = ImVec2(0.0f, 0.0f);
		}

		// Display current texture.
		ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
		ImGui::SetNextWindowSize(ImVec2(float(winW), float(winH - toolbarFinalHeight)));
		if(ImGui::Begin("Image", nullptr, winFlags) && ( refImg.data != nullptr) ){


			const ImVec2 winSize = ImGui::GetContentRegionAvail();
			const ImVec2 imageSize(float(refImg.w), float(refImg.h));

			if(loaded){
				pixelScale = 1.5f * std::max( imageSize[0] / winSize[0], imageSize[1] / winSize[1]);
			}

			float screenRatio = std::max(winSize[1], 1.f) / std::max(winSize[0], 1.f);
			float imageRatio  = imageSize[1] / imageSize[0];
			float widthRatio  = winSize[0] / imageSize[0];

			ImVec2 uv0(-pixelScale - 2.0f * mouseShift[0], -pixelScale - 2.0f * mouseShift[1]);
			ImVec2 uv1( pixelScale - 2.0f * mouseShift[0],  pixelScale - 2.0f * mouseShift[1]);

			ImVec2 scales(imageRatio * widthRatio, screenRatio * widthRatio);
			uv0[0] = uv0[0] * scales[0] * 0.5f + 0.5f;
			uv0[1] = uv0[1] * scales[1] * 0.5f + 0.5f;
			uv1[0] = uv1[0] * scales[0] * 0.5f + 0.5f;
			uv1[1] = uv1[1] * scales[1] * 0.5f + 0.5f;


			const uintptr_t textureId = static_cast<uintptr_t>(showRef ? refTexId : outTexId);

			ImGui::Image(reinterpret_cast<void *>(textureId), winSize, uv0, uv1);

			if(ImGui::IsItemHovered()){
				pixelScale += 0.01f * ImGui::GetIO().MouseWheel;
				pixelScale = std::min(std::max(pixelScale, 0.001f), 1000.0f);

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					mousePrev = ImGui::GetMousePos();
				}
				if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
					const ImVec2 mouseNew = ImGui::GetMousePos();
					mouseShift[0] += pixelScale * (mouseNew[0] - mousePrev[0]) / float(winW);
					mouseShift[1] += pixelScale * (mouseNew[1] - mousePrev[1]) / float(winH);
					mousePrev = mouseNew;
				}
			}
		}
		ImGui::End();


		// Render the interface.
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);
	}

	// Cleanup.

	glDeleteTextures(1, &refTexId);
	glDeleteTextures(1, &outTexId);
	refImg.clean();
	outImg.clean();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	sr_gui_cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
