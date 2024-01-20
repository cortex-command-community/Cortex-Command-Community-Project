#ifndef _RTETEXTURE2D_
#define _RTETEXTURE2D_
namespace RTE {
	class Texture2D {
	public:
		Texture2D();
		virtual ~Texture2D() = default;

		virtual void GetWidth() = 0;
		virtual void GetHeight() = 0;
	};
}
#endif