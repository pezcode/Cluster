#pragma once

#include <bx/string.h>
#include <bigg.hpp>

class Cluster : public bigg::Application
{
public:

    Cluster();

    void initialize(int _argc, char* _argv[]) override;
    void onReset() override;
	void onChar(unsigned int codepoint) override;
    void update(float dt) override;

private:

	struct TextureBuffer
    {
        bgfx::TextureHandle handle;
        const char* name;
    };

	struct Config
    {
		bool showUI;
        bool showConfigWindow;
		bool showPerfOverlay;
        struct
        {
			bool fps;
			bool frameTime;
			bool gpuMemory;
        } overlays;
		bool showBuffers;
    };

	class UI
    {
    public:

        UI(Config& config, const TextureBuffer* buffers) : config(config), buffers(buffers) { }

		void initialize();
		void update(float time);
        void imageTooltip(ImTextureID tex, ImVec2 tex_size, float region_size);

		Config& config;
        const TextureBuffer* buffers;
    };

	Config config;
	UI ui;

    bgfx::ProgramHandle mProgram;
    bgfx::VertexBufferHandle mVbh;
    bgfx::IndexBufferHandle mIbh;

    TextureBuffer buffers[4];

    float mTime;

	float fps;
	float frameTime;
};
