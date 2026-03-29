#include "BresenhamLine.h"

namespace RTE {

size_t TraverseBresenhamLine(int x0, int y0, int x1, int y1, int skip, std::function<bool(int& x, int& y, int domStep)> callback) {
	int intPos[2] = {x0, y0};
	int delta[2];
	delta[0] = x1 - x0;
	delta[1] = y1 - y0;

	if (delta[0] == 0 && delta[1] == 0) {
		return 0;
	}

	int increment[2] = {1, 1};
	if (delta[0] < 0) {
		increment[0] = -1;
		delta[0] = -delta[0];
	}
	if (delta[1] < 0) {
		increment[1] = -1;
		delta[1] = -delta[1];
	}

	int delta2[2] = {delta[0] << 1, delta[1] << 1};

	int dom = 0, sub = 1;
	if (delta[0] > delta[1]) {
		dom = 0;
		sub = 1;
	} else {
		dom = 1;
		sub = 0;
	}

	int error = delta2[sub] - delta[dom];
	int skipped = 0;
	size_t visitCount = 0;

	for (int domSteps = 0; domSteps < delta[dom]; ++domSteps) {
		intPos[dom] += increment[dom];
		if (error >= 0) {
			intPos[sub] += increment[sub];
			error -= delta2[dom];
		}
		error += delta2[sub];

		if (++skipped > skip || domSteps + 1 == delta[dom]) {
			if (callback(intPos[0], intPos[1], domSteps)) {
				++visitCount;
			} else {
				break;
			}
			skipped = 0;
		}
	}

	return visitCount;
}

}
