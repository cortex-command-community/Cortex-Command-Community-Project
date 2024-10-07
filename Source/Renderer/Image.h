namespace RTE {
	class Image {
	public:
		Image();
		~Image();

	private:
		class ImageImpl* impl;
	};
} // namespace RTE