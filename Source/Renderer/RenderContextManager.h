#ifndef _RTERENDERCONTEXTMANAGER_H_
#define _RTERENDERCONTEXTMANAGER_H_
#include "Image2D.h"
namespace RTE {
	template <typename ContextType, typename RendererType, typename TextureType>
	class RenderContextManager<RendererType> {
	public:
		RenderContextManager();
		virtual ~RenderContextManager();

		void IntializeContext(SDL_Window* window);
		void DestroyContext(SDL_Window* window);
		ContextType GetContext(SDL_Window* window);

		void ApplyDefaults();

		std::shared_ptr<RendererType> CreateRenderer();
		std::shared_ptr<RendererType> GetRenderer(int id);

		std::shared_ptr<Image2D> CreateTexture();
		TextureType GetTexture(int id);
	private:
		std::unordered_map<SDL_Window*, ContextType> m_Context;
		static std::vector<RendererType*> m_Renderers;
		static int m_RendererCount;
		static std::vector<TextureType> m_Textures;
		static int m_TexureCount;
	 };
}

#endif