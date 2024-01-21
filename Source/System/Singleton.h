#pragma once

#include "RTEError.h"

namespace RTE {

	template <typename Type>

	/// Anything derived from this class will adhere to the Singleton pattern.
	/// To convert any class into a Singleton, do the three following steps:
	/// 1. Publicly derive your class MyClass from Singleton.
	/// 2. Make sure to instantiate MyClass once before using it.
	/// 3. Call MyClass::Instance() to use the MyClass object from anywhere.
	class Singleton {

	public:
		/// Destructor method used to clean up a Singleton object before deletion.
		~Singleton() = default;

		/// Returns the sole instance of this Singleton.
		/// @return A reference to the sole instance of this Singleton.
		inline static Type& Instance() { return *s_Instance; }

		/// Constructs this Singleton.
		inline static void Construct() { s_Instance = new Type(); }

	protected:
		/// Constructor method used to instantiate a Singleton object.
		Singleton() { RTEAssert(!s_Instance, "Trying to create a second instance of a Singleton!"); }

	private:
		static Type* s_Instance; //!< Pointer to instance of this singleton.

		// Disallow the use of some implicit methods.
		Singleton(const Singleton& reference) = delete;
		Singleton& operator=(const Singleton& rhs) = delete;
	};

	template <typename Type> Type* Singleton<Type>::s_Instance = nullptr;
} // namespace RTE
