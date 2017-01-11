/*
 * ZeroTier One - Network Virtualization Everywhere
 * Copyright (C) 2011-2016  ZeroTier, Inc.  https://www.zerotier.com/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ZT_SOFTWAREUPDATER_HPP
#define ZT_SOFTWAREUPDATER_HPP

#include <stdint.h>

#include <vector>
#include <map>
#include <string>

#include "../include/ZeroTierOne.h"
#include "../node/Identity.hpp"
#include "../node/Array.hpp"

#include "../ext/json/json.hpp"

/**
 * VERB_USER_MESSAGE type ID for software update messages
 */
#define ZT_SOFTWARE_UPDATE_USER_MESSAGE_TYPE 1000

/**
 * ZeroTier address of node that provides software updates
 */
#define ZT_SOFTWARE_UPDATE_SERVICE 0xc1243d3869ULL

/**
 * ZeroTier identity that must be used to sign software updates
 */
#define ZT_SOFTWARE_UPDATE_SIGNING_AUTHORITY ""

/**
 * Chunk size for in-band downloads (can be changed, designed to always fit in one UDP packet easily)
 */
#define ZT_SOFTWARE_UPDATE_CHUNK_SIZE 1380

/**
 * Sanity limit for the size of an update binary image
 */
#define ZT_SOFTWARE_UPDATE_MAX_SIZE (1024 * 1024 * 256)

/**
 * How often (ms) do we check?
 */
#define ZT_SOFTWARE_UPDATE_CHECK_PERIOD (60 * 60 * 1000)

#define ZT_SOFTWARE_UPDATE_JSON_VERSION_MAJOR "versionMajor"
#define ZT_SOFTWARE_UPDATE_JSON_VERSION_MINOR "versionMinor"
#define ZT_SOFTWARE_UPDATE_JSON_VERSION_REVISION "versionRev"
#define ZT_SOFTWARE_UPDATE_JSON_EXPECT_SIGNED_BY "expectedSigner"
#define ZT_SOFTWARE_UPDATE_JSON_PLATFORM "platform"
#define ZT_SOFTWARE_UPDATE_JSON_ARCHITECTURE "arch"
#define ZT_SOFTWARE_UPDATE_JSON_WORD_SIZE "wordSize"
#define ZT_SOFTWARE_UPDATE_JSON_VENDOR "vendor"
#define ZT_SOFTWARE_UPDATE_JSON_CHANNEL "channel"
#define ZT_SOFTWARE_UPDATE_JSON_UPDATE_SIGNED_BY "updateSigner"
#define ZT_SOFTWARE_UPDATE_JSON_UPDATE_SIGNATURE "updateSig"
#define ZT_SOFTWARE_UPDATE_JSON_UPDATE_HASH "updateHash"
#define ZT_SOFTWARE_UPDATE_JSON_UPDATE_SIZE "updateSize"
#define ZT_SOFTWARE_UPDATE_JSON_EXEC_ARGS "updateExecArgs"

namespace ZeroTier {

class Node;

/**
 * This class handles retrieving and executing updates, or serving them
 */
class SoftwareUpdater
{
public:
	/**
	 * Each message begins with an 8-bit message verb
	 */
	enum MessageVerb
	{
		/**
		 * Payload: JSON containing current system platform, version, etc.
		 */
		VERB_GET_LATEST = 1,

		/**
		 * Payload: JSON describing latest update for this target. (No response is sent if there is none.)
		 */
		VERB_LATEST = 2,

		/**
		 * Payload:
		 *   <[16] first 128 bits of hash of data object>
		 *   <[4] 32-bit index of chunk to get>
		 */
		VERB_GET_DATA = 3,

		/**
		 * Payload:
		 *   <[16] first 128 bits of hash of data object>
		 *   <[4] 32-bit index of chunk>
		 *   <[...] chunk data>
		 */
		VERB_DATA = 4
	};

	SoftwareUpdater(Node &node,const char *homePath,bool updateDistributor);
	~SoftwareUpdater();

	/**
	 * Handle a software update user message
	 *
	 * @param origin ZeroTier address of message origin
	 * @param data Message payload
	 * @param len Length of message
	 */
	void handleSoftwareUpdateUserMessage(uint64_t origin,const void *data,unsigned int len);

	/**
	 * Check for updates and do other update-related housekeeping
	 *
	 * It should be called about every 10 seconds.
	 *
	 * @return Null JSON object or update information if there is an update downloaded and ready
	 */
	nlohmann::json check();

	/**
	 * Apply any ready update now
	 *
	 * Depending on the platform this function may never return and may forcibly
	 * exit the process. It does nothing if no update is ready.
	 */
	void apply();

private:
	Node &_node;
	uint64_t _lastCheckTime;
	std::string _homePath;

	// Offered software updates if we are an update host (we have update-dist.d and update hosting is enabled)
	struct _D
	{
		nlohmann::json meta;
		std::string bin;
	};
	std::map< Array<uint8_t,16>,_D > _dist; // key is first 16 bytes of hash

	nlohmann::json _latestMeta;
	std::string _latestBin;
	Array<uint8_t,16> _latestBinHashPrefix;
	unsigned long _latestBinLength;
	bool _latestBinValid;
};

} // namespace ZeroTier

#endif
