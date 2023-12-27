#ifndef _RTETEXTURE2D_
#define _RTETEXTURE2D_
namespace RTE {
	class Texture2D {
	public:
		~Texture2D() = default;

		void GetWidth();
		void GetHeight();

		void* GetPixels();
		unsigned int GetPixel();

		unsigned int GetTextureID();

	private:
		int m_Width{0};
		int m_Height{0};
		int m_BitDepth{32};

		void* m_Pixels{nullptr};

		unsigned int m_TextureID{-1};
		Texture2D();
	};
}
#endif