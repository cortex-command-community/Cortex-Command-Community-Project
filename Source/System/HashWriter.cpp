#include "HashWriter.h"
#include "Entity.h"

using namespace RTE;

void HashWriter::WriteIdentity() {
	// Two copies of the same preset are told apart only by what's written of them after this, whether either is the original preset or not.
	AddToHash(m_Hash, Hash(Current<Entity>().GetModuleAndPresetName()));
}

HashWriter& HashWriter::operator<<(const Serializable& object) {
	WriteObject(object, nullptr, false);
	ObjectEnd();
	return *this;
}

HashWriter& HashWriter::operator<<(const Serializable* object) {
	if (object) {
		*this << *object;
	} else {
		NoObject();
	}
	return *this;
}
