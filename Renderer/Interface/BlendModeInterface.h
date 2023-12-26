#ifndef _RTEBLENDMODEINTERFACE_
#define _RTEBLENDMODEINTERFACE_

namespace RTE {
	class BlendMode {
	public:
		constexpr BlendMode() = default;
		virtual ~BlendMode() = default;

		virtual void Enable() = 0;

		virtual bool operator==(const BlendMode &rhs) const = 0;
	};
} // namespace RTE

#endif