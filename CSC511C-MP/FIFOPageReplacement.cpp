#include "FIFOPageReplacement.h"

#include <queue>
#include <stdexcept>

FIFOPageReplacement::FIFOPageReplacement(size_t frameCount)
	: frameCount(frameCount) {
}

void FIFOPageReplacement::Reset(size_t newFrameCount) {
	frameCount = newFrameCount;
	while (!loadedFrameQueue.empty()) {
		loadedFrameQueue.pop();
	}
}

void FIFOPageReplacement::RecordFrameLoad(size_t frameIndex) {
	if (frameIndex >= frameCount) {
		return;
	}
	loadedFrameQueue.push(frameIndex);
}

size_t FIFOPageReplacement::SelectVictimFrame() const {
	if (loadedFrameQueue.empty()) {
		throw std::runtime_error("FIFO page replacement: no loaded frames to evict.");
	}
	return loadedFrameQueue.front();
}

void FIFOPageReplacement::RecordFrameEviction(size_t frameIndex) {
	std::queue<size_t> rebuiltQueue;
	while (!loadedFrameQueue.empty()) {
		const size_t queuedFrame = loadedFrameQueue.front();
		loadedFrameQueue.pop();
		if (queuedFrame != frameIndex) {
			rebuiltQueue.push(queuedFrame);
		}
	}
	loadedFrameQueue = std::move(rebuiltQueue);
}

size_t FIFOPageReplacement::GetFrameCount() const {
	return frameCount;
}
