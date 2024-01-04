#ifndef _RTETEXTURE2D_
#define _RTETEXTURE2D_


namespace RTE {
	class Image2D {
	public:
		~Image2D() = default;

		void GetWidth();
		void GetHeight();

		void* GetPixels();
		unsigned int GetPixel(int x, int y);

		unsigned int GetTextureID();
	private:
		friend class RenderContextManager;
		int m_Width{0};
		int m_Height{0};
		int m_BitDepth{32};

		void* m_Pixels{nullptr};

		unsigned int m_TextureID{-1};
		Image2D();
	};
}
#endif