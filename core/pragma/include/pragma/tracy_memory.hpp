// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <new>
#include <unordered_set>
#include <mutex>
#include <cstdio>
#include <cassert>

#define TRACY_CALLSTACK 12
#include <tracy/Tracy.hpp>


extern "C" {
	void *__libc_malloc(std::size_t size);
	void  __libc_free(void *ptr);
	void *__libc_calloc(std::size_t n, std::size_t size);
	void *__libc_realloc(void *ptr, std::size_t size);
}





static std::mutex g_track_mutex;
static std::unordered_set<void *> g_active_allocations;
static thread_local bool g_is_tracking = false;

inline void TrackAlloc(void *ptr, std::size_t size)
{
	if(!ptr || g_is_tracking)
		return;

	g_is_tracking = true; // Prevent recursion through unordered_set (g_active_allocations)
	/*{
		std::lock_guard<std::mutex> lock(g_track_mutex);
		if(!g_active_allocations.insert(ptr).second) {
			std::cerr << "Address " << ptr << " allocated without being freed first!" << std::endl;
			std::abort();
		}
	}
	g_is_tracking = false;*/
	TracyAlloc(ptr, size);
	g_is_tracking = false;
}

inline void TrackFree(void *ptr)
{
	if(!ptr)
		return;

	/*if(!g_is_tracking) {
		g_is_tracking = true;
		{
			std::lock_guard<std::mutex> lock(g_track_mutex);
			g_active_allocations.erase(ptr);
		}
		g_is_tracking = false;
		TracyFree(ptr);
	}*/
	g_is_tracking = true;
	TracyFree(ptr);
	g_is_tracking = false;
}

inline void *SafeAlignedAlloc(std::size_t size, std::align_val_t align)
{
	std::size_t al = static_cast<std::size_t>(align);
	std::size_t rounded_size = (size + al - 1) & ~(al - 1);
	return ::aligned_alloc(al, rounded_size);
}

void *operator new(std::size_t count)
{
	//auto ptr = malloc(count);
	auto ptr = __libc_malloc(count);
	if(!ptr)
		throw std::bad_alloc {};
	TrackAlloc(ptr, count);
	return ptr;
}

void *operator new[](std::size_t count)
{
	//auto ptr = malloc(count);
	auto ptr = __libc_malloc(count);
	if(!ptr)
		throw std::bad_alloc {};
	TrackAlloc(ptr, count);
	return ptr;
}

void operator delete(void *ptr) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

void operator delete[](void *ptr) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

void *operator new(std::size_t count, const std::nothrow_t &) noexcept
{
	//auto *ptr = malloc(count);
	auto ptr = __libc_malloc(count);
	TrackAlloc(ptr, count);
	return ptr;
}

void *operator new[](std::size_t count, const std::nothrow_t &) noexcept
{
	//auto *ptr = malloc(count);
	auto ptr = __libc_malloc(count);
	TrackAlloc(ptr, count);
	return ptr;
}

void operator delete(void *ptr, const std::nothrow_t &) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

// Sized delete (c++14)
void operator delete(void *ptr, std::size_t count) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

void operator delete[](void *ptr, std::size_t count) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

// Aligned new/delete (c++17)
void *operator new(std::size_t size, std::align_val_t align)
{
	auto *ptr = SafeAlignedAlloc(size, align);
	if(!ptr)
		throw std::bad_alloc {};
	TrackAlloc(ptr, size);
	return ptr;
}

void *operator new[](std::size_t size, std::align_val_t align)
{
	auto *ptr = SafeAlignedAlloc(size, align);
	if(!ptr)
		throw std::bad_alloc {};
	TrackAlloc(ptr, size);
	return ptr;
}

void operator delete(void *ptr, std::align_val_t) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

void operator delete[](void *ptr, std::align_val_t) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

// Sized Aligned delete (c++17)
void operator delete(void *ptr, std::size_t count, std::align_val_t) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

void operator delete[](void *ptr, std::size_t count, std::align_val_t) noexcept
{
	TrackFree(ptr);
	//free(ptr);
	__libc_free(ptr);
}

extern "C" {
	void *malloc(std::size_t size)
	{
		auto *ptr = __libc_malloc(size);
		TrackAlloc(ptr, size);
		return ptr;
	}

	void free(void *ptr)
	{
		TrackFree(ptr);
		__libc_free(ptr);
	}

	void *calloc(std::size_t n, std::size_t size)
	{
		auto *ptr = __libc_calloc(n, size);
		TrackAlloc(ptr, n * size);
		return ptr;
	}

	void *realloc(void *ptr, std::size_t size)
	{
		// realloc can free ptr and return a new one, or return the same ptr
		// We must un-track the old pointer before re-tracking the new one
		TrackFree(ptr);
		auto *newptr = __libc_realloc(ptr, size);
		if(newptr)
			TrackAlloc(newptr, size);
		else if(size != 0)
			TrackAlloc(ptr, 0); // realloc failed; original ptr is still valid, re-track it
		return newptr;
	}
}
