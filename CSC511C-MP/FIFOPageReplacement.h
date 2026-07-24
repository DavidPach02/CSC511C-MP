#pragma once

#include <cstddef>
#include <queue>

// FIFO page replacement: evict the frame that was loaded earliest.
// Mirrors the course model: memoryQueue tracks load order; SelectVictimFrame returns the oldest.
class FIFOPageReplacement {
public:
	explicit FIFOPageReplacement(size_t frameCount = 0);

	void Reset(size_t frameCount);

	// Called when a virtual page is mapped into a physical frame.
	void RecordFrameLoad(size_t frameIndex);

	// Returns the oldest loaded frame index (front of the FIFO queue).
	size_t SelectVictimFrame() const;

	// Removes a frame from the replacement queue (after eviction or process cleanup).
	void RecordFrameEviction(size_t frameIndex);

	size_t GetFrameCount() const;

private:
	size_t frameCount;
	std::queue<size_t> loadedFrameQueue;
};
