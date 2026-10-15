#pragma once

#include <cstddef>
#include <string>
#include "record.hpp"
#include "queue.hpp"

std::size_t find_first_prefix(Record* const* index, std::size_t n, const std::string& key_cp866_lower);

void enqueue_prefix_matches(Record* const* index,
                            std::size_t n,
                            std::size_t first,
                            const std::string& key_cp866_lower,
                            RecordQueue& q);
