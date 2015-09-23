#pragma once
#include <atomic>
#include <ostream>

struct c_statistics {
  private:
	std::atomic<uint32_t> receivedMessages;
	std::atomic<uint32_t> sentMessages;

	std::atomic<uint64_t> receivedBytes;
	std::atomic<uint64_t> sentBytes;
  public:
	c_statistics();
	c_statistics(const c_statistics& other);

	uint32_t GetReceivedMessages() const;

	uint64_t GetReceivedBytes() const;

	uint32_t GetSentMessages() const;

	uint64_t GetSentBytes() const;

	void RegisterSentMessage(int32_t size);

	void RegisterReceivedMessage(int32_t size);

};

std::ostream& operator<<(std::ostream& os, const c_statistics& obj);

